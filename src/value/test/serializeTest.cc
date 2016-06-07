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

#include "TestSupport.hh"
#include "ValueType.hh"

#include <cstring>

using namespace PLEXIL;

static size_t const BUFSIZE = 256; // increase for arrays, obviously

static char buffer[BUFSIZE];

static bool testBooleanSerDes()
{
  // Fill buffer
  memset((void *) buffer, 0xFF, BUFSIZE);

  // Write
  char *bufptr = buffer;
  size_t offset = 0;
  Boolean const falls = false;
  Boolean const treu = true;

  bufptr = serialize(falls, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > (char *) buffer, "serialize didn't return incremented pointer");
  offset += serialSize(falls);
  assertTrueMsg(bufptr == offset + (char *) buffer,
		"serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(treu, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > (char *) buffer, "serialize didn't return incremented pointer");
  offset += serialSize(treu);
  assertTrueMsg(bufptr == offset + (char *) buffer,
		"serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  // Read
  char const *cbufptr = buffer;
  Boolean boolRead = true;
  offset = 0;

  cbufptr = deserialize(boolRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(boolRead == falls, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize didn't increment buffer pointer");
  offset += serialSize(falls);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  char const *oldcbufptr = cbufptr;
  cbufptr = deserialize(boolRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(boolRead == treu, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > oldcbufptr, "deserialize didn't increment buffer pointer");
  offset += serialSize(falls);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  // test reading past end
  oldcbufptr = cbufptr;
  cbufptr = deserialize(boolRead, cbufptr);
  assertTrueMsg(!cbufptr, "deserialize failed to return null buffer pointer on bogus input");
  assertTrueMsg(boolRead, "deserialize modified result on bogus input");

  boolRead = false;
  cbufptr = deserialize(boolRead, oldcbufptr);
  assertTrueMsg(!cbufptr, "deserialize failed to return null buffer pointer on bogus input");
  assertTrueMsg(!boolRead, "deserialize modified result on bogus input");

  return true;
}

static bool testIntegerSerDes()
{
  // Fill buffer
  memset((void *) buffer, 0xFF, BUFSIZE);

  // Write
  char *bufptr = buffer;
  Integer zero = 0;
  Integer one = 1;
  Integer minusOne = -1;
  Integer largeInt = 2000000000;
  Integer largeNegInt = -2000000000;
  size_t offset = 0;

  bufptr = serialize(zero, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > (char *) buffer, "serialize didn't return incremented pointer");
  offset += serialSize(zero);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  char *oldbufptr = bufptr;
  bufptr = serialize(one, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > oldbufptr, "serialize didn't return incremented pointer");
  offset += serialSize(one);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  oldbufptr = bufptr;
  bufptr = serialize(minusOne, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > oldbufptr, "serialize didn't return incremented pointer");
  offset += serialSize(minusOne);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  oldbufptr = bufptr;
  bufptr = serialize(largeInt, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > oldbufptr, "serialize didn't return incremented pointer");
  offset += serialSize(largeInt);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  oldbufptr = bufptr;
  bufptr = serialize(largeNegInt, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > oldbufptr, "serialize didn't return incremented pointer");
  offset += serialSize(largeNegInt);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  // Read
  char const *cbufptr = buffer;
  Integer intRead = -42; // distinctive initial value
  offset = 0;

  cbufptr = deserialize(intRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(intRead == zero, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize didn't increment buffer pointer");
  offset += serialSize(zero);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  char const *oldcbufptr = cbufptr;
  cbufptr = deserialize(intRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(intRead == one, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > oldcbufptr, "deserialize didn't increment buffer pointer");
  offset += serialSize(one);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  oldcbufptr = cbufptr;
  cbufptr = deserialize(intRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(intRead == minusOne, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > oldcbufptr, "deserialize didn't increment buffer pointer");
  offset += serialSize(minusOne);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  oldcbufptr = cbufptr;
  cbufptr = deserialize(intRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(intRead == largeInt, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > oldcbufptr, "deserialize didn't increment buffer pointer");
  offset += serialSize(largeInt);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  oldcbufptr = cbufptr;
  cbufptr = deserialize(intRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(intRead == largeNegInt, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > oldcbufptr, "deserialize didn't increment buffer pointer");
  offset += serialSize(largeNegInt);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  // test reading past end
  cbufptr = deserialize(intRead, cbufptr);
  assertTrueMsg(!cbufptr, "deserialize failed to return null buffer pointer on bogus input");
  assertTrueMsg(intRead == largeNegInt, "deserialize modified result on bogus input");
  return true;
}

static bool testRealSerDes()
{
  // Fill buffer
  memset((void *) buffer, 0xFF, BUFSIZE);

  // Write
  char *bufptr = buffer;
  Real zero = 0;
  Real one = 1;
  Real minusOne = -1;
  Real largeReal = 2e100;
  Real smallNegReal = -2e-100;
  size_t offset = 0;

  bufptr = serialize(zero, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > (char *) buffer, "serialize didn't return incremented pointer");
  offset += serialSize(zero);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  char *oldbufptr = bufptr;
  bufptr = serialize(one, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > oldbufptr, "serialize didn't return incremented pointer");
  offset += serialSize(one);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  oldbufptr = bufptr;
  bufptr = serialize(minusOne, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > oldbufptr, "serialize didn't return incremented pointer");
  offset += serialSize(minusOne);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  oldbufptr = bufptr;
  bufptr = serialize(largeReal, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > oldbufptr, "serialize didn't return incremented pointer");
  offset += serialSize(largeReal);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  oldbufptr = bufptr;
  bufptr = serialize(smallNegReal, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > oldbufptr, "serialize didn't return incremented pointer");
  offset += serialSize(smallNegReal);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  // Read
  char const *cbufptr = buffer;
  Real realRead = -42; // distinctive initial value
  offset = 0;

  cbufptr = deserialize(realRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(realRead == zero, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize didn't increment buffer pointer");
  offset += serialSize(zero);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  char const *oldcbufptr = cbufptr;
  cbufptr = deserialize(realRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(realRead == one, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > oldcbufptr, "deserialize didn't increment buffer pointer");
  offset += serialSize(one);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  oldcbufptr = cbufptr;
  cbufptr = deserialize(realRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(realRead == minusOne, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > oldcbufptr, "deserialize didn't increment buffer pointer");
  offset += serialSize(minusOne);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  oldcbufptr = cbufptr;
  cbufptr = deserialize(realRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(realRead == largeReal, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > oldcbufptr, "deserialize didn't increment buffer pointer");
  offset += serialSize(largeReal);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  oldcbufptr = cbufptr;
  cbufptr = deserialize(realRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(realRead == smallNegReal, "deserialize didn't set result equal to source");
  assertTrueMsg(cbufptr > oldcbufptr, "deserialize didn't increment buffer pointer");
  offset += serialSize(smallNegReal);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");

  // test reading past end
  cbufptr = deserialize(realRead, cbufptr);
  assertTrueMsg(!cbufptr, "deserialize failed to return null buffer pointer on bogus input");
  assertTrueMsg(realRead == smallNegReal, "deserialize modified result on bogus input");
  return true;
}

static bool testStringSerDes()
{
  // Fill buffer
  memset((void *) buffer, 0xFF, BUFSIZE);
  char *bufptr = buffer;
  size_t offset = 0;

  String mt = "";
  String simple = "simple";

  // Write
  bufptr = serialize(mt, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > (char *) buffer, "serialize didn't return incremented pointer");
  assertTrueMsg(serialSize(mt) == 4, "serialSize returned wrong size for empty string");
  offset += serialSize(mt); 
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(simple, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize didn't return incremented pointer");
  assertTrueMsg(serialSize(simple) == 10, "serialSize returned wrong size for simple string");
  offset += serialSize(simple); 
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  // Read

  char const *cbufptr = buffer;
  String stringRead;
  offset = 0;

  cbufptr = deserialize(stringRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize didn't increment buffer pointer");
  offset += serialSize(mt); 
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment buffer pointer by expected number");
  assertTrueMsg(stringRead.empty(), "deserialize put garbage in empty string");

  cbufptr = deserialize(stringRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(cbufptr > offset + (char *) buffer, "deserialize didn't return incremented pointer");
  offset += serialSize(simple);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment pointer by expected number");
  assertTrueMsg(!stringRead.empty(), "deserialize returned empty result string");
  assertTrueMsg(stringRead.size() == simple.size(), "deserialize returned wrong string length");
  assertTrueMsg(stringRead == simple, "deserialize put garbage in simple string");

  // Test reading junk
  String bogus = "bOgUs";
  stringRead = bogus;
  cbufptr = deserialize(stringRead, cbufptr);
  assertTrueMsg(!cbufptr, "deserialize failed to return null buffer pointer on bogus input");
  assertTrueMsg(stringRead == bogus, "deserialize modified result on bogus input");

  return true;
}

static bool testCharStringSerDes()
{
  // Fill buffer
  memset((void *) buffer, 0xFF, BUFSIZE);
  char *bufptr = buffer;
  size_t offset = 0;

  char const *mt = "";
  char const *simple = "simple";

  // Write
  bufptr = serialize(mt, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > (char *) buffer, "serialize didn't return incremented pointer");
  assertTrueMsg(serialSize(mt) == 4, "serialSize returned wrong size for empty string");
  offset += serialSize(mt); 
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(simple, bufptr);
  assertTrueMsg(bufptr, "serialize returned NULL");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize didn't return incremented pointer");
  assertTrueMsg(serialSize(simple) == 10, "serialSize returned wrong size for simple string");
  offset += serialSize(simple); 
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize didn't increment pointer by expected number");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  // Read

  char const *cbufptr = buffer;
  char *stringRead = NULL;
  offset = 0;

  cbufptr = deserialize(stringRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize didn't increment buffer pointer");
  offset += serialSize(mt); 
  assertTrueMsg(cbufptr == offset + (char *) buffer,
		"deserialize didn't increment buffer pointer by expected number " << serialSize(mt));
  assertTrueMsg(!strlen(stringRead), "deserialize put garbage in empty string");

  delete stringRead;
  stringRead = NULL;
  cbufptr = deserialize(stringRead, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null buffer pointer");
  assertTrueMsg(cbufptr > offset + (char *) buffer, "deserialize didn't return incremented pointer");
  offset += serialSize(simple);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize didn't increment pointer by expected number");
  assertTrueMsg(strlen(stringRead) == strlen(simple), "deserialize returned wrong string length");
  assertTrueMsg(!strcmp(stringRead, simple), "deserialize put garbage in simple string");

  // Test reading junk
  delete stringRead;
  stringRead = NULL;
  cbufptr = deserialize(stringRead, cbufptr);
  assertTrueMsg(!cbufptr, "deserialize failed to return null buffer pointer on bogus input");
  assertTrueMsg(stringRead == NULL, "deserialize modified result on bogus input");

  return true;
}

static bool testMixedBasicSerDes()
{
  return true;
}

static bool testBasicSerDes()
{
  testBooleanSerDes();
  testIntegerSerDes();
  testRealSerDes();
  testStringSerDes();
  testCharStringSerDes();
  testMixedBasicSerDes();

  return true;
}

bool serializeTest()
{
  runTest(testBasicSerDes);
  // runTest(testArraySerDes);
  // runTest(testValueSerDes);
  return true;
}  
