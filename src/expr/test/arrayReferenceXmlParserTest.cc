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

#include "Debug.hh"
#include "Alias.hh"
#include "ArrayReference.hh"
#include "ArrayVariable.hh"
#include "Constant.hh"
#include "ExpressionFactories.hh"
#include "ExpressionFactory.hh"
#include "FactoryTestNodeConnector.hh"
#include "TestSupport.hh"
#include "UserVariable.hh"

#include "pugixml.hpp"

#include <cstring> // for strcmp()

using namespace PLEXIL;

using pugi::xml_attribute;
using pugi::xml_document;
using pugi::xml_node;
using pugi::node_pcdata;

// Global variables for convenience
static NodeConnector *nc = NULL;
static FactoryTestNodeConnector *realNc = NULL;

static bool testArrayConstantReferenceXmlParser()
{
  // Set up test data
  std::vector<bool>        vb(2);
  std::vector<int32_t>     vi(4);
  std::vector<double>      vd(4);
  std::vector<std::string> vs(4);

  vb[0] = false;
  vb[1] = true;

  vi[0] = 0;
  vi[1] = 1;
  vi[2] = 2;
  vi[3] = 3;

  vd[0] = 0;
  vd[1] = 1;
  vd[2] = 2;
  vd[3] = 3;

  vs[0] = std::string("zero");
  vs[1] = std::string("one");
  vs[2] = std::string("two");
  vs[3] = std::string("three");

  BooleanArrayConstant bc(vb);
  IntegerArrayConstant ic(vi);
  RealArrayConstant    dc(vd);
  StringArrayConstant  sc(vs);
  
  // Associate arrays with names
  realNc->storeVariable("bul", &bc);
  realNc->storeVariable("int", &ic);
  realNc->storeVariable("dbl", &dc);
  realNc->storeVariable("str", &sc);

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", &iv);

  xml_document doc;
  doc.set_name("testArrayConstantReferenceXmlParser");

  bool wasCreated = false;
  
  xml_node bart0Xml = doc.append_child("ArrayElement");
  bart0Xml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *bar0 = createExpression(bart0Xml, nc, wasCreated);
  assertTrue_1(bar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);
  
  xml_node bartiXml = doc.append_child("ArrayElement");
  bartiXml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *bari = createExpression(bartiXml, nc, wasCreated);
  assertTrue_1(bari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  xml_node iart0Xml = doc.append_child("ArrayElement");
  iart0Xml.append_child("Name").append_child(node_pcdata).set_value("int");
  iart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *iar0 = createExpression(iart0Xml, nc, wasCreated);
  assertTrue_1(iar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);
  
  xml_node iartiXml = doc.append_child("ArrayElement");
  iartiXml.append_child("Name").append_child(node_pcdata).set_value("int");
  iartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *iari = createExpression(iartiXml, nc, wasCreated);
  assertTrue_1(iari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  xml_node dart0Xml = doc.append_child("ArrayElement");
  dart0Xml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *dar0 = createExpression(dart0Xml, nc, wasCreated);
  assertTrue_1(dar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);
  
  xml_node dartiXml = doc.append_child("ArrayElement");
  dartiXml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *dari = createExpression(dartiXml, nc, wasCreated);
  assertTrue_1(dari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  xml_node sart0Xml = doc.append_child("ArrayElement");
  sart0Xml.append_child("Name").append_child(node_pcdata).set_value("str");
  sart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *sar0 = createExpression(sart0Xml, nc, wasCreated);
  assertTrue_1(sar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);
  
  xml_node sartiXml = doc.append_child("ArrayElement");
  sartiXml.append_child("Name").append_child(node_pcdata).set_value("str");
  sartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *sari = createExpression(sartiXml, nc, wasCreated);
  assertTrue_1(sari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sari->exprName()));
  assertTrue_1(sari->valueType() == STRING_TYPE);

  // Check that array ref values are unknown while inactive
  assertTrue_1(!bar0->isKnown());
  assertTrue_1(!bari->isKnown());
  assertTrue_1(!iar0->isKnown());
  assertTrue_1(!iari->isKnown());
  assertTrue_1(!dar0->isKnown());
  assertTrue_1(!dari->isKnown());
  assertTrue_1(!sar0->isKnown());
  assertTrue_1(!sari->isKnown());

  bool        pb;
  int32_t     pi;
  double      pd;
  std::string ps;

  int32_t n;

  // Check boolean
  bar0->activate();
  assertTrue_1(bar0->getValue(pb));
  assertTrue_1(!pb);

  bari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vb.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(bari->getValue(pb));
    assertTrue_1(pb == vb[i]);
  }
  bari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check integer
  iar0->activate();
  assertTrue_1(iar0->getValue(pi));
  assertTrue_1(pi == 0);

  iari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vi.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(iari->getValue(pi));
    assertTrue_1(pi == vi[i]);
  }
  iari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check double
  dar0->activate();
  assertTrue_1(dar0->getValue(pd));
  assertTrue_1(pd == 0);

  dari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vd.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(dari->getValue(pd));
    assertTrue_1(pd == vd[i]);
  }
  dari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check string
  sar0->activate();
  assertTrue_1(sar0->getValue(ps));
  assertTrue_1(ps == "zero");

  sari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vs.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(sari->getValue(ps));
    assertTrue_1(ps == vs[i]);
  }

  delete bar0;
  delete bari;
  delete iar0;
  delete iari;
  delete dar0;
  delete dari;
  delete sar0;
  delete sari;

  return true;
}

static bool testArrayVariableReferenceXmlParser()
{
  // Set up test data
  std::vector<bool>        vb(2);
  std::vector<int32_t>     vi(4);
  std::vector<double>      vd(4);
  std::vector<std::string> vs(4);

  vb[0] = false;
  vb[1] = true;

  vi[0] = 0;
  vi[1] = 1;
  vi[2] = 2;
  vi[3] = 3;

  vd[0] = 0;
  vd[1] = 1;
  vd[2] = 2;
  vd[3] = 3;

  vs[0] = std::string("zero");
  vs[1] = std::string("one");
  vs[2] = std::string("two");
  vs[3] = std::string("three");

  BooleanArrayConstant bc(vb);
  IntegerArrayConstant ic(vi);
  RealArrayConstant    dc(vd);
  StringArrayConstant  sc(vs);
  
  BooleanArrayVariable bav(nc, "bul", NULL, &bc, false, false);
  IntegerArrayVariable iav(nc, "int", NULL, &ic, false, false);
  RealArrayVariable    dav(nc, "dbl", NULL, &dc, false, false);
  StringArrayVariable  sav(nc, "str", NULL, &sc, false, false);

  // Associate arrays with names
  realNc->storeVariable("bul", &bav);
  realNc->storeVariable("int", &iav);
  realNc->storeVariable("dbl", &dav);
  realNc->storeVariable("str", &sav);

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", &iv);

  bool wasCreated = false;

  xml_document doc;
  doc.set_name("testArrayVariableReferenceXmlParser");
  
  xml_node bart0Xml = doc.append_child("ArrayElement");
  bart0Xml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *bar0 = createExpression(bart0Xml, nc, wasCreated);
  assertTrue_1(bar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);
  
  xml_node bartiXml = doc.append_child("ArrayElement");
  bartiXml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *bari = createExpression(bartiXml, nc, wasCreated);
  assertTrue_1(bari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  xml_node iart0Xml = doc.append_child("ArrayElement");
  iart0Xml.append_child("Name").append_child(node_pcdata).set_value("int");
  iart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *iar0 = createExpression(iart0Xml, nc, wasCreated);
  assertTrue_1(iar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);
  
  xml_node iartiXml = doc.append_child("ArrayElement");
  iartiXml.append_child("Name").append_child(node_pcdata).set_value("int");
  iartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *iari = createExpression(iartiXml, nc, wasCreated);
  assertTrue_1(iari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  xml_node dart0Xml = doc.append_child("ArrayElement");
  dart0Xml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *dar0 = createExpression(dart0Xml, nc, wasCreated);
  assertTrue_1(dar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);
  
  xml_node dartiXml = doc.append_child("ArrayElement");
  dartiXml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *dari = createExpression(dartiXml, nc, wasCreated);
  assertTrue_1(dari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  xml_node sart0Xml = doc.append_child("ArrayElement");
  sart0Xml.append_child("Name").append_child(node_pcdata).set_value("str");
  sart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *sar0 = createExpression(sart0Xml, nc, wasCreated);
  assertTrue_1(sar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);

  xml_node sartiXml = doc.append_child("ArrayElement");
  sartiXml.append_child("Name").append_child(node_pcdata).set_value("str");
  sartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *sari = createExpression(sartiXml, nc, wasCreated);
  assertTrue_1(sari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sari->exprName()));
  assertTrue_1(sari->valueType() == STRING_TYPE);

  // Check that array ref values are unknown while inactive
  assertTrue_1(!bar0->isKnown());
  assertTrue_1(!bari->isKnown());
  assertTrue_1(!iar0->isKnown());
  assertTrue_1(!iari->isKnown());
  assertTrue_1(!dar0->isKnown());
  assertTrue_1(!dari->isKnown());
  assertTrue_1(!sar0->isKnown());
  assertTrue_1(!sari->isKnown());

  bool        pb;
  int32_t     pi;
  double      pd;
  std::string ps;

  int32_t n;

  // Check boolean
  bar0->activate();
  assertTrue_1(bar0->getValue(pb));
  assertTrue_1(!pb);

  bari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vb.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(bari->getValue(pb));
    assertTrue_1(pb == vb[i]);
  }
  bari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check integer
  iar0->activate();
  assertTrue_1(iar0->getValue(pi));
  assertTrue_1(pi == 0);

  iari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vi.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(iari->getValue(pi));
    assertTrue_1(pi == vi[i]);
  }
  iari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check double
  dar0->activate();
  assertTrue_1(dar0->getValue(pd));
  assertTrue_1(pd == 0);

  dari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vd.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(dari->getValue(pd));
    assertTrue_1(pd == vd[i]);
  }
  dari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check string
  sar0->activate();
  assertTrue_1(sar0->getValue(ps));
  assertTrue_1(ps == "zero");

  sari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vs.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(sari->getValue(ps));
    assertTrue_1(ps == vs[i]);
  }

  delete bar0;
  delete bari;
  delete iar0;
  delete iari;
  delete dar0;
  delete dari;
  delete sar0;
  delete sari;

  return true;
}

static bool testArrayAliasReferenceXmlParser()
{
  // Set up test data
  std::vector<bool>        vb(2);
  std::vector<int32_t>     vi(4);
  std::vector<double>      vd(4);
  std::vector<std::string> vs(4);

  vb[0] = false;
  vb[1] = true;

  vi[0] = 0;
  vi[1] = 1;
  vi[2] = 2;
  vi[3] = 3;

  vd[0] = 0;
  vd[1] = 1;
  vd[2] = 2;
  vd[3] = 3;

  vs[0] = std::string("zero");
  vs[1] = std::string("one");
  vs[2] = std::string("two");
  vs[3] = std::string("three");

  BooleanArrayConstant bc(vb);
  IntegerArrayConstant ic(vi);
  RealArrayConstant    dc(vd);
  StringArrayConstant  sc(vs);
  
  BooleanArrayVariable bav(nc, "rbul", NULL, &bc, false, false);
  IntegerArrayVariable iav(nc, "rint", NULL, &ic, false, false);
  RealArrayVariable    dav(nc, "rdbl", NULL, &dc, false, false);
  StringArrayVariable  sav(nc, "rstr", NULL, &sc, false, false);

  Alias abav(nc, "bul", &bav, false);
  Alias aiav(nc, "int", &iav, false);
  Alias adav(nc, "dbl", &dav, false);
  Alias asav(nc, "str", &sav, false);

  // Associate aliases with names
  realNc->storeVariable("bul", &abav);
  realNc->storeVariable("int", &aiav);
  realNc->storeVariable("dbl", &adav);
  realNc->storeVariable("str", &asav);

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", &iv);

  bool wasCreated = false;

  xml_document doc;
  doc.set_name("testArrayAliasReferenceXmlParser");
  
  xml_node bart0Xml = doc.append_child("ArrayElement");
  bart0Xml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *bar0 = createExpression(bart0Xml, nc, wasCreated);
  assertTrue_1(bar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);
  
  xml_node bartiXml = doc.append_child("ArrayElement");
  bartiXml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *bari = createExpression(bartiXml, nc, wasCreated);
  assertTrue_1(bari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  xml_node iart0Xml = doc.append_child("ArrayElement");
  iart0Xml.append_child("Name").append_child(node_pcdata).set_value("int");
  iart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *iar0 = createExpression(iart0Xml, nc, wasCreated);
  assertTrue_1(iar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);
  
  xml_node iartiXml = doc.append_child("ArrayElement");
  iartiXml.append_child("Name").append_child(node_pcdata).set_value("int");
  iartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *iari = createExpression(iartiXml, nc, wasCreated);
  assertTrue_1(iari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  xml_node dart0Xml = doc.append_child("ArrayElement");
  dart0Xml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *dar0 = createExpression(dart0Xml, nc, wasCreated);
  assertTrue_1(dar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);
  
  xml_node dartiXml = doc.append_child("ArrayElement");
  dartiXml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *dari = createExpression(dartiXml, nc, wasCreated);
  assertTrue_1(dari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  xml_node sart0Xml = doc.append_child("ArrayElement");
  sart0Xml.append_child("Name").append_child(node_pcdata).set_value("str");
  sart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Expression *sar0 = createExpression(sart0Xml, nc, wasCreated);
  assertTrue_1(sar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);
  
  xml_node sartiXml = doc.append_child("ArrayElement");
  sartiXml.append_child("Name").append_child(node_pcdata).set_value("str");
  sartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Expression *sari = createExpression(sartiXml, nc, wasCreated);
  assertTrue_1(sari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sari->exprName()));
  assertTrue_1(sari->valueType() == STRING_TYPE);

  // Check that array ref values are unknown while inactive
  assertTrue_1(!bar0->isKnown());
  assertTrue_1(!bari->isKnown());
  assertTrue_1(!iar0->isKnown());
  assertTrue_1(!iari->isKnown());
  assertTrue_1(!dar0->isKnown());
  assertTrue_1(!dari->isKnown());
  assertTrue_1(!sar0->isKnown());
  assertTrue_1(!sari->isKnown());

  // Activate arrays (NOT aliases) and try again
  bav.activate();
  iav.activate();
  dav.activate();
  sav.activate();
  assertTrue_1(!bar0->isKnown());
  assertTrue_1(!bari->isKnown());
  assertTrue_1(!iar0->isKnown());
  assertTrue_1(!iari->isKnown());
  assertTrue_1(!dar0->isKnown());
  assertTrue_1(!dari->isKnown());
  assertTrue_1(!sar0->isKnown());
  assertTrue_1(!sari->isKnown());

  bool        pb;
  int32_t     pi;
  double      pd;
  std::string ps;

  int32_t n;

  // Check boolean
  bar0->activate();
  assertTrue_1(bar0->getValue(pb));
  assertTrue_1(!pb);

  bari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vb.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(bari->getValue(pb));
    assertTrue_1(pb == vb[i]);
  }
  bari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check integer
  iar0->activate();
  assertTrue_1(iar0->getValue(pi));
  assertTrue_1(pi == 0);

  iari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vi.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(iari->getValue(pi));
    assertTrue_1(pi == vi[i]);
  }
  iari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check double
  dar0->activate();
  assertTrue_1(dar0->getValue(pd));
  assertTrue_1(pd == 0);

  dari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vd.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(dari->getValue(pd));
    assertTrue_1(pd == vd[i]);
  }
  dari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check string
  sar0->activate();
  assertTrue_1(sar0->getValue(ps));
  assertTrue_1(ps == "zero");

  sari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vs.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(sari->getValue(ps));
    assertTrue_1(ps == vs[i]);
  }

  delete bar0;
  delete bari;
  delete iar0;
  delete iari;
  delete dar0;
  delete dari;
  delete sar0;
  delete sari;

  return true;
}

static bool testMutableArrayReferenceXmlParser()
{
  // Set up test data
  std::vector<bool>        vb(2);
  std::vector<int32_t>     vi(4);
  std::vector<double>      vd(4);
  std::vector<std::string> vs(4);

  vb[0] = false;
  vb[1] = true;

  vi[0] = 0;
  vi[1] = 1;
  vi[2] = 2;
  vi[3] = 3;

  vd[0] = 0;
  vd[1] = 1;
  vd[2] = 2;
  vd[3] = 3;

  vs[0] = std::string("zero");
  vs[1] = std::string("one");
  vs[2] = std::string("two");
  vs[3] = std::string("three");

  BooleanArrayConstant bc(vb);
  IntegerArrayConstant ic(vi);
  RealArrayConstant    dc(vd);
  StringArrayConstant  sc(vs);
  
  BooleanArrayVariable bav(nc, "bul", NULL, &bc, false, false);
  IntegerArrayVariable iav(nc, "int", NULL, &ic, false, false);
  RealArrayVariable    dav(nc, "dbl", NULL, &dc, false, false);
  StringArrayVariable  sav(nc, "str", NULL, &sc, false, false);

  // Associate arrays with names
  realNc->storeVariable("bul", &bav);
  realNc->storeVariable("int", &iav);
  realNc->storeVariable("dbl", &dav);
  realNc->storeVariable("str", &sav);

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", &iv);

  bool wasCreated = false;

  xml_document doc;
  doc.set_name("testMutableArrayReferenceXmlParser");
  
  xml_node bart0Xml = doc.append_child("ArrayElement");
  bart0Xml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Assignable *bar0 = createAssignable(bart0Xml, nc, wasCreated);
  assertTrue_1(bar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);

  xml_node bartiXml = doc.append_child("ArrayElement");
  bartiXml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Assignable *bari = createAssignable(bartiXml, nc, wasCreated);
  assertTrue_1(bari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  xml_node iart0Xml = doc.append_child("ArrayElement");
  iart0Xml.append_child("Name").append_child(node_pcdata).set_value("int");
  iart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Assignable *iar0 = createAssignable(iart0Xml, nc, wasCreated);
  assertTrue_1(iar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);
  
  xml_node iartiXml = doc.append_child("ArrayElement");
  iartiXml.append_child("Name").append_child(node_pcdata).set_value("int");
  iartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Assignable *iari = createAssignable(iartiXml, nc, wasCreated);
  assertTrue_1(iari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  xml_node dart0Xml = doc.append_child("ArrayElement");
  dart0Xml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Assignable *dar0 = createAssignable(dart0Xml, nc, wasCreated);
  assertTrue_1(dar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);
  
  xml_node dartiXml = doc.append_child("ArrayElement");
  dartiXml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Assignable *dari = createAssignable(dartiXml, nc, wasCreated);
  assertTrue_1(dari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  xml_node sart0Xml = doc.append_child("ArrayElement");
  sart0Xml.append_child("Name").append_child(node_pcdata).set_value("str");
  sart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Assignable *sar0 = createAssignable(sart0Xml, nc, wasCreated);
  assertTrue_1(sar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);
  
  xml_node sartiXml = doc.append_child("ArrayElement");
  sartiXml.append_child("Name").append_child(node_pcdata).set_value("str");
  sartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Assignable *sari = createAssignable(sartiXml, nc, wasCreated);
  assertTrue_1(sari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sari->exprName()));
  assertTrue_1(sari->valueType() == STRING_TYPE);

  // Check that array ref values are unknown while inactive
  assertTrue_1(!bar0->isKnown());
  assertTrue_1(!bari->isKnown());
  assertTrue_1(!iar0->isKnown());
  assertTrue_1(!iari->isKnown());
  assertTrue_1(!dar0->isKnown());
  assertTrue_1(!dari->isKnown());
  assertTrue_1(!sar0->isKnown());
  assertTrue_1(!sari->isKnown());

  bool        pb;
  int32_t     pi;
  double      pd;
  std::string ps;

  int32_t n;

  // Read tests

  // Check boolean
  bar0->activate();
  assertTrue_1(bar0->getValue(pb));
  assertTrue_1(!pb);

  bari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vb.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(bari->getValue(pb));
    assertTrue_1(pb == vb[i]);
  }
  bari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check integer
  iar0->activate();
  assertTrue_1(iar0->getValue(pi));
  assertTrue_1(pi == 0);

  iari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vi.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(iari->getValue(pi));
    assertTrue_1(pi == vi[i]);
  }
  iari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check double
  dar0->activate();
  assertTrue_1(dar0->getValue(pd));
  assertTrue_1(pd == 0);

  dari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vd.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(dari->getValue(pd));
    assertTrue_1(pd == vd[i]);
  }
  dari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check string
  sar0->activate();
  assertTrue_1(sar0->getValue(ps));
  assertTrue_1(ps == "zero");

  sari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vs.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(sari->getValue(ps));
    assertTrue_1(ps == vs[i]);
  }

  // Write tests

  // Boolean
  bari->activate();
  for (int32_t i = 0; i < vb.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(bari->getValue(pb));
    assertTrue_1(pb == vb[i]);
    bari->setValue(!pb);
    assertTrue_1(bari->getValue(pb));
    assertTrue_1(pb == !vb[i]);
  }
  
  // Integer
  iari->activate();
  for (int32_t i = 0; i < vi.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(iari->getValue(pi));
    assertTrue_1(pi == vi[i]);
    iari->setValue(-pi);
    pi = 0;
    assertTrue_1(iari->getValue(pi));
    assertTrue_1(pi == -vi[i]);
  }

  // Real
  dari->activate();
  for (int32_t i = 0; i < vd.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(dari->getValue(pd));
    assertTrue_1(pd == vd[i]);
    dari->setValue(-pd);
    pd = 0;
    assertTrue_1(dari->getValue(pd));
    assertTrue_1(pd == -vd[i]);
  }

  // String
  sari->activate();
  for (int32_t i = 0; i < vs.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(sari->getValue(ps));
    assertTrue_1(ps == vs[i]);
    ps.push_back('x');
    sari->setValue(ps);
    ps.clear();
    assertTrue_1(sari->getValue(ps));
    assertTrue_1(ps != vs[i]);
    assertTrue_1(ps.substr(0, ps.size() -1) == vs[i]);
  }

  delete bar0;
  delete bari;
  delete iar0;
  delete iari;
  delete dar0;
  delete dari;
  delete sar0;
  delete sari;

  return true;
}

static bool testMutableArrayAliasReferenceXmlParser()
{
  // Set up test data
  std::vector<bool>        vb(2);
  std::vector<int32_t>     vi(4);
  std::vector<double>      vd(4);
  std::vector<std::string> vs(4);

  vb[0] = false;
  vb[1] = true;

  vi[0] = 0;
  vi[1] = 1;
  vi[2] = 2;
  vi[3] = 3;

  vd[0] = 0;
  vd[1] = 1;
  vd[2] = 2;
  vd[3] = 3;

  vs[0] = std::string("zero");
  vs[1] = std::string("one");
  vs[2] = std::string("two");
  vs[3] = std::string("three");

  BooleanArrayConstant bc(vb);
  IntegerArrayConstant ic(vi);
  RealArrayConstant    dc(vd);
  StringArrayConstant  sc(vs);
  
  BooleanArrayVariable bav(nc, "rbul", NULL, &bc, false, false);
  IntegerArrayVariable iav(nc, "rint", NULL, &ic, false, false);
  RealArrayVariable    dav(nc, "rdbl", NULL, &dc, false, false);
  StringArrayVariable  sav(nc, "rstr", NULL, &sc, false, false);

  InOutAlias abav(nc, "bul", &bav, false);
  InOutAlias aiav(nc, "int", &iav, false);
  InOutAlias adav(nc, "dbl", &dav, false);
  InOutAlias asav(nc, "str", &sav, false);

  // Associate aliases with names
  realNc->storeVariable("bul", &abav);
  realNc->storeVariable("int", &aiav);
  realNc->storeVariable("dbl", &adav);
  realNc->storeVariable("str", &asav);

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", &iv);

  bool wasCreated = false;

  xml_document doc;
  doc.set_name("testMutableArrayAliasReferenceXmlParser");
  
  xml_node bart0Xml = doc.append_child("ArrayElement");
  bart0Xml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Assignable *bar0 = createAssignable(bart0Xml, nc, wasCreated);
  assertTrue_1(bar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);
  
  xml_node bartiXml = doc.append_child("ArrayElement");
  bartiXml.append_child("Name").append_child(node_pcdata).set_value("bul");
  bartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Assignable *bari = createAssignable(bartiXml, nc, wasCreated);
  assertTrue_1(bari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  xml_node iart0Xml = doc.append_child("ArrayElement");
  iart0Xml.append_child("Name").append_child(node_pcdata).set_value("int");
  iart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Assignable *iar0 = createAssignable(iart0Xml, nc, wasCreated);
  assertTrue_1(iar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);
  
  xml_node iartiXml = doc.append_child("ArrayElement");
  iartiXml.append_child("Name").append_child(node_pcdata).set_value("int");
  iartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Assignable *iari = createAssignable(iartiXml, nc, wasCreated);
  assertTrue_1(iari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  xml_node dart0Xml = doc.append_child("ArrayElement");
  dart0Xml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Assignable *dar0 = createAssignable(dart0Xml, nc, wasCreated);
  assertTrue_1(dar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);
  
  xml_node dartiXml = doc.append_child("ArrayElement");
  dartiXml.append_child("Name").append_child(node_pcdata).set_value("dbl");
  dartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Assignable *dari = createAssignable(dartiXml, nc, wasCreated);
  assertTrue_1(dari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  xml_node sart0Xml = doc.append_child("ArrayElement");
  sart0Xml.append_child("Name").append_child(node_pcdata).set_value("str");
  sart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

  Assignable *sar0 = createAssignable(sart0Xml, nc, wasCreated);
  assertTrue_1(sar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);
  
  xml_node sartiXml = doc.append_child("ArrayElement");
  sartiXml.append_child("Name").append_child(node_pcdata).set_value("str");
  sartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

  Assignable *sari = createAssignable(sartiXml, nc, wasCreated);
  assertTrue_1(sari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sari->exprName()));
  assertTrue_1(sari->valueType() == STRING_TYPE);

  // Check that array ref values are unknown while inactive
  assertTrue_1(!bar0->isKnown());
  assertTrue_1(!bari->isKnown());
  assertTrue_1(!iar0->isKnown());
  assertTrue_1(!iari->isKnown());
  assertTrue_1(!dar0->isKnown());
  assertTrue_1(!dari->isKnown());
  assertTrue_1(!sar0->isKnown());
  assertTrue_1(!sari->isKnown());

  // Activate arrays (NOT aliases) and try again
  bav.activate();
  iav.activate();
  dav.activate();
  sav.activate();
  assertTrue_1(!bar0->isKnown());
  assertTrue_1(!bari->isKnown());
  assertTrue_1(!iar0->isKnown());
  assertTrue_1(!iari->isKnown());
  assertTrue_1(!dar0->isKnown());
  assertTrue_1(!dari->isKnown());
  assertTrue_1(!sar0->isKnown());
  assertTrue_1(!sari->isKnown());

  bool        pb;
  int32_t     pi;
  double      pd;
  std::string ps;

  int32_t n;

  // Read tests

  // Check boolean
  bar0->activate();
  assertTrue_1(bar0->getValue(pb));
  assertTrue_1(!pb);

  bari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vb.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(bari->getValue(pb));
    assertTrue_1(pb == vb[i]);
  }
  bari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check integer
  iar0->activate();
  assertTrue_1(iar0->getValue(pi));
  assertTrue_1(pi == 0);

  iari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vi.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(iari->getValue(pi));
    assertTrue_1(pi == vi[i]);
  }
  iari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check double
  dar0->activate();
  assertTrue_1(dar0->getValue(pd));
  assertTrue_1(pd == 0);

  dari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vd.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(dari->getValue(pd));
    assertTrue_1(pd == vd[i]);
  }
  dari->deactivate();
  assertTrue_1(!iv.isActive());

  // Check string
  sar0->activate();
  assertTrue_1(sar0->getValue(ps));
  assertTrue_1(ps == "zero");

  sari->activate();
  assertTrue_1(iv.isActive());
  for (int32_t i = 0; i < vs.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(sari->getValue(ps));
    assertTrue_1(ps == vs[i]);
  }

  // Write tests

  // Boolean
  bari->activate();
  for (int32_t i = 0; i < vb.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(bari->getValue(pb));
    assertTrue_1(pb == vb[i]);
    bari->setValue(!pb);
    assertTrue_1(bari->getValue(pb));
    assertTrue_1(pb == !vb[i]);
  }
  
  // Integer
  iari->activate();
  for (int32_t i = 0; i < vi.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(iari->getValue(pi));
    assertTrue_1(pi == vi[i]);
    iari->setValue(-pi);
    pi = 0;
    assertTrue_1(iari->getValue(pi));
    assertTrue_1(pi == -vi[i]);
  }

  // Real
  dari->activate();
  for (int32_t i = 0; i < vd.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(dari->getValue(pd));
    assertTrue_1(pd == vd[i]);
    dari->setValue(-pd);
    pd = 0;
    assertTrue_1(dari->getValue(pd));
    assertTrue_1(pd == -vd[i]);
  }

  // String
  sari->activate();
  for (int32_t i = 0; i < vs.size(); ++i) {
    iv.setValue(i);
    assertTrue_1(iv.getValue(n));
    assertTrue_1(n == i);
    assertTrue_1(sari->getValue(ps));
    assertTrue_1(ps == vs[i]);
    ps.push_back('x');
    sari->setValue(ps);
    ps.clear();
    assertTrue_1(sari->getValue(ps));
    assertTrue_1(ps != vs[i]);
    assertTrue_1(ps.substr(0, ps.size() -1) == vs[i]);
  }

  delete bar0;
  delete bari;
  delete iar0;
  delete iari;
  delete dar0;
  delete dari;
  delete sar0;
  delete sari;

  return true;
}

bool arrayReferenceXmlParserTest()
{
  // Initialize factories
  registerBasicExpressionFactories();
  // Initialize infrastructure
  realNc = new FactoryTestNodeConnector();
  nc = realNc;

  runTest(testArrayConstantReferenceXmlParser);
  runTest(testArrayVariableReferenceXmlParser);
  runTest(testArrayAliasReferenceXmlParser);

  runTest(testMutableArrayReferenceXmlParser);
  runTest(testMutableArrayAliasReferenceXmlParser);

  nc = NULL;
  delete realNc;
  return true;
}
