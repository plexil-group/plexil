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
#include "FactoryTestNodeConnector.hh"
#include "PlexilExpr.hh"
#include "TestSupport.hh"

using namespace PLEXIL;

// Global variables for convenience
static NodeConnector *nc = NULL;
static FactoryTestNodeConnector *realNc = NULL;

static bool booleanVariableFactoryTest()
{
  // uninitialized
  PlexilVar bVar("b", BOOLEAN_TYPE);
  // initialized
  PlexilVar fVar("f", BOOLEAN_TYPE, "0");
  PlexilVar tVar("t", BOOLEAN_TYPE, "1");
  PlexilVar uVar("u", BOOLEAN_TYPE, "UNKNOWN");
  PlexilVar bogusVar("bogus", BOOLEAN_TYPE, "bOgUs");

  // initialized via expression
  PlexilVar xVar("x", BOOLEAN_TYPE, new PlexilValue(BOOLEAN_TYPE, "0"));

  bool wasCreated;
  bool temp;

  Expression *bExp = createExpression(&bVar, nc, wasCreated);
  assertTrue_1(bExp);
  assertTrue_1(wasCreated);
  assertTrue_1(bExp->isAssignable());
  assertTrue_1(bExp->valueType() == BOOLEAN_TYPE);
  bExp->activate();
  assertTrue_1(!bExp->isKnown());
  assertTrue_1(!bExp->getValue(temp));
  realNc->storeVariable("b", bExp);

  Expression *fExp = createExpression(&fVar, nc, wasCreated);
  assertTrue_1(fExp);
  assertTrue_1(wasCreated);
  assertTrue_1(fExp->isAssignable());
  assertTrue_1(fExp->valueType() == BOOLEAN_TYPE);
  fExp->activate();
  assertTrue_1(fExp->isKnown());
  assertTrue_1(fExp->getValue(temp));
  assertTrue_1(!temp);
  realNc->storeVariable("f", fExp);

  Expression *tExp = createExpression(&tVar, nc, wasCreated);
  assertTrue_1(tExp);
  assertTrue_1(wasCreated);
  assertTrue_1(tExp->isAssignable());
  assertTrue_1(tExp->valueType() == BOOLEAN_TYPE);
  tExp->activate();
  assertTrue_1(tExp->isKnown());
  assertTrue_1(tExp->getValue(temp));
  assertTrue_1(temp);
  realNc->storeVariable("t", tExp);

  Expression *uExp = createExpression(&uVar, nc, wasCreated);
  assertTrue_1(uExp);
  assertTrue_1(wasCreated);
  assertTrue_1(uExp->isAssignable());
  assertTrue_1(uExp->valueType() == BOOLEAN_TYPE);
  uExp->activate();
  assertTrue_1(!uExp->isKnown());
  assertTrue_1(!uExp->getValue(temp));
  
  try {
    Expression *bogusExp = createExpression(&bogusVar, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  Expression *xExp = createExpression(&xVar, nc, wasCreated);
  assertTrue_1(xExp);
  assertTrue_1(wasCreated);
  assertTrue_1(xExp->isAssignable());
  assertTrue_1(xExp->valueType() == BOOLEAN_TYPE);
  xExp->activate();
  assertTrue_1(xExp->isKnown());
  assertTrue_1(xExp->getValue(temp));
  assertTrue_1(!temp);

  // Variable references

  PlexilVarRef bRef("b", BOOLEAN_TYPE);
  Expression *bExpRef = createExpression(&bRef, nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(bExpRef == bExp);

  PlexilVarRef qRef("q", BOOLEAN_TYPE);
  try {
    Expression *qExpRef = createExpression(&qRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilVarRef tBadRef("t", INTEGER_TYPE);
  try {
    Expression *tBadExpRef = createExpression(&tBadRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type conflict");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }
    
  return true;
}

static bool integerVariableFactoryTest()
{
  // uninitialized
  PlexilVar iVar("i", INTEGER_TYPE);
  // initialized
  PlexilVar zeroVar("z", INTEGER_TYPE, "0");
  PlexilVar tVar("t", INTEGER_TYPE, "-2000000000");
  PlexilVar hVar("h", INTEGER_TYPE, "0xBADF00D");
  PlexilVar uVar("u", INTEGER_TYPE, "UNKNOWN");
  PlexilVar bogusVar("bogus", INTEGER_TYPE, "bOgUs");
  PlexilVar tooBigVar("tooBig", INTEGER_TYPE, "3000000000");

  // initialized via expression
  PlexilVar xVar("x", INTEGER_TYPE, new PlexilValue(INTEGER_TYPE, "0"));

  bool wasCreated;
  int32_t temp;

  Expression *iExp = createExpression(&iVar, nc, wasCreated);
  assertTrue_1(iExp);
  assertTrue_1(wasCreated);
  assertTrue_1(iExp->isAssignable());
  assertTrue_1(iExp->valueType() == INTEGER_TYPE);
  iExp->activate();
  assertTrue_1(!iExp->isKnown());
  assertTrue_1(!iExp->getValue(temp));
  realNc->storeVariable("i", iExp);

  Expression *zeroExp = createExpression(&zeroVar, nc, wasCreated);
  assertTrue_1(zeroExp);
  assertTrue_1(wasCreated);
  assertTrue_1(zeroExp->isAssignable());
  assertTrue_1(zeroExp->valueType() == INTEGER_TYPE);
  zeroExp->activate();
  assertTrue_1(zeroExp->isKnown());
  assertTrue_1(zeroExp->getValue(temp));
  assertTrue_1(temp == 0);
  realNc->storeVariable("z", zeroExp);

  Expression *tExp = createExpression(&tVar, nc, wasCreated);
  assertTrue_1(tExp);
  assertTrue_1(wasCreated);
  assertTrue_1(tExp->isAssignable());
  assertTrue_1(tExp->valueType() == INTEGER_TYPE);
  tExp->activate();
  assertTrue_1(tExp->isKnown());
  assertTrue_1(tExp->getValue(temp));
  assertTrue_1(temp == (int32_t) -2000000000);
  realNc->storeVariable("t", tExp);

  Expression *hExp = createExpression(&hVar, nc, wasCreated);
  assertTrue_1(hExp);
  assertTrue_1(wasCreated);
  assertTrue_1(hExp->isAssignable());
  assertTrue_1(hExp->valueType() == INTEGER_TYPE);
  hExp->activate();
  assertTrue_1(hExp->isKnown());
  assertTrue_1(hExp->getValue(temp));
  assertTrue_1(temp == 0xBADF00D);
  realNc->storeVariable("t", hExp);

  Expression *uExp = createExpression(&uVar, nc, wasCreated);
  assertTrue_1(uExp);
  assertTrue_1(wasCreated);
  assertTrue_1(uExp->isAssignable());
  assertTrue_1(uExp->valueType() == INTEGER_TYPE);
  uExp->activate();
  assertTrue_1(!uExp->isKnown());
  assertTrue_1(!uExp->getValue(temp));
  
  try {
    Expression *bogusExp = createExpression(&bogusVar, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }
  
  try {
    Expression *tooBigExp = createExpression(&tooBigVar, nc, wasCreated);
    assertTrue_2(false, "Failed to detect out-of-range initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  Expression *xExp = createExpression(&xVar, nc, wasCreated);
  assertTrue_1(xExp);
  assertTrue_1(wasCreated);
  assertTrue_1(xExp->isAssignable());
  assertTrue_1(xExp->valueType() == INTEGER_TYPE);
  xExp->activate();
  assertTrue_1(xExp->isKnown());
  assertTrue_1(xExp->getValue(temp));
  assertTrue_1(temp == 0);

  // Variable references

  PlexilVarRef iRef("i", INTEGER_TYPE);
  Expression *iExpRef = createExpression(&iRef, nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(iExpRef == iExp);

  PlexilVarRef qRef("q", INTEGER_TYPE);
  try {
    Expression *qExpRef = createExpression(&qRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilVarRef tBadRef("z", BOOLEAN_TYPE);
  try {
    Expression *tBadExpRef = createExpression(&tBadRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type conflict");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }
    
  return true;
}

static bool realVariableFactoryTest()
{
  // uninitialized
  PlexilVar iVar("i", REAL_TYPE);
  // initialized
  PlexilVar zeroVar("z", REAL_TYPE, "0");
  PlexilVar tVar("t", REAL_TYPE, "-2e10");
  PlexilVar piVar("pi", REAL_TYPE, "3.1415");
  PlexilVar uVar("u", REAL_TYPE, "UNKNOWN");
  PlexilVar bogusVar("bogus", REAL_TYPE, "bOgUs");
  PlexilVar tooBigVar("tooBig", REAL_TYPE, "3e1000000000");

  // initialized via expression
  PlexilVar xVar("x", REAL_TYPE, new PlexilValue(REAL_TYPE, "0"));

  bool wasCreated;
  double temp;

  Expression *iExp = createExpression(&iVar, nc, wasCreated);
  assertTrue_1(iExp);
  assertTrue_1(wasCreated);
  assertTrue_1(iExp->isAssignable());
  assertTrue_1(iExp->valueType() == REAL_TYPE);
  iExp->activate();
  assertTrue_1(!iExp->isKnown());
  assertTrue_1(!iExp->getValue(temp));
  realNc->storeVariable("i", iExp);

  Expression *zeroExp = createExpression(&zeroVar, nc, wasCreated);
  assertTrue_1(zeroExp);
  assertTrue_1(wasCreated);
  assertTrue_1(zeroExp->isAssignable());
  assertTrue_1(zeroExp->valueType() == REAL_TYPE);
  zeroExp->activate();
  assertTrue_1(zeroExp->isKnown());
  assertTrue_1(zeroExp->getValue(temp));
  assertTrue_1(temp == 0);
  realNc->storeVariable("z", zeroExp);

  Expression *tExp = createExpression(&tVar, nc, wasCreated);
  assertTrue_1(tExp);
  assertTrue_1(wasCreated);
  assertTrue_1(tExp->isAssignable());
  assertTrue_1(tExp->valueType() == REAL_TYPE);
  tExp->activate();
  assertTrue_1(tExp->isKnown());
  assertTrue_1(tExp->getValue(temp));
  assertTrue_1(temp == -20000000000.0);
  realNc->storeVariable("t", tExp);

  Expression *piExp = createExpression(&piVar, nc, wasCreated);
  assertTrue_1(piExp);
  assertTrue_1(wasCreated);
  assertTrue_1(piExp->isAssignable());
  assertTrue_1(piExp->valueType() == REAL_TYPE);
  piExp->activate();
  assertTrue_1(piExp->isKnown());
  assertTrue_1(piExp->getValue(temp));
  assertTrue_1(temp == 3.1415);
  realNc->storeVariable("pi", piExp);

  Expression *uExp = createExpression(&uVar, nc, wasCreated);
  assertTrue_1(uExp);
  assertTrue_1(wasCreated);
  assertTrue_1(uExp->isAssignable());
  assertTrue_1(uExp->valueType() == REAL_TYPE);
  uExp->activate();
  assertTrue_1(!uExp->isKnown());
  assertTrue_1(!uExp->getValue(temp));
  
  try {
    Expression *bogusExp = createExpression(&bogusVar, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }
  
  try {
    Expression *tooBigExp = createExpression(&tooBigVar, nc, wasCreated);
    assertTrue_2(false, "Failed to detect out-of-range initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  Expression *xExp = createExpression(&xVar, nc, wasCreated);
  assertTrue_1(xExp);
  assertTrue_1(wasCreated);
  assertTrue_1(xExp->isAssignable());
  assertTrue_1(xExp->valueType() == REAL_TYPE);
  xExp->activate();
  assertTrue_1(xExp->isKnown());
  assertTrue_1(xExp->getValue(temp));
  assertTrue_1(temp == 0);

  // Variable references

  PlexilVarRef iRef("i", REAL_TYPE);
  Expression *iExpRef = createExpression(&iRef, nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(iExpRef);
  assertTrue_1(iExpRef == iExp);

  PlexilVarRef qRef("q", REAL_TYPE);
  try {
    Expression *qExpRef = createExpression(&qRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilVarRef tBadRef("z", BOOLEAN_TYPE);
  try {
    Expression *tBadExpRef = createExpression(&tBadRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type conflict");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }
    
  return true;
}

static bool stringVariableFactoryTest()
{
  PlexilVar unk("unk", STRING_TYPE); // uninited
  PlexilVar mt("mt", STRING_TYPE, ""); // empty
  PlexilVar foo("foo", STRING_TYPE, "Foo!"); // literal init
  PlexilVar bar("bar", STRING_TYPE,
                new PlexilVarRef("foo", STRING_TYPE)); // init from var ref

  bool wasCreated;
  std::string const *temp = NULL;

  Expression *unkExp = createExpression(&unk, nc, wasCreated);
  assertTrue_1(unkExp);
  assertTrue_1(wasCreated);
  assertTrue_1(unkExp->isAssignable());
  assertTrue_1(unkExp->valueType() == STRING_TYPE);
  unkExp->activate();
  assertTrue_1(!unkExp->isKnown());
  assertTrue_1(!unkExp->getValuePointer(temp));
  assertTrue_1(temp == NULL);
  realNc->storeVariable("unk", unkExp);

  Expression *mtExp = createExpression(&mt, nc, wasCreated);
  assertTrue_1(mtExp);
  assertTrue_1(wasCreated);
  assertTrue_1(mtExp->isAssignable());
  assertTrue_1(mtExp->valueType() == STRING_TYPE);
  mtExp->activate();
  assertTrue_1(mtExp->isKnown());
  assertTrue_1(mtExp->getValuePointer(temp));
  assertTrue_1(temp);
  assertTrue_1(temp->empty());
  realNc->storeVariable("mt", mtExp);

  Expression *fooExp = createExpression(&foo, nc, wasCreated);
  assertTrue_1(fooExp);
  assertTrue_1(wasCreated);
  assertTrue_1(fooExp->isAssignable());
  assertTrue_1(fooExp->valueType() == STRING_TYPE);
  fooExp->activate();
  assertTrue_1(fooExp->isKnown());
  assertTrue_1(fooExp->getValuePointer(temp));
  assertTrue_1(temp);
  assertTrue_1(!temp->empty());
  assertTrue_1(*temp == "Foo!");
  realNc->storeVariable("foo", fooExp);

  Expression *barExp = createExpression(&bar, nc, wasCreated);
  assertTrue_1(barExp);
  assertTrue_1(wasCreated);
  assertTrue_1(barExp->isAssignable());
  assertTrue_1(barExp->valueType() == STRING_TYPE);
  barExp->activate();
  assertTrue_1(barExp->isKnown());
  assertTrue_1(barExp->getValuePointer(temp));
  assertTrue_1(temp);
  assertTrue_1(!temp->empty());
  assertTrue_1(*temp == "Foo!");
  realNc->storeVariable("bar", barExp);

  // Variable references

  PlexilVarRef unkRef("unk", STRING_TYPE);
  Expression *unkRefExp = createExpression(&unkRef, nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(unkRefExp);
  assertTrue_1(unkRefExp = unkExp);

  PlexilVarRef badRef("bad", STRING_TYPE);
  try {
    Expression *badRefExp = createExpression(&badRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilVarRef badTypeRef("mt", BOOLEAN_TYPE);
  try {
    Expression *badTypeRefExp = createExpression(&badTypeRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type conflict");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool booleanArrayVariableFactoryTest()
{
  PlexilArrayVar ba1Var("ba1", BOOLEAN_TYPE, 2);

  std::vector<std::string> ba2Vector(7);
  ba2Vector[0] = "0";
  ba2Vector[1] = "1";
  ba2Vector[2] = "UNKNOWN";
  ba2Vector[3] = "true";
  ba2Vector[4] = "false";
  ba2Vector[5] = "FALSE";
  ba2Vector[6] = "TRUE";
  PlexilArrayVar ba2Var("ba2", BOOLEAN_TYPE, ba2Vector.size(), ba2Vector);

  bool wasCreated, temp;
  BooleanArray const *aryTemp = NULL;

  Expression *ba1Exp = createExpression(&ba1Var, nc, wasCreated);
  assertTrue_1(wasCreated);
  assertTrue_1(ba1Exp);
  assertTrue_1(ba1Exp->valueType() == BOOLEAN_ARRAY_TYPE);
  ba1Exp->activate();
  assertTrue_1(ba1Exp->isKnown());
  assertTrue_1(ba1Exp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp);
  assertTrue_1(aryTemp->size() == 2);
  assertTrue_1(!aryTemp->anyElementsKnown());
  realNc->storeVariable("ba1", ba1Exp);

  Expression *ba2Exp = createExpression(&ba2Var, nc, wasCreated);
  assertTrue_1(wasCreated);
  assertTrue_1(ba2Exp);
  assertTrue_1(ba2Exp->valueType() == BOOLEAN_ARRAY_TYPE);
  ba2Exp->activate();
  assertTrue_1(ba2Exp->isKnown());
  assertTrue_1(ba2Exp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp);
  assertTrue_1(aryTemp->size() == ba2Vector.size());
  assertTrue_1(aryTemp->anyElementsKnown());
  assertTrue_1(!aryTemp->allElementsKnown());
  assertTrue_1(aryTemp->getElement(0, temp));
  assertTrue_1(!temp);
  assertTrue_1(aryTemp->getElement(1, temp));
  assertTrue_1(temp);
  assertTrue_1(!aryTemp->getElement(2, temp));
  assertTrue_1(aryTemp->getElement(3, temp));
  assertTrue_1(temp);
  assertTrue_1(aryTemp->getElement(4, temp));
  assertTrue_1(!temp);
  assertTrue_1(aryTemp->getElement(5, temp));
  assertTrue_1(!temp);
  assertTrue_1(aryTemp->getElement(6, temp));
  assertTrue_1(temp);
  realNc->storeVariable("ba2", ba2Exp);

  std::vector<std::string> parseErrVector(1, "bOgUs");
  PlexilArrayVar parseErrVar("parseErr", BOOLEAN_TYPE, parseErrVector.size(), parseErrVector);
  try {
    Expression *parseErrExp = createExpression(&parseErrVar, nc, wasCreated);
    assertTrue_2(false, "Failed to detect bogus initial value");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  // Variable refs
  PlexilVarRef ba1Ref("ba1", BOOLEAN_ARRAY_TYPE);
  Expression *ba1RefExp = createExpression(&ba1Ref, nc, wasCreated);
  assertTrue_1(ba1RefExp);
  assertTrue_1(!wasCreated);
  assertTrue_1(ba1RefExp == ba1Exp);

  PlexilVarRef ba2Ref("ba2", BOOLEAN_TYPE);
  try {
    Expression *ba2RefExp = createExpression(&ba2Ref, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type mismatch");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  PlexilVarRef badRef("bad", BOOLEAN_ARRAY_TYPE);
  try {
    Expression *badRefExp = createExpression(&badRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid variable reference");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  return true;
}

static bool integerArrayVariableFactoryTest()
{
  PlexilArrayVar emptyVar("empty", INTEGER_TYPE, 0, std::vector<std::string>());

  std::vector<std::string> validValVector(6);
  validValVector[0] = "0";
  validValVector[1] = "1";
  validValVector[2] = "UNKNOWN";
  validValVector[3] = "-123456789";
  validValVector[4] = "987654321";
  validValVector[5] = "0x69";
  PlexilArrayVar validVar("valid", INTEGER_TYPE, validValVector.size(), validValVector);

  std::vector<std::string> bogusValueVector(1, "bOgUs");
  PlexilArrayVar bogusValueVar("bogus", INTEGER_TYPE, bogusValueVector.size(), bogusValueVector);

  std::vector<std::string> rangeErrVector(1, "-3000000000");
  PlexilArrayVar rangeErrVar("rangeErr", INTEGER_TYPE, rangeErrVector.size(), rangeErrVector);

  bool wasCreated;
  IntegerArray const *aryTemp = NULL;

  Expression *emptyExp = createExpression(&emptyVar, nc, wasCreated);
  assertTrue_1(emptyExp);
  assertTrue_1(wasCreated);
  assertTrue_1(emptyExp->valueType() == INTEGER_ARRAY_TYPE);
  emptyExp->activate();
  assertTrue_1(emptyExp->isKnown());
  assertTrue_1(emptyExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 0);
  realNc->storeVariable("empty", emptyExp);

  int32_t temp;
  Expression *validValExp = createExpression(&validVar, nc, wasCreated);
  assertTrue_1(validValExp);
  assertTrue_1(wasCreated);
  assertTrue_1(validValExp->valueType() == INTEGER_ARRAY_TYPE);
  validValExp->activate();
  assertTrue_1(validValExp->isKnown());
  assertTrue_1(validValExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == validValVector.size());
  assertTrue_1(aryTemp->getElement(0, temp));
  assertTrue_1(temp == 0);
  assertTrue_1(aryTemp->getElement(1, temp));
  assertTrue_1(temp == 1);
  assertTrue_1(!aryTemp->getElement(2, temp));
  assertTrue_1(aryTemp->getElement(3, temp));
  assertTrue_1(temp == -123456789);
  assertTrue_1(aryTemp->getElement(4, temp));
  assertTrue_1(temp == 987654321);
  assertTrue_1(aryTemp->getElement(5, temp));
  assertTrue_1(temp == 0x69);
  realNc->storeVariable("valid", validValExp);

  try {
    Expression *bogusValueExp = createExpression(&bogusValueVar, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  try {
    Expression *rangeErrExp = createExpression(&rangeErrVar, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect out-of-range integer");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  // Variable reference tests

  PlexilVarRef validRef("valid", INTEGER_ARRAY_TYPE);
  Expression *validRefExp = createExpression(&validRef, nc, wasCreated);
  assertTrue_1(validRefExp);
  assertTrue_1(!wasCreated);
  assertTrue_1(validRefExp == validValExp);

  PlexilVarRef badNameRef("bad", INTEGER_ARRAY_TYPE);
  try {
    Expression *badNameExp = createExpression(&badNameRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid variable reference");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  PlexilVarRef wrongTypeRef("empty", INTEGER_TYPE);
  try {
    Expression *wrongTypeExp = createExpression(&wrongTypeRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type mismatch");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  return true;
}

static bool realArrayVariableFactoryTest()
{
  PlexilArrayVar emptyVar("emmty", REAL_TYPE, 0, std::vector<std::string>());

  std::vector<std::string> validVarVector(6);
  validVarVector[0] = "0";
  validVarVector[1] = "1";
  validVarVector[2] = "UNKNOWN";
  validVarVector[3] = "3.14";
  validVarVector[4] = "1e-100";
  validVarVector[5] = "6.0221413e+23";
  PlexilArrayVar validVar("vallid", REAL_TYPE, validVarVector.size(), validVarVector);

  std::vector<std::string> bogusValueVector(1, "bOgUs");
  PlexilArrayVar bogusValueVar("bogus", REAL_TYPE, bogusValueVector.size(), bogusValueVector);

  std::vector<std::string> rangeErrVector(1, "-3e1000000000");
  PlexilArrayVar rangeErrVar("rangeErr", REAL_TYPE, rangeErrVector.size(), rangeErrVector);

  bool wasCreated;
  RealArray const *aryTemp = NULL;
  double temp;

  Expression *emptyExp = createExpression(&emptyVar, nc, wasCreated);
  assertTrue_1(emptyExp);
  assertTrue_1(wasCreated);
  assertTrue_1(emptyExp->valueType() == REAL_ARRAY_TYPE);
  emptyExp->activate();
  assertTrue_1(emptyExp->isKnown());
  assertTrue_1(emptyExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 0);
  realNc->storeVariable("emmty", emptyExp);

  Expression *validVarExp = createExpression(&validVar, nc, wasCreated);
  assertTrue_1(validVarExp);
  assertTrue_1(wasCreated);
  assertTrue_1(validVarExp->valueType() == REAL_ARRAY_TYPE);
  validVarExp->activate();
  assertTrue_1(validVarExp->isKnown());
  assertTrue_1(validVarExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == validVarVector.size());
  assertTrue_1(aryTemp->getElement(0, temp));
  assertTrue_1(temp == 0);
  assertTrue_1(aryTemp->getElement(1, temp));
  assertTrue_1(temp == 1);
  assertTrue_1(!aryTemp->getElement(2, temp));
  assertTrue_1(aryTemp->getElement(3, temp));
  assertTrue_1(temp == 3.14);
  assertTrue_1(aryTemp->getElement(4, temp));
  assertTrue_1(temp == 1e-100);
  assertTrue_1(aryTemp->getElement(5, temp));
  assertTrue_1(temp == 6.0221413e+23);
  realNc->storeVariable("vallid", validVarExp);

  try {
    Expression *bogusValueExp = createExpression(&bogusValueVar, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  try {
    Expression *rangeErrExp = createExpression(&rangeErrVar, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect out-of-range real");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  // Variable reference tests

  PlexilVarRef validRef("vallid", REAL_ARRAY_TYPE);
  Expression *validRefExp = createExpression(&validRef, nc, wasCreated);
  assertTrue_1(validRefExp);
  assertTrue_1(!wasCreated);
  assertTrue_1(validRefExp == validVarExp);

  PlexilVarRef badNameRef("bad", REAL_ARRAY_TYPE);
  try {
    Expression *badNameExp = createExpression(&badNameRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid variable reference");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  PlexilVarRef wrongTypeRef("emmty", REAL_TYPE);
  try {
    Expression *wrongTypeExp = createExpression(&wrongTypeRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type mismatch");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  return true;
}

static bool stringArrayVariableFactoryTest()
{
  PlexilArrayVar emptyVal("mty", STRING_TYPE, 0, std::vector<std::string>());

  std::vector<std::string> validValVector(6);
  validValVector[0] = "0";
  validValVector[1] = "1";
  validValVector[2] = "UNKNOWN";
  validValVector[3] = "3.14";
  validValVector[4] = "1e-100";
  validValVector[5] = "6.0221413e+23";
  PlexilArrayVar validVal("vlaid", STRING_TYPE, validValVector.size(), validValVector);

  bool wasCreated;
  StringArray const *aryTemp = NULL;
  std::string const *temp;

  Expression *emptyExp = createExpression(&emptyVal, nc, wasCreated);
  assertTrue_1(emptyExp);
  assertTrue_1(wasCreated);
  assertTrue_1(emptyExp->valueType() == STRING_ARRAY_TYPE);
  emptyExp->activate();
  assertTrue_1(emptyExp->isKnown());
  assertTrue_1(emptyExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 0);
  realNc->storeVariable("mty", emptyExp);

  Expression *validValExp = createExpression(&validVal, nc, wasCreated);
  assertTrue_1(validValExp);
  assertTrue_1(wasCreated);
  assertTrue_1(validValExp->valueType() == STRING_ARRAY_TYPE);
  validValExp->activate();
  assertTrue_1(validValExp->isKnown());
  assertTrue_1(validValExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == validValVector.size());
  assertTrue_1(aryTemp->getElementPointer(0, temp));
  assertTrue_1(*temp == "0");
  assertTrue_1(aryTemp->getElementPointer(1, temp));
  assertTrue_1(*temp == "1");
  assertTrue_1(aryTemp->getElementPointer(2, temp));
  assertTrue_1(*temp == "UNKNOWN");
  assertTrue_1(aryTemp->getElementPointer(3, temp));
  assertTrue_1(*temp == "3.14");
  assertTrue_1(aryTemp->getElementPointer(4, temp));
  assertTrue_1(*temp == "1e-100");
  assertTrue_1(aryTemp->getElementPointer(5, temp));
  assertTrue_1(*temp == "6.0221413e+23");
  realNc->storeVariable("vlaid", emptyExp);

  // Variable reference tests
  PlexilVarRef emptyRef("mty", STRING_ARRAY_TYPE);
  Expression *emptyRefExp = createExpression(&emptyRef, nc, wasCreated);
  assertTrue_1(emptyRefExp);
  assertTrue_1(!wasCreated);
  assertTrue_1(emptyRefExp == emptyExp);

  PlexilVarRef wrongTypeRef("vlaid", STRING_TYPE);
  try {
    Expression *wrongTypeExp = createExpression(&wrongTypeRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type mismatch");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilVarRef badRef("bad", STRING_ARRAY_TYPE);
  try {
    Expression *badExp = createExpression(&badRef, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid variable reference");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

bool variableFactoryTest()
{
  // Initialize factories
  registerBasicExpressionFactories();
  // Initialize infrastructure
  realNc = new FactoryTestNodeConnector();
  nc = realNc;

  runTest(booleanVariableFactoryTest);
  runTest(integerVariableFactoryTest);
  runTest(realVariableFactoryTest);
  runTest(stringVariableFactoryTest);
  runTest(booleanArrayVariableFactoryTest);
  runTest(integerArrayVariableFactoryTest);
  runTest(realArrayVariableFactoryTest);
  runTest(stringArrayVariableFactoryTest);

  nc = NULL;
  delete realNc;
  return true;
}
