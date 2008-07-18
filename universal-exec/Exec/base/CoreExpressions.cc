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
#include "Utils.hh"
#include "CommandHandle.hh"
#include <string>
#include <sstream>
#include <list>

namespace PLEXIL 
{

  ArrayVariable::ArrayVariable(unsigned maxSize, PlexilType type,
                               const bool isConst)
    : Variable(isConst), m_maxSize(maxSize), m_type(type)
  {
    StoredArray array(m_maxSize, Expression::UNKNOWN());
    setValue(array.getKey());
  }

  ArrayVariable::ArrayVariable(unsigned maxSize, PlexilType type, 
                               std::vector<double>& values, const bool isConst)
    : Variable(isConst), m_maxSize(maxSize), m_type(type)
  {
    StoredArray array(m_maxSize, Expression::UNKNOWN());
    setValue(array.getKey());
    setValues(values);
  }

  ArrayVariable::ArrayVariable(const PlexilExprId& expr, 
                               const NodeConnectorId& node,
                               const bool isConst)
    : Variable(expr, node, isConst)
  {
    // confirm that we have a an array

    checkError(Id<PlexilArrayValue>::convertable(expr),
               "Expected an array value.");
    PlexilArrayValue* arrayValue = (PlexilArrayValue*)expr;

    // init the local type and array
    m_type = arrayValue->type().plexilType();
    m_maxSize = arrayValue->maxSize();
    StoredArray array(m_maxSize, Expression::UNKNOWN());
    setValue(array.getKey());

    // convert strings to doubles for internal storage
    std::vector<double> convertedValues;
    const std::vector<std::string>& values = arrayValue->values();

    for (std::vector<std::string>::const_iterator value = values.begin();
         value != values.end(); ++value)
      {
        double convertedValue;
        if (m_type == STRING)
          convertedValue = (double)LabelStr(*value);
        if (m_type == BOOLEAN)
          {
            if (compareIgnoreCase(*value, "true") || 
                (strcmp(value->c_str(), "1") == 0))
              convertedValue = 1;
            else if (compareIgnoreCase(*value, "false") || 
                     (strcmp(value->c_str(), "0") == 0))
              convertedValue = 0;
            else
              checkError(false, "Invalid boolean value \"" << *value << "\"");
          }
        else
          {
            std::stringstream valueStream(*value);
            valueStream >> convertedValue;
          }
        convertedValues.push_back(convertedValue);
      }

    // set values

    setValues(convertedValues);
    m_initialValue = m_value;
  }

  // 
  // Destructor
  // Frees the array
  //

  ArrayVariable::~ArrayVariable()
  {
    StoredArray theArray(m_value);
    theArray.unregister();
  }


  // set the value of this array

  void ArrayVariable::setValue(const double value)
  {
    // if the value is unknown, then set the value of the
    // array as a whole to unknown, and free the old storage.

    if (value == Expression::UNKNOWN())
      {
        double oldValue = m_value;
        Variable::setValue(value);

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

    else if (m_value == Expression::UNKNOWN())
      {
        // create a copy of the array
        StoredArray newArray(m_maxSize, Expression::UNKNOWN());
        StoredArray sourceArray(value);
        checkError(newArray.size() >= sourceArray.size(),
                   "Source array size " << sourceArray.size() <<
                   ", exceeds target size " << newArray.size() << ".");
        for (size_t i = 0; i < sourceArray.size(); i++)
          {
            newArray[i] = sourceArray[i];
          }

        Variable::setValue(newArray.getKey());
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
        checkError(checkElementValue(*value),
                   "Attempted to initialize a variable with an invalid value.");
        array[index++] = *value;
      }

    // fill out the rest of the array with "UNKNOWN"

    while (index < m_maxSize)
      array[index++] = Expression::UNKNOWN();

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
      array[index++] = Expression::UNKNOWN();

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
    checkError(!isConst(),
               "Attempted to set element value " << value << " to " << toString());
    checkError(checkElementValue(value),
               "Attempted to assign an invalid value to an array element");
    checkError(checkIndex(index),
               "Array index " << index << " exceeds bound of " 
               << m_maxSize);
    checkError(m_value != Expression::UNKNOWN(),
               "Attempted to assign an array element in an UNKNOWN array");

    // set the element
    StoredArray theArray(m_value);
    theArray[index] = value;

    // publish change
    publishChange();
  }

  // lookup a value in an array variable

  double ArrayVariable::lookupValue(unsigned long index)
  {
    checkError(checkIndex(index),
               "Array index " << index << " exceeds bound of " 
               << m_maxSize);
    return m_value == Expression::UNKNOWN()
      ? Expression::UNKNOWN()
      : StoredArray(m_value)[index];
  }

  std::string ArrayVariable::toString() const 
  {
    std::stringstream retval;
    retval << Expression::toString();
    if (m_value == Expression::UNKNOWN())
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
            if (value == Expression::UNKNOWN())
              retval << "<unknown>";
            else
              {
                switch (m_type)
                  {
                  case INTEGER:
                    retval << (int)value;
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
    if (val == Expression::UNKNOWN())
      return true;
    if (StoredArray::isItem(val))
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
        return val == Expression::UNKNOWN() ||
          ((val >= MINUS_INFINITY && val <= PLUS_INFINITY) &&
           val == (double) (int) val);
      case REAL:
        return (val >= REAL_MINUS_INFINITY && val <= REAL_PLUS_INFINITY) ||
          val == Expression::UNKNOWN();
      case BOOLEAN:
        return val == Expression::UNKNOWN() || val == 0.0 || val == 1.0;
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
    : DerivedVariable(expr),
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
    ExpressionId arrayVar = node->findVariable(&arrayRef);
    checkError(ArrayVariableId::convertable(arrayVar),
               "Expected Array Variable but found: " << 
               arrayVar->toString());
    m_arrayVariable = (ArrayVariableId) arrayVar;    
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

//   double ArrayElement::recalculate()
//   {
//     // etablish name and index of array

//     LabelStr name = LabelStr(m_a->getValue());
//     unsigned index = (unsigned)m_b->getValue();

//     // find the array

//     PlexilVarRef arrayRef;
//     arrayRef.setName(name.toString());
//     ExpressionId arrayExpr = m_node->findVariable(&arrayRef);
//     checkError(ArrayVariableId::convertable(arrayExpr),
//                "Expected Array Variable but found: " << 
//                arrayExpr->toString());

//     // add a listener

//     arrayExpr->addListener(m_listener.getId());

//     // get array elemeent value

//     ArrayVariable* array = (ArrayVariable*)arrayExpr;
//     double value = array->lookupValue(index);
//     return value;
//   }

  // confirm that new value to assign is valid

  bool ArrayElement::checkValue(const double val)
  {
    return m_arrayVariable->checkElementValue(val);
  }

  std::string ArrayElement::toString() const
  {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "ArrayElement: " << m_arrayVariable->toString()
           << "[" << m_index->toString() << "])";
    return retval.str();
  }

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

  void ArrayElement::handleChange(const ExpressionId& ignored)
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
    if (index == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    return m_arrayVariable->lookupValue((unsigned long) index);
  }

  StringVariable::StringVariable(const bool isConst) : Variable(isConst) {}

  StringVariable::StringVariable(const std::string& value, const bool isConst)
    : Variable(LabelStr(value), isConst) {}

  StringVariable::StringVariable(const char* value, const bool isConst)
    : Variable(LabelStr(value), isConst) {}

  StringVariable::StringVariable(const double value, const bool isConst)
    : Variable(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  StringVariable::StringVariable(const LabelStr& value, const bool isConst)
    : Variable(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  StringVariable::StringVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				 const bool isConst)
    : Variable(expr, node, isConst) {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == "String",
	       "Expected a String value.  Got " << val->type());

    m_initialValue = m_value = (double)LabelStr(val->value());
  }


  std::string StringVariable::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "string)";
    return retval.str();
  }

  bool StringVariable::checkValue(const double val) {
    return LabelStr::isString(val);
  }

  RealVariable::RealVariable(const bool isConst) : Variable(isConst) {}

  RealVariable::RealVariable(const double value, const bool isConst)
    : Variable(value, isConst) 
  {
    checkError(checkValue(value),
               "Attempted to initialize a variable with an invalid value.");
  }

  RealVariable::RealVariable(const PlexilExprId& expr, const NodeConnectorId& node,
                             const bool isConst) : Variable(expr, node, isConst) 
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    commonNumericInit((PlexilValue*)expr);
  }

  std::string RealVariable::toString() const 
  {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "real)";
    return retval.str();
  }
   
  std::string RealVariable::valueString() const 
  {
    std::stringstream retval;
    if(m_value == Expression::UNKNOWN())
      retval << "UNKNOWN";
    else if(m_value == REAL_PLUS_INFINITY)
      retval << "inf";
    else if(m_value == REAL_MINUS_INFINITY)
      retval << "-inf";
    else
      retval << m_value;
    return retval.str();
  }

  bool RealVariable::checkValue(const double val) {
    return (val >= REAL_MINUS_INFINITY && val <= REAL_PLUS_INFINITY) ||
      val == Expression::UNKNOWN();
  }

  ExpressionId& RealVariable::ZERO_EXP() {
    static ExpressionId sl_zero_exp = (new RealVariable(0.0, true))->getId();
    if(!sl_zero_exp->isActive())
      sl_zero_exp->activate();
    return sl_zero_exp;
  }

  ExpressionId& RealVariable::ONE_EXP() {
    static ExpressionId sl_one_exp = (new RealVariable(1.0, true))->getId();
    if(!sl_one_exp->isActive())
      sl_one_exp->activate();
    return sl_one_exp;
  }

  ExpressionId& RealVariable::MINUS_ONE_EXP() {
    static ExpressionId sl_minus_one_exp = (new RealVariable(-1.0, true))->getId();
    if(!sl_minus_one_exp->isActive())
      sl_minus_one_exp->activate();
    return sl_minus_one_exp;
  }

  IntegerVariable::IntegerVariable(const bool isConst) : Variable(isConst) {}

  IntegerVariable::IntegerVariable(const double value, const bool isConst)
    : Variable(value, isConst) 
  {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");

  }

  IntegerVariable::IntegerVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool isConst) : Variable(expr, node, isConst) {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    commonNumericInit((PlexilValue*)expr);
  }

  std::string IntegerVariable::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "int)";
    return retval.str();
  }

  std::string IntegerVariable::valueString() const {
    std::stringstream retval;
    if(m_value == Expression::UNKNOWN())
      retval << "UNKNOWN";
    else if(m_value == REAL_PLUS_INFINITY)
      retval << "inf";
    else if(m_value == REAL_MINUS_INFINITY)
      retval << "-inf";
    else
      retval << (int) m_value;
    return retval.str();
  }

  bool IntegerVariable::checkValue(const double val) {
    return val == Expression::UNKNOWN() ||
      ((val >= MINUS_INFINITY && val <= PLUS_INFINITY) &&
       val == (double) (int) val);  //more cast means more double
  }

  ExpressionId& IntegerVariable::ZERO_EXP()
  {
    static ExpressionId sl_zero_exp = (new IntegerVariable(0.0, true))->getId();
    if(!sl_zero_exp->isActive())
      sl_zero_exp->activate();
    return sl_zero_exp;
  }

  ExpressionId& IntegerVariable::ONE_EXP()
  {
    static ExpressionId sl_one_exp = (new IntegerVariable(1.0, true))->getId();
    if(!sl_one_exp->isActive())
      sl_one_exp->activate();
    return sl_one_exp;
  }

  ExpressionId& IntegerVariable::MINUS_ONE_EXP()
  {
    static ExpressionId sl_minus_one_exp =
      (new IntegerVariable(-1.0, true))->getId();
    if(!sl_minus_one_exp->isActive())
      sl_minus_one_exp->activate();
    return sl_minus_one_exp;
  }

  BooleanVariable::BooleanVariable(const bool isConst) : Variable(isConst){}
  BooleanVariable::BooleanVariable(const double value, const bool isConst)
    : Variable(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  BooleanVariable::BooleanVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool isConst) : Variable(expr, node) {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    commonNumericInit((PlexilValue*)expr);
  }

  std::string BooleanVariable::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "boolean)";
    return retval.str();
  }

  std::string BooleanVariable::valueString() const {
    std::stringstream retval;
    if(m_value == Expression::UNKNOWN())
      retval << "UNKNOWN";
    else
      retval << m_value;
    return retval.str();
  }

  bool BooleanVariable::checkValue(const double val) {
    return val == Expression::UNKNOWN() || val == 0.0 || val == 1.0;
  }

  ExpressionId& BooleanVariable::TRUE_EXP() {
    static ExpressionId sl_exp = (new BooleanVariable(1.0, true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  ExpressionId& BooleanVariable::FALSE_EXP() {
    static ExpressionId sl_exp = (new BooleanVariable(0.0, false))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  //
  // StateVariable
  //

  StateVariable::StateVariable(const bool isConst) : Variable(INACTIVE(), isConst) {}

  StateVariable::StateVariable(const double value, const bool isConst)
    : Variable(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  StateVariable::StateVariable(const PlexilExprId& expr, const NodeConnectorId& node,
			       const bool isConst) : Variable(expr, node, isConst) {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == "NodeState",
	       "Expected NodeState value.  Found '" << val->type() << "'");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  bool StateVariable::checkValue(const double val) {
    return val == INACTIVE() || val == WAITING() || val == EXECUTING() ||
      val == FINISHING() || val == FINISHED() || val == FAILING() || val == ITERATION_ENDED();
  }

  std::string StateVariable::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "state(" << LabelStr(m_value).toString() << "))";
    return retval.str();
  }

  const std::set<double>& StateVariable::ALL_STATES() {
    static bool init = true;
    static std::set<double> allStates;
    if(init) {
      allStates.insert(INACTIVE());
      allStates.insert(WAITING());
      allStates.insert(EXECUTING());
      allStates.insert(FINISHING());
      allStates.insert(FINISHED());
      allStates.insert(FAILING());
      allStates.insert(ITERATION_ENDED());
      init = false;
    }
    return allStates;
  }

  ExpressionId& StateVariable::INACTIVE_EXP() {
    static ExpressionId sl_exp = (new StateVariable(INACTIVE(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::WAITING_EXP() {
    static ExpressionId sl_exp = (new StateVariable(WAITING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::EXECUTING_EXP() {
    static ExpressionId sl_exp = (new StateVariable(EXECUTING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::FINISHING_EXP() {
    static ExpressionId sl_exp = (new StateVariable(FINISHING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::FINISHED_EXP() {
    static ExpressionId sl_exp = (new StateVariable(FINISHED(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::FAILING_EXP() {
    static ExpressionId sl_exp = (new StateVariable(FAILING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::ITERATION_ENDED_EXP() {
    static ExpressionId sl_exp = (new StateVariable(ITERATION_ENDED(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::NO_STATE_EXP() {
    static ExpressionId sl_exp = (new StateVariable(NO_STATE(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  OutcomeVariable::OutcomeVariable(const bool isConst) : Variable(isConst) {}
  OutcomeVariable::OutcomeVariable(const double value, const bool isConst)
    : Variable(isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }
  OutcomeVariable::OutcomeVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool isConst) : Variable(expr, node, isConst) {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == "NodeOutcome",
	       "Expected NodeOutcome value.  Found " << val->type() << ".");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  bool OutcomeVariable::checkValue(const double val) {
    return (val == UNKNOWN() || val == SUCCESS() || val == FAILURE() || val == SKIPPED());
  }

  std::string OutcomeVariable::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << " outcome)";
    return retval.str();
  }


  FailureVariable::FailureVariable(const bool isConst) : Variable(isConst) {}

  FailureVariable::FailureVariable(const double value, const bool isConst)
    : Variable(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  FailureVariable::FailureVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool isConst) : Variable(expr, node, isConst) {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == "NodeFailure",
	       "Expected NodeFailure value.  Found " << val->type() << ".");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  std::string FailureVariable::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << " failure)";
    return retval.str();
  }

  bool FailureVariable::checkValue(const double val) {
    return val == UNKNOWN() || val == INFINITE_LOOP() || val == PRE_CONDITION_FAILED() ||
      val == POST_CONDITION_FAILED() || val == INVARIANT_CONDITION_FAILED() || ANCESTOR_INVARIANT_CONDITION_FAILED() || val == PARENT_FAILED();
  }

  CommandHandleVariable::CommandHandleVariable(const bool isConst) : Variable(isConst) {}
  CommandHandleVariable::CommandHandleVariable(const double value, const bool isConst)
    : Variable(isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }
  CommandHandleVariable::CommandHandleVariable(const PlexilExprId& expr, const NodeConnectorId& node,
                                               const bool isConst) : Variable(expr, node, isConst) {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == "NodeCommandHandle",
	       "Expected NodeCommandHandle value.  Found " << val->type() << ".");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  bool CommandHandleVariable::checkValue(const double val) {
    return (val == UNKNOWN() || val == COMMAND_SENT_TO_SYSTEM() || val == COMMAND_ACCEPTED() || val == COMMAND_RCVD_BY_SYSTEM() || val == COMMAND_SUCCESS() || val == COMMAND_DENIED() || val == COMMAND_FAILED());
  }

  std::string CommandHandleVariable::toString() const {
    std::stringstream retval;
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
    checkError(v == 1.0 || v == 0.0 || v == Expression::UNKNOWN(),
	       "Invalid value in logical negation: " << v);
    if (v == 1.0)
      return 0.0;
    else if (v == 0.0)
      return 1.0;
    else if (v == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    check_error(ALWAYS_FAIL);
    return -1.0;
  }

  std::string LogicalNegation::toString() const
  {
    std::stringstream retval;
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

  NaryExpression::NaryExpression(ExpressionList& children)
    : Calculable()
  {
    for (ExpressionListItr it = children.begin(); it != children.end(); ++it)
      addSubexpression(*it, false);
  }

  NaryExpression::NaryExpression(ExpressionList& children,
                                 std::list<bool>& garbage)
    : Calculable()
  {
    // children and garbage should be isomorphic

    checkError(children.size() == garbage.size(),
               "Expression and garbage list size missmatch.");

    // get iterators

    std::list<bool>::iterator garbageItr = garbage.begin();
    ExpressionListItr childrenItr = children.begin();

    // run through children expressions and associated
    // garbage indicators and add them as sub expresssions

    while (childrenItr != children.end() && 
           garbageItr != garbage.end())
      {
        addSubexpression(*childrenItr, *garbageItr);
        ++childrenItr;
        ++garbageItr;
      }
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
    // make a new list for values
     
    std::list<double> values;
     
    // compute and store values for all subexpressions
     
    for (ExpressionListConstItr child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        double value = (*child)->getValue();
        values.push_back(value);
        
        // validate values (you look ma'valouse!)
         
        checkError(checkValue(value), "Invalid conjunction value: " << value);
      }
    // result is assumed to be true. from this point
    // the result may only be demoted to UNKNOWN or false
     
    double result = BooleanVariable::TRUE();
     
    // inspect values of all subexpressions
     
    for (std::list<double>::iterator value = values.begin();
         value != values.end(); ++value)
      {
        // if the value is false, the expression is false, we're done
        
        if (*value == BooleanVariable::FALSE())
          {
            result = BooleanVariable::FALSE();
            break;
          }
        // if the value is unknown, the expression might be
        // unknown, but we need to keep looking
        
        if (*value == BooleanVariable::UNKNOWN())
          result = BooleanVariable::UNKNOWN();
      }
    // return the result
     
    return result;
  }
   
  std::string Conjunction::toString() const
  {
    std::stringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionListConstItr child = m_subexpressions.begin();
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
    // make a new list for values
      
    std::list<double> values;
      
    // compute and store values for all subexpressions
      
    for (ExpressionListConstItr child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        double value = (*child)->getValue();
        values.push_back(value);
         
        // validate values (your and important part of things!)
         
        checkError(checkValue(value), "Invalid distjunct value: " << value);
      }
    // result is assumed to be false. from this point
    // the result may only be demoted to UNKNOWN or true
      
    double result = BooleanVariable::FALSE();
      
    // inspect values of all subexpressions
      
    for (std::list<double>::iterator value = values.begin();
         value != values.end(); ++value)
      {
        // if the value is true, the expression is true, we're done
         
        if (*value == BooleanVariable::TRUE())
          {
            result = BooleanVariable::TRUE();
            break;
          }
        // if the value is unknown, the expression might be
        // unknown, but we need to keep looking
         
        if (*value == BooleanVariable::UNKNOWN())
          result = BooleanVariable::UNKNOWN();
      }
    // return the result
      
    return result;
  }


  std::string Disjunction::toString() const
  {
    std::stringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionListConstItr child = m_subexpressions.begin();
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
      
    for (ExpressionListConstItr child = m_subexpressions.begin();
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
    std::stringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionListConstItr child = m_subexpressions.begin();
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
    std::stringstream retval; 
    for (ExpressionListConstItr child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        double value = (*child)->getValue();
        // values.push_back(value);
        
        // validate values (you look ma'valouse!)
        checkError(checkValue(value), "Invalid concatenation value: " << value);

        // if a sub expression is UNKNOWN return UNKNOWN
        if (value == Expression::UNKNOWN()){
          // LabelStr ls2 (value);
          return Expression::UNKNOWN();
        }
        LabelStr ls1 (value);
        retval << ls1.toString();
      }
    LabelStr retvalLabel(retval.str());
    return retvalLabel.getKey();
  }

  std::string Concatenation::toString() const
  {
    std::stringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionListConstItr child = m_subexpressions.begin();
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
    if(v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      value = Expression::UNKNOWN();
    else
      value = (double) (v1 == v2);
    return value;
  }

  std::string Equality::toString() const {
    std::stringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " == " << m_b->toString() << "))";
    return retval.str();
  }

  DerivedVariable::DerivedVariable(const PlexilExprId& expr)
    : EssentialVariable(expr)
  {
  }


  DerivedVariableListener::DerivedVariableListener(const ExpressionId& exp)
    : ExpressionListener(), m_exp(exp)
  {
    checkError(Id<DerivedVariable>::convertable(exp),
               "Attempt to create a DerivedVariableListener instance with invalid expression type");
  }

  void DerivedVariableListener::notifyValueChanged(const ExpressionId& exp)
  {
    // prevent infinite loop
    if (m_exp != exp)
      {
        ((Id<DerivedVariable>) m_exp)->handleChange(exp);
      }
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
    if(v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      value = Expression::UNKNOWN();
    else
      value = (double) (v1 != v2);
    return value;
  }

  std::string Inequality::toString() const
  {
    std::stringstream retval;
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
    if(v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      value = Expression::UNKNOWN();
    else
      value = (double) (v1 < v2);
    return value;
  }

  std::string LessThan::toString() const
  {
    std::stringstream retval;
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
    if(v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    return (double) (v1 <= v2);
  }

  std::string LessEqual::toString() const
  {
    std::stringstream retval;
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
    if(v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    return (double) (v1 > v2);
  }

  std::string GreaterThan::toString() const
  {
    std::stringstream retval;
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
    if(v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    return (double) (v1 >= v2);
  }

  std::string GreaterEqual::toString() const
  {
    std::stringstream retval;
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
    if (v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    return (double) (v1 + v2);
  }

  std::string Addition::toString() const
  {
    std::stringstream retval;
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
    if(v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    return (double) (v1 - v2);
  }


  std::string Subtraction::toString() const
  {
    std::stringstream retval;
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
    if(v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    return (double) (v1 * v2);
  }

  std::string Multiplication::toString() const
  {
    std::stringstream retval;
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
    if(v1 == Expression::UNKNOWN() || v2 == Expression::UNKNOWN())
      return Expression::UNKNOWN();

    check_error(v2 != 0.0, "Attempt to divide by zero");

    return (double) (v1 / v2);
  }

  std::string Division::toString() const
  {
    std::stringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " / " << m_b->toString() << "))";
    return retval.str();
  }

  PlexilType Division::getValueType() const
  {
    return REAL;
  }


  AllChildrenFinishedCondition::AllChildrenFinishedCondition(std::list<NodeId>& children)
    : Calculable(), m_listener(*this), m_total(0), m_count(0), m_constructed(false) {
    for(std::list<NodeId>::iterator it = children.begin(); it != children.end(); ++it) {
      NodeId child = *it;
      check_error(child.isValid());
      addChild(child);
    }
    internalSetValue(recalculate());
    m_constructed = true;
  }

  AllChildrenFinishedCondition::~AllChildrenFinishedCondition() {
    for(std::list<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      NodeId child = *it;
      child->getStateVariable()->removeListener(m_listener.getId());
    }
  }

  void AllChildrenFinishedCondition::addChild(const NodeId& node) {
    m_children.push_back(node);
    ++m_total;
    node->getStateVariable()->addListener(m_listener.getId());
    if(m_constructed) {
      if(node->getStateVariable()->getValue() == StateVariable::FINISHED())
	incrementCount(node->getStateVariable());
      else if(getValue() == BooleanVariable::TRUE())
	internalSetValue(BooleanVariable::FALSE());
    }
  }

  void AllChildrenFinishedCondition::incrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = Expression::UNKNOWN();
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
      m_lastValues[expr] = Expression::UNKNOWN();
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
    for(std::list<NodeId>::const_iterator it = m_children.begin(); it != m_children.end();
	++it) {
      NodeId child = *it;
      check_error(child.isValid());
      ExpressionId expr = child->getStateVariable();
      if(m_lastValues.find(expr) == m_lastValues.end())
	m_lastValues[expr] = Expression::UNKNOWN();
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
    std::stringstream retval;
    retval << Expression::toString();
    retval << "childrenFinished(" << m_count << ":" << m_total << "))";
    return retval.str();
  }

  /***************************/

  AllChildrenWaitingOrFinishedCondition::AllChildrenWaitingOrFinishedCondition(std::list<NodeId>& children)
    : Calculable(), m_listener(*this), m_total(0), m_count(0), m_constructed(false) {
    for(std::list<NodeId>::iterator it = children.begin(); it != children.end(); ++it) {
      NodeId child = *it;
      check_error(child.isValid());
      addChild(child);
    }
    internalSetValue(recalculate());
    m_constructed = true;
  }

  AllChildrenWaitingOrFinishedCondition::~AllChildrenWaitingOrFinishedCondition() {
    for(std::list<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      NodeId child = *it;
      child->getStateVariable()->removeListener(m_listener.getId());
    }
  }

  void AllChildrenWaitingOrFinishedCondition::addChild(const NodeId& node) {
    m_children.push_back(node);
    ++m_total;
    node->getStateVariable()->addListener(m_listener.getId());
    if(m_constructed) {
      if(node->getStateVariable()->getValue() == StateVariable::WAITING() ||
	 node->getStateVariable()->getValue() == StateVariable::FINISHED())
	incrementCount(node->getStateVariable());
      else if(getValue() == BooleanVariable::TRUE())
	internalSetValue(BooleanVariable::FALSE());
    }
  }

  void AllChildrenWaitingOrFinishedCondition::incrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = Expression::UNKNOWN();
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
      m_lastValues[expr] = Expression::UNKNOWN();
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
    for(std::list<NodeId>::const_iterator it = m_children.begin(); it != m_children.end();
	++it) {
      NodeId child = *it;
      check_error(child.isValid());
      ExpressionId expr = child->getStateVariable();
      if(m_lastValues.find(expr) == m_lastValues.end())
	m_lastValues[expr] = Expression::UNKNOWN();
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
    std::stringstream retval;
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

  InternalCondition::InternalCondition(const PlexilExprId& xml)
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
    std::stringstream retval;
    retval << Expression::toString();
    retval << m_expr->toString();
    retval << ")";
    return retval.str();
  }

  TimepointVariable::TimepointVariable(const PlexilExprId& xml) : ConstVariableWrapper() {
    checkError(ALWAYS_FAIL, "Timepoint references require a node argument.");
  }

  TimepointVariable::TimepointVariable(const PlexilExprId& expr, const NodeConnectorId& node)
    : ConstVariableWrapper() {
    checkError(Id<PlexilTimepointVar>::convertable(expr),
	       "Expected NodeTimepoint element, got " << expr->name());

    setWrapped(node->findVariable((PlexilVarRef*)expr));
  }

  //   class CoreExpressionsLocalStatic {
  //   public:
  //     CoreExpressionsLocalStatic() {
  //       static bool sl_called = false;
  //       if(!sl_called) {
  //  REGISTER_EXPRESSION(Conjunction, AND);
  //  REGISTER_EXPRESSION(Disjunction, OR);
  //  REGISTER_EXPRESSION(LogicalNegation, NOT);
  //  REGISTER_EXPRESSION(Equality, EQ);
  //  REGISTER_EXPRESSION(Equality, EQNumeric);
  //  REGISTER_EXPRESSION(Equality, EQBoolean);
  //  REGISTER_EXPRESSION(Inequality, NE);
  //  REGISTER_EXPRESSION(Inequality, NENumeric);
  //  REGISTER_EXPRESSION(Inequality, NEBoolean);
  //  REGISTER_EXPRESSION(LessThan, LT);
  //  REGISTER_EXPRESSION(LessEqual, LE);
  //  REGISTER_EXPRESSION(GreaterThan, GT);
  //  REGISTER_EXPRESSION(GreaterEqual, GE);
  //  REGISTER_EXPRESSION(Addition, ADD);
  //  REGISTER_EXPRESSION(Subtraction, SUB);
  //  REGISTER_EXPRESSION(Multiplication, MUL);
  //  REGISTER_EXPRESSION(Division, DIV);
  //  REGISTER_EXPRESSION(BooleanVariable, BooleanValue);
  //  REGISTER_EXPRESSION(IntegerVariable, IntegerValue);
  //  REGISTER_EXPRESSION(RealVariable, RealValue);
  //  REGISTER_EXPRESSION(StringVariable, StringValue);
  //  REGISTER_EXPRESSION(StateVariable, NodeStateValue);
  //  REGISTER_EXPRESSION(OutcomeVariable, NodeOutcomeValue);
  //  REGISTER_EXPRESSION(InternalCondition, EQInternal);
  //  REGISTER_EXPRESSION(InternalCondition, NEInternal);
  //  sl_called = true;
  //       }
  //     }
  //   };

  //   CoreExpressionsLocalStatic s_coreExpressions;
}
