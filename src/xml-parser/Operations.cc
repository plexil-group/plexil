/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "Operations.hh"

#include <limits>

namespace PLEXIL
{

  //! @class OperationBase
  //! Provides the minimum basis for implementations of the Operation API.

  class OperationBase : public Operation
  {
  public:
    virtual ~OperationBase() = default;

    // Get the name of this operation.
    virtual std::string const &getName() const
    {
      return m_name;
    }
    
    // Return true if the given argument count is legal for the operation.
    virtual bool checkArgCount(size_t count) const
    {
      return count >= m_minArgs && count <= m_maxArgs;
    }

  protected:

    // Only available to derived classes
    OperationBase(std::string const &name,
                  size_t minArgs = 0,
                  size_t maxArgs = std::numeric_limits<size_t>::max())
      : Operation(),
        m_name(name),
        m_minArgs(minArgs),
        m_maxArgs(maxArgs)
    {
    }

  private:
    // Not implemented
    OperationBase() = delete;
    OperationBase(OperationBase const &) = delete;
    OperationBase(OperationBase &&) = delete;

    std::string const m_name;
    size_t const m_minArgs;
    size_t const m_maxArgs;
  };

  // Helper function for SimpleOperation
  static bool allSameTypeOrUnknown(ValueType typ, std::vector<ValueType> const &typeVec)
  {
    for (ValueType const &actual : typeVec) {
      if (actual != typ && actual != UNKNOWN_TYPE)
        return false;
    }
    return true;
  }

  //! @class SimpleOperation
  //! Represents operations with the same parameter and return type, and one operator.
  //! E.g. Boolean OR, string concatenation.

  class SimpleOperation : public OperationBase
  {
  public:
    SimpleOperation(std::string const &name,
                    Operator const *oper,
                    ValueType argType,
                    ValueType returnType,
                    size_t minArgs = 0,
                    size_t maxArgs = std::numeric_limits<size_t>::max())
      : OperationBase(name, minArgs, maxArgs),
        m_operator(oper),
        m_argType(argType),
        m_returnType(returnType)
    {
    }

    virtual ~SimpleOperation() = default;

    virtual bool checkArgTypes(std::vector<ValueType> const &typeVec) const
    {
      return allSameTypeOrUnknown(m_argType, typeVec);
    }

    virtual ValueType getValueType(std::vector<ValueType> const & /* typeVec */,
                                   ValueType /* desiredType */) const
    {
      return m_returnType;
    }

    virtual Operator const *getOperator(std::vector<ValueType> const & /* typeVec */,
                                        ValueType /* desiredType */) const
    {
      return m_operator;
    }

  private:

    // Not implemented
    SimpleOperation() = delete;
    SimpleOperation(SimpleOperation const &) = delete;
    SimpleOperation(SimpleOperation &&) = delete;

    Operator const *m_operator;
    ValueType const m_argType;
    ValueType const m_returnType;
  };

  std::unique_ptr<Operation>
  makeSimpleOperation(std::string const &name,
                      Operator const *oper,
                      ValueType argType,
                      ValueType returnType,
                      size_t minArgs,
                      size_t maxArgs)
  {
    return std::make_unique<SimpleOperation>(name, oper, argType, returnType, minArgs, maxArgs);
  }

  // Represents an operation which accepts arguments of any type,
  // and returns one specific type.
  // E.g. isKnown(), print-to-string functions.

  class AnyArgOperation : public OperationBase
  {
  public:
    AnyArgOperation(std::string const &name,
                    Operator const *oper,
                    ValueType returnType,
                    size_t minArgs = 0,
                    size_t maxArgs = std::numeric_limits<size_t>::max())
      : OperationBase(name, minArgs, maxArgs),
        m_operator(oper),
        m_returnType(returnType)
    {
    }

    virtual ~AnyArgOperation() = default;

    virtual bool checkArgTypes(std::vector<ValueType> const & /* typeVec */) const
    {
      return true;
    }

    virtual ValueType getValueType(std::vector<ValueType> const & /* typeVec */,
                                   ValueType desiredType) const
    {
      return m_returnType;
    }

    virtual Operator const *getOperator(std::vector<ValueType> const & /* typeVec */,
                                        ValueType desiredType) const
    {
      return m_operator;
    }

  private:

    // Not implemented
    AnyArgOperation() = delete;
    AnyArgOperation(AnyArgOperation const &) = delete;
    AnyArgOperation(AnyArgOperation &&) = delete;

    Operator const *m_operator;
    ValueType const m_returnType;
  };
  
  std::unique_ptr<Operation>
  makeAnyArgOperation(std::string const &name,
                      Operator const *oper,
                      ValueType returnType,
                      size_t minArgs,
                      size_t maxArgs)
  {
    return std::make_unique<AnyArgOperation>(name, oper, returnType,
                                             minArgs, maxArgs);
  }

  //
  // Arithmetic operations
  //

  // Helper function

  static ValueType arithmeticCommonType(std::vector<ValueType> const &types,
                                        ValueType desiredType)
  {
    assertTrue_1(types.size() > 0); // must have at least one operand

    ValueType result = UNKNOWN_TYPE;
    switch (types[0]) {
    case REAL_TYPE:
    case DATE_TYPE:
    case DURATION_TYPE:
      result = REAL_TYPE;
      break;

    case INTEGER_TYPE:
      result = INTEGER_TYPE;
      break;

    case UNKNOWN_TYPE: // e.g. undeclared/indeterminate
      if (desiredType == INTEGER_TYPE)
        result = INTEGER_TYPE;
      else
        result = REAL_TYPE;
      break;

    default: // anything else is not a valid type in an arithmetic expression
      return UNKNOWN_TYPE;
    }

    for (size_t i = 1; i < types.size(); ++i) {
      switch (types[i]) {
      case REAL_TYPE:
      case DATE_TYPE:
      case DURATION_TYPE:
        result = REAL_TYPE;
        break;

      case UNKNOWN_TYPE:
      case INTEGER_TYPE:
        if (result != REAL_TYPE)
          result = INTEGER_TYPE;
        break;

      default:
        return UNKNOWN_TYPE; // bail out early
      }
    }
    // No type info? Choose a "safe" default.
    if (result == UNKNOWN_TYPE)
      result = REAL_TYPE;
    return result;
  }

  //! @class ArithmeticOperation
  //! Represents common behaviors of arithmetic operations and the like.
  //! E.g. add, subtract.

  class ArithmeticOperation : public OperationBase
  {
  public:
    ArithmeticOperation(std::string const &name,
                        Operator const *integerOper,
                        Operator const *realOper,
                        size_t minArgs = 1,
                        size_t maxArgs = std::numeric_limits<size_t>::max())
      : OperationBase(name, minArgs, maxArgs),
        m_integerOperator(integerOper),
        m_realOperator(realOper)
    {
    }

    virtual ~ArithmeticOperation() = default;

    virtual bool checkArgTypes(std::vector<ValueType> const &typeVec) const
    {
      for (ValueType argtype : typeVec)
        if (!isNumericType(argtype) && argtype != UNKNOWN_TYPE)
          return false;
      return true;
    }

    virtual ValueType getValueType(std::vector<ValueType> const &typeVec,
                                   ValueType desiredType) const
    {
      return arithmeticCommonType(typeVec, desiredType);
    }

    virtual Operator const *getOperator(std::vector<ValueType> const &typeVec,
                                        ValueType desiredType) const
    {
      switch(arithmeticCommonType(typeVec, desiredType)) {
      case INTEGER_TYPE:
        return m_integerOperator;

      case REAL_TYPE:
        return m_realOperator;

      default:
        return nullptr;
      }
    }

  private:

    // Not implemented
    ArithmeticOperation() = delete;
    ArithmeticOperation(ArithmeticOperation const &) = delete;
    ArithmeticOperation(ArithmeticOperation &&) = delete;

    Operator const *m_integerOperator;
    Operator const *m_realOperator;
  };

  std::unique_ptr<Operation>
  makeArithmeticOperation(std::string const &name,
                          Operator const *integerOper,
                          Operator const *realOper,
                          size_t minArgs,
                          size_t maxArgs)
  {
    return std::make_unique<ArithmeticOperation>(name, integerOper, realOper,
                                                 minArgs, maxArgs);
  }


  // Special case of arithmetic operation -
  // takes exactly one arg, and only one return type implemented
  // E.g. square root (Real), RealToInteger (Integer)
  class SpecialArithmeticOperation : public OperationBase
  {
  public:
    SpecialArithmeticOperation(std::string const &name,
                               Operator const *oper,
                               ValueType returnType)
      : OperationBase(name, 1, 1),
        m_operator(oper),
        m_returnType(returnType)
    {
    }

    virtual ~SpecialArithmeticOperation() = default;

    virtual bool checkArgTypes(std::vector<ValueType> const &typeVec) const
    {
      return isNumericType(typeVec.at(0)) || typeVec.at(0) == UNKNOWN_TYPE;
    }

    virtual ValueType getValueType(std::vector<ValueType> const &typeVec,
                                   ValueType /* desiredType */) const
    {
      return m_returnType;
    }

    virtual Operator const *getOperator(std::vector<ValueType> const &typeVec,
                                        ValueType /* desiredType */) const
    {
      return m_operator;
    }

  private:
    SpecialArithmeticOperation() = delete;
    SpecialArithmeticOperation(SpecialArithmeticOperation const &) = delete;
    SpecialArithmeticOperation(SpecialArithmeticOperation &&) = delete;

    Operator const *m_operator;
    ValueType const m_returnType;
  };

  std::unique_ptr<Operation>
  makeSpecialArithmeticOperation(std::string const &name,
                                 Operator const *oper,
                                 ValueType returnType)
  {
    return std::make_unique<SpecialArithmeticOperation>(name, oper, returnType);
  }

  // Special behavior for conversion operators
  // These are operators which can return either an Integer or a Real,
  // and can switch result type if the parent expression requests it.

  static ValueType conversionReturnType(ValueType argType, ValueType desiredType)
  {
    if (desiredType == INTEGER_TYPE)
      return INTEGER_TYPE;
    else if (desiredType == REAL_TYPE)
      return REAL_TYPE;
    else if (argType == UNKNOWN_TYPE)
      return REAL_TYPE; // default is safe
    else
      return argType;
  }

  class ArithmeticConversionOperation : public OperationBase
  {
  public:
    ArithmeticConversionOperation(std::string const &name,
                                  Operator const *integerOper,
                                  Operator const *realOper)
      : OperationBase(name, 1, 1),
        m_integerOperator(integerOper),
        m_realOperator(realOper)
    {
    }

    virtual ~ArithmeticConversionOperation() = default;

    virtual bool checkArgTypes(std::vector<ValueType> const &typeVec) const
    {
      return isNumericType(typeVec.at(0)) || typeVec.at(0) == UNKNOWN_TYPE;
    }

    virtual ValueType getValueType(std::vector<ValueType> const &typeVec,
                                   ValueType desiredType) const
    {
      return conversionReturnType(typeVec.at(0), desiredType);
    }

    virtual Operator const *getOperator(std::vector<ValueType> const &typeVec,
                                        ValueType desiredType) const
    {
      switch (conversionReturnType(typeVec.at(0), desiredType)) {
      case INTEGER_TYPE:
        return m_integerOperator;
      case REAL_TYPE:
        return m_realOperator;
      default:
        return nullptr;
      }
    }

  private:
    ArithmeticConversionOperation() = delete;
    ArithmeticConversionOperation(ArithmeticConversionOperation const &) = delete;
    ArithmeticConversionOperation(ArithmeticConversionOperation &&) = delete;

    Operator const *m_integerOperator;
    Operator const *m_realOperator;
  };

  std::unique_ptr<Operation>
  makeArithmeticConversionOperation(std::string const &name,
                                    Operator const *integerOper,
                                    Operator const *realOper)
  {
    return std::make_unique<ArithmeticConversionOperation>(name, integerOper, realOper);
  }

  //
  // Helpers for Equal, NotEqual
  //

  static bool canBeEqual(ValueType typeA, ValueType typeB)
  {
    // Identical types can always be compared for equality
    if (typeA == typeB)
      return true;

    // Punt if either type unknown
    if (typeA == UNKNOWN_TYPE || typeB == UNKNOWN_TYPE)
      return true;

    // Arithmetic types
    if (isNumericType(typeA) && isNumericType(typeB))
      return true;

    // Anything else is an error.
    return false;
  }

  // Equality comparisons take exactly two arguments
  // of compatible types, and return a Boolean.

  class EqualityOperation : public OperationBase
  {
  public:
    EqualityOperation(std::string const &name,
                      Operator const *oper)
      : OperationBase(name, 2, 2),
        m_operator(oper)
    {
    }

    virtual ~EqualityOperation() = default;

    virtual bool checkArgTypes(std::vector<ValueType> const &typeVec) const
    {

      return canBeEqual(typeVec.at(0), typeVec.at(1));
    }

    virtual ValueType getValueType(std::vector<ValueType> const &typeVec,
                                   ValueType /* desiredType */) const
    {
      return BOOLEAN_TYPE;
    }

    virtual Operator const *getOperator(std::vector<ValueType> const &typeVec,
                                        ValueType /* desiredType */) const
    {
      return m_operator;
    }

  private:
    EqualityOperation() = delete;
    EqualityOperation(EqualityOperation const &) = delete;
    EqualityOperation(EqualityOperation &&) = delete;
    EqualityOperation &operator=(EqualityOperation const &) = delete;
    EqualityOperation &operator=(EqualityOperation &&) = delete;

    Operator const *m_operator;
  };

  std::unique_ptr<Operation>
  makeEqualityOperation(std::string const &name, Operator const *oper)
  {
    return std::make_unique<EqualityOperation>(name, oper);
  }

  //
  // Comparisons
  //
  
  // Helper functions
  static bool canBeCompared(ValueType typeA, ValueType typeB)
  {
    if (typeA == UNKNOWN_TYPE || typeB == UNKNOWN_TYPE)
      return true;

    if (isNumericType(typeA))
      return isNumericType(typeB);

    if (typeA == STRING_TYPE)
      return typeB == STRING_TYPE;

    // No ordering defined for other types in PLEXIL
    return false;
  }

  static ValueType comparisonType(ValueType typeA, ValueType typeB)
  {
    // Coerce Date and Duration to the underlying Real type
    switch (typeA) {
    case DATE_TYPE:
    case DURATION_TYPE:
      typeA = REAL_TYPE;
      break;
    default:
      break;
    }
    switch (typeB) {
    case DATE_TYPE:
    case DURATION_TYPE:
      typeB = REAL_TYPE;
      break;
    default:
      break;
    }
      
    if (typeA == typeB)
      return typeA;

    // handle numeric type coercion
    switch (typeA) {
    case INTEGER_TYPE:
      switch (typeB) {
      case REAL_TYPE:
      case UNKNOWN_TYPE:
        return REAL_TYPE; // safe
      default:
        return UNKNOWN_TYPE; // illegal
      }
      
    case REAL_TYPE:
      if (typeB == INTEGER_TYPE || typeB == UNKNOWN_TYPE)
        return REAL_TYPE;
      else
        return UNKNOWN_TYPE; // illegal

    default:
      return UNKNOWN_TYPE;
    }
  }

  class ComparisonOperation : public OperationBase
  {
  public:
    ComparisonOperation(std::string const &name,
                        Operator const *integerOper,
                        Operator const *realOper,
                        Operator const *stringOper)
      : OperationBase(name, 2, 2),
        m_integerOperator(integerOper),
        m_realOperator(realOper),
        m_stringOperator(stringOper)
    {
    }

    virtual ~ComparisonOperation() = default;

    virtual bool checkArgTypes(std::vector<ValueType> const &typeVec) const
    {
      return canBeCompared(typeVec.at(0), typeVec.at(1));
    }

    virtual ValueType getValueType(std::vector<ValueType> const &typeVec,
                                   ValueType /* desiredType */) const
    {
      return BOOLEAN_TYPE;
    }

    virtual Operator const *getOperator(std::vector<ValueType> const &typeVec,
                                        ValueType /* desiredType */) const
    {
      switch (comparisonType(typeVec.at(0), typeVec.at(1))) {
      case INTEGER_TYPE:
        return m_integerOperator;
      case REAL_TYPE:
        return m_realOperator;
      case STRING_TYPE:
        return m_stringOperator;
      default:
        return nullptr;
      }
    }

  private:
    ComparisonOperation() = delete;
    ComparisonOperation(ComparisonOperation const &) = delete;
    ComparisonOperation(ComparisonOperation &&) = delete;
    ComparisonOperation &operator=(ComparisonOperation const &) = delete;
    ComparisonOperation &operator=(ComparisonOperation &&) = delete;

    Operator const *m_integerOperator;
    Operator const *m_realOperator;
    Operator const *m_stringOperator;
  };

  std::unique_ptr<Operation>
  makeComparisonOperation(std::string const &name,
                          Operator const *integerOper,
                          Operator const *realOper,
                          Operator const *stringOper)
  {
    return std::make_unique<ComparisonOperation>(name, integerOper, realOper, stringOper);
  }

} // namespace PLEXIL
