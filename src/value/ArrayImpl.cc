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

#include "ArrayImpl.hh"
#include "PlanError.hh"
#include "PlexilTypeTraits.hh"
#include "Value.hh"

#if defined(HAVE_CSTRING)
#include <cstring> // memcpy()
#elif defined(HAVE_STRING_H)
#include <string.h> // memcpy()
#endif

namespace PLEXIL
{

  template <typename T>
  ArrayImpl<T>::ArrayImpl()
    : Array()
  {
  }

  ArrayImpl<String>::ArrayImpl()
    : Array()
  {
  }

  template <typename T>
  ArrayImpl<T>::ArrayImpl(ArrayImpl<T> const &orig)
    : Array(orig),
      m_contents(orig.m_contents)
  {
  }

  ArrayImpl<String>::ArrayImpl(ArrayImpl<String> const &orig)
    : Array(orig),
      m_contents(orig.m_contents)
  {
  }

#if __cplusplus >= 201103L
  template <typename T>
  ArrayImpl<T>::ArrayImpl(ArrayImpl<T> &&orig)
    : Array(orig),
      m_contents(std::move(orig.m_contents))
  {
  }

  ArrayImpl<String>::ArrayImpl(ArrayImpl<String> &&orig)
    : Array(orig),
      m_contents(std::move(orig.m_contents))
  {
  }
#endif

  template <typename T>
  ArrayImpl<T>::ArrayImpl(size_t size)
  : Array(size, false),
    m_contents(size)
  {
  }

  ArrayImpl<String>::ArrayImpl(size_t size)
  : Array(size, false),
    m_contents(size)
  {
  }

  template <typename T>
  ArrayImpl<T>::ArrayImpl(size_t size, T const &initval)
  : Array(size, true),
    m_contents(size, initval)
  {
  }

  ArrayImpl<String>::ArrayImpl(size_t size, String const &initval)
  : Array(size, true),
    m_contents(size, initval)
  {
  }

#if __cplusplus >= 201103L
  template <typename T>
  ArrayImpl<T>::ArrayImpl(std::vector<T> &&initval)
    : Array(initval.size(), true),
    m_contents(std::move(initval))
  {
  }

  ArrayImpl<String>::ArrayImpl(std::vector<String> &&initval)
    : Array(initval.size(), true),
    m_contents(std::move(initval))
  {
  }
#endif

  template <typename T>
  ArrayImpl<T>::ArrayImpl(std::vector<T> const &initval)
    : Array(initval.size(), true),
      m_contents(initval)
  {
  }

  ArrayImpl<String>::ArrayImpl(std::vector<String> const &initval)
    : Array(initval.size(), true),
      m_contents(initval)
  {
  }

  template <typename T>
  ArrayImpl<T>::~ArrayImpl()
  {
  }

  ArrayImpl<String>::~ArrayImpl()
  {
  }

  template <typename T>
  Array *ArrayImpl<T>::clone() const
  {
    return new ArrayImpl<T>(*this);
  }

  Array *ArrayImpl<String>::clone() const
  {
    return new ArrayImpl<String>(*this);
  }

  template <typename T>
  Array &ArrayImpl<T>::operator=(Array const &orig)
  {
    ArrayImpl<T> const *typedOrig =
      dynamic_cast<ArrayImpl<T> const *>(&orig);
    checkPlanError(typedOrig,
                   "Can't assign array of element type " << valueTypeName(orig.getElementType())
                   << " to array of element type " << valueTypeName(getElementType()));
    return this->operator=(*typedOrig);
  }

  Array &ArrayImpl<String>::operator=(Array const &orig)
  {
    ArrayImpl<String> const *typedOrig =
      dynamic_cast<ArrayImpl<String> const *>(&orig);
    checkPlanError(typedOrig,
                   "Can't assign array of element type " << valueTypeName(orig.getElementType())
                   << " to array of element type String");
    return this->operator=(*typedOrig);
  }

#if __cplusplus >= 201103L
  template <typename T>
  Array &ArrayImpl<T>::operator=(Array && orig)
  {
    checkPlanError(dynamic_cast<ArrayImpl<T> *>(&orig),
                   "Can't assign array of element type " << valueTypeName(orig.getElementType())
                   << " to array of element type " << valueTypeName(getElementType()));
    return this->operator=(static_cast<ArrayImpl<T> &&>(orig));
  }

  Array &ArrayImpl<String>::operator=(Array &&orig)
  {
    checkPlanError(dynamic_cast<ArrayImpl<String> *>(&orig),
                   "Can't assign array of element type " << valueTypeName(orig.getElementType())
                   << " to array of element type String");
    return this->operator=(static_cast<ArrayImpl<String> &&>(orig));
  }
#endif
  
  template <typename T>
  ArrayImpl<T> &ArrayImpl<T>::operator=(ArrayImpl<T> const &orig)
  {
    Array::operator=(orig);
    m_contents = orig.m_contents;
    return *this;
  }

  ArrayImpl<String> &ArrayImpl<String>::operator=(ArrayImpl<String> const &orig)
  {
    Array::operator=(orig);
    m_contents = orig.m_contents;
    return *this;
  }

#if __cplusplus >= 201103L
  template <typename T>
  ArrayImpl<T> &ArrayImpl<T>::operator=(ArrayImpl<T> &&orig)
  {
    Array::operator=(orig);
    m_contents = std::move(orig.m_contents);
    return *this;
  }

  ArrayImpl<String> &ArrayImpl<String>::operator=(ArrayImpl<String> &&orig)
  {
    Array::operator=(orig);
    m_contents = std::move(orig.m_contents);
    return *this;
  }
#endif

  template <typename T>
  void ArrayImpl<T>::resize(size_t size)
  {
    Array::resize(size);
    m_contents.resize(size);
  }

  void ArrayImpl<String>::resize(size_t size)
  {
    Array::resize(size);
    m_contents.resize(size);
  }

  template <typename T>
  ValueType ArrayImpl<T>::getElementType() const
  {
    return PlexilValueType<T>::value;
  }

  ValueType ArrayImpl<String>::getElementType() const
  {
    return STRING_TYPE;
  }

  template <typename T>
  Value ArrayImpl<T>::getElementValue(size_t index) const
  {
    if (this->checkIndex(index) && this->m_known[index])
      return Value(m_contents[index]);
    return Value(); // unknown
  }

  Value ArrayImpl<String>::getElementValue(size_t index) const
  {
    if (this->checkIndex(index) && this->m_known[index])
      return Value(m_contents[index]);
    return Value(); // unknown
  }

  template <typename T>
  bool ArrayImpl<T>::getElement(size_t index, T &result) const
  {
    if (!this->checkIndex(index))
      return false;
    if (!this->m_known[index])
      return false;
    result = m_contents[index];
    return true;
  }

  bool ArrayImpl<String>::getElement(size_t index, String &result) const
  {
    if (!this->checkIndex(index))
      return false;
    if (!this->m_known[index])
      return false;
    result = m_contents[index];
    return true;
  }

  bool ArrayImpl<String>::getElementPointer(size_t index, String const *&result) const
  {
    if (!this->checkIndex(index))
      return false;
    if (!this->m_known[index])
      return false;
    result = &m_contents[index];
    return true;
  }

  // Generic equality
  template <typename T>
  bool ArrayImpl<T>::operator==(Array const &other) const
  {
    ArrayImpl<T> const *typedOther = 
      dynamic_cast<ArrayImpl<T> const *>(&other);
    if (!typedOther)
      return false;
    return operator==(*typedOther);
  }

  bool ArrayImpl<String>::operator==(Array const &other) const
  {
    ArrayImpl<String> const *typedOther = 
      dynamic_cast<ArrayImpl<String> const *>(&other);
    if (!typedOther)
      return false;
    return operator==(*typedOther);
  }

  // Specific equality
  template <typename T>
  bool ArrayImpl<T>::operator==(ArrayImpl<T> const &other) const
  {
    if (!(this->getKnownVector() == other.getKnownVector()))
      return false;
    return m_contents == other.m_contents;
  }

  bool ArrayImpl<String>::operator==(ArrayImpl<String> const &other) const
  {
    if (!(this->getKnownVector() == other.getKnownVector()))
      return false;
    return m_contents == other.m_contents;
  }

  template <typename T>
  void ArrayImpl<T>::getContentsVector(std::vector<T> const *&result) const
  {
    result = &m_contents;
  }

  void ArrayImpl<String>::getContentsVector(std::vector<String> const *&result) const
  {
    result = &m_contents;
  }

  template <typename T>
  void ArrayImpl<T>::setElement(size_t index, T const &newval)
  {
    if (!this->checkIndex(index))
      return;
    m_contents[index] = newval;
    this->m_known[index] = true;
  }

  void ArrayImpl<String>::setElement(size_t index, String const &newval)
  {
    if (!this->checkIndex(index))
      return;
    m_contents[index] = newval;
    this->m_known[index] = true;
  }

  template <typename T>
  void ArrayImpl<T>::setElementValue(size_t index, Value const &value)
  {
    if (!this->checkIndex(index))
      return;
    T temp;
    bool known = value.getValue(temp);
    if (known)
      m_contents[index] = temp;
    this->m_known[index] = known;
  }

  // Slight optimization for String
  void ArrayImpl<String>::setElementValue(size_t index, Value const &value)
  {
    if (!this->checkIndex(index))
      return;
    String const *temp;
    bool known = value.getValuePointer(temp);
    if (known)
      m_contents[index] = *temp;
    this->m_known[index] = known;
  }

  template <typename T>
  void ArrayImpl<T>::print(std::ostream &str) const
  {
    str << "#(";

    size_t len = this->size();
    size_t i = 0;

    while (i < len) {
      T temp;
      if (getElement(i, temp))
        printValue<T>(temp, str);
      else
        str << "UNKNOWN";
      if (++i < len)
        str << ' ';
    }
    // Print tail
    str << ')';
  }

  // Slight optimization for String
  void ArrayImpl<String>::print(std::ostream &str) const
  {
    str << "#(";

    size_t len = this->size();
    size_t i = 0;

    while (i < len) {
      String const *temp;
      if (getElementPointer(i, temp))
        printValue(*temp, str);
      else
        str << "UNKNOWN";
      if (++i < len)
        str << ' ';
    }
    // Print tail
    str << ')';
  }

  //
  // Non-member functions
  //

  template <typename T>
  bool operator==(ArrayImpl<T> const &arya, ArrayImpl<T> const &aryb)
  {
    if (!(arya.getKnownVector() == aryb.getKnownVector()))
      return false;
    std::vector<T> const *avec, *bvec;
    arya.getContentsVector(avec);
    aryb.getContentsVector(bvec);
    return *avec == *bvec;
  }

  // Generic
  template <typename T>
  bool operator!=(ArrayImpl<T> const &arya, Array const &aryb)
  {
    return !(arya == aryb);
  }

  // Specific
  template <typename T>
  bool operator!=(ArrayImpl<T> const &arya, ArrayImpl<T> const &aryb)
  {
    return !(arya == aryb);
  }

  template <typename T>
  bool operator<(ArrayImpl<T> const &arya, ArrayImpl<T> const &aryb)
  {
    std::vector<bool> const &aKnownVec = arya.getKnownVector();
    std::vector<bool> const &bKnownVec = aryb.getKnownVector();
    // Shorter is less
    size_t aSize = aKnownVec.size();
    size_t bSize = bKnownVec.size();
    if (aSize < bSize)
      return true;
    if (aSize > bSize)
      return false;

    // Same size
    std::vector<T> const *aVec, *bVec;
    arya.getContentsVector(aVec);
    aryb.getContentsVector(bVec);

    for (size_t i = 0; i < aSize; ++i) {
      // Unknown is less than known
      bool aKnown = aKnownVec[i];
      bool bKnown = bKnownVec[i];
      if (!aKnown && bKnown)
        return true; // unknown < known
      if (aKnown && !bKnown)
        return false;
      if (!aKnown)
        continue; // neither known

      // Compare values
      if ((*aVec)[i] < (*bVec)[i])
        return true;
      if ((*aVec)[i] > (*bVec)[i])
        return false;
    }
    return false; // equal
  }

  // Specialization for bool
  template <>
  bool operator< <Boolean>(ArrayImpl<Boolean> const &arya, ArrayImpl<Boolean> const &aryb)
  {
    std::vector<bool> const &aKnownVec = arya.getKnownVector();
    std::vector<bool> const &bKnownVec = aryb.getKnownVector();
    // Shorter is less
    size_t aSize = aKnownVec.size();
    size_t bSize = bKnownVec.size();
    if (aSize < bSize)
      return true;
    if (aSize > bSize)
      return false;

    // Same size
    std::vector<Boolean> const *aVec, *bVec;
    arya.getContentsVector(aVec);
    aryb.getContentsVector(bVec);

    for (size_t i = 0; i < aSize; ++i) {
      // Unknown is less than known
      bool aKnown = aKnownVec[i];
      bool bKnown = bKnownVec[i];
      if (!aKnown && bKnown)
        return true; // unknown < known
      if (aKnown && !bKnown)
        return false;
      if (!aKnown)
        continue; // neither known

      // False less than true
      if (!(*aVec)[i] && (*bVec)[i])
        return true;
      if ((*aVec)[i] && !(*bVec)[i])
        return false;
    }
    return false; // equal
  }

  template <typename T>
  bool operator<=(ArrayImpl<T> const &arya, ArrayImpl<T> const &aryb)
  {
    return !(aryb < arya);
  }


  template <typename T>
  bool operator>(ArrayImpl<T> const &arya, ArrayImpl<T> const &aryb)
  {
    return aryb < arya;
  }

  template <typename T>
  bool operator>=(ArrayImpl<T> const &arya, ArrayImpl<T> const &aryb)
  {
    return !(arya < aryb);
  }


  template <typename T>
  std::ostream &operator<<(std::ostream &str, ArrayImpl<T> const &ary)
  {
    ary.print(str);
    return str;
  }

  //
  // Serialization/deserialization
  //

  // Internal template
  template <typename T>
  size_t elementSerialSize(T const & /* o */)
  {
    return 0;
  }

  // Internal template
  template <typename T>
  char *serializeElement(T const & /* o */, char * /* buf */)
  {
    return NULL;
  }

  // Internal template
  template <typename T>
  char const *deserializeElement(T & /* o */, char const * /* buf */)
  {
    return NULL;
  }

  //
  // Boolean
  //

  template <>
  char *serializeElement<Boolean>(Boolean const &val, char *buf)
  {
    *buf++ = (char) val;
    return buf;
  }

  template <>
  char const *deserializeElement<Boolean>(Boolean &val, char const *buf)
  {
    val = (Boolean) *buf++;
    return buf;
  }

  template <>
  size_t elementSerialSize<Boolean>(Boolean const & /* o */)
  {
    return 1;
  }

  //
  // Integer
  //

  template <>
  char *serializeElement<Integer>(Integer const &val, char *buf)
  {
    // Store in big-endian format
    *buf++ = (char) (0xFF & (val >> 24));
    *buf++ = (char) (0xFF & (val >> 16));
    *buf++ = (char) (0xFF & (val >> 8));
    *buf++ = (char) (0xFF & val);
    return buf;
  }

  template <>
  char const *deserializeElement<Integer>(Integer &val, char const *buf)
  {
    uint32_t result = ((uint32_t) (unsigned char) *buf++) << 8;
    result = (result + (uint32_t) (unsigned char) *buf++) << 8;
    result = (result + (uint32_t) (unsigned char) *buf++) << 8;
    result = (result + (uint32_t) (unsigned char) *buf++);
    val = (Integer) result;
    return buf;
  }

  template <>
  size_t elementSerialSize<Integer>(Integer const & /* o */)
  {
    return 4;
  }

  //
  // Real
  //

  template <>
  char *serializeElement<Real>(Real const &val, char *buf)
  {
    union {
      Real reel;
      uint64_t lung;
    };
    reel = val;
    // Store in big-endian format
    *buf++ = (char) (0xFF & (lung >> 56));
    *buf++ = (char) (0xFF & (lung >> 48));
    *buf++ = (char) (0xFF & (lung >> 40));
    *buf++ = (char) (0xFF & (lung >> 32));
    *buf++ = (char) (0xFF & (lung >> 24));
    *buf++ = (char) (0xFF & (lung >> 16));
    *buf++ = (char) (0xFF & (lung >> 8));
    *buf++ = (char) (0xFF & lung);
    return buf;
  }

  template <>
  char const *deserializeElement<Real>(Real &val, char const *buf)
  {
    union {
      Real reel;
      uint64_t lung;
    };
    lung = (uint64_t) (unsigned char) *buf++; lung = lung << 8;
    lung += (uint64_t) (unsigned char) *buf++; lung = lung << 8;
    lung += (uint64_t) (unsigned char) *buf++; lung = lung << 8;
    lung += (uint64_t) (unsigned char) *buf++; lung = lung << 8;
    lung += (uint64_t) (unsigned char) *buf++; lung = lung << 8;
    lung += (uint64_t) (unsigned char) *buf++; lung = lung << 8;
    lung += (uint64_t) (unsigned char) *buf++; lung = lung << 8;
    lung += (uint64_t) (unsigned char) *buf++;
    val = reel;
    return buf;
  }

  template <>
  size_t elementSerialSize<Real>(Real const & /* o */)
  {
    return 8;
  }

  //
  // String
  //

  template <>
  char *serializeElement<String>(String const &val, char *buf)
  {
    size_t siz = val.size();
    if (siz > 0xFFFFFF)
      return NULL; // too big

    // Put 3 bytes of size first - std::string may contain embedded NUL
    *buf++ = (char) (0xFF & (siz >> 16));
    *buf++ = (char) (0xFF & (siz >> 8));
    *buf++ = (char) (0xFF & siz);
    memcpy(buf, val.c_str(), siz);
    return buf + siz;
  }

  template <>
  char const *deserializeElement<String>(String &val, char const *buf)
  {
    // Get 3 bytes of size
    size_t siz = ((size_t) (unsigned char) *buf++) << 8;
    siz = (siz + (size_t) (unsigned char) *buf++) << 8;
    siz = siz + (size_t) (unsigned char) *buf++;

    val.replace(val.begin(), val.end(), buf, siz);
    return buf + siz;
  }

  template <>
  size_t elementSerialSize<String>(String const &val)
  {
    return 3 + val.size();
  }

  // Internal function
  // Big-endian by bit, little-endian by byte
  static char *serializeBoolVector(std::vector<bool> const &val, char *buf)
  {
    int siz = val.size();
    size_t i = 0;
    while (siz > 0) {
      uint8_t tmp = 0;
      uint8_t mask = 0x80;
      switch (siz) {
      default: // siz >= 8
	if (val[i++])
	  tmp |= mask;
	mask = mask >> 1;

      case 7:
	if (val[i++])
	  tmp |= mask;
	mask = mask >> 1;

      case 6:
	if (val[i++])
	  tmp |= mask;
	mask = mask >> 1;

      case 5:
	if (val[i++])
	  tmp |= mask;
	mask = mask >> 1;

      case 4:
	if (val[i++])
	  tmp |= mask;
	mask = mask >> 1;

      case 3:
	if (val[i++])
	  tmp |= mask;
	mask = mask >> 1;

      case 2:
	if (val[i++])
	  tmp |= mask;
	mask = mask >> 1;

      case 1:
	if (val[i++])
	  tmp |= mask;
	break;
      }

      *buf++ = tmp;
      siz -= 8;
    }
    
    return buf;
  }

  // Internal function
  // Read from buffer in big-endian form
  // Presumes vector size has already been set.
  static char const *deserializeBoolVector(std::vector<bool> &val, char const *buf)
  {
    int siz = val.size();
    size_t i = 0;
    while (siz > 0) {
      uint8_t tmp = *buf++;
      uint8_t mask = 0x80;
      switch (siz) {
      default: // siz >= 8
        val[i++] = tmp & mask;
        mask = mask >> 1;

      case 7:
        val[i++] = tmp & mask;
        mask = mask >> 1;

      case 6:
        val[i++] = tmp & mask;
        mask = mask >> 1;

      case 5:
        val[i++] = tmp & mask;
        mask = mask >> 1;

      case 4:
        val[i++] = tmp & mask;
        mask = mask >> 1;

      case 3:
        val[i++] = tmp & mask;
        mask = mask >> 1;

      case 2:
        val[i++] = tmp & mask;
        mask = mask >> 1;

      case 1:
        val[i++] = tmp & mask;
        break;
      }
      siz -= 8;
    }
    return buf;
  }

  // Internal function
  static size_t bitVectorSize(size_t nbits)
  {
    size_t result = nbits / 8; // integer division, rounds towards 0
    if (nbits % 8)
      result++;
    return result;
  }

  /**
   * @brief Write a binary version of the object to the given buffer.
   * @param o The object.
   * @param b Pointer to the insertion point in the buffer.
   * @return Pointer to first byte after the object; NULL if failed.
   */

  template <typename T>
  char *ArrayImpl<T>::serialize(char *buf) const
  {
    size_t siz = this->size();
    if (siz > 0xFFFFFF)
      return NULL; // too big to serialize

    // Write type code
    *buf++ = (char) PlexilValueType<T>::arrayValue;

    // Write 3 bytes of size
    *buf++ = (char) (0xFF & (siz >> 16));
    *buf++ = (char) (0xFF & (siz >> 8));
    *buf++ = (char) (0xFF & siz);

    // Write known vector
    buf = serializeBoolVector(this->m_known, buf);

    // Write array contents
    for (size_t i = 0; i < siz; ++i) {
      buf = serializeElement(m_contents[i], buf);
      if (!buf)
        return NULL; // serializeElement failed
    }
    return buf;
  }

  template <>
  char *ArrayImpl<Boolean>::serialize(char *buf) const
  {
    size_t siz = this->size();
    if (siz > 0xFFFFFF)
      return NULL; // too big to serialize

    // Write type code
    *buf++ = BOOLEAN_ARRAY_TYPE;

    // Write 3 bytes of size
    *buf++ = (char) (0xFF & (siz >> 16));
    *buf++ = (char) (0xFF & (siz >> 8));
    *buf++ = (char) (0xFF & siz);

    // Write known vector
    buf = serializeBoolVector(this->m_known, buf);

    // Write array contents
    buf = serializeBoolVector(m_contents, buf);

    return buf;
  }

  char *ArrayImpl<String>::serialize(char *buf) const
  {
    size_t siz = this->size();
    if (siz > 0xFFFFFF)
      return NULL; // too big to serialize

    // Write type code
    *buf++ = (char) PlexilValueType<String>::arrayValue;

    // Write 3 bytes of size
    *buf++ = (char) (0xFF & (siz >> 16));
    *buf++ = (char) (0xFF & (siz >> 8));
    *buf++ = (char) (0xFF & siz);

    // Write known vector
    buf = serializeBoolVector(this->m_known, buf);

    // Write array contents
    for (size_t i = 0; i < siz; ++i) {
      buf = serializeElement(m_contents[i], buf);
      if (!buf)
        return NULL; // serializeElement failed
    }
    return buf;
  }

  /**
   * @brief Read a binary representation from the buffer and store it to the result object.
   * @param o The result object.
   * @param buf Pointer to the representation in the buffer.
   * @return Pointer to first byte after the object; NULL if failed.
   */

  // General case
  template <typename T>
  char const *ArrayImpl<T>::deserialize(char const *buf)
  {
    // Check type code
    if (PlexilValueType<T>::arrayValue != (ValueType) *buf++)
      return NULL; // not an appropriate array

    // Get 3 bytes of size
    size_t siz = (size_t) *buf++; siz = siz << 8;
    siz += (size_t) *buf++; siz = siz << 8;
    siz += (size_t) *buf++;
    
    this->resize(siz);
    
    buf = deserializeBoolVector(this->m_known, buf);
    for (size_t i = 0; i < siz; ++i)
      buf = deserializeElement(m_contents[i], buf);

    return buf;
  }

  // Special case for Boolean
  template <>
  char const *ArrayImpl<Boolean>::deserialize(char const *buf)
  {
    if (BOOLEAN_ARRAY_TYPE != (ValueType) *buf++)
      return NULL; // not a Boolean array

    // Get 3 bytes of size
    size_t siz = (size_t) *buf++; siz = siz << 8;
    siz += (size_t) *buf++; siz = siz << 8;
    siz += (size_t) *buf++;
    this->resize(siz);
    
    buf = deserializeBoolVector(this->m_known, buf);
    buf = deserializeBoolVector(m_contents, buf);
    
    return buf;
  }

  char const *ArrayImpl<String>::deserialize(char const *buf)
  {
    // Check type code
    if (PlexilValueType<String>::arrayValue != (ValueType) *buf++)
      return NULL; // not an appropriate array

    // Get 3 bytes of size
    size_t siz = (size_t) *buf++; siz = siz << 8;
    siz += (size_t) *buf++; siz = siz << 8;
    siz += (size_t) *buf++;
    
    this->resize(siz);
    
    buf = deserializeBoolVector(this->m_known, buf);
    for (size_t i = 0; i < siz; ++i)
      buf = deserializeElement(m_contents[i], buf);

    return buf;
  }

  /**
   * @brief Calculate the size of the serial representation of the object.
   * @param o The object.
   * @return Number of bytes; 0 if the object is not serializable.
   */

  // Numeric case
  template <typename NUM>
  size_t ArrayImpl<NUM>::serialSize() const
  {
    NUM const dummy = 0;
    size_t siz = this->size();
    return 4 + bitVectorSize(siz) + siz * elementSerialSize(dummy);
  }

  template <>
  size_t ArrayImpl<Boolean>::serialSize() const
  {
    return 4 + 2 * bitVectorSize(this->size());
  }

  // Requires traversing entire array
  size_t ArrayImpl<String>::serialSize() const
  {
    size_t siz = this->size();
    size_t result = 4 + bitVectorSize(siz);
    for (size_t i = 0; i < siz; ++i)
      result += 3 + m_contents[i].size();
    return result;
  }

  //
  // Wrappers
  //

  // Seems template functions can't be partially specialized. Fooey.

#define DEF_ARRAY_SERDES_WRAPPERS(typ) \
  template <> char *serialize(ArrayImpl<typ> const &o, char *buf)	\
  {return o.serialize(buf);} \
\
  template <> char const *deserialize(ArrayImpl<typ> &o, char const *buf) \
  {return o.deserialize(buf);} \
\
  template <> size_t serialSize(ArrayImpl<typ> const &o) \
  {return o.serialSize();}

  DEF_ARRAY_SERDES_WRAPPERS(Boolean)
  DEF_ARRAY_SERDES_WRAPPERS(Integer)
  DEF_ARRAY_SERDES_WRAPPERS(Real)
  DEF_ARRAY_SERDES_WRAPPERS(String)

#undef DEF_ARRAY_SERDES_WRAPPERS

  //
  // Explicit instantiations
  //
  template class ArrayImpl<Boolean>;
  template class ArrayImpl<Integer>;
  template class ArrayImpl<Real>;
  // template class ArrayImpl<String>;

  template bool operator!=(ArrayImpl<Boolean> const &, ArrayImpl<Boolean> const &);
  template bool operator!=(ArrayImpl<Integer> const &, ArrayImpl<Integer> const &);
  template bool operator!=(ArrayImpl<Real> const &,    ArrayImpl<Real> const &);
  template bool operator!=(ArrayImpl<String> const &,  ArrayImpl<String> const &);

  // Explicitly defined above
  // template bool operator<(ArrayImpl<Boolean> const &, ArrayImpl<Boolean> const &);
  template bool operator<(ArrayImpl<Integer> const &, ArrayImpl<Integer> const &);
  template bool operator<(ArrayImpl<Real> const &,    ArrayImpl<Real> const &);
  template bool operator<(ArrayImpl<String> const &,  ArrayImpl<String> const &);

  template bool operator<=(ArrayImpl<Boolean> const &, ArrayImpl<Boolean> const &);
  template bool operator<=(ArrayImpl<Integer> const &, ArrayImpl<Integer> const &);
  template bool operator<=(ArrayImpl<Real> const &,    ArrayImpl<Real> const &);
  template bool operator<=(ArrayImpl<String> const &,  ArrayImpl<String> const &);

  template bool operator>(ArrayImpl<Boolean> const &, ArrayImpl<Boolean> const &);
  template bool operator>(ArrayImpl<Integer> const &, ArrayImpl<Integer> const &);
  template bool operator>(ArrayImpl<Real> const &,    ArrayImpl<Real> const &);
  template bool operator>(ArrayImpl<String> const &,  ArrayImpl<String> const &);

  template bool operator>=(ArrayImpl<Boolean> const &, ArrayImpl<Boolean> const &);
  template bool operator>=(ArrayImpl<Integer> const &, ArrayImpl<Integer> const &);
  template bool operator>=(ArrayImpl<Real> const &,    ArrayImpl<Real> const &);
  template bool operator>=(ArrayImpl<String> const &,  ArrayImpl<String> const &);

  template std::ostream &operator<<(std::ostream &s, ArrayImpl<Boolean> const &);
  template std::ostream &operator<<(std::ostream &s, ArrayImpl<Integer> const &);
  template std::ostream &operator<<(std::ostream &s, ArrayImpl<Real> const &);
  template std::ostream &operator<<(std::ostream &s, ArrayImpl<String> const &);

} // namespace PLEXIL
