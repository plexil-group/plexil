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

#include "ArrayImpl.hh"
#include "ExpressionFactory.hh"
#include "ExpressionFactories.hh"
#include "NodeConnector.hh"
#include "ParserException.hh"
#include "PlexilExpr.hh"
#include "TestSupport.hh"
#include "TrivialNodeConnector.hh"

using namespace PLEXIL;

// Global for convenience

static NodeConnectorId nc = NodeConnectorId::noId();

static bool isKnownFactoryTest()
{
  PlexilOp known("IsKnown", BOOLEAN_TYPE);

  bool wasCreated, temp;

  // Check no-arg error detection
  try {
    ExpressionId zeroArgExp = createExpression(known.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  known.addSubExpr((new PlexilValue(BOOLEAN_TYPE))->getId());
  ExpressionId oneArgExp = createExpression(known.getId(), nc, wasCreated);
  assertTrue_1(oneArgExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgExp->valueType() == BOOLEAN_TYPE);
  oneArgExp->activate();
  assertTrue_1(oneArgExp->isKnown());
  assertTrue_1(oneArgExp->getValue(temp));
  assertTrue_1(!temp);

  // Check two-arg form
  known.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "1"))->getId());
  try {
    ExpressionId twoArgExp = createExpression(known.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too many args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool stringFunctionFactoryTest()
{

  // StringLength
  PlexilOp len("STRLEN", INTEGER_TYPE);
  bool wasCreated;
  int32_t itemp;

  // Check no-arg error detection
  try {
    ExpressionId zeroArgExp = createExpression(len.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  len.addSubExpr((new PlexilValue(STRING_TYPE, "Foo"))->getId());
  ExpressionId oneArgExp = createExpression(len.getId(), nc, wasCreated);
  assertTrue_1(oneArgExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgExp->valueType() == INTEGER_TYPE);
  oneArgExp->activate();
  assertTrue_1(oneArgExp->isKnown());
  assertTrue_1(oneArgExp->getValue(itemp));
  assertTrue_1(itemp == 3);

  // Check two-arg form
  len.addSubExpr((new PlexilValue(STRING_TYPE, "1"))->getId());
  try {
    ExpressionId twoArgExp = createExpression(len.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too many args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // String concatenation

  PlexilOp cat("CONCAT", STRING_TYPE);
  std::string const *stemp;

  // No-arg form
  ExpressionId noArgCat = createExpression(cat.getId(), nc, wasCreated);
  assertTrue_1(noArgCat.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(noArgCat->valueType() == STRING_TYPE);
  noArgCat->activate();
  assertTrue_1(noArgCat->isKnown());
  assertTrue_1(noArgCat->getValuePointer(stemp));
  assertTrue_1(stemp->empty());

  // One-arg form
  cat.addSubExpr((new PlexilValue(STRING_TYPE, "Foo"))->getId());
  ExpressionId oneArgCat = createExpression(cat.getId(), nc, wasCreated);
  assertTrue_1(oneArgCat.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgCat->valueType() == STRING_TYPE);
  oneArgCat->activate();
  assertTrue_1(oneArgCat->isKnown());
  assertTrue_1(oneArgCat->getValuePointer(stemp));
  assertTrue_1(*stemp == "Foo");

  // Two-arg form
  cat.addSubExpr((new PlexilValue(STRING_TYPE, "Bar"))->getId());
  ExpressionId twoArgCat = createExpression(cat.getId(), nc, wasCreated);
  assertTrue_1(twoArgCat.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(twoArgCat->valueType() == STRING_TYPE);
  twoArgCat->activate();
  assertTrue_1(twoArgCat->isKnown());
  assertTrue_1(twoArgCat->getValuePointer(stemp));
  assertTrue_1(*stemp == "FooBar");

  // Three-arg form
  cat.addSubExpr((new PlexilValue(STRING_TYPE, "Baz"))->getId());
  ExpressionId threeArgCat = createExpression(cat.getId(), nc, wasCreated);
  assertTrue_1(threeArgCat.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(threeArgCat->valueType() == STRING_TYPE);
  threeArgCat->activate();
  assertTrue_1(threeArgCat->isKnown());
  assertTrue_1(threeArgCat->getValuePointer(stemp));
  assertTrue_1(*stemp == "FooBarBaz");

  return true;
}


bool functionFactoryTest()
{
  // Initialize factories
  registerBasicExpressionFactories();
  // Initialize infrastructure
  nc = (new TrivialNodeConnector())->getId();

  runTest(isKnownFactoryTest);
  runTest(stringFunctionFactoryTest);

  delete (NodeConnector *) nc;
  return true;
}
