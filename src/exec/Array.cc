/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

  size_t ArrayAliasVariable::maxSize() const
  {
    return m_originalArray->maxSize();
  }

  double ArrayAliasVariable::lookupValue(size_t index) const
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

  ArrayVariable::ArrayVariable(size_t maxSize,
                               PlexilType type,
                               const bool isConst)
    : ArrayVariableBase(), 
      VariableImpl(false), // set m_isConst below
      m_array(maxSize, UNKNOWN()),
      m_initialArray(m_array),
      m_maxSize(maxSize), 
      m_type(type)
  {
    debugMsg("ArrayVariable", " constructor, no initial elements");
    m_initialValue = m_array.getKey();
    setValue(m_array.getKey());
    if (isConst)
      makeConst();
  }

  ArrayVariable::ArrayVariable(size_t maxSize, 
                               PlexilType type, 
                               const std::vector<double>& values, 
                               const bool isConst)
    : ArrayVariableBase(),
      VariableImpl(false),
      m_array(m_maxSize, UNKNOWN()),
      m_initialArray(m_array),
      m_maxSize(maxSize),
      m_type(type)
  {
    assertTrueMsg(values.size() <= maxSize,
                  "ArrayVariable constructor: Initial array size " << values.size()
                  << " exceeds target size " << maxSize);
    debugMsg("ArrayVariable", " constructor, " << values.size() << " initial elements");
    m_initialValue = m_array.getKey();
    // Let StringArrayVariable do its own initialization
    if (m_type != STRING) {
      // Set array values
      for (size_t i = 0; i < values.size(); ++i) {
        checkError(checkElementValue(values[i]),
                   "Attempted to initialize element of " << PlexilParser::valueTypeString(getElementType())
                   << " array to invalid value \"" << valueToString(values[i]) << "\"");
        m_array[i] = values[i];
      }
      setValue(m_array.getKey());
      if (isConst)
        makeConst();
    }
  }

  ArrayVariable::ArrayVariable(const PlexilExprId& expr, 
                               const NodeConnectorId& node,
                               const bool isConst)
    : ArrayVariableBase(),
      VariableImpl(expr, node),
      m_array(),
      m_initialArray()
  {
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
      assertTrueMsg(ALWAYS_FAIL,
                    "ArrayVariable constructor: Expected a PlexilArrayVar or PlexilArrayValue");
    }

    // init the local type and array
    m_initialArray = m_array = StoredArray(m_maxSize, UNKNOWN());
    m_initialValue = m_array.getKey();

    // Defer to StringArrayVariable constructor
    if (m_type != STRING) {
      if (arrayValue != NULL) {
        const std::vector<std::string>& values = arrayValue->values();
        assertTrueMsg(values.size() <= m_maxSize,
                      "ArrayVariable constructor: Number of initial values, "
                      << values.size() << ", exceeds max size, " << m_maxSize);
        for (size_t i = 0; i < values.size(); ++i) {
          double convertedValue;
          if (m_type == BOOLEAN) {
            if (compareIgnoreCase(values[i], "true") || 
                (strcmp(values[i].c_str(), "1") == 0))
              convertedValue = 1;
            else if (compareIgnoreCase(values[i], "false") || 
                     (strcmp(values[i].c_str(), "0") == 0))
              convertedValue = 0;
            else
              assertTrueMsg(ALWAYS_FAIL, 
                            "Attempt to initialize Boolean array variable with invalid value \""
                            << values[i] << "\"");
          }
          else {
            std::istringstream valueStream(values[i]);
            valueStream >> convertedValue;
            checkError(checkElementValue(convertedValue),
                       "Attempted to initialize element of " << PlexilParser::valueTypeString(getElementType())
                       << " array to invalid value \"" << values[i] << "\"");
          }
          m_array[i] = convertedValue;
        }
      }
      VariableImpl::setValue(m_array.getKey());
      if (isConst)
        makeConst();
    }
  }

  // 
  // Destructor
  //

  ArrayVariable::~ArrayVariable()
  {
  }


  /**
   * @brief Set the value of this expression back to the initial value with which it was
   *        created.
   */
  void ArrayVariable::reset()
  {
    m_array = m_initialArray;
    VariableImpl::setValue(m_initialValue);
  }

  // set the value of this array
  void ArrayVariable::setValue(const double value)
  {
    // Check if new array value == current
    if (m_value == value
        || StoredArray::isKey(value) && m_array.getArray() == StoredArray::getArray(value)) {
      debugMsg("ArrayVariable:setValue", " to existing value");
      return;
    }

    // Check if new == initial (e.g. Variable::reset() or retracting an assignment)
    if (value == m_initialValue
        || StoredArray::isKey(value) && m_initialArray == StoredArray::getArray(value)) {
      debugMsg("ArrayVariable:setValue", " to initial value");
      m_array = m_initialArray;
      VariableImpl::setValue(value);
      return;
    }

    // Check new value
    if (value != UNKNOWN()) {
      assertTrueMsg(StoredArray::isKey(value),
                    "ArrayVariable::setValue: new value " << Expression::valueToString(value)
                    << " is not an array value or UNKNOWN");
      assertTrueMsg(StoredArray::getArray(value).size() <= m_maxSize,
                    "ArrayVariable::setValue: new value size, "
                    << StoredArray::getArray(value).size()
                    << ", is larger than the maximum size, " << m_maxSize);
    }

    // Below this line, we will be replacing contents of the current array
    if (m_value == m_initialValue) {
      debugMsg("ArrayVariable:setValue", " allocating new array");
      // Allocate a new array so as not to disturb the initial value
      m_array = StoredArray(m_maxSize, UNKNOWN());
      if (value == UNKNOWN()) {
        debugMsg("ArrayVariable:setValue", " to UNKNOWN()");
        VariableImpl::setValue(value);
        return;
      }
    }

    // if the new value is unknown, then clear the array contents
    if (value == UNKNOWN()) {
      debugMsg("ArrayVariable:setValue", " to UNKNOWN()");
      // clear array contents
      m_array.getArray().assign(m_array.size(), UNKNOWN());
      VariableImpl::setValue(value);
      return;
    }

    debugMsg("ArrayVariable:setValue", " general case");
    // copy the array
    const std::vector<double>& sourceArray = StoredArray::getArray(value);
    size_t i = 0;
    for (; i < sourceArray.size(); ++i)
      m_array[i] = sourceArray[i];
    while (i < m_maxSize)
      m_array[i++] = UNKNOWN();

    // VariableImpl::setValue(m_array.getKey());
    // FIXME: This is a kludge to ensure listeners are notified.
    // As we have copied in place, the array key may not have changed,
    // so Expression::internalSetValue() may not notify listeners.
    // This kludge doesn't respect the Expression class lock.
    m_value = m_array.getKey();
    publishChange();
  }

  /**
   * @brief Temporarily stores the previous value of this variable.
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void ArrayVariable::saveCurrentValue()
  {
    VariableImpl::saveCurrentValue();
    m_savedArray = m_array.getArray();
  }

  /**
   * @brief Restore the value set aside by saveCurrentValue().
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void ArrayVariable::restoreSavedValue()
  {
    // Copy the array contents back from the saved array
    std::vector<double>& ary = m_array.getArray();
    for (size_t i = 0; i < m_maxSize; ++i) {
      ary[i] = m_savedArray[i];
    }
    VariableImpl::restoreSavedValue();
  }
     
  /**
   * @brief Commit the assignment by erasing the saved previous value.
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void ArrayVariable::commitAssignment()
  {
    m_savedArray.clear();
    VariableImpl::commitAssignment();
  }

  // set an element value in an array variable

  void ArrayVariable::setElementValue(unsigned index, const double value)
  {
    // lotsa potential errors to check
    assertTrueMsg(!VariableImpl::isConst(),
               "Attempted to set element value " << value << " of const array " << *this);
    assertTrueMsg(m_value != UNKNOWN(),
               "Attempted to assign an array element in an UNKNOWN array");
    assertTrueMsg(checkElementValue(value),
               "Attempted to set element of " << PlexilParser::valueTypeString(getElementType())
               << " array variable to invalid value \"" << valueToString(value) << "\"");
    assertTrueMsg(checkIndex(index),
               "Array index " << index << " exceeds bound of " 
               << m_maxSize);

    // set the element
    if (value != m_array[index]) {
      bool newArray = false;
      // Implement copy-on-write semantics to avoid clobbering initial value
      if (m_array.getKey() == m_initialArray.getKey()) {
        // Clone the initial array
        m_array = StoredArray(m_initialArray.getArray());
        newArray = true;
      }

      m_array[index] = value;
      if (newArray)
        VariableImpl::setValue(m_array.getKey());
      else
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
  double ArrayVariable::lookupValue(size_t index) const
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
    if (StoredArray::isKey(val)) {
      StoredArray valArray(val);
      if (valArray.size() > m_maxSize)
        return false;
      for (size_t i = 0; i < valArray.size(); i++) {
        if (!checkElementValue(valArray[i]))
          return false;
      }
      return true;
    }
    return false;
  }

  // confirm that array element is valid

  bool ArrayVariable::checkElementValue(const double val)
  {
    // check value based on array type
    switch (m_type) {
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
      return LabelStr::isString(val) || val == UNKNOWN();
    case ARRAY:
      checkError(ALWAYS_FAIL, "Arrays of arrays not supported.");
    case TIME:
      checkError(ALWAYS_FAIL, "TimePoints not supported in arrays.");
    default:
      checkError(ALWAYS_FAIL, "Unknown variable type: " << m_type);
    }
    // should never get here

    return false;
  }
   
  //
  // StringArrayVariable
  //

  StringArrayVariable::StringArrayVariable(size_t maxSize,
                                           PlexilType type,
                                           const bool isConst)
    : ArrayVariable(maxSize, type),
      m_labels(maxSize),
      m_initialLabels()
  {
    debugMsg("StringArrayVariable", " constructor, no initial elements");
    checkError(type == STRING,
               "StringArrayVariable constructor: type is not STRING");
    if (isConst)
      makeConst();
  }

  StringArrayVariable::StringArrayVariable(size_t maxSize, 
                                           PlexilType type, 
                                           const std::vector<double>& values,
                                           const bool isConst)
    : ArrayVariable(maxSize, type, values, false),
      m_labels(maxSize),
      m_initialLabels(values.size())
  {
    debugMsg("StringArrayVariable", " constructor, " << values.size() << " initial elements");
    checkError(type == STRING,
               "StringArrayVariable constructor: type is not STRING");
    for (size_t i = 0; i < values.size(); ++i) {
      checkError(checkElementValue(values[i]),
                 "Attempted to initialize element of " << PlexilParser::valueTypeString(getElementType())
                 << " array to invalid value \"" << valueToString(values[i]) << "\"");
      if (values[i] != UNKNOWN()) {
        m_initialLabels[i] = m_labels[i] = m_array[i] = values[i];
      }
    }
    setValue(m_array.getKey());
    if (isConst)
      makeConst();
  }

  StringArrayVariable::StringArrayVariable(const PlexilExprId& expr, 
                                           const NodeConnectorId& node,
                                           const bool isConst)
    : ArrayVariable(expr, node, false),
      m_labels(m_maxSize), // set in ArrayVariable constructor
      m_initialLabels()
  {
    debugMsg("StringArrayVariable", " constructor from intermediate representation");
    checkError(m_type == STRING,
               "StringArrayVariable constructor: type is not STRING");
    const PlexilArrayValue* arrayValue = NULL;
    if (Id<PlexilArrayVar>::convertable(expr)) {
      const Id<PlexilArrayVar> var = (const Id<PlexilArrayVar>) expr;
      if (var->value() != NULL) {
        arrayValue = dynamic_cast<const PlexilArrayValue*>(var->value());
        // should have been caught in ArrayVariable constructor
        assertTrueMsg(arrayValue != NULL,
                      "Array variable initial value is not a PlexilArrayValue");
      }
    }
    else if (Id<PlexilArrayValue>::convertable(expr)) {
      arrayValue = (const PlexilArrayValue*) expr;
    }
    else {
      // should have been caught in ArrayVariable constructor
      assertTrueMsg(ALWAYS_FAIL,
                    "StringArrayVariable constructor: Expected a PlexilArrayVar or PlexilArrayValue");
    }

    if (arrayValue != NULL) {
      // Cache initial values
      const std::vector<std::string>& values = arrayValue->values();
      assertTrueMsg(values.size() <= maxSize(),
                    "StringArrayVariable constructor: Number of initial values, "
                    << values.size() << ", exceeds max size, " << maxSize());
      m_initialLabels.resize(values.size());
      for (size_t i = 0; i < values.size(); ++i)
        m_labels[i] = m_initialLabels[i] = m_array[i] = LabelStr(values[i]).getKey();
    }
    VariableImpl::setValue(m_array.getKey());
    if (isConst)
      makeConst();
  }

  StringArrayVariable::~StringArrayVariable()
  {
  }

  /**
   * @brief Set the contents of this array from the given value.
   * @note Value must be an array or UNKNOWN.
   */
  void StringArrayVariable::setValue(const double value)
  {
    ArrayVariable::setValue(value);
    if (value == UNKNOWN()) {
      // Clear labels
      for (size_t i = 0; i < m_maxSize; ++i)
        m_labels[i] = EMPTY_LABEL();
    }
    else {
      // Copy labels
      for (size_t i = 0; i < m_maxSize; ++i)
        m_labels[i] =
          (LabelStr::isString(m_array[i]) ? m_array[i] : EMPTY_LABEL().getKey());
    }
  }

  /**
   * @brief Temporarily stores the previous value of this variable.
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void StringArrayVariable::saveCurrentValue()
  {
    ArrayVariable::saveCurrentValue();
    m_savedLabels = m_labels;
  }

  /**
   * @brief Restore the value set aside by saveCurrentValue().
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void StringArrayVariable::restoreSavedValue()
  {
    m_labels = m_savedLabels;
    ArrayVariable::restoreSavedValue();
  }
     
  /**
   * @brief Commit the assignment by erasing the saved previous value.
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void StringArrayVariable::commitAssignment()
  {
    m_savedLabels.clear();
    ArrayVariable::commitAssignment();
  }
    
  /**
   * @brief Set the value of this expression back to the initial value with which it was
   *        created.
   */
  void StringArrayVariable::reset()
  {
    ArrayVariable::reset();
    size_t i = 0;
    for (; i < m_initialLabels.size(); ++i)
      m_labels[i] = m_initialLabels[i];
    for (; i < m_maxSize; ++i)
      m_labels[i] = EMPTY_LABEL();
  }

  /**
   * @brief Set one element of this array from the given value.
   * @note Value must be an array or UNKNOWN.
   * @note Index must be less than maximum length
   */
  void StringArrayVariable::setElementValue(unsigned index, const double value)
  {
    ArrayVariable::setElementValue(index, value);
    m_labels[index] = 
      (value == UNKNOWN() 
       ? EMPTY_LABEL() 
       : LabelStr(value)); // can assert if value not a LabelStr key
  }

  /**
   * @brief Check to make sure an element value is appropriate for this array.
   */
  bool StringArrayVariable::checkElementValue(const double val)
  {
    return LabelStr::isString(val) || val == UNKNOWN();
  }

  //
  // Array elements
  //

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
    arrayRef.setName(arrayElement->getArrayName());
    VariableId arrayVar = node->findVariable(&arrayRef);
    checkError(ArrayVariableId::convertable(arrayVar),
               "Expected Array Variable but found: " << 
               arrayVar->toString());
    m_arrayVariable = (ArrayVariableId) arrayVar;    
    m_arrayVariable->addListener(m_listener.getId());

    // initialize index expression
    const std::vector<PlexilExprId>& subExprs = arrayElement->subExprs();
    // *** update this if we ever support n-dimensional arrays ***
    checkError(subExprs.size() == 1, 
               subExprs.size()
               << " is an invalid number of index subexpressions to array element");
    PlexilExprId indexExpr = subExprs.front();
    m_index = ExpressionFactory::createInstance(LabelStr(indexExpr->name()),
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

  // FIXME: is this the right approach?
  void ArrayElement::reset()
  {}

  void ArrayElement::setValue(const double value)
  {
    // delegate to the array
    m_arrayVariable->setElementValue((unsigned) m_index->getValue(),
                                     value);
    internalSetValue(value);
  }

  /**
   * @brief Temporarily stores the previous value of this variable.
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void ArrayElement::saveCurrentValue()
  {
    m_savedValue = recalculate();
    if (LabelStr::isString(m_savedValue))
      m_savedString = m_savedValue;
  }

  /**
   * @brief Restore the value set aside by saveCurrentValue().
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void ArrayElement::restoreSavedValue()
  {
    setValue(m_savedValue);
  }
     
  /**
   * @brief Commit the assignment by erasing the saved previous value.
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void ArrayElement::commitAssignment()
  {
    m_savedValue = UNKNOWN();
    m_savedString = EMPTY_LABEL();
  }

  PlexilType ArrayElement::getValueType() const
  {
    return m_arrayVariable->getElementType();
  }

  /**
   * @brief Notify this expression that a subexpression's value has changed.
   * @param exp The changed subexpression.
   */

  // FIXME: should index range check happen here?
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

  // FIXME: should index range check happen here?
  double ArrayElement::recalculate()
  {
    double index = m_index->getValue();
    if (index == UNKNOWN())
      return UNKNOWN();

    return m_arrayVariable->lookupValue((size_t) index);
  }

}
