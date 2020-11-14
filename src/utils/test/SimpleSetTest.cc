/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "Error.hh"
#include "SimpleSet.hh"
#include "TestSupport.hh"
#include "map-utils.hh"

using namespace PLEXIL;

bool testIntSet()
{
  SimpleSet<int> intSet;

  assertTrue_1(intSet.empty());
  assertTrue_1(intSet.size() == 0);

  // insert out of order
  intSet.insert(1);
  intSet.insert(3);
  intSet.insert(7);
  intSet.insert(2);
  intSet.insert(6);
  intSet.insert(5);
  intSet.insert(4);
  intSet.insert(8);

  assertTrue_1(!intSet.empty());
  assertTrue_1(intSet.size() == 8);

  assertTrue_1(intSet.find(4) != intSet.end());
  assertTrue_1(*intSet.find(4) == 4);
  assertTrue_1(intSet.find(1) != intSet.end());
  assertTrue_1(*intSet.find(1) == 1);
  assertTrue_1(intSet.find(8) != intSet.end());
  assertTrue_1(*intSet.find(8) == 8);
  assertTrue_1(intSet.find(0) == intSet.end());
  assertTrue_1(intSet.find(9) == intSet.end());

  // Test that they are iterated over in sorted order
  // TODO
  
  return true;
}

bool testStringSet()
{
  SimpleSet<std::string> stringSet;

  assertTrue_1(stringSet.empty());
  assertTrue_1(stringSet.size() == 0);

  // insert out of order
  stringSet.insert("one");
  stringSet.insert("three");
  stringSet.insert("seven");
  stringSet.insert("two");
  stringSet.insert("six");
  stringSet.insert("five");
  stringSet.insert("four");
  stringSet.insert("eight");

  assertTrue_1(!stringSet.empty());
  assertTrue_1(stringSet.size() == 8);

  assertTrue_1(stringSet.find("four") != stringSet.end());
  assertTrue_1(*stringSet.find("four") == "four");
  assertTrue_1(stringSet.find("one") != stringSet.end());
  assertTrue_1(*stringSet.find("one") == "one");
  assertTrue_1(stringSet.find("eight") != stringSet.end());
  assertTrue_1(*stringSet.find("eight") == "eight");
  assertTrue_1(stringSet.find("zero") == stringSet.end());
  assertTrue_1(stringSet.find("nine") == stringSet.end());

  // Test that they are iterated over in sorted order
  // TODO
  
  return true;
}

bool testCStringSet()
{
  SimpleSet<char const *, CStringComparator> cStringMap;

  assertTrue_1(cStringMap.empty());
  assertTrue_1(cStringMap.size() == 0);

  // insert out of order
  cStringMap.insert("one");
  cStringMap.insert("three");
  cStringMap.insert("seven");
  cStringMap.insert("two");
  cStringMap.insert("six");
  cStringMap.insert("five");
  cStringMap.insert("four");
  cStringMap.insert("eight");

  assertTrue_1(!cStringMap.empty());
  assertTrue_1(cStringMap.size() == 8);

  assertTrue_1(cStringMap.find("four") != cStringMap.end());
  assertTrue_1(!strcmp(*cStringMap.find("four"), "four"));
  assertTrue_1(cStringMap.find("one") != cStringMap.end());
  assertTrue_1(!strcmp(*cStringMap.find("one"), "one"));
  assertTrue_1(cStringMap.find("eight") != cStringMap.end());
  assertTrue_1(!strcmp(*cStringMap.find("eight"), "eight"));
  assertTrue_1(cStringMap.find("zero") == cStringMap.end());
  assertTrue_1(cStringMap.find("nine") == cStringMap.end());

  // Test that they are iterated over in sorted order
  // TODO
  
  return true;
}

bool SimpleSetTest()
{
  runTest(testIntSet);
  runTest(testStringSet);
  runTest(testCStringSet);

  return true;
}

