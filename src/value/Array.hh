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

#ifndef PLEXIL_ARRAY_HH
#define PLEXIL_ARRAY_HH

#include "ValueType.hh"

#include <vector>

namespace PLEXIL
{
  // Forward references
  class Value;

  //! \class Array
  //! \brief Abstract base class representing the PLEXIL API of an Array.
  //! \ingroup Values

  class Array
  {
  public:

    //! \brief Default constructor.
    Array();

    //! \brief Copy constructor.
    Array(Array const &);

    //! \brief Move constructor.
    Array(Array &&);

    //! \brief Constructor called from implementation classes.
    Array(size_t size, bool known = false);

    //! \brief Virtual destructor.
    virtual ~Array();

    //! \brief Create an exact duplicate of this Array.
    //! \return Pointer to the new Array.
    virtual Array *clone() const = 0;

    //! \brief Copy assignment operator.
    //! \param other The array being copied.
    virtual Array &operator=(Array const &other);

    //! \brief Move assignment operator.
    //! \param other The array being moved.
    virtual Array &operator=(Array &&other);

    // Generic accessors

    //! \brief Return the size of the array, i.e. the number of
    //         elements actually allocated.
    //! \return The size.
    size_t size() const;

    //! \brief Query whether the element at a particular index is known.
    //! \param index The index.
    //! \return True if known, false if unknown.
    bool elementKnown(size_t index) const;

    //! \brief Query whether every element of the array is known.
    //! \return True if all elements are known, false otherwise.
    bool allElementsKnown() const;

    //! \brief Query whether any element of the array is known.
    //! \return True if one or more elements are known, false otherwise.
    bool anyElementsKnown() const;

    //! \brief Get the vector of known flags for the elements of this array.
    //! \return A const reference to the vector.
    inline std::vector<bool> const &getKnownVector() const
    {
      return m_known;
    }


    //! \brief Get the valuue type of the elements of the array.
    //! \return The value type.
    virtual ValueType getElementType() const = 0;

    //! \brief Get the value of an element of the array as a Value instance.
    //! \param index The index.
    //! \return The Value instance.
    virtual Value getElementValue(size_t index) const = 0;

    //! \brief Equality operator.
    //! \param other Const reference to the Array being compared.
    //! \return True if the arrays have the same size, the same known vector,
    ///          and the same contents; false otherwise.
    virtual bool operator==(Array const &other) const;

    // Generic setters

    //! \brief Expand the array to the requested size.
    //!        Mark the new elements as unknown.
    //!        If already that size or larger, does nothing.
    //! \param size The requested size.
    virtual void resize(size_t size);

    //! \brief Set the element at the given index to unknown.
    //! \param index The index.
    void setElementUnknown(size_t index);

    //! \brief Set the element at the given index to the given value.
    //! \param index The index.
    //! \param value The new value.
    virtual void setElementValue(size_t index, Value const &value) = 0;

    //! \brief Set all elements of the array to unknown.
    virtual void reset();

    // Typed accessors
    // Default methods throw PlanError

    //! \brief Get the value of an array element as the specified type,
    //!        and store it in the result variable.
    //! \param index The index.
    //! \param result Reference to the result variable.
    //! \return true if the element value is known and of the requested type,
    //!         false otherwise.
    virtual bool getElement(size_t index, Boolean &result) const;
    virtual bool getElement(size_t index, Integer &result) const;
    virtual bool getElement(size_t index, Real &result) const;
    virtual bool getElement(size_t index, String &result) const;

    //! \brief Get a const pointer to the value of an array element,
    //!        and store it in the result variable.
    //! \param index The index.
    //! \param result Reference to the result variable.
    //! \return true if the element value is known and of the requested type,
    //!         false otherwise.
    virtual bool getElementPointer(size_t index, String const *&result) const;

    // Typed setters
    // Default methods throw PlanError

    //! \brief Set an element of the array to the new value.
    //! \param index The index.
    //! \param newVal Const reference to the new value.
    virtual void setElement(size_t index, Boolean const &newVal);
    virtual void setElement(size_t index, Integer const &newVal);
    virtual void setElement(size_t index, Real const &newVal);
    virtual void setElement(size_t index, String const &newVal);

    // Utility

    //! \brief Print the array and its contents to an output stream.
    //! \param s The stream.
    virtual void print(std::ostream &s) const = 0;

    //! \brief Get the printed representation of the array to a string.
    //! \return The string.
    virtual std::string toString() const;

    //
    // De/Serialization API
    //

    //! \brief Write a serial representation of this object to the given
    //!        character array buffer.
    //! \param b First character of the buffer to write to.
    //! \return Pointer to the character after the last character written.
    virtual char *serialize(char *b) const = 0; 

    //! \brief Read a serial representation from a buffer into this object.
    //! \param b Pointer to first character of the serial representation.
    //! \return Pointer to the character after the last character read.
    virtual char const *deserialize(char const *b) = 0;

    //! \brief Get the number of bytes required by a serial
    //!        representation of this object.
    //! \return The size.
    virtual size_t serialSize() const = 0;

  protected:

    // For use by implementation classes

    //! \brief Check whether an index is valid for this array.
    //! \param index The index.
    //! \return True if the index is valid, false if not.
    inline bool checkIndex(size_t index) const
    {
      return index < m_known.size();
    }

    //! \brief The vector of known flags.
    std::vector<bool> m_known;
  };

  //! \ingroup Values

  //! \brief Formatted output operator for Array.
  //! \param s The output stream.
  //! \param a Const reference to an Array.
  //! \return The output stream.
  std::ostream &operator<<(std::ostream &s, Array const &a);

  //! \ingroup Values
  template <> char *serialize<Array>(Array const &o, char *b);

  //! \ingroup Values
  template <> char const *deserialize<Array>(Array &o, char const *b);

  //! \ingroup Values
  template <> size_t serialSize(Array const &o);

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_HH
