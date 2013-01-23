/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#ifndef KEY_SOURCE_HH
#define KEY_SOURCE_HH

#include <limits>
#include "Error.hh"

namespace PLEXIL
{

  /**
   * @class KeyTraits
   * @brief Provides key traits for KeySource templatized class.
   *
   * By default:
   * - Floating point keys use the (positive) denormalized range of values. 
   * - Integer types use the full range of values for that type.
   *
   * Derived classes may be specialized for subranges of num_t.
   * At a minimum, specializations should provide new methods for:
   * * static num_t unassignedKey()
   *   The result may not be a valid key value for the base class or
   *   any specialization for num_t.
   * * static num_t keyMin()
   * * static num_t keyMax()
   *   keyMin() and keyMax() should bound an inclusive range that is disjoint with
   *   the base class, and with other specializations for num_t.
   *
   * Specializations may provide new methods for:
   * * static bool isValid()
   *   Only required if the specialization imposes stricter requirements on num_t.
   * * static num_t keyIncrement()
   *
   * @tparam num_t A numeric type name.
   */

  template <typename num_t>
  class KeyTraits
  {
  protected:
    typedef typename std::numeric_limits<num_t> limits_t;

  public:
    
    /**
     * @brief Check the basic assumptions of the key traits.
     * @return True if the class is valid for use as a key, false otherwise.
     */
    static bool isValid()
    {
      return limits_t::is_specialized
        && (limits_t::is_integer || limits_t::has_denorm == std::denorm_present);
    }

    /**
     * @brief Calculate the increment between keys.
     * @return The increment.
     */
    static num_t keyIncrement()
    {
      if (limits_t::is_integer)
        return 1;
      else
        return limits_t::denorm_min();
    }

    /**
     * @brief Calculate the unassigned-key value.
     * @return The unassigned key value.
     */
    static num_t unassignedKey()
    {
      if (limits_t::is_integer)
        return limits_t::min();
      else
        return limits_t::denorm_min();
    }

    /**
     * @brief Calculate the inclusive minimum valid key value.
     * @return The minimum key value.
     */
    static num_t keyMin()
    {
      return unassignedKey() + keyIncrement();
    }

    /**
     * @brief Calculate the inclusive maximum valid key value.
     * @return The maximum key value.
     */
    static num_t keyMax()
    {
      if (limits_t::is_integer)
        return limits_t::max();
      else
        return limits_t::min() - keyIncrement();
    }
  };

  /**
   * @class NegativeDenormKeyTraits
   * @brief A specialization of KeyTraits using the negative denormalized range of
   *        floating point types.
   * @tparam num_t A fundamental floating point numeric type.
   */
  template <typename num_t>
  class NegativeDenormKeyTraits :
    public KeyTraits<num_t>
  {
  private:
    // Work arouud apparent lack of inheritance from base class
    typedef typename std::numeric_limits<num_t> limits_t;

  public:
    static bool isValid()
    {
      return limits_t::is_specialized && limits_t::is_signed
        && (limits_t::has_denorm == std::denorm_present);
    }
    /**
     * @brief Calculate the unassigned-key value.
     * @return The unassigned key value.
     */
    static num_t unassignedKey()
    {
      return limits_t::denorm_min() - limits_t::min();
    }

    /**
     * @brief Calculate the inclusive minimum valid key value.
     * @return The minimum key value.
     */
    static num_t keyMin()
    {
      return unassignedKey() + KeyTraits<num_t>::keyIncrement();
    }

    /**
     * @brief Calculate the inclusive maximum valid key value.
     * @return The maximum key value.
     */
    static num_t keyMax()
    {
      return -limits_t::denorm_min();
    }
  };

  /**
   * @class KeySource
   * @brief Provides an abstract key source for ItemStore.
   * @tparam key_t A numeric type for which std::numeric_limits<key_t> is specialized.
   * @tparam key_traits_t
   *
   */

  template <typename key_t,
            typename key_traits_t = KeyTraits<key_t> >
  class KeySource
  {
  public:

    KeySource()
      : m_counter(unassigned())
    {
      // Check potential gotchas
      assertTrue(key_traits_t::isValid());
    }

    ~KeySource()
    {
    }

    /**
     * @brief Checks whether an arbitrary key is within the valid range. 
     * @return True if in range, false otherwise.
     */
    inline static bool rangeCheck(const key_t & key)
    {
      return key >= keyMin() && key <= keyMax();
    }

    /**
     * @brief Returns the next available key.
     */
    const key_t next()
    {
      // increment the counter
      assertTrue(keyMax() > m_counter, "KeySource::next: Key space exhausted.");
      m_counter += increment();
      return m_counter;
    }

    /**
     * @brief Returns the total number of keys which may be generated.
     * @note Currently used only in the module tests.
     */

    static const size_t& totalKeys()
    {
      static const size_t sl_totalKeys = 
        ((size_t) ((keyMax() - keyMin()) / increment())) + 1;
      return sl_totalKeys;
    }

    /**
     * @brief Returns the number of available keys remaining.
     * @note Currently used only in the module tests.
     */

    const size_t availableKeys()
    {
      return ((size_t) ((keyMax() - m_counter) / increment()));
    }

    /**
     * @brief Return the unassigned key value.
     * @return The unassigned key value.
     */
         
    static const key_t& unassigned()
    {
      static const key_t sl_unassigned = key_traits_t::unassignedKey();
      return sl_unassigned;
    }

    /**
     * @brief Return the minimum valid key value.
     * @return The smallest value of a key.
     */

    static const key_t& keyMin()
    {
      static const key_t sl_min = key_traits_t::keyMin();
      return sl_min;
    }

    /**
     * @brief Return the maximum key value.
     * @return The largest value of a key.
     */

    static const key_t& keyMax()
    {
      static const key_t sl_max = key_traits_t::keyMax();
      return sl_max;
    }

  protected:

    //
    // Below this point should be considered implementation details.
    //

    /**
     * @brief Return the key increment value.
     * @return The smallest allowable increment for a given key_t.
     */

    static const key_t& increment()
    {
      static const key_t sl_increment = key_traits_t::keyIncrement();
      return sl_increment;
    }

    void setCounter(key_t x)
    {
      m_counter = x;
    }

    key_t m_counter;     //!< The next key value.
  };

  /**
   * @class PartitionedKeySource
   * @brief A specialization of KeySource with a special subrange.
   *
   * The special key range is from keyMin() to specialMax() inclusive.
   * Regular keys use the range from specialMax() + increment() to keyMax() inclusive.
   * If the special range is exhausted, new keys are allocated from the regular range
   * until it too is exhausted.
   */

  template <typename key_t>
  class PartitionedKeySource :
    public KeySource<key_t, KeyTraits<key_t> >
  {
  public:
    PartitionedKeySource()
      : KeySource<key_t, KeyTraits<key_t> >(),
        m_specialCounter(KeySource<key_t, KeyTraits<key_t> >::unassigned())
    {
      // Bump regular counter past special range
      setCounter(specialMax() + KeySource<key_t, KeyTraits<key_t> >::increment());
    }

    static const key_t specialMax()
    {
      key_t sl_specialMax = 
        KeySource<key_t, KeyTraits<key_t> >::unassigned() + 1024 * KeySource<key_t, KeyTraits<key_t> >::increment();
      return sl_specialMax;
    }

    static size_t totalSpecialKeys()
    {
      return (size_t)
        ((specialMax() - KeySource<key_t, KeyTraits<key_t> >::unassigned())/KeySource<key_t, KeyTraits<key_t> >::increment());
    }

    size_t availableSpecialKeys() const
    {
      return (size_t) (specialMax() - m_specialCounter)/KeySource<key_t, KeyTraits<key_t> >::increment();

    }

    static bool isSpecial(key_t key)
    {
      return key > KeySource<key_t, KeyTraits<key_t> >::unassigned() && key <= specialMax();
    }

    /**
     * @brief Returns the next available key.
     * @param special True if the key should be allocated in the special range.
     * @note If special keys are exhausted, will return a normal key.
     */
    const key_t next(bool special = false)
    {
      if (special && m_specialCounter <= specialMax()) {
        m_specialCounter += KeySource<key_t, KeyTraits<key_t> >::increment();
        return m_specialCounter;
      }
      return KeySource<key_t, KeyTraits<key_t> >::next();
    }

  protected:

    key_t m_specialCounter;
  };

}

#endif // KEY_SOURCE_HH
