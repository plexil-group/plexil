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

  Value stringv2(foo.c_str());
  assertTrue_1(stringv2.isKnown());
  assertTrue_1(STRING_TYPE == stringv2.valueType());
  assertTrue_1(stringv2.getValue(temps));
  assertTrue_1(temps == foo);
  assertTrue_1(stringv2.getValuePointer(tempsp));
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

  IntegerArray sizedInt(2);
  Value siav(sizedInt);
  assertTrue_1(siav.isKnown());
  assertTrue_1(INTEGER_ARRAY_TYPE == siav.valueType());
  assertTrue_1(siav.getValuePointer(tempiap));
  assertTrue_1(sizedInt == *tempiap);

  RealArray sizedReal(2);
  Value srav(sizedReal);
  assertTrue_1(srav.isKnown());
  assertTrue_1(REAL_ARRAY_TYPE == srav.valueType());
  assertTrue_1(srav.getValuePointer(temprap));
  assertTrue_1(sizedReal == *temprap);

  StringArray sizedString(2);
  Value ssav(sizedString);
  assertTrue_1(ssav.isKnown());
  assertTrue_1(STRING_ARRAY_TYPE == ssav.valueType());
  assertTrue_1(ssav.getValuePointer(tempsap));
  assertTrue_1(sizedString == *tempsap);

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

  std::vector<int32_t> iv(2);
  iv[0] = 42;
  iv[1] = 6;
  IntegerArray initedInt(iv);
  Value iiav(initedInt);
  assertTrue_1(iiav.isKnown());
  assertTrue_1(INTEGER_ARRAY_TYPE == iiav.valueType());
  assertTrue_1(iiav.getValuePointer(tempiap));
  assertTrue_1(initedInt == *tempiap);

  std::vector<double> dv(2);
  dv[0] = 3.14;
  dv[1] = 4.5;
  RealArray initedReal(dv);
  Value irav(initedReal);
  assertTrue_1(irav.isKnown());
  assertTrue_1(REAL_ARRAY_TYPE == irav.valueType());
  assertTrue_1(irav.getValuePointer(temprap));
  assertTrue_1(initedReal == *temprap);

  std::vector<std::string> sv(2);
  sv[0] = std::string("yo ");
  sv[1] = std::string("mama");
  StringArray initedString(sv);
  Value isav(initedString);
  assertTrue_1(isav.isKnown());
  assertTrue_1(STRING_ARRAY_TYPE == isav.valueType());
  assertTrue_1(isav.getValuePointer(tempsap));
  assertTrue_1(initedString == *tempsap);

  return true;
}

static bool testEquality()
{
  // Basics
  Value unkv, tempv; // both type & value unknown
  Value boolv(true);
  Value intv(42);
  Value realv(2.5);
  std::string foo("Foo");
  Value stringv(foo);

  // Identity
  assertTrue_1(unkv == unkv);
  assertTrue_1(!(unkv != unkv));

  assertTrue_1(tempv == tempv);
  assertTrue_1(!(tempv != tempv));

  assertTrue_1(boolv == boolv);
  assertTrue_1(!(boolv != boolv));

  assertTrue_1(intv == intv);
  assertTrue_1(!(intv != intv));

  assertTrue_1(realv == realv);
  assertTrue_1(!(realv != realv));

  assertTrue_1(stringv == stringv);
  assertTrue_1(!(stringv != stringv));

  // Unknown type, unknown value
  assertTrue_1(unkv == tempv);
  assertTrue_1(!(unkv != tempv));
  assertTrue_1(tempv == unkv);
  assertTrue_1(!(tempv != unkv));

  // Across types
  assertTrue_1(!(unkv == boolv));
  assertTrue_1(unkv != boolv);
  assertTrue_1(!(boolv == unkv));
  assertTrue_1(boolv != unkv);

  assertTrue_1(!(unkv == intv));
  assertTrue_1(unkv != intv);
  assertTrue_1(!(intv == unkv));
  assertTrue_1(intv != unkv);

  assertTrue_1(!(unkv == realv));
  assertTrue_1(unkv != realv);
  assertTrue_1(!(realv == unkv));
  assertTrue_1(realv != unkv);

  assertTrue_1(!(unkv == stringv));
  assertTrue_1(unkv != stringv);
  assertTrue_1(!(stringv == unkv));
  assertTrue_1(stringv != unkv);

  assertTrue_1(!(boolv == intv));
  assertTrue_1(boolv != intv);
  assertTrue_1(!(intv == boolv));
  assertTrue_1(intv != boolv);

  assertTrue_1(!(boolv == realv));
  assertTrue_1(boolv != realv);
  assertTrue_1(!(realv == boolv));
  assertTrue_1(realv != boolv);

  assertTrue_1(!(boolv == stringv));
  assertTrue_1(boolv != stringv);
  assertTrue_1(!(stringv == boolv));
  assertTrue_1(stringv != boolv);

  assertTrue_1(!(intv == realv));
  assertTrue_1(intv != realv);
  assertTrue_1(!(realv == intv));
  assertTrue_1(realv != intv);

  assertTrue_1(!(intv == stringv));
  assertTrue_1(intv != stringv);
  assertTrue_1(!(stringv == intv));
  assertTrue_1(stringv != intv);

  assertTrue_1(!(realv == stringv));
  assertTrue_1(realv != stringv);
  assertTrue_1(!(stringv == realv));
  assertTrue_1(stringv != realv);

  // Via assignment
  tempv = unkv;
  assertTrue_1(tempv == unkv);
  assertTrue_1(!(tempv != unkv));
  assertTrue_1(unkv == tempv);
  assertTrue_1(!(unkv != tempv));
  
  tempv = boolv;
  assertTrue_1(tempv == boolv);
  assertTrue_1(!(tempv != boolv));
  assertTrue_1(boolv == tempv);
  assertTrue_1(!(boolv != tempv));
  
  tempv = intv;
  assertTrue_1(tempv == intv);
  assertTrue_1(!(tempv != intv));
  assertTrue_1(intv == tempv);
  assertTrue_1(!(intv != tempv));
  
  tempv = realv;
  assertTrue_1(tempv == realv);
  assertTrue_1(!(tempv != realv));
  assertTrue_1(realv == tempv);
  assertTrue_1(!(realv != tempv));
  
  tempv = stringv;
  assertTrue_1(tempv == stringv);
  assertTrue_1(!(tempv != stringv));
  assertTrue_1(stringv == tempv);
  assertTrue_1(!(stringv != tempv));

  // Real vs integer
  Value real42v((double) 42);
  assertTrue_1(real42v.valueType() == REAL_TYPE);
  assertTrue_1(intv == real42v);
  assertTrue_1(!(intv != real42v));
  assertTrue_1(real42v == intv);
  assertTrue_1(!(real42v != intv));

  assertTrue_1(!(realv == real42v));
  assertTrue_1(realv != real42v);
  assertTrue_1(!(real42v == realv));
  assertTrue_1(real42v != realv);

  // Arrays
  BooleanArray emptyBool;
  Value bav(emptyBool);

  assertTrue_1(bav == bav);
  assertTrue_1(!(bav != bav));

  BooleanArray sizedBool(2);
  Value sbav(sizedBool);

  assertTrue_1(sbav == sbav);
  assertTrue_1(!(sbav != sbav));

  assertTrue_1(!(bav == sbav));
  assertTrue_1(bav != sbav);
  assertTrue_1(!(sbav == bav));
  assertTrue_1(sbav != bav);

  std::vector<bool> bv(2);
  bv[0] = false;
  bv[1] = true;
  BooleanArray initedBool(bv);
  Value ibav(initedBool);

  assertTrue_1(ibav == ibav);
  assertTrue_1(!(ibav != ibav));

  assertTrue_1(!(bav == ibav));
  assertTrue_1(bav != ibav);
  assertTrue_1(!(ibav == bav));
  assertTrue_1(ibav != bav);

  assertTrue_1(!(sbav == ibav));
  assertTrue_1(sbav != ibav);
  assertTrue_1(!(ibav == sbav));
  assertTrue_1(ibav != sbav);

  IntegerArray emptyInt;
  Value iav(emptyInt);

  assertTrue_1(iav == iav);
  assertTrue_1(!(iav != iav));

  IntegerArray sizedInt(2);
  Value siav(sizedInt);

  assertTrue_1(siav == siav);
  assertTrue_1(!(siav != siav));

  assertTrue_1(!(iav == siav));
  assertTrue_1(iav != siav);
  assertTrue_1(!(siav == iav));
  assertTrue_1(siav != iav);

  std::vector<int32_t> iv(2);
  iv[0] = 42;
  iv[1] = 6;
  IntegerArray initedInt(iv);
  Value iiav(initedInt);

  assertTrue_1(iiav == iiav);
  assertTrue_1(!(iiav != iiav));

  assertTrue_1(!(iav == iiav));
  assertTrue_1(iav != iiav);
  assertTrue_1(!(iiav == iav));
  assertTrue_1(iiav != iav);

  assertTrue_1(!(siav == iiav));
  assertTrue_1(siav != iiav);
  assertTrue_1(!(iiav == siav));
  assertTrue_1(iiav != siav);

  RealArray emptyReal;
  Value rav(emptyReal);

  assertTrue_1(rav == rav);
  assertTrue_1(!(rav != rav));

  RealArray sizedReal(2);
  Value srav(sizedReal);

  assertTrue_1(srav == srav);
  assertTrue_1(!(srav != srav));

  assertTrue_1(!(rav == srav));
  assertTrue_1(rav != srav);
  assertTrue_1(!(srav == rav));
  assertTrue_1(srav != rav);

  std::vector<double> dv(2);
  dv[0] = 3.14;
  dv[1] = 4.5;
  RealArray initedReal(dv);
  Value irav(initedReal);

  assertTrue_1(irav == irav);
  assertTrue_1(!(irav != irav));

  assertTrue_1(!(rav == irav));
  assertTrue_1(rav != irav);
  assertTrue_1(!(irav == rav));
  assertTrue_1(irav != rav);

  assertTrue_1(!(srav == irav));
  assertTrue_1(srav != irav);
  assertTrue_1(!(irav == srav));
  assertTrue_1(irav != srav);

  StringArray emptyString;
  Value sav(emptyString);

  assertTrue_1(sav == sav);
  assertTrue_1(!(sav != sav));

  StringArray sizedString(2);
  Value ssav(sizedString);

  assertTrue_1(ssav == ssav);
  assertTrue_1(!(ssav != ssav));

  assertTrue_1(!(sav == ssav));
  assertTrue_1(sav != ssav);
  assertTrue_1(!(ssav == sav));
  assertTrue_1(ssav != sav);

  std::vector<std::string> sv(2);
  sv[0] = std::string("yo ");
  sv[1] = std::string("mama");
  StringArray initedString(sv);
  Value isav(initedString);

  assertTrue_1(isav == isav);
  assertTrue_1(!(isav != isav));

  assertTrue_1(!(sav == isav));
  assertTrue_1(sav != isav);
  assertTrue_1(!(isav == sav));
  assertTrue_1(isav != sav);

  assertTrue_1(!(ssav == isav));
  assertTrue_1(ssav != isav);
  assertTrue_1(!(isav == ssav));
  assertTrue_1(isav != ssav);

  // Via assignment
  tempv = bav;
  assertTrue_1(tempv == bav);
  assertTrue_1(!(tempv != bav));
  assertTrue_1(bav == tempv);
  assertTrue_1(!(bav != tempv));

  tempv = sbav;
  assertTrue_1(tempv == sbav);
  assertTrue_1(!(tempv != sbav));
  assertTrue_1(sbav == tempv);
  assertTrue_1(!(sbav != tempv));

  tempv = ibav;
  assertTrue_1(tempv == ibav);
  assertTrue_1(!(tempv != ibav));
  assertTrue_1(ibav == tempv);
  assertTrue_1(!(ibav != tempv));

  tempv = iav;
  assertTrue_1(tempv == iav);
  assertTrue_1(!(tempv != iav));
  assertTrue_1(iav == tempv);
  assertTrue_1(!(iav != tempv));

  tempv = siav;
  assertTrue_1(tempv == siav);
  assertTrue_1(!(tempv != siav));
  assertTrue_1(siav == tempv);
  assertTrue_1(!(siav != tempv));

  tempv = iiav;
  assertTrue_1(tempv == iiav);
  assertTrue_1(!(tempv != iiav));
  assertTrue_1(iiav == tempv);
  assertTrue_1(!(iiav != tempv));

  tempv = rav;
  assertTrue_1(tempv == rav);
  assertTrue_1(!(tempv != rav));
  assertTrue_1(rav == tempv);
  assertTrue_1(!(rav != tempv));

  tempv = srav;
  assertTrue_1(tempv == srav);
  assertTrue_1(!(tempv != srav));
  assertTrue_1(srav == tempv);
  assertTrue_1(!(srav != tempv));

  tempv = irav;
  assertTrue_1(tempv == irav);
  assertTrue_1(!(tempv != irav));
  assertTrue_1(irav == tempv);
  assertTrue_1(!(irav != tempv));

  tempv = sav;
  assertTrue_1(tempv == sav);
  assertTrue_1(!(tempv != sav));
  assertTrue_1(sav == tempv);
  assertTrue_1(!(sav != tempv));

  tempv = ssav;
  assertTrue_1(tempv == ssav);
  assertTrue_1(!(tempv != ssav));
  assertTrue_1(ssav == tempv);
  assertTrue_1(!(ssav != tempv));

  tempv = isav;
  assertTrue_1(tempv == isav);
  assertTrue_1(!(tempv != isav));
  assertTrue_1(isav == tempv);
  assertTrue_1(!(isav != tempv));

  // Cross array type
  assertTrue_1(!(bav == iav));
  assertTrue_1(bav != iav);
  assertTrue_1(!(iav == bav));
  assertTrue_1(iav != bav);

  assertTrue_1(!(bav == rav));
  assertTrue_1(bav != rav);
  assertTrue_1(!(rav == bav));
  assertTrue_1(rav != bav);

  assertTrue_1(!(bav == sav));
  assertTrue_1(bav != sav);
  assertTrue_1(!(sav == bav));
  assertTrue_1(sav != bav);

  assertTrue_1(!(iav == rav));
  assertTrue_1(iav != rav);
  assertTrue_1(!(rav == iav));
  assertTrue_1(rav != iav);

  assertTrue_1(!(iav == sav));
  assertTrue_1(iav != sav);
  assertTrue_1(!(sav == iav));
  assertTrue_1(sav != iav);

  assertTrue_1(!(rav == sav));
  assertTrue_1(rav != sav);
  assertTrue_1(!(sav == rav));
  assertTrue_1(sav != rav);

  // Array to unknown
  assertTrue(!(unkv == bav));
  assertTrue(unkv != bav);
  assertTrue(!(bav == unkv));
  assertTrue(bav != unkv);

  assertTrue(!(unkv == ibav));
  assertTrue(unkv != ibav);
  assertTrue(!(ibav == unkv));
  assertTrue(ibav != unkv);

  assertTrue(!(unkv == sbav));
  assertTrue(unkv != sbav);
  assertTrue(!(sbav == unkv));
  assertTrue(sbav != unkv);

  assertTrue(!(unkv == iav));
  assertTrue(unkv != iav);
  assertTrue(!(iav == unkv));
  assertTrue(iav != unkv);

  assertTrue(!(unkv == iiav));
  assertTrue(unkv != iiav);
  assertTrue(!(iiav == unkv));
  assertTrue(iiav != unkv);

  assertTrue(!(unkv == siav));
  assertTrue(unkv != siav);
  assertTrue(!(siav == unkv));
  assertTrue(siav != unkv);

  assertTrue(!(unkv == rav));
  assertTrue(unkv != rav);
  assertTrue(!(rav == unkv));
  assertTrue(rav != unkv);

  assertTrue(!(unkv == irav));
  assertTrue(unkv != irav);
  assertTrue(!(irav == unkv));
  assertTrue(irav != unkv);

  assertTrue(!(unkv == srav));
  assertTrue(unkv != srav);
  assertTrue(!(srav == unkv));
  assertTrue(srav != unkv);

  assertTrue(!(unkv == sav));
  assertTrue(unkv != sav);
  assertTrue(!(sav == unkv));
  assertTrue(sav != unkv);

  assertTrue(!(unkv == isav));
  assertTrue(unkv != isav);
  assertTrue(!(isav == unkv));
  assertTrue(isav != unkv);

  assertTrue(!(unkv == ssav));
  assertTrue(unkv != ssav);
  assertTrue(!(ssav == unkv));
  assertTrue(ssav != unkv);

  // Array to Boolean
  assertTrue(!(boolv == bav));
  assertTrue(boolv != bav);
  assertTrue(!(bav == boolv));
  assertTrue(bav != boolv);

  assertTrue(!(boolv == ibav));
  assertTrue(boolv != ibav);
  assertTrue(!(ibav == boolv));
  assertTrue(ibav != boolv);

  assertTrue(!(boolv == sbav));
  assertTrue(boolv != sbav);
  assertTrue(!(sbav == boolv));
  assertTrue(sbav != boolv);

  assertTrue(!(boolv == iav));
  assertTrue(boolv != iav);
  assertTrue(!(iav == boolv));
  assertTrue(iav != boolv);

  assertTrue(!(boolv == iiav));
  assertTrue(boolv != iiav);
  assertTrue(!(iiav == boolv));
  assertTrue(iiav != boolv);

  assertTrue(!(boolv == siav));
  assertTrue(boolv != siav);
  assertTrue(!(siav == boolv));
  assertTrue(siav != boolv);

  assertTrue(!(boolv == rav));
  assertTrue(boolv != rav);
  assertTrue(!(rav == boolv));
  assertTrue(rav != boolv);

  assertTrue(!(boolv == irav));
  assertTrue(boolv != irav);
  assertTrue(!(irav == boolv));
  assertTrue(irav != boolv);

  assertTrue(!(boolv == srav));
  assertTrue(boolv != srav);
  assertTrue(!(srav == boolv));
  assertTrue(srav != boolv);

  assertTrue(!(boolv == sav));
  assertTrue(boolv != sav);
  assertTrue(!(sav == boolv));
  assertTrue(sav != boolv);

  assertTrue(!(boolv == isav));
  assertTrue(boolv != isav);
  assertTrue(!(isav == boolv));
  assertTrue(isav != boolv);

  assertTrue(!(boolv == ssav));
  assertTrue(boolv != ssav);
  assertTrue(!(ssav == boolv));
  assertTrue(ssav != boolv);

  // Array to Integer
  assertTrue(!(intv == bav));
  assertTrue(intv != bav);
  assertTrue(!(bav == intv));
  assertTrue(bav != intv);

  assertTrue(!(intv == ibav));
  assertTrue(intv != ibav);
  assertTrue(!(ibav == intv));
  assertTrue(ibav != intv);

  assertTrue(!(intv == sbav));
  assertTrue(intv != sbav);
  assertTrue(!(sbav == intv));
  assertTrue(sbav != intv);

  assertTrue(!(intv == iav));
  assertTrue(intv != iav);
  assertTrue(!(iav == intv));
  assertTrue(iav != intv);

  assertTrue(!(intv == iiav));
  assertTrue(intv != iiav);
  assertTrue(!(iiav == intv));
  assertTrue(iiav != intv);

  assertTrue(!(intv == siav));
  assertTrue(intv != siav);
  assertTrue(!(siav == intv));
  assertTrue(siav != intv);

  assertTrue(!(intv == rav));
  assertTrue(intv != rav);
  assertTrue(!(rav == intv));
  assertTrue(rav != intv);

  assertTrue(!(intv == irav));
  assertTrue(intv != irav);
  assertTrue(!(irav == intv));
  assertTrue(irav != intv);

  assertTrue(!(intv == srav));
  assertTrue(intv != srav);
  assertTrue(!(srav == intv));
  assertTrue(srav != intv);

  assertTrue(!(intv == sav));
  assertTrue(intv != sav);
  assertTrue(!(sav == intv));
  assertTrue(sav != intv);

  assertTrue(!(intv == isav));
  assertTrue(intv != isav);
  assertTrue(!(isav == intv));
  assertTrue(isav != intv);

  assertTrue(!(intv == ssav));
  assertTrue(intv != ssav);
  assertTrue(!(ssav == intv));
  assertTrue(ssav != intv);

  // Array to Real
  assertTrue(!(realv == bav));
  assertTrue(realv != bav);
  assertTrue(!(bav == realv));
  assertTrue(bav != realv);

  assertTrue(!(realv == ibav));
  assertTrue(realv != ibav);
  assertTrue(!(ibav == realv));
  assertTrue(ibav != realv);

  assertTrue(!(realv == sbav));
  assertTrue(realv != sbav);
  assertTrue(!(sbav == realv));
  assertTrue(sbav != realv);

  assertTrue(!(realv == iav));
  assertTrue(realv != iav);
  assertTrue(!(iav == realv));
  assertTrue(iav != realv);

  assertTrue(!(realv == iiav));
  assertTrue(realv != iiav);
  assertTrue(!(iiav == realv));
  assertTrue(iiav != realv);

  assertTrue(!(realv == siav));
  assertTrue(realv != siav);
  assertTrue(!(siav == realv));
  assertTrue(siav != realv);

  assertTrue(!(realv == rav));
  assertTrue(realv != rav);
  assertTrue(!(rav == realv));
  assertTrue(rav != realv);

  assertTrue(!(realv == irav));
  assertTrue(realv != irav);
  assertTrue(!(irav == realv));
  assertTrue(irav != realv);

  assertTrue(!(realv == srav));
  assertTrue(realv != srav);
  assertTrue(!(srav == realv));
  assertTrue(srav != realv);

  assertTrue(!(realv == sav));
  assertTrue(realv != sav);
  assertTrue(!(sav == realv));
  assertTrue(sav != realv);

  assertTrue(!(realv == isav));
  assertTrue(realv != isav);
  assertTrue(!(isav == realv));
  assertTrue(isav != realv);

  assertTrue(!(realv == ssav));
  assertTrue(realv != ssav);
  assertTrue(!(ssav == realv));
  assertTrue(ssav != realv);

  // Array to String
  assertTrue(!(stringv == bav));
  assertTrue(stringv != bav);
  assertTrue(!(bav == stringv));
  assertTrue(bav != stringv);

  assertTrue(!(stringv == ibav));
  assertTrue(stringv != ibav);
  assertTrue(!(ibav == stringv));
  assertTrue(ibav != stringv);

  assertTrue(!(stringv == sbav));
  assertTrue(stringv != sbav);
  assertTrue(!(sbav == stringv));
  assertTrue(sbav != stringv);

  assertTrue(!(stringv == iav));
  assertTrue(stringv != iav);
  assertTrue(!(iav == stringv));
  assertTrue(iav != stringv);

  assertTrue(!(stringv == iiav));
  assertTrue(stringv != iiav);
  assertTrue(!(iiav == stringv));
  assertTrue(iiav != stringv);

  assertTrue(!(stringv == siav));
  assertTrue(stringv != siav);
  assertTrue(!(siav == stringv));
  assertTrue(siav != stringv);

  assertTrue(!(stringv == rav));
  assertTrue(stringv != rav);
  assertTrue(!(rav == stringv));
  assertTrue(rav != stringv);

  assertTrue(!(stringv == irav));
  assertTrue(stringv != irav);
  assertTrue(!(irav == stringv));
  assertTrue(irav != stringv);

  assertTrue(!(stringv == srav));
  assertTrue(stringv != srav);
  assertTrue(!(srav == stringv));
  assertTrue(srav != stringv);

  assertTrue(!(stringv == sav));
  assertTrue(stringv != sav);
  assertTrue(!(sav == stringv));
  assertTrue(sav != stringv);

  assertTrue(!(stringv == isav));
  assertTrue(stringv != isav);
  assertTrue(!(isav == stringv));
  assertTrue(isav != stringv);

  assertTrue(!(stringv == ssav));
  assertTrue(stringv != ssav);
  assertTrue(!(ssav == stringv));
  assertTrue(ssav != stringv);

  return true;
}

static bool testLessThan()
{
  // Basics
  
  // both type & value unknown
  Value unkv, tempv;
  assertTrue_1(!(unkv < unkv));
  assertTrue_1(!(unkv < tempv));
  assertTrue_1(!(tempv < unkv));

  // unknown but typed
  Value unkbool(0, BOOLEAN_TYPE);
  Value unkint(0, INTEGER_TYPE);
  Value unkreal(0, REAL_TYPE);
  Value unkstr(0, STRING_TYPE);
  assertTrue_1(!(unkbool < unkbool));
  assertTrue_1(!(unkint < unkint));
  assertTrue_1(!(unkreal < unkreal));
  assertTrue_1(!(unkstr < unkstr));
  // Unknown unknown always less than typed unknown
  assertTrue_1(unkv < unkbool);
  assertTrue_1(!(unkbool < unkv));
  assertTrue_1(unkv < unkint);
  assertTrue_1(!(unkint < unkv));
  assertTrue_1(unkv < unkreal);
  assertTrue_1(!(unkreal < unkv));
  assertTrue_1(unkv < unkstr);
  assertTrue_1(!(unkstr < unkv));

  Value troo(true);
  Value falls(false);
  assertTrue_1(!(troo < troo));
  assertTrue_1(!(falls < falls));
  assertTrue_1(!(troo < falls));
  assertTrue_1(falls < troo);

  // Unknown unknown always < typed known
  assertTrue_1(unkv < troo);
  assertTrue_1(!(troo < unkv));
  assertTrue_1(unkv < falls);
  assertTrue_1(!(falls < unkv));

  // Same type unknown always < known
  assertTrue_1(unkbool < falls);
  assertTrue_1(unkbool < troo);
  assertTrue_1(!(falls < unkbool));
  assertTrue_1(!(troo < unkbool));

  // Greater type unknown always < known
  assertTrue_1(unkint < falls);
  assertTrue_1(unkint < troo);
  assertTrue_1(!(falls < unkint));
  assertTrue_1(!(troo < unkint));

  assertTrue_1(unkreal < falls);
  assertTrue_1(unkreal < troo);
  assertTrue_1(!(falls < unkreal));
  assertTrue_1(!(troo < unkreal));

  assertTrue_1(unkstr < falls);
  assertTrue_1(unkstr < troo);
  assertTrue_1(!(falls < unkstr));
  assertTrue_1(!(troo < unkstr));

  Value fortytwo(42);
  Value fortythree(43);
  assertTrue_1(!(fortytwo < fortytwo));
  assertTrue_1(!(fortythree < fortythree));
  assertTrue_1(fortytwo < fortythree);
  assertTrue_1(!(fortythree < fortytwo));

  assertTrue_1(unkv < fortytwo);
  assertTrue_1(!(fortytwo < unkv));
  assertTrue_1(unkv < fortythree);
  assertTrue_1(!(fortythree < unkv));

  // Lesser typed unknown always less than typed known
  assertTrue_1(unkbool < fortytwo);
  assertTrue_1(!(fortytwo < unkbool));
  assertTrue_1(unkbool < fortythree);
  assertTrue_1(!(fortythree < unkbool));

  assertTrue_1(unkint < fortytwo);
  assertTrue_1(!(fortytwo < unkint));
  assertTrue_1(unkint < fortythree);
  assertTrue_1(!(fortythree < unkint));

  // Greater typed unknown always less than typed known
  assertTrue_1(unkreal < fortytwo);
  assertTrue_1(!(fortytwo < unkreal));
  assertTrue_1(unkreal < fortythree);
  assertTrue_1(!(fortythree < unkreal));

  assertTrue_1(unkstr < fortytwo);
  assertTrue_1(!(fortytwo < unkstr));
  assertTrue_1(unkstr < fortythree);
  assertTrue_1(!(fortythree < unkstr));

  // Both known, lesser type is less
  assertTrue_1(troo < fortytwo);
  assertTrue_1(!(fortytwo < troo));

  Value ev(2.718);
  Value piv(3.14);
  assertTrue_1(!(ev < ev));
  assertTrue_1(!(piv < piv));
  assertTrue_1(ev < piv);
  assertTrue_1(!(piv < ev));

  assertTrue_1(unkv < ev);
  assertTrue_1(!(ev < unkv));
  assertTrue_1(unkv < piv);
  assertTrue_1(!(piv < unkv));

  // Lesser typed unknown always less than typed known
  assertTrue_1(unkbool < ev);
  assertTrue_1(!(ev < unkbool));
  assertTrue_1(unkbool < piv);
  assertTrue_1(!(piv < unkbool));

  assertTrue_1(unkint < ev);
  assertTrue_1(!(ev < unkint));
  assertTrue_1(unkint < piv);
  assertTrue_1(!(piv < unkint));

  assertTrue_1(unkreal < ev);
  assertTrue_1(!(ev < unkreal));
  assertTrue_1(unkreal < piv);
  assertTrue_1(!(piv < unkreal));

  // Greater typed unknown always less than typed known
  assertTrue_1(unkstr < ev);
  assertTrue_1(!(ev < unkstr));
  assertTrue_1(unkstr < piv);
  assertTrue_1(!(piv < unkstr));

  // Both known, lesser type is less
  assertTrue_1(troo < ev);
  assertTrue_1(!(ev < troo));

  // Standard numeric rules apply
  assertTrue_1(!(fortytwo < ev));
  assertTrue_1(ev < fortytwo);

  std::string foo("Foo");
  std::string fop("Fop");
  Value foov(foo);
  Value fopv(fop);
  assertTrue_1(!(foov < foov));
  assertTrue_1(!(fopv < fopv));
  assertTrue_1(foov < fopv);
  assertTrue_1(!(fopv < foov));

  assertTrue_1(unkv < foov);
  assertTrue_1(!(foov < unkv));
  assertTrue_1(unkv < fopv);
  assertTrue_1(!(fopv < unkv));

  // Lesser typed unknown always less than typed known
  assertTrue_1(unkbool < foov);
  assertTrue_1(!(foov < unkbool));
  assertTrue_1(unkbool < fopv);
  assertTrue_1(!(fopv < unkbool));

  assertTrue_1(unkint < foov);
  assertTrue_1(!(foov < unkint));
  assertTrue_1(unkint < fopv);
  assertTrue_1(!(fopv < unkint));

  assertTrue_1(unkreal < foov);
  assertTrue_1(!(foov < unkreal));
  assertTrue_1(unkreal < fopv);
  assertTrue_1(!(fopv < unkreal));

  // Same type, unknown less than known
  assertTrue_1(unkstr < foov);
  assertTrue_1(!(foov < unkstr));
  assertTrue_1(unkstr < fopv);
  assertTrue_1(!(fopv < unkstr));

  // Both known, lesser type is less
  assertTrue_1(troo < foov);
  assertTrue_1(!(foov < troo));

  assertTrue_1(fortytwo < foov);
  assertTrue_1(!(foov < fortytwo));

  assertTrue_1(piv < foov);
  assertTrue_1(!(foov < piv));

  // Real vs integer - std numerical rules
  assertTrue_1(ev < fortytwo);
  assertTrue_1(!(fortytwo < ev));

  Value real42v((double) 42);
  assertTrue_1(real42v.valueType() == REAL_TYPE);
  // Equal
  assertTrue_1(!(fortytwo < real42v));
  assertTrue_1(!(real42v < fortytwo));

  // Arrays
  Value ubav(0, BOOLEAN_ARRAY_TYPE);
  assertTrue_1(!(ubav < ubav));

  BooleanArray emptyBool;
  Value ebav(emptyBool);
  assertTrue_1(!(ebav < ebav));

  assertTrue_1(ubav < ebav);
  assertTrue_1(!(ebav < ubav));

  BooleanArray sizedBool(2);
  Value sbav(sizedBool);
  assertTrue_1(!(sbav < sbav));

  assertTrue_1(ubav < sbav);
  assertTrue_1(!(sbav < ubav));

  assertTrue_1(ebav < sbav);
  assertTrue_1(!(sbav < ebav));

  std::vector<bool> bv(2);
  bv[0] = false;
  bv[1] = true;
  BooleanArray initedBool(bv);
  Value ibav(initedBool);
  assertTrue_1(!(ibav < ibav));

  assertTrue_1(ubav < ibav);
  assertTrue_1(!(ibav < ubav));

  assertTrue_1(ebav < ibav);
  assertTrue_1(!(ibav < ebav));

  assertTrue_1(sbav < ibav);
  assertTrue_1(!(ibav < sbav));

  std::vector<bool> bv2(2);
  bv2[0] = false;
  bv2[1] = false;
  BooleanArray initedBool2(bv2);
  Value ibav2(initedBool2);
  assertTrue_1(!(ibav2 < ibav2));

  assertTrue_1(ubav < ibav2);
  assertTrue_1(!(ibav2 < ubav));

  assertTrue_1(ebav < ibav2);
  assertTrue_1(!(ibav2 < ebav));

  assertTrue_1(sbav < ibav2);
  assertTrue_1(!(ibav2 < sbav));

  // Same size, contents determine the victor
  assertTrue_1(!(ibav < ibav2));
  assertTrue_1(ibav2 < ibav);

  Value uiav(0, INTEGER_ARRAY_TYPE);
  assertTrue_1(!(uiav < uiav));

  IntegerArray emptyInt;
  Value eiav(emptyInt);
  assertTrue_1(!(eiav < eiav));

  assertTrue_1(uiav < eiav);
  assertTrue_1(!(eiav < uiav));

  IntegerArray sizedInt(2);
  Value siav(sizedInt);
  assertTrue_1(!(siav < siav));

  assertTrue_1(uiav < siav);
  assertTrue_1(!(siav < uiav));

  assertTrue_1(eiav < siav);
  assertTrue_1(!(siav < eiav));

  std::vector<int32_t> iv(2);
  iv[0] = 42;
  iv[1] = 6;
  IntegerArray initedInt(iv);
  Value iiav(initedInt);
  assertTrue_1(!(iiav < iiav));

  assertTrue_1(uiav < iiav);
  assertTrue_1(!(iiav < uiav));

  assertTrue_1(eiav < iiav);
  assertTrue_1(!(iiav < eiav));

  assertTrue_1(siav < iiav);
  assertTrue_1(!(iiav < siav));

  std::vector<int32_t> iv2(2);
  iv2[0] = 42;
  iv2[1] = 7;
  IntegerArray initedInt2(iv2);
  Value iiav2(initedInt2);
  assertTrue_1(!(iiav2 < iiav2));

  assertTrue_1(uiav < iiav2);
  assertTrue_1(!(iiav2 < uiav));

  assertTrue_1(eiav < iiav2);
  assertTrue_1(!(iiav2 < eiav));

  assertTrue_1(siav < iiav2);
  assertTrue_1(!(iiav2 < siav));

  // Same size, contents determine the victor
  assertTrue_1(iiav < iiav2);
  assertTrue_1(!(iiav2 < iiav));

  Value urav(0, REAL_ARRAY_TYPE);
  assertTrue_1(!(urav < urav));

  RealArray emptyReal;
  Value erav(emptyReal);
  assertTrue_1(!(erav < erav));

  assertTrue_1(urav < erav);
  assertTrue_1(!(erav < urav));

  RealArray sizedReal(2);
  Value srav(sizedReal);
  assertTrue_1(!(srav < srav));

  assertTrue_1(urav < srav);
  assertTrue_1(!(srav < urav));

  assertTrue_1(erav < srav);
  assertTrue_1(!(srav < erav));

  std::vector<double> dv(2);
  dv[0] = 3.14;
  dv[1] = 4.5;
  RealArray initedReal(dv);
  Value irav(initedReal);
  assertTrue_1(!(irav < irav));

  assertTrue_1(urav < irav);
  assertTrue_1(!(irav < urav));

  assertTrue_1(erav < irav);
  assertTrue_1(!(irav < erav));

  assertTrue_1(srav < irav);
  assertTrue_1(!(irav < srav));

  std::vector<double> dv2(2);
  dv2[0] = 3.14;
  dv2[1] = 4.6;
  RealArray initedReal2(dv2);
  Value irav2(initedReal2);
  assertTrue_1(!(irav2 < irav2));

  assertTrue_1(urav < irav2);
  assertTrue_1(!(irav2 < urav));

  assertTrue_1(erav < irav2);
  assertTrue_1(!(irav2 < erav));

  assertTrue_1(srav < irav2);
  assertTrue_1(!(irav2 < srav));

  // Same size, contents determine the victor
  assertTrue_1(irav < irav2);
  assertTrue_1(!(irav2 < irav));

  Value usav(0, STRING_ARRAY_TYPE);
  assertTrue_1(!(usav < usav));

  StringArray emptyString;
  Value esav(emptyString);
  assertTrue_1(!(esav < esav));

  assertTrue_1(usav < esav);
  assertTrue_1(!(esav < usav));

  StringArray sizedString(2);
  Value ssav(sizedString);
  assertTrue_1(!(ssav < ssav));

  assertTrue_1(usav < ssav);
  assertTrue_1(!(ssav < usav));

  assertTrue_1(esav < ssav);
  assertTrue_1(!(ssav < esav));

  std::vector<std::string> sv(2);
  sv[0] = std::string("yo ");
  sv[1] = std::string("mama");
  StringArray initedString(sv);
  Value isav(initedString);
  assertTrue_1(!(isav < isav));

  assertTrue_1(usav < isav);
  assertTrue_1(!(isav < usav));

  assertTrue_1(esav < isav);
  assertTrue_1(!(isav < esav));

  assertTrue_1(ssav < isav);
  assertTrue_1(!(isav < ssav));

  std::vector<std::string> sv2(2);
  sv2[0] = std::string("yo ");
  sv2[1] = std::string("mamb");
  StringArray initedString2(sv2);
  Value isav2(initedString2);
  assertTrue_1(!(isav2 < isav2));

  assertTrue_1(usav < isav2);
  assertTrue_1(!(isav2 < usav));

  assertTrue_1(esav < isav2);
  assertTrue_1(!(isav2 < esav));

  assertTrue_1(ssav < isav2);
  assertTrue_1(!(isav2 < ssav));

  // Same size, contents determine the victor
  assertTrue_1(isav < isav2);
  assertTrue_1(!(isav2 < isav));

  // *** TODO ***

  // Cross array type

  // Array to unknown

  // Array to Boolean

  // Array to Integer

  // Array to Real

  // Array to String

  return true;
}

bool valueTest()
{
  runTest(testConstructorsAndAccessors);
  runTest(testEquality);
  runTest(testLessThan);

  return true;
}
