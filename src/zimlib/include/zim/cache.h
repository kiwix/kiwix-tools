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

#include <map>
#include <limits>
#include <iostream>

#ifdef _WIN32
#define NOMINMAX
#  include <windows.h>
#undef NOMINMAX
#undef max
#endif

namespace zim
{
  /**
     Implements a container for caching elements.

     The cache holds a list of key-value-pairs. There are 2 main operations for
     accessing the cache: put and get. Put takes a key and a value and puts the
     element into the list. Get takes a key and optional a value. If the value
     for the key is found, it is returned. The passed value otherwise. By
     default the value is constructed with the empty ctor of the value-type.

     The cache has a maximum size, after which key-value-pairs are dropped,
     when a new item is put into the cache.

     The algorithm for this cache is as follows:
       - when the cache is not full, new elements are appended
       - new elements are put into the middle of the list otherwise
       - the last element of the list is then dropped
       - when getting a value and the value is found, it is put to the
         beginning of the list

     When elements are searched, a linear search is done using the ==-operator
     of the key type.

     The caching algorithm keeps elements, which are fetched more than once in
     the first half of the list. In the second half the elements are either new
     or the elements are pushed from the first half to the second half by other
     elements, which are found in the cache.

     You should be aware, that the key type should be simple. Comparing keys
     must be cheap. Copying elements (both key and value) must be possible and
     should be cheap, since they are moved in the underlying container.

   */
  template <typename Key, typename Value>
  class Cache
  {
      struct Data
      {
        bool winner;
        unsigned serial;
        Value value;
        Data() { }
        Data(bool winner_, unsigned serial_, const Value& value_)
          : winner(winner_),
            serial(serial_),
            value(value_)
            { }
      };

      typedef std::map<Key, Data> DataType;
      DataType data;

      typename DataType::size_type maxElements;
      unsigned serial;
      unsigned hits;
      unsigned misses;

      unsigned _nextSerial()
      {
        if (serial == std::numeric_limits<unsigned>::max())
        {
          for (typename DataType::iterator it = data.begin(); it != data.end(); ++it)
            it->second.serial = 0;
          serial = 1;
        }

        return serial++;
      }

      typename DataType::iterator _getOldest(bool winner)
      {
        typename DataType::iterator foundElement = data.begin();

        typename DataType::iterator it = data.begin();

        for (++it; it != data.end(); ++it)
          if (it->second.winner == winner
            && (foundElement->second.winner != winner || it->second.serial < foundElement->second.serial))
              foundElement = it;

        return foundElement;
      }

      typename DataType::iterator _getNewest(bool winner)
      {
        typename DataType::iterator foundElement = data.begin();

        typename DataType::iterator it = data.begin();

        for (++it; it != data.end(); ++it)
          if (it->second.winner == winner
            && (foundElement->second.winner != winner || it->second.serial > foundElement->second.serial))
              foundElement = it;

        return foundElement;
      }

      // drop one element
      void _dropLooser()
      {
        // look for the oldest element in the list of loosers to drop it
        data.erase(_getOldest(false));
      }

      void _makeLooser()
      {
        // look for the oldest element in the list of winners to make it a looser
        typename DataType::iterator it = _getOldest(true);
        it->second.winner = false;
        it->second.serial = _nextSerial();
      }

    public:
      typedef typename DataType::size_type size_type;
      typedef Value value_type;

      explicit Cache(size_type maxElements_)
        : maxElements(maxElements_ + (maxElements_ & 1)),
          serial(0),
          hits(0),
          misses(0)
        { }

      /// returns the number of elements currently in the cache
      size_type size() const        { return data.size(); }

      /// returns the maximum number of elements in the cache
      size_type getMaxElements() const      { return maxElements; }

      void setMaxElements(size_type maxElements_)
      {
        size_type numWinners = size() < maxElements / 2 ? size() : maxElements / 2;

        maxElements_ += (maxElements_ & 1);

        if (maxElements_ > maxElements)
        {
          maxElements = maxElements_;

          while (numWinners < maxElements / 2)
          {
            _getNewest(false)->winner = true;
            ++numWinners;
          }
        }
        else
        {
          while (maxElements > maxElements_)
          {
            _dropLooser();
            _dropLooser();
            _makeLooser();
            maxElements -= 2;
          }

          while (numWinners > maxElements / 2)
          {
            _getNewest(true)->winner = false;
            --numWinners;
          }
        }

      }

      /// removes a element from the cache and returns true, if found
      bool erase(const Key& key)
      {
        typename DataType::iterator it = data.find(key);
        if (it == data.end())
          return false;

        if (it->second.winner)
          _getNewest(false)->winner=true;

        data.erase(it);
        return true;
      }

      /// clears the cache.
      void clear(bool stats = false)
      {
        data.clear();
        if (stats)
          hits = misses = 0;
      }

      /// puts a new element in the cache. If the element is already found in
      /// the cache, it is considered a cache hit and pushed to the top of the
      /// list.
      void put(const Key& key, const Value& value)
      {
        typename DataType::iterator it;
        if (data.size() < maxElements)
        {
          data.insert(data.begin(),
            typename DataType::value_type(key,
              Data(data.size() < maxElements / 2, _nextSerial(), value)));
        }
        else if ((it = data.find(key)) == data.end())
        {
          // element not found
          _dropLooser();
          data.insert(data.begin(),
            typename DataType::value_type(key,
              Data(false, _nextSerial(), value)));
        }
        else
        {
          // element found
          it->second.serial = _nextSerial();
          if (!it->second.winner)
          {
            // move element to the winner part
            it->second.winner = true;
            _makeLooser();
          }
        }
      }

      /// puts a new element on the top of the cache. If the element is already
      /// found in the cache, it is considered a cache hit and pushed to the
      /// top of the list. This method actually overrides the need, that a element
      /// needs a hit to get to the top of the cache.
      void put_top(const Key& key, const Value& value)
      {
        typename DataType::iterator it;
        if (data.size() < maxElements)
        {
          if (data.size() >= maxElements / 2)
            _makeLooser();

          data.insert(data.begin(),
            typename DataType::value_type(key,
              Data(true, _nextSerial(), value)));
        }
        else if ((it = data.find(key)) == data.end())
        {
          // element not found
          _dropLooser();
          _makeLooser();
          data.insert(data.begin(),
            typename DataType::value_type(key,
              Data(true, _nextSerial(), value)));
        }
        else
        {
          // element found
          it->second.serial = _nextSerial();
          if (!it->second.winner)
          {
            // move element to the winner part
            it->second.winner = true;
            _makeLooser();
          }
        }
      }

      Value* getptr(const Key& key)
      {
        typename DataType::iterator it = data.find(key);
        if (it == data.end())
          return 0;

        it->second.serial = _nextSerial();

        if (!it->second.winner)
        {
          // move element to the winner part
          it->second.winner = true;
          _makeLooser();
        }

        return &it->second.value;
      }

      /// returns a pair of values - a flag, if the value was found and the
      /// value if found or the passed default otherwise. If the value is
      /// found it is a cahce hit and pushed to the top of the list.
      std::pair<bool, Value> getx(const Key& key, Value def = Value())
      {
        Value* v = getptr(key);
        return v ? std::pair<bool, Value>(true, *v)
                 : std::pair<bool, Value>(false, def);
      }

      /// returns the value to a key or the passed default value if not found.
      /// If the value is found it is a cahce hit and pushed to the top of the
      /// list.
      Value get(const Key& key, Value def = Value())
      {
        return getx(key, def).second;
      }

      /// returns the number of hits.
      unsigned getHits() const    { return hits; }
      /// returns the number of misses.
      unsigned getMisses() const  { return misses; }
      /// returns the cache hit ratio between 0 and 1.
      double hitRatio() const     { return hits+misses > 0 ? static_cast<double>(hits)/static_cast<double>(hits+misses) : 0; }
      /// returns the ratio, between held elements and maximum elements.
      double fillfactor() const   { return static_cast<double>(data.size()) / static_cast<double>(maxElements); }

/*
      void dump(std::ostream& out) const
      {
        out << "cache max size=" << maxElements << " current size=" << size() << '\n';
        for (typename DataType::const_iterator it = data.begin(); it != data.end(); ++it)
        {
          out << "\tkey=\"" << it->first << "\" value=\"" << it->second.value << "\" serial=" << it->second.serial << " winner=" << it->second.winner << '\n';
        }
        out << "--------\n";
      }
*/

  };

}

#endif // ZIM_CACHE_H
