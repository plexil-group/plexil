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

//
// Base class methods for Operator
//

#include "Operator.hh"

#include "PlanError.hh"
#include "PlexilTypeTraits.hh"

#include <vector>

namespace PLEXIL
{

  Operator::Operator(std::string const &name)
    : m_name(name)
  {
  }

  std::string const &Operator::getName() const
  {
    return m_name;
  }

  bool Operator::isPropagationSource() const
  {
    return false;
  }

  bool Operator::checkArgTypes(std::vector<ValueType> const & /* typeVec */) const
  {
    return true;
  }

  // Static helper for checkArgTypes()
  bool Operator::allSameTypeOrUnknown(ValueType typ,
                                      std::vector<ValueType> const &typeVec)
  {
    for (ValueType const &actual : typeVec) {
      if (actual != typ && actual != UNKNOWN_TYPE)
        return false;
    }
    return true;
  }

#define DEFINE_OPERATOR_DEFAULT_METHODS(_rtype_) \
  bool Operator::operator()(_rtype_ & /* result */, Expression const * /* arg */) const \
  { \
      reportPlanError(m_name << ": Attempt to get " \
                      << valueTypeName(PlexilValueType<_rtype_>::value) \
                      << " result from a "                              \
                      << valueTypeName(this->valueType())               \
                      << " expression");                                \
      return false; \
  } \
  bool Operator::operator()(_rtype_ & /* result */, \
                            Expression const * /* arg0 */, \
                            Expression const * /* arg1 */) const \
  { \
      reportPlanError(m_name << ": Attempt to get " \
                      << valueTypeName(PlexilValueType<_rtype_>::value) \
                      << " result from a "                              \
                      << valueTypeName(this->valueType())               \
                      << " expression");                                \
      return false; \
  } \
  bool Operator::operator()(_rtype_ & /* result */, Function const & /* args */) const \
  { \
      reportPlanError(m_name << ": Attempt to get " \
                      << valueTypeName(PlexilValueType<_rtype_>::value) \
                      << " result from a "                              \
                      << valueTypeName(this->valueType())               \
                      << " expression");                                \
      return false; \
  }

DEFINE_OPERATOR_DEFAULT_METHODS(Boolean)
DEFINE_OPERATOR_DEFAULT_METHODS(Integer)
DEFINE_OPERATOR_DEFAULT_METHODS(Real)
DEFINE_OPERATOR_DEFAULT_METHODS(String)

DEFINE_OPERATOR_DEFAULT_METHODS(NodeState)
DEFINE_OPERATOR_DEFAULT_METHODS(NodeOutcome)
DEFINE_OPERATOR_DEFAULT_METHODS(FailureType)
DEFINE_OPERATOR_DEFAULT_METHODS(CommandHandleValue)

DEFINE_OPERATOR_DEFAULT_METHODS(Array)
DEFINE_OPERATOR_DEFAULT_METHODS(BooleanArray)
DEFINE_OPERATOR_DEFAULT_METHODS(IntegerArray)
DEFINE_OPERATOR_DEFAULT_METHODS(RealArray)
DEFINE_OPERATOR_DEFAULT_METHODS(StringArray)

#undef DEFINE_OPERATOR_DEFAULT_METHODS

}
