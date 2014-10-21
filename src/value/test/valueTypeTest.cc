/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "TestSupport.hh"
#include "ValueType.hh"

#include <cstring>

using namespace PLEXIL;

// String constants for parser testing
static char const *mt = "";
static char const *bogus = "bOgUs";

static bool parseValueTypeTest()
{
  // Test bogus input
  assertTrue_1(parseValueType(NULL) == UNKNOWN_TYPE);
  assertTrue_1(parseValueType(mt) == UNKNOWN_TYPE);
  assertTrue_1(parseValueType(std::string()) == UNKNOWN_TYPE);
  assertTrue_1(parseValueType(bogus) == UNKNOWN_TYPE);
  assertTrue_1(parseValueType(std::string(bogus)) == UNKNOWN_TYPE);

  assertTrue_1(parseValueType(BOOLEAN_STR) == BOOLEAN_TYPE);
  assertTrue_1(parseValueType(std::string(BOOLEAN_STR)) == BOOLEAN_TYPE);

  assertTrue_1(parseValueType(INTEGER_STR) == INTEGER_TYPE);
  assertTrue_1(parseValueType(std::string(INTEGER_STR)) == INTEGER_TYPE);

  assertTrue_1(parseValueType(REAL_STR) == REAL_TYPE);
  assertTrue_1(parseValueType(std::string(REAL_STR)) == REAL_TYPE);

  assertTrue_1(parseValueType(STRING_STR) == STRING_TYPE);
  assertTrue_1(parseValueType(std::string(STRING_STR)) == STRING_TYPE);

  assertTrue_1(parseValueType(DATE_STR) == DATE_TYPE);
  assertTrue_1(parseValueType(std::string(DATE_STR)) == DATE_TYPE);

  assertTrue_1(parseValueType(DURATION_STR) == DURATION_TYPE);
  assertTrue_1(parseValueType(std::string(DURATION_STR)) == DURATION_TYPE);

  assertTrue_1(parseValueType(ARRAY_STR) == ARRAY_TYPE);
  assertTrue_1(parseValueType(std::string(ARRAY_STR)) == ARRAY_TYPE);

  assertTrue_1(parseValueType(BOOLEAN_ARRAY_STR) == BOOLEAN_ARRAY_TYPE);
  assertTrue_1(parseValueType(std::string(BOOLEAN_ARRAY_STR)) == BOOLEAN_ARRAY_TYPE);

  assertTrue_1(parseValueType(INTEGER_ARRAY_STR) == INTEGER_ARRAY_TYPE);
  assertTrue_1(parseValueType(std::string(INTEGER_ARRAY_STR)) == INTEGER_ARRAY_TYPE);

  assertTrue_1(parseValueType(REAL_ARRAY_STR) == REAL_ARRAY_TYPE);
  assertTrue_1(parseValueType(std::string(REAL_ARRAY_STR)) == REAL_ARRAY_TYPE);

  assertTrue_1(parseValueType(STRING_ARRAY_STR) == STRING_ARRAY_TYPE);
  assertTrue_1(parseValueType(std::string(STRING_ARRAY_STR)) == STRING_ARRAY_TYPE);

  assertTrue_1(parseValueType(NODE_STATE_STR) == NODE_STATE_TYPE);
  assertTrue_1(parseValueType(std::string(NODE_STATE_STR)) == NODE_STATE_TYPE);

  assertTrue_1(parseValueType(NODE_OUTCOME_STR) == OUTCOME_TYPE);
  assertTrue_1(parseValueType(std::string(NODE_OUTCOME_STR)) == OUTCOME_TYPE);

  assertTrue_1(parseValueType(NODE_FAILURE_STR) == FAILURE_TYPE);
  assertTrue_1(parseValueType(std::string(NODE_FAILURE_STR)) == FAILURE_TYPE);

  assertTrue_1(parseValueType(NODE_COMMAND_HANDLE_STR) == COMMAND_HANDLE_TYPE);
  assertTrue_1(parseValueType(std::string(NODE_COMMAND_HANDLE_STR)) == COMMAND_HANDLE_TYPE);

  return true;
}

// String constants for prefix parser testing

static const char *boolVar = "BooleanVariable";
static const char *boolArrVar = "BooleanArrayVariable";
static const char *dateVal = "DateValue";

static bool scanValueTypePrefixTest()
{
  // Test bogus input
  ValueType result;

  assertTrue_1(scanValueTypePrefix(NULL, result) == 0);
  assertTrue_1(scanValueTypePrefix(mt, result) == 0);
  assertTrue_1(scanValueTypePrefix(bogus, result) == 0);

  assertTrue_1(scanValueTypePrefix(BOOLEAN_STR, result) == strlen(BOOLEAN_STR));
  assertTrue_1(result == BOOLEAN_TYPE);
  assertTrue_1(scanValueTypePrefix(INTEGER_STR, result) == strlen(INTEGER_STR));
  assertTrue_1(result == INTEGER_TYPE);
  assertTrue_1(scanValueTypePrefix(REAL_STR, result) == strlen(REAL_STR));
  assertTrue_1(result == REAL_TYPE);
  assertTrue_1(scanValueTypePrefix(STRING_STR, result) == strlen(STRING_STR));
  assertTrue_1(result == STRING_TYPE);
  assertTrue_1(scanValueTypePrefix(DATE_STR, result) == strlen(DATE_STR));
  assertTrue_1(result == DATE_TYPE);
  assertTrue_1(scanValueTypePrefix(DURATION_STR, result) == strlen(DURATION_STR));
  assertTrue_1(result == DURATION_TYPE);
  assertTrue_1(scanValueTypePrefix(ARRAY_STR, result) == strlen(ARRAY_STR));
  assertTrue_1(result == ARRAY_TYPE);
  assertTrue_1(scanValueTypePrefix(BOOLEAN_ARRAY_STR, result) == strlen(BOOLEAN_ARRAY_STR));
  assertTrue_1(result == BOOLEAN_ARRAY_TYPE);
  assertTrue_1(scanValueTypePrefix(INTEGER_ARRAY_STR, result) == strlen(INTEGER_ARRAY_STR));
  assertTrue_1(result == INTEGER_ARRAY_TYPE);
  assertTrue_1(scanValueTypePrefix(REAL_ARRAY_STR, result) == strlen(REAL_ARRAY_STR));
  assertTrue_1(result == REAL_ARRAY_TYPE);
  assertTrue_1(scanValueTypePrefix(STRING_ARRAY_STR, result) == strlen(STRING_ARRAY_STR));
  assertTrue_1(result == STRING_ARRAY_TYPE);
  assertTrue_1(scanValueTypePrefix(NODE_STATE_STR, result) == strlen(NODE_STATE_STR));
  assertTrue_1(result == NODE_STATE_TYPE);
  assertTrue_1(scanValueTypePrefix(NODE_OUTCOME_STR, result) == strlen(NODE_OUTCOME_STR));
  assertTrue_1(result == OUTCOME_TYPE);
  assertTrue_1(scanValueTypePrefix(NODE_FAILURE_STR, result) == strlen(NODE_FAILURE_STR));
  assertTrue_1(result == FAILURE_TYPE);
  assertTrue_1(scanValueTypePrefix(NODE_COMMAND_HANDLE_STR, result) == strlen(NODE_COMMAND_HANDLE_STR));
  assertTrue_1(result == COMMAND_HANDLE_TYPE);

  // Test actual prefixes
  assertTrue_1(scanValueTypePrefix(boolVar, result) == strlen(BOOLEAN_STR));
  assertTrue_1(result == BOOLEAN_TYPE);
  assertTrue_1(scanValueTypePrefix(dateVal, result) == strlen(DATE_STR));
  assertTrue_1(result == DATE_TYPE);
  assertTrue_1(scanValueTypePrefix(boolArrVar, result) == strlen(BOOLEAN_ARRAY_STR));
  assertTrue_1(result == BOOLEAN_ARRAY_TYPE);

  return true;
}

bool valueTypeTest()
{
  runTest(parseValueTypeTest);
  runTest(scanValueTypePrefixTest);

  return true;
}
