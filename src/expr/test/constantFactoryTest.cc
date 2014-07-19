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
#include "PlexilExpr.hh"
#include "TestSupport.hh"
#include "TrivialNodeConnector.hh"

using namespace PLEXIL;

// Global variable for convenience
static NodeConnector *nc = NULL;

static bool booleanConstantFactoryTest()
{
  PlexilValue falseValue(BOOLEAN_TYPE, "false");
  PlexilValue zeroValue(BOOLEAN_TYPE, "0");
  PlexilValue trueValue(BOOLEAN_TYPE, "true");
  PlexilValue oneValue(BOOLEAN_TYPE, "1");
  PlexilValue unknownValue(BOOLEAN_TYPE);
  PlexilValue bogus(BOOLEAN_TYPE, "bogus");

  bool wasCreated;
  bool temp;

  Expression *falseConstant = createExpression(&falseValue, nc, wasCreated);
  assertTrue_1(falseConstant);
  assertTrue_1(!wasCreated);
  assertTrue_1(!falseConstant->isAssignable());
  assertTrue_1(falseConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(falseConstant->getValue(temp));
  assertTrue_1(!temp);

  Expression *zeroConstant = createExpression(&zeroValue, nc, wasCreated);
  assertTrue_1(zeroConstant);
  assertTrue_1(!wasCreated);
  assertTrue_1(!zeroConstant->isAssignable());
  assertTrue_1(zeroConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(zeroConstant->getValue(temp));
  assertTrue_1(!temp);

  Expression *trueConstant = createExpression(&trueValue, nc, wasCreated);
  assertTrue_1(trueConstant);
  assertTrue_1(!wasCreated);
  assertTrue_1(!trueConstant->isAssignable());
  assertTrue_1(trueConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(trueConstant->getValue(temp));
  assertTrue_1(temp);

  Expression *oneConstant = createExpression(&oneValue, nc, wasCreated);
  assertTrue_1(oneConstant);
  assertTrue_1(!wasCreated);
  assertTrue_1(!oneConstant->isAssignable());
  assertTrue_1(oneConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(oneConstant->getValue(temp));
  assertTrue_1(temp);

  Expression *unknownConstant = createExpression(&unknownValue, nc, wasCreated);
  assertTrue_1(unknownConstant);
  assertTrue_1(!wasCreated);
  assertTrue_1(!unknownConstant->isAssignable());
  assertTrue_1(unknownConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(!unknownConstant->getValue(temp));

  try {
    Expression *bogusConstant = createExpression(&bogus, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool integerConstantFactoryTest()
{
  PlexilValue zeroVal(INTEGER_TYPE, "0");
  PlexilValue oneVal(INTEGER_TYPE, "1");
  PlexilValue minusOneVal(INTEGER_TYPE, "-1");
  PlexilValue unkVal(INTEGER_TYPE);
  PlexilValue hexVal(INTEGER_TYPE, "0x42");
  PlexilValue hexWithJunkVal(INTEGER_TYPE, "0x42r");
  PlexilValue tooBig(INTEGER_TYPE, "3000000000");
  PlexilValue wayTooBig(INTEGER_TYPE, "0x30000000000000000000000");
  PlexilValue tooSmall(INTEGER_TYPE, "-3000000000");
  PlexilValue bogus(INTEGER_TYPE, "bogus");

  bool wasCreated;
  int32_t temp;

  Expression *zeroConstant = createExpression(&zeroVal, nc, wasCreated);
  assertTrue_1(zeroConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!zeroConstant->isAssignable());
  assertTrue_1(zeroConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(zeroConstant->getValue(temp));
  assertTrue_1(temp == 0);

  Expression *oneConstant = createExpression(&oneVal, nc, wasCreated);
  assertTrue_1(oneConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!oneConstant->isAssignable());
  assertTrue_1(oneConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(oneConstant->getValue(temp));
  assertTrue_1(temp == 1);

  Expression *minusOneConstant = createExpression(&minusOneVal, nc, wasCreated);
  assertTrue_1(minusOneConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!minusOneConstant->isAssignable());
  assertTrue_1(minusOneConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(minusOneConstant->getValue(temp));
  assertTrue_1(temp == -1);

  Expression *unkConstant = createExpression(&unkVal, nc, wasCreated);
  assertTrue_1(unkConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!unkConstant->isAssignable());
  assertTrue_1(unkConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(!unkConstant->getValue(temp));

  Expression *hexConstant = createExpression(&hexVal, nc, wasCreated);
  assertTrue_1(hexConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!hexConstant->isAssignable());
  assertTrue_1(hexConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(hexConstant->getValue(temp));
  assertTrue_1(temp == 0x42); // = 66 decimal

  try {
    Expression *hexWithJunkConstant = createExpression(&hexWithJunkVal, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect number followed by junk");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  try {
    Expression *tooBigConstant = createExpression(&tooBig, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect integer overflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  try {
    Expression *wayTooBigConstant = createExpression(&wayTooBig, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect integer overflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  try {
    Expression *tooSmallConstant = createExpression(&tooSmall, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect integer underflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  try {
    Expression *bogusConstant = createExpression(&bogus, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool realConstantFactoryTest()
{
  PlexilValue zeroVal(REAL_TYPE, "0");
  PlexilValue minusZeroVal(REAL_TYPE, "-0");
  PlexilValue oneVal(REAL_TYPE, "1");
  PlexilValue minusOneVal(REAL_TYPE, "-1");
  PlexilValue piVal(REAL_TYPE, "3.14");
  PlexilValue piWithJunkVal(REAL_TYPE, "3.14T");
  PlexilValue expNotationVal(REAL_TYPE, "1e-100");
  PlexilValue tooBigVal(REAL_TYPE, "1e10000000");
  PlexilValue unkVal(REAL_TYPE);
  PlexilValue bogus(REAL_TYPE, "bogus");

  bool wasCreated;
  double temp;

  Expression *zeroConstant = createExpression(&zeroVal, nc, wasCreated);
  assertTrue_1(zeroConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!zeroConstant->isAssignable());
  assertTrue_1(zeroConstant->valueType() == REAL_TYPE);
  assertTrue_1(zeroConstant->getValue(temp));
  assertTrue_1(temp == 0);

  Expression *minusZeroConstant = createExpression(&minusZeroVal, nc, wasCreated);
  assertTrue_1(minusZeroConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!minusZeroConstant->isAssignable());
  assertTrue_1(minusZeroConstant->valueType() == REAL_TYPE);
  assertTrue_1(minusZeroConstant->getValue(temp));
  assertTrue_1(temp == 0);

  Expression *oneConstant = createExpression(&oneVal, nc, wasCreated);
  assertTrue_1(oneConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!oneConstant->isAssignable());
  assertTrue_1(oneConstant->valueType() == REAL_TYPE);
  assertTrue_1(oneConstant->getValue(temp));
  assertTrue_1(temp == 1);

  Expression *minusOneConstant = createExpression(&minusOneVal, nc, wasCreated);
  assertTrue_1(minusOneConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!minusOneConstant->isAssignable());
  assertTrue_1(minusOneConstant->valueType() == REAL_TYPE);
  assertTrue_1(minusOneConstant->getValue(temp));
  assertTrue_1(temp == -1);

  Expression *piConstant = createExpression(&piVal, nc, wasCreated);
  assertTrue_1(piConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!piConstant->isAssignable());
  assertTrue_1(piConstant->valueType() == REAL_TYPE);
  assertTrue_1(piConstant->getValue(temp));
  assertTrue_1(temp == 3.14);

  try {
    Expression *piWithJunkConstant = createExpression(&piWithJunkVal, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect junk after valid real");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  Expression *expNotationConstant = createExpression(&expNotationVal, nc, wasCreated);
  assertTrue_1(expNotationConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!expNotationConstant->isAssignable());
  assertTrue_1(expNotationConstant->valueType() == REAL_TYPE);
  assertTrue_1(expNotationConstant->getValue(temp));
  assertTrue_1(temp == 1e-100);

  try {
    Expression *tooBigConstant = createExpression(&tooBigVal, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect real overflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  try {
    Expression *bogusConstant = createExpression(&bogus, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool stringConstantFactoryTest()
{
  PlexilValue s1(STRING_TYPE, "");
  PlexilValue s2(STRING_TYPE, "foo");

  bool wasCreated;
  std::string temp;

  Expression *s1Constant = createExpression(&s1, nc, wasCreated);
  assertTrue_1(s1Constant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(s1Constant->getValue(temp));
  assertTrue_1(temp.empty());

  Expression *s2Constant = createExpression(&s2, nc, wasCreated);
  assertTrue_1(s2Constant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(s2Constant->getValue(temp));
  assertTrue_1(temp == "foo");

  return true;
}

static bool booleanArrayConstantFactoryTest()
{
  PlexilArrayValue emptyVal(BOOLEAN_TYPE, 0, std::vector<std::string>());

  std::vector<std::string> validValVector(7);
  validValVector[0] = "0";
  validValVector[1] = "1";
  validValVector[2] = "UNKNOWN";
  validValVector[3] = "true";
  validValVector[4] = "false";
  validValVector[5] = "FALSE";
  validValVector[6] = "TRUE";
  PlexilArrayValue validVal(BOOLEAN_TYPE, validValVector.size(), validValVector);

  std::vector<std::string> parseErrVector(1, "bOgUs");
  PlexilArrayValue parseErrVal(BOOLEAN_TYPE, parseErrVector.size(), parseErrVector);

  bool wasCreated;
  BooleanArray const *aryTemp = NULL;

  Expression *emptyConstant = createExpression(&emptyVal, nc, wasCreated);
  assertTrue_1(emptyConstant);
  assertTrue_1(wasCreated);
  assertTrue_1(emptyConstant->valueType() == BOOLEAN_ARRAY_TYPE);
  assertTrue_1(emptyConstant->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 0);

  bool temp;
  Expression *validValConstant = createExpression(&validVal, nc, wasCreated);
  assertTrue_1(validValConstant);
  assertTrue_1(wasCreated);
  assertTrue_1(validValConstant->valueType() == BOOLEAN_ARRAY_TYPE);
  assertTrue_1(validValConstant->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == validValVector.size());
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

  try {
    Expression *parseErrConstant = createExpression(&parseErrVal, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool integerArrayConstantFactoryTest()
{
  PlexilArrayValue emptyVal(INTEGER_TYPE, 0, std::vector<std::string>());

  std::vector<std::string> validValVector(6);
  validValVector[0] = "0";
  validValVector[1] = "1";
  validValVector[2] = "UNKNOWN";
  validValVector[3] = "-123456789";
  validValVector[4] = "987654321";
  validValVector[5] = "0x69";
  PlexilArrayValue validVal(INTEGER_TYPE, validValVector.size(), validValVector);

  std::vector<std::string> bogusValueVector(1, "bOgUs");
  PlexilArrayValue bogusValueVal(INTEGER_TYPE, bogusValueVector.size(), bogusValueVector);

  std::vector<std::string> rangeErrVector(1, "-3000000000");
  PlexilArrayValue rangeErrVal(INTEGER_TYPE, rangeErrVector.size(), rangeErrVector);

  bool wasCreated;
  IntegerArray const *aryTemp = NULL;

  Expression *emptyConstant = createExpression(&emptyVal, nc, wasCreated);
  assertTrue_1(emptyConstant);
  assertTrue_1(wasCreated);
  assertTrue_1(emptyConstant->valueType() == INTEGER_ARRAY_TYPE);
  assertTrue_1(emptyConstant->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 0);

  int32_t temp;
  Expression *validValConstant = createExpression(&validVal, nc, wasCreated);
  assertTrue_1(validValConstant);
  assertTrue_1(wasCreated);
  assertTrue_1(validValConstant->valueType() == INTEGER_ARRAY_TYPE);
  assertTrue_1(validValConstant->getValuePointer(aryTemp));
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

  try {
    Expression *bogusValueConstant = createExpression(&bogusValueVal, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  try {
    Expression *rangeErrConstant = createExpression(&rangeErrVal, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect out-of-range integer");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool realArrayConstantFactoryTest()
{
  PlexilArrayValue emptyVal(REAL_TYPE, 0, std::vector<std::string>());

  std::vector<std::string> validValVector(6);
  validValVector[0] = "0";
  validValVector[1] = "1";
  validValVector[2] = "UNKNOWN";
  validValVector[3] = "3.14";
  validValVector[4] = "1e-100";
  validValVector[5] = "6.0221413e+23";
  PlexilArrayValue validVal(REAL_TYPE, validValVector.size(), validValVector);

  std::vector<std::string> bogusValueVector(1, "bOgUs");
  PlexilArrayValue bogusValueVal(REAL_TYPE, bogusValueVector.size(), bogusValueVector);

  std::vector<std::string> rangeErrVector(1, "-3e1000000000");
  PlexilArrayValue rangeErrVal(REAL_TYPE, rangeErrVector.size(), rangeErrVector);

  bool wasCreated;
  RealArray const *aryTemp = NULL;

  Expression *emptyConstant = createExpression(&emptyVal, nc, wasCreated);
  assertTrue_1(emptyConstant);
  assertTrue_1(wasCreated);
  assertTrue_1(emptyConstant->valueType() == REAL_ARRAY_TYPE);
  assertTrue_1(emptyConstant->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 0);

  double temp;
  Expression *validValConstant = createExpression(&validVal, nc, wasCreated);
  assertTrue_1(validValConstant);
  assertTrue_1(wasCreated);
  assertTrue_1(validValConstant->valueType() == REAL_ARRAY_TYPE);
  assertTrue_1(validValConstant->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == validValVector.size());
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

  try {
    Expression *bogusValueConstant = createExpression(&bogusValueVal, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  try {
    Expression *rangeErrConstant = createExpression(&rangeErrVal, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect out-of-range real");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool stringArrayConstantFactoryTest()
{
  PlexilArrayValue emptyVal(STRING_TYPE, 0, std::vector<std::string>());

  std::vector<std::string> validValVector(6);
  validValVector[0] = "0";
  validValVector[1] = "1";
  validValVector[2] = "UNKNOWN";
  validValVector[3] = "3.14";
  validValVector[4] = "1e-100";
  validValVector[5] = "6.0221413e+23";
  PlexilArrayValue validVal(STRING_TYPE, validValVector.size(), validValVector);

  bool wasCreated;
  StringArray const *aryTemp = NULL;

  Expression *emptyConstant = createExpression(&emptyVal, nc, wasCreated);
  assertTrue_1(emptyConstant);
  assertTrue_1(wasCreated);
  assertTrue_1(emptyConstant->valueType() == STRING_ARRAY_TYPE);
  assertTrue_1(emptyConstant->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 0);

  std::string const *temp;
  Expression *validValConstant = createExpression(&validVal, nc, wasCreated);
  assertTrue_1(validValConstant);
  assertTrue_1(wasCreated);
  assertTrue_1(validValConstant->valueType() == STRING_ARRAY_TYPE);
  assertTrue_1(validValConstant->getValuePointer(aryTemp));
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

  return true;
}

bool constantFactoryTest()
{
  // Initialize factories
  registerBasicExpressionFactories();
  // Initialize infrastructure
  TrivialNodeConnector conn;
  nc = &conn;

  runTest(booleanConstantFactoryTest);
  runTest(integerConstantFactoryTest);
  runTest(realConstantFactoryTest);
  runTest(stringConstantFactoryTest);

  runTest(booleanArrayConstantFactoryTest);
  runTest(integerArrayConstantFactoryTest);
  runTest(realArrayConstantFactoryTest);
  runTest(stringArrayConstantFactoryTest);

  nc = NULL;
  return true;
}
