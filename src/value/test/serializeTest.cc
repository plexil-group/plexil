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

#include "ArrayImpl.hh"
#include "TestSupport.hh"

#include <cstring>

using namespace PLEXIL;

static size_t const BUFSIZE = 4096;

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
  // TODO
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

static bool testBooleanArraySerDes()
{
  // Fill buffer
  memset((void *) buffer, 0xFF, BUFSIZE);
  char *bufptr = buffer;
  size_t offset = 0;

  // Initialize test data
  BooleanArray const b0;
  BooleanArray const b10f(10, false);
  BooleanArray brand(32);
  size_t ix = 0;
  for (size_t i = 0; ix < brand.size(); ++i, ix += i)
    brand.setElement(ix, (ix & 1) != 0);

  bufptr = serialize(b0, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr != (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(b0);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(b10f, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(b10f);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(brand, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(brand);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  // Read
  char const *cbufptr = buffer;
  BooleanArray tmp(1, true); // initialize differently than test data
  offset = 0;

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(b0);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 1, "deserialize failed to resize destination");
  assertTrueMsg(tmp == b0, "deserialize failed to extract data correctly");

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(b10f);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 0, "deserialize failed to resize destination");
  assertTrueMsg(tmp == b10f, "deserialize failed to extract data correctly");

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(brand);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 10, "deserialize failed to resize destination");
  assertTrueMsg(tmp == brand, "deserialize failed to extract data correctly");

  return true;
}

static bool testIntegerArraySerDes()
{
  // Fill buffer
  memset((void *) buffer, 0xFF, BUFSIZE);
  char *bufptr = buffer;
  size_t offset = 0;

  // Initialize test data
  IntegerArray const i0;
  IntegerArray const i10_0(10, 0);
  IntegerArray irand(32);
  size_t ix = 0;
  for (size_t i = 0; ix < irand.size(); ++i, ix += i)
    irand.setElement(ix, (Integer) i);

  bufptr = serialize(i0, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr != (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(i0);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(i10_0, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(i10_0);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(irand, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(irand);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  // Read
  char const *cbufptr = buffer;
  IntegerArray tmp(1, 1); // initialize differently than test data
  offset = 0;

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(i0);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 1, "deserialize failed to resize destination");
  assertTrueMsg(tmp == i0, "deserialize failed to extract data correctly");

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(i10_0);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 0, "deserialize failed to resize destination");
  assertTrueMsg(tmp == i10_0, "deserialize failed to extract data correctly");

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(irand);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 10, "deserialize failed to resize destination");
  assertTrueMsg(tmp == irand, "deserialize failed to extract data correctly");

  return true;
}

static bool testRealArraySerDes()
{
  // Fill buffer
  memset((void *) buffer, 0xFF, BUFSIZE);
  char *bufptr = buffer;
  size_t offset = 0;

  // Initialize test data
  RealArray const r0;
  RealArray const r10_0(10, 0);
  RealArray rrand(32);
  size_t ix = 0;
  for (size_t i = 0; ix < rrand.size(); ++i, ix += i)
    rrand.setElement(ix, (Real) i);

  bufptr = serialize(r0, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr != (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(r0);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(r10_0, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(r10_0);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(rrand, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(rrand);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  // Read
  char const *cbufptr = buffer;
  RealArray tmp(1, 1); // initialize differently than test data
  offset = 0;

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(r0);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 1, "deserialize failed to resize destination");
  assertTrueMsg(tmp == r0, "deserialize failed to extract data correctly");

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(r10_0);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 0, "deserialize failed to resize destination");
  assertTrueMsg(tmp == r10_0, "deserialize failed to extract data correctly");

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(rrand);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 10, "deserialize failed to resize destination");
  assertTrueMsg(tmp == rrand, "deserialize failed to extract data correctly");

  return true;
}

static bool testStringArraySerDes()
{
  // Fill buffer
  memset((void *) buffer, 0xFF, BUFSIZE);
  char *bufptr = buffer;
  size_t offset = 0;

  // Initialize test data
  StringArray const s0;
  StringArray const s10_e(10, "");
  StringArray srand(32);
  size_t ix = 0;
  for (size_t i = 0; ix < srand.size(); ++i, ix += i)
    srand.setElement(ix, String(i, 'a'));

  bufptr = serialize(s0, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr != (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(s0);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(s10_e, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(s10_e);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  bufptr = serialize(srand, bufptr);
  assertTrueMsg(bufptr, "serialize returned null pointer");
  assertTrueMsg(bufptr > offset + (char *) buffer, "serialize failed to increment pointer");
  offset += serialSize(srand);
  assertTrueMsg(bufptr == offset + (char *) buffer, "serialize failed to increment pointer as expected");
  assertTrueMsg(0xFF == (unsigned char) buffer[offset], "serialize wrote more than it should have");

  // Read
  char const *cbufptr = buffer;
  StringArray tmp(1, "1"); // initialize differently than test data
  offset = 0;

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(s0);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 1, "deserialize failed to resize destination");
  assertTrueMsg(tmp == s0, "deserialize failed to extract data correctly");

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(s10_e);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 0, "deserialize failed to resize destination");
  assertTrueMsg(tmp == s10_e, "deserialize failed to extract data correctly");

  cbufptr = deserialize(tmp, cbufptr);
  assertTrueMsg(cbufptr, "deserialize returned null pointer");
  assertTrueMsg(cbufptr > (char *) buffer, "deserialize failed to increment pointer");
  offset += serialSize(srand);
  assertTrueMsg(cbufptr == offset + (char *) buffer, "deserialize failed to increment pointer as expected");
  assertTrueMsg(tmp.size() != 10, "deserialize failed to resize destination");
  assertTrueMsg(tmp == srand, "deserialize failed to extract data correctly");

  return true;
}

static bool testArraySerDes()
{
  testBooleanArraySerDes();
  testIntegerArraySerDes();
  testRealArraySerDes();
  testStringArraySerDes();

  // more later
  return true;
}

bool serializeTest()
{
  runTest(testBasicSerDes);
  runTest(testArraySerDes);
  // runTest(testValueSerDes);
  return true;
}  
