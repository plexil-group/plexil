/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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


#include "CoreExpressions.hh"
#include "Node.hh"
#include "ExternalInterface.hh"
#include "LabelStr.hh"
#include "Debug.hh"
#include "CommandHandle.hh"
#include <cmath> // for fabs()
#include <list>
#include <string>
#include <sstream>
#include <stdint.h> // for int32_t

namespace PLEXIL 
{

  ArrayVariable::ArrayVariable(unsigned long maxSize,
							   PlexilType type,
                               const bool isConst)
    : EssentialArrayVariable(), 
	  VariableImpl(isConst), 
	  m_maxSize(maxSize), 
      m_type(type),
      m_initialVector()
  {
    debugMsg("ArrayVariable", " constructor, no initial elements");
    StoredArray array(m_maxSize, UNKNOWN());
    setValue(array.getKey());
  }

  ArrayVariable::ArrayVariable(unsigned long maxSize, 
							   PlexilType type, 
                               std::vector<double>& values, 
							   const bool isConst)
    : EssentialArrayVariable(),
	  VariableImpl(isConst),
	  m_maxSize(maxSize),
      m_initialVector(values)
  {
    debugMsg("ArrayVariable", " constructor, " << values.size() << " initial elements");
    StoredArray array(m_maxSize, UNKNOWN());
    setValue(array.getKey());
    setValues(values);
  }

  ArrayVariable::ArrayVariable(const PlexilExprId& expr, 
                               const NodeConnectorId& node,
                               const bool isConst)
    : EssentialArrayVariable(node),
	  VariableImpl(expr, node, isConst)
  {
    debugMsg("ArrayVariable", " constructor from intermediate representation");

    // confirm that we have a an array
    checkError(Id<PlexilArrayValue>::convertable(expr),
               "Expected an array value.");
    PlexilArrayValue* arrayValue = (PlexilArrayValue*)expr;

    // init the local type and array
    m_type = arrayValue->type();
    m_maxSize = arrayValue->maxSize();
    StoredArray array(m_maxSize, UNKNOWN());
    setValue(array.getKey());

    // convert strings to doubles for internal storage
    const std::vector<std::string>& values = arrayValue->values();
    for (std::vector<std::string>::const_iterator value = values.begin();
         value != values.end(); ++value) {
      double convertedValue;
      if (m_type == STRING)
	convertedValue = (double)LabelStr(*value);
      else if (m_type == BOOLEAN) {
	if (compareIgnoreCase(*value, "true") || 
	    (strcmp(value->c_str(), "1") == 0))
	  convertedValue = 1;
	else if (compareIgnoreCase(*value, "false") || 
		 (strcmp(value->c_str(), "0") == 0))
	  convertedValue = 0;
	else
	  checkError(false, "Invalid boolean value \"" << *value << "\"");
      }
      else {
	std::istringstream valueStream(*value);
	valueStream >> convertedValue;
      }
      m_initialVector.push_back(convertedValue);
    }

    // Store the converted values
    setValues(m_initialVector);
  }

  // 
  // Destructor
  // Frees the array
  //

  ArrayVariable::~ArrayVariable()
  {
    StoredArray theArray(m_value);
    m_value = Expression::UNKNOWN();
    theArray.unregister();
  }


  /**
   * @brief Set the value of this expression back to the initial value with which it was
   *        created.
   */
  void ArrayVariable::reset()
  {
    // Check that array storage is allocated
    if (m_value == Expression::UNKNOWN()) {
      // Reallocate to original size
      // N.B. can only get here if array var was assigned UNKNOWN somehow
      StoredArray newArray(m_maxSize, Expression::UNKNOWN());
      m_value = newArray.getKey();
    }

    if (m_initialVector.empty()) {
      // Clear the array
      StoredArray myArray(m_value);
      for (size_t i = 0; i < myArray.size(); i++)
	myArray[i] = Expression::UNKNOWN();
      publishChange();
    }
    else {
      setValues(m_initialVector);
    }
  }

  // set the value of this array

  void ArrayVariable::setValue(const double value)
  {
    // if the value is unknown, then set the value of the
    // array as a whole to unknown, and free the old storage.

    if (value == UNKNOWN())
      {
        double oldValue = m_value;
        VariableImpl::setValue(value);

        // dispose of old array if it's not the saved initial value
        // (needed for Variable::reset())
        if (oldValue != m_initialValue)
          {
            StoredArray oldArray(m_value);
            oldArray.unregister();
          }
      }

    // if the value of the array is
    // currently unknown, then create a new array of m_maxSize,
    // copy source array to it, and set value to the new array

    else if (m_value == UNKNOWN())
      {
        // create a copy of the array
        StoredArray newArray(m_maxSize, UNKNOWN());
        StoredArray sourceArray(value);
        checkError(newArray.size() >= sourceArray.size(),
                   "Source array size " << sourceArray.size() <<
                   ", exceeds target size " << newArray.size() << ".");
        for (unsigned long i = 0; i < sourceArray.size(); i++)
          {
            newArray[i] = sourceArray[i];
          }

        VariableImpl::setValue(newArray.getKey());
      }

    // just copy from the other array

    else
      setValues(value);
  }

  // set all values for this array from an array

  void ArrayVariable::setValues(std::vector<double>& values)
  {
    unsigned index = 0;
    StoredArray array(m_value);

    checkError(m_maxSize >= values.size(), 
               "Attempted to initialize array variable beyond its maximum size");

    // set all the values
      
    for (std::vector<double>::iterator value = values.begin();
         value != values.end(); ++value)
      {
	debugMsg("ArrayVariable::setValues", 
		 " checking element value " << plexilValueToString(*value));
        checkError(checkElementValue(*value),
                   "Attempted to initialize a variable with an invalid value.");
        array[index++] = *value;
      }

    // fill out the rest of the array with "UNKNOWN"

    while (index < m_maxSize)
      array[index++] = UNKNOWN();

    // publish change

    publishChange();
  }

  // set all values for this array

  void ArrayVariable::setValues(const double key)
  {
    unsigned index = 0;
    StoredArray array(m_value);
    StoredArray source(key); // error if not an array (I hope!)

    checkError(source.size() <= array.size(),
               "Source array size " << source.size() <<
               ", exceeds target size " << array.size() << ".");
    debugMsg("ArrayVariable:setValues", 
             '(' << source.toString() << ')');
      
    // set all the values
      
    while (index < source.size())
      {
        double value = source[index];
        checkError(checkElementValue(value),
                   "Attempted to initialize a variable with an invalid value.");
        array[index++] = value;
      }

    // fill out the rest of the array with "UNKNOWN"

    while (index < array.size())
      array[index++] = UNKNOWN();

    debugMsg("ArrayVariable:setValues", 
             " result is " << array.toString());

    // publish change

    publishChange();
  }

  // Propagate changes from elements

  void ArrayVariable::handleElementChanged(const ExpressionId & elt)
  {
    debugMsg("ArrayVariable:handleElementChanged", " for " << getId());
    publishElementChange(elt);
  }

  void ArrayVariable::publishElementChange(const ExpressionId & elt)
  {
    if (!isActive())
      return;
    for (std::list<ExpressionListenerId>::iterator it = m_outgoingListeners.begin();
         it != m_outgoingListeners.end(); ++it)
      {
        check_error((*it).isValid());
        if ((*it)->isActive())
          debugMsg("ArrayVariable:publishElementChanged", " notifying " << (*it)->getId());
          (*it)->notifyValueChanged(elt);
      }
  }

  // set an element value in an array variable
  void ArrayVariable::setElementValue(unsigned index, const double value)
  {
    // lotsa potential errors to check
    checkError(!VariableImpl::isConst(),
               "Attempted to set element value " << value << " to " << toString());
    checkError(checkElementValue(value),
               "Attempted to assign an invalid value to an array element");
    checkError(checkIndex(index),
               "Array index " << index << " exceeds bound of " 
               << m_maxSize);
    checkError(m_value != UNKNOWN(),
               "Attempted to assign an array element in an UNKNOWN array");

    // set the element
    StoredArray theArray(m_value);
    theArray[index] = value;

    // publish change
    publishChange();
  }

  // lookup a value in an array variable

  double ArrayVariable::lookupValue(unsigned long index) const
  {
    checkError(checkIndex(index),
               "Array index " << index << " exceeds bound of " 
               << m_maxSize);
    return m_value == UNKNOWN()
      ? UNKNOWN()
      : StoredArray(m_value)[index];
  }

  std::string ArrayVariable::toString() const 
  {
    std::ostringstream retval;
    retval << Expression::toString();
    if (m_value == UNKNOWN())
      {
        retval << "Array: <uninited, max size = " << m_maxSize << ">";
      }
    else
      {
        StoredArray array(m_value);
        retval << "Array: [";
        for (unsigned i = 0; i < array.size(); ++i)
          {
            const double& value = array.at(i);
            if (i != 0)
              retval << ", ";
            if (value == UNKNOWN())
              retval << "<unknown>";
            else
              {
                switch (m_type)
                  {
                  case INTEGER:
                    retval << (int32_t) value;
                    break;
                  case REAL:
                    retval << value;
                    break;
                  case BOOLEAN:
                    retval << (value ? "true" : "false");
                    break;
                  case STRING:
                    retval << "\"" << LabelStr(value).toString() << "\"";
                    break;
                  case BLOB:
                    checkError(ALWAYS_FAIL, "Blobs not supported in arrays.");
                    break;
                  case ARRAY:
                    checkError(ALWAYS_FAIL, "Arrarys of arrays not supported.");
                    break;
                  case TIME:
                    checkError(ALWAYS_FAIL, "TimePoints not supported in arrays.");
                    break;
                  default:
                    retval << value << "(Unknown type: " << m_type << ")";
                  }
              }
          }
        retval << "])";
      }
    return retval.str();
  }

  // confirm that new value to assign is valid

  bool ArrayVariable::checkValue(const double val)
  {
    if (val == UNKNOWN())
      return true;
    if (StoredArray::isKey(val))
      {
        StoredArray valArray(val);
        if (valArray.size() <= m_maxSize)
          return true;
      }
    return false;
  }

  // confirm that array element is valid

  bool ArrayVariable::checkElementValue(const double val)
  {
    // check value based on array type

    switch (m_type)
      {
      case INTEGER:
        return val == UNKNOWN() ||
          ((val >= MINUS_INFINITY && val <= PLUS_INFINITY) &&
           val == (double) (int32_t) val);
      case REAL:
        return (val >= REAL_MINUS_INFINITY && val <= REAL_PLUS_INFINITY) ||
          val == UNKNOWN();
      case BOOLEAN:
        return val == UNKNOWN() || val == 0.0 || val == 1.0;
      case STRING:
        return LabelStr::isString(val);
      case BLOB:
        checkError(ALWAYS_FAIL, "Blobs not supported in arrays.");
      case ARRAY:
        checkError(ALWAYS_FAIL, "Arrarys of arrays not supported.");
      case TIME:
        checkError(ALWAYS_FAIL, "TimePoints not supported in arrays.");
      default:
        checkError(ALWAYS_FAIL, "Unknown variable type: " << m_type);
      }
    // should never get here

    return false;
  }
   

  // construct an array element expression from a PlexilExpr

  ArrayElement::ArrayElement(const PlexilExprId& expr, 
                             const NodeConnectorId& node)
    : DerivedVariable(node),
      m_deleteIndex(false),
      m_listener(getId())
  {
    // confirm that we have an array element
    checkError(Id<PlexilArrayElement>::convertable(expr),
               "Expected an array element.");
    PlexilArrayElement* arrayElement = (PlexilArrayElement*) expr;
    debugMsg("ArrayElement:ArrayElement", " name = " << arrayElement->getArrayName());

    // initialize array variable
    const std::string & name = arrayElement->getArrayName();
    PlexilVarRef arrayRef;
    arrayRef.setName(name);
    VariableId arrayVar = node->findVariable(&arrayRef);
    checkError(EssentialArrayVariableId::convertable(arrayVar),
               "Expected Array Variable but found: " << 
               arrayVar->toString());
    m_arrayVariable = (EssentialArrayVariableId) arrayVar;    
    m_arrayVariable->addListener(m_listener.getId());

    // initialize index expression
    const std::vector<PlexilExprId>& subExprs = expr->subExprs();
    // *** update this if we ever support n-dimensional arrays ***
    checkError(subExprs.size() == 1, 
               subExprs.size()
               << " is an invalid number of index subexpressions to array element");
    PlexilExprId indexExpr = subExprs.front();
    m_index = ExpressionFactory::createInstance(indexExpr->name(),
                                                indexExpr,
                                                node,
                                                m_deleteIndex);
    m_index->addListener(m_listener.getId());

  }

  ArrayElement::~ArrayElement()
  {
    check_error(m_arrayVariable.isValid());
    m_arrayVariable->removeListener(m_listener.getId());
    if (m_deleteIndex)
      {
        check_error(m_index.isValid());
        m_index->removeListener(m_listener.getId());
      }
  }

  // confirm that new value to assign is valid

  bool ArrayElement::checkValue(const double val)
  {
    return m_arrayVariable->checkElementValue(val);
  }

  std::string ArrayElement::toString() const
  {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "ArrayElement: " << m_arrayVariable->toString()
           << "[" << m_index->toString() << "])";
    return retval.str();
  }

  // *** FIXME: is this the right approach?
  void ArrayElement::reset()
  {}

  void ArrayElement::setValue(const double value)
  {
    // delegate to the array
    m_arrayVariable->setElementValue((unsigned) m_index->getValue(),
                                     value);
    internalSetValue(value);
  }

  PlexilType ArrayElement::getValueType() const
  {
    return m_arrayVariable->getElementType();
  }

  /**
   * @brief Notify listeners that the value of this expression has changed.
   */

  void ArrayElement::publishChange()
  {
    Expression::publishChange();
    m_arrayVariable->handleElementChanged(getId());
  }

  /**
   * @brief Notify this expression that a subexpression's value has changed.
   * @param exp The changed subexpression.
   */

  void ArrayElement::handleChange(const ExpressionId& /* ignored */)
  {
    internalSetValue(recalculate());
  }

  //this could be optimized slightly more to check for dirtiness on subexpressions
  //but that would require setting dirtiness when deactivated, not just when locked

  void ArrayElement::handleActivate(const bool changed) {
    if (!changed)
      return;
    m_listener.activate();
    check_error(m_index.isValid());
    m_index->activate();
    m_arrayVariable->activate();
    internalSetValue(recalculate());
  }

  void ArrayElement::handleDeactivate(const bool changed) {
    if (!changed)
      return;
    m_listener.deactivate();
    check_error(m_index.isValid());
    m_arrayVariable->deactivate();
    m_index->deactivate();
  }

  double ArrayElement::recalculate()
  {
    double index = m_index->getValue();
    if (index == UNKNOWN())
      return UNKNOWN();
    return m_arrayVariable->lookupValue((unsigned long) index);
  }

  /**
   * @brief Constructor. Creates a variable that indirects to another variable.
   * @param name The name of this variable in the node that constructed the alias.
   * @param nodeConnector The node connector of the node which owns this alias.
   * @param original The original variable for this alias.
   * @param isConst True if assignments to the alias are forbidden.
   */
  AliasVariable::AliasVariable(const std::string& name, 
							   const NodeConnectorId& nodeConnector,
							   VariableId original,
							   const bool isConst)
	: DerivedVariable(nodeConnector),
	  m_originalVariable(original),
	  m_listener(getId()),
	  m_name(name),
	  m_isConst(isConst)
  {
	// Check original, node for validity
	assertTrue(original.isValid(),
			   "Invalid variable ID passed to AliasVariable constructor");
	assertTrue(nodeConnector.isValid(),
			   "Invalid node connector ID passed to AliasVariable constructor");
	m_originalVariable->addListener(m_listener.getId());
	m_value = m_originalVariable->getValue();
  }

  AliasVariable::~AliasVariable()
  {
	assertTrue(m_originalVariable.isValid(),
			   "Original variable ID invalid in AliasVariable destructor");
	m_originalVariable->removeListener(m_listener.getId());
  }

  /**
   * @brief Get a string representation of this Expression.
   * @return The string representation.
   */
  std::string AliasVariable::toString() const
  {
	std::ostringstream str;
	str << Expression::toString()
		<< "AliasVariable " << m_name
		<< ", aliased to " << m_originalVariable->toString()
		<< ")";
	return str.str();
  }

  /**
   * @brief Set the value of this expression back to the initial value with which it was
   *        created.
   */
  void AliasVariable::reset()
  { 
	// *** FIXME: should this do anything at all??
	// m_originalVariable->reset(); 
  }

  /**
   * @brief Retrieve the value type of this Expression.
   * @return The value type of this Expression.
   * @note Delegates to original.
   */
  PlexilType AliasVariable::getValueType() const
  {
	return m_originalVariable->getValueType();
  }

  bool AliasVariable::checkValue(const double val)
  {
	return m_originalVariable->checkValue(val);
  }	

  /**
   * @brief Sets the value of this variable.  Will throw an error if the variable was
   *        constructed with isConst == true.
   * @param value The new value for this variable.
   */
  void AliasVariable::setValue(const double value)
  {
    checkError(!isConst(),
			   "Attempted to assign value " << Expression::valueToString(value)
			   << " to read-only alias variable " << toString());
	m_originalVariable->setValue(value);
  }

  void AliasVariable::handleChange(const ExpressionId& exp)
  {
	if (exp == m_originalVariable) {
	  // propagate value from original
	  internalSetValue(m_originalVariable->getValue());
	}
  }

  void AliasVariable::handleActivate(const bool changed)
  {
	if (changed) {
	  m_originalVariable->activate();
	  // refresh value from original
	  internalSetValue(m_originalVariable->getValue());
	}
  }

  // *** FIXME: Inhibit deactivatation if const?
  void AliasVariable::handleDeactivate(const bool changed)
  {
	if (changed) {
	  m_originalVariable->deactivate();
	}
  }

  void AliasVariable::handleReset()
  {
  }

  StringVariable::StringVariable(const bool isConst) : VariableImpl(isConst) {}

  StringVariable::StringVariable(const std::string& value, const bool isConst)
    : VariableImpl(LabelStr(value), isConst) {}

  StringVariable::StringVariable(const char* value, const bool isConst)
    : VariableImpl(LabelStr(value), isConst) {}

  StringVariable::StringVariable(const double value, const bool isConst)
    : VariableImpl(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  StringVariable::StringVariable(const LabelStr& value, const bool isConst)
    : VariableImpl(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  StringVariable::StringVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				 const bool isConst)
    : VariableImpl(expr, node, isConst) {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::STRING,
	       "Expected a String value.  Got " << PlexilParser::valueTypeString(val->type()));

    m_initialValue = m_value = (double)LabelStr(val->value());
  }


  std::string StringVariable::toString() const {
    std::ostringstream retval;
    retval << VariableImpl::toString();
    retval << "string)";
    return retval.str();
  }

  bool StringVariable::checkValue(const double val) {
    return LabelStr::isString(val);
  }

  RealVariable::RealVariable(const bool isConst) : VariableImpl(isConst) {}

  RealVariable::RealVariable(const double value, const bool isConst)
    : VariableImpl(value, isConst) 
  {
    checkError(checkValue(value),
               "Attempted to initialize a variable with an invalid value.");
  }

  RealVariable::RealVariable(const PlexilExprId& expr, const NodeConnectorId& node,
                             const bool isConst)
	: VariableImpl(expr, node, isConst) 
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    commonNumericInit((PlexilValue*)expr);
  }

  std::string RealVariable::toString() const 
  {
    std::ostringstream retval;
    retval << VariableImpl::toString();
    retval << "real)";
    return retval.str();
  }
   
  bool RealVariable::checkValue(const double val) {
    return (val >= REAL_MINUS_INFINITY && val <= REAL_PLUS_INFINITY) ||
      val == UNKNOWN();
  }

  ExpressionId& RealVariable::ZERO_EXP() {
    static ExpressionId sl_zero_exp;
    if (sl_zero_exp.isNoId()) {
	  VariableImpl* var = new RealVariable(0.0, true);
	  var->setName("Real constant 0");
      sl_zero_exp = var->getId();
	}
    if(!sl_zero_exp->isActive())
      sl_zero_exp->activate();
    return sl_zero_exp;
  }

  ExpressionId& RealVariable::ONE_EXP() {
    static ExpressionId sl_one_exp;
    if (sl_one_exp.isNoId()) {
	  VariableImpl* var = new RealVariable(1.0, true);
	  var->setName("Real constant 1");
      sl_one_exp = var->getId();
	}
    if(!sl_one_exp->isActive())
      sl_one_exp->activate();
    return sl_one_exp;
  }

  ExpressionId& RealVariable::MINUS_ONE_EXP() {
    static ExpressionId sl_minus_one_exp;
    if (sl_minus_one_exp.isNoId()) {
	  VariableImpl* var = new RealVariable(-1.0, true);
	  var->setName("Real constant -1");
      sl_minus_one_exp = var->getId();
	}
    if(!sl_minus_one_exp->isActive())
      sl_minus_one_exp->activate();
    return sl_minus_one_exp;
  }

  IntegerVariable::IntegerVariable(const bool isConst) : VariableImpl(isConst) {}

  IntegerVariable::IntegerVariable(const double value, const bool isConst)
    : VariableImpl(value, isConst) 
  {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");

  }

  IntegerVariable::IntegerVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool isConst)
	: VariableImpl(expr, node, isConst) 
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    commonNumericInit((PlexilValue*)expr);
  }

  std::string IntegerVariable::toString() const {
    std::ostringstream retval;
    retval << VariableImpl::toString();
    retval << "int)";
    return retval.str();
  }

  bool IntegerVariable::checkValue(const double val) {
    if (val == UNKNOWN())
      return true;
    if (val < MINUS_INFINITY || val > PLUS_INFINITY)
      return false;
    if (fabs(val - ((double) (int32_t) val)) < EPSILON)
      return true;
    return false;
  }

  ExpressionId& IntegerVariable::ZERO_EXP()
  {
    static ExpressionId sl_zero_exp;
    if (sl_zero_exp.isNoId()) {
	  VariableImpl* var = new IntegerVariable(0.0, true);
	  var->setName("Integer constant 0");
      sl_zero_exp = var->getId();
	}
    if(!sl_zero_exp->isActive())
      sl_zero_exp->activate();
    return sl_zero_exp;
  }

  ExpressionId& IntegerVariable::ONE_EXP()
  {
    static ExpressionId sl_one_exp;
    if (sl_one_exp.isNoId()) {
	  VariableImpl* var = new IntegerVariable(1.0, true);
	  var->setName("Integer constant 1");
      sl_one_exp = var->getId();
	}
    if(!sl_one_exp->isActive())
      sl_one_exp->activate();
    return sl_one_exp;
  }

  ExpressionId& IntegerVariable::MINUS_ONE_EXP()
  {
    static ExpressionId sl_minus_one_exp;
    if (sl_minus_one_exp.isNoId()) {
	  VariableImpl* var = new IntegerVariable(-1.0, true);
	  var->setName("Integer constant -1");
      sl_minus_one_exp = var->getId();
	}
    if(!sl_minus_one_exp->isActive())
      sl_minus_one_exp->activate();
    return sl_minus_one_exp;
  }

  BooleanVariable::BooleanVariable(const bool isConst) : VariableImpl(isConst){}
  BooleanVariable::BooleanVariable(const double value, const bool isConst)
    : VariableImpl(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  BooleanVariable::BooleanVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool /* isConst */)
    : VariableImpl(expr, node) {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    commonNumericInit((PlexilValue*)expr);
  }

  std::string BooleanVariable::toString() const {
    std::ostringstream retval;
    retval << VariableImpl::toString();
    retval << "boolean)";
    return retval.str();
  }

  bool BooleanVariable::checkValue(const double val) {
    return val == UNKNOWN() || val == FALSE() || val == TRUE();
  }

  ExpressionId& BooleanVariable::TRUE_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId()) {
	  VariableImpl* var = new BooleanVariable(TRUE(), true);
	  var->setName("Boolean constant true");
      sl_exp = var->getId();
	}
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  ExpressionId& BooleanVariable::FALSE_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId()) {
	  VariableImpl* var = new BooleanVariable(FALSE(), true);
	  var->setName("Boolean constant false");
      sl_exp = var->getId();
	}
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  ExpressionId& BooleanVariable::UNKNOWN_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId()) {
	  VariableImpl* var = new BooleanVariable(UNKNOWN(), true);
	  var->setName("Boolean constant unknown");
      sl_exp = var->getId();
	}
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  //
  // StateVariable
  //

  StateVariable::StateVariable(const bool isConst) : VariableImpl(INACTIVE(), isConst) {}

  StateVariable::StateVariable(const double value, const bool isConst)
    : VariableImpl(value, isConst) 
  {
    checkError(checkValue(value),
	       "Attempted to initialize a state variable with invalid value "
	       << Expression::valueToString(value));
  }

  StateVariable::StateVariable(const PlexilExprId& expr, 
							   const NodeConnectorId& node,
							   const bool isConst)
	: VariableImpl(expr, node, isConst) 
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::NODE_STATE,
	       "Expected NodeState value.  Found '" << PlexilParser::valueTypeString(val->type()) << "'");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a state variable with invalid value "
	       << Expression::valueToString(value));
  }

  // N.B. Depends on ALL_STATES() matching order of NodeState enumeration.
  bool StateVariable::checkValue(const double val) {
    for (size_t s = INACTIVE_STATE; s < NO_NODE_STATE; s++) {
      if (val == ALL_STATES()[s])
	return true;
    }
    return false;
  }

  std::string StateVariable::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "state(" << LabelStr(m_value).toString() << "))";
    return retval.str();
  }

  void StateVariable::setNodeState(NodeState newValue)
  {
    checkError(newValue < NO_NODE_STATE,
	       "Attempted to set an invalid NodeState value");
    this->setValue(ALL_STATES()[newValue].getKey());
  }

  const std::vector<LabelStr>& StateVariable::ALL_STATES() {
    static std::vector<LabelStr>* allStates = NULL;
    if (allStates == NULL) {
      allStates = new std::vector<LabelStr>();
      allStates->reserve(NODE_STATE_MAX);
      allStates->push_back(INACTIVE());
      allStates->push_back(WAITING());
      allStates->push_back(EXECUTING());
      allStates->push_back(FINISHING());
      allStates->push_back(FINISHED());
      allStates->push_back(FAILING());
      allStates->push_back(ITERATION_ENDED());
      allStates->push_back(NO_STATE());
    }
    return *allStates;
  }

  ExpressionId& StateVariable::INACTIVE_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(INACTIVE(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::WAITING_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(WAITING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::EXECUTING_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(EXECUTING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::FINISHING_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(FINISHING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::FINISHED_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(FINISHED(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::FAILING_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(FAILING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::ITERATION_ENDED_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(ITERATION_ENDED(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::NO_STATE_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(NO_STATE(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  const LabelStr& StateVariable::nodeStateName(NodeState state)
  {
    return ALL_STATES()[state];
  }

  NodeState StateVariable::nodeStateFromName(double nameAsLabelStrKey)
  {
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++)
      if (ALL_STATES()[s].getKey() == nameAsLabelStrKey)
	return (NodeState) s;
    return NO_NODE_STATE;
  }


  OutcomeVariable::OutcomeVariable(const bool isConst) : VariableImpl(isConst) {}
  OutcomeVariable::OutcomeVariable(const double value, const bool isConst)
    : VariableImpl(isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }
  OutcomeVariable::OutcomeVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool isConst)
	: VariableImpl(expr, node, isConst)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::NODE_OUTCOME,
	       "Expected NodeOutcome value.  Found " << PlexilParser::valueTypeString(val->type()) << ".");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  bool OutcomeVariable::checkValue(const double val) {
    return (val == UNKNOWN() || val == SUCCESS() || val == FAILURE() || val == SKIPPED());
  }

  std::string OutcomeVariable::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << " outcome)";
    return retval.str();
  }


  FailureVariable::FailureVariable(const bool isConst) : VariableImpl(isConst) {}

  FailureVariable::FailureVariable(const double value, const bool isConst)
    : VariableImpl(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  FailureVariable::FailureVariable(const PlexilExprId& expr, 
								   const NodeConnectorId& node,
								   const bool isConst)
	: VariableImpl(expr, node, isConst)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::FAILURE_TYPE,
	       "Expected NodeFailure value.  Found " << PlexilParser::valueTypeString(val->type()) << ".");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  std::string FailureVariable::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << " failure)";
    return retval.str();
  }

  bool FailureVariable::checkValue(const double val) {
    return val == UNKNOWN()
	  || val == PRE_CONDITION_FAILED()
	  || val == POST_CONDITION_FAILED()
	  || val == INVARIANT_CONDITION_FAILED()
	  || val == PARENT_FAILED();
  }

  CommandHandleVariable::CommandHandleVariable(const bool isConst) : VariableImpl(isConst) {}
  CommandHandleVariable::CommandHandleVariable(const double value, const bool isConst)
    : VariableImpl(isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }
  CommandHandleVariable::CommandHandleVariable(const PlexilExprId& expr, 
											   const NodeConnectorId& node,
                                               const bool isConst)
	: VariableImpl(expr, node, isConst)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::COMMAND_HANDLE,
	       "Expected NodeCommandHandle value.  Found " << PlexilParser::valueTypeString(val->type()) << ".");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  bool CommandHandleVariable::checkValue(const double val) {
    return (val == UNKNOWN() || val == COMMAND_SENT_TO_SYSTEM() || val == COMMAND_ACCEPTED() || val == COMMAND_RCVD_BY_SYSTEM() || val == COMMAND_SUCCESS() || val == COMMAND_DENIED() || val == COMMAND_FAILED());
  }

  std::string CommandHandleVariable::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << " command_handle)";
    return retval.str();
  }

  //
  // Unary expressions
  //

  UnaryExpression::UnaryExpression(const PlexilExprId& expr, const NodeConnectorId& node)
    : Calculable(expr, node) {
    checkError(m_subexpressions.size() == 1,
	       "Expected exactly one subexpression in unary " << expr->name() <<
	       ", but have " << m_subexpressions.size());
    m_e = m_subexpressions.front();
  }

  UnaryExpression::UnaryExpression(const ExpressionId& e)
    : Calculable(),
      m_e(e)
  {
    addSubexpression(e, false);
  }

  bool LogicalNegation::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double LogicalNegation::recalculate() {
    double v = m_e->getValue();
    checkError(v == 1.0 || v == 0.0 || v == UNKNOWN(),
	       "Invalid value in logical negation: " << v);
    if (v == 1.0)
      return 0.0;
    else if (v == 0.0)
      return 1.0;
    else if (v == UNKNOWN())
      return UNKNOWN();
    check_error(ALWAYS_FAIL);
    return -1.0;
  }

  std::string LogicalNegation::toString() const
  {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "!" << m_e->toString();
    retval << ")";
    return retval.str();
  }

  //
  // Binary expressions
  //

  BinaryExpression::BinaryExpression(const PlexilExprId& expr, const NodeConnectorId& node)
    : Calculable(expr, node) {
    checkError(m_subexpressions.size() == 2,
	       "Expected exactly two subexpressions in binary " << expr->name() <<
	       ", but have " << m_subexpressions.size());
    m_a = m_subexpressions.front();
    m_b = m_subexpressions.back();
  }

  BinaryExpression::BinaryExpression(const ExpressionId& a, const ExpressionId& b)
    : Calculable(), m_a(a), m_b(b) {
    addSubexpression(a, false);
    addSubexpression(b, false);
  }

  BinaryExpression::BinaryExpression(const ExpressionId& a, bool aGarbage,
				     const ExpressionId& b, bool bGarbage)
    : Calculable(), m_a(a), m_b(b) {
    addSubexpression(a, aGarbage);
    addSubexpression(b, bGarbage);
  }

  //
  // N-Ary expressions
  //

  NaryExpression::NaryExpression(const PlexilExprId& expr, 
                                 const NodeConnectorId& node)
    : Calculable(expr, node) 
  {
  }

  NaryExpression::NaryExpression(const ExpressionId& a, const ExpressionId& b)
    : Calculable()
  {
    addSubexpression(a, false);
    addSubexpression(b, false);
  }
   
  NaryExpression::NaryExpression(const ExpressionId& a, bool aGarbage,
                                 const ExpressionId& b, bool bGarbage)
    : Calculable()
  {
    addSubexpression(a, aGarbage);
    addSubexpression(b, bGarbage);
  }
   
  bool Conjunction::checkValue(const double val)
  {
    return 
      val == BooleanVariable::TRUE() || 
      val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double Conjunction::recalculate()
  {
    // result is assumed to be true. from this point
    // the result may only be demoted to UNKNOWN or false
     
    double result = BooleanVariable::TRUE();
    double value = 0;
     
    // compute and store values for all subexpressions
     
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        value = (*child)->getValue();
        
        // validate values 
       
       checkError(checkValue(value), "Invalid (non-boolean) conjunction value '"
        		<< (*child)->valueString() << "' was returned to condition expression. "
        		<< "More details condition expression: " << (*child)->toString());
	
        // if the value is false, the expression is false, we're done
        
        if (value == BooleanVariable::FALSE())
          {	    
            result = BooleanVariable::FALSE();
            break;
          }
	  
        // if the value is unknown, the expression might be
        // unknown, but we need to keep looking
        
        if (value == BooleanVariable::UNKNOWN())
          result = BooleanVariable::UNKNOWN();
      }
    
    // return the result
     
    return result;
  }
   
  std::string Conjunction::toString() const
  {
    std::ostringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
         
        retval << (*child)->toString() << 
          (*child != m_subexpressions.back()
           ? " && " 
           : ")");
      }
    return retval.str();
  }

  bool Disjunction::checkValue(const double val)
  {
    return 
      val == BooleanVariable::TRUE() || 
      val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }
  
  double Disjunction::recalculate()
  {
    // result is assumed to be false. from this point
    // the result may only be demoted to UNKNOWN or true
      
    double result = BooleanVariable::FALSE();
    double value = 0;
      
    // compute and store values for all subexpressions
      
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        value = (*child)->getValue();
         
        // validate values 
         
        checkError(checkValue(value), "Invalid (non-boolean) disjunction value '"
        		<< (*child)->valueString() << "' was returned to condition expression. "
        		<< "More details condition expression: " << (*child)->toString());
	
        // if the value is true, the expression is true, we're done
         
        if (value == BooleanVariable::TRUE())
          {
            result = BooleanVariable::TRUE();
            break;
          }
	  
        // if the value is unknown, the expression might be
        // unknown, but we need to keep looking
         
        if (value == BooleanVariable::UNKNOWN())
          result = BooleanVariable::UNKNOWN();
      }
    
    // return the result
      
    return result;
  }


  std::string Disjunction::toString() const
  {
    std::ostringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
         
        retval << (*child)->toString() << 
          (*child != m_subexpressions.back()
           ? " || " 
           : ")");
      }
    return retval.str();
  }

  bool ExclusiveDisjunction::checkValue(const double val)
  {
    return 
      val == BooleanVariable::TRUE() || 
      val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }
  
  double ExclusiveDisjunction::recalculate()
  {
    // make a new list for values
      
    std::list<double> values;
      
    // compute and store values for all subexpressions
      
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        double value = (*child)->getValue();
        values.push_back(value);
         
        // validate values (your and important part of things!)
         
        checkError(checkValue(value), "Invalid exclusive or value: " << value);
      }
    // confirm we've got enough values

    checkError(values.size()  > 0, "Exclusive OR expression " <<
               this->toString() << "requires one or more subexpressions.");
      
    // inspect values of all subexpressions

    double result = 0;
    for (std::list<double>::iterator value = values.begin();
         value != values.end(); ++value)
      {
        // if the value is unknow, the entire expression is unknown

        if (*value == BooleanVariable::UNKNOWN())
          {
            result = BooleanVariable::UNKNOWN();
            break;
          }
        // if this is the first value init result to it's value

        if (value == values.begin())
          result = *value;

        // otherwise the value is the XOR of the result and the new value

        else
          result = 
            (result == BooleanVariable::TRUE() && 
             *value == BooleanVariable::FALSE()) ||
            (result == BooleanVariable::FALSE() && 
             *value == BooleanVariable::TRUE());
      }
    // return the result
      
    return result;
  }

  std::string ExclusiveDisjunction::toString() const
  {
    std::ostringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
         
        retval << (*child)->toString() << 
          (*child != m_subexpressions.back()
           ? " ^^ " 
           : ")");
      }
    return retval.str();
  }

  bool Concatenation::checkValue(const double val)
  {
    return LabelStr::isString(val);
  }

  double Concatenation::recalculate()
  {   
    std::ostringstream retval; 
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        double value = (*child)->getValue();
        // values.push_back(value);
        
        // validate values (you look ma'valouse!)
        checkError(checkValue(value), "Invalid concatenation value: " << value);

        // if a sub expression is UNKNOWN return UNKNOWN
        if (value == UNKNOWN()){
          // LabelStr ls2 (value);
          return UNKNOWN();
        }
        LabelStr ls1 (value);
        retval << ls1.toString();
      }
    LabelStr retvalLabel(retval.str());
    return retvalLabel.getKey();
  }

  std::string Concatenation::toString() const
  {
    std::ostringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
         
        retval << (*child)->toString() << 
          (*child != m_subexpressions.back()
           ? " + " 
           : ")");
      }
    return retval.str();
  }

  //
  // Comparisons
  //

  bool Equality::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double Equality::recalculate() {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();

    double value;
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      value = UNKNOWN();
    else
      value = (double) (v1 == v2);
    return value;
  }

  std::string Equality::toString() const {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " == " << m_b->toString() << "))";
    return retval.str();
  }

  bool Inequality::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double Inequality::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    double value;
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      value = UNKNOWN();
    else
      value = (double) (v1 != v2);
    return value;
  }

  std::string Inequality::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " != " << m_b->toString() << "))";
    return retval.str();
  }

  bool LessThan::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double LessThan::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    double value;
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      value = UNKNOWN();
    else
      value = (double) (v1 < v2);
    return value;
  }

  std::string LessThan::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " < " << m_b->toString() << "))";
    return retval.str();
  }

  bool LessEqual::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double LessEqual::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 <= v2);
  }

  std::string LessEqual::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " <= " << m_b->toString() << "))";
    return retval.str();
  }

  bool GreaterThan::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double GreaterThan::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 > v2);
  }

  std::string GreaterThan::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " > " << m_b->toString() << "))";
    return retval.str();
  }

  bool GreaterEqual::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double GreaterEqual::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 >= v2);
  }

  std::string GreaterEqual::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " >= " << m_b->toString() << "))";
    return retval.str();
  }


  //
  // Arithmetic expressions
  //

  double Addition::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if (v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 + v2);
  }

  std::string Addition::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " + " << m_b->toString() << "))";
    return retval.str();
  }

  PlexilType Addition::getValueType() const
  {
    PlexilType aType = m_a->getValueType();
    if (aType == REAL)
      return REAL;
    PlexilType bType = m_b->getValueType();
    if (aType == bType)
      return aType;
    // default to real
    return REAL;
  }


  double Subtraction::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 - v2);
  }


  std::string Subtraction::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " - " << m_b->toString() << "))";
    return retval.str();
  }

  PlexilType Subtraction::getValueType() const
  {
    PlexilType aType = m_a->getValueType();
    if (aType == REAL)
      return REAL;
    PlexilType bType = m_b->getValueType();
    if (aType == bType)
      return aType;
    // default to real
    return REAL;
  }


  double Multiplication::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 * v2);
  }

  std::string Multiplication::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " * " << m_b->toString() << "))";
    return retval.str();
  }

  PlexilType Multiplication::getValueType() const
  {
    PlexilType aType = m_a->getValueType();
    if (aType == REAL)
      return REAL;
    PlexilType bType = m_b->getValueType();
    if (aType == bType)
      return aType;
    // default to real
    return REAL;
  }


  double Division::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();

    check_error(v2 != 0.0, "Attempt to divide by zero");

    return (double) (v1 / v2);
  }

  std::string Division::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " / " << m_b->toString() << "))";
    return retval.str();
  }

  PlexilType Division::getValueType() const
  {
    return REAL;
  }


  double Modulo::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();

    check_error(v2 != 0.0, "Attempt to divide by zero");

    return (double) fmod (v1, v2);
  }

  std::string Modulo::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " % " << m_b->toString() << "))";
    return retval.str();
  }

  PlexilType Modulo::getValueType() const
  {
    return REAL;
  }


  AllChildrenFinishedCondition::AllChildrenFinishedCondition(std::vector<NodeId>& children)
    : Calculable(), m_listener(*this), m_total(0), m_count(0), m_constructed(false) {
    for(std::vector<NodeId>::iterator it = children.begin(); it != children.end(); ++it) {
      NodeId child = *it;
      check_error(child.isValid());
      addChild(child);
    }
    internalSetValue(recalculate());
    m_constructed = true;
  }

  AllChildrenFinishedCondition::~AllChildrenFinishedCondition() {
    for(std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      NodeId child = *it;
      child->getStateVariable()->removeListener(m_listener.getId());
    }
  }

  void AllChildrenFinishedCondition::addChild(const NodeId& node) {
    m_children.push_back(node);
    ++m_total;
    node->getStateVariable()->addListener(m_listener.getId());
    if(m_constructed) {
      if(node->getStateName() == StateVariable::FINISHED())
	incrementCount(node->getStateVariable());
      else if(getValue() == BooleanVariable::TRUE())
	internalSetValue(BooleanVariable::FALSE());
    }
  }

  void AllChildrenFinishedCondition::incrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = UNKNOWN();
    if(expr->getValue() == StateVariable::FINISHED() &&
       m_lastValues[expr] != StateVariable::FINISHED()) {
      debugMsg("AllChildrenFinished:increment",
	       "State var " << expr->toString() << " is now FINISHED.  Incrementing count.");
      m_count++;
      checkError(m_count <= m_total,
		 "Error: somehow counted more nodes in finished than were actually there.");
      if(m_count == m_total) {
	debugMsg("AllChildrenFinished:increment",
		 "Counted " << m_count << " children finished of " << m_total <<
		 ".  Setting TRUE.");
	internalSetValue(BooleanVariable::TRUE());
      }
    }
    m_lastValues[expr] = expr->getValue();
  }

  void AllChildrenFinishedCondition::decrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = UNKNOWN();
    if(expr->getValue() != StateVariable::FINISHED() &&
       m_lastValues[expr] == StateVariable::FINISHED()) {
      debugMsg("AllChildrenFinished:decrement",
	       "State var " << expr->toString() <<
	       " is no longer FINISHED.  Decrementing count.");
      m_count--;
      checkError(m_count <= m_total,
		 "Error: somehow counted more nodes in finished than were actually there.");
      if(getValue() == BooleanVariable::TRUE()) {
	debugMsg("AllChildrenFinished:decrement",
		 m_count << " children of " << m_total << " are FINISHED.  Setting FALSE.");
	internalSetValue(BooleanVariable::FALSE());
      }
    }
    m_lastValues[expr] = expr->getValue();
  }

  void AllChildrenFinishedCondition::handleActivate(const bool changed) {
    if(changed)
      m_listener.activate();
    Calculable::handleActivate(changed);
  }

  void AllChildrenFinishedCondition::handleDeactivate(const bool changed) {
    if(changed)
      m_listener.deactivate();
    Calculable::handleDeactivate(changed);
  }

  double AllChildrenFinishedCondition::recalculate() {
    m_count = 0;
    for(std::vector<NodeId>::const_iterator it = m_children.begin(); it != m_children.end();
	++it) {
      NodeId child = *it;
      check_error(child.isValid());
      ExpressionId expr = child->getStateVariable();
      if(m_lastValues.find(expr) == m_lastValues.end())
	m_lastValues[expr] = UNKNOWN();
      m_lastValues[expr] = expr->getValue();

      if(m_lastValues[expr] == StateVariable::FINISHED())
	++m_count;
    }
    checkError(m_count <= m_total,
	       "Error: somehow counted more nodes in waiting or finished (" << m_count <<
	       ") than were actually there (" << m_total << ").");
    if(m_count == m_total) {
      debugMsg("AllChildrenFinished:recalculate",
	       "Counted " << m_count << " of " << m_total <<
	       " children FINISHED.  Setting TRUE.");
      return BooleanVariable::TRUE();
    }
    else {
      debugMsg("AllChildrenFinished:recalculate",
	       "Counted " << m_count << " of " << m_total <<
	       " children FINISHED.  Setting FALSE.");
      return BooleanVariable::FALSE();
    }
  }

  bool AllChildrenFinishedCondition::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  AllChildrenFinishedCondition::FinishedListener::FinishedListener(AllChildrenFinishedCondition& cond)
    : ExpressionListener(), m_cond(cond) {}

  void
  AllChildrenFinishedCondition::FinishedListener::notifyValueChanged(const ExpressionId& expression) 
  {
    checkError(dynamic_cast<const StateVariable*>((const Expression*)expression) != NULL,
	       "Finished listener not listening on a state variable.");
    if(expression->getValue() == StateVariable::FINISHED())
      m_cond.incrementCount(expression);
    else
      m_cond.decrementCount(expression);
  }

  std::string AllChildrenFinishedCondition::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "childrenFinished(" << m_count << ":" << m_total << "))";
    return retval.str();
  }

  /***************************/

  AllChildrenWaitingOrFinishedCondition::AllChildrenWaitingOrFinishedCondition(std::vector<NodeId>& children)
    : Calculable(), m_listener(*this), m_total(0), m_count(0), m_constructed(false) {
    for(std::vector<NodeId>::iterator it = children.begin(); it != children.end(); ++it) {
      NodeId child = *it;
      check_error(child.isValid());
      addChild(child);
    }
    internalSetValue(recalculate());
    m_constructed = true;
  }

  AllChildrenWaitingOrFinishedCondition::~AllChildrenWaitingOrFinishedCondition() {
    for(std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      NodeId child = *it;
      child->getStateVariable()->removeListener(m_listener.getId());
    }
  }

  void AllChildrenWaitingOrFinishedCondition::addChild(const NodeId& node) {
    m_children.push_back(node);
    ++m_total;
    node->getStateVariable()->addListener(m_listener.getId());
    if(m_constructed) {
      const LabelStr& state = node->getStateName();
      if (state == StateVariable::WAITING() ||
	  state == StateVariable::FINISHED())
	incrementCount(node->getStateVariable());
      else if(getValue() == BooleanVariable::TRUE())
	internalSetValue(BooleanVariable::FALSE());
    }
  }

  void AllChildrenWaitingOrFinishedCondition::incrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = UNKNOWN();
    if((expr->getValue() == StateVariable::WAITING() || expr->getValue() == StateVariable::FINISHED()) &&
       !(m_lastValues[expr] == StateVariable::WAITING() || m_lastValues[expr] == StateVariable::FINISHED())) {
      m_count++;
      checkError(m_count <= m_total,
		 "Error: somehow counted more nodes in waiting or finished than were actually there.");
      if(m_count == m_total)
	internalSetValue(BooleanVariable::TRUE());
    }
    m_lastValues[expr] = expr->getValue();
  }

  void AllChildrenWaitingOrFinishedCondition::decrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = UNKNOWN();
    if(!(expr->getValue() == StateVariable::WAITING() || expr->getValue() == StateVariable::FINISHED()) &&
       (m_lastValues[expr] == StateVariable::WAITING() || m_lastValues[expr] == StateVariable::FINISHED())) {
      m_count--;
      checkError(m_count <= m_total,
		 "Error: somehow counted more nodes in waiting or finished than were actually there.");
      if(getValue() == BooleanVariable::TRUE())
	internalSetValue(BooleanVariable::FALSE());
    }
    m_lastValues[expr] = expr->getValue();
  }

  void AllChildrenWaitingOrFinishedCondition::handleActivate(const bool changed) {
    if(changed)
      m_listener.activate();
    Calculable::handleActivate(changed);
  }

  void AllChildrenWaitingOrFinishedCondition::handleDeactivate(const bool changed) {
    if(changed)
      m_listener.deactivate();
    Calculable::handleDeactivate(changed);
  }

  double AllChildrenWaitingOrFinishedCondition::recalculate() {
    m_count = 0;
    for(std::vector<NodeId>::const_iterator it = m_children.begin(); it != m_children.end();
	++it) {
      NodeId child = *it;
      check_error(child.isValid());
      ExpressionId expr = child->getStateVariable();
      if(m_lastValues.find(expr) == m_lastValues.end())
	m_lastValues[expr] = UNKNOWN();
      m_lastValues[expr] = expr->getValue();

      if(m_lastValues[expr] == StateVariable::WAITING() ||
	 m_lastValues[expr] == StateVariable::FINISHED())
	++m_count;
    }
    checkError(m_count <= m_total,
	       "Error: somehow counted more nodes in waiting or finished (" << m_count <<
	       ") than were actually there (" << m_total << ").");
    if(m_count == m_total)
      return BooleanVariable::TRUE();
    else
      return BooleanVariable::FALSE();
  }

  bool AllChildrenWaitingOrFinishedCondition::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  AllChildrenWaitingOrFinishedCondition::WaitingOrFinishedListener::WaitingOrFinishedListener(AllChildrenWaitingOrFinishedCondition& cond)
    : ExpressionListener(), m_cond(cond) {}

  void 
  AllChildrenWaitingOrFinishedCondition::WaitingOrFinishedListener::notifyValueChanged(const ExpressionId& expression)
  {
    checkError(dynamic_cast<StateVariable*>((Expression*)expression) != NULL,
	       "Waiting or finished listener not listening on a state variable.");
    if(expression->getValue() == StateVariable::WAITING() ||
       expression->getValue() == StateVariable::FINISHED())
      m_cond.incrementCount(expression);
    else
      m_cond.decrementCount(expression);
  }

  std::string AllChildrenWaitingOrFinishedCondition::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "childrenWaitingOrFinished(" << m_count << ":" << m_total << "))";
    return retval.str();
  }

  /*
    <(N?)EQInternal>
    <NodeStateVariable> ||
    <NodeStateValue>
    OR
    <NodeOutcomeVariable> ||
    <NodeOutcomeValue>
    </(N?)EQInternal>
  */

  InternalCondition::InternalCondition(const PlexilExprId& /* xml */)
    : Calculable() {
    checkError(ALWAYS_FAIL, "Internal conditions require a Node argument");
  }

  InternalCondition::InternalCondition(const PlexilExprId& expr, const NodeConnectorId& node)
    : Calculable() {
    checkError(Id<PlexilOp>::convertable(expr), "Expected an op.");
    PlexilOp* op = (PlexilOp*) expr;
    checkError(op->getOp() == "EQInternal" || op->getOp() == "NEInternal",
	       "Expected EQInternal or NEInternal");

    bool first = true;
    for(std::vector<PlexilExprId>::const_iterator it = op->subExprs().begin();
	it != op->subExprs().end(); ++it) {
      bool garbage = false;
      ExpressionId subExpr = getSubexpression(*it, node, garbage);
      debugMsg("InternalCondition:InternalCondition",
	       "Adding " << subExpr->toString() << " as " << (garbage ? "" : "non-") << " garbage.");
      if(garbage)
	m_garbage.insert(subExpr);
      if(first) {
	m_first = subExpr;
	first = false;
      }
      else
	m_second = subExpr;
    }

    checkError(m_first.isValid() && m_second.isValid(),
	       "Expected two subexpressions in " << expr->name());

    //m_subexpressions.clear();

    if(op->getOp() == "EQInternal")
      m_expr = (new Equality(m_first, m_second))->getId();
    else if(op->getOp() == "NEInternal")
      m_expr = (new Inequality(m_first, m_second))->getId();
    addSubexpression(m_expr, false);
  }

  InternalCondition::~InternalCondition() {
    removeSubexpression(m_expr);
    delete (Expression*) m_expr;
  }

  double InternalCondition::recalculate() {return m_expr->getValue();}

  bool InternalCondition::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  std::string InternalCondition::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << m_expr->toString();
    retval << ")";
    return retval.str();
  }

  TimepointVariable::TimepointVariable(const PlexilExprId& expr, const NodeConnectorId& node)
    : ConstVariableWrapper() {
    checkError(Id<PlexilTimepointVar>::convertable(expr),
	       "Expected NodeTimepoint element, got " << expr->name());

    setWrapped(node->findVariable((PlexilVarRef*)expr));
  }

}
