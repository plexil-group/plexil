/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

  //
  // ArrayImpl
  //

  //! \class ArrayImpl
  //! \brief Class template for the internal representation of a PLEXIL Array.
  //! \ingroup Values
  template <typename T>
  class ArrayImpl : public Array
  {
  public:

    //! \brief Default constructor.
    ArrayImpl();

    //! \brief Copy constructor.
    ArrayImpl(ArrayImpl const &);

    //! \brief Move constructor.
    ArrayImpl(ArrayImpl &&);

    //! \brief Constructor with specified size.
    //! \param size The number of elements to allocate.
    //! \note All elements are initialized to unknown.
    ArrayImpl(size_t size);

    //! \brief Constructor with specified size and initial element value.
    //! \param size The number of elements to allocate.
    //! \param initval Const reference to the initial element value.
    //! \note All elements are initialized to the given initial value..
    ArrayImpl(size_t size, T const &initval);

    //! \brief Constructor from std::vector.
    //! \param initval The vector of initial values.
    //! \note The array's size is taken from the vector size.
    ArrayImpl(std::vector<T> const &initval);

    //! \brief Move constructor from std::vector.
    //! \param initval The vector of initial values.
    //! \note The array's size is taken from the vector size.
    ArrayImpl(std::vector<T> &&initval);

    //! \brief Virtual destructor.
    ~ArrayImpl();

    //! \brief Create an exact duplicate of this Array.
    //! \return Pointer to the new Array.
    virtual Array *clone() const override;

    //! \brief Copy assignment operator from generic Array.
    //! \param other The array being copied.
    virtual Array &operator=(Array const &other) override;

    //! \brief Move assignment operator from generic Array.
    //! \param other The array being moved.
    virtual Array &operator=(Array &&other) override;

    //! \brief Copy assignment operator from another ArrayImpl.
    //! \param other The array being copied.
    virtual ArrayImpl &operator=(ArrayImpl<T> const &other);

    //! \brief Move assignment operator from another ArrayImpl.
    //! \param other The array being moved.
    virtual ArrayImpl &operator=(ArrayImpl<T> &&other);

    //! \brief Expand the array to the requested size.
    //!        Mark the new elements as unknown.
    //!        If already that size or larger, does nothing.
    //! \param size The requested size.
    virtual void resize(size_t size) override;

    // Generic accessors

    //! \brief Get the valuue type of the elements of the array.
    //! \return The value type.
    virtual ValueType getElementType() const override;

    //! \brief Get the value of an element of the array as a Value instance.
    //! \param index The index.
    //! \return The Value instance.
    virtual Value getElementValue(size_t index) const override;

    //! \brief Equality operator.
    //! \param other Const reference to the Array being compared.
    //! \return True if the arrays have the same size, the same known vector,
    ///          and the same contents; false otherwise.
    virtual bool operator==(Array const &other) const override;

    //! \brief Equality operator.
    //! \param other Const reference to the ArrayImpl being compared.
    //! \return True if the arrays have the same size, the same known vector,
    ///          and the same contents; false otherwise.
    bool operator==(ArrayImpl<T> const &other) const;

    // Generic setter

    //! \brief Set the element at the given index to the given value.
    //! \param index The index.
    //! \param value The new value.
    virtual void setElementValue(size_t index, Value const &value) override;

    // Typed accessors

    //! \brief Get the value of an array element as the specified type,
    //!        and store it in the result variable.
    //! \param index The index.
    //! \param result Reference to the result variable.
    //! \return true if the element value is known and of the requested type,
    //!         false otherwise.
    virtual bool getElement(size_t index, T &result) const override;

    //! \brief Get a pointer to the implementation vector containing the array elements.
    //! \param result Reference to a pointer to the const vector.
    void getContentsVector(std::vector<T> const *&result) const;

    //! \brief Set an element of the array to the new value.
    //! \param index The index.
    //! \param newVal Const reference to the new value.
    virtual void setElement(size_t index, T const &newVal) override;

    //! \brief Print the array and its contents to an output stream.
    //! \param s The stream.
    virtual void print(std::ostream &s) const override;

    //
    // De/Serialization
    //

    //! \brief Write a serial representation of this object to the given
    //!        character array buffer.
    //! \param b First character of the buffer to write to.
    //! \return Pointer to the character after the last character written.
    virtual char *serialize(char *b) const override;

    //! \brief Read a serial representation from a buffer into this object.
    //! \param b Pointer to first character of the serial representation.
    //! \return Pointer to the character after the last character read.
    virtual char const *deserialize(char const *b) override;

    //! \brief Get the number of bytes required by a serial
    //!        representation of this object.
    //! \return The size.
    virtual size_t serialSize() const override; 

  private:
    std::vector<T> m_contents; //!< Storage for the array elements.
  };

  //
  // String is special because of getElementPointer()
  // So we have to duplicate every blasted member function.
  // C++ sucks at templates.
  //

  //! \class ArrayImpl<String>
  //! \brief Specialized internal representation for a PLEXIL Array of String.
  //! \ingroup Values
  template <>
  class ArrayImpl<String> : public Array
  {
  public:

    //! \brief Default constructor.
    ArrayImpl();

    //! \brief Copy constructor.
    ArrayImpl(ArrayImpl const &);

    //! \brief Move constructor.
    ArrayImpl(ArrayImpl &&);

    //! \brief Constructor with specified size.
    //! \param size The number of elements to allocate.
    //! \note All elements are initialized to unknown.
    ArrayImpl(size_t size);

    //! \brief Constructor with specified size and initial element value.
    //! \param size The number of elements to allocate.
    //! \param initval Const reference to the initial element value.
    //! \note All elements are initialized to the given initial value..
    ArrayImpl(size_t size, String const &initval);

    //! \brief Constructor from std::vector.
    //! \param initval The vector of initial values.
    //! \note The array's size is taken from the vector size.
    ArrayImpl(std::vector<String> const &initval);

    //! \brief Move constructor from std::vector of String.
    //! \param initval The vector of initial values.
    //! \note The array's size is taken from the vector size.
    ArrayImpl(std::vector<String> &&initval);

    //! \brief Virtual destructor.
    ~ArrayImpl();

    //! \brief Create an exact duplicate of this Array.
    //! \return Pointer to the new Array.
    virtual Array *clone() const override;

    //! \brief Copy assignment operator from generic Array.
    //! \param other The array being copied.
    virtual Array &operator=(Array const &) override;

    //! \brief Move assignment operator from generic Array.
    //! \param other The array being moved.
    virtual Array &operator=(Array &&) override;

    //! \brief Copy assignment operator from another ArrayImpl.
    //! \param other The array being copied.
    ArrayImpl &operator=(ArrayImpl<String> const &);

    //! \brief Move assignment operator from another ArrayImpl.
    //! \param other The array being moved.
    ArrayImpl &operator=(ArrayImpl<String> &&);

    //! \brief Expand the array to the requested size.
    //!        Mark the new elements as unknown.
    //!        If already that size or larger, does nothing.
    //! \param size The requested size.
    virtual void resize(size_t size) override;

    // Generic accessors

    //! \brief Get the valuue type of the elements of the array.
    //! \return The value type.
    virtual ValueType getElementType() const override;

    //! \brief Get the value of an element of the array as a Value instance.
    //! \param index The index.
    //! \return The Value instance.
    virtual Value getElementValue(size_t index) const override;

    //! \brief Equality operator.
    //! \param other Const reference to the Array being compared.
    //! \return True if the arrays have the same size, the same known vector,
    ///          and the same contents; false otherwise.
    virtual bool operator==(Array const &other) const override;

    //! \brief Equality operator.
    //! \param other Const reference to the ArrayImpl being compared.
    //! \return True if the arrays have the same size, the same known vector,
    ///          and the same contents; false otherwise.
    bool operator==(ArrayImpl<String> const &other) const;

    // Generic setter

    //! \brief Set the element at the given index to the given value.
    //! \param index The index.
    //! \param value The new value.
    virtual void setElementValue(size_t index, Value const &value) override;

    // Typed accessors

    //! \brief Get the value of an array element as the specified type,
    //!        and store it in the result variable.
    //! \param index The index.
    //! \param result Reference to the result variable.
    //! \return true if the element value is known and of the requested type,
    //!         false otherwise.
    virtual bool getElement(size_t index, String &result) const override;

    //! \brief Get a const pointer to the value of an array element,
    //!        and store it in the result variable.
    //! \param index The index.
    //! \param result Reference to the result variable.
    //! \return true if the element value is known and of the requested type,
    //!         false otherwise.
    virtual bool getElementPointer(size_t index, String const *&ptr) const override;

    //! \brief Get a pointer to the implementation vector containing the array elements.
    //! \param result Reference to a pointer to the const vector.
    void getContentsVector(std::vector<String> const *&result) const;

    //! \brief Set an element of the array to the new value.
    //! \param index The index.
    //! \param newVal Const reference to the new value.
    virtual void setElement(size_t index, String const &newVal) override;

    //! \brief Print the array and its contents to an output stream.
    //! \param s The stream.
    virtual void print(std::ostream &s) const override;

    //
    // De/Serialization
    //

    //! \brief Write a serial representation of this object to the given
    //!        character array buffer.
    //! \param b First character of the buffer to write to.
    //! \return Pointer to the character after the last character written.
    virtual char *serialize(char *b) const override;

    //! \brief Read a serial representation from a buffer into this object.
    //! \param b Pointer to first character of the serial representation.
    //! \return Pointer to the character after the last character read.
    virtual char const *deserialize(char const *b) override;

    //! \brief Get the number of bytes required by a serial
    //!        representation of this object.
    //! \return The size.
    virtual size_t serialSize() const override; 

  private:
    std::vector<String> m_contents; //!< Storage for the array elements.
  };

  //! \brief Overloaded inequality operator function template for ArrayImpl.
  //! \param arya Const reference to an ArrayImpl.
  //! \param aryb Const reference to another ArrayImpl.
  //! \return false if the arrays are equal in size and contents,
  //!         true otherwise.
  //! \ingroup Values
  template <typename T>
  bool operator!=(ArrayImpl<T> const &, ArrayImpl<T> const &);

  //! \brief Overloaded less-than operator function template for ArrayImpl.
  //! \param arya Const reference to an ArrayImpl.
  //! \param aryb Const reference to another ArrayImpl.
  //! \return true if arya is "less than" aryb, false otherwise.
  //! \ingroup Values
  template <typename T>
  bool operator<(ArrayImpl<T> const &, ArrayImpl<T> const &);

  //! \brief Overloaded less-than-or-equal operator function template for ArrayImpl.
  //! \param arya Const reference to an ArrayImpl.
  //! \param aryb Const reference to another ArrayImpl.
  //! \return true if arya is "less than or equal to" aryb, false otherwise.
  //! \ingroup Values
  template <typename T>
  bool operator<=(ArrayImpl<T> const &, ArrayImpl<T> const &);

  //! \brief Overloaded greater-than operator function template for ArrayImpl.
  //! \param arya Const reference to an ArrayImpl.
  //! \param aryb Const reference to another ArrayImpl.
  //! \return true if arya is "greater than" aryb, false otherwise.
  //! \ingroup Values
  template <typename T>
  bool operator>(ArrayImpl<T> const &, ArrayImpl<T> const &);

  //! \brief Overloaded greater-than-or-equal operator function template for ArrayImpl.
  //! \param arya Const reference to an ArrayImpl.
  //! \param aryb Const reference to another ArrayImpl.
  //! \return true if arya is "greater than or equal to" aryb, false otherwise.
  //! \ingroup Values
  template <typename T>
  bool operator>=(ArrayImpl<T> const &, ArrayImpl<T> const &);

  //! \brief Overloaded formatted output operator function template for ArrayImpl.
  //! \param str Reference to the output stream.
  //! \param ary Const reference to the array.
  //! \return Reference to the output stream.
  //! \ingroup Values
  template <typename T>
  std::ostream &operator<<(std::ostream &, ArrayImpl<T> const &);

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_IMPL_HH
