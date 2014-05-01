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

static bool testBasics()
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

  return true;
}

bool constantsTest()
{
  runTest(testBasics);
  return true;
}
