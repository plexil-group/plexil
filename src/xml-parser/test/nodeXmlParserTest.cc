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

#include "Assignment.hh"
#include "AssignmentNode.hh"
#include "ExpressionFactory.hh"
#include "Node.hh"
#include "parseNode.hh"
#include "TestSupport.hh"
#include "test/FactoryTestNodeConnector.hh"
#include "test/TransitionExternalInterface.hh"

#include "pugixml.hpp"

using namespace PLEXIL;

using pugi::xml_attribute;
using pugi::xml_document;
using pugi::xml_node;
using pugi::node_pcdata;

bool emptyNodeXmlParserTest()
{
  xml_document doc;
  doc.set_name("emptyNodeXmlParserTest");

  xml_node minimal = doc.append_child("Node");
  minimal.append_attribute("NodeType").set_value("Empty");
  minimal.append_child("NodeId").append_child(node_pcdata).set_value("minimal");

  {
    Node *minimalNode = parseNode(minimal, NULL);
    assertTrue_1(minimalNode);
    assertTrue_1(minimalNode->getType() == NodeType_Empty);
    assertTrue_1(minimalNode->getChildren().empty());
    finalizeNode(minimalNode, minimal);
    assertTrue_1(minimalNode->getLocalVariables().empty());
    delete minimalNode;
  }

  {
    xml_node comment = doc.append_copy(minimal);
    comment.child("NodeId").first_child().set_value("comment");
    comment.append_child("Comment").append_child(node_pcdata).set_value("This is a comment");

    Node *commentNode = parseNode(comment, NULL);
    assertTrue_1(commentNode);
    assertTrue_1(commentNode->getType() == NodeType_Empty);
    assertTrue_1(commentNode->getChildren().empty());
    finalizeNode(commentNode, comment);
    assertTrue_1(commentNode->getLocalVariables().empty());
    delete commentNode;
  }

  //
  // Test conditions
  //

  {
    xml_node preCond = doc.append_copy(minimal);
    preCond.child("NodeId").first_child().set_value("preCond");
    preCond.append_child("PreCondition").append_child("BooleanValue").append_child(node_pcdata).set_value("true");

    Node *preCondNode = parseNode(preCond, NULL);
    assertTrue_1(preCondNode);
    assertTrue_1(preCondNode->getType() == NodeType_Empty);
    assertTrue_1(preCondNode->getChildren().empty());
    finalizeNode(preCondNode, preCond);
    assertTrue_1(preCondNode->getLocalVariables().empty());
    Expression *sc = preCondNode->getPreCondition();
    assertTrue_1(sc);
    assertTrue_1(sc->isConstant());
    assertTrue_1(sc->valueType() == BOOLEAN_TYPE);
    bool scValue;
    assertTrue_1(sc->getValue(scValue));
    assertTrue_1(scValue);
    delete preCondNode;
  }

  {
    xml_node endCond = doc.append_copy(minimal);
    endCond.child("NodeId").first_child().set_value("endCond");
    endCond.append_child("EndCondition").append_child("BooleanValue").append_child(node_pcdata).set_value("true");

    Node *endCondNode = parseNode(endCond, NULL);
    assertTrue_1(endCondNode);
    assertTrue_1(endCondNode->getType() == NodeType_Empty);
    assertTrue_1(endCondNode->getChildren().empty());
    finalizeNode(endCondNode, endCond);
    assertTrue_1(endCondNode->getLocalVariables().empty());
    Expression *sc = endCondNode->getEndCondition();
    assertTrue_1(sc);
    assertTrue_1(sc->isConstant());
    assertTrue_1(sc->valueType() == BOOLEAN_TYPE);
    bool scValue;
    assertTrue_1(sc->getValue(scValue));
    assertTrue_1(scValue);
    delete endCondNode;
  }

  {
    xml_node exitCond = doc.append_copy(minimal);
    exitCond.child("NodeId").first_child().set_value("exitCond");
    exitCond.append_child("ExitCondition").append_child("BooleanValue").append_child(node_pcdata).set_value("true");

    Node *exitCondNode = parseNode(exitCond, NULL);
    assertTrue_1(exitCondNode);
    assertTrue_1(exitCondNode->getType() == NodeType_Empty);
    assertTrue_1(exitCondNode->getChildren().empty());
    finalizeNode(exitCondNode, exitCond);
    assertTrue_1(exitCondNode->getLocalVariables().empty());
    Expression *sc = exitCondNode->getExitCondition();
    assertTrue_1(sc);
    assertTrue_1(sc->isConstant());
    assertTrue_1(sc->valueType() == BOOLEAN_TYPE);
    bool scValue;
    assertTrue_1(sc->getValue(scValue));
    assertTrue_1(scValue);
    delete exitCondNode;
  }

  {
    xml_node postCond = doc.append_copy(minimal);
    postCond.child("NodeId").first_child().set_value("postCond");
    postCond.append_child("PostCondition").append_child("BooleanValue").append_child(node_pcdata).set_value("true");

    Node *postCondNode = parseNode(postCond, NULL);
    assertTrue_1(postCondNode);
    assertTrue_1(postCondNode->getType() == NodeType_Empty);
    assertTrue_1(postCondNode->getChildren().empty());
    finalizeNode(postCondNode, postCond);
    assertTrue_1(postCondNode->getLocalVariables().empty());
    Expression *sc = postCondNode->getPostCondition();
    assertTrue_1(sc);
    assertTrue_1(sc->isConstant());
    assertTrue_1(sc->valueType() == BOOLEAN_TYPE);
    bool scValue;
    assertTrue_1(sc->getValue(scValue));
    assertTrue_1(scValue);
    delete postCondNode;
  }

  {
    xml_node skipCond = doc.append_copy(minimal);
    skipCond.child("NodeId").first_child().set_value("skipCond");
    skipCond.append_child("SkipCondition").append_child("BooleanValue").append_child(node_pcdata).set_value("true");

    Node *skipCondNode = parseNode(skipCond, NULL);
    assertTrue_1(skipCondNode);
    assertTrue_1(skipCondNode->getType() == NodeType_Empty);
    assertTrue_1(skipCondNode->getChildren().empty());
    finalizeNode(skipCondNode, skipCond);
    assertTrue_1(skipCondNode->getLocalVariables().empty());
    Expression *sc = skipCondNode->getSkipCondition();
    assertTrue_1(sc);
    assertTrue_1(sc->isConstant());
    assertTrue_1(sc->valueType() == BOOLEAN_TYPE);
    bool scValue;
    assertTrue_1(sc->getValue(scValue));
    assertTrue_1(scValue);
    delete skipCondNode;
  }

  {
    xml_node startCond = doc.append_copy(minimal);
    startCond.child("NodeId").first_child().set_value("startCond");
    startCond.append_child("StartCondition").append_child("BooleanValue").append_child(node_pcdata).set_value("true");

    Node *startCondNode = parseNode(startCond, NULL);
    assertTrue_1(startCondNode);
    assertTrue_1(startCondNode->getType() == NodeType_Empty);
    assertTrue_1(startCondNode->getChildren().empty());
    finalizeNode(startCondNode, startCond);
    assertTrue_1(startCondNode->getLocalVariables().empty());
    Expression *sc = startCondNode->getStartCondition();
    assertTrue_1(sc);
    assertTrue_1(sc->isConstant());
    assertTrue_1(sc->valueType() == BOOLEAN_TYPE);
    bool scValue;
    assertTrue_1(sc->getValue(scValue));
    assertTrue_1(scValue);
    delete startCondNode;
  }

  {
    xml_node repeatCond = doc.append_copy(minimal);
    repeatCond.child("NodeId").first_child().set_value("repeatCond");
    repeatCond.append_child("RepeatCondition").append_child("BooleanValue").append_child(node_pcdata).set_value("true");

    Node *repeatCondNode = parseNode(repeatCond, NULL);
    assertTrue_1(repeatCondNode);
    assertTrue_1(repeatCondNode->getType() == NodeType_Empty);
    assertTrue_1(repeatCondNode->getChildren().empty());
    finalizeNode(repeatCondNode, repeatCond);
    assertTrue_1(repeatCondNode->getLocalVariables().empty());
    Expression *sc = repeatCondNode->getRepeatCondition();
    assertTrue_1(sc);
    assertTrue_1(sc->isConstant());
    assertTrue_1(sc->valueType() == BOOLEAN_TYPE);
    bool scValue;
    assertTrue_1(sc->getValue(scValue));
    assertTrue_1(scValue);
    delete repeatCondNode;
  }

  {
    xml_node invariantCond = doc.append_copy(minimal);
    invariantCond.child("NodeId").first_child().set_value("invariantCond");
    invariantCond.append_child("InvariantCondition").append_child("BooleanValue").append_child(node_pcdata).set_value("true");

    Node *invariantCondNode = parseNode(invariantCond, NULL);
    assertTrue_1(invariantCondNode);
    assertTrue_1(invariantCondNode->getType() == NodeType_Empty);
    assertTrue_1(invariantCondNode->getChildren().empty());
    finalizeNode(invariantCondNode, invariantCond);
    assertTrue_1(invariantCondNode->getLocalVariables().empty());
    Expression *sc = invariantCondNode->getInvariantCondition();
    assertTrue_1(sc);
    assertTrue_1(sc->isConstant());
    assertTrue_1(sc->valueType() == BOOLEAN_TYPE);
    bool scValue;
    assertTrue_1(sc->getValue(scValue));
    assertTrue_1(scValue);
    delete invariantCondNode;
  }

  {
    xml_node simpleVarDecl = doc.append_copy(minimal);
    simpleVarDecl.child("NodeId").first_child().set_value("simpleVarDecl");
    xml_node simpleDecl = simpleVarDecl.append_child("VariableDeclarations").append_child("DeclareVariable");
    simpleDecl.append_child("Name").append_child(node_pcdata).set_value("b");
    simpleDecl.append_child("Type").append_child(node_pcdata).set_value("Boolean");

    Node *simpleVarDeclNode = parseNode(simpleVarDecl, NULL);
    assertTrue_1(simpleVarDeclNode);
    assertTrue_1(simpleVarDeclNode->getType() == NodeType_Empty);
    assertTrue_1(simpleVarDeclNode->getChildren().empty());
    finalizeNode(simpleVarDeclNode, simpleVarDecl);
    assertTrue_1(!simpleVarDeclNode->getLocalVariables().empty());
    Expression *simpleVar = simpleVarDeclNode->findLocalVariable("b");
    assertTrue_1(simpleVar);
    assertTrue_1(simpleVar->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!simpleVar->isActive());
    assertTrue_1(simpleVar->isAssignable());
    Assignable *simpleVarA = simpleVar->asAssignable();
    assertTrue_1(simpleVarA);
    assertTrue_1(simpleVarA->getBaseVariable() == simpleVarA);
    assertTrue_1(simpleVarA->getNode() == simpleVarDeclNode);
    delete simpleVarDeclNode;
  }

  {
    xml_node initedVarDecl = doc.append_copy(minimal);
    initedVarDecl.child("NodeId").first_child().set_value("initedVarDecl");
    xml_node initedDecl = initedVarDecl.append_child("VariableDeclarations").append_child("DeclareVariable");
    initedDecl.append_child("Name").append_child(node_pcdata).set_value("b");
    initedDecl.append_child("Type").append_child(node_pcdata).set_value("Boolean");
    initedDecl.append_child("InitialValue").append_child("BooleanValue").append_child(node_pcdata).set_value("false");

    Node *initedVarDeclNode = parseNode(initedVarDecl, NULL);
    assertTrue_1(initedVarDeclNode);
    assertTrue_1(initedVarDeclNode->getType() == NodeType_Empty);
    assertTrue_1(initedVarDeclNode->getChildren().empty());
    finalizeNode(initedVarDeclNode, initedVarDecl);
    assertTrue_1(!initedVarDeclNode->getLocalVariables().empty());
    Expression *initedVar = initedVarDeclNode->findLocalVariable("b");
    assertTrue_1(initedVar);
    assertTrue_1(initedVar->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!initedVar->isActive());
    assertTrue_1(initedVar->isAssignable());
    initedVar->activate();
    bool initedVal = true;
    assertTrue_1(initedVar->getValue(initedVal));
    assertTrue_1(!initedVal);
    Assignable *initedVarA = initedVar->asAssignable();
    assertTrue_1(initedVarA);
    assertTrue_1(initedVarA->getBaseVariable() == initedVarA);
    assertTrue_1(initedVarA->getNode() == initedVarDeclNode);
    delete initedVarDeclNode;
  }

  {
    xml_node simpleArrayVarDecl = doc.append_copy(minimal);
    simpleArrayVarDecl.child("NodeId").first_child().set_value("simpleArrayVarDecl");
    xml_node simpleArrayDecl = simpleArrayVarDecl.append_child("VariableDeclarations").append_child("DeclareArray");
    simpleArrayDecl.append_child("Name").append_child(node_pcdata).set_value("ba");
    simpleArrayDecl.append_child("Type").append_child(node_pcdata).set_value("Boolean");
    simpleArrayDecl.append_child("MaxSize").append_child(node_pcdata).set_value("1");

    Node *simpleArrayVarDeclNode = parseNode(simpleArrayVarDecl, NULL);
    assertTrue_1(simpleArrayVarDeclNode);
    assertTrue_1(simpleArrayVarDeclNode->getType() == NodeType_Empty);
    assertTrue_1(simpleArrayVarDeclNode->getChildren().empty());
    finalizeNode(simpleArrayVarDeclNode, simpleArrayVarDecl);
    assertTrue_1(!simpleArrayVarDeclNode->getLocalVariables().empty());
    Expression *simpleArrayVar = simpleArrayVarDeclNode->findLocalVariable("ba");
    assertTrue_1(simpleArrayVar);
    assertTrue_1(simpleArrayVar->valueType() == BOOLEAN_ARRAY_TYPE);
    assertTrue_1(!simpleArrayVar->isActive());
    assertTrue_1(simpleArrayVar->isAssignable());
    Assignable *simpleArrayVarA = simpleArrayVar->asAssignable();
    assertTrue_1(simpleArrayVarA);
    assertTrue_1(simpleArrayVarA->getBaseVariable() == simpleArrayVarA);
    assertTrue_1(simpleArrayVarA->getNode() == simpleArrayVarDeclNode);
    delete simpleArrayVarDeclNode;
  }

  {
    xml_node initedArrayVarDecl = doc.append_copy(minimal);
    initedArrayVarDecl.child("NodeId").first_child().set_value("initedArrayVarDecl");
    xml_node initedArrayDecl = initedArrayVarDecl.append_child("VariableDeclarations").append_child("DeclareArray");
    initedArrayDecl.append_child("Name").append_child(node_pcdata).set_value("iba");
    initedArrayDecl.append_child("Type").append_child(node_pcdata).set_value("Boolean");
    initedArrayDecl.append_child("MaxSize").append_child(node_pcdata).set_value("1");
    xml_node initXml = initedArrayDecl.append_child("InitialValue").append_child("ArrayValue");
    initXml.append_attribute("Type").set_value("Boolean");
    initXml.append_child("BooleanValue").append_child(node_pcdata).set_value("false");

    Node *initedArrayVarDeclNode = parseNode(initedArrayVarDecl, NULL);
    assertTrue_1(initedArrayVarDeclNode);
    assertTrue_1(initedArrayVarDeclNode->getType() == NodeType_Empty);
    assertTrue_1(initedArrayVarDeclNode->getChildren().empty());
    finalizeNode(initedArrayVarDeclNode, initedArrayVarDecl);
    assertTrue_1(!initedArrayVarDeclNode->getLocalVariables().empty());
    Expression *initedArrayVar = initedArrayVarDeclNode->findLocalVariable("iba");
    assertTrue_1(initedArrayVar);
    assertTrue_1(initedArrayVar->valueType() == BOOLEAN_ARRAY_TYPE);
    assertTrue_1(!initedArrayVar->isActive());
    assertTrue_1(initedArrayVar->isAssignable());
    initedArrayVar->activate();
    BooleanArray const *initedArrayVal = NULL;
    assertTrue_1(initedArrayVar->getValuePointer(initedArrayVal));
    assertTrue_1(initedArrayVal);
    assertTrue_1(initedArrayVal->size() == 1);
    assertTrue_1(initedArrayVal->allElementsKnown());
    bool eltVal = true;
    assertTrue_1(initedArrayVal->getElement(0, eltVal));
    assertTrue_1(!eltVal);

    Assignable *initedArrayVarA = initedArrayVar->asAssignable();
    assertTrue_1(initedArrayVarA);
    assertTrue_1(initedArrayVarA->getBaseVariable() == initedArrayVarA);
    assertTrue_1(initedArrayVarA->getNode() == initedArrayVarDeclNode);
    delete initedArrayVarDeclNode;
  }

  return true;
}

static bool listNodeXmlParserTest()
{
  xml_document doc;
  doc.set_name("listNodeXmlParserTest");

  xml_node basicListXml = doc.append_child("Node");
  basicListXml.append_attribute("NodeType").set_value("NodeList");
  basicListXml.append_child("NodeId").append_child(node_pcdata).set_value("basicList");
  xml_node list = basicListXml.append_child("NodeBody").append_child("NodeList");

  {
    Node *emptyList = parseNode(basicListXml, NULL);
    assertTrue_1(emptyList);
    assertTrue_1(emptyList->getType() == NodeType_NodeList);
    assertTrue_1(emptyList->getNodeId() == "basicList");
    finalizeNode(emptyList, basicListXml);
    assertTrue_1(emptyList->getChildren().empty());
    assertTrue_1(emptyList->getLocalVariables().empty());
    delete emptyList;
  }

  {
    xml_node oneListXml = doc.append_copy(basicListXml);
    oneListXml.child("NodeId").first_child().set_value("oneList");
    xml_node oneListKid = oneListXml.child("NodeBody").child("NodeList").append_child("Node");
    oneListKid.append_attribute("NodeType").set_value("Empty");
    oneListKid.append_child("NodeId").append_child(node_pcdata).set_value("oneListKid");
    Node *oneList = parseNode(oneListXml, NULL);
    assertTrue_1(oneList);
    assertTrue_1(oneList->getType() == NodeType_NodeList);
    assertTrue_1(oneList->getNodeId() == "oneList");
    assertTrue_1(!oneList->getChildren().empty());
    assertTrue_1(oneList->getChildren().size() == 1);
    Node *kid = oneList->getChildren().front();
    assertTrue_1(kid->getType() == NodeType_Empty); 
    assertTrue_1(kid->getNodeId() == "oneListKid");
    assertTrue_1(kid->getChildren().empty());
    finalizeNode(oneList, oneListXml);
    assertTrue_1(oneList->getLocalVariables().empty());
    assertTrue_1(kid->getLocalVariables().empty());
    delete oneList;
  }

  {
    xml_node anotherListXml = doc.append_copy(basicListXml);
    anotherListXml.child("NodeId").first_child().set_value("anotherList");
    xml_node theList = anotherListXml.child("NodeBody").child("NodeList");
    xml_node anotherListKid = theList.append_child("Node");
    anotherListKid.append_attribute("NodeType").set_value("Empty");
    anotherListKid.append_child("NodeId").append_child(node_pcdata).set_value("anotherListKid0");
    xml_node yaListKid = theList.append_child("Node");
    yaListKid.append_attribute("NodeType").set_value("Empty");
    yaListKid.append_child("NodeId").append_child(node_pcdata).set_value("anotherListKid1");
    Node *anotherList = parseNode(anotherListXml, NULL);
    assertTrue_1(anotherList);
    assertTrue_1(anotherList->getType() == NodeType_NodeList);
    assertTrue_1(anotherList->getNodeId() == "anotherList");
    assertTrue_1(!anotherList->getChildren().empty());
    assertTrue_1(anotherList->getChildren().size() == 2);
    Node *kid0 = anotherList->getChildren().at(0);
    assertTrue_1(kid0->getType() == NodeType_Empty);
    assertTrue_1(kid0->getNodeId() == "anotherListKid0");
    assertTrue_1(kid0->getChildren().empty());
    Node *kid1 = anotherList->getChildren().at(1);
    assertTrue_1(kid1->getType() == NodeType_Empty);
    assertTrue_1(kid1->getNodeId() == "anotherListKid1");
    assertTrue_1(kid1->getChildren().empty());
    finalizeNode(anotherList, anotherListXml);
    assertTrue_1(anotherList->getLocalVariables().empty());
    assertTrue_1(kid0->getLocalVariables().empty());
    assertTrue_1(kid1->getLocalVariables().empty());
    delete anotherList;
  }

  {
    xml_node varAccessListXml = doc.append_copy(basicListXml);
    varAccessListXml.child("NodeId").first_child().set_value("varAccessList");
    xml_node rootDecls = varAccessListXml.append_child("VariableDeclarations");
    xml_node decl0 = rootDecls.append_child("DeclareVariable");
    decl0.append_child("Name").append_child(node_pcdata).set_value("foo");
    decl0.append_child("Type").append_child(node_pcdata).set_value("Integer");
    decl0.append_child("InitialValue").append_child("IntegerValue").append_child(node_pcdata).set_value("42");
    xml_node varAccessListKid = varAccessListXml.child("NodeBody").child("NodeList").append_child("Node");
    varAccessListKid.append_attribute("NodeType").set_value("Empty");
    varAccessListKid.append_child("NodeId").append_child(node_pcdata).set_value("varAccessListKid");
    xml_node kidDecls = varAccessListKid.append_child("VariableDeclarations");
    xml_node decl1 = kidDecls.append_child("DeclareVariable");
    decl1.append_child("Name").append_child(node_pcdata).set_value("bar");
    decl1.append_child("Type").append_child(node_pcdata).set_value("Integer");
    decl1.append_child("InitialValue").append_child("IntegerVariable").append_child(node_pcdata).set_value("foo");

    Node *varAccessList = parseNode(varAccessListXml, NULL);
    assertTrue_1(varAccessList);
    assertTrue_1(varAccessList->getType() == NodeType_NodeList);
    assertTrue_1(varAccessList->getNodeId() == "varAccessList");
    assertTrue_1(!varAccessList->getChildren().empty());
    assertTrue_1(varAccessList->getChildren().size() == 1);
    Node *kid = varAccessList->getChildren().front();
    assertTrue_1(kid->getType() == NodeType_Empty); 
    assertTrue_1(kid->getNodeId() == "varAccessListKid");
    assertTrue_1(kid->getChildren().empty());
    finalizeNode(varAccessList, varAccessListXml);
    assertTrue_1(!varAccessList->getLocalVariables().empty());
    assertTrue_1(varAccessList->getLocalVariables().size() == 1);
    Assignable *rootVar = varAccessList->getLocalVariables().front()->asAssignable();
    assertTrue_1(rootVar);
    assertTrue_1(rootVar->valueType() == INTEGER_TYPE);
    assertTrue_1(rootVar->getNode() == varAccessList);
    assertTrue_1(rootVar == varAccessList->findVariable("foo"));
    assertTrue_1(!varAccessList->findVariable("bar", true));

    assertTrue_1(!kid->getLocalVariables().empty());
    assertTrue_1(kid->getLocalVariables().size() == 1);
    Assignable *kidVar = kid->getLocalVariables().front()->asAssignable();
    assertTrue_1(kidVar);
    assertTrue_1(kidVar->valueType() == INTEGER_TYPE);
    assertTrue_1(kidVar->getNode() == kid);
    assertTrue_1(rootVar == kid->findVariable("foo"));
    assertTrue_1(kidVar == kid->findVariable("bar"));

    delete varAccessList;
  }

  {
    xml_node nodeRefTestXml = doc.append_copy(basicListXml);
    nodeRefTestXml.child("NodeId").first_child().set_value("nodeRefTest");
    xml_node eq = nodeRefTestXml.append_child("ExitCondition").append_child("EQInternal");
    eq.append_child("NodeOutcomeVariable").append_child("NodeId").append_child(node_pcdata).set_value("nodeRefTestKid");
    eq.append_child("NodeOutcomeValue").append_child(node_pcdata).set_value("FAILURE");
    xml_node nodeRefTestKid = nodeRefTestXml.child("NodeBody").child("NodeList").append_child("Node");
    nodeRefTestKid.append_attribute("NodeType").set_value("Empty");
    nodeRefTestKid.append_child("NodeId").append_child(node_pcdata).set_value("nodeRefTestKid");
    xml_node neq = nodeRefTestKid.append_child("InvariantCondition").append_child("NEInternal");
    neq.append_child("NodeStateVariable").append_child("NodeId").append_child(node_pcdata).set_value("nodeRefTest");
    neq.append_child("NodeStateValue").append_child(node_pcdata).set_value("EXECUTING");
    Node *nodeRefTest = parseNode(nodeRefTestXml, NULL);
    assertTrue_1(nodeRefTest);
    assertTrue_1(nodeRefTest->getType() == NodeType_NodeList);
    assertTrue_1(nodeRefTest->getNodeId() == "nodeRefTest");
    assertTrue_1(!nodeRefTest->getChildren().empty());
    assertTrue_1(nodeRefTest->getChildren().size() == 1);
    Node *kid = nodeRefTest->getChildren().front();
    assertTrue_1(kid->getType() == NodeType_Empty); 
    assertTrue_1(kid->getNodeId() == "nodeRefTestKid");
    assertTrue_1(kid->getChildren().empty());
    finalizeNode(nodeRefTest, nodeRefTestXml);
    assertTrue_1(nodeRefTest->getLocalVariables().empty());
    assertTrue_1(kid->getLocalVariables().empty());
    assertTrue_1(nodeRefTest->getExitCondition());
    assertTrue_1(nodeRefTest->getExitCondition()->valueType() == BOOLEAN_TYPE);
    assertTrue_1(kid->getInvariantCondition());
    assertTrue_1(kid->getInvariantCondition()->valueType() == BOOLEAN_TYPE);
    delete nodeRefTest;
  }

  return true;
}

static bool assignmentNodeXmlParserTest()
{
  xml_document doc;
  doc.set_name("assignmentNodeXmlParserTest");

  // Scalar assignment
  {
    xml_node listNodeXml = doc.append_child("Node");
    listNodeXml.append_attribute("NodeType").set_value("NodeList");
    listNodeXml.append_child("NodeId").append_child(node_pcdata).set_value("listNode");
    xml_node listNodeDecls = listNodeXml.append_child("VariableDeclarations");
    xml_node decl0 = listNodeDecls.append_child("DeclareVariable");
    decl0.append_child("Name").append_child(node_pcdata).set_value("foo");
    decl0.append_child("Type").append_child(node_pcdata).set_value("Integer");
    xml_node listNodeList = listNodeXml.append_child("NodeBody").append_child("NodeList");

    xml_node basicAssnXml = listNodeList.append_child("Node");
    basicAssnXml.append_attribute("NodeType").set_value("Assignment");
    basicAssnXml.append_child("NodeId").append_child(node_pcdata).set_value("basicAssn");
    xml_node assnXml = basicAssnXml.append_child("NodeBody").append_child("Assignment");
    assnXml.append_child("IntegerVariable").append_child(node_pcdata).set_value("foo");
    assnXml.append_child("NumericRHS").append_child("IntegerValue").append_child(node_pcdata).set_value("2");

    Node *listNode = parseNode(listNodeXml, NULL);
    assertTrue_1(listNode);
    assertTrue_1(listNode->getType() == NodeType_NodeList);
    assertTrue_1(!listNode->getChildren().empty());
    assertTrue_1(listNode->getChildren().size() == 1);
    assertTrue_1(!listNode->getLocalVariables().empty());
    assertTrue_1(listNode->getLocalVariables().size() == 1);

    Node *basicAssn = listNode->getChildren().front();
    assertTrue_1(basicAssn);
    assertTrue_1(basicAssn->getType() == NodeType_Assignment);
    assertTrue_1(basicAssn->getChildren().empty());
    assertTrue_1(basicAssn->getLocalVariables().empty());

    finalizeNode(listNode, listNodeXml);
    AssignmentNode *anode = dynamic_cast<AssignmentNode *>(basicAssn);
    assertTrue_1(anode);
    Assignment *assn = anode->getAssignment();
    assertTrue_1(assn);

    Assignable *fooVar = listNode->findLocalVariable("foo")->asAssignable();
    assertTrue_1(fooVar);
    assertTrue_1(fooVar->valueType() == INTEGER_TYPE);
    assertTrue_1(assn->getDest() == fooVar);

    delete listNode;
  }

  // Array assignment
  {
    xml_node listNode2Xml = doc.append_child("Node");
    listNode2Xml.append_attribute("NodeType").set_value("NodeList");
    listNode2Xml.append_child("NodeId").append_child(node_pcdata).set_value("listNode2");
    xml_node listNode2Decls = listNode2Xml.append_child("VariableDeclarations");
    xml_node decl1 = listNode2Decls.append_child("DeclareArray");
    decl1.append_child("Name").append_child(node_pcdata).set_value("bar");
    decl1.append_child("Type").append_child(node_pcdata).set_value("Integer");
    decl1.append_child("MaxSize").append_child(node_pcdata).set_value("2");
    xml_node listNode2List = listNode2Xml.append_child("NodeBody").append_child("NodeList");

    xml_node arrayAssnXml = listNode2List.append_child("Node");
    arrayAssnXml.append_attribute("NodeType").set_value("Assignment");
    arrayAssnXml.append_child("NodeId").append_child(node_pcdata).set_value("arrayAssn");
    xml_node assnXml = arrayAssnXml.append_child("NodeBody").append_child("Assignment");
    assnXml.append_child("ArrayVariable").append_child(node_pcdata).set_value("bar");
    xml_node arrayXml = assnXml.append_child("ArrayRHS").append_child("ArrayValue");
    arrayXml.append_attribute("Type").set_value("Integer");
    arrayXml.append_child("IntegerValue").append_child(node_pcdata).set_value("2");
    arrayXml.append_child("IntegerValue").append_child(node_pcdata).set_value("3");

    Node *listNode2 = parseNode(listNode2Xml, NULL);
    assertTrue_1(listNode2);
    assertTrue_1(listNode2->getType() == NodeType_NodeList);
    assertTrue_1(!listNode2->getChildren().empty());
    assertTrue_1(listNode2->getChildren().size() == 1);
    assertTrue_1(!listNode2->getLocalVariables().empty());
    assertTrue_1(listNode2->getLocalVariables().size() == 1);

    Node *arrayAssn = listNode2->getChildren().front();
    assertTrue_1(arrayAssn);
    assertTrue_1(arrayAssn->getType() == NodeType_Assignment);
    assertTrue_1(arrayAssn->getChildren().empty());
    assertTrue_1(arrayAssn->getLocalVariables().empty());

    finalizeNode(listNode2, listNode2Xml);
    AssignmentNode *aanode = dynamic_cast<AssignmentNode *>(arrayAssn);
    assertTrue_1(aanode);
    Assignment *aassn = aanode->getAssignment();
    assertTrue_1(aassn);

    Assignable *barVar = listNode2->findLocalVariable("bar")->asAssignable();
    assertTrue_1(barVar);
    assertTrue_1(barVar->valueType() == INTEGER_ARRAY_TYPE);
    assertTrue_1(aassn->getDest() == barVar);

    delete listNode2;
  }

  // Array element assignment
  {
    xml_node listNode3Xml = doc.append_child("Node");
    listNode3Xml.append_attribute("NodeType").set_value("NodeList");
    listNode3Xml.append_child("NodeId").append_child(node_pcdata).set_value("listNode3");
    xml_node listNode3Decls = listNode3Xml.append_child("VariableDeclarations");
    xml_node decl1 = listNode3Decls.append_child("DeclareArray");
    decl1.append_child("Name").append_child(node_pcdata).set_value("baz");
    decl1.append_child("Type").append_child(node_pcdata).set_value("Integer");
    decl1.append_child("MaxSize").append_child(node_pcdata).set_value("2");
    xml_node listNode3List = listNode3Xml.append_child("NodeBody").append_child("NodeList");

    xml_node arrayAssnXml = listNode3List.append_child("Node");
    arrayAssnXml.append_attribute("NodeType").set_value("Assignment");
    arrayAssnXml.append_child("NodeId").append_child(node_pcdata).set_value("arrayAssn");
    xml_node assnXml = arrayAssnXml.append_child("NodeBody").append_child("Assignment");
    xml_node elemXml = assnXml.append_child("ArrayElement");
    elemXml.append_child("Name").append_child(node_pcdata).set_value("baz");
    elemXml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    assnXml.append_child("NumericRHS").append_child("IntegerValue").append_child(node_pcdata).set_value("3");

    Node *listNode3 = parseNode(listNode3Xml, NULL);
    assertTrue_1(listNode3);
    assertTrue_1(listNode3->getType() == NodeType_NodeList);
    assertTrue_1(!listNode3->getChildren().empty());
    assertTrue_1(listNode3->getChildren().size() == 1);
    assertTrue_1(!listNode3->getLocalVariables().empty());
    assertTrue_1(listNode3->getLocalVariables().size() == 1);

    Node *arrayAssn = listNode3->getChildren().front();
    assertTrue_1(arrayAssn);
    assertTrue_1(arrayAssn->getType() == NodeType_Assignment);
    assertTrue_1(arrayAssn->getChildren().empty());
    assertTrue_1(arrayAssn->getLocalVariables().empty());

    finalizeNode(listNode3, listNode3Xml);
    AssignmentNode *aanode = dynamic_cast<AssignmentNode *>(arrayAssn);
    assertTrue_1(aanode);
    Assignment *aassn = aanode->getAssignment();
    assertTrue_1(aassn);

    Assignable *bazVar = listNode3->findLocalVariable("baz")->asAssignable();
    assertTrue_1(bazVar);
    assertTrue_1(bazVar->valueType() == INTEGER_ARRAY_TYPE);
    assertTrue_1(aassn->getDest()->valueType() == INTEGER_TYPE);
    assertTrue_1(aassn->getDest() != bazVar);
    assertTrue_1(aassn->getDest()->asAssignable()->getBaseVariable() == bazVar);

    delete listNode3;
  }

  return true;
}

static bool commandNodeXmlParserTest()
{
  xml_document doc;
  doc.set_name("commandNodeXmlParserTest");

  xml_node basicCmdXml = doc.append_child("Node");
  basicCmdXml.append_attribute("NodeType").set_value("Command");
  basicCmdXml.append_child("NodeId").append_child(node_pcdata).set_value("basicCmd");
  xml_node cmd = basicCmdXml.append_child("NodeBody").append_child("Command");

  return true;
}

static bool updateNodeXmlParserTest()
{
  xml_document doc;
  doc.set_name("updateNodeXmlParserTest");

  xml_node basicUpdXml = doc.append_child("Node");
  basicUpdXml.append_attribute("NodeType").set_value("Update");
  basicUpdXml.append_child("NodeId").append_child(node_pcdata).set_value("basicUpd");
  xml_node upd = basicUpdXml.append_child("NodeBody").append_child("Update");

  return true;
}

static bool libraryCallNodeXmlParserTest()
{
  xml_document doc;
  doc.set_name("libraryCallNodeXmlParserTest");

  xml_node basicLibCallXml = doc.append_child("Node");
  basicLibCallXml.append_attribute("NodeType").set_value("LibraryNodeCall");
  basicLibCallXml.append_child("NodeId").append_child(node_pcdata).set_value("basicLibCall");
  xml_node libCall = basicLibCallXml.append_child("NodeBody").append_child("LibraryNodeCall");

  return true;
}

bool nodeXmlParserTest()
{
  TransitionExternalInterface tif;
  g_interface = &tif;

  runTest(emptyNodeXmlParserTest);
  runTest(listNodeXmlParserTest);
  runTest(assignmentNodeXmlParserTest);
  runTest(commandNodeXmlParserTest);
  runTest(updateNodeXmlParserTest);
  runTest(libraryCallNodeXmlParserTest);

  g_interface = NULL;
  return true;
}
