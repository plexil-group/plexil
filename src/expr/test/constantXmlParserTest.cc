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
#include "TestSupport.hh"
#include "TrivialNodeConnector.hh"

#include "pugixml.hpp"

using namespace PLEXIL;

using pugi::xml_document;
using pugi::xml_node;
using pugi::node_pcdata;


// Global variable for convenience
static NodeConnector *nc = NULL;

static bool booleanConstantXmlParserTest()
{
  xml_document doc;
  doc.set_name("booleanConstantXmlParserTest");

  bool wasCreated;
  bool temp;

  pugi::xml_node falseXml = doc.append_child();
  falseXml.set_name("BooleanValue");
  pugi::xml_node falseText = falseXml.append_child(node_pcdata);
  falseText.set_value("false");

  Expression *falseConstant = createExpression(falseXml, nc, wasCreated);
  assertTrue_1(falseConstant);
  assertTrue_1(!wasCreated);
  assertTrue_1(!falseConstant->isAssignable());
  assertTrue_1(falseConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(falseConstant->getValue(temp));
  assertTrue_1(!temp);

  pugi::xml_node zeroXml = doc.append_child();
  zeroXml.set_name("BooleanValue");
  pugi::xml_node zeroText = zeroXml.append_child(node_pcdata);
  zeroText.set_value("0");

  Expression *zeroConstant = createExpression(zeroXml, nc, wasCreated);
  assertTrue_1(zeroConstant);
  assertTrue_1(!wasCreated);
  assertTrue_1(!zeroConstant->isAssignable());
  assertTrue_1(zeroConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(zeroConstant->getValue(temp));
  assertTrue_1(!temp);

  pugi::xml_node trueXml = doc.append_child();
  trueXml.set_name("BooleanValue");
  pugi::xml_node trueText = trueXml.append_child(node_pcdata);
  trueText.set_value("true");

  Expression *trueConstant = createExpression(trueXml, nc, wasCreated);
  assertTrue_1(trueConstant);
  assertTrue_1(!wasCreated);
  assertTrue_1(!trueConstant->isAssignable());
  assertTrue_1(trueConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(trueConstant->getValue(temp));
  assertTrue_1(temp);

  pugi::xml_node oneXml = doc.append_child();
  oneXml.set_name("BooleanValue");
  pugi::xml_node oneText = oneXml.append_child(node_pcdata);
  oneText.set_value("1");

  Expression *oneConstant = createExpression(trueXml, nc, wasCreated);
  assertTrue_1(oneConstant);
  assertTrue_1(!wasCreated);
  assertTrue_1(!oneConstant->isAssignable());
  assertTrue_1(oneConstant->valueType() == BOOLEAN_TYPE);
  assertTrue_1(oneConstant->getValue(temp));
  assertTrue_1(temp);

  pugi::xml_node bogusXml = doc.append_child();
  bogusXml.set_name("BooleanValue");
  pugi::xml_node bogusText = bogusXml.append_child(node_pcdata);
  bogusText.set_value("bogus");

  try {
    Expression *bogusConstant = createExpression(bogusXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool integerConstantXmlParserTest()
{
  xml_document doc;
  doc.set_name("integerConstantXmlParserTest");

  bool wasCreated;
  int32_t temp;

  pugi::xml_node zeroXml = doc.append_child();
  zeroXml.set_name("IntegerValue");
  pugi::xml_node zeroText = zeroXml.append_child(node_pcdata);
  zeroText.set_value("0");

  Expression *zeroConstant = createExpression(zeroXml, nc, wasCreated);
  assertTrue_1(zeroConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!zeroConstant->isAssignable());
  assertTrue_1(zeroConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(zeroConstant->getValue(temp));
  assertTrue_1(temp == 0);

  pugi::xml_node oneXml = doc.append_child();
  oneXml.set_name("IntegerValue");
  pugi::xml_node oneText = oneXml.append_child(node_pcdata);
  oneText.set_value("1");

  Expression *oneConstant = createExpression(oneXml, nc, wasCreated);
  assertTrue_1(oneConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!oneConstant->isAssignable());
  assertTrue_1(oneConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(oneConstant->getValue(temp));
  assertTrue_1(temp == 1);

  pugi::xml_node minusOneXml = doc.append_child();
  minusOneXml.set_name("IntegerValue");
  pugi::xml_node minusOneText = minusOneXml.append_child(node_pcdata);
  minusOneText.set_value("-1");

  Expression *minusOneConstant = createExpression(minusOneXml, nc, wasCreated);
  assertTrue_1(minusOneConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!minusOneConstant->isAssignable());
  assertTrue_1(minusOneConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(minusOneConstant->getValue(temp));
  assertTrue_1(temp == -1);

  pugi::xml_node hexXml = doc.append_child();
  hexXml.set_name("IntegerValue");
  pugi::xml_node hexText = hexXml.append_child(node_pcdata);
  hexText.set_value("0x42");

  Expression *hexConstant = createExpression(hexXml, nc, wasCreated);
  assertTrue_1(hexConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!hexConstant->isAssignable());
  assertTrue_1(hexConstant->valueType() == INTEGER_TYPE);
  assertTrue_1(hexConstant->getValue(temp));
  assertTrue_1(temp == 0x42); // = 66 decimal

  pugi::xml_node hexWithJunkXml = doc.append_child();
  hexWithJunkXml.set_name("IntegerValue");
  pugi::xml_node hexWithJunkText = hexWithJunkXml.append_child(node_pcdata);
  hexWithJunkText.set_value("0x42r");

  try {
    Expression *hexWithJunkConstant = createExpression(hexWithJunkXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect number followed by junk");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  pugi::xml_node tooBigXml = doc.append_child();
  tooBigXml.set_name("IntegerValue");
  pugi::xml_node tooBigText = tooBigXml.append_child(node_pcdata);
  tooBigText.set_value("3000000000");

  try {
    Expression *tooBigConstant = createExpression(tooBigXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect integer overflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  pugi::xml_node wayTooBigXml = doc.append_child();
  wayTooBigXml.set_name("IntegerValue");
  pugi::xml_node wayTooBigText = wayTooBigXml.append_child(node_pcdata);
  wayTooBigText.set_value("0x30000000000000000000000");

  try {
    Expression *wayTooBigConstant = createExpression(wayTooBigXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect integer overflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  pugi::xml_node tooSmallXml = doc.append_child();
  tooSmallXml.set_name("IntegerValue");
  pugi::xml_node tooSmallText = tooSmallXml.append_child(node_pcdata);
  tooSmallText.set_value("-3000000000");

  try {
    Expression *tooSmallConstant = createExpression(tooSmallXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect integer underflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  pugi::xml_node emptyXml = doc.append_child();
  emptyXml.set_name("IntegerValue");
  emptyXml.append_child(node_pcdata);

  try {
    Expression *emptyConstant = createExpression(emptyXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect empty input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  pugi::xml_node bogusXml = doc.append_child();
  bogusXml.set_name("IntegerValue");
  pugi::xml_node bogusText = bogusXml.append_child(node_pcdata);
  bogusText.set_value("bogus");

  try {
    Expression *bogusConstant = createExpression(bogusXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool realConstantXmlParserTest()
{
  xml_document doc;
  doc.set_name("realConstantXmlParserTest");

  bool wasCreated;
  double temp;

  pugi::xml_node zeroXml = doc.append_child();
  zeroXml.set_name("RealValue");
  pugi::xml_node zeroText = zeroXml.append_child(node_pcdata);
  zeroText.set_value("0");

  Expression *zeroConstant = createExpression(zeroXml, nc, wasCreated);
  assertTrue_1(zeroConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!zeroConstant->isAssignable());
  assertTrue_1(zeroConstant->valueType() == REAL_TYPE);
  assertTrue_1(zeroConstant->getValue(temp));
  assertTrue_1(temp == 0);

  pugi::xml_node minusZeroXml = doc.append_child();
  minusZeroXml.set_name("RealValue");
  pugi::xml_node minusZeroText = minusZeroXml.append_child(node_pcdata);
  minusZeroText.set_value("-0");

  Expression *minusZeroConstant = createExpression(minusZeroXml, nc, wasCreated);
  assertTrue_1(minusZeroConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!minusZeroConstant->isAssignable());
  assertTrue_1(minusZeroConstant->valueType() == REAL_TYPE);
  assertTrue_1(minusZeroConstant->getValue(temp));
  assertTrue_1(temp == 0);

  pugi::xml_node oneXml = doc.append_child();
  oneXml.set_name("RealValue");
  pugi::xml_node oneText = oneXml.append_child(node_pcdata);
  oneText.set_value("1");

  Expression *oneConstant = createExpression(oneXml, nc, wasCreated);
  assertTrue_1(oneConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!oneConstant->isAssignable());
  assertTrue_1(oneConstant->valueType() == REAL_TYPE);
  assertTrue_1(oneConstant->getValue(temp));
  assertTrue_1(temp == 1);

  pugi::xml_node minusOneXml = doc.append_child();
  minusOneXml.set_name("RealValue");
  pugi::xml_node minusOneText = minusOneXml.append_child(node_pcdata);
  minusOneText.set_value("-1");

  Expression *minusOneConstant = createExpression(minusOneXml, nc, wasCreated);
  assertTrue_1(minusOneConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!minusOneConstant->isAssignable());
  assertTrue_1(minusOneConstant->valueType() == REAL_TYPE);
  assertTrue_1(minusOneConstant->getValue(temp));
  assertTrue_1(temp == -1);

  pugi::xml_node piXml = doc.append_child();
  piXml.set_name("RealValue");
  pugi::xml_node piText = piXml.append_child(node_pcdata);
  piText.set_value("3.14");

  Expression *piConstant = createExpression(piXml, nc, wasCreated);
  assertTrue_1(piConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!piConstant->isAssignable());
  assertTrue_1(piConstant->valueType() == REAL_TYPE);
  assertTrue_1(piConstant->getValue(temp));
  assertTrue_1(temp == 3.14);

  pugi::xml_node piWithJunkXml = doc.append_child();
  piWithJunkXml.set_name("RealValue");
  pugi::xml_node piWithJunkText = piWithJunkXml.append_child(node_pcdata);
  piWithJunkText.set_value("3.14T");

  try {
    Expression *piWithJunkConstant = createExpression(piWithJunkXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect junk after valid real");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  pugi::xml_node expNotationXml = doc.append_child();
  expNotationXml.set_name("RealValue");
  pugi::xml_node expNotationText = expNotationXml.append_child(node_pcdata);
  expNotationText.set_value("1e-100");

  Expression *expNotationConstant = createExpression(expNotationXml, nc, wasCreated);
  assertTrue_1(expNotationConstant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(!expNotationConstant->isAssignable());
  assertTrue_1(expNotationConstant->valueType() == REAL_TYPE);
  assertTrue_1(expNotationConstant->getValue(temp));
  assertTrue_1(temp == 1e-100);

  // Overflow not detected on VxWorks/PPC
#if !defined(__VXWORKS__)
  pugi::xml_node tooBigXml = doc.append_child();
  tooBigXml.set_name("RealValue");
  pugi::xml_node tooBigText = tooBigXml.append_child(node_pcdata);
  tooBigText.set_value("1e10000000");

  try {
    Expression *tooBigConstant = createExpression(tooBigXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect real overflow");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }
#endif // !defined(__VXWORKS__)

  pugi::xml_node emptyXml = doc.append_child();
  emptyXml.set_name("RealValue");
  emptyXml.append_child(node_pcdata);

  try {
    Expression *emptyConstant = createExpression(emptyXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect empty input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  pugi::xml_node bogusXml = doc.append_child();
  bogusXml.set_name("RealValue");
  pugi::xml_node bogusText = bogusXml.append_child(node_pcdata);
  bogusText.set_value("0");

  try {
    Expression *bogusConstant = createExpression(bogusXml, nc, wasCreated);
    assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
  }
  catch (ParserException const & /* exc */) {
    std::cout << "Caught expected exception" << std::endl;
  }

  return true;
}

static bool stringConstantXmlParserTest()
{
  xml_document doc;
  doc.set_name("stringConstantXmlParserTest");

  bool wasCreated;
  std::string temp;

  xml_node emptyXml = doc.append_child();
  emptyXml.set_name("StringValue");
  emptyXml.append_child(node_pcdata);

  Expression *s1Constant = createExpression(emptyXml, nc, wasCreated);
  assertTrue_1(s1Constant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(s1Constant->getValue(temp));
  assertTrue_1(temp.empty());

  xml_node s2Xml = doc.append_child();
  emptyXml.set_name("StringValue");
  xml_node s2Text = emptyXml.append_child(node_pcdata);
  s2Text.set_value("foo");

  Expression *s2Constant = createExpression(s2Xml, nc, wasCreated);
  assertTrue_1(s2Constant);
  assertTrue_1(wasCreated); // was created - may not be true in future
  assertTrue_1(s2Constant->getValue(temp));
  assertTrue_1(temp == "foo");

  return true;
}

// static bool booleanArrayConstantXmlParserTest()
// {
//   PlexilArrayValue emptyVal(BOOLEAN_TYPE, 0, std::vector<std::string>());

//   std::vector<std::string> validValVector(7);
//   validValVector[0] = "0";
//   validValVector[1] = "1";
//   validValVector[2] = "UNKNOWN";
//   validValVector[3] = "true";
//   validValVector[4] = "false";
//   validValVector[5] = "FALSE";
//   validValVector[6] = "TRUE";
//   PlexilArrayValue validVal(BOOLEAN_TYPE, validValVector.size(), validValVector);

//   std::vector<std::string> parseErrVector(1, "bOgUs");
//   PlexilArrayValue parseErrVal(BOOLEAN_TYPE, parseErrVector.size(), parseErrVector);

//   bool wasCreated;
//   BooleanArray const *aryTemp = NULL;

//   Expression *emptyConstant = createExpression(&emptyVal, nc, wasCreated);
//   assertTrue_1(emptyConstant);
//   assertTrue_1(wasCreated);
//   assertTrue_1(emptyConstant->valueType() == BOOLEAN_ARRAY_TYPE);
//   assertTrue_1(emptyConstant->getValuePointer(aryTemp));
//   assertTrue_1(aryTemp != NULL);
//   assertTrue_1(aryTemp->size() == 0);

//   bool temp;
//   Expression *validValConstant = createExpression(&validVal, nc, wasCreated);
//   assertTrue_1(validValConstant);
//   assertTrue_1(wasCreated);
//   assertTrue_1(validValConstant->valueType() == BOOLEAN_ARRAY_TYPE);
//   assertTrue_1(validValConstant->getValuePointer(aryTemp));
//   assertTrue_1(aryTemp != NULL);
//   assertTrue_1(aryTemp->size() == validValVector.size());
//   assertTrue_1(aryTemp->getElement(0, temp));
//   assertTrue_1(!temp);
//   assertTrue_1(aryTemp->getElement(1, temp));
//   assertTrue_1(temp);
//   assertTrue_1(!aryTemp->getElement(2, temp));
//   assertTrue_1(aryTemp->getElement(3, temp));
//   assertTrue_1(temp);
//   assertTrue_1(aryTemp->getElement(4, temp));
//   assertTrue_1(!temp);
//   assertTrue_1(aryTemp->getElement(5, temp));
//   assertTrue_1(!temp);
//   assertTrue_1(aryTemp->getElement(6, temp));
//   assertTrue_1(temp);

//   try {
//     Expression *parseErrConstant = createExpression(&parseErrVal, nc, wasCreated);
//     assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
//   }
//   catch (ParserException const & /* exc */) {
//     std::cout << "Caught expected exception" << std::endl;
//   }

//   return true;
// }

// static bool integerArrayConstantXmlParserTest()
// {
//   PlexilArrayValue emptyVal(INTEGER_TYPE, 0, std::vector<std::string>());

//   std::vector<std::string> validValVector(6);
//   validValVector[0] = "0";
//   validValVector[1] = "1";
//   validValVector[2] = "UNKNOWN";
//   validValVector[3] = "-123456789";
//   validValVector[4] = "987654321";
//   validValVector[5] = "0x69";
//   PlexilArrayValue validVal(INTEGER_TYPE, validValVector.size(), validValVector);

//   std::vector<std::string> bogusValueVector(1, "bOgUs");
//   PlexilArrayValue bogusValueVal(INTEGER_TYPE, bogusValueVector.size(), bogusValueVector);

//   std::vector<std::string> rangeErrVector(1, "-3000000000");
//   PlexilArrayValue rangeErrVal(INTEGER_TYPE, rangeErrVector.size(), rangeErrVector);

//   bool wasCreated;
//   IntegerArray const *aryTemp = NULL;

//   Expression *emptyConstant = createExpression(&emptyVal, nc, wasCreated);
//   assertTrue_1(emptyConstant);
//   assertTrue_1(wasCreated);
//   assertTrue_1(emptyConstant->valueType() == INTEGER_ARRAY_TYPE);
//   assertTrue_1(emptyConstant->getValuePointer(aryTemp));
//   assertTrue_1(aryTemp != NULL);
//   assertTrue_1(aryTemp->size() == 0);

//   int32_t temp;
//   Expression *validValConstant = createExpression(&validVal, nc, wasCreated);
//   assertTrue_1(validValConstant);
//   assertTrue_1(wasCreated);
//   assertTrue_1(validValConstant->valueType() == INTEGER_ARRAY_TYPE);
//   assertTrue_1(validValConstant->getValuePointer(aryTemp));
//   assertTrue_1(aryTemp != NULL);
//   assertTrue_1(aryTemp->size() == validValVector.size());
//   assertTrue_1(aryTemp->getElement(0, temp));
//   assertTrue_1(temp == 0);
//   assertTrue_1(aryTemp->getElement(1, temp));
//   assertTrue_1(temp == 1);
//   assertTrue_1(!aryTemp->getElement(2, temp));
//   assertTrue_1(aryTemp->getElement(3, temp));
//   assertTrue_1(temp == -123456789);
//   assertTrue_1(aryTemp->getElement(4, temp));
//   assertTrue_1(temp == 987654321);
//   assertTrue_1(aryTemp->getElement(5, temp));
//   assertTrue_1(temp == 0x69);

//   try {
//     Expression *bogusValueConstant = createExpression(&bogusValueVal, nc, wasCreated);
//     assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
//   }
//   catch (ParserException const & /* exc */) {
//     std::cout << "Caught expected exception" << std::endl;
//   }

//   try {
//     Expression *rangeErrConstant = createExpression(&rangeErrVal, nc, wasCreated);
//     assertTrue_2(ALWAYS_FAIL, "Failed to detect out-of-range integer");
//   }
//   catch (ParserException const & /* exc */) {
//     std::cout << "Caught expected exception" << std::endl;
//   }

//   return true;
// }

// static bool realArrayConstantXmlParserTest()
// {
//   PlexilArrayValue emptyVal(REAL_TYPE, 0, std::vector<std::string>());

//   std::vector<std::string> validValVector(6);
//   validValVector[0] = "0";
//   validValVector[1] = "1";
//   validValVector[2] = "UNKNOWN";
//   validValVector[3] = "3.14";
//   validValVector[4] = "1e-100";
//   validValVector[5] = "6.0221413e+23";
//   PlexilArrayValue validVal(REAL_TYPE, validValVector.size(), validValVector);

//   std::vector<std::string> bogusValueVector(1, "bOgUs");
//   PlexilArrayValue bogusValueVal(REAL_TYPE, bogusValueVector.size(), bogusValueVector);

//   std::vector<std::string> rangeErrVector(1, "-3e1000000000");
//   PlexilArrayValue rangeErrVal(REAL_TYPE, rangeErrVector.size(), rangeErrVector);

//   bool wasCreated;
//   RealArray const *aryTemp = NULL;

//   Expression *emptyConstant = createExpression(&emptyVal, nc, wasCreated);
//   assertTrue_1(emptyConstant);
//   assertTrue_1(wasCreated);
//   assertTrue_1(emptyConstant->valueType() == REAL_ARRAY_TYPE);
//   assertTrue_1(emptyConstant->getValuePointer(aryTemp));
//   assertTrue_1(aryTemp != NULL);
//   assertTrue_1(aryTemp->size() == 0);

//   double temp;
//   Expression *validValConstant = createExpression(&validVal, nc, wasCreated);
//   assertTrue_1(validValConstant);
//   assertTrue_1(wasCreated);
//   assertTrue_1(validValConstant->valueType() == REAL_ARRAY_TYPE);
//   assertTrue_1(validValConstant->getValuePointer(aryTemp));
//   assertTrue_1(aryTemp != NULL);
//   assertTrue_1(aryTemp->size() == validValVector.size());
//   assertTrue_1(aryTemp->getElement(0, temp));
//   assertTrue_1(temp == 0);
//   assertTrue_1(aryTemp->getElement(1, temp));
//   assertTrue_1(temp == 1);
//   assertTrue_1(!aryTemp->getElement(2, temp));
//   assertTrue_1(aryTemp->getElement(3, temp));
//   assertTrue_1(temp == 3.14);
//   assertTrue_1(aryTemp->getElement(4, temp));
//   assertTrue_1(temp == 1e-100);
//   assertTrue_1(aryTemp->getElement(5, temp));
//   assertTrue_1(temp == 6.0221413e+23);

//   try {
//     Expression *bogusValueConstant = createExpression(&bogusValueVal, nc, wasCreated);
//     assertTrue_2(ALWAYS_FAIL, "Failed to detect bogus input");
//   }
//   catch (ParserException const & /* exc */) {
//     std::cout << "Caught expected exception" << std::endl;
//   }

// #if !defined(__VXWORKS__)
//   try {
//     Expression *rangeErrConstant = createExpression(&rangeErrVal, nc, wasCreated);
//     assertTrue_2(ALWAYS_FAIL, "Failed to detect out-of-range real");
//   }
//   catch (ParserException const & /* exc */) {
//     std::cout << "Caught expected exception" << std::endl;
//   }
// #endif // !defined(__VXWORKS__)

//   return true;
// }

// static bool stringArrayConstantXmlParserTest()
// {
//   PlexilArrayValue emptyVal(STRING_TYPE, 0, std::vector<std::string>());

//   std::vector<std::string> validValVector(6);
//   validValVector[0] = "0";
//   validValVector[1] = "1";
//   validValVector[2] = "UNKNOWN";
//   validValVector[3] = "3.14";
//   validValVector[4] = "1e-100";
//   validValVector[5] = "6.0221413e+23";
//   PlexilArrayValue validVal(STRING_TYPE, validValVector.size(), validValVector);

//   bool wasCreated;
//   StringArray const *aryTemp = NULL;

//   Expression *emptyConstant = createExpression(&emptyVal, nc, wasCreated);
//   assertTrue_1(emptyConstant);
//   assertTrue_1(wasCreated);
//   assertTrue_1(emptyConstant->valueType() == STRING_ARRAY_TYPE);
//   assertTrue_1(emptyConstant->getValuePointer(aryTemp));
//   assertTrue_1(aryTemp != NULL);
//   assertTrue_1(aryTemp->size() == 0);

//   std::string const *temp;
//   Expression *validValConstant = createExpression(&validVal, nc, wasCreated);
//   assertTrue_1(validValConstant);
//   assertTrue_1(wasCreated);
//   assertTrue_1(validValConstant->valueType() == STRING_ARRAY_TYPE);
//   assertTrue_1(validValConstant->getValuePointer(aryTemp));
//   assertTrue_1(aryTemp != NULL);
//   assertTrue_1(aryTemp->size() == validValVector.size());
//   assertTrue_1(aryTemp->getElementPointer(0, temp));
//   assertTrue_1(*temp == "0");
//   assertTrue_1(aryTemp->getElementPointer(1, temp));
//   assertTrue_1(*temp == "1");
//   assertTrue_1(aryTemp->getElementPointer(2, temp));
//   assertTrue_1(*temp == "UNKNOWN");
//   assertTrue_1(aryTemp->getElementPointer(3, temp));
//   assertTrue_1(*temp == "3.14");
//   assertTrue_1(aryTemp->getElementPointer(4, temp));
//   assertTrue_1(*temp == "1e-100");
//   assertTrue_1(aryTemp->getElementPointer(5, temp));
//   assertTrue_1(*temp == "6.0221413e+23");

//   return true;
// }

bool constantXmlParserTest()
{
  // Initialize factories
  registerBasicExpressionFactories();
  // Initialize infrastructure
  TrivialNodeConnector conn;
  nc = &conn;

  runTest(booleanConstantXmlParserTest);
  runTest(integerConstantXmlParserTest);
  runTest(realConstantXmlParserTest);
  runTest(stringConstantXmlParserTest);

  // runTest(booleanArrayConstantXmlParserTest);
  // runTest(integerArrayConstantXmlParserTest);
  // runTest(realArrayConstantXmlParserTest);
  // runTest(stringArrayConstantXmlParserTest);

  nc = NULL;
  return true;
}
