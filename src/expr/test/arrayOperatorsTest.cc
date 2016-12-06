/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#include "ArrayOperators.hh"
#include "ArrayReference.hh"
#include "ArrayVariable.hh"
#include "Constant.hh"
#include "Error.hh"
#include "Function.hh"
#include "TestSupport.hh"
#include "UserVariable.hh"

using namespace PLEXIL;

static bool testArraySize()
{
  BooleanArrayVariable bav;
  IntegerArrayVariable iav;
  RealArrayVariable rav;
  StringArrayVariable sav;

  ArraySize lop;

  Function *bl = makeFunction(&lop, &bav, false);
  Function *il = makeFunction(&lop, &iav, false);
  Function *rl = makeFunction(&lop, &rav, false);
  Function *sl = makeFunction(&lop, &sav, false);

  int32_t len;

  // test inactive
  assertTrue_1(!bl->getValue(len));
  assertTrue_1(!il->getValue(len));
  assertTrue_1(!rl->getValue(len));
  assertTrue_1(!sl->getValue(len));

  bl->activate();
  il->activate();
  rl->activate();
  sl->activate();

  // test uninitialized
  assertTrue_1(!bl->getValue(len));
  assertTrue_1(!il->getValue(len));
  assertTrue_1(!rl->getValue(len));
  assertTrue_1(!sl->getValue(len));

  // Assign empty arrays
  BooleanArrayConstant emptybac(0);
  IntegerArrayConstant emptyiac(0);
  RealArrayConstant emptyrac(0);
  StringArrayConstant emptysac(0);

  bav.setValue(emptybac);
  iav.setValue(emptyiac);
  rav.setValue(emptyrac);
  sav.setValue(emptysac);

  assertTrue_1(bl->getValue(len));
  assertTrue_1(len == 0);
  assertTrue_1(il->getValue(len));
  assertTrue_1(len == 0);
  assertTrue_1(rl->getValue(len));
  assertTrue_1(len == 0);
  assertTrue_1(sl->getValue(len));
  assertTrue_1(len == 0);

  // Assign short (but uninited) arrays
  BooleanArrayConstant shortbac(8);
  IntegerArrayConstant shortiac(8);
  RealArrayConstant shortrac(8);
  StringArrayConstant shortsac(8);

  bav.setValue(shortbac);
  iav.setValue(shortiac);
  rav.setValue(shortrac);
  sav.setValue(shortsac);

  assertTrue_1(bl->getValue(len));
  assertTrue_1(len == 8);
  assertTrue_1(il->getValue(len));
  assertTrue_1(len == 8);
  assertTrue_1(rl->getValue(len));
  assertTrue_1(len == 8);
  assertTrue_1(sl->getValue(len));
  assertTrue_1(len == 8);

  // Deactivate and try again
  bl->deactivate();
  il->deactivate();
  rl->deactivate();
  sl->deactivate();

  assertTrue_1(!bl->getValue(len));
  assertTrue_1(!il->getValue(len));
  assertTrue_1(!rl->getValue(len));
  assertTrue_1(!sl->getValue(len));

  // Reactivate
  bl->activate();
  il->activate();
  rl->activate();
  sl->activate();

  assertTrue_1(bl->getValue(len));
  assertTrue_1(len == 8);
  assertTrue_1(il->getValue(len));
  assertTrue_1(len == 8);
  assertTrue_1(rl->getValue(len));
  assertTrue_1(len == 8);
  assertTrue_1(sl->getValue(len));
  assertTrue_1(len == 8);

  // Assign unknown arrays
  BooleanArrayConstant unknownbac;
  IntegerArrayConstant unknowniac;
  RealArrayConstant unknownrac;
  StringArrayConstant unknownsac;

  bav.setValue(unknownbac);
  iav.setValue(unknowniac);
  rav.setValue(unknownrac);
  sav.setValue(unknownsac);

  assertTrue_1(!bl->getValue(len));
  assertTrue_1(!il->getValue(len));
  assertTrue_1(!rl->getValue(len));
  assertTrue_1(!sl->getValue(len));

  delete sl;
  delete rl;
  delete il;
  delete bl;

  return true;
}

static bool testAllElementsKnown()
{
  BooleanArrayVariable bav;
  IntegerArrayVariable iav;
  RealArrayVariable rav;
  StringArrayVariable sav;

  AllElementsKnown op;
  
  Function *bl = makeFunction(&op, &bav, false);
  Function *il = makeFunction(&op, &iav, false);
  Function *rl = makeFunction(&op, &rav, false);
  Function *sl = makeFunction(&op, &sav, false);

  bool temp;

  // test inactive
  assertTrue_1(!bl->getValue(temp));
  assertTrue_1(!il->getValue(temp));
  assertTrue_1(!rl->getValue(temp));
  assertTrue_1(!sl->getValue(temp));

  bl->activate();
  il->activate();
  rl->activate();
  sl->activate();

  // test uninitialized
  assertTrue_1(!bl->getValue(temp));
  assertTrue_1(!il->getValue(temp));
  assertTrue_1(!rl->getValue(temp));
  assertTrue_1(!sl->getValue(temp));

  // Assign empty arrays
  BooleanArrayConstant emptybac(0);
  IntegerArrayConstant emptyiac(0);
  RealArrayConstant emptyrac(0);
  StringArrayConstant emptysac(0);

  bav.setValue(emptybac);
  iav.setValue(emptyiac);
  rav.setValue(emptyrac);
  sav.setValue(emptysac);

  // *** Boundary case -- see Array.cc ***
  assertTrue_1(bl->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(il->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(rl->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(sl->getValue(temp));
  assertTrue_1(temp);

  // Assign short (but uninited) arrays
  BooleanArrayConstant shortbac(2);
  IntegerArrayConstant shortiac(2);
  RealArrayConstant shortrac(2);
  StringArrayConstant shortsac(2);

  bav.setValue(shortbac);
  iav.setValue(shortiac);
  rav.setValue(shortrac);
  sav.setValue(shortsac);

  assertTrue_1(bl->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(il->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(rl->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(sl->getValue(temp));
  assertTrue_1(!temp);

  // Assign elements and try again
  IntegerVariable index((int32_t) 0);
  MutableArrayReference bref(&bav, &index);
  MutableArrayReference iref(&iav, &index);
  MutableArrayReference rref(&rav, &index);
  MutableArrayReference sref(&sav, &index);

  bref.activate();
  iref.activate();
  rref.activate();
  sref.activate();

  bref.setValue(false);
  iref.setValue((int32_t) 0);
  rref.setValue(0.0);
  sref.setValue(std::string(""));

  assertTrue_1(bl->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(il->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(rl->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(sl->getValue(temp));
  assertTrue_1(!temp);

  // Set other element and try again
  index.setValue((int32_t) 1);
  bref.setValue(false);
  iref.setValue((int32_t) 0);
  rref.setValue(0.0);
  sref.setValue(std::string(""));

  assertTrue_1(bl->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(il->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(rl->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(sl->getValue(temp));
  assertTrue_1(temp);

  // Assign unknown arrays
  BooleanArrayConstant unknownbac;
  IntegerArrayConstant unknowniac;
  RealArrayConstant unknownrac;
  StringArrayConstant unknownsac;

  bav.setValue(unknownbac);
  iav.setValue(unknowniac);
  rav.setValue(unknownrac);
  sav.setValue(unknownsac);

  assertTrue_1(!bl->getValue(temp));
  assertTrue_1(!il->getValue(temp));
  assertTrue_1(!rl->getValue(temp));
  assertTrue_1(!sl->getValue(temp));

  delete sl;
  delete rl;
  delete il;
  delete bl;

  return true;
}

// *** TODO: Implement tests for ArrayMaxSize ***

static bool testAnyElementsKnown()
{
  BooleanArrayVariable bav;
  IntegerArrayVariable iav;
  RealArrayVariable rav;
  StringArrayVariable sav;

  AnyElementsKnown op;

  Function *bl = makeFunction(&op, &bav, false);
  Function *il = makeFunction(&op, &iav, false);
  Function *rl = makeFunction(&op, &rav, false);
  Function *sl = makeFunction(&op, &sav, false);

  bool temp;

  // test inactive
  assertTrue_1(!bl->getValue(temp));
  assertTrue_1(!il->getValue(temp));
  assertTrue_1(!rl->getValue(temp));
  assertTrue_1(!sl->getValue(temp));

  bl->activate();
  il->activate();
  rl->activate();
  sl->activate();

  // test uninitialized
  assertTrue_1(!bl->getValue(temp));
  assertTrue_1(!il->getValue(temp));
  assertTrue_1(!rl->getValue(temp));
  assertTrue_1(!sl->getValue(temp));

  // Assign empty arrays
  BooleanArrayConstant emptybac(0);
  IntegerArrayConstant emptyiac(0);
  RealArrayConstant emptyrac(0);
  StringArrayConstant emptysac(0);

  bav.setValue(emptybac);
  iav.setValue(emptyiac);
  rav.setValue(emptyrac);
  sav.setValue(emptysac);

  // *** Boundary case -- see Array.cc ***
  assertTrue_1(bl->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(il->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(rl->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(sl->getValue(temp));
  assertTrue_1(!temp);

  // Assign short (but uninited) arrays
  BooleanArrayConstant shortbac(2);
  IntegerArrayConstant shortiac(2);
  RealArrayConstant shortrac(2);
  StringArrayConstant shortsac(2);

  bav.setValue(shortbac);
  iav.setValue(shortiac);
  rav.setValue(shortrac);
  sav.setValue(shortsac);

  assertTrue_1(bl->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(il->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(rl->getValue(temp));
  assertTrue_1(!temp);
  assertTrue_1(sl->getValue(temp));
  assertTrue_1(!temp);

  // Assign elements and try again
  IntegerVariable index((int32_t) 0);
  MutableArrayReference bref(&bav, &index);
  MutableArrayReference iref(&iav, &index);
  MutableArrayReference rref(&rav, &index);
  MutableArrayReference sref(&sav, &index);

  bref.activate();
  iref.activate();
  rref.activate();
  sref.activate();

  bref.setValue(false);
  iref.setValue((int32_t) 0);
  rref.setValue(0.0);
  sref.setValue(std::string(""));

  assertTrue_1(bl->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(il->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(rl->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(sl->getValue(temp));
  assertTrue_1(temp);

  // Set other element and try again
  index.setValue((int32_t) 1);
  bref.setValue(false);
  iref.setValue((int32_t) 0);
  rref.setValue(0.0);
  sref.setValue(std::string(""));

  assertTrue_1(bl->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(il->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(rl->getValue(temp));
  assertTrue_1(temp);
  assertTrue_1(sl->getValue(temp));
  assertTrue_1(temp);

  // Assign unknown arrays
  BooleanArrayConstant unknownbac;
  IntegerArrayConstant unknowniac;
  RealArrayConstant unknownrac;
  StringArrayConstant unknownsac;

  bav.setValue(unknownbac);
  iav.setValue(unknowniac);
  rav.setValue(unknownrac);
  sav.setValue(unknownsac);

  assertTrue_1(!bl->getValue(temp));
  assertTrue_1(!il->getValue(temp));
  assertTrue_1(!rl->getValue(temp));
  assertTrue_1(!sl->getValue(temp));

  delete sl;
  delete rl;
  delete il;
  delete bl;

  return true;
}

bool arrayOperatorsTest()
{
  runTest(testArraySize);
  runTest(testAllElementsKnown);
  runTest(testAnyElementsKnown);
  return true;
}
