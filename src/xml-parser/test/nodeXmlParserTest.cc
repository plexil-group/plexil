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
#include "Node.hh"
#include "parseNode.hh"
#include "TestSupport.hh"
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
    finalizeNode(minimalNode, minimal);
    assertTrue_1(minimalNode->getChildren().empty());
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
    finalizeNode(commentNode, comment);
    assertTrue_1(commentNode->getChildren().empty());
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
    finalizeNode(preCondNode, preCond);
    assertTrue_1(preCondNode->getChildren().empty());
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
    finalizeNode(endCondNode, endCond);
    assertTrue_1(endCondNode->getChildren().empty());
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
    finalizeNode(exitCondNode, exitCond);
    assertTrue_1(exitCondNode->getChildren().empty());
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
    finalizeNode(postCondNode, postCond);
    assertTrue_1(postCondNode->getChildren().empty());
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
    finalizeNode(skipCondNode, skipCond);
    assertTrue_1(skipCondNode->getChildren().empty());
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
    finalizeNode(startCondNode, startCond);
    assertTrue_1(startCondNode->getChildren().empty());
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
    finalizeNode(repeatCondNode, repeatCond);
    assertTrue_1(repeatCondNode->getChildren().empty());
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
    finalizeNode(invariantCondNode, invariantCond);
    assertTrue_1(invariantCondNode->getChildren().empty());
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
    finalizeNode(simpleVarDeclNode, simpleVarDecl);
    assertTrue_1(simpleVarDeclNode->getChildren().empty());
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
    finalizeNode(initedVarDeclNode, initedVarDecl);
    assertTrue_1(initedVarDeclNode->getChildren().empty());
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
    finalizeNode(simpleArrayVarDeclNode, simpleArrayVarDecl);
    assertTrue_1(simpleArrayVarDeclNode->getChildren().empty());
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
    finalizeNode(initedArrayVarDeclNode, initedArrayVarDecl);
    assertTrue_1(initedArrayVarDeclNode->getChildren().empty());
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

bool nodeXmlParserTest()
{
  TransitionExternalInterface tif;
  g_interface = &tif;

  runTest(emptyNodeXmlParserTest);

  g_interface = NULL;
  return true;
}
