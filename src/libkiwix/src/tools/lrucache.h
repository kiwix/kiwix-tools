/*
 * Copyrigth (c) 2021, Matthieu Gautier <mgautier@kymeria.fr>
 * Copyright (c) 2020, Veloman Yunkan
 * Copyright (c) 2014, lamerman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of lamerman nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * File:   lrucache.hpp
 * Author: Alexander Ponomarev
 *
 * Created on June 20, 2013, 5:09 PM
 */

#ifndef _LRUCACHE_HPP_INCLUDED_
#define _LRUCACHE_HPP_INCLUDED_

#include <map>
#include <list>
#include <set>
#include <cstddef>
#include <stdexcept>
#include <cassert>

namespace kiwix {

template<typename key_t, typename value_t>
class lru_cache {
public: // types
  typedef typename std::pair<key_t, value_t> key_value_pair_t;
  typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

  enum AccessStatus {
    HIT, // key was found in the cache
    PUT, // key was not in the cache but was created by the getOrPut() access
    MISS // key was not in the cache; get() access failed
  };

  class AccessResult
  {
    const AccessStatus status_;
    const value_t val_;
  public:
    AccessResult(const value_t& val, AccessStatus status)
      : status_(status), val_(val)
    {}
    AccessResult() : status_(MISS), val_() {}

    bool hit() const { return status_ == HIT; }
    bool miss() const { return !hit(); }
    const value_t& value() const
    {
      if ( status_ == MISS )
        throw std::range_error("There is no such key in cache");
      return val_;
    }

    operator const value_t& () const { return value(); }
  };

public: // functions
  explicit lru_cache(size_t max_size) :
    _max_size(max_size) {
  }

  // If 'key' is present in the cache, returns the associated value,
  // otherwise puts the given value into the cache (and returns it with
  // a status of a cache miss).
  AccessResult getOrPut(const key_t& key, const value_t& value) {
    auto it = _cache_items_map.find(key);
    if (it != _cache_items_map.end()) {
      _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
      return AccessResult(it->second->second, HIT);
    } else {
      putMissing(key, value);
      return AccessResult(value, PUT);
    }
  }

  void put(const key_t& key, const value_t& value) {
    auto it = _cache_items_map.find(key);
    if (it != _cache_items_map.end()) {
      _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
      it->second->second = value;
    } else {
      putMissing(key, value);
    }
  }

  AccessResult get(const key_t& key) {
    auto it = _cache_items_map.find(key);
    if (it == _cache_items_map.end()) {
      return AccessResult();
    } else {
      _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
      return AccessResult(it->second->second, HIT);
    }
  }

  bool drop(const key_t& key) {
    try {
      auto list_it = _cache_items_map.at(key);
      _cache_items_list.erase(list_it);
      _cache_items_map.erase(key);
      return true;
    } catch (std::out_of_range& e) {
      return false;
    }
  }

  bool exists(const key_t& key) const {
    return _cache_items_map.find(key) != _cache_items_map.end();
  }

  size_t size() const {
    return _cache_items_map.size();
  }

  size_t setMaxSize(size_t new_size) {
    size_t previous = _max_size;
    _max_size = new_size;
    return previous;
  }

  std::set<key_t> keys() const  {
    std::set<key_t> keys;
    for(auto& item:_cache_items_map) {
      keys.insert(item.first);
    }
    return keys;
  }

private: // functions
  void putMissing(const key_t& key, const value_t& value) {
    assert(_cache_items_map.find(key) == _cache_items_map.end());
    _cache_items_list.push_front(key_value_pair_t(key, value));
    _cache_items_map[key] = _cache_items_list.begin();
    while (_cache_items_map.size() > _max_size) {
      _cache_items_map.erase(_cache_items_list.back().first);
      _cache_items_list.pop_back();
    }
  }

private: // data
  std::list<key_value_pair_t> _cache_items_list;
  std::map<key_t, list_iterator_t> _cache_items_map;
  size_t _max_size;
};

} // namespace kiwix

#endif  /* _LRUCACHE_HPP_INCLUDED_ */
