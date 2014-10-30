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

#include "State.hh"
#include "TestSupport.hh"

using namespace PLEXIL;

static bool testConstructorsAndAccessors()
{
  // Default constructor
  State mt;
  assertTrue_1(mt.name().empty());
  assertTrue_1(mt.parameters().empty());
  assertTrue_1(mt.parameterCount() == 0);
  // Array bounds checking test
  assertTrue_1(!mt.isParameterKnown(1));
  assertTrue_1(mt.parameterType(1) == UNKNOWN_TYPE);

  // Name only
  std::string const foo("Foo");
  State named(foo);
  assertTrue_1(!named.name().empty());
  assertTrue_1(named.parameters().empty());
  assertTrue_1(named.parameterCount() == 0);
  assertTrue_1(named.name() == foo);

  // Set up parameters
  Value too((int32_t) 2);
  Value roo(3.5);
  std::string soo("Soo");
  Value sue(soo);

  // Name and params
  State test1(foo, 3);
  test1.setParameter(0, too);
  test1.setParameter(1, roo);
  test1.setParameter(2, sue);

  assertTrue_1(!test1.name().empty());
  assertTrue_1(test1.name() == foo);
  assertTrue_1(!test1.parameters().empty());
  assertTrue_1(test1.parameterCount() == 3);
  assertTrue_1(test1.parameter(0) == too);
  assertTrue_1(test1.parameter(1) == roo);
  assertTrue_1(test1.parameter(2) == sue);
  // Array bounds checking test
  assertTrue_1(!test1.isParameterKnown(3));
  assertTrue_1(test1.parameterType(3) == UNKNOWN_TYPE);

  // Copy
  State test2(test1);
  assertTrue_1(!test2.name().empty());
  assertTrue_1(test2.name() == foo);
  assertTrue_1(!test2.parameters().empty());
  assertTrue_1(test2.parameterCount() == 3);
  assertTrue_1(test2.parameter(0) == too);
  assertTrue_1(test2.parameter(1) == roo);
  assertTrue_1(test2.parameter(2) == sue);
  // Array bounds checking test
  assertTrue_1(!test2.isParameterKnown(3));
  assertTrue_1(test2.parameterType(3) == UNKNOWN_TYPE);

  return true;
}

static bool testAssignment()
{
  // Default constructor
  State temp;

  // empty
  State mt;
  temp = mt;
  assertTrue_1(temp.name().empty());
  assertTrue_1(temp.parameters().empty());
  assertTrue_1(temp.parameterCount() == 0);

  // Name only
  std::string const foo("Foo");
  State named(foo);
  temp = named;
  assertTrue_1(!temp.name().empty());
  assertTrue_1(temp.parameters().empty());
  assertTrue_1(temp.parameterCount() == 0);
  assertTrue_1(temp.name() == foo);

  // Set empty again
  temp = mt;
  assertTrue_1(temp.name().empty());
  assertTrue_1(temp.parameters().empty());
  assertTrue_1(temp.parameterCount() == 0);

  // Set up parameters
  Value too((int32_t) 2);
  Value roo(3.5);
  std::string soo("Soo");
  Value sue(soo);

  // Name and params
  State test1(foo, 3);
  test1.setParameter(0, too);
  test1.setParameter(1, roo);
  test1.setParameter(2, sue);

  temp = test1;

  assertTrue_1(!temp.name().empty());
  assertTrue_1(temp.name() == foo);
  assertTrue_1(!temp.parameters().empty());
  assertTrue_1(temp.parameterCount() == 3);
  assertTrue_1(temp.parameter(0) == too);
  assertTrue_1(temp.parameter(1) == roo);
  assertTrue_1(temp.parameter(2) == sue);

  // Set empty again
  temp = mt;
  assertTrue_1(temp.name().empty());
  assertTrue_1(temp.parameters().empty());
  assertTrue_1(temp.parameterCount() == 0);

  return true;
}

static bool testEquality()
{
  State mt;
  assertTrue_1(mt == mt);
  assertTrue_1(!(mt != mt));

  State named("Foo");
  assertTrue_1(named == named);
  assertTrue_1(!(named != named));
  assertTrue_1(!(named == mt));
  assertTrue_1(!(mt == named));
  assertTrue_1(named != mt);
  assertTrue_1(mt != named);

  std::string const foo("Foo");
  State named2(foo);
  assertTrue_1(named2 == named2);
  assertTrue_1(!(named2 != named2));
  assertTrue_1(!(named2 == mt));
  assertTrue_1(!(mt == named2));
  assertTrue_1(named2 != mt);
  assertTrue_1(mt != named2);
  assertTrue_1(named2 == named);
  assertTrue_1(named == named2);
  assertTrue_1(!(named2 != named));
  assertTrue_1(!(named != named2));

  State named3("Fop");
  assertTrue_1(named3 == named3);
  assertTrue_1(!(named3 != named3));
  assertTrue_1(!(named3 == mt));
  assertTrue_1(!(mt == named3));
  assertTrue_1(named3 != mt);
  assertTrue_1(mt != named3);
  assertTrue_1(!(named3 == named));
  assertTrue_1(!(named == named3));
  assertTrue_1(named3 != named);
  assertTrue_1(named != named3);
  assertTrue_1(!(named3 == named2));
  assertTrue_1(!(named2 == named3));
  assertTrue_1(named3 != named2);
  assertTrue_1(named2 != named3);

  // Set up parameter vector
  Value too((int32_t) 2);
  Value roo(3.5);
  std::string soo("Soo");
  Value sue(soo);

  // Name and params
  State test1(foo, 3);
  test1.setParameter(0, too);
  test1.setParameter(1, roo);
  test1.setParameter(2, soo);

  assertTrue_1(test1 == test1);
  assertTrue_1(!(test1 != test1));
  assertTrue_1(!(test1 == mt));
  assertTrue_1(!(mt == test1));
  assertTrue_1(test1 != mt);
  assertTrue_1(mt != test1);
  assertTrue_1(!(test1 == named));
  assertTrue_1(!(named == test1));
  assertTrue_1(test1 != named);
  assertTrue_1(named != test1);
  assertTrue_1(!(test1 == named2));
  assertTrue_1(!(named2 == test1));
  assertTrue_1(test1 != named2);
  assertTrue_1(named2 != test1);
  assertTrue_1(!(test1 == named3));
  assertTrue_1(!(named3 == test1));
  assertTrue_1(test1 != named3);
  assertTrue_1(named3 != test1);

  // Clone of test1
  State clone1(test1);
  assertTrue_1(clone1 == clone1);
  assertTrue_1(!(clone1 != clone1));
  assertTrue_1(!(clone1 == mt));
  assertTrue_1(!(mt == clone1));
  assertTrue_1(clone1 != mt);
  assertTrue_1(mt != clone1);
  assertTrue_1(!(clone1 == named));
  assertTrue_1(!(named == clone1));
  assertTrue_1(clone1 != named);
  assertTrue_1(named != clone1);
  assertTrue_1(!(clone1 == named2));
  assertTrue_1(!(named2 == clone1));
  assertTrue_1(clone1 != named2);
  assertTrue_1(named2 != clone1);
  assertTrue_1(!(clone1 == named3));
  assertTrue_1(!(named3 == clone1));
  assertTrue_1(clone1 != named3);
  assertTrue_1(named3 != clone1);
  assertTrue_1(clone1 == test1);
  assertTrue_1(test1 == clone1);
  assertTrue_1(!(clone1 != test1));
  assertTrue_1(!(test1 != clone1));

  State test2(test1);
  Value sop("Sop");
  test2.setParameter(2, sop);

  assertTrue_1(test2 == test2);
  assertTrue_1(!(test2 != test2));
  assertTrue_1(!(test2 == mt));
  assertTrue_1(!(mt == test2));
  assertTrue_1(test2 != mt);
  assertTrue_1(mt != test2);
  assertTrue_1(!(test2 == named));
  assertTrue_1(!(named == test2));
  assertTrue_1(test2 != named);
  assertTrue_1(named != test2);
  assertTrue_1(!(test2 == named2));
  assertTrue_1(!(named2 == test2));
  assertTrue_1(test2 != named2);
  assertTrue_1(named2 != test2);
  assertTrue_1(!(test2 == named3));
  assertTrue_1(!(named3 == test2));
  assertTrue_1(test2 != named3);
  assertTrue_1(named3 != test2);
  assertTrue_1(!(test2 == test1));
  assertTrue_1(!(test1 == test2));
  assertTrue_1(test2 != test1);
  assertTrue_1(test1 != test2);

  return true;
}

static bool testLessThan()
{
  State mt;
  assertTrue_1(!(mt < mt));

  State named("Foo");
  assertTrue_1(!(named < named));
  assertTrue_1(!(named < mt));
  assertTrue_1(mt < named);

  std::string const foo("Foo");
  State named2(foo);
  assertTrue_1(!(named2 < named2));
  assertTrue_1(!(named2 < mt));
  assertTrue_1(mt < named2);
  assertTrue_1(!(named2 < named));
  assertTrue_1(!(named < named2));

  State named3("Fop");
  assertTrue_1(!(named3 < named3));
  assertTrue_1(!(named3 < mt));
  assertTrue_1(mt < named3);
  assertTrue_1(!(named3 < named));
  assertTrue_1(named < named3);
  assertTrue_1(!(named3 < named2));
  assertTrue_1(named2 < named3);

  // Set up parameters
  Value too((int32_t) 2);
  Value roo(3.5);
  std::string soo("Soo");
  Value sue(soo);
  State test1(foo, 3);
  test1.setParameter(0, too);
  test1.setParameter(1, roo);
  test1.setParameter(2, sue);

  assertTrue_1(!(test1 < test1));
  assertTrue_1(!(test1 < mt));
  assertTrue_1(mt < test1);
  assertTrue_1(!(test1 < named));
  assertTrue_1(named < test1);
  assertTrue_1(!(test1 < named2));
  assertTrue_1(named2 < test1);
  assertTrue_1(test1 < named3);
  assertTrue_1(!(named3 < test1));

  State test2(test1);
  Value sop("Sop");
  test2.setParameter(2, sop);

  assertTrue_1(!(test2 < test2));
  assertTrue_1(!(test2 < mt));
  assertTrue_1(mt < test2);
  assertTrue_1(!(test2 < named));
  assertTrue_1(named < test2);
  assertTrue_1(!(test2 < named2));
  assertTrue_1(named2 < test2);
  assertTrue_1(test2 < named3);
  assertTrue_1(!(named3 < test2));
  assertTrue_1(!(test2 < test1));
  assertTrue_1(test1 < test2);

  return true;
}


bool stateTest()
{
  runTest(testConstructorsAndAccessors);
  runTest(testAssignment);
  runTest(testEquality);
  runTest(testLessThan);

  return true;
}
