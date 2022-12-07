// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
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

#ifndef PLEXIL_TYPE_TRAITS_HH
#define PLEXIL_TYPE_TRAITS_HH

#include "ValueType.hh"

namespace PLEXIL
{

  ///@{
  //! \struct PlexilValueType
  //! \brief Template representing traits of a particular PLEXIL ValueType.
  //! \ingroup Values
  template <typename T>
  struct PlexilValueType
  {
    static constexpr ValueType value = UNKNOWN_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = UNKNOWN_STR;
    static constexpr bool isPlexilType = false;
  };

  template <>
  struct PlexilValueType<Boolean>
  {
    static constexpr ValueType value = BOOLEAN_TYPE;
    static constexpr ValueType arrayValue = BOOLEAN_ARRAY_TYPE;
    static constexpr char const *typeName = BOOLEAN_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<NodeState>
  {
    static constexpr ValueType value = NODE_STATE_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = NODE_STATE_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<NodeOutcome>
  {
    static constexpr ValueType value = OUTCOME_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = NODE_OUTCOME_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<FailureType>
  {
    static constexpr ValueType value = FAILURE_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = NODE_FAILURE_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<CommandHandleValue>
  {
    static constexpr ValueType value = COMMAND_HANDLE_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = NODE_COMMAND_HANDLE_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<Integer>
  {
    static constexpr ValueType value = INTEGER_TYPE;
    static constexpr ValueType arrayValue = INTEGER_ARRAY_TYPE;
    static constexpr char const *typeName = INTEGER_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<Real>
  {
    static constexpr ValueType value = REAL_TYPE;
    static constexpr ValueType arrayValue = REAL_ARRAY_TYPE;
    static constexpr char const *typeName = REAL_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<String>
  {
    static constexpr ValueType value = STRING_TYPE;
    static constexpr ValueType arrayValue = STRING_ARRAY_TYPE;
    static constexpr char const *typeName = STRING_STR;
    static constexpr bool isPlexilType = true;
  };

  // *** Not sure about this ***
  template <>
  struct PlexilValueType<Array>
  {
    static constexpr ValueType value = ARRAY_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = ARRAY_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<BooleanArray>
  {
    static constexpr ValueType value = BOOLEAN_ARRAY_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = BOOLEAN_ARRAY_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<IntegerArray>
  {
    static constexpr ValueType value = INTEGER_ARRAY_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = INTEGER_ARRAY_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<RealArray>
  {
    static constexpr ValueType value = REAL_ARRAY_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = REAL_ARRAY_STR;
    static constexpr bool isPlexilType = true;
  };

  template <>
  struct PlexilValueType<StringArray>
  {
    static constexpr ValueType value = STRING_ARRAY_TYPE;
    static constexpr ValueType arrayValue = UNKNOWN_TYPE;
    static constexpr char const *typeName = STRING_ARRAY_STR;
    static constexpr bool isPlexilType = true;
  };
  ///@}

} // namespace PLEXIL

#endif // PLEXIL_TYPE_TRAITS_HH
