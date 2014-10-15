/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PLEXIL_SIMPLE_MAP_HH
#define PLEXIL_SIMPLE_MAP_HH

#include <algorithm>
#include <vector>
#include <utility>

namespace PLEXIL
{
  /**
   * @class SimpleMap
   * @brief A key-value mapping stored as a vector, sorted by key value.
   */
  template <typename KEY, typename VALUE>
  class SimpleMap
  {
  public:
    typedef std::pair<KEY, VALUE> MapEntry;
    typedef std::vector<MapEntry> MapVector;

    typedef typename MapVector::const_iterator const_iterator;
    typedef typename MapVector::iterator iterator;

    SimpleMap()
    {
    }

    SimpleMap(size_t initialCapacity)
    {
      m_vector.reserve(initialCapacity);
    }

    ~SimpleMap()
    {
    }

    /*
     * @brief Reserve space for n additional entries.
     * @param n The number of additional entries.
     */
    void grow(size_t n)
    {
      size_t desired = m_vector.size() + n;
      m_vector.reserve(desired);
    }

    bool insert(KEY const &index, VALUE const &val)
    {
      typename MapVector::iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, SimpleMapCompare());
      if (it != m_vector.end() && it->first == index)
        return false; // duplicate
      m_vector.insert(it, MapEntry(index, val));
      return true;
    }

    VALUE &operator[](KEY const &index)
    {
      typename MapVector::iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, SimpleMapCompare());
      if (it == m_vector.end() || it->first != index)
        it = m_vector.insert(it, MapEntry(index, VALUE()));
      return it->second;
    }

    VALUE const &operator[](KEY const &index) const
    {
      typename MapVector::const_iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, SimpleMapCompare());
      if (it == m_vector.end() || it->first != index) {
        static VALUE const sl_empty;
        return sl_empty;
      }
      return it->second;
    }

    const_iterator find(KEY const &index) const
    {
      typename MapVector::const_iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, SimpleMapCompare());
      if (it == m_vector.end())
        return it;
      else if (it->first == index)
        return it;
      else
        return m_vector.end();
    }

    iterator find(KEY const &index)
    {
      typename MapVector::iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, SimpleMapCompare());
      if (it == m_vector.end())
        return it;
      else if (it->first == index)
        return it;
      else
        return m_vector.end();
    }

    const_iterator begin() const
    {
      return m_vector.begin();
    }

    iterator begin()
    {
      return m_vector.begin();
    }

    const_iterator end() const
    {
      return m_vector.end();
    }

    iterator end()
    {
      return m_vector.end();
    }

    void clear()
    {
      m_vector.clear();
    }

    bool empty() const
    {
      return m_vector.empty();
    }

    size_t size() const
    {
      return m_vector.size();
    }

  private:
    // Not implemented
    SimpleMap(SimpleMap const &);
    SimpleMap &operator=(SimpleMap const &);

    // Utility class
    class SimpleMapCompare
    {
    public:
      bool operator()(SimpleMap::MapEntry const &a, SimpleMap::MapEntry const &b) const
      {
        return a.first < b.first;
      }

      bool operator()(SimpleMap::MapEntry const &a, KEY const &b) const
      {
        return a.first < b;
      }
    };

    MapVector m_vector;
  };

} // namespace PLEXIL

#endif // PLEXIL_SIMPLE_MAP_HH
