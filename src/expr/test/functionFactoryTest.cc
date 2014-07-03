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
    Expression *zeroArgExp = createExpression(known.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  known.addSubExpr((new PlexilValue(BOOLEAN_TYPE))->getId());
  Expression *oneArgExp = createExpression(known.getId(), nc, wasCreated);
  assertTrue_1(oneArgExp);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgExp->valueType() == BOOLEAN_TYPE);
  oneArgExp->activate();
  assertTrue_1(oneArgExp->isKnown());
  assertTrue_1(oneArgExp->getValue(temp));
  assertTrue_1(!temp);

  // Check two-arg form
  known.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "1"))->getId());
  try {
    Expression *twoArgExp = createExpression(known.getId(), nc, wasCreated);
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
    Expression *zeroArgExp = createExpression(len.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  len.addSubExpr((new PlexilValue(STRING_TYPE, "Foo"))->getId());
  Expression *oneArgExp = createExpression(len.getId(), nc, wasCreated);
  assertTrue_1(oneArgExp);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgExp->valueType() == INTEGER_TYPE);
  oneArgExp->activate();
  assertTrue_1(oneArgExp->isKnown());
  assertTrue_1(oneArgExp->getValue(itemp));
  assertTrue_1(itemp == 3);

  // Check two-arg form
  len.addSubExpr((new PlexilValue(STRING_TYPE, "1"))->getId());
  try {
    Expression *twoArgExp = createExpression(len.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too many args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // String concatenation

  PlexilOp cat("CONCAT", STRING_TYPE);
  std::string const *stemp;

  // No-arg form
  Expression *noArgCat = createExpression(cat.getId(), nc, wasCreated);
  assertTrue_1(noArgCat);
  assertTrue_1(wasCreated);
  assertTrue_1(noArgCat->valueType() == STRING_TYPE);
  noArgCat->activate();
  assertTrue_1(noArgCat->isKnown());
  assertTrue_1(noArgCat->getValuePointer(stemp));
  assertTrue_1(stemp->empty());

  // One-arg form
  cat.addSubExpr((new PlexilValue(STRING_TYPE, "Foo"))->getId());
  Expression *oneArgCat = createExpression(cat.getId(), nc, wasCreated);
  assertTrue_1(oneArgCat);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgCat->valueType() == STRING_TYPE);
  oneArgCat->activate();
  assertTrue_1(oneArgCat->isKnown());
  assertTrue_1(oneArgCat->getValuePointer(stemp));
  assertTrue_1(*stemp == "Foo");

  // Two-arg form
  cat.addSubExpr((new PlexilValue(STRING_TYPE, "Bar"))->getId());
  Expression *twoArgCat = createExpression(cat.getId(), nc, wasCreated);
  assertTrue_1(twoArgCat);
  assertTrue_1(wasCreated);
  assertTrue_1(twoArgCat->valueType() == STRING_TYPE);
  twoArgCat->activate();
  assertTrue_1(twoArgCat->isKnown());
  assertTrue_1(twoArgCat->getValuePointer(stemp));
  assertTrue_1(*stemp == "FooBar");

  // Three-arg form
  cat.addSubExpr((new PlexilValue(STRING_TYPE, "Baz"))->getId());
  Expression *threeArgCat = createExpression(cat.getId(), nc, wasCreated);
  assertTrue_1(threeArgCat);
  assertTrue_1(wasCreated);
  assertTrue_1(threeArgCat->valueType() == STRING_TYPE);
  threeArgCat->activate();
  assertTrue_1(threeArgCat->isKnown());
  assertTrue_1(threeArgCat->getValuePointer(stemp));
  assertTrue_1(*stemp == "FooBarBaz");

  return true;
}

static bool booleanFunctionFactoryTest()
{
  PlexilOp notOp("NOT", BOOLEAN_TYPE);

  bool wasCreated, temp;

  // Check no-arg error detection
  try {
    Expression *zeroArgNot = createExpression(notOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  notOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "1"))->getId());
  Expression *oneArgNot = createExpression(notOp.getId(), nc, wasCreated);
  assertTrue_1(oneArgNot);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgNot->valueType() == BOOLEAN_TYPE);
  oneArgNot->activate();
  assertTrue_1(oneArgNot->isKnown());
  assertTrue_1(oneArgNot->getValue(temp));
  assertTrue_1(!temp);

  // Check two-arg form
  notOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "1"))->getId());
  try {
    Expression *twoArgNot = createExpression(notOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too many args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilOp orOp("OR", BOOLEAN_TYPE);

  // Check no-arg error detection
  try {
    Expression *zeroArgOr = createExpression(orOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  orOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "0"))->getId());
  Expression *oneArgOr = createExpression(orOp.getId(), nc, wasCreated);
  assertTrue_1(oneArgOr);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgOr->valueType() == BOOLEAN_TYPE);
  oneArgOr->activate();
  assertTrue_1(oneArgOr->isKnown());
  assertTrue_1(oneArgOr->getValue(temp));
  assertTrue_1(!temp);

  // Check two-arg form
  orOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "1"))->getId());
  Expression *twoArgOr = createExpression(orOp.getId(), nc, wasCreated);
  assertTrue_1(twoArgOr);
  assertTrue_1(wasCreated);
  assertTrue_1(twoArgOr->valueType() == BOOLEAN_TYPE);
  twoArgOr->activate();
  assertTrue_1(twoArgOr->isKnown());
  assertTrue_1(twoArgOr->getValue(temp));
  assertTrue_1(temp);

  // check 3-arg
  orOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE))->getId());
  Expression *threeArgOr = createExpression(orOp.getId(), nc, wasCreated);
  assertTrue_1(threeArgOr);
  assertTrue_1(wasCreated);
  assertTrue_1(threeArgOr->valueType() == BOOLEAN_TYPE);
  threeArgOr->activate();
  assertTrue_1(threeArgOr->isKnown());
  assertTrue_1(threeArgOr->getValue(temp));
  assertTrue_1(temp);

  PlexilOp andOp("AND", BOOLEAN_TYPE);

  // Check no-arg error detection
  try {
    Expression *zeroArgAnd = createExpression(andOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  andOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "0"))->getId());
  Expression *oneArgAnd = createExpression(andOp.getId(), nc, wasCreated);
  assertTrue_1(oneArgAnd);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgAnd->valueType() == BOOLEAN_TYPE);
  oneArgAnd->activate();
  assertTrue_1(oneArgAnd->isKnown());
  assertTrue_1(oneArgAnd->getValue(temp));
  assertTrue_1(!temp);

  // Check two-arg form
  andOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "1"))->getId());
  Expression *twoArgAnd = createExpression(andOp.getId(), nc, wasCreated);
  assertTrue_1(twoArgAnd);
  assertTrue_1(wasCreated);
  assertTrue_1(twoArgAnd->valueType() == BOOLEAN_TYPE);
  twoArgAnd->activate();
  assertTrue_1(twoArgAnd->isKnown());
  assertTrue_1(twoArgAnd->getValue(temp));
  assertTrue_1(!temp);

  // check 3-arg
  andOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE))->getId());
  Expression *threeArgAnd = createExpression(andOp.getId(), nc, wasCreated);
  assertTrue_1(threeArgAnd);
  assertTrue_1(wasCreated);
  assertTrue_1(threeArgAnd->valueType() == BOOLEAN_TYPE);
  threeArgAnd->activate();
  assertTrue_1(threeArgAnd->isKnown());
  assertTrue_1(threeArgAnd->getValue(temp));
  assertTrue_1(!temp);

  PlexilOp xorOp("XOR", BOOLEAN_TYPE);

  // Check no-arg error detection
  try {
    Expression *zeroArgXor = createExpression(xorOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  xorOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "0"))->getId());
  Expression *oneArgXor = createExpression(xorOp.getId(), nc, wasCreated);
  assertTrue_1(oneArgXor);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgXor->valueType() == BOOLEAN_TYPE);
  oneArgXor->activate();
  assertTrue_1(oneArgXor->isKnown());
  assertTrue_1(oneArgXor->getValue(temp));
  assertTrue_1(!temp);

  // Check two-arg form
  xorOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE, "1"))->getId());
  Expression *twoArgXor = createExpression(xorOp.getId(), nc, wasCreated);
  assertTrue_1(twoArgXor);
  assertTrue_1(wasCreated);
  assertTrue_1(twoArgXor->valueType() == BOOLEAN_TYPE);
  twoArgXor->activate();
  assertTrue_1(twoArgXor->isKnown());
  assertTrue_1(twoArgXor->getValue(temp));
  assertTrue_1(temp);

  // check 3-arg
  xorOp.addSubExpr((new PlexilValue(BOOLEAN_TYPE))->getId());
  Expression *threeArgXor = createExpression(xorOp.getId(), nc, wasCreated);
  assertTrue_1(threeArgXor);
  assertTrue_1(wasCreated);
  assertTrue_1(threeArgXor->valueType() == BOOLEAN_TYPE);
  threeArgXor->activate();
  assertTrue_1(!threeArgXor->isKnown());
  assertTrue_1(!threeArgXor->getValue(temp));

  return true;
}

static bool arithmeticFunctionFactoryTest()
{
  bool wasCreated;
  int32_t itemp;
  double rtemp;

  PlexilOp sqrtOp("SQRT", REAL_TYPE);

  // Check no-arg error detection
  try {
    Expression *zeroArgSqrt = createExpression(sqrtOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  sqrtOp.addSubExpr((new PlexilValue(REAL_TYPE, "4"))->getId());
  Expression *oneArgSqrt = createExpression(sqrtOp.getId(), nc, wasCreated);
  assertTrue_1(oneArgSqrt);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgSqrt->valueType() == REAL_TYPE);
  oneArgSqrt->activate();
  assertTrue_1(oneArgSqrt->isKnown());
  assertTrue_1(oneArgSqrt->getValue(rtemp));
  assertTrue_1(rtemp == 2);

  // check 2-arg
  sqrtOp.addSubExpr((new PlexilValue(REAL_TYPE, "0"))->getId());
  try {
    Expression *twoArgSqrt = createExpression(sqrtOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too many args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilOp realToIntegerOp("REAL_TO_INT", INTEGER_TYPE);

  // Check no-arg error detection
  try {
    Expression *zeroArgRealToInteger = createExpression(realToIntegerOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  realToIntegerOp.addSubExpr((new PlexilValue(REAL_TYPE, "4"))->getId());
  Expression *oneArgRealToInteger = createExpression(realToIntegerOp.getId(), nc, wasCreated);
  assertTrue_1(oneArgRealToInteger);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgRealToInteger->valueType() == INTEGER_TYPE);
  oneArgRealToInteger->activate();
  assertTrue_1(oneArgRealToInteger->isKnown());
  assertTrue_1(oneArgRealToInteger->getValue(itemp));
  assertTrue_1(itemp == 4);

  // check 2-arg
  realToIntegerOp.addSubExpr((new PlexilValue(REAL_TYPE, "0"))->getId());
  try {
    Expression *twoArgRealToInteger = createExpression(realToIntegerOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too many args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilOp absOp("ABS", REAL_TYPE);

  // Check no-arg error detection
  try {
    Expression *zeroArgAbs = createExpression(absOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  absOp.addSubExpr((new PlexilValue(REAL_TYPE, "-2"))->getId());
  Expression *oneArgAbs = createExpression(absOp.getId(), nc, wasCreated);
  assertTrue_1(oneArgAbs);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgAbs->valueType() == REAL_TYPE);
  oneArgAbs->activate();
  assertTrue_1(oneArgAbs->isKnown());
  assertTrue_1(oneArgAbs->getValue(rtemp));
  assertTrue_1(rtemp == 2);

  // check 2-arg
  absOp.addSubExpr((new PlexilValue(REAL_TYPE, "0"))->getId());
  try {
    Expression *twoArgAbs = createExpression(absOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too many args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilOp subOp("SUB", REAL_TYPE);

  // Check no-arg error detection
  try {
    Expression *zeroArgSub = createExpression(subOp.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect too few args");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Check one-arg form
  subOp.addSubExpr((new PlexilValue(INTEGER_TYPE, "-2"))->getId());
  Expression *oneArgSub = createExpression(subOp.getId(), nc, wasCreated);
  assertTrue_1(oneArgSub);
  assertTrue_1(wasCreated);
  assertTrue_1(oneArgSub->valueType() == INTEGER_TYPE);
  oneArgSub->activate();
  assertTrue_1(oneArgSub->isKnown());
  assertTrue_1(oneArgSub->getValue(itemp));
  assertTrue_1(itemp == 2);

  // Check two-arg form
  subOp.addSubExpr((new PlexilValue(REAL_TYPE, "-2.5"))->getId());
  Expression *twoArgSub = createExpression(subOp.getId(), nc, wasCreated);
  assertTrue_1(twoArgSub);
  assertTrue_1(wasCreated);
  assertTrue_1(twoArgSub->valueType() == REAL_TYPE);
  twoArgSub->activate();
  assertTrue_1(twoArgSub->isKnown());
  assertTrue_1(twoArgSub->getValue(rtemp));
  assertTrue_1(rtemp == 0.5);

  // Check three-arg form
  subOp.addSubExpr((new PlexilValue(INTEGER_TYPE, "3"))->getId());
  Expression *threeArgSub = createExpression(subOp.getId(), nc, wasCreated);
  assertTrue_1(threeArgSub);
  assertTrue_1(wasCreated);
  assertTrue_1(threeArgSub->valueType() == REAL_TYPE);
  threeArgSub->activate();
  assertTrue_1(threeArgSub->isKnown());
  assertTrue_1(threeArgSub->getValue(rtemp));
  assertTrue_1(rtemp == -2.5);

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
  runTest(booleanFunctionFactoryTest);
  runTest(arithmeticFunctionFactoryTest);

  delete (NodeConnector *) nc;
  return true;
}
