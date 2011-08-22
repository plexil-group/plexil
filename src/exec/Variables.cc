/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

#include "Variables.hh"
#include "Debug.hh"
#include "ExpressionFactory.hh"
#include "NodeConnector.hh"
#include "StoredArray.hh"

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

  TimepointVariable::TimepointVariable(const PlexilExprId& expr, const NodeConnectorId& node)
    : ConstVariableWrapper() {
    checkError(Id<PlexilTimepointVar>::convertable(expr),
	       "Expected NodeTimepoint element, got " << expr->name());

    setWrapped(node->findVariable((PlexilVarRef*)expr));
  }

}
