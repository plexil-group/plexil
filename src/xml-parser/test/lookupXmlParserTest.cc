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
#include "test/FactoryTestNodeConnector.hh"
#include "Lookup.hh"
#include "TestSupport.hh"

#include "pugixml.hpp"

#include <cstring>

using namespace PLEXIL;

using pugi::xml_attribute;
using pugi::xml_document;
using pugi::xml_node;
using pugi::node_pcdata;

static bool testBasics()
{
  FactoryTestNodeConnector conn;
  bool wasCreated = false;

  xml_document doc;

  // Basics
  {
    xml_node test1Xml = doc.append_child("LookupNow");
    test1Xml.append_child("Name").append_child("StringValue").append_child(node_pcdata).set_value("foo");

    Expression *lookup1 = createExpression(test1Xml, &conn, wasCreated);
    assertTrue_1(lookup1);
    assertTrue_1(wasCreated);
    assertTrue_1(0 == strcmp(lookup1->exprName(), "LookupNow"));
    delete lookup1;
  }

  {
    xml_node test2Xml = doc.append_child("LookupOnChange");
    test2Xml.append_child("Name").append_child("StringValue").append_child(node_pcdata).set_value("bar");
    test2Xml.append_child("Tolerance").append_child("RealValue").append_child(node_pcdata).set_value("0.5");
    test2Xml.append_child("Arguments").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

    wasCreated = false;
    Expression *lookup2 = createExpression(test2Xml, &conn, wasCreated);
    assertTrue_1(lookup2);
    assertTrue_1(wasCreated);
    assertTrue_1(0 == strcmp(lookup2->exprName(), "LookupOnChange"));
    delete lookup2;
  }

  {
    xml_node test3Xml = doc.append_child("LookupOnChange");
    test3Xml.append_child("Name").append_child("StringValue").append_child(node_pcdata).set_value("baz");
    xml_node argsXml = test3Xml.append_child("Arguments");
    argsXml.append_child("IntegerValue").append_child(node_pcdata).set_value("1");
    argsXml.append_child("RealValue").append_child(node_pcdata).set_value("1.5");
    argsXml.append_child("StringValue").append_child(node_pcdata).set_value("too");

    wasCreated = false;
    Expression *lookup3 = createExpression(test3Xml, &conn, wasCreated);
    assertTrue_1(lookup3);
    assertTrue_1(wasCreated);
    assertTrue_1(0 == strcmp(lookup3->exprName(), "LookupNow"));
    delete lookup3;
  }

  return true;
}

bool lookupXmlParserTest()
{
  runTest(testBasics);
  return true;
}
