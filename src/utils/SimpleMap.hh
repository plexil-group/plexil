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
   * @class SimpleMapComparator
   * @brief A templatized comparator class for SimpleMap
   */
  template <typename KEY_TYPE, typename VALUE_TYPE>
  struct SimpleMapComparator
  {
    typedef std::pair<KEY_TYPE, VALUE_TYPE> MapEntry;

    //* Compare entry a less than entry b
    bool operator()(MapEntry const &a, MapEntry const &b) const
    {
      return a.first < b.first;
    }

    //* Compare entry a less than key b
    bool operator()(MapEntry const &a, KEY_TYPE const &b) const
    {
      return a.first < b;
    }

    //* Compare key a equal to key b
    bool equal(KEY_TYPE const &a, KEY_TYPE const &b) const
    {
      return a == b;
    }
  };

  /**
   * @class SimpleMap
   * @brief A key-value mapping stored as a vector, sorted by key value.
   */
  template <typename KEY_TYPE,
            typename VALUE_TYPE,
            class COMPARATOR = SimpleMapComparator<KEY_TYPE, VALUE_TYPE> >
  class SimpleMap
  {
  public:
    typedef std::pair<KEY_TYPE, VALUE_TYPE> MapEntry;
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

    // Virtual to allow for derived classes.
    virtual ~SimpleMap()
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

    bool insert(KEY_TYPE const &index, VALUE_TYPE const &val)
    {
      typename MapVector::iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, COMPARATOR());
      if (it != m_vector.end() && it->first == index)
        return false; // duplicate
      this->insertEntry(it, index, val);
      return true;
    }

    VALUE_TYPE &operator[](KEY_TYPE const &index)
    {
      typename MapVector::iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, COMPARATOR());
      if (it == m_vector.end() || it->first != index)
        it = this->insertEntry(it, index, VALUE_TYPE());
      return it->second;
    }

    VALUE_TYPE const &operator[](KEY_TYPE const &index) const
    {
      typename MapVector::const_iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, COMPARATOR());
      if (it == m_vector.end() || it->first != index) {
        static VALUE_TYPE const sl_empty;
        return sl_empty;
      }
      return it->second;
    }

    const_iterator find(KEY_TYPE const &index) const
    {
      static COMPARATOR s_comp;
      typename MapVector::const_iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, s_comp);
      if (it == m_vector.end())
        return it;
      else if (s_comp.equal(it->first, index))
        return it;
      else
        return m_vector.end();
    }

    iterator find(KEY_TYPE const &index)
    {
      static COMPARATOR s_comp;
      typename MapVector::iterator it = 
        std::lower_bound(m_vector.begin(), m_vector.end(), index, s_comp);
      if (it == m_vector.end())
        return it;
      else if (s_comp.equal(it->first, index))
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

    virtual void clear()
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

    size_t capacity() const
    {
      return m_vector.capacity();
    }

  protected:

    //
    // Extension API provided for implementors of derived classes
    //

    // Returns iterator to the new entry
    virtual iterator insertEntry(iterator it, KEY_TYPE const &k, VALUE_TYPE const &v)
    {
      return m_vector.insert(it, MapEntry(k, v));
    }
    
    MapVector m_vector;

  private:
    // Not implemented
    SimpleMap(SimpleMap const &);
    SimpleMap &operator=(SimpleMap const &);

  };

} // namespace PLEXIL

#endif // PLEXIL_SIMPLE_MAP_HH
