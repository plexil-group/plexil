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

#include "ExpressionFactory.hh"
#include "ExpressionFactories.hh"
#include "test/FactoryTestNodeConnector.hh"
#include "Lookup.hh"
#include "LookupFactory.hh"
#include "PlexilLookup.hh"
#include "TestSupport.hh"

#include <cstring>

using namespace PLEXIL;

static bool testBasics()
{
  FactoryTestNodeConnector conn;

  PlexilState *state1 = new PlexilState();
  state1->setName("foo");

  PlexilState *state2 = new PlexilState();
  state2->setName("bar");
  state2->addArg(new PlexilValue(INTEGER_TYPE, "0"));

  PlexilState *state3 = new PlexilState();
  state3->setName("baz");
  state3->addArg(new PlexilValue(INTEGER_TYPE, "1"));
  state3->addArg(new PlexilValue(REAL_TYPE, "1.5"));
  state3->addArg(new PlexilValue(STRING_TYPE, "too"));

  // Basics
  PlexilLookup test1;
  test1.setName("LookupNow");
  test1.setState(state1);
  bool wasCreated = false;
  Expression *lookup1 = createExpression(&test1, &conn, wasCreated);
  assertTrue_1(lookup1);
  assertTrue_1(wasCreated);
  assertTrue_1(0 == strcmp(lookup1->exprName(), "LookupNow"));

  PlexilChangeLookup test2;
  test2.setName("LookupOnChange");
  test2.setState(state2);
  test2.setTolerance(new PlexilValue(REAL_TYPE, "0.5"));
  wasCreated = false;
  Expression *lookup2 = createExpression(&test2, &conn, wasCreated);
  assertTrue_1(lookup2);
  assertTrue_1(wasCreated);
  assertTrue_1(0 == strcmp(lookup2->exprName(), "LookupOnChange"));

  PlexilChangeLookup test3;
  test3.setName("LookupOnChange");
  test3.setState(state3);
  wasCreated = false;
  Expression *lookup3 = createExpression(&test3, &conn, wasCreated);
  assertTrue_1(lookup3);
  assertTrue_1(wasCreated);
  assertTrue_1(0 == strcmp(lookup3->exprName(), "LookupNow"));

  return true;
}

bool lookupFactoryTest()
{
  registerBasicExpressionFactories();
  registerLookupFactories();

  runTest(testBasics);

  return true;
}
