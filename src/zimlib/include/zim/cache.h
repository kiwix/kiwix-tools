/*
 * Copyright (C) 2008 Tommi Maekitalo
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

#ifndef ZIM_CACHE_H
#define ZIM_CACHE_H

#include <deque>
#include <utility>

namespace zim
{
  template <typename Key, typename Value>
  class Cache
  {
      typedef std::deque<std::pair<Key, Value> > DataType;
      DataType data;
      typename DataType::size_type maxElements;
      unsigned hits;
      unsigned misses;

    public:
      typedef typename DataType::size_type size_type;
      typedef typename DataType::value_type value_type;
      typedef typename DataType::iterator iterator;
      typedef typename DataType::const_iterator const_iterator;

      explicit Cache(size_type maxElements_)
        : maxElements(maxElements_),
          hits(0),
          misses(0)
        { }

      size_type size() const        { return data.size(); }
      iterator begin()              { return data.begin(); }
      iterator end()                { return data.end(); }
      const_iterator begin() const  { return data.begin(); }
      const_iterator end() const    { return data.end(); }

      size_type getMaxElements() const { return maxElements; }
      void setMaxElements(size_type maxElements_)
      {
        maxElements = maxElements_;
        if (data.size() > maxElements)
          data.erase(data.begin() + maxElements, data.end());
      }

      bool erase(const Key& key)  // returns true, if key was found and removed
      {
        for (typename DataType::iterator it = data.begin(); it != data.end(); ++it)
        {
          if (it->first == key)
          {
            data.erase(it);
            return true;
          }
        }

        return false;
      }

      void put(const Key& key, const Value& value)
      {
        for (typename DataType::iterator it = data.begin(); it != data.end(); ++it)
        {
          if (it->first == key)
          {
            data.erase(it);
            data.push_front(typename DataType::value_type(key, value));
            return;
          }
        }

        ++misses;

        if (data.size() < maxElements / 2)
          data.push_back(typename DataType::value_type(key, value));
        else
          data.insert(data.begin() + maxElements / 2, typename DataType::value_type(key, value));

        if (data.size() > maxElements)
          data.pop_back();
      }

      std::pair<bool, Value> getx(const Key& key, Value def = Value())
      {
        for (typename DataType::iterator it = data.begin(); it != data.end(); ++it)
        {
          if (it->first == key)
          {
            typename DataType::value_type v = *it;
            data.erase(it);
            data.push_front(v);
            ++hits;
            return std::pair<bool, Value>(true, v.second);
          }
        }

        return std::pair<bool, Value>(false, def);
      }

      Value get(const Key& key, Value def = Value())
      {
        return getx(key, def).second;
      }

      unsigned getHits() const    { return hits; }
      unsigned getMisses() const  { return misses; }
      double hitRatio() const     { return hits+misses > 0 ? static_cast<double>(hits)/static_cast<double>(hits+misses) : 0; }
      double fillfactor() const   { return static_cast<double>(data.size()) / static_cast<double>(maxElements); }
  };
}
#endif // ZIM_CACHE_H
