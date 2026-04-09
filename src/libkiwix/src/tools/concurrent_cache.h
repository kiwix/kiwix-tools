
/*
 * Copyright (C) 2021 Matthieu Gautier <mgautier@kymeria.fr>
 * Copyright (C) 2020 Veloman Yunkan
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef ZIM_CONCURRENT_CACHE_H
#define ZIM_CONCURRENT_CACHE_H

#include "lrucache.h"

#include <future>
#include <mutex>

namespace kiwix
{

/**
   ConcurrentCache implements a concurrent thread-safe cache

   Compared to kiwix::lru_cache, each access operation is slightly more expensive.
   However, different slots of the cache can be safely accessed concurrently
   with minimal blocking. Concurrent access to the same element is also
   safe, and, in case of a cache miss, will block until that element becomes
   available.
 */
template <typename Key, typename Value>
class ConcurrentCache
{
private: // types
  typedef std::shared_future<Value> ValuePlaceholder;
  typedef lru_cache<Key, ValuePlaceholder> Impl;

public: // types
  explicit ConcurrentCache(size_t maxEntries)
    : impl_(maxEntries)
  {}

  // Gets the entry corresponding to the given key. If the entry is not in the
  // cache, it is obtained by calling f() (without any arguments) and the
  // result is put into the cache.
  //
  // The cache as a whole is locked only for the duration of accessing
  // the respective slot. If, in the case of the a cache miss, the generation
  // of the missing element takes a long time, only attempts to access that
  // element will block - the rest of the cache remains open to concurrent
  // access.
  template<class F>
  Value getOrPut(const Key& key, F f)
  {
    std::promise<Value> valuePromise;
    std::unique_lock<std::mutex> l(lock_);
    const auto x = impl_.getOrPut(key, valuePromise.get_future().share());
    l.unlock();
    if ( x.miss() ) {
      try {
        valuePromise.set_value(f());
      } catch (std::exception& e) {
        drop(key);
        throw;
      }
    }

    return x.value().get();
  }

  bool drop(const Key& key)
  {
    std::unique_lock<std::mutex> l(lock_);
    return impl_.drop(key);
  }

  size_t setMaxSize(size_t new_size) {
    std::unique_lock<std::mutex> l(lock_);
    return  impl_.setMaxSize(new_size);
  }

protected: // data
  Impl impl_;
  std::mutex lock_;
};


/**
  WeakStore represent a thread safe store (map) of weak ptr.
  It allows to store weak_ptr from shared_ptr and retrieve shared_ptr from
  potential non expired weak_ptr.
  It is not limited in size.
*/
template<typename Key, typename Value>
class WeakStore {
  private: // types
    typedef std::weak_ptr<Value> WeakValue;

  public:
    explicit WeakStore() = default;

    std::shared_ptr<Value> get(const Key& key)
    {
      std::lock_guard<std::mutex> l(m_lock);
      auto it = m_weakMap.find(key);
      if (it != m_weakMap.end()) {
        auto shared = it->second.lock();
        if (shared) {
          return shared;
        } else {
          m_weakMap.erase(it);
        }
      }
      throw std::runtime_error("No weak ptr");
    }

    void add(const Key& key, std::shared_ptr<Value> shared)
    {
      std::lock_guard<std::mutex> l(m_lock);
      m_weakMap[key] = WeakValue(shared);
    }


  private: //data
    std::map<Key, WeakValue> m_weakMap;
    std::mutex m_lock;
};

template <typename Key, typename RawValue>
class ConcurrentCache<Key, std::shared_ptr<RawValue>>
{
private: // types
  typedef std::shared_ptr<RawValue> Value;
  typedef std::shared_future<Value> ValuePlaceholder;
  typedef lru_cache<Key, ValuePlaceholder> Impl;

public: // types
  explicit ConcurrentCache(size_t maxEntries)
    : impl_(maxEntries)
  {}

  // Gets the entry corresponding to the given key. If the entry is not in the
  // cache, it is obtained by calling f() (without any arguments) and the
  // result is put into the cache.
  //
  // The cache as a whole is locked only for the duration of accessing
  // the respective slot. If, in the case of the a cache miss, the generation
  // of the missing element takes a long time, only attempts to access that
  // element will block - the rest of the cache remains open to concurrent
  // access.
  template<class F>
  Value getOrPut(const Key& key, F f)
  {
    std::promise<Value> valuePromise;
    std::unique_lock<std::mutex> l(lock_);
    const auto x = impl_.getOrPut(key, valuePromise.get_future().share());
    l.unlock();
    if ( x.miss() ) {
      // Try to get back the shared_ptr from the weak_ptr first.
      try {
        valuePromise.set_value(m_weakStore.get(key));
      } catch(const std::runtime_error& e) {
        try {
          const auto value = f();
          valuePromise.set_value(value);
          m_weakStore.add(key, value);
        } catch (std::exception& e) {
          drop(key);
          throw;
        }
      }
    }

    return x.value().get();
  }

  bool drop(const Key& key)
  {
    std::unique_lock<std::mutex> l(lock_);
    return impl_.drop(key);
  }

  size_t setMaxSize(size_t new_size) {
    std::unique_lock<std::mutex> l(lock_);
    return  impl_.setMaxSize(new_size);
  }

protected: // data
  std::mutex lock_;
  Impl impl_;
  WeakStore<Key, RawValue> m_weakStore;
};

} // namespace kiwix

#endif // ZIM_CONCURRENT_CACHE_H

