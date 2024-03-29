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

#include "CachedValue.hh"

#include "InterfaceError.hh"
#include "PlexilTypeTraits.hh"

namespace PLEXIL
{
  CachedValue::CachedValue()
    : m_timestamp(0)
  {
  }

  CachedValue::CachedValue(CachedValue const &orig)
    : m_timestamp(orig.m_timestamp)
  {
  }

  CachedValue &CachedValue::operator=(CachedValue const &other)
  {
    m_timestamp = other.m_timestamp;
    return *this;
  }

  unsigned int CachedValue::getTimestamp() const
  {
    return m_timestamp;
  }

#define DEFINE_DEFAULT_GET_VALUE_METHOD(_rtype_) \
  bool CachedValue::getValue(_rtype_ & /* result */) const   \
  { \
    errorMsg("getValue: trying to get a " << PlexilValueType<_rtype_>::typeName \
             << " value from a " << valueTypeName(this->valueType()) << " typed object"); \
    return false; \
  }

  DEFINE_DEFAULT_GET_VALUE_METHOD(Boolean)
  DEFINE_DEFAULT_GET_VALUE_METHOD(Integer)
  DEFINE_DEFAULT_GET_VALUE_METHOD(Real)
  DEFINE_DEFAULT_GET_VALUE_METHOD(String)

#undef DEFINE_DEFAULT_GET_VALUE_METHOD

#define DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(_rtype_) \
  bool CachedValue::getValuePointer(_rtype_ const *& /* ptr */) const   \
  { \
    errorMsg("getValuePointer: trying to get a " << PlexilValueType<_rtype_>::typeName \
             << " pointer value from a " << valueTypeName(this->valueType()) << " typed object"); \
    return false; \
  }

  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(String)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(Array)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(BooleanArray)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(IntegerArray)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(RealArray)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(StringArray)

#undef DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD

#define DEFINE_DEFAULT_UPDATE_METHOD(_type_) \
  bool CachedValue::update(unsigned int timestamp, _type_ const & /* val */) \
  { \
    reportInterfaceError("Attempt to update a " << valueTypeName(this->valueType()) \
                         << " CachedValue with a " << PlexilValueType<_type_>::typeName); \
    return false; \
  }

  DEFINE_DEFAULT_UPDATE_METHOD(Boolean)
  DEFINE_DEFAULT_UPDATE_METHOD(Integer)
  DEFINE_DEFAULT_UPDATE_METHOD(Real)
  DEFINE_DEFAULT_UPDATE_METHOD(String)

#undef DEFINE_DEFAULT_UPDATE_METHOD

#define DEFINE_DEFAULT_UPDATE_PTR_METHOD(_type_) \
  bool CachedValue::updatePtr(unsigned int /* timestamp */, _type_ const * /* ptr */) \
  { \
    reportInterfaceError("Attempt to update a " << valueTypeName(this->valueType()) \
                         << " CachedValue with a " << PlexilValueType<_type_>::typeName); \
    return false; \
  } 

  DEFINE_DEFAULT_UPDATE_PTR_METHOD(String)
  DEFINE_DEFAULT_UPDATE_PTR_METHOD(BooleanArray)
  DEFINE_DEFAULT_UPDATE_PTR_METHOD(IntegerArray)
  DEFINE_DEFAULT_UPDATE_PTR_METHOD(RealArray)
  DEFINE_DEFAULT_UPDATE_PTR_METHOD(StringArray)

#undef DEFINE_DEFAULT_UPDATE_PTR_METHOD
  
} // namespace PLEXIL
