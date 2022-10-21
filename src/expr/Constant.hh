// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_CONSTANT_HH
#define PLEXIL_CONSTANT_HH

#include "ArrayImpl.hh"
#include "GetValueImpl.hh"

namespace PLEXIL {

  //! \class Constant
  //! \brief Templatized class for expressions whose value cannot change.
  //!        Expression listeners are therefore not required.
  //! \ingroup Expressions
  template <typename T>
  class Constant :
    public GetValueImpl<T>
  {
  public:

    //! \brief Default constructor.
    Constant();

    //! \brief Copy constructor.
    //! \param other The Constant instance to copy.
    Constant(const Constant &other);

    //! \brief Constructor from value type.
    //! \param value Const reference to the constant's value.
    Constant(const T &value);

    //! \brief Constructor from char *.
    //! \param value The initial value, as a null-terminated string.
    //! \note Unimplemented conversions will cause a link time error.
    Constant(const char * value);

    //! \brief Virtual destructor.
    virtual ~Constant() = default;

    //! \brief Return a print name for the expression type.
    //! \return A constant character string.
    virtual const char *exprName() const override;

    //! \brief Retrieve the value of this Expression in its native type.
    //! \param result The appropriately typed place to put the result.
    //! \return True if known, false if unknown.
    virtual bool getValue(T &result) const override;

    //! \brief Query whether the expression's value is known.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Query whether this expression is constant, i.e. incapable of change.
    //! \return True if constant, false otherwise.
    virtual bool isConstant() const override;

  protected:

    T m_value;    //!< The constant's value.
    bool m_known; //!< True if the constant is known, false if unknown.

  private:

    // Disallow assignment
    Constant &operator=(Constant const &) = delete;
    Constant &operator=(Constant &&) = delete;
  };

  //! \class Constant<String>
  //! \brief Specialization of the Constant template for String valued constants.
  //! \ingroup Expressions
  template <>
  class Constant<String> :
    public GetValueImpl<String>
  {
  public:

    //! \brief Default constructor.
    Constant();

    //! \brief Copy constructor.
    //! \param other The Constant instance to copy.
    Constant(const Constant &other);

    //! \brief Constructor from value type.
    //! \param value Const reference to the constant's value.
    Constant(const String &value);

    //! \brief Constructor from char *.
    //! \param value The initial value, as a null-terminated string.
    Constant(const char * value);

    //! \brief Virtual destructor.
    virtual ~Constant() = default;

    //! \brief Return a print name for the expression type.
    //! \return A constant character string.
    const char *exprName() const override;

    //! \brief Retrieve the value of this Expression in its native type.
    //! \param result The appropriately typed place to put the result.
    //! \return True if known, false if unknown.
    virtual bool getValue(String &result) const override;

    //! \brief Retrieve a pointer to the (const) value of this Expression.
    //! \param ptr Reference to the pointer variable to receive the result.
    //! \return True if known, false if unknown.
    virtual bool getValuePointer(String const *& ptr) const override;

    //! \brief Query whether the expression's value is known.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Query whether this expression is constant, i.e. incapable of change.
    //! \return True if constant, false otherwise.
    virtual bool isConstant() const override;

  protected:

    String m_value; //!< The constant's value.
    bool m_known;   //!< True if the value is known, false if unknown.

  private:

    // Disallow assignment
    Constant &operator=(Constant const &) = delete;
    Constant &operator=(Constant &&) = delete;
  };

  //! \brief Specialization of Constant class template for array-valued constants.
  //! \ingroup Expressions
  template <typename T>
  class Constant<ArrayImpl<T> > :
    public GetValueImpl<ArrayImpl<T> >
  {
  public:

    //! \brief Default constructor.
    Constant();

    //! \brief Copy constructor.
    //! \param other The Constant instance to copy.
    Constant(const Constant &other);

    //! \brief Constructor from value type.
    //! \param value Const reference to the constant's value.
    Constant(const ArrayImpl<T> &value);

    //! \brief Virtual destructor.
    virtual ~Constant() = default;

    //! \brief Return a print name for the expression type.
    //! \return A constant character string.
    const char *exprName() const override;

    //! \brief Retrieve a pointer to the (const) value of this Expression.
    //! \param ptr Reference to the pointer variable to receive the result.
    //! \return True if known, false if unknown.
    virtual bool getValuePointer(ArrayImpl<T> const *& ptr) const override;

    //! \brief Query whether the expression's value is known.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Query whether this expression is constant, i.e. incapable of change.
    //! \return True if constant, false otherwise.
    virtual bool isConstant() const override;

  protected:

    ArrayImpl<T> m_value; //!< The constant's value.
    bool m_known;         //!< True if the value is known, false if unknown.

  private:

    // Disallow assignment
    Constant &operator=(const Constant &) = delete;
    Constant &operator=(Constant &&) = delete;
  };


  //
  // Convenience type aliases
  //

  using BooleanConstant = Constant<Boolean>;
  using IntegerConstant = Constant<Integer>;
  using RealConstant    = Constant<Real>;
  using StringConstant  = Constant<String>;

  using BooleanArrayConstant = Constant<BooleanArray>;
  using IntegerArrayConstant = Constant<IntegerArray>;
  using RealArrayConstant    = Constant<RealArray>;
  using StringArrayConstant  = Constant<StringArray>;
  
} // namespace PLEXIL

#endif // PLEXIL_CONSTANT_HH
