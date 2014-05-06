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

#include "Constant.hh"
#include "TestSupport.hh"

using namespace PLEXIL;

//
// Constants
//

static bool testScalars()
{
  // Default constructor check
  BooleanConstant ub;
  IntegerConstant ui;
  RealConstant ud;
  StringConstant us;

  // Direct access
  assertTrue_1(ub.isConstant());
  assertTrue_1(ui.isConstant());
  assertTrue_1(ud.isConstant());
  assertTrue_1(us.isConstant());
  assertTrue_1(!ub.isAssignable());
  assertTrue_1(!ui.isAssignable());
  assertTrue_1(!ud.isAssignable());
  assertTrue_1(!us.isAssignable());
  assertTrue_1(!ub.isKnown());
  assertTrue_1(!ui.isKnown());
  assertTrue_1(!ud.isKnown());
  assertTrue_1(!us.isKnown());

  // Access through ExpressionId
  ExpressionId iub = ub.getId();
  ExpressionId iui = ui.getId();
  ExpressionId iud = ud.getId();
  ExpressionId ius = us.getId();
  assertTrue_1(iub->isConstant());
  assertTrue_1(iui->isConstant());
  assertTrue_1(iud->isConstant());
  assertTrue_1(ius->isConstant());
  assertTrue_1(!iub->isAssignable());
  assertTrue_1(!iui->isAssignable());
  assertTrue_1(!iud->isAssignable());
  assertTrue_1(!ius->isAssignable());
  assertTrue_1(!iub->isKnown());
  assertTrue_1(!iui->isKnown());
  assertTrue_1(!iud->isKnown());
  assertTrue_1(!ius->isKnown());

  // getValue on unknowns
  double food;
  std::string foos;
  int32_t fooi;
  bool foob;
  assertTrue_1(!ub.getValue(foob));
  assertTrue_1(!ui.getValue(fooi));
  assertTrue_1(!ud.getValue(food));
  assertTrue_1(!us.getValue(foos));
  assertTrue_1(!iub->getValue(foob));
  assertTrue_1(!iui->getValue(fooi));
  assertTrue_1(!iud->getValue(food));
  assertTrue_1(!ius->getValue(foos));

  // getValuePointer on unknowns
  std::string const *pfoos;
  assertTrue_1(!us.getValuePointer(pfoos));
  assertTrue_1(!ius->getValuePointer(pfoos));

  // Constants with values
  BooleanConstant troo = true;
  IntegerConstant too = 2;
  RealConstant doo = 2.718;
  StringConstant soo = std::string("Sue");

  assertTrue_1(troo.isConstant());
  assertTrue_1(too.isConstant());
  assertTrue_1(doo.isConstant());
  assertTrue_1(soo.isConstant());
  assertTrue_1(troo.isKnown());
  assertTrue_1(too.isKnown());
  assertTrue_1(doo.isKnown());
  assertTrue_1(soo.isKnown());
    
  // getValue() test
  assertTrue_1(troo.getValue(foob));
  assertTrue_1(foob == true);
  assertTrue_1(too.getValue(fooi));
  assertTrue_1(fooi == 2);
  assertTrue_1(doo.getValue(food));
  assertTrue_1(food == 2.718);
  assertTrue_1(soo.getValue(foos));
  assertTrue_1(foos == std::string("Sue"));
  // Numeric conversion
  assertTrue_1(too.getValue(food));
  assertTrue_1(food == 2);
    
  // getValue() through ExpressionId test
  assertTrue_1((troo.getId())->getValue(foob));
  assertTrue_1(foob == true);
  assertTrue_1((too.getId())->getValue(fooi));
  assertTrue_1(fooi == 2);
  assertTrue_1((doo.getId())->getValue(food));
  assertTrue_1(food == 2.718);
  assertTrue_1((soo.getId())->getValue(foos));
  assertTrue_1(foos == std::string("Sue"));
  // Numeric conversion
  assertTrue_1((too.getId())->getValue(food));
  assertTrue_1(food == 2);

  // getValuePointer() tests
  assertTrue_1(soo.getValuePointer(pfoos));
  assertTrue_1(*pfoos == std::string("Sue"));
  assertTrue_1((soo.getId())->getValuePointer(pfoos));
  assertTrue_1(*pfoos == std::string("Sue"));

  return true;
}

bool testArrays()
{
  // Default constructor check
  BooleanArrayConstant ub;
  IntegerArrayConstant ui;
  RealArrayConstant ud;
  StringArrayConstant us;

  // Direct access
  assertTrue_1(ub.isConstant());
  assertTrue_1(ui.isConstant());
  assertTrue_1(ud.isConstant());
  assertTrue_1(us.isConstant());
  assertTrue_1(!ub.isAssignable());
  assertTrue_1(!ui.isAssignable());
  assertTrue_1(!ud.isAssignable());
  assertTrue_1(!us.isAssignable());
  assertTrue_1(!ub.isKnown());
  assertTrue_1(!ui.isKnown());
  assertTrue_1(!ud.isKnown());
  assertTrue_1(!us.isKnown());

  // Access through ExpressionId
  ExpressionId iub = ub.getId();
  ExpressionId iui = ui.getId();
  ExpressionId iud = ud.getId();
  ExpressionId ius = us.getId();
  assertTrue_1(iub->isConstant());
  assertTrue_1(iui->isConstant());
  assertTrue_1(iud->isConstant());
  assertTrue_1(ius->isConstant());
  assertTrue_1(!iub->isAssignable());
  assertTrue_1(!iui->isAssignable());
  assertTrue_1(!iud->isAssignable());
  assertTrue_1(!ius->isAssignable());
  assertTrue_1(!iub->isKnown());
  assertTrue_1(!iui->isKnown());
  assertTrue_1(!iud->isKnown());
  assertTrue_1(!ius->isKnown());

  // getValuePointer test
  std::vector<bool> const *pbool = NULL;
  std::vector<int32_t> const *pint = NULL;
  std::vector<double> const *preal = NULL;
  std::vector<std::string> const *pstr = NULL;

  assertTrue_1(!ub.getValuePointer(pbool));
  assertTrue_1(!ui.getValuePointer(pint));
  assertTrue_1(!ud.getValuePointer(preal));
  assertTrue_1(!us.getValuePointer(pstr));
  assertTrue_1(pbool == NULL);
  assertTrue_1(pint == NULL);
  assertTrue_1(preal == NULL);
  assertTrue_1(pstr == NULL);
  assertTrue_1(!iub->getValuePointer(pbool));
  assertTrue_1(!iui->getValuePointer(pint));
  assertTrue_1(!iud->getValuePointer(preal));
  assertTrue_1(!ius->getValuePointer(pstr));
  assertTrue_1(pbool == NULL);
  assertTrue_1(pint == NULL);
  assertTrue_1(preal == NULL);
  assertTrue_1(pstr == NULL);
  
  // Constants with values
  std::vector<bool> oob;
  oob.push_back(true);
  oob.push_back(false);
  BooleanArrayConstant troo = oob;

  std::vector<int32_t> ooi;
  ooi.push_back(42);
  ooi.push_back(69);
  ooi.push_back(14);
  IntegerArrayConstant too = ooi;

  std::vector<double> ood;
  ood.push_back(1.414);
  ood.push_back(2.718);
  ood.push_back(3.1416);
  ood.push_back(3.162);
  RealArrayConstant doo = ood;

  std::vector<std::string> oos;
  oos.push_back(std::string("foo"));
  oos.push_back(std::string("bar"));
  oos.push_back(std::string("baz"));
  oos.push_back(std::string("bletch"));
  oos.push_back(std::string("blorf"));
  StringArrayConstant soo = oos;

  ExpressionId ptroo = troo.getId();
  ExpressionId ptoo = too.getId();
  ExpressionId pdoo = doo.getId();
  ExpressionId psoo = soo.getId();

  assertTrue_1(troo.isConstant());
  assertTrue_1(too.isConstant());
  assertTrue_1(doo.isConstant());
  assertTrue_1(soo.isConstant());
  assertTrue_1(ptroo->isConstant());
  assertTrue_1(ptoo->isConstant());
  assertTrue_1(pdoo->isConstant());
  assertTrue_1(psoo->isConstant());

  assertTrue_1(!troo.isAssignable());
  assertTrue_1(!too.isAssignable());
  assertTrue_1(!doo.isAssignable());
  assertTrue_1(!soo.isAssignable());
  assertTrue_1(!ptroo->isAssignable());
  assertTrue_1(!ptoo->isAssignable());
  assertTrue_1(!pdoo->isAssignable());
  assertTrue_1(!psoo->isAssignable());

  assertTrue_1(troo.isKnown());
  assertTrue_1(too.isKnown());
  assertTrue_1(doo.isKnown());
  assertTrue_1(soo.isKnown());
  assertTrue_1(ptroo->isKnown());
  assertTrue_1(ptoo->isKnown());
  assertTrue_1(pdoo->isKnown());
  assertTrue_1(psoo->isKnown());

  // Access via getValuePointer
  assertTrue_1(troo.getValuePointer(pbool));
  assertTrue_1(pbool != NULL);
  assertTrue_1(oob == *pbool);
  assertTrue_1(too.getValuePointer(pint));
  assertTrue_1(pint != NULL);
  assertTrue_1(ooi == *pint);
  assertTrue_1(doo.getValuePointer(preal));
  assertTrue_1(preal != NULL);
  assertTrue_1(ood == *preal);
  assertTrue_1(soo.getValuePointer(pstr));
  assertTrue_1(pstr != NULL);
  assertTrue_1(oos == *pstr);

  pbool = NULL;
  pint = NULL;
  preal = NULL;
  pstr = NULL;

  assertTrue_1(ptroo->getValuePointer(pbool));
  assertTrue_1(pbool != NULL);
  assertTrue_1(oob == *pbool);
  assertTrue_1(ptoo->getValuePointer(pint));
  assertTrue_1(pint != NULL);
  assertTrue_1(ooi == *pint);
  assertTrue_1(pdoo->getValuePointer(preal));
  assertTrue_1(preal != NULL);
  assertTrue_1(ood == *preal);
  assertTrue_1(psoo->getValuePointer(pstr));
  assertTrue_1(pstr != NULL);
  assertTrue_1(oos == *pstr);

  return true;
}


bool constantsTest()
{
  runTest(testScalars);
  runTest(testArrays)
  return true;
}

