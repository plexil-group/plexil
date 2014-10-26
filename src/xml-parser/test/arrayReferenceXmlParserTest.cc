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
#include "ExpressionFactory.hh"
#include "test/FactoryTestNodeConnector.hh"
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
  IntegerVariable *iv = new IntegerVariable;
  realNc->storeVariable("i", iv);
  xml_document doc;
  int32_t n;
  bool wasCreated = false;

  // Boolean array
  {
    std::vector<bool> vb(2);
    vb[0] = false;
    vb[1] = true;
    BooleanArrayConstant *bc = new BooleanArrayConstant(vb);
    realNc->storeVariable("bul", bc);
  
    xml_node bart0Xml = doc.append_child("ArrayElement");
    bart0Xml.append_child("Name").append_child(node_pcdata).set_value("bul");
    bart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

    Expression *bar0 = createExpression(bart0Xml, nc, wasCreated);
    assertTrue_1(bar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
    assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!bar0->isKnown());
  
    xml_node bartiXml = doc.append_child("ArrayElement");
    bartiXml.append_child("Name").append_child(node_pcdata).set_value("bul");
    bartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

    Expression *bari = createExpression(bartiXml, nc, wasCreated);
    assertTrue_1(bari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
    assertTrue_1(bari->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!bari->isKnown());

    bool        pb;
    bar0->activate();
    assertTrue_1(bar0->getValue(pb));
    assertTrue_1(!pb);

    bari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vb.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(bari->getValue(pb));
      assertTrue_1(pb == vb[i]);
    }
    bari->deactivate();
    assertTrue_1(!iv->isActive());

    delete bar0;
    delete bari;
  }

  // Integer array
  {
    std::vector<int32_t>     vi(4);
    vi[0] = 0;
    vi[1] = 1;
    vi[2] = 2;
    vi[3] = 3;

    IntegerArrayConstant *ic = new IntegerArrayConstant(vi);
    realNc->storeVariable("int", ic);

    xml_node iart0Xml = doc.append_child("ArrayElement");
    iart0Xml.append_child("Name").append_child(node_pcdata).set_value("int");
    iart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

    Expression *iar0 = createExpression(iart0Xml, nc, wasCreated);
    assertTrue_1(iar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
    assertTrue_1(iar0->valueType() == INTEGER_TYPE);
    assertTrue_1(!iar0->isKnown());
  
    xml_node iartiXml = doc.append_child("ArrayElement");
    iartiXml.append_child("Name").append_child(node_pcdata).set_value("int");
    iartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

    Expression *iari = createExpression(iartiXml, nc, wasCreated);
    assertTrue_1(iari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
    assertTrue_1(iari->valueType() == INTEGER_TYPE);
    assertTrue_1(!iari->isKnown());

    int32_t pi;
    iar0->activate();
    assertTrue_1(iar0->getValue(pi));
    assertTrue_1(pi == 0);

    iari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vi.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(iari->getValue(pi));
      assertTrue_1(pi == vi[i]);
    }
    iari->deactivate();
    assertTrue_1(!iv->isActive());

    delete iar0;
    delete iari;
  }

  // Real array
  {
    std::vector<double> vd(4);
    vd[0] = 0;
    vd[1] = 1;
    vd[2] = 2;
    vd[3] = 3;
    RealArrayConstant *dc = new RealArrayConstant(vd);
    realNc->storeVariable("dbl", dc);

    xml_node dart0Xml = doc.append_child("ArrayElement");
    dart0Xml.append_child("Name").append_child(node_pcdata).set_value("dbl");
    dart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

    Expression *dar0 = createExpression(dart0Xml, nc, wasCreated);
    assertTrue_1(dar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
    assertTrue_1(dar0->valueType() == REAL_TYPE);
    assertTrue_1(!dar0->isKnown());
  
    xml_node dartiXml = doc.append_child("ArrayElement");
    dartiXml.append_child("Name").append_child(node_pcdata).set_value("dbl");
    dartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

    Expression *dari = createExpression(dartiXml, nc, wasCreated);
    assertTrue_1(dari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
    assertTrue_1(dari->valueType() == REAL_TYPE);
    assertTrue_1(!dari->isKnown());

    double      pd;
    dar0->activate();
    assertTrue_1(dar0->getValue(pd));
    assertTrue_1(pd == 0);

    dari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vd.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(dari->getValue(pd));
      assertTrue_1(pd == vd[i]);
    }
    dari->deactivate();
    assertTrue_1(!iv->isActive());

    delete dar0;
    delete dari;
  }

  // String array
  {
    std::vector<std::string> vs(4);
    vs[0] = std::string("zero");
    vs[1] = std::string("one");
    vs[2] = std::string("two");
    vs[3] = std::string("three");

    StringArrayConstant *sc = new StringArrayConstant(vs);
    realNc->storeVariable("str", sc);

    xml_node sart0Xml = doc.append_child("ArrayElement");
    sart0Xml.append_child("Name").append_child(node_pcdata).set_value("str");
    sart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

    Expression *sar0 = createExpression(sart0Xml, nc, wasCreated);
    assertTrue_1(sar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
    assertTrue_1(sar0->valueType() == STRING_TYPE);
    assertTrue_1(!sar0->isKnown());
  
    xml_node sartiXml = doc.append_child("ArrayElement");
    sartiXml.append_child("Name").append_child(node_pcdata).set_value("str");
    sartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

    Expression *sari = createExpression(sartiXml, nc, wasCreated);
    assertTrue_1(sari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", sari->exprName()));
    assertTrue_1(sari->valueType() == STRING_TYPE);
    assertTrue_1(!sari->isKnown());

    std::string ps;
    sar0->activate();
    assertTrue_1(sar0->getValue(ps));
    assertTrue_1(ps == "zero");

    sari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vs.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(sari->getValue(ps));
      assertTrue_1(ps == vs[i]);
    }

    delete sar0;
    delete sari;
  }

  return true;
}

static bool testArrayVariableReferenceXmlParser()
{
  IntegerVariable *iv = new IntegerVariable;
  realNc->storeVariable("i", iv);
  xml_document doc;
  int32_t n;
  bool wasCreated = false;
  
  {
    std::vector<bool> vb(2);
    vb[0] = false;
    vb[1] = true;
    BooleanArrayConstant *bc = new BooleanArrayConstant(vb);
    BooleanArrayVariable *bav = new BooleanArrayVariable(nc, "bul", NULL, false);
    bav->setInitializer(bc, true);
    realNc->storeVariable("bul", bav);

    xml_node bart0Xml = doc.append_child("ArrayElement");
    bart0Xml.append_child("Name").append_child(node_pcdata).set_value("bul");
    bart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");

    Expression *bar0 = createExpression(bart0Xml, nc, wasCreated);
    assertTrue_1(bar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
    assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!bar0->isKnown());
  
    xml_node bartiXml = doc.append_child("ArrayElement");
    bartiXml.append_child("Name").append_child(node_pcdata).set_value("bul");
    bartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

    Expression *bari = createExpression(bartiXml, nc, wasCreated);
    assertTrue_1(bari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
    assertTrue_1(bari->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!bari->isKnown());

    bool pb;
    bar0->activate();
    assertTrue_1(bar0->getValue(pb));
    assertTrue_1(!pb);

    bari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vb.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(bari->getValue(pb));
      assertTrue_1(pb == vb[i]);
    }
    bari->deactivate();
    assertTrue_1(!iv->isActive());

    delete bar0;
    delete bari;
  }

  // Integer array
  {
    std::vector<int32_t> vi(4);
    vi[0] = 0;
    vi[1] = 1;
    vi[2] = 2;
    vi[3] = 3;
    IntegerArrayConstant *ic = new IntegerArrayConstant(vi);
    IntegerArrayVariable *iav = new IntegerArrayVariable(nc, "int", NULL, false);
    iav->setInitializer(ic, true);
    realNc->storeVariable("int", iav);

    xml_node iart0Xml = doc.append_child("ArrayElement");
    iart0Xml.append_child("Name").append_child(node_pcdata).set_value("int");
    iart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Expression *iar0 = createExpression(iart0Xml, nc, wasCreated);
    assertTrue_1(iar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
    assertTrue_1(iar0->valueType() == INTEGER_TYPE);
    assertTrue_1(!iar0->isKnown());
  
    xml_node iartiXml = doc.append_child("ArrayElement");
    iartiXml.append_child("Name").append_child(node_pcdata).set_value("int");
    iartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");

    Expression *iari = createExpression(iartiXml, nc, wasCreated);
    assertTrue_1(iari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
    assertTrue_1(iari->valueType() == INTEGER_TYPE);
    assertTrue_1(!iari->isKnown());

    int32_t pi;
    iar0->activate();
    assertTrue_1(iar0->getValue(pi));
    assertTrue_1(pi == 0);

    iari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vi.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(iari->getValue(pi));
      assertTrue_1(pi == vi[i]);
    }
    iari->deactivate();
    assertTrue_1(!iv->isActive());

    delete iar0;
    delete iari;
  }

  // Real array
  {
    std::vector<double> vd(4);
    vd[0] = 0;
    vd[1] = 1;
    vd[2] = 2;
    vd[3] = 3;
    RealArrayConstant *dc = new RealArrayConstant(vd);
    RealArrayVariable *dav = new RealArrayVariable(nc, "dbl", NULL, false);
    dav->setInitializer(dc, true);
    realNc->storeVariable("dbl", dav);

    xml_node dart0Xml = doc.append_child("ArrayElement");
    dart0Xml.append_child("Name").append_child(node_pcdata).set_value("dbl");
    dart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Expression *dar0 = createExpression(dart0Xml, nc, wasCreated);
    assertTrue_1(dar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
    assertTrue_1(dar0->valueType() == REAL_TYPE);
    assertTrue_1(!dar0->isKnown());
  
    xml_node dartiXml = doc.append_child("ArrayElement");
    dartiXml.append_child("Name").append_child(node_pcdata).set_value("dbl");
    dartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Expression *dari = createExpression(dartiXml, nc, wasCreated);
    assertTrue_1(dari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
    assertTrue_1(dari->valueType() == REAL_TYPE);
    assertTrue_1(!dari->isKnown());

    double pd;
    dar0->activate();
    assertTrue_1(dar0->getValue(pd));
    assertTrue_1(pd == 0);

    dari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vd.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(dari->getValue(pd));
      assertTrue_1(pd == vd[i]);
    }
    dari->deactivate();
    assertTrue_1(!iv->isActive());

    delete dar0;
    delete dari;
  }

  // String array
  {
    std::vector<std::string> vs(4);
    vs[0] = std::string("zero");
    vs[1] = std::string("one");
    vs[2] = std::string("two");
    vs[3] = std::string("three");

    StringArrayConstant *sc = new StringArrayConstant(vs);
    StringArrayVariable *sav = new StringArrayVariable(nc, "str", NULL, false);
    sav->setInitializer(sc, true);
    realNc->storeVariable("str", sav);

    // Store array index too
    xml_node sart0Xml = doc.append_child("ArrayElement");
    sart0Xml.append_child("Name").append_child(node_pcdata).set_value("str");
    sart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Expression *sar0 = createExpression(sart0Xml, nc, wasCreated);
    assertTrue_1(sar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
    assertTrue_1(sar0->valueType() == STRING_TYPE);
    assertTrue_1(!sar0->isKnown());

    xml_node sartiXml = doc.append_child("ArrayElement");
    sartiXml.append_child("Name").append_child(node_pcdata).set_value("str");
    sartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Expression *sari = createExpression(sartiXml, nc, wasCreated);
    assertTrue_1(sari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", sari->exprName()));
    assertTrue_1(sari->valueType() == STRING_TYPE);
    assertTrue_1(!sari->isKnown());

    std::string ps;
    sar0->activate();
    assertTrue_1(sar0->getValue(ps));
    assertTrue_1(ps == "zero");

    sari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vs.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(sari->getValue(ps));
      assertTrue_1(ps == vs[i]);
    }

    delete sar0;
    delete sari;
  }

  return true;
}

static bool testArrayAliasReferenceXmlParser()
{
  IntegerVariable *iv = new IntegerVariable;
  realNc->storeVariable("i", iv);
  xml_document doc;
  int32_t n;
  bool wasCreated = false;

  // Boolean array
  {
    std::vector<bool> vb(2);
    vb[0] = false;
    vb[1] = true;
    BooleanArrayConstant *bc = new BooleanArrayConstant(vb);
    BooleanArrayVariable *bav = new BooleanArrayVariable(nc, "bul", NULL, false);
    bav->setInitializer(bc, true);
    Alias* abav = new Alias(nc, "bul", bav, true);
    realNc->storeVariable("bul", abav);

    xml_node bart0Xml = doc.append_child("ArrayElement");
    bart0Xml.append_child("Name").append_child(node_pcdata).set_value("bul");
    bart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Expression *bar0 = createExpression(bart0Xml, nc, wasCreated);
    assertTrue_1(bar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
    assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!bar0->isKnown());
  
    xml_node bartiXml = doc.append_child("ArrayElement");
    bartiXml.append_child("Name").append_child(node_pcdata).set_value("bul");
    bartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Expression *bari = createExpression(bartiXml, nc, wasCreated);
    assertTrue_1(bari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
    assertTrue_1(bari->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!bari->isKnown());

    bav->activate();
    assertTrue_1(!bar0->isKnown());
    assertTrue_1(!bari->isKnown());

    bool pb;
    bar0->activate();
    assertTrue_1(bar0->getValue(pb));
    assertTrue_1(!pb);

    bari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vb.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(bari->getValue(pb));
      assertTrue_1(pb == vb[i]);
    }
    bari->deactivate();
    assertTrue_1(!iv->isActive());

    delete bar0;
    delete bari;
  }

  // Integer array
  {
    std::vector<int32_t> vi(4);
    vi[0] = 0;
    vi[1] = 1;
    vi[2] = 2;
    vi[3] = 3;
    IntegerArrayConstant *ic = new IntegerArrayConstant(vi);
    IntegerArrayVariable *iav = new IntegerArrayVariable(nc, "int", NULL, false);
    iav->setInitializer(ic, true);
    Alias *aiav = new Alias(nc, "int", iav, true);
    realNc->storeVariable("int", aiav);

    xml_node iart0Xml = doc.append_child("ArrayElement");
    iart0Xml.append_child("Name").append_child(node_pcdata).set_value("int");
    iart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Expression *iar0 = createExpression(iart0Xml, nc, wasCreated);
    assertTrue_1(iar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
    assertTrue_1(iar0->valueType() == INTEGER_TYPE);
    assertTrue_1(!iar0->isKnown());
  
    xml_node iartiXml = doc.append_child("ArrayElement");
    iartiXml.append_child("Name").append_child(node_pcdata).set_value("int");
    iartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Expression *iari = createExpression(iartiXml, nc, wasCreated);
    assertTrue_1(iari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
    assertTrue_1(iari->valueType() == INTEGER_TYPE);
    assertTrue_1(!iari->isKnown());

    iav->activate();
    assertTrue_1(!iar0->isKnown());
    assertTrue_1(!iari->isKnown());

    int32_t pi;
    iar0->activate();
    assertTrue_1(iar0->getValue(pi));
    assertTrue_1(pi == 0);

    iari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vi.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(iari->getValue(pi));
      assertTrue_1(pi == vi[i]);
    }
    iari->deactivate();
    assertTrue_1(!iv->isActive());

    delete iar0;
    delete iari;
  }

  // Real array
  {
    std::vector<double> vd(4);
    vd[0] = 0;
    vd[1] = 1;
    vd[2] = 2;
    vd[3] = 3;
    RealArrayConstant *dc = new RealArrayConstant(vd);
    RealArrayVariable *dav = new RealArrayVariable(nc, "dbl", NULL, false);
    dav->setInitializer(dc, true);
    Alias *adav = new Alias(nc, "dbl", dav, true);
    realNc->storeVariable("dbl", adav);

    xml_node dart0Xml = doc.append_child("ArrayElement");
    dart0Xml.append_child("Name").append_child(node_pcdata).set_value("dbl");
    dart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Expression *dar0 = createExpression(dart0Xml, nc, wasCreated);
    assertTrue_1(dar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
    assertTrue_1(dar0->valueType() == REAL_TYPE);
    assertTrue_1(!dar0->isKnown());
  
    xml_node dartiXml = doc.append_child("ArrayElement");
    dartiXml.append_child("Name").append_child(node_pcdata).set_value("dbl");
    dartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Expression *dari = createExpression(dartiXml, nc, wasCreated);
    assertTrue_1(dari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
    assertTrue_1(dari->valueType() == REAL_TYPE);
    assertTrue_1(!dari->isKnown());

    dav->activate();
    assertTrue_1(!dar0->isKnown());
    assertTrue_1(!dari->isKnown());

    double pd;
    dar0->activate();
    assertTrue_1(dar0->getValue(pd));
    assertTrue_1(pd == 0);

    dari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vd.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(dari->getValue(pd));
      assertTrue_1(pd == vd[i]);
    }
    dari->deactivate();
    assertTrue_1(!iv->isActive());

    delete dar0;
    delete dari;
  }

  // String array
  {
    std::vector<std::string> vs(4);
    vs[0] = std::string("zero");
    vs[1] = std::string("one");
    vs[2] = std::string("two");
    vs[3] = std::string("three");
    StringArrayConstant *sc = new StringArrayConstant(vs);
    StringArrayVariable *sav = new StringArrayVariable(nc, "str", NULL, false);
    sav->setInitializer(sc, true);
    Alias *asav = new Alias(nc, "str", sav, true);
    realNc->storeVariable("str", asav);

    xml_node sart0Xml = doc.append_child("ArrayElement");
    sart0Xml.append_child("Name").append_child(node_pcdata).set_value("str");
    sart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Expression *sar0 = createExpression(sart0Xml, nc, wasCreated);
    assertTrue_1(sar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
    assertTrue_1(sar0->valueType() == STRING_TYPE);
    assertTrue_1(!sar0->isKnown());
  
    xml_node sartiXml = doc.append_child("ArrayElement");
    sartiXml.append_child("Name").append_child(node_pcdata).set_value("str");
    sartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Expression *sari = createExpression(sartiXml, nc, wasCreated);
    assertTrue_1(sari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", sari->exprName()));
    assertTrue_1(sari->valueType() == STRING_TYPE);
    assertTrue_1(!sari->isKnown());

    sav->activate();
    assertTrue_1(!sar0->isKnown());
    assertTrue_1(!sari->isKnown());

    std::string ps;
    sar0->activate();
    assertTrue_1(sar0->getValue(ps));
    assertTrue_1(ps == "zero");

    sari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vs.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(sari->getValue(ps));
      assertTrue_1(ps == vs[i]);
    }

    delete sar0;
    delete sari;
  }

  return true;
}

static bool testMutableArrayReferenceXmlParser()
{
  IntegerVariable *iv = new IntegerVariable;
  realNc->storeVariable("i", iv);
  xml_document doc;
  int32_t n;
  bool wasCreated = false;
  
  // Boolean array
  {
    std::vector<bool> vb(2);
    vb[0] = false;
    vb[1] = true;
    BooleanArrayConstant *bc = new BooleanArrayConstant(vb);
    BooleanArrayVariable *bav = new BooleanArrayVariable(nc, "bul", NULL, false);
    bav->setInitializer(bc, true);
    realNc->storeVariable("bul", bav);
    xml_node bart0Xml = doc.append_child("ArrayElement");
    bart0Xml.append_child("Name").append_child(node_pcdata).set_value("bul");
    bart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Assignable *bar0 = createAssignable(bart0Xml, nc, wasCreated);
    assertTrue_1(bar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
    assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!bar0->isKnown());

    xml_node bartiXml = doc.append_child("ArrayElement");
    bartiXml.append_child("Name").append_child(node_pcdata).set_value("bul");
    bartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Assignable *bari = createAssignable(bartiXml, nc, wasCreated);
    assertTrue_1(bari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
    assertTrue_1(bari->valueType() == BOOLEAN_TYPE);
    assertTrue_1(!bari->isKnown());

    bool        pb;
    bar0->activate();
    assertTrue_1(bar0->getValue(pb));
    assertTrue_1(!pb);

    bari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vb.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(bari->getValue(pb));
      assertTrue_1(pb == vb[i]);
    }
    bari->deactivate();
    assertTrue_1(!iv->isActive());

    bari->activate();
    for (int32_t i = 0; i < vb.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(bari->getValue(pb));
      assertTrue_1(pb == vb[i]);
      bari->setValue(!pb);
      assertTrue_1(bari->getValue(pb));
      assertTrue_1(pb == !vb[i]);
    }
    bari->deactivate();
  
    delete bar0;
    delete bari;
  }

  // Integer array
  {
    std::vector<int32_t>     vi(4);
    vi[0] = 0;
    vi[1] = 1;
    vi[2] = 2;
    vi[3] = 3;
    IntegerArrayConstant *ic = new IntegerArrayConstant(vi);
    IntegerArrayVariable *iav = new IntegerArrayVariable(nc, "int", NULL, false);
    iav->setInitializer(ic, true);
    realNc->storeVariable("int", iav);

    xml_node iart0Xml = doc.append_child("ArrayElement");
    iart0Xml.append_child("Name").append_child(node_pcdata).set_value("int");
    iart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Assignable *iar0 = createAssignable(iart0Xml, nc, wasCreated);
    assertTrue_1(iar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
    assertTrue_1(iar0->valueType() == INTEGER_TYPE);
    assertTrue_1(!iar0->isKnown());
  
    xml_node iartiXml = doc.append_child("ArrayElement");
    iartiXml.append_child("Name").append_child(node_pcdata).set_value("int");
    iartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Assignable *iari = createAssignable(iartiXml, nc, wasCreated);
    assertTrue_1(iari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
    assertTrue_1(iari->valueType() == INTEGER_TYPE);
    assertTrue_1(!iari->isKnown());

    int32_t pi;
    iar0->activate();
    assertTrue_1(iar0->getValue(pi));
    assertTrue_1(pi == 0);

    iari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vi.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(iari->getValue(pi));
      assertTrue_1(pi == vi[i]);
    }
    iari->deactivate();
    assertTrue_1(!iv->isActive());

    iari->activate();
    for (int32_t i = 0; i < vi.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(iari->getValue(pi));
      assertTrue_1(pi == vi[i]);
      iari->setValue(-pi);
      pi = 0;
      assertTrue_1(iari->getValue(pi));
      assertTrue_1(pi == -vi[i]);
    }
    iari->deactivate();

    delete iar0;
    delete iari;
  }

  // Real array
  {
    std::vector<double> vd(4);
    vd[0] = 0;
    vd[1] = 1;
    vd[2] = 2;
    vd[3] = 3;
    RealArrayConstant *dc = new RealArrayConstant(vd);
    RealArrayVariable *dav = new RealArrayVariable(nc, "dbl", NULL, false);
    dav->setInitializer(dc, true);
    realNc->storeVariable("dbl", dav);

    xml_node dart0Xml = doc.append_child("ArrayElement");
    dart0Xml.append_child("Name").append_child(node_pcdata).set_value("dbl");
    dart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Assignable *dar0 = createAssignable(dart0Xml, nc, wasCreated);
    assertTrue_1(dar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
    assertTrue_1(dar0->valueType() == REAL_TYPE);
    assertTrue_1(!dar0->isKnown());

    xml_node dartiXml = doc.append_child("ArrayElement");
    dartiXml.append_child("Name").append_child(node_pcdata).set_value("dbl");
    dartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Assignable *dari = createAssignable(dartiXml, nc, wasCreated);
    assertTrue_1(dari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
    assertTrue_1(dari->valueType() == REAL_TYPE);
    assertTrue_1(!dari->isKnown());

    double pd;
    dar0->activate();
    assertTrue_1(dar0->getValue(pd));
    assertTrue_1(pd == 0);

    dari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vd.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(dari->getValue(pd));
      assertTrue_1(pd == vd[i]);
    }
    dari->deactivate();
    assertTrue_1(!iv->isActive());

    dari->activate();
    for (int32_t i = 0; i < vd.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(dari->getValue(pd));
      assertTrue_1(pd == vd[i]);
      dari->setValue(-pd);
      pd = 0;
      assertTrue_1(dari->getValue(pd));
      assertTrue_1(pd == -vd[i]);
    }
    dari->deactivate();

    delete dar0;
    delete dari;
  }

  // String array
  {
    std::vector<std::string> vs(4);
    vs[0] = std::string("zero");
    vs[1] = std::string("one");
    vs[2] = std::string("two");
    vs[3] = std::string("three");
    StringArrayConstant *sc = new StringArrayConstant(vs);
    StringArrayVariable *sav = new StringArrayVariable(nc, "str", NULL, false);
    sav->setInitializer(sc, true);
    realNc->storeVariable("str", sav);

    xml_node sart0Xml = doc.append_child("ArrayElement");
    sart0Xml.append_child("Name").append_child(node_pcdata).set_value("str");
    sart0Xml.append_child("Index").append_child("IntegerValue").append_child(node_pcdata).set_value("0");
    Assignable *sar0 = createAssignable(sart0Xml, nc, wasCreated);
    assertTrue_1(sar0);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
    assertTrue_1(sar0->valueType() == STRING_TYPE);
    assertTrue_1(!sar0->isKnown());
  
    xml_node sartiXml = doc.append_child("ArrayElement");
    sartiXml.append_child("Name").append_child(node_pcdata).set_value("str");
    sartiXml.append_child("Index").append_child("IntegerVariable").append_child(node_pcdata).set_value("i");
    Assignable *sari = createAssignable(sartiXml, nc, wasCreated);
    assertTrue_1(sari);
    assertTrue_1(wasCreated);
    assertTrue_1(!strcmp("ArrayReference", sari->exprName()));
    assertTrue_1(sari->valueType() == STRING_TYPE);
    assertTrue_1(!sari->isKnown());

    std::string ps;
    sar0->activate();
    assertTrue_1(sar0->getValue(ps));
    assertTrue_1(ps == "zero");

    sari->activate();
    assertTrue_1(iv->isActive());
    for (int32_t i = 0; i < vs.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
      assertTrue_1(n == i);
      assertTrue_1(sari->getValue(ps));
      assertTrue_1(ps == vs[i]);
    }

    sari->activate();
    for (int32_t i = 0; i < vs.size(); ++i) {
      iv->setValue(i);
      assertTrue_1(iv->getValue(n));
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
    sari->deactivate();

    delete sar0;
    delete sari;
  }

  return true;
}

bool arrayReferenceXmlParserTest()
{
  // Initialize infrastructure
  realNc = new FactoryTestNodeConnector();
  nc = realNc;

  runTest(testArrayConstantReferenceXmlParser);
  runTest(testArrayVariableReferenceXmlParser);
  runTest(testArrayAliasReferenceXmlParser);

  runTest(testMutableArrayReferenceXmlParser);

  nc = NULL;
  delete realNc;
  return true;
}
