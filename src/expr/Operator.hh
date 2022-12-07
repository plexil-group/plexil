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

#ifndef PLEXIL_OPERATOR_HH
#define PLEXIL_OPERATOR_HH

#include "ValueType.hh"

#include <vector>

namespace PLEXIL
{
  // Forward references
  class Expression;
  class Function;
  class Value;

  // TODO:
  // - Support printing

  //! \class Operator
  //! \brief Abstract base class representing a computation process to
  //!        be performed on zero or more expressions, returning a value.
  //! \see Function
  //! \ingroup Expressions
  class Operator
  {
  public:

    //! \brief Virtual destructor.
    virtual ~Operator() = default;

    //! \brief Get the name of this Operator.
    //! \return Const reference to the name string.
    std::string const &getName() const;

    //! \brief Query whether this operator is a source of change events.
    //! \return True if the value may change independently of any subexpressions, false otherwise.
    //! \note Default method returns false, i.e. return value depends entirely on subexpressions.
    //! \note Implementors should override where appropriate, e.g. random number generators.
    virtual bool isPropagationSource() const;

    //! \brief Check that the number of arguments is valid for this Operator.
    //! \param count The number of arguments.
    //! \return true if valid, false if not.
    //! \note Delegated to each individual operator.  Derived classes must implement a method.
    virtual bool checkArgCount(size_t count) const = 0;

    //! \brief Check that the argument types are valid for this Operator.
    //! \param typeVec Const reference to vector of types, 
    //! \return true if valid, false if not.
    //! \note Default method returns true.
    //! \note Derived classes may override this method..
    virtual bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    // Delegated to OperatorImpl by default

    //! \brief Return the value type of this Operation's result.
    //! \return The ValueType.
    virtual ValueType valueType() const = 0;

    //! \brief Allocate a cache for the result of this Operation.
    //! \return Pointer to the cache.  May be NULL.
    virtual void *allocateCache() const = 0;

    //! \brief Delete a cache for the result of this Operation.
    //! \param ptr Pointer to the cache
    virtual void deleteCache(void *Ptr) const = 0;

    // Local macro to generate a truckload of boilerplate
#define DECLARE_OPERATOR_METHODS(_rtype_) \
    virtual bool operator()(_rtype_ &result, Expression const *arg) const; \
    virtual bool operator()(_rtype_ &result, Expression const *arg0, Expression const *arg1) const; \
    virtual bool operator()(_rtype_ &result, Function const &args) const;

    DECLARE_OPERATOR_METHODS(Boolean)
    DECLARE_OPERATOR_METHODS(Integer)
    DECLARE_OPERATOR_METHODS(Real)
    DECLARE_OPERATOR_METHODS(String)

    DECLARE_OPERATOR_METHODS(NodeState)
    DECLARE_OPERATOR_METHODS(NodeOutcome)
    DECLARE_OPERATOR_METHODS(FailureType)
    DECLARE_OPERATOR_METHODS(CommandHandleValue)

    DECLARE_OPERATOR_METHODS(Array)
    DECLARE_OPERATOR_METHODS(BooleanArray)
    DECLARE_OPERATOR_METHODS(IntegerArray)
    DECLARE_OPERATOR_METHODS(RealArray)
    DECLARE_OPERATOR_METHODS(StringArray)

#undef DECLARE_OPERATOR_METHODS

    //! \brief Is the result of this Operation on this Function known?
    //! \param exprs Const reference to Function containing subexpressions.
    //! \return true if known, false if unknown.
    virtual bool isKnown(Function const &exprs) const = 0;

    //! \brief Print the result of this Operation on this Function to an output stream.
    //! \param s The stream.
    //! \param exprs Const reference to Function containing subexpressions.
    virtual void printValue(std::ostream &s, Function const &exprs) const = 0;

    //! \brief Return the result of this Operation on this Function as a Value instance.
    //! \param exprs Const reference to Function containing subexpressions.
    //! \return The Value.
    virtual Value toValue(Function const &exprs) const = 0;

    //! \brief Are all of the types in the vector the same as the requested type,
    //!        or UNKNOWN_TYPE?
    //! \return true if all are same type or the unknown type, false if not.
    //! \note Helper for checkArgTypes() methods
    static bool allSameTypeOrUnknown(ValueType typ, std::vector<ValueType> const &typeVec);

  protected:

    //! \brief Protected constructor.  Only accessible to derived classes.
    //! \param name The name of this Operator.
    Operator(std::string const &name);

    std::string const m_name; //!< The Operator's name.

  private:

    // Explicitly unimplemented
    Operator() = delete;
    Operator(Operator const &) = delete;
    Operator(Operator &&) = delete;
    Operator& operator=(Operator const &) = delete;
    Operator& operator=(Operator &&) = delete;
  };

} // namespace PLEXIL

#endif // PLEXIL_OPERATOR_HH
