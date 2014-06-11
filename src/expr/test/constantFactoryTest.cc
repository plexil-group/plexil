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
#include "NodeConnector.hh"
#include "PlexilExpr.hh"
#include "TestSupport.hh"

using namespace PLEXIL;

class TrivialNodeConnector : public NodeConnector
{
public:
  TrivialNodeConnector()
    : NodeConnector()
  {
  }

  ~TrivialNodeConnector()
  {
  }

  ExpressionId const &findVariable(const PlexilVarRef* /* ref */)
  {
    return ExpressionId::noId();
  }

  ExpressionId const &findVariable(const std::string & /* name */,
                                   bool /* recursive */ = false)
  {
    return ExpressionId::noId();
  }

  NodeId const &findNodeRef(PlexilNodeRefId const & /* nodeRef */) const
  {
    return NodeId::noId();
  }

  ExecListenerHubId const &getExecListenerHub() const
  {
    return ExecListenerHubId::noId();
  }

};

// Global variable for convenience
static NodeConnectorId nc;

static bool booleanConstantFactoryTest()
{
  PlexilValue falseValue(BOOLEAN_TYPE, "false");
  PlexilValue zeroValue(BOOLEAN_TYPE, "0");
  PlexilValue trueValue(BOOLEAN_TYPE, "true");
  PlexilValue oneValue(BOOLEAN_TYPE, "1");
  PlexilValue unknownValue(BOOLEAN_TYPE);
  PlexilValue bogus(BOOLEAN_TYPE, "bogus");

  bool temp;

  ExpressionId falseConstant = createExpression(falseValue.getId(), nc, temp);
  assertTrue_1(falseConstant.isId());
  assertTrue_1(temp); // was created - may not be true in future
  assertTrue_1(!falseConstant->isAssignable());
  assertTrue_1(falseConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(falseConstant->getValue(temp));
  assertTrue_1(!temp);

  ExpressionId zeroConstant = createExpression(zeroValue.getId(), nc, temp);
  assertTrue_1(zeroConstant.isId());
  assertTrue_1(temp); // was created - may not be true in future
  assertTrue_1(!zeroConstant->isAssignable());
  assertTrue_1(zeroConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(zeroConstant->getValue(temp));
  assertTrue_1(!temp);

  ExpressionId trueConstant = createExpression(trueValue.getId(), nc, temp);
  assertTrue_1(trueConstant.isId());
  assertTrue_1(temp); // was created - may not be true in future
  assertTrue_1(!trueConstant->isAssignable());
  assertTrue_1(trueConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(trueConstant->getValue(temp));
  assertTrue_1(temp);

  ExpressionId oneConstant = createExpression(oneValue.getId(), nc, temp);
  assertTrue_1(oneConstant.isId());
  assertTrue_1(temp); // was created - may not be true in future
  assertTrue_1(!oneConstant->isAssignable());
  assertTrue_1(oneConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(oneConstant->getValue(temp));
  assertTrue_1(temp);

  ExpressionId unknownConstant = createExpression(unknownValue.getId(), nc, temp);
  assertTrue_1(unknownConstant.isId());
  assertTrue_1(temp); // was created - may not be true in future
  assertTrue_1(!unknownConstant->isAssignable());
  assertTrue_1(unknownConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(!unknownConstant->getValue(temp));

  try {
    ExpressionId bogusConstant = createExpression(bogus.getId(), nc, temp);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
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

  ExpressionId zeroConstant = createExpression(zeroVal.getId(), nc, wasCreated);
  assertTrue_1(zeroConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!zeroConstant->isAssignable());
  assertTrue_1(zeroConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(zeroConstant->getValue(temp));
  assertTrue_1(temp == 0);

  ExpressionId oneConstant = createExpression(oneVal.getId(), nc, wasCreated);
  assertTrue_1(oneConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!oneConstant->isAssignable());
  assertTrue_1(oneConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(oneConstant->getValue(temp));
  assertTrue_1(temp == 1);

  ExpressionId minusOneConstant = createExpression(minusOneVal.getId(), nc, wasCreated);
  assertTrue_1(minusOneConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!minusOneConstant->isAssignable());
  assertTrue_1(minusOneConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(minusOneConstant->getValue(temp));
  assertTrue_1(temp == -1);

  ExpressionId unkConstant = createExpression(unkVal.getId(), nc, wasCreated);
  assertTrue_1(unkConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!unkConstant->isAssignable());
  assertTrue_1(unkConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(!unkConstant->getValue(temp));

  ExpressionId hexConstant = createExpression(hexVal.getId(), nc, wasCreated);
  assertTrue_1(hexConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!hexConstant->isAssignable());
  assertTrue_1(hexConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(hexConstant->getValue(temp));
  assertTrue_1(temp == 0x42); // = 66 decimal

  try {
    ExpressionId hexWithJunkConstant = createExpression(hexWithJunkVal.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect number followed by junk");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  try {
    ExpressionId tooBigConstant = createExpression(tooBig.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect integer overflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  try {
    ExpressionId wayTooBigConstant = createExpression(wayTooBig.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect integer overflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  try {
    ExpressionId tooSmallConstant = createExpression(tooSmall.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect integer underflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  try {
    ExpressionId bogusConstant = createExpression(bogus.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
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
  PlexilValue tooSmallVal(REAL_TYPE, "1e-10000000");
  PlexilValue hexVal(REAL_TYPE, "0x42");
  PlexilValue unkVal(REAL_TYPE);
  PlexilValue bogus(REAL_TYPE, "bogus");

  bool wasCreated;
  double temp;

  ExpressionId zeroConstant = createExpression(zeroVal.getId(), nc, wasCreated);
  assertTrue_1(zeroConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!zeroConstant->isAssignable());
  assertTrue_1(zeroConstant->valueType() == REAL_TYPE);
  assertTrue_1(zeroConstant->getValue(temp));
  assertTrue_1(temp == 0);

  ExpressionId minusZeroConstant = createExpression(minusZeroVal.getId(), nc, wasCreated);
  assertTrue_1(minusZeroConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!minusZeroConstant->isAssignable());
  assertTrue_1(minusZeroConstant->valueType() == REAL_TYPE);
  assertTrue_1(minusZeroConstant->getValue(temp));
  assertTrue_1(temp == 0);

  ExpressionId oneConstant = createExpression(oneVal.getId(), nc, wasCreated);
  assertTrue_1(oneConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!oneConstant->isAssignable());
  assertTrue_1(oneConstant->valueType() == REAL_TYPE);
  assertTrue_1(oneConstant->getValue(temp));
  assertTrue_1(temp == 1);

  ExpressionId minusOneConstant = createExpression(minusOneVal.getId(), nc, wasCreated);
  assertTrue_1(minusOneConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!minusOneConstant->isAssignable());
  assertTrue_1(minusOneConstant->valueType() == REAL_TYPE);
  assertTrue_1(minusOneConstant->getValue(temp));
  assertTrue_1(temp == -1);

  ExpressionId piConstant = createExpression(piVal.getId(), nc, wasCreated);
  assertTrue_1(piConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!piConstant->isAssignable());
  assertTrue_1(piConstant->valueType() == REAL_TYPE);
  assertTrue_1(piConstant->getValue(temp));
  assertTrue_1(temp == 3.14);

  try {
    ExpressionId piWithJunkConstant = createExpression(piWithJunkVal.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect junk after valid real");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  ExpressionId expNotationConstant = createExpression(expNotationVal.getId(), nc, wasCreated);
  assertTrue_1(expNotationConstant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!expNotationConstant->isAssignable());
  assertTrue_1(expNotationConstant->valueType() == REAL_TYPE);
  assertTrue_1(expNotationConstant->getValue(temp));
  assertTrue_1(temp == 1e-100);

  try {
    ExpressionId tooBigConstant = createExpression(tooBigVal.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect real overflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  try {
    ExpressionId tooSmallValConstant = createExpression(tooSmallVal.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect real underflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  try {
    ExpressionId hexValConstant = createExpression(hexVal.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect invalid (hex) format");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  try {
    ExpressionId bogusConstant = createExpression(bogus.getId(), nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  return true;
}

static bool stringConstantFactoryTest()
{
  PlexilValue s1(STRING_TYPE, "");
  PlexilValue s2(STRING_TYPE, "foo");

  bool wasCreated;
  std::string temp;

  ExpressionId s1Constant = createExpression(s1.getId(), nc, wasCreated);
  assertTrue_1(s1Constant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(s1Constant->getValue(temp));
  assertTrue_1(temp.empty());

  ExpressionId s2Constant = createExpression(s2.getId(), nc, wasCreated);
  assertTrue_1(s2Constant.isId());
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(s2Constant->getValue(temp));
  assertTrue_1(temp == "foo");

  return true;
}

bool constantFactoryTest()
{
  // Initialize factories
  registerBasicExpressionFactories();
  // Initialize infrastructure
  TrivialNodeConnector conn;
  nc = conn.getId();

  runTest(booleanConstantFactoryTest);
  runTest(integerConstantFactoryTest);
  runTest(realConstantFactoryTest);
  runTest(stringConstantFactoryTest);

  // TODO: arrays

  return true;
}


