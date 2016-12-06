/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#include "ArrayReference.hh"

#include "Array.hh"
#include "PlanError.hh"
#include "PlexilTypeTraits.hh"

namespace PLEXIL
{
  ArrayReference::ArrayReference(Expression *ary,
                                 Expression *idx,
                                 bool aryIsGarbage,
                                 bool idxIsGarbage)
    : NotifierImpl(),
      m_array(ary),
      m_index(idx),
      m_arrayIsGarbage(aryIsGarbage),
      m_indexIsGarbage(idxIsGarbage),
      m_namePtr(new std::string())
  {
    m_array->addListener(this);
    m_index->addListener(this);
  }

  ArrayReference::~ArrayReference()
  {
    m_array->removeListener(this);
    m_index->removeListener(this);
    if (m_arrayIsGarbage)
      delete m_array;
    if (m_indexIsGarbage)
      delete m_index;
    delete m_namePtr;
  }

  char const *ArrayReference::getName() const
  {
    Expression const *base = getBaseExpression();
    if (base) {
      std::ostringstream s;
      s << base->getName() << '[' << m_index->valueString() << ']';
      *m_namePtr = s.str();
      return m_namePtr->c_str();
    }
    static char const *sl_dummy = "";
    return sl_dummy;
  }

  char const *ArrayReference::exprName() const
  {
    return "ArrayReference";
  }

  ValueType ArrayReference::valueType() const
  {
    ValueType aryType = m_array->valueType();
    if (!isArrayType(aryType))
      return UNKNOWN_TYPE;
    else
      return arrayElementType(aryType);
  }

  bool ArrayReference::isKnown() const
  {
    Array const *dummyAry;
    size_t dummyIdx;
    return selfCheck(dummyAry, dummyIdx);
  }

  bool ArrayReference::isConstant() const
  {
    return m_array->isConstant() && m_index->isConstant();
  }

  bool ArrayReference::isAssignable() const
  {
    return false;
  }

  Expression const *ArrayReference::getBaseExpression() const
  {
    return m_array->getBaseExpression();
  }

  Expression *ArrayReference::getBaseExpression()
  {
    return m_array->getBaseExpression();
  }

  void ArrayReference::printValue(std::ostream &s) const
  {
    Array const *ary;
    size_t idx;
    if (!selfCheck(ary, idx)) {
      s << "UNKNOWN";
      return;
    }

    // Punt for now
    s << ary->getElementValue(idx);
  }

  bool ArrayReference::selfCheck(Array const *&valuePtr,
                                 size_t &idx) const
  {
    if (!(this->isActive() && m_array->isActive() && m_index->isActive()))
      return false;
    int32_t idxTemp;
    if (!m_index->getValue(idxTemp))
      return false; // index is unknown
    checkPlanError(idxTemp >= 0, "Array index " << idxTemp << " is negative");
    idx = (size_t) idxTemp;
    if (!m_array->getValuePointer(valuePtr))
      return false; // array unknown or invalid
    std::vector<bool> const &kv = valuePtr->getKnownVector();
    checkPlanError(idx < kv.size(),
                   "Array index " << idx
                   << " equals or exceeds array size " << kv.size());
    return kv[idx];
  }

  // Local macro
#define DEFINE_AREF_GET_VALUE_METHOD(_type_) \
  bool ArrayReference::getValue(_type_ &result) const \
  { \
    Array const *ary; \
    size_t idx; \
    if (!selfCheck(ary, idx)) \
      return false; \
    return ary->getElement(idx, result); \
  }

  // getValue explicit instantiations
  DEFINE_AREF_GET_VALUE_METHOD(Boolean)
  DEFINE_AREF_GET_VALUE_METHOD(Integer)
  DEFINE_AREF_GET_VALUE_METHOD(Real)
  DEFINE_AREF_GET_VALUE_METHOD(String)

#undef DEFINE_AREF_GET_VALUE_METHOD

  bool ArrayReference::getValue(uint16_t &result) const
  {
    checkPlanError(ALWAYS_FAIL,
                   "Array references not implemented for internal values");
    return false;
  }

  bool ArrayReference::getValuePointer(String const *&ptr) const
  {
    Array const *ary;
    size_t idx;
    if (!selfCheck(ary, idx))
      return false;
    return ary->getElementPointer(idx, ptr);
  }

  // Error for arrays and any other types we can think of.
#define DEFINE_AREF_GET_VALUE_PTR_ERROR_METHOD(_type_) \
  bool ArrayReference::getValuePointer(_type_ const *&ptr) const \
  { \
    assertTrueMsg(ALWAYS_FAIL, \
                  "getValuePointer: trying to get a " << PlexilValueType<_type_>::typeName \
                  << " pointer value from an ArrayReference"); \
    return false; \
  }

  DEFINE_AREF_GET_VALUE_PTR_ERROR_METHOD(Array)
  DEFINE_AREF_GET_VALUE_PTR_ERROR_METHOD(BooleanArray)
  DEFINE_AREF_GET_VALUE_PTR_ERROR_METHOD(IntegerArray)
  DEFINE_AREF_GET_VALUE_PTR_ERROR_METHOD(RealArray)
  DEFINE_AREF_GET_VALUE_PTR_ERROR_METHOD(StringArray)

#undef DEFINE_AREF_GET_VALUE_PTR_ERROR_METHOD

  Value ArrayReference::toValue() const
  {
    Array const *ary;
    size_t idx;
    if (!selfCheck(ary, idx))
      return Value(); // unknown
    else
      return ary->getElementValue(idx);
  }
  
  void ArrayReference::addListener(ExpressionListener *l)
  {
    if (!hasListeners()) {
      m_array->addListener(this);
      m_index->addListener(this);
    }
    NotifierImpl::addListener(l);
  }

  void ArrayReference::handleActivate()
  {
    m_array->activate();
    m_index->activate();
  }
  
  void ArrayReference::handleDeactivate()
  {
    m_array->deactivate();
    m_index->deactivate();
  }

  //
  // MutableArrayReference
  //

  MutableArrayReference::MutableArrayReference(Expression *ary,
                                               Expression *idx,
                                               bool aryIsGarbage,
                                               bool idxIsGarbage)
    : Assignable(),
      ArrayReference(ary, idx, aryIsGarbage, idxIsGarbage),
      m_mutableArray(ary->asAssignable()),
      m_saved(false)
  {
  }

  MutableArrayReference::~MutableArrayReference()
  {
  }

  bool MutableArrayReference::isAssignable() const
  {
    return true;
  }

  Assignable const *MutableArrayReference::asAssignable() const
  {
    return dynamic_cast<Assignable const *>(this);
  }

  Assignable *MutableArrayReference::asAssignable()
  {
    return dynamic_cast<Assignable *>(this);
  }

  bool MutableArrayReference::mutableSelfCheck(Array *&valuePtr,
                                               size_t &idx)
  {
    if (!(this->isActive()
          && ArrayReference::m_array->isActive()
          && ArrayReference::m_index->isActive()))
      return false;
    int32_t idxTemp;
    if (!ArrayReference::m_index->getValue(idxTemp))
      return false; // index is unknown
    checkPlanError(idxTemp >= 0,
                   "Array index " << idxTemp << " is negative");
    idx = (size_t) idxTemp;
    if (!m_mutableArray->getMutableValuePointer(valuePtr))
      return false; // array unknown
    checkPlanError(idx < valuePtr->size(),
                   "Array index " << idx
                   << " equals or exceeds array size " << valuePtr->size());
    return true;
  }

#define DEFINE_MAREF_SET_VALUE_METHOD(_type_) \
  void MutableArrayReference::setValue(_type_ const &val) \
  { \
    Array *ary; \
    size_t idx; \
    if (!mutableSelfCheck(ary, idx)) \
      return; \
    _type_ oldValue; \
    bool known = ary->getElement(idx, oldValue); /* error here if wrong type */ \
    bool changed = (!known || (val != oldValue)); \
    if (changed) { \
      ary->setElement(idx, val); \
      publishChange(this); \
    } \
  }

  // Instantiations of the above
  DEFINE_MAREF_SET_VALUE_METHOD(Boolean)
  DEFINE_MAREF_SET_VALUE_METHOD(Real)
  DEFINE_MAREF_SET_VALUE_METHOD(String)

#undef DEFINE_MAREF_SET_VALUE_METHOD

  // Specialized for Integer
  void MutableArrayReference::setValue(Integer const &value)
  {
    Array *ary;
    size_t idx;
    if (!mutableSelfCheck(ary, idx))
      return;
    // Check for case of assigning integer to real array
    bool changed = false;
    switch (this->m_array->valueType()) {
    case REAL_ARRAY_TYPE: {
      double oldValue;
      bool known = ary->getElement(idx, oldValue);
      double newValue = (double) value;
      if ((changed = (!known || (oldValue != newValue))))
        ary->setElement(idx, newValue);
    }
      break;
        
    case INTEGER_ARRAY_TYPE: {
      int32_t oldValue;
      bool known = ary->getElement(idx, oldValue);
      if ((changed = (!known || (oldValue != value))))
        ary->setElement(idx, value);
    }      
      break;

    default:
      checkPlanError(ALWAYS_FAIL,
                     "Can't assign an Integer value to element of a "
                     << valueTypeName(m_array->valueType()));
      return;
    }
    if (changed)
      publishChange(this);
  }

  void MutableArrayReference::setValue(Expression const &valex)
  {
    if (valex.isKnown())
      switch (valex.valueType()) {
      case BOOLEAN_TYPE: {
        Boolean b;
        valex.getValue(b);
        this->setValue(b);
      }
        break;
	
      case INTEGER_TYPE: {
        Integer i;
        valex.getValue(i);
        this->setValue(i);
      }
        break;

      case REAL_TYPE: {
        Real r;
        valex.getValue(r);
        this->setValue(r);
      }
        break;

      case STRING_TYPE: {
        String const *ptr;
        valex.getValuePointer(ptr);
        this->setValue(*ptr);
      }
        break;

      default:
        assertTrueMsg(ALWAYS_FAIL,
                      "ArrayReference:setValue: illegal or unimplemented type "
                      << valueTypeName(valex.valueType()));
        break;
      }
    else
      setUnknown();
  }

  void MutableArrayReference::setValue(Value const &value)
  {
    Array *ary;
    size_t idx;
    if (!mutableSelfCheck(ary, idx))
      return;
    Value oldValue = ary->getElementValue(idx);
    if (value != oldValue) {
      ary->setElementValue(idx, value);
      publishChange(this);
    }
  }

  void MutableArrayReference::setUnknown()
  {
    Array *ary;
    size_t idx;
    if (!mutableSelfCheck(ary, idx))
      return;
    bool changed = ary->elementKnown(idx);
    ary->setElementUnknown(idx);
    if (changed)
      publishChange(this);
  }

  bool MutableArrayReference::getMutableValuePointer(String *&ptr)
  {
    Array *ary;
    size_t idx;
    if (!mutableSelfCheck(ary, idx))
      return false;
    return ary->getMutableElementPointer(idx, ptr);
  }

  bool MutableArrayReference::getMutableValuePointer(Array *&ptr)
  {
    check_error_2(ALWAYS_FAIL, "MutableArrayReference::getMutableValuePointer: type error");
    return false;
  }

  void MutableArrayReference::reset()
  {
    // No-op
  }

  void MutableArrayReference::saveCurrentValue()
  {
    Array *ary;
    size_t idx;
    if (!mutableSelfCheck(ary, idx)) {
      // unknown or invalid
      m_saved = false;
      return;
    }
    m_savedValue = ary->getElementValue(idx);
    m_saved = true;
  }

  void MutableArrayReference::restoreSavedValue()
  {
    Array *ary;
    size_t idx;
    if (!mutableSelfCheck(ary, idx) || !m_saved) 
      return;
    if (m_savedValue != ary->getElementValue(idx)) {
      ary->setElementValue(idx, m_savedValue);
      publishChange(this);
    }
    m_saved = false;
  }

  Value MutableArrayReference::getSavedValue() const
  {
    return Value(m_savedValue);
  }

  NodeConnector const *MutableArrayReference::getNode() const
  {
    return getBaseVariable()->asAssignable()->getNode();
  }

  NodeConnector *MutableArrayReference::getNode()
  {
    return getBaseVariable()->asAssignable()->getNode();
  }

  Expression *MutableArrayReference::getBaseVariable() 
  {
    return m_mutableArray->getBaseVariable();
  }

  Expression const *MutableArrayReference::getBaseVariable() const
  {
    return m_mutableArray->getBaseVariable();
  }

  void MutableArrayReference::publishChange(Expression const *src)
  {
    NotifierImpl::publishChange(src);
    m_array->notifyChanged(src);
  }

} // namespace PLEXIL

