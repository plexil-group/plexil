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
#include "PlexilExpr.hh"
#include "TestSupport.hh"
#include "UserVariable.hh"

#include <cstdlib> // for strcmp()

using namespace PLEXIL;

// Global variables for convenience
static NodeConnectorId nc;
static FactoryTestNodeConnector *realNc = NULL;

static bool testArrayConstantReferenceFactory()
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
  realNc->storeVariable("bul", bc.getId());
  realNc->storeVariable("int", ic.getId());
  realNc->storeVariable("dbl", dc.getId());
  realNc->storeVariable("str", sc.getId());

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", iv.getId());

  // Construct reference templates
  PlexilArrayElement bart0((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement barti((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement iart0((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement iarti((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement dart0((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement darti((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement sart0((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement sarti((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());

  bool wasCreated = false;

  ExpressionId bar0 = createExpression(bart0.getId(), nc, wasCreated);
  assertTrue_1(bar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);

  ExpressionId bari = createExpression(barti.getId(), nc, wasCreated);
  assertTrue_1(bari.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  ExpressionId iar0 = createExpression(iart0.getId(), nc, wasCreated);
  assertTrue_1(iar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);

  ExpressionId iari = createExpression(iarti.getId(), nc, wasCreated);
  assertTrue_1(iari.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  ExpressionId dar0 = createExpression(dart0.getId(), nc, wasCreated);
  assertTrue_1(dar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);

  ExpressionId dari = createExpression(darti.getId(), nc, wasCreated);
  assertTrue_1(dari.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  ExpressionId sar0 = createExpression(sart0.getId(), nc, wasCreated);
  assertTrue_1(sar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);

  ExpressionId sari = createExpression(sarti.getId(), nc, wasCreated);
  assertTrue_1(sari.isId());
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

  delete (Expression *) bar0;
  delete (Expression *) bari;
  delete (Expression *) iar0;
  delete (Expression *) iari;
  delete (Expression *) dar0;
  delete (Expression *) dari;
  delete (Expression *) sar0;
  delete (Expression *) sari;

  return true;
}

static bool testArrayVariableReferenceFactory()
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
  
  BooleanArrayVariable bav(nc, "bul", ExpressionId::noId(), bc.getId(), false, false);
  IntegerArrayVariable iav(nc, "int", ExpressionId::noId(), ic.getId(), false, false);
  RealArrayVariable    dav(nc, "dbl", ExpressionId::noId(), dc.getId(), false, false);
  StringArrayVariable  sav(nc, "str", ExpressionId::noId(), sc.getId(), false, false);

  // Associate arrays with names
  realNc->storeVariable("bul", bav.getId());
  realNc->storeVariable("int", iav.getId());
  realNc->storeVariable("dbl", dav.getId());
  realNc->storeVariable("str", sav.getId());

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", iv.getId());

  // Construct reference templates
  PlexilArrayElement bart0((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement barti((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement iart0((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement iarti((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement dart0((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement darti((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement sart0((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement sarti((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());

  bool wasCreated = false;

  ExpressionId bar0 = createExpression(bart0.getId(), nc, wasCreated);
  assertTrue_1(bar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);

  ExpressionId bari = createExpression(barti.getId(), nc, wasCreated);
  assertTrue_1(bari.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  ExpressionId iar0 = createExpression(iart0.getId(), nc, wasCreated);
  assertTrue_1(iar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);

  ExpressionId iari = createExpression(iarti.getId(), nc, wasCreated);
  assertTrue_1(iari.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  ExpressionId dar0 = createExpression(dart0.getId(), nc, wasCreated);
  assertTrue_1(dar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);

  ExpressionId dari = createExpression(darti.getId(), nc, wasCreated);
  assertTrue_1(dari.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  ExpressionId sar0 = createExpression(sart0.getId(), nc, wasCreated);
  assertTrue_1(sar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);

  ExpressionId sari = createExpression(sarti.getId(), nc, wasCreated);
  assertTrue_1(sari.isId());
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

  delete (Expression *) bar0;
  delete (Expression *) bari;
  delete (Expression *) iar0;
  delete (Expression *) iari;
  delete (Expression *) dar0;
  delete (Expression *) dari;
  delete (Expression *) sar0;
  delete (Expression *) sari;

  return true;
}

static bool testArrayAliasReferenceFactory()
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
  
  BooleanArrayVariable bav(nc, "rbul", ExpressionId::noId(), bc.getId(), false, false);
  IntegerArrayVariable iav(nc, "rint", ExpressionId::noId(), ic.getId(), false, false);
  RealArrayVariable    dav(nc, "rdbl", ExpressionId::noId(), dc.getId(), false, false);
  StringArrayVariable  sav(nc, "rstr", ExpressionId::noId(), sc.getId(), false, false);

  Alias abav(nc, "bul", bav.getId(), false);
  Alias aiav(nc, "int", iav.getId(), false);
  Alias adav(nc, "dbl", dav.getId(), false);
  Alias asav(nc, "str", sav.getId(), false);

  // Associate aliases with names
  realNc->storeVariable("bul", abav.getId());
  realNc->storeVariable("int", aiav.getId());
  realNc->storeVariable("dbl", adav.getId());
  realNc->storeVariable("str", asav.getId());

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", iv.getId());

  // Construct reference templates
  PlexilArrayElement bart0((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement barti((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement iart0((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement iarti((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement dart0((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement darti((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement sart0((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement sarti((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());

  bool wasCreated = false;

  ExpressionId bar0 = createExpression(bart0.getId(), nc, wasCreated);
  assertTrue_1(bar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);

  ExpressionId bari = createExpression(barti.getId(), nc, wasCreated);
  assertTrue_1(bari.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  ExpressionId iar0 = createExpression(iart0.getId(), nc, wasCreated);
  assertTrue_1(iar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);

  ExpressionId iari = createExpression(iarti.getId(), nc, wasCreated);
  assertTrue_1(iari.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  ExpressionId dar0 = createExpression(dart0.getId(), nc, wasCreated);
  assertTrue_1(dar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);

  ExpressionId dari = createExpression(darti.getId(), nc, wasCreated);
  assertTrue_1(dari.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  ExpressionId sar0 = createExpression(sart0.getId(), nc, wasCreated);
  assertTrue_1(sar0.isId());
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);

  ExpressionId sari = createExpression(sarti.getId(), nc, wasCreated);
  assertTrue_1(sari.isId());
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

  delete (Expression *) bar0;
  delete (Expression *) bari;
  delete (Expression *) iar0;
  delete (Expression *) iari;
  delete (Expression *) dar0;
  delete (Expression *) dari;
  delete (Expression *) sar0;
  delete (Expression *) sari;

  return true;
}

static bool testMutableArrayReferenceFactory()
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
  
  BooleanArrayVariable bav(nc, "bul", ExpressionId::noId(), bc.getId(), false, false);
  IntegerArrayVariable iav(nc, "int", ExpressionId::noId(), ic.getId(), false, false);
  RealArrayVariable    dav(nc, "dbl", ExpressionId::noId(), dc.getId(), false, false);
  StringArrayVariable  sav(nc, "str", ExpressionId::noId(), sc.getId(), false, false);

  // Associate arrays with names
  realNc->storeVariable("bul", bav.getId());
  realNc->storeVariable("int", iav.getId());
  realNc->storeVariable("dbl", dav.getId());
  realNc->storeVariable("str", sav.getId());

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", iv.getId());

  // Construct reference templates
  PlexilArrayElement bart0((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement barti((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement iart0((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement iarti((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement dart0((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement darti((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement sart0((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement sarti((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());

  bool wasCreated = false;

  Assignable *bar0 = createAssignable(bart0.getId(), nc, wasCreated);
  assertTrue_1(bar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);

  Assignable *bari = createAssignable(barti.getId(), nc, wasCreated);
  assertTrue_1(bari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  Assignable *iar0 = createAssignable(iart0.getId(), nc, wasCreated);
  assertTrue_1(iar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);

  Assignable *iari = createAssignable(iarti.getId(), nc, wasCreated);
  assertTrue_1(iari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  Assignable *dar0 = createAssignable(dart0.getId(), nc, wasCreated);
  assertTrue_1(dar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);

  Assignable *dari = createAssignable(darti.getId(), nc, wasCreated);
  assertTrue_1(dari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  Assignable *sar0 = createAssignable(sart0.getId(), nc, wasCreated);
  assertTrue_1(sar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);

  Assignable *sari = createAssignable(sarti.getId(), nc, wasCreated);
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

  delete (Expression *) bar0;
  delete (Expression *) bari;
  delete (Expression *) iar0;
  delete (Expression *) iari;
  delete (Expression *) dar0;
  delete (Expression *) dari;
  delete (Expression *) sar0;
  delete (Expression *) sari;

  return true;
}

static bool testMutableArrayAliasReferenceFactory()
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
  
  BooleanArrayVariable bav(nc, "rbul", ExpressionId::noId(), bc.getId(), false, false);
  IntegerArrayVariable iav(nc, "rint", ExpressionId::noId(), ic.getId(), false, false);
  RealArrayVariable    dav(nc, "rdbl", ExpressionId::noId(), dc.getId(), false, false);
  StringArrayVariable  sav(nc, "rstr", ExpressionId::noId(), sc.getId(), false, false);

  InOutAlias abav(nc, "bul", bav.getId(), false);
  InOutAlias aiav(nc, "int", iav.getId(), false);
  InOutAlias adav(nc, "dbl", dav.getId(), false);
  InOutAlias asav(nc, "str", sav.getId(), false);

  // Associate aliases with names
  realNc->storeVariable("bul", abav.getId());
  realNc->storeVariable("int", aiav.getId());
  realNc->storeVariable("dbl", adav.getId());
  realNc->storeVariable("str", asav.getId());

  // Store array index too
  IntegerVariable iv;
  realNc->storeVariable("i", iv.getId());

  // Construct reference templates
  PlexilArrayElement bart0((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement barti((new PlexilVarRef("bul", BOOLEAN_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement iart0((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement iarti((new PlexilVarRef("int", INTEGER_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement dart0((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement darti((new PlexilVarRef("dbl", REAL_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());
  PlexilArrayElement sart0((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilValue(INTEGER_TYPE, "0"))->getId());
  PlexilArrayElement sarti((new PlexilVarRef("str", STRING_ARRAY_TYPE))->getId(),
                           (new PlexilVarRef("i", INTEGER_TYPE))->getId());

  bool wasCreated = false;

  Assignable *bar0 = createAssignable(bart0.getId(), nc, wasCreated);
  assertTrue_1(bar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bar0->exprName()));
  assertTrue_1(bar0->valueType() == BOOLEAN_TYPE);

  Assignable *bari = createAssignable(barti.getId(), nc, wasCreated);
  assertTrue_1(bari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", bari->exprName()));
  assertTrue_1(bari->valueType() == BOOLEAN_TYPE);

  Assignable *iar0 = createAssignable(iart0.getId(), nc, wasCreated);
  assertTrue_1(iar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iar0->exprName()));
  assertTrue_1(iar0->valueType() == INTEGER_TYPE);

  Assignable *iari = createAssignable(iarti.getId(), nc, wasCreated);
  assertTrue_1(iari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", iari->exprName()));
  assertTrue_1(iari->valueType() == INTEGER_TYPE);

  Assignable *dar0 = createAssignable(dart0.getId(), nc, wasCreated);
  assertTrue_1(dar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dar0->exprName()));
  assertTrue_1(dar0->valueType() == REAL_TYPE);

  Assignable *dari = createAssignable(darti.getId(), nc, wasCreated);
  assertTrue_1(dari);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", dari->exprName()));
  assertTrue_1(dari->valueType() == REAL_TYPE);

  Assignable *sar0 = createAssignable(sart0.getId(), nc, wasCreated);
  assertTrue_1(sar0);
  assertTrue_1(wasCreated);
  assertTrue_1(!strcmp("ArrayReference", sar0->exprName()));
  assertTrue_1(sar0->valueType() == STRING_TYPE);

  Assignable *sari = createAssignable(sarti.getId(), nc, wasCreated);
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

  delete (Expression *) bar0;
  delete (Expression *) bari;
  delete (Expression *) iar0;
  delete (Expression *) iari;
  delete (Expression *) dar0;
  delete (Expression *) dari;
  delete (Expression *) sar0;
  delete (Expression *) sari;

  return true;
}

bool arrayReferenceFactoryTest()
{
  // Initialize factories
  registerBasicExpressionFactories();
  // Initialize infrastructure
  realNc = new FactoryTestNodeConnector();
  nc = realNc->getId();

  runTest(testArrayConstantReferenceFactory);
  runTest(testArrayVariableReferenceFactory);
  runTest(testArrayAliasReferenceFactory);

  runTest(testMutableArrayReferenceFactory);
  runTest(testMutableArrayAliasReferenceFactory);

  nc = NodeConnectorId::noId();
  delete realNc;
  return true;
}
