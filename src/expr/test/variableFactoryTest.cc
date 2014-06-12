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

class VariableFactoryTestNodeConnector : public NodeConnector
{
public:
  VariableFactoryTestNodeConnector()
    : NodeConnector()
  {
  }

  ~VariableFactoryTestNodeConnector()
  {
    m_variableMap.clear();
  }

  ExpressionId const &findVariable(const PlexilVarRef* ref)
  {
    return this->findVariable(ref->varName(), false);
  }

  ExpressionId const &findVariable(const std::string & name,
                                   bool ignored = false)
  {
    TestVariableMap::const_iterator it = m_variableMap.find(name);
    if (it != m_variableMap.end())
      return it->second;
    else
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

  // For variable lookup testing
  void storeVariable(const std::string & name, ExpressionId var)
  {
    m_variableMap.insert(std::pair<std::string, ExpressionId>(name, var));
  }

private:
  typedef std::map<std::string, ExpressionId> TestVariableMap;

  TestVariableMap m_variableMap;
};

// Global variables for convenience
static NodeConnectorId nc;
static VariableFactoryTestNodeConnector *realNc = NULL;

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
  PlexilVar xVar("x", BOOLEAN_TYPE, (new PlexilValue(BOOLEAN_TYPE, "0"))->getId());

  bool wasCreated;
  bool temp;

  ExpressionId bExp = createExpression(bVar.getId(), nc, wasCreated);
  assertTrue_1(bExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(bExp->isAssignable());
  assertTrue_1(bExp->valueType() == BOOLEAN_TYPE);
  bExp->activate();
  assertTrue_1(!bExp->isKnown());
  assertTrue_1(!bExp->getValue(temp));
  realNc->storeVariable("b", bExp);

  ExpressionId fExp = createExpression(fVar.getId(), nc, wasCreated);
  assertTrue_1(fExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(fExp->isAssignable());
  assertTrue_1(fExp->valueType() == BOOLEAN_TYPE);
  fExp->activate();
  assertTrue_1(fExp->isKnown());
  assertTrue_1(fExp->getValue(temp));
  assertTrue_1(!temp);
  realNc->storeVariable("f", fExp);

  ExpressionId tExp = createExpression(tVar.getId(), nc, wasCreated);
  assertTrue_1(tExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(tExp->isAssignable());
  assertTrue_1(tExp->valueType() == BOOLEAN_TYPE);
  tExp->activate();
  assertTrue_1(tExp->isKnown());
  assertTrue_1(tExp->getValue(temp));
  assertTrue_1(temp);
  realNc->storeVariable("t", tExp);

  ExpressionId uExp = createExpression(uVar.getId(), nc, wasCreated);
  assertTrue_1(uExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(uExp->isAssignable());
  assertTrue_1(uExp->valueType() == BOOLEAN_TYPE);
  uExp->activate();
  assertTrue_1(!uExp->isKnown());
  assertTrue_1(!uExp->getValue(temp));
  
  try {
    ExpressionId bogusExp = createExpression(bogusVar.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  ExpressionId xExp = createExpression(xVar.getId(), nc, wasCreated);
  assertTrue_1(xExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(xExp->isAssignable());
  assertTrue_1(xExp->valueType() == BOOLEAN_TYPE);
  xExp->activate();
  assertTrue_1(xExp->isKnown());
  assertTrue_1(xExp->getValue(temp));
  assertTrue_1(!temp);

  // Variable references

  PlexilVarRef bRef("b", BOOLEAN_TYPE);
  ExpressionId bExpRef = createExpression(bRef.getId(), nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(bExpRef == bExp);

  PlexilVarRef qRef("q", BOOLEAN_TYPE);
  try {
    ExpressionId qExpRef = createExpression(qRef.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilVarRef tBadRef("t", INTEGER_TYPE);
  try {
    ExpressionId tBadExpRef = createExpression(tBadRef.getId(), nc, wasCreated);
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
  PlexilVar xVar("x", INTEGER_TYPE, (new PlexilValue(INTEGER_TYPE, "0"))->getId());

  bool wasCreated;
  int32_t temp;

  ExpressionId iExp = createExpression(iVar.getId(), nc, wasCreated);
  assertTrue_1(iExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(iExp->isAssignable());
  assertTrue_1(iExp->valueType() == INTEGER_TYPE);
  iExp->activate();
  assertTrue_1(!iExp->isKnown());
  assertTrue_1(!iExp->getValue(temp));
  realNc->storeVariable("i", iExp);

  ExpressionId zeroExp = createExpression(zeroVar.getId(), nc, wasCreated);
  assertTrue_1(zeroExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(zeroExp->isAssignable());
  assertTrue_1(zeroExp->valueType() == INTEGER_TYPE);
  zeroExp->activate();
  assertTrue_1(zeroExp->isKnown());
  assertTrue_1(zeroExp->getValue(temp));
  assertTrue_1(temp == 0);
  realNc->storeVariable("z", zeroExp);

  ExpressionId tExp = createExpression(tVar.getId(), nc, wasCreated);
  assertTrue_1(tExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(tExp->isAssignable());
  assertTrue_1(tExp->valueType() == INTEGER_TYPE);
  tExp->activate();
  assertTrue_1(tExp->isKnown());
  assertTrue_1(tExp->getValue(temp));
  assertTrue_1(temp == -2000000000);
  realNc->storeVariable("t", tExp);

  ExpressionId hExp = createExpression(hVar.getId(), nc, wasCreated);
  assertTrue_1(hExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(hExp->isAssignable());
  assertTrue_1(hExp->valueType() == INTEGER_TYPE);
  hExp->activate();
  assertTrue_1(hExp->isKnown());
  assertTrue_1(hExp->getValue(temp));
  assertTrue_1(temp == 0xBADF00D);
  realNc->storeVariable("t", hExp);

  ExpressionId uExp = createExpression(uVar.getId(), nc, wasCreated);
  assertTrue_1(uExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(uExp->isAssignable());
  assertTrue_1(uExp->valueType() == INTEGER_TYPE);
  uExp->activate();
  assertTrue_1(!uExp->isKnown());
  assertTrue_1(!uExp->getValue(temp));
  
  try {
    ExpressionId bogusExp = createExpression(bogusVar.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }
  
  try {
    ExpressionId tooBigExp = createExpression(tooBigVar.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect out-of-range initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  ExpressionId xExp = createExpression(xVar.getId(), nc, wasCreated);
  assertTrue_1(xExp.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(xExp->isAssignable());
  assertTrue_1(xExp->valueType() == INTEGER_TYPE);
  xExp->activate();
  assertTrue_1(xExp->isKnown());
  assertTrue_1(xExp->getValue(temp));
  assertTrue_1(temp == 0);

  // Variable references

  PlexilVarRef iRef("i", INTEGER_TYPE);
  ExpressionId iExpRef = createExpression(iRef.getId(), nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(iExpRef == iExp);

  PlexilVarRef qRef("q", INTEGER_TYPE);
  try {
    ExpressionId qExpRef = createExpression(qRef.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  PlexilVarRef tBadRef("z", BOOLEAN_TYPE);
  try {
    ExpressionId tBadExpRef = createExpression(tBadRef.getId(), nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type conflict");
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
  realNc = new VariableFactoryTestNodeConnector();
  nc = realNc->getId();

  runTest(booleanVariableFactoryTest);
  runTest(integerVariableFactoryTest);

  nc = NodeConnectorId::noId();
  delete realNc;
  return true;
}
