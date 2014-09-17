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
#include "TestSupport.hh"

#include "pugixml.hpp"

using namespace PLEXIL;

using pugi::xml_attribute;
using pugi::xml_document;
using pugi::xml_node;
using pugi::node_pcdata;

// Global variables for convenience
static NodeConnector *nc = NULL;
static FactoryTestNodeConnector *realNc = NULL;

static bool booleanVariableXmlParserTest()
{
  bool wasCreated;
  bool temp;

  xml_document doc;
  doc.set_name("booleanVariableXmlParserTest");

  // uninitialized
  xml_node bXml = doc.append_child("DeclareVariable");
  bXml.append_child("Name").append_child(node_pcdata).set_value("b");
  bXml.append_child("Type").append_child(node_pcdata).set_value("Boolean");

  Expression *bExp = createExpression(bXml, nc, wasCreated);
  assertTrue_1(bExp);
  assertTrue_1(wasCreated);
  assertTrue_1(bExp->isAssignable());
  assertTrue_1(bExp->valueType() == BOOLEAN_TYPE);
  bExp->activate();
  assertTrue_1(!bExp->isKnown());
  assertTrue_1(!bExp->getValue(temp));
  realNc->storeVariable("b", bExp);

  // initialized
  xml_node fXml = doc.append_copy(bXml);
  fXml.first_child().first_child().set_value("f");
  xml_node tempText = fXml.append_child("InitialValue").append_child("BooleanValue").append_child(node_pcdata);
  tempText.set_value("0");

  Expression *fExp = createExpression(fXml, nc, wasCreated);
  assertTrue_1(fExp);
  assertTrue_1(wasCreated);
  assertTrue_1(fExp->isAssignable());
  assertTrue_1(fExp->valueType() == BOOLEAN_TYPE);
  fExp->activate();
  assertTrue_1(fExp->isKnown());
  assertTrue_1(fExp->getValue(temp));
  assertTrue_1(!temp);
  realNc->storeVariable("f", fExp);

  xml_node tXml = doc.append_copy(bXml);
  tXml.first_child().first_child().set_value("t");
  tempText = tXml.append_child("InitialValue").append_child("BooleanValue").append_child(node_pcdata);
  tempText.set_value("1");

  Expression *tExp = createExpression(tXml, nc, wasCreated);
  assertTrue_1(tExp);
  assertTrue_1(wasCreated);
  assertTrue_1(tExp->isAssignable());
  assertTrue_1(tExp->valueType() == BOOLEAN_TYPE);
  tExp->activate();
  assertTrue_1(tExp->isKnown());
  assertTrue_1(tExp->getValue(temp));
  assertTrue_1(temp);
  realNc->storeVariable("t", tExp);

  xml_node uXml = doc.append_copy(bXml);
  uXml.first_child().first_child().set_value("u");
  tempText = uXml.append_child("InitialValue").append_child("BooleanValue").append_child(node_pcdata);
  tempText.set_value("UNKNOWN");

  Expression *uExp = createExpression(uXml, nc, wasCreated);
  assertTrue_1(uExp);
  assertTrue_1(wasCreated);
  assertTrue_1(uExp->isAssignable());
  assertTrue_1(uExp->valueType() == BOOLEAN_TYPE);
  uExp->activate();
  assertTrue_1(!uExp->isKnown());
  assertTrue_1(!uExp->getValue(temp));

  xml_node bogusValueXml = doc.append_copy(bXml);
  bogusValueXml.first_child().first_child().set_value("bogusValue");
  tempText = bogusValueXml.append_child("InitialValue").append_child("BooleanValue").append_child(node_pcdata);
  tempText.set_value("bOgUs");

  try {
    Expression *bogusExp = createExpression(bogusValueXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  xml_node bogusTypeXml = doc.append_copy(bXml);
  bogusTypeXml.first_child().first_child().set_value("bogusType");
  tempText = bogusTypeXml.append_child("InitialValue").append_child("StringValue").append_child(node_pcdata);
  tempText.set_value("12345");
  
  try {
    Expression *bogusTypeExp = createExpression(bogusTypeXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Variable references

  xml_node bRefXml = doc.append_child("BooleanVariable");
  bRefXml.append_child(node_pcdata).set_value("b");

  Expression *bExpRef = createExpression(bRefXml, nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(bExpRef == bExp);

  xml_node qRefXml = doc.append_child("BooleanVariable");
  qRefXml.append_child(node_pcdata).set_value("q");

  try {
    Expression *qExpRef = createExpression(qRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  xml_node tBadRefXml = doc.append_child("IntegerVariable");
  tBadRefXml.append_child(node_pcdata).set_value("t");

  try {
    Expression *tBadExpRef = createExpression(tBadRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type conflict");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }
    
  return true;
}

static bool integerVariableXmlParserTest()
{
  bool wasCreated;
  int32_t temp;

  xml_document doc;
  doc.set_name("integerVariableXmlParserTest");

  // uninitialized
  xml_node iXml = doc.append_child("DeclareVariable");
  iXml.append_child("Name").append_child(node_pcdata).set_value("i");
  iXml.append_child("Type").append_child(node_pcdata).set_value("Integer");

  Expression *iExp = createExpression(iXml, nc, wasCreated);
  assertTrue_1(iExp);
  assertTrue_1(wasCreated);
  assertTrue_1(iExp->isAssignable());
  assertTrue_1(iExp->valueType() == INTEGER_TYPE);
  iExp->activate();
  assertTrue_1(!iExp->isKnown());
  assertTrue_1(!iExp->getValue(temp));
  realNc->storeVariable("i", iExp);

  // initialized
  xml_node zeroXml = doc.append_copy(iXml);
  zeroXml.first_child().first_child().set_value("z");
  xml_node tempText = zeroXml.append_child("InitialValue").append_child("IntegerValue").append_child(node_pcdata);
  tempText.set_value("0");

  Expression *zeroExp = createExpression(zeroXml, nc, wasCreated);
  assertTrue_1(zeroExp);
  assertTrue_1(wasCreated);
  assertTrue_1(zeroExp->isAssignable());
  assertTrue_1(zeroExp->valueType() == INTEGER_TYPE);
  zeroExp->activate();
  assertTrue_1(zeroExp->isKnown());
  assertTrue_1(zeroExp->getValue(temp));
  assertTrue_1(temp == 0);
  realNc->storeVariable("z", zeroExp);

  xml_node tXml = doc.append_copy(iXml);
  tXml.first_child().first_child().set_value("t");
  tempText = tXml.append_child("InitialValue").append_child("IntegerValue").append_child(node_pcdata);
  tempText.set_value("-2000000000");

  Expression *tExp = createExpression(tXml, nc, wasCreated);
  assertTrue_1(tExp);
  assertTrue_1(wasCreated);
  assertTrue_1(tExp->isAssignable());
  assertTrue_1(tExp->valueType() == INTEGER_TYPE);
  tExp->activate();
  assertTrue_1(tExp->isKnown());
  assertTrue_1(tExp->getValue(temp));
  assertTrue_1(temp == (int32_t) -2000000000);
  realNc->storeVariable("t", tExp);

  xml_node hXml = doc.append_copy(iXml);
  hXml.first_child().first_child().set_value("h");
  tempText = hXml.append_child("InitialValue").append_child("IntegerValue").append_child(node_pcdata);
  tempText.set_value("0xBADF00D");

  Expression *hExp = createExpression(hXml, nc, wasCreated);
  assertTrue_1(hExp);
  assertTrue_1(wasCreated);
  assertTrue_1(hExp->isAssignable());
  assertTrue_1(hExp->valueType() == INTEGER_TYPE);
  hExp->activate();
  assertTrue_1(hExp->isKnown());
  assertTrue_1(hExp->getValue(temp));
  assertTrue_1(temp == 0xBADF00D);
  realNc->storeVariable("t", hExp);

  xml_node uXml = doc.append_copy(iXml);
  uXml.first_child().first_child().set_value("u");
  tempText = uXml.append_child("InitialValue").append_child("IntegerValue").append_child(node_pcdata);
  tempText.set_value("UNKNOWN");

  Expression *uExp = createExpression(uXml, nc, wasCreated);
  assertTrue_1(uExp);
  assertTrue_1(wasCreated);
  assertTrue_1(uExp->isAssignable());
  assertTrue_1(uExp->valueType() == INTEGER_TYPE);
  uExp->activate();
  assertTrue_1(!uExp->isKnown());
  assertTrue_1(!uExp->getValue(temp));

  xml_node bogusValueXml = doc.append_copy(iXml);
  bogusValueXml.first_child().first_child().set_value("bogusValue");
  tempText = bogusValueXml.append_child("InitialValue").append_child("IntegerValue").append_child(node_pcdata);
  tempText.set_value("bOgUs");
  
  try {
    Expression *bogusValueExp = createExpression(bogusValueXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  xml_node bogusTypeXml = doc.append_copy(iXml);
  bogusTypeXml.first_child().first_child().set_value("bogusType");
  tempText = bogusTypeXml.append_child("InitialValue").append_child("StringValue").append_child(node_pcdata);
  tempText.set_value("12345");
  
  try {
    Expression *bogusTypeExp = createExpression(bogusTypeXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }
  
#if !defined(__VXWORKS__)
  xml_node tooBigXml = doc.append_copy(iXml);
  tooBigXml.first_child().first_child().set_value("tooBig");
  tempText = tooBigXml.append_child("InitialValue").append_child("IntegerValue").append_child(node_pcdata);
  tempText.set_value("3000000000");

  try {
    Expression *tooBigExp = createExpression(tooBigXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect out-of-range initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }
#endif // !defined(__VXWORKS__)

  // Variable references

  xml_node iRefXml = doc.append_child("IntegerVariable");
  iRefXml.append_child(node_pcdata).set_value("i");

  Expression *iExpRef = createExpression(iRefXml, nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(iExpRef == iExp);

  xml_node qRefXml = doc.append_child("IntegerVariable");
  qRefXml.append_child(node_pcdata).set_value("q");
  try {
    Expression *qExpRef = createExpression(qRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  xml_node zBadRefXml = doc.append_child("BooleanVariable");
  zBadRefXml.append_child(node_pcdata).set_value("z");
  try {
    Expression *tBadExpRef = createExpression(zBadRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type conflict");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }
    
  return true;
}

static bool realVariableXmlParserTest()
{
  bool wasCreated;
  double temp;

  xml_document doc;
  doc.set_name("realVariableXmlParserTest");

  // uninitialized
  xml_node iXml = doc.append_child("DeclareVariable");
  iXml.append_child("Name").append_child(node_pcdata).set_value("i");
  iXml.append_child("Type").append_child(node_pcdata).set_value("Real");

  Expression *iExp = createExpression(iXml, nc, wasCreated);
  assertTrue_1(iExp);
  assertTrue_1(wasCreated);
  assertTrue_1(iExp->isAssignable());
  assertTrue_1(iExp->valueType() == REAL_TYPE);
  iExp->activate();
  assertTrue_1(!iExp->isKnown());
  assertTrue_1(!iExp->getValue(temp));
  realNc->storeVariable("i", iExp);

  // initialized
  xml_node zeroXml = doc.append_copy(iXml);
  zeroXml.first_child().first_child().set_value("z");
  xml_node tempText = zeroXml.append_child("InitialValue").append_child("RealValue").append_child(node_pcdata);
  tempText.set_value("0");

  Expression *zeroExp = createExpression(zeroXml, nc, wasCreated);
  assertTrue_1(zeroExp);
  assertTrue_1(wasCreated);
  assertTrue_1(zeroExp->isAssignable());
  assertTrue_1(zeroExp->valueType() == REAL_TYPE);
  zeroExp->activate();
  assertTrue_1(zeroExp->isKnown());
  assertTrue_1(zeroExp->getValue(temp));
  assertTrue_1(temp == 0);
  realNc->storeVariable("z", zeroExp);

  xml_node tXml = doc.append_copy(iXml);
  tXml.first_child().first_child().set_value("t");
  tempText = tXml.append_child("InitialValue").append_child("RealValue").append_child(node_pcdata);
  tempText.set_value("-2e10");

  Expression *tExp = createExpression(tXml, nc, wasCreated);
  assertTrue_1(tExp);
  assertTrue_1(wasCreated);
  assertTrue_1(tExp->isAssignable());
  assertTrue_1(tExp->valueType() == REAL_TYPE);
  tExp->activate();
  assertTrue_1(tExp->isKnown());
  assertTrue_1(tExp->getValue(temp));
  assertTrue_1(temp == -20000000000.0);
  realNc->storeVariable("t", tExp);

  xml_node piXml = doc.append_copy(iXml);
  piXml.first_child().first_child().set_value("pi");
  tempText = piXml.append_child("InitialValue").append_child("RealValue").append_child(node_pcdata);
  tempText.set_value("3.1415");

  Expression *piExp = createExpression(piXml, nc, wasCreated);
  assertTrue_1(piExp);
  assertTrue_1(wasCreated);
  assertTrue_1(piExp->isAssignable());
  assertTrue_1(piExp->valueType() == REAL_TYPE);
  piExp->activate();
  assertTrue_1(piExp->isKnown());
  assertTrue_1(piExp->getValue(temp));
  assertTrue_1(temp == 3.1415);
  realNc->storeVariable("pi", piExp);

  xml_node uXml = doc.append_copy(iXml);
  uXml.first_child().first_child().set_value("u");
  tempText = uXml.append_child("InitialValue").append_child("RealValue").append_child(node_pcdata);
  tempText.set_value("UNKNOWN");

  Expression *uExp = createExpression(uXml, nc, wasCreated);
  assertTrue_1(uExp);
  assertTrue_1(wasCreated);
  assertTrue_1(uExp->isAssignable());
  assertTrue_1(uExp->valueType() == REAL_TYPE);
  uExp->activate();
  assertTrue_1(!uExp->isKnown());
  assertTrue_1(!uExp->getValue(temp));
  
  xml_node bogusValueXml = doc.append_copy(iXml);
  bogusValueXml.first_child().first_child().set_value("bogusValue");
  tempText = bogusValueXml.append_child("InitialValue").append_child("RealValue").append_child(node_pcdata);
  tempText.set_value("bOgUs");

  try {
    Expression *bogusExp = createExpression(bogusValueXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  xml_node bogusTypeXml = doc.append_copy(iXml);
  bogusTypeXml.first_child().first_child().set_value("bogusType");
  tempText = bogusTypeXml.append_child("InitialValue").append_child("StringValue").append_child(node_pcdata);
  tempText.set_value("12345");
  
  try {
    Expression *bogusTypeExp = createExpression(bogusTypeXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

#if !defined(__VXWORKS__)
  xml_node tooBigXml = doc.append_copy(iXml);
  tooBigXml.first_child().first_child().set_value("tooBig");
  tempText = tooBigXml.append_child("InitialValue").append_child("RealValue").append_child(node_pcdata);
  tempText.set_value("3e1000000000");

  try {
    Expression *tooBigExp = createExpression(tooBigXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect out-of-range initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }
#endif // !defined(__VXWORKS__)

  // Variable references

  xml_node iRefXml = doc.append_child("RealVariable");
  iRefXml.append_child(node_pcdata).set_value("i");

  Expression *iExpRef = createExpression(iRefXml, nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(iExpRef);
  assertTrue_1(iExpRef == iExp);

  xml_node qRefXml = doc.append_child("RealVariable");
  qRefXml.append_child(node_pcdata).set_value("q");
  try {
    Expression *qExpRef = createExpression(qRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  xml_node zBadRefXml = doc.append_child("BooleanVariable");
  zBadRefXml.append_child(node_pcdata).set_value("z");
  try {
    Expression *zBadExpRef = createExpression(zBadRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type conflict");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }
    
  return true;
}

static bool stringVariableXmlParserTest()
{
  bool wasCreated;
  std::string const *temp = NULL;

  xml_document doc;
  doc.set_name("realVariableXmlParserTest");

  // uninitialized
  xml_node unkXml = doc.append_child("DeclareVariable");
  unkXml.append_child("Name").append_child(node_pcdata).set_value("unk");
  unkXml.append_child("Type").append_child(node_pcdata).set_value("String");

  Expression *unkExp = createExpression(unkXml, nc, wasCreated);
  assertTrue_1(unkExp);
  assertTrue_1(wasCreated);
  assertTrue_1(unkExp->isAssignable());
  assertTrue_1(unkExp->valueType() == STRING_TYPE);
  unkExp->activate();
  assertTrue_1(!unkExp->isKnown());
  assertTrue_1(!unkExp->getValuePointer(temp));
  assertTrue_1(temp == NULL);
  realNc->storeVariable("unk", unkExp);

  // initialized empty
  xml_node mtXml = doc.append_copy(unkXml);
  mtXml.first_child().first_child().set_value("mt");
  mtXml.append_child("InitialValue").append_child("StringValue").append_child(node_pcdata);

  Expression *mtExp = createExpression(mtXml, nc, wasCreated);
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

  // initialized to literal
  xml_node fooXml = doc.append_copy(unkXml);
  fooXml.first_child().first_child().set_value("foo");
  xml_node tempText = fooXml.append_child("InitialValue").append_child("StringValue").append_child(node_pcdata);
  tempText.set_value("Foo!");

  Expression *fooExp = createExpression(fooXml, nc, wasCreated);
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

  // initialized to var ref
  xml_node barXml = doc.append_copy(unkXml);
  barXml.first_child().first_child().set_value("bar");
  barXml.append_child("InitialValue").append_child("StringVariable").append_child(node_pcdata).set_value("foo");

  Expression *barExp = createExpression(barXml, nc, wasCreated);
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

  xml_node bogusTypeXml = doc.append_copy(unkXml);
  bogusTypeXml.first_child().first_child().set_value("bogusType");
  bogusTypeXml.append_child("InitialValue").append_child("IntegerValue").append_child(node_pcdata).set_value("12345");
  
  try {
    Expression *bogusTypeExp = createExpression(bogusTypeXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid initial value");
  }
  catch (ParserException const & /*exc*/) {
    std::cout << "Caught expected exception" << std::endl;
  }

  // Variable references

  xml_node unkRefXml = doc.append_child("StringVariable");
  unkRefXml.append_child(node_pcdata).set_value("unk");

  Expression *unkRefExp = createExpression(unkRefXml, nc, wasCreated);
  assertTrue_1(!wasCreated);
  assertTrue_1(unkRefExp);
  assertTrue_1(unkRefExp = unkExp);

  xml_node badRefXml = doc.append_child("StringVariable");
  badRefXml.append_child(node_pcdata).set_value("bad");

  try {
    Expression *badRefExp = createExpression(badRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect nonexistent variable");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  xml_node badTypeRefXml = doc.append_child("BooleanVariable");
  badTypeRefXml.append_child(node_pcdata).set_value("mt");

  try {
    Expression *badTypeRefExp = createExpression(badTypeRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type conflict");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool booleanArrayVariableXmlParserTest()
{
  bool wasCreated, temp;
  BooleanArray const *aryTemp = NULL;

  xml_document doc;
  doc.set_name("booleanArrayVariableXmlParserTest");

  xml_node ba1Xml = doc.append_child("DeclareArray");
  ba1Xml.append_child("Name").append_child(node_pcdata).set_value("ba1");
  ba1Xml.append_child("Type").append_child(node_pcdata).set_value("Boolean");
  ba1Xml.append_child("MaxSize").append_child(node_pcdata).set_value("2");

  Expression *ba1Exp = createExpression(ba1Xml, nc, wasCreated);
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

  xml_node ba2Xml = doc.append_child("DeclareArray");
  ba2Xml.append_child("Name").append_child(node_pcdata).set_value("ba2");
  ba2Xml.append_child("Type").append_child(node_pcdata).set_value("Boolean");
  ba2Xml.append_child("MaxSize").append_child(node_pcdata).set_value("7");
  xml_node initvals = ba2Xml.append_child("InitialValue").append_child("ArrayValue");
  initvals.append_attribute("Type").set_value("Boolean");
  initvals.append_child("BooleanValue").append_child(node_pcdata).set_value("0");
  initvals.append_child("BooleanValue").append_child(node_pcdata).set_value("1");
  initvals.append_child("BooleanValue").append_child(node_pcdata).set_value("UNKNOWN");
  initvals.append_child("BooleanValue").append_child(node_pcdata).set_value("true");
  initvals.append_child("BooleanValue").append_child(node_pcdata).set_value("false");
  initvals.append_child("BooleanValue").append_child(node_pcdata).set_value("FALSE");
  initvals.append_child("BooleanValue").append_child(node_pcdata).set_value("TRUE");

  Expression *ba2Exp = createExpression(ba2Xml, nc, wasCreated);
  assertTrue_1(wasCreated);
  assertTrue_1(ba2Exp);
  assertTrue_1(ba2Exp->valueType() == BOOLEAN_ARRAY_TYPE);
  ba2Exp->activate();
  assertTrue_1(ba2Exp->isKnown());
  assertTrue_1(ba2Exp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp);
  assertTrue_1(aryTemp->size() == 7);
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

  // Variable refs
  
  xml_node ba1RefXml = doc.append_child("ArrayVariable");
  ba1RefXml.append_child(node_pcdata).set_value("ba1");

  Expression *ba1RefExp = createExpression(ba1RefXml, nc, wasCreated);
  assertTrue_1(ba1RefExp);
  assertTrue_1(!wasCreated);
  assertTrue_1(ba1RefExp == ba1Exp);

  xml_node ba2RefXml = doc.append_child("BooleanVariable");
  ba2RefXml.append_child(node_pcdata).set_value("ba2");

  try {
    Expression *ba2RefExp = createExpression(ba2RefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type mismatch");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  xml_node badRefXml = doc.append_child("ArrayVariable");
  badRefXml.append_child(node_pcdata).set_value("bad");

  try {
    Expression *badRefExp = createExpression(badRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid variable reference");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  return true;
}

static bool integerArrayVariableXmlParserTest()
{
  bool wasCreated;
  IntegerArray const *aryTemp = NULL;

  xml_document doc;
  doc.set_name("integerArrayVariableXmlParserTest");

  xml_node emptyXml = doc.append_child("DeclareArray");
  emptyXml.append_child("Name").append_child(node_pcdata).set_value("empty");
  emptyXml.append_child("Type").append_child(node_pcdata).set_value("Integer");
  emptyXml.append_child("MaxSize").append_child(node_pcdata).set_value("2");
  emptyXml.append_child("InitialValue").append_child("ArrayValue").append_attribute("Type").set_value("Integer");

  Expression *emptyExp = createExpression(emptyXml, nc, wasCreated);
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

  xml_node validXml = doc.append_child("DeclareArray");
  validXml.append_child("Name").append_child(node_pcdata).set_value("valid");
  validXml.append_child("Type").append_child(node_pcdata).set_value("Integer");
  validXml.append_child("MaxSize").append_child(node_pcdata).set_value("6");
  xml_node validContents = validXml.append_child("InitialValue").append_child("ArrayValue");
  validContents.append_attribute("Type").set_value("Integer");
  validContents.append_child("IntegerValue").append_child(node_pcdata).set_value("0");
  validContents.append_child("IntegerValue").append_child(node_pcdata).set_value("1");
  validContents.append_child("IntegerValue").append_child(node_pcdata).set_value("UNKNOWN");
  validContents.append_child("IntegerValue").append_child(node_pcdata).set_value("-123456789");
  validContents.append_child("IntegerValue").append_child(node_pcdata).set_value("987654321");
  validContents.append_child("IntegerValue").append_child(node_pcdata).set_value("0x69");

  Expression *validValExp = createExpression(validXml, nc, wasCreated);
  assertTrue_1(validValExp);
  assertTrue_1(wasCreated);
  assertTrue_1(validValExp->valueType() == INTEGER_ARRAY_TYPE);
  validValExp->activate();
  assertTrue_1(validValExp->isKnown());
  assertTrue_1(validValExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 6);
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

  xml_node bogusValueXml = doc.append_child("DeclareArray");
  bogusValueXml.append_child("Name").append_child(node_pcdata).set_value("bogusValue");
  bogusValueXml.append_child("Type").append_child(node_pcdata).set_value("Integer");
  bogusValueXml.append_child("MaxSize").append_child(node_pcdata).set_value("1");
  xml_node bogusContents = bogusValueXml.append_child("InitialValue").append_child("ArrayValue");
  bogusContents.append_attribute("Type").set_value("Integer");
  bogusContents.append_child("IntegerValue").append_child(node_pcdata).set_value("bOgUs");

  try {
    Expression *bogusValueExp = createExpression(bogusValueXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus element value");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  xml_node tooLongValueXml = doc.append_child("DeclareArray");
  tooLongValueXml.append_child("Name").append_child(node_pcdata).set_value("tooLongValue");
  tooLongValueXml.append_child("Type").append_child(node_pcdata).set_value("Integer");
  tooLongValueXml.append_child("MaxSize").append_child(node_pcdata).set_value("1");
  xml_node tooLongContents = tooLongValueXml.append_child("InitialValue").append_child("ArrayValue");
  tooLongContents.append_attribute("Type").set_value("Integer");
  tooLongContents.append_child("IntegerValue").append_child(node_pcdata).set_value("1");
  tooLongContents.append_child("IntegerValue").append_child(node_pcdata).set_value("2");

  try {
    Expression *tooLongValueExp = createExpression(tooLongValueXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect oversized initial value");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  xml_node rangeErrXml = doc.append_child("DeclareArray");
  rangeErrXml.append_child("Name").append_child(node_pcdata).set_value("rangeErr");
  rangeErrXml.append_child("Type").append_child(node_pcdata).set_value("Integer");
  rangeErrXml.append_child("MaxSize").append_child(node_pcdata).set_value("1");
  bogusContents = rangeErrXml.append_child("InitialValue").append_child("ArrayValue");
  bogusContents.append_attribute("Type").set_value("Integer");
  bogusContents.append_child("IntegerValue").append_child(node_pcdata).set_value("-3000000000");

  try {
    Expression *rangeErrExp = createExpression(rangeErrXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect out-of-range integer");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  // Variable reference tests

  xml_node validRefXml = doc.append_child("ArrayVariable");
  validRefXml.append_child(node_pcdata).set_value("valid");

  Expression *validRefExp = createExpression(validRefXml, nc, wasCreated);
  assertTrue_1(validRefExp);
  assertTrue_1(!wasCreated);
  assertTrue_1(validRefExp == validValExp);

  xml_node badNameRefXml = doc.append_child("ArrayVariable");
  badNameRefXml.append_child(node_pcdata).set_value("bad");

  try {
    Expression *badNameExp = createExpression(badNameRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid variable reference");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  xml_node wrongTypeRefXml = doc.append_child("IntegerVariable");
  wrongTypeRefXml.append_child(node_pcdata).set_value("empty");

  try {
    Expression *wrongTypeExp = createExpression(wrongTypeRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type mismatch");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  return true;
}

static bool realArrayVariableXmlParserTest()
{
  bool wasCreated;
  RealArray const *aryTemp = NULL;
  double temp;

  xml_document doc;
  doc.set_name("realArrayVariableXmlParserTest");

  xml_node emptyXml = doc.append_child("DeclareArray");
  emptyXml.append_child("Name").append_child(node_pcdata).set_value("empty");
  emptyXml.append_child("Type").append_child(node_pcdata).set_value("Real");
  emptyXml.append_child("MaxSize").append_child(node_pcdata).set_value("0");
  emptyXml.append_child("InitialValue").append_child("ArrayValue").append_attribute("Type").set_value("Real");

  Expression *emptyExp = createExpression(emptyXml, nc, wasCreated);
  assertTrue_1(emptyExp);
  assertTrue_1(wasCreated);
  assertTrue_1(emptyExp->valueType() == REAL_ARRAY_TYPE);
  emptyExp->activate();
  assertTrue_1(emptyExp->isKnown());
  assertTrue_1(emptyExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 0);
  realNc->storeVariable("emmty", emptyExp);

  xml_node validXml = doc.append_child("DeclareArray");
  validXml.append_child("Name").append_child(node_pcdata).set_value("vallid");
  validXml.append_child("Type").append_child(node_pcdata).set_value("Real");
  validXml.append_child("MaxSize").append_child(node_pcdata).set_value("6");
  xml_node contentsElt = validXml.append_child("InitialValue").append_child("ArrayValue");
  contentsElt.append_attribute("Type").set_value("Real");
  contentsElt.append_child("RealValue").append_child(node_pcdata).set_value("0");
  contentsElt.append_child("RealValue").append_child(node_pcdata).set_value("1");
  contentsElt.append_child("RealValue").append_child(node_pcdata).set_value("UNKNOWN");
  contentsElt.append_child("RealValue").append_child(node_pcdata).set_value("3.14");
  contentsElt.append_child("RealValue").append_child(node_pcdata).set_value("1e-100");
  contentsElt.append_child("RealValue").append_child(node_pcdata).set_value("6.0221413e+23");

  Expression *validVarExp = createExpression(validXml, nc, wasCreated);
  assertTrue_1(validVarExp);
  assertTrue_1(wasCreated);
  assertTrue_1(validVarExp->valueType() == REAL_ARRAY_TYPE);
  validVarExp->activate();
  assertTrue_1(validVarExp->isKnown());
  assertTrue_1(validVarExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 6);
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

  xml_node bogusValueXml = doc.append_child("DeclareArray");
  bogusValueXml.append_child("Name").append_child(node_pcdata).set_value("bogusValue");
  bogusValueXml.append_child("Type").append_child(node_pcdata).set_value("Real");
  bogusValueXml.append_child("MaxSize").append_child(node_pcdata).set_value("1");
  contentsElt = bogusValueXml.append_child("InitialValue").append_child("ArrayValue");
  contentsElt.append_attribute("Type").set_value("Real");
  contentsElt.append_child("RealValue").append_child(node_pcdata).set_value("bOgUs");

  try {
    Expression *bogusValueExp = createExpression(bogusValueXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

#if !defined(__VXWORKS__)
  xml_node rangeErrXml = doc.append_child("DeclareArray");
  rangeErrXml.append_child("Name").append_child(node_pcdata).set_value("rangeErr");
  rangeErrXml.append_child("Type").append_child(node_pcdata).set_value("Real");
  rangeErrXml.append_child("MaxSize").append_child(node_pcdata).set_value("0");
  contentsElt = rangeErrXml.append_child("InitialValue").append_child("ArrayValue");
  contentsElt.append_attribute("Type").set_value("Real");
  contentsElt.append_child("RealValue").append_child(node_pcdata).set_value("-3e1000000000");

  try {
    Expression *rangeErrExp = createExpression(rangeErrXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect out-of-range real");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }
#endif // !defined(__VXWORKS__)

  // Variable reference tests

  xml_node validRefXml = doc.append_child("ArrayVariable");
  validRefXml.append_child(node_pcdata).set_value("vallid");

  Expression *validRefExp = createExpression(validRefXml, nc, wasCreated);
  assertTrue_1(validRefExp);
  assertTrue_1(!wasCreated);
  assertTrue_1(validRefExp == validVarExp);

  xml_node badNameRefXml = doc.append_child("ArrayVariable");
  badNameRefXml.append_child(node_pcdata).set_value("bad");
  try {
    Expression *badNameExp = createExpression(badNameRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid variable reference");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  xml_node wrongTypeRefXml = doc.append_child("RealVariable");
  wrongTypeRefXml.append_child(node_pcdata).set_value("empty");

  try {
    Expression *wrongTypeExp = createExpression(wrongTypeRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type mismatch");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected error" << std::endl;
  }

  return true;
}

static bool stringArrayVariableXmlParserTest()
{
  bool wasCreated;
  StringArray const *aryTemp = NULL;
  std::string const *temp;

  xml_document doc;
  doc.set_name("realArrayVariableXmlParserTest");

  xml_node emptyXml = doc.append_child("DeclareArray");
  emptyXml.append_child("Name").append_child(node_pcdata).set_value("mty");
  emptyXml.append_child("Type").append_child(node_pcdata).set_value("String");
  emptyXml.append_child("MaxSize").append_child(node_pcdata).set_value("0");
  emptyXml.append_child("InitialValue").append_child("ArrayValue").append_attribute("Type").set_value("String");

  Expression *emptyExp = createExpression(emptyXml, nc, wasCreated);
  assertTrue_1(emptyExp);
  assertTrue_1(wasCreated);
  assertTrue_1(emptyExp->valueType() == STRING_ARRAY_TYPE);
  emptyExp->activate();
  assertTrue_1(emptyExp->isKnown());
  assertTrue_1(emptyExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 0);
  realNc->storeVariable("mty", emptyExp);

  xml_node validValXml = doc.append_child("DeclareArray");
  validValXml.append_child("Name").append_child(node_pcdata).set_value("vlaid");
  validValXml.append_child("Type").append_child(node_pcdata).set_value("String");
  validValXml.append_child("MaxSize").append_child(node_pcdata).set_value("6");
  xml_node contentsElt = validValXml.append_child("InitialValue").append_child("ArrayValue");
  contentsElt.append_attribute("Type").set_value("String");
  contentsElt.append_child("StringValue").append_child(node_pcdata).set_value("0");
  contentsElt.append_child("StringValue").append_child(node_pcdata).set_value("1");
  contentsElt.append_child("StringValue").append_child(node_pcdata).set_value("UNKNOWN");
  contentsElt.append_child("StringValue").append_child(node_pcdata).set_value("3.14");
  contentsElt.append_child("StringValue").append_child(node_pcdata).set_value("1e-100");
  contentsElt.append_child("StringValue").append_child(node_pcdata).set_value("6.0221413e+23");

  Expression *validValExp = createExpression(validValXml, nc, wasCreated);
  assertTrue_1(validValExp);
  assertTrue_1(wasCreated);
  assertTrue_1(validValExp->valueType() == STRING_ARRAY_TYPE);
  validValExp->activate();
  assertTrue_1(validValExp->isKnown());
  assertTrue_1(validValExp->getValuePointer(aryTemp));
  assertTrue_1(aryTemp != NULL);
  assertTrue_1(aryTemp->size() == 6);
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

  xml_node emptyRefXml = doc.append_child("ArrayVariable");
  emptyRefXml.append_child(node_pcdata).set_value("mty");

  Expression *emptyRefExp = createExpression(emptyRefXml, nc, wasCreated);
  assertTrue_1(emptyRefExp);
  assertTrue_1(!wasCreated);
  assertTrue_1(emptyRefExp == emptyExp);

  xml_node wrongTypeRefXml = doc.append_child("StringVariable");
  wrongTypeRefXml.append_child(node_pcdata).set_value("vlaid");

  try {
    Expression *wrongTypeExp = createExpression(wrongTypeRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect variable type mismatch");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  xml_node badRefXml = doc.append_child("ArrayVariable");
  badRefXml.append_child(node_pcdata).set_value("bad");

  try {
    Expression *badExp = createExpression(badRefXml, nc, wasCreated);
    assertTrue_2(false, "Failed to detect invalid variable reference");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

bool variableXmlParserTest()
{
  // Initialize factories
  registerBasicExpressionFactories();
  // Initialize infrastructure
  realNc = new FactoryTestNodeConnector();
  nc = realNc;

  runTest(booleanVariableXmlParserTest);
  runTest(integerVariableXmlParserTest);
  runTest(realVariableXmlParserTest);
  runTest(stringVariableXmlParserTest);

  runTest(booleanArrayVariableXmlParserTest);
  runTest(integerArrayVariableXmlParserTest);
  runTest(realArrayVariableXmlParserTest);
  runTest(stringArrayVariableXmlParserTest);

  nc = NULL;
  delete realNc;
  return true;
}
