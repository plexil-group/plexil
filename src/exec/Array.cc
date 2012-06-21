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

#include "Array.hh"
#include "StoredArray.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExecListenerHub.hh"
#include "ExpressionFactory.hh"
#include "NodeConnector.hh"

#include <cstring> // for strcmp()

namespace PLEXIL
{

  //
  // ArrayVariableBase
  //

  ArrayVariableBase::ArrayVariableBase()
	: Variable(),
	  m_avid(this, Variable::getId())
  {
  }

  ArrayVariableBase::~ArrayVariableBase()
  {
	m_avid.removeDerived(Variable::getId());
  }

  //
  // ArrayAliasVariable
  //

  ArrayAliasVariable::ArrayAliasVariable(const std::string& name,
										 const NodeConnectorId& nodeConnector,
										 const ExpressionId& exp,
										 bool expIsGarbage,
										 bool isConst)
	: ArrayVariableBase(),
	  AliasVariable(name, nodeConnector, exp, expIsGarbage, isConst),
	  m_originalArray((ArrayVariableId) exp)
  {
	// Check original, node for validity
	assertTrueMsg(m_originalArray.isId(),
				  "Invalid array passed to ArrayAliasVariable constructor");
	assertTrue(nodeConnector.isValid(),
			   "Invalid node connector ID passed to AliasVariable constructor");
  }

  ArrayAliasVariable::~ArrayAliasVariable()
  {
	assertTrue(m_originalArray.isValid(),
			   "Original expression ID invalid in AliasVariable destructor");
  }

  /**
   * @brief Get a string representation of this Expression.
   * @return The string representation.
   */
  void ArrayAliasVariable::print(std::ostream& s) const
  {
	s << getName() << " ";
	Expression::print(s);
	s << (isConst() ? "const " : "") 
	  << "ArrayAliasVariable for "
	  << *m_originalArray
	  << ")";
  }

  PlexilType ArrayAliasVariable::getValueType() const
  {
	return m_originalArray->getValueType();
  }

  unsigned long ArrayAliasVariable::maxSize() const
  {
	return m_originalArray->maxSize();
  }

  double ArrayAliasVariable::lookupValue(unsigned long index) const
  {
	return m_originalArray->lookupValue(index);
  }

  void ArrayAliasVariable::setElementValue(unsigned /* index */, const double /* value */)
  {
	assertTrueMsg(!isConst(),
				  "Attempt to call setElementValue() on const array alias " << *this);
  }

  PlexilType ArrayAliasVariable::getElementType() const
  {
	return m_originalArray->getElementType();
  }

  bool ArrayAliasVariable::checkElementValue(const double val)
  {
	return m_originalArray->checkElementValue(val);
  }

  ArrayVariable::ArrayVariable(unsigned long maxSize,
							   PlexilType type,
                               const bool isConst)
    : ArrayVariableBase(), 
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
    : ArrayVariableBase(),
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
    : ArrayVariableBase(),
	  VariableImpl(expr, node, isConst)
  {
	assertTrueMsg(expr.isValid(), "Attempt to create an ArrayVariable from an invalid Id");

    debugMsg("ArrayVariable", " constructor from intermediate representation");

    // confirm that we have a an array variable
	const PlexilArrayValue* arrayValue = NULL;
    if (Id<PlexilArrayVar>::convertable(expr)) {
	  const Id<PlexilArrayVar> var = (const Id<PlexilArrayVar>) expr;
	  m_type = var->elementType();
	  m_maxSize = var->maxSize();
	  if (var->value() != NULL) {
		arrayValue = dynamic_cast<const PlexilArrayValue*>(var->value());
		assertTrueMsg(arrayValue != NULL,
					  "Array variable initial value is not a PlexilArrayValue");
	  }
	}
    else if (Id<PlexilArrayValue>::convertable(expr)) {
	  arrayValue = (const PlexilArrayValue*) expr;
	  m_type = arrayValue->type();
	  m_maxSize = arrayValue->maxSize();
	}
	else {
	  assertTrueMsg(ALWAYS_FAIL, "Expected a PlexilArrayVar or PlexilArrayValue");
	}

    // init the local type and array
    StoredArray array(m_maxSize, UNKNOWN());
    setValue(array.getKey());

	if (arrayValue != NULL) {
	  // convert strings to doubles for internal storage
	  const std::vector<std::string>& values = arrayValue->values();
	  for (std::vector<std::string>::const_iterator value = values.begin();
		   value != values.end(); ++value) {
		double convertedValue;
		if (m_type == STRING)
		  convertedValue = LabelStr(*value).getKey();
		else if (m_type == BOOLEAN) {
		  if (compareIgnoreCase(*value, "true") || 
			  (strcmp(value->c_str(), "1") == 0))
			convertedValue = 1;
		  else if (compareIgnoreCase(*value, "false") || 
				   (strcmp(value->c_str(), "0") == 0))
			convertedValue = 0;
		  else
			checkError(false, 
					   "Attempt to initialize Boolean array variable with invalid value \"" << *value << "\"");
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
      for (size_t i = 0; i < myArray.size(); ++i)
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
        for (size_t i = 0; i < sourceArray.size(); ++i)
          newArray[i] = sourceArray[i];

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
         value != values.end(); ++value) {
        checkError(checkElementValue(*value),
                   "Attempted to set element of " << PlexilParser::valueTypeString(getElementType())
				   << " array variable to invalid value \"" << valueToString(*value) << "\"");
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
      
    while (index < source.size()) {
	  double value = source[index];
	  checkError(checkElementValue(value),
				 "Attempted to set element of " << PlexilParser::valueTypeString(getElementType())
				 << " array variable to invalid value \"" << valueToString(value) << "\"");
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

  // set an element value in an array variable
  void ArrayVariable::setElementValue(unsigned index, const double value)
  {
    // lotsa potential errors to check
    checkError(!VariableImpl::isConst(),
               "Attempted to set element value " << value << " of const array " << *this);
    checkError(checkElementValue(value),
               "Attempted to set element of " << PlexilParser::valueTypeString(getElementType())
			   << " array variable to invalid value \"" << valueToString(value) << "\"");
    checkError(checkIndex(index),
               "Array index " << index << " exceeds bound of " 
               << m_maxSize);
    checkError(m_value != UNKNOWN(),
               "Attempted to assign an array element in an UNKNOWN array");

    // set the element
    StoredArray theArray(m_value);
	if (value != theArray[index]) {
	  theArray[index] = value;
	  publishChange();
	}
	ExecListenerHubId hub = getExecListenerHub();
	if (hub.isId()) {
	  std::ostringstream s;
	  s << m_name << '[' << index << ']'; // FIXME: this is unlikely to be right
	  hub->notifyOfAssignment(Expression::getId(), s.str(), value);
	}
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

  void ArrayVariable::print(std::ostream& s) const 
  {
	VariableImpl::print(s);
	s << "array)"; // contents already printed!
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
        return val == UNKNOWN() || LabelStr::isString(val);
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
    : Variable(),
	  m_node(node.isId() ? node->getNode() : NodeId::noId()),
      m_deleteIndex(false),
      m_listener(getId()),
      m_name()
  {
    // confirm that we have an array element
    checkError(Id<PlexilArrayElement>::convertable(expr),
               "Expected an array element.");
    PlexilArrayElement* arrayElement = (PlexilArrayElement*) expr;
    m_name = arrayElement->getArrayName();
    debugMsg("ArrayElement:ArrayElement", " name = " << m_name);

    // initialize array variable
    PlexilVarRef arrayRef;
    arrayRef.setName(m_name);
    VariableId arrayVar = node->findVariable(&arrayRef);
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
	check_error(m_index.isValid());
	m_index->removeListener(m_listener.getId());
	if (m_deleteIndex)
	  delete (Expression*) m_index;
  }

  // confirm that new value to assign is valid

  bool ArrayElement::checkValue(const double val)
  {
    return m_arrayVariable->checkElementValue(val);
  }

  void ArrayElement::print(std::ostream& s) const
  {
	Expression::print(s);
	s << "ArrayElement: " << *m_arrayVariable // *** FIXME: this is probably too verbose ***
	  << "[" << *m_index << "])";
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

}
