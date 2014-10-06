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

#ifndef PLEXIL_ARRAY_IMPL_HH
#define PLEXIL_ARRAY_IMPL_HH

#include "Array.hh"

namespace PLEXIL
{
  /**
   * @class ArrayAdapter
   * @brief Dispatching layer between Array and ArrayImpl<T>. Contains no state of its own.
   * @note Another instance of the Curiously Recurring Template Pattern.
   * @note Should never be referenced except inside ArrayImpl<T>.
   */
  template <class IMPL>
  class ArrayAdapter : public Array
  {
  public:
    ~ArrayAdapter()
    {
    }

    bool getElement(size_t index, bool &result) const
    {
      return static_cast<const IMPL *>(this)->getElementImpl(index, result);
    }

    bool getElement(size_t index, int32_t &result) const
    {
      return static_cast<const IMPL *>(this)->getElementImpl(index, result);
    }

    bool getElement(size_t index, double &result) const
    {
      return static_cast<const IMPL *>(this)->getElementImpl(index, result);
    }

    bool getElement(size_t index, std::string &result) const
    {
      return static_cast<const IMPL *>(this)->getElementImpl(index, result);
    }

    bool getElementPointer(size_t index, std::string const *&result) const
    {
      return static_cast<const IMPL *>(this)->getElementPointerImpl(index, result);
    }

    void setElement(size_t index, bool const &newVal)
    {
      static_cast<IMPL *>(this)->setElementImpl(index, newVal);
    }

    void setElement(size_t index, int32_t const &newVal)
    {
      static_cast<IMPL *>(this)->setElementImpl(index, newVal);
    }

    void setElement(size_t index, double const &newVal)
    {
      static_cast<IMPL *>(this)->setElementImpl(index, newVal);
    }

    void setElement(size_t index, std::string const &newVal)
    {
      static_cast<IMPL *>(this)->setElementImpl(index, newVal);
    }

    void getContentsVector(std::vector<bool> const *& result) const
    {
      static_cast<IMPL const *>(this)->getContentsVectorImpl(result);
    }

    void getContentsVector(std::vector<int32_t> const *& result) const
    {
      static_cast<IMPL const *>(this)->getContentsVectorImpl(result);
    }

    void getContentsVector(std::vector<double> const *& result) const
    {
      static_cast<IMPL const *>(this)->getContentsVectorImpl(result);
    }

    void getContentsVector(std::vector<std::string> const *& result) const
    {
      static_cast<IMPL const *>(this)->getContentsVectorImpl(result);
    }

  protected:
    // Only available to derived classes
    ArrayAdapter()
      : Array()
    {
    }

    ArrayAdapter(ArrayAdapter const &orig)
    : Array(orig)
    {
    }
    
    ArrayAdapter(size_t size, bool known)
      : Array(size, known)
    {
    }

  private:
    // No one should ever call this.
    ArrayAdapter &operator=(ArrayAdapter const &);
  };

  //
  // ArrayImpl
  //
  template <typename T>
  class ArrayImpl : public ArrayAdapter<ArrayImpl<T> >
  {
  public:
    ArrayImpl();
    ArrayImpl(ArrayImpl const &);
    ArrayImpl(size_t size);
    ArrayImpl(size_t size, T const &initval);
    ArrayImpl(std::vector<T> const &initval);

    ~ArrayImpl();

    Array *clone() const;
    ArrayImpl &operator=(ArrayImpl<T> const &);

    void resize(size_t size);

    // Generic accessors
    ValueType getElementType() const;
    Value getElementValue(size_t index) const;

    bool operator==(Array const &other) const;
    bool operator==(ArrayImpl<T> const &other) const;

    // Generic setter
    void setElementValue(size_t index, Value const &value);

    // Typed accessors
    bool getElementImpl(size_t index, T &result) const;
    template <typename U>
    bool getElementImpl(size_t index, U &result) const;

    bool getElementPointerImpl(size_t index, T const *&result) const;
    template <typename U>
    bool getElementPointerImpl(size_t index, U const *&result) const;

    void getContentsVectorImpl(std::vector<T> const *&result) const;
    template <typename U>
    void getContentsVectorImpl(std::vector<U> const *&result) const;

    void setElementImpl(size_t index, T const &newVal);
    template <typename U>
    void setElementImpl(size_t index, U const &newVal);

    bool getMutableElementPointer(size_t index, std::string *&result);

    void print(std::ostream &s) const;

  private:
    std::vector<T> m_contents;
  };

  template <typename T>
  bool operator!=(ArrayImpl<T> const &, ArrayImpl<T> const &);

  template <typename T>
  bool operator<(ArrayImpl<T> const &, ArrayImpl<T> const &);

  template <typename T>
  bool operator<=(ArrayImpl<T> const &, ArrayImpl<T> const &);

  template <typename T>
  bool operator>(ArrayImpl<T> const &, ArrayImpl<T> const &);

  template <typename T>
  bool operator>=(ArrayImpl<T> const &, ArrayImpl<T> const &);

  template <typename T>
  std::ostream &operator<<(std::ostream &, ArrayImpl<T> const &);

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_IMPL_HH
