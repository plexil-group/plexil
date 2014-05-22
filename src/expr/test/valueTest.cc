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
#include "TestSupport.hh"
#include "Value.hh"

using namespace PLEXIL;

static bool testConstructorsAndAccessors()
{
  Value unkv;
  assertTrue_1(!unkv.isKnown());
  assertTrue_1(UNKNOWN_TYPE == unkv.valueType());

  Value boolv(true);
  assertTrue_1(boolv.isKnown());
  assertTrue_1(BOOLEAN_TYPE == boolv.valueType());
  bool tempb;
  assertTrue_1(boolv.getValue(tempb));
  assertTrue_1(tempb);

  Value intv(42);
  assertTrue_1(intv.isKnown());
  assertTrue_1(INTEGER_TYPE == intv.valueType());
  int32_t tempi;
  assertTrue_1(intv.getValue(tempi));
  assertTrue_1(tempi == 42);

  Value realv(2.5);
  assertTrue_1(realv.isKnown());
  assertTrue_1(REAL_TYPE == realv.valueType());
  double tempr;
  assertTrue_1(realv.getValue(tempr));
  assertTrue_1(tempr == 2.5);

  std::string foo("Foo");
  Value stringv(foo);
  assertTrue_1(stringv.isKnown());
  assertTrue_1(STRING_TYPE == stringv.valueType());
  std::string temps;
  assertTrue_1(stringv.getValue(temps));
  assertTrue_1(temps == foo);
  std::string const *tempsp;
  assertTrue_1(stringv.getValuePointer(tempsp));
  assertTrue_1(*tempsp == foo);

  BooleanArray emptyBool;
  Value bav(emptyBool);
  assertTrue_1(bav.isKnown());
  assertTrue_1(BOOLEAN_ARRAY_TYPE == bav.valueType());
  BooleanArray const * tempbap;
  assertTrue_1(bav.getValuePointer(tempbap));
  assertTrue_1(emptyBool == *tempbap);

  IntegerArray emptyInt;
  Value iav(emptyInt);
  assertTrue_1(iav.isKnown());
  assertTrue_1(INTEGER_ARRAY_TYPE == iav.valueType());
  IntegerArray const * tempiap;
  assertTrue_1(iav.getValuePointer(tempiap));
  assertTrue_1(emptyInt == *tempiap);

  RealArray emptyReal;
  Value rav(emptyReal);
  assertTrue_1(rav.isKnown());
  assertTrue_1(REAL_ARRAY_TYPE == rav.valueType());
  RealArray const * temprap;
  assertTrue_1(rav.getValuePointer(temprap));
  assertTrue_1(emptyReal == *temprap);

  StringArray emptyString;
  Value sav(emptyString);
  assertTrue_1(sav.isKnown());
  assertTrue_1(STRING_ARRAY_TYPE == sav.valueType());
  StringArray const * tempsap;
  assertTrue_1(sav.getValuePointer(tempsap));
  assertTrue_1(emptyString == *tempsap);

  BooleanArray sizedBool(2);
  Value sbav(sizedBool);
  assertTrue_1(sbav.isKnown());
  assertTrue_1(BOOLEAN_ARRAY_TYPE == sbav.valueType());
  assertTrue_1(sbav.getValuePointer(tempbap));
  assertTrue_1(sizedBool == *tempbap);
  assertTrue_1(tempbap->size() == 2);
  assertTrue_1(bav != sbav);

  IntegerArray sizedInt(2);
  Value siav(sizedInt);
  assertTrue_1(siav.isKnown());
  assertTrue_1(INTEGER_ARRAY_TYPE == siav.valueType());
  assertTrue_1(siav.getValuePointer(tempiap));
  assertTrue_1(sizedInt == *tempiap);
  assertTrue_1(iav != siav);

  RealArray sizedReal(2);
  Value srav(sizedReal);
  assertTrue_1(srav.isKnown());
  assertTrue_1(REAL_ARRAY_TYPE == srav.valueType());
  assertTrue_1(srav.getValuePointer(temprap));
  assertTrue_1(sizedReal == *temprap);
  assertTrue_1(rav != srav);

  StringArray sizedString(2);
  Value ssav(sizedString);
  assertTrue_1(ssav.isKnown());
  assertTrue_1(STRING_ARRAY_TYPE == ssav.valueType());
  assertTrue_1(ssav.getValuePointer(tempsap));
  assertTrue_1(sizedString == *tempsap);
  assertTrue_1(sav != ssav);

  std::vector<bool> bv(2);
  bv[0] = false;
  bv[1] = true;
  BooleanArray initedBool(bv);
  Value ibav(initedBool);
  assertTrue_1(ibav.isKnown());
  assertTrue_1(BOOLEAN_ARRAY_TYPE == ibav.valueType());
  assertTrue_1(ibav.getValuePointer(tempbap));
  assertTrue_1(initedBool == *tempbap);
  assertTrue_1(tempbap->size() == 2);
  assertTrue_1(bav != ibav);
  assertTrue_1(sbav != ibav);

  std::vector<int32_t> iv(2);
  iv[0] = 42;
  iv[1] = 6;
  IntegerArray initedInt(iv);
  Value iiav(initedInt);
  assertTrue_1(iiav.isKnown());
  assertTrue_1(INTEGER_ARRAY_TYPE == iiav.valueType());
  assertTrue_1(iiav.getValuePointer(tempiap));
  assertTrue_1(initedInt == *tempiap);
  assertTrue_1(iav != iiav);
  assertTrue_1(siav != iiav);

  std::vector<double> dv(2);
  dv[0] = 3.14;
  dv[1] = 4.5;
  RealArray initedReal(dv);
  Value irav(initedReal);
  assertTrue_1(irav.isKnown());
  assertTrue_1(REAL_ARRAY_TYPE == irav.valueType());
  assertTrue_1(irav.getValuePointer(temprap));
  assertTrue_1(initedReal == *temprap);
  assertTrue_1(rav != irav);
  assertTrue_1(srav != irav);

  std::vector<std::string> sv(2);
  sv[0] = std::string("yo ");
  sv[1] = std::string("mama");
  StringArray initedString(sv);
  Value isav(initedString);
  assertTrue_1(isav.isKnown());
  assertTrue_1(STRING_ARRAY_TYPE == isav.valueType());
  assertTrue_1(isav.getValuePointer(tempsap));
  assertTrue_1(initedString == *tempsap);
  assertTrue_1(sav != isav);
  assertTrue_1(ssav != isav);

  return true;
}

bool valueTest()
{
  runTest(testConstructorsAndAccessors);

  return true;
}
