/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_SIMPLE_SET_HH
#define PLEXIL_SIMPLE_SET_HH

#include <algorithm>  // std::lower_bound, std::binary_search
#include <functional> // std::less
#include <vector>

namespace PLEXIL
{
  /**
   * @class SimpleSet
   * @brief A set stored in a vector, sorted by value.
   */
  template <typename VALUE_TYPE,
            class COMP = std::less<VALUE_TYPE> >
  class SimpleSet
  {
  public:
    typedef std::vector<VALUE_TYPE> STORE_TYPE;

    typedef typename STORE_TYPE::const_iterator const_iterator;
    typedef typename STORE_TYPE::iterator iterator;

    SimpleSet()
    {
    }

    SimpleSet(size_t initialCapacity)
    {
      m_store.reserve(initialCapacity);
    }

    // Virtual to allow for derived classes.
    virtual ~SimpleSet()
    {
    }

    /*
     * @brief Reserve space for n additional entries.
     * @param n The number of additional entries.
     */
    void grow(size_t n)
    {
      size_t desired = m_store.size() + n;
      m_store.reserve(desired);
    }

    bool insert(VALUE_TYPE const &val)
    {
      static Comparator s_comp;
      typename STORE_TYPE::iterator it = 
        std::lower_bound(m_store.begin(), m_store.end(), val, s_comp);
      if (it != m_store.end() && s_comp.equal(*it, val))
        return false; // duplicate
      this->insertEntry(it, val);
      return true;
    }

    // TODO: emplace() method

    const_iterator find(VALUE_TYPE const &val) const
    {
      static Comparator s_comp;
      typename STORE_TYPE::iterator it = 
        std::lower_bound(m_store.begin(), m_store.end(), val, s_comp);
      if (it == m_store.end() || s_comp.equal(*it, val))
        return it;
      else
        return m_store.end();
    }

    iterator find(VALUE_TYPE const &val)
    {
      static Comparator s_comp;
      typename STORE_TYPE::iterator it = 
        std::lower_bound(m_store.begin(), m_store.end(), val, s_comp);
      if (it == m_store.end() || s_comp.equal(*it, val))
        return it;
      else
        return m_store.end();
    }

    bool contains(VALUE_TYPE const &val) const
    {
      static Comparator s_comp;
      return std::binary_search(m_store.begin(), m_store.end(), val, s_comp);
    }

    iterator erase(iterator pos)
    {
      return m_store.erase(pos);
    }

    iterator erase(const_iterator pos)
    {
      return m_store.erase(pos);
    }

    typename STORE_TYPE::size_type erase(VALUE_TYPE const &val)
    {
      typename STORE_TYPE::iterator it = find(val);
      if (it == m_store.end())
        return 0;
      erase(it);
      return 1;
    }
    
    const_iterator begin() const
    {
      return m_store.begin();
    }

    iterator begin()
    {
      return m_store.begin();
    }

    const_iterator end() const
    {
      return m_store.end();
    }

    iterator end()
    {
      return m_store.end();
    }

    virtual void clear()
    {
      m_store.clear();
    }

    bool empty() const
    {
      return m_store.empty();
    }

    size_t size() const
    {
      return m_store.size();
    }

  protected:

    //
    // Extension API provided for implementors of derived classes
    //

    // Returns iterator to the new entry
    virtual iterator insertEntry(iterator it, VALUE_TYPE const &v)
    {
      return m_store.insert(it, v);
    }
    
    STORE_TYPE m_store;

  private:

    // Not implemented
#if __cplusplus >= 201103L
    SimpleSet(SimpleSet const &) = delete;
    SimpleSet(SimpleSet &&) = delete;
    SimpleSet &operator=(SimpleSet const &) = delete;
    SimpleSet &operator=(SimpleSet &&) = delete;
#else
    SimpleSet(SimpleSet const &);
    SimpleSet &operator=(SimpleSet const &);
#endif

    struct Comparator
    {
      bool operator() (VALUE_TYPE const &a, VALUE_TYPE const &b)
      {
        return COMP()(a, b);
      }

      bool equal(VALUE_TYPE const &a, VALUE_TYPE const &b)
      {
        return !COMP()(a, b) && !COMP()(b, a);
      }
    };
    
  };

} // namespace PLEXIL

#endif // PLEXIL_SIMPLE_SET_HH
