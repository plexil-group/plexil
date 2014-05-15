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

#include "Array.hh"
#include "TestSupport.hh"

using PLEXIL::Array;

static bool testConstructors()
{
  // Default constructor
  Array<bool>        emptyBool;
  Array<int32_t>     emptyInt;
  Array<double>      emptyReal;
  Array<std::string> emptyString;

  assertTrue_1(emptyBool.size()   == 0);
  assertTrue_1(emptyInt.size()    == 0);
  assertTrue_1(emptyReal.size()   == 0);
  assertTrue_1(emptyString.size() == 0);
  // not much else we can do with them in this state!

  // Sized constructor
  Array<bool>        sizedBool(2);
  Array<int32_t>     sizedInt(2);
  Array<double>      sizedReal(2);
  Array<std::string> sizedString(2);

  assertTrue_1(sizedBool.size()   == 2);
  assertTrue_1(sizedInt.size()    == 2);
  assertTrue_1(sizedReal.size()   == 2);
  assertTrue_1(sizedString.size() == 2);

  assertTrue_1(!sizedBool.elementKnown(0));
  assertTrue_1(!sizedBool.elementKnown(1));
  assertTrue_1(!sizedInt.elementKnown(0));
  assertTrue_1(!sizedInt.elementKnown(1));
  assertTrue_1(!sizedReal.elementKnown(0));
  assertTrue_1(!sizedReal.elementKnown(1));
  assertTrue_1(!sizedString.elementKnown(0));
  assertTrue_1(!sizedString.elementKnown(1));

  assertTrue_1(sizedBool   != emptyBool);
  assertTrue_1(sizedInt    != emptyInt);
  assertTrue_1(sizedReal   != emptyReal);
  assertTrue_1(sizedString != emptyString);

  bool btemp;
  int32_t itemp;
  double dtemp;
  std::string stemp;
  assertTrue_1(!sizedBool.getElement(0, btemp));
  assertTrue_1(!sizedBool.getElement(1, btemp));
  assertTrue_1(!sizedInt.getElement(0, itemp));
  assertTrue_1(!sizedInt.getElement(1, itemp));
  assertTrue_1(!sizedReal.getElement(0, dtemp));
  assertTrue_1(!sizedReal.getElement(1, dtemp));
  assertTrue_1(!sizedString.getElement(0, stemp));
  assertTrue_1(!sizedString.getElement(1, stemp));

  // Constructor from vector
  std::vector<bool>        bv(2);
  std::vector<int32_t>     iv(2);
  std::vector<double>      dv(2);
  std::vector<std::string> sv(2);

  bv[0] = false;
  bv[1] = true;

  iv[0] = 42;
  iv[1] = 6;

  dv[0] = 3.14;
  dv[1] = 4.5;

  sv[0] = std::string("yo ");
  sv[1] = std::string("mama");

  Array<bool>        initedBool(bv);
  Array<int32_t>     initedInt(iv);
  Array<double>      initedReal(dv);
  Array<std::string> initedString(sv);

  assertTrue_1(initedBool   != emptyBool);
  assertTrue_1(initedInt    != emptyInt);
  assertTrue_1(initedReal   != emptyReal);
  assertTrue_1(initedString != emptyString);

  assertTrue_1(initedBool   != sizedBool);
  assertTrue_1(initedInt    != sizedInt);
  assertTrue_1(initedReal   != sizedReal);
  assertTrue_1(initedString != sizedString);

  assertTrue_1(initedBool.size()   == 2);
  assertTrue_1(initedInt.size()    == 2);
  assertTrue_1(initedReal.size()   == 2);
  assertTrue_1(initedString.size() == 2);

  assertTrue_1(initedBool.elementKnown(0));
  assertTrue_1(initedBool.elementKnown(1));
  assertTrue_1(initedInt.elementKnown(0));
  assertTrue_1(initedInt.elementKnown(1));
  assertTrue_1(initedReal.elementKnown(0));
  assertTrue_1(initedReal.elementKnown(1));
  assertTrue_1(initedString.elementKnown(0));
  assertTrue_1(initedString.elementKnown(1));

  assertTrue_1(initedBool.getElement(0, btemp));
  assertTrue_1(btemp == bv[0]);
  assertTrue_1(initedBool.getElement(1, btemp));
  assertTrue_1(btemp == bv[1]);
  assertTrue_1(initedInt.getElement(0, itemp));
  assertTrue_1(itemp == iv[0]);
  assertTrue_1(initedInt.getElement(1, itemp));
  assertTrue_1(itemp == iv[1]);
  assertTrue_1(initedReal.getElement(0, dtemp));
  assertTrue_1(dtemp == dv[0]);
  assertTrue_1(initedReal.getElement(1, dtemp));
  assertTrue_1(dtemp == dv[1]);
  assertTrue_1(initedString.getElement(0, stemp));
  assertTrue_1(stemp == sv[0]);
  assertTrue_1(initedString.getElement(1, stemp));
  assertTrue_1(stemp == sv[1]);

  Array<bool>        copyBool(initedBool);
  Array<int32_t>     copyInt(initedInt);
  Array<double>      copyReal(initedReal);
  Array<std::string> copyString(initedString);

  assertTrue_1(copyBool   != emptyBool);
  assertTrue_1(copyInt    != emptyInt);
  assertTrue_1(copyReal   != emptyReal);
  assertTrue_1(copyString != emptyString);

  assertTrue_1(copyBool   != sizedBool);
  assertTrue_1(copyInt    != sizedInt);
  assertTrue_1(copyReal   != sizedReal);
  assertTrue_1(copyString != sizedString);

  assertTrue_1(copyBool   == initedBool);
  assertTrue_1(copyInt    == initedInt);
  assertTrue_1(copyReal   == initedReal);
  assertTrue_1(copyString == initedString);

  assertTrue_1(copyBool.size()   == 2);
  assertTrue_1(copyInt.size()    == 2);
  assertTrue_1(copyReal.size()   == 2);
  assertTrue_1(copyString.size() == 2);

  assertTrue_1(copyBool.elementKnown(0));
  assertTrue_1(copyBool.elementKnown(1));
  assertTrue_1(copyInt.elementKnown(0));
  assertTrue_1(copyInt.elementKnown(1));
  assertTrue_1(copyReal.elementKnown(0));
  assertTrue_1(copyReal.elementKnown(1));
  assertTrue_1(copyString.elementKnown(0));
  assertTrue_1(copyString.elementKnown(1));

  assertTrue_1(copyBool.getElement(0, btemp));
  assertTrue_1(btemp == bv[0]);
  assertTrue_1(copyBool.getElement(1, btemp));
  assertTrue_1(btemp == bv[1]);
  assertTrue_1(copyInt.getElement(0, itemp));
  assertTrue_1(itemp == iv[0]);
  assertTrue_1(copyInt.getElement(1, itemp));
  assertTrue_1(itemp == iv[1]);
  assertTrue_1(copyReal.getElement(0, dtemp));
  assertTrue_1(dtemp == dv[0]);
  assertTrue_1(copyReal.getElement(1, dtemp));
  assertTrue_1(dtemp == dv[1]);
  assertTrue_1(copyString.getElement(0, stemp));
  assertTrue_1(stemp == sv[0]);
  assertTrue_1(copyString.getElement(1, stemp));
  assertTrue_1(stemp == sv[1]);

  // Test assignment operator
  emptyBool   = copyBool;
  emptyInt    = copyInt;
  emptyReal   = copyReal;
  emptyString = copyString;

  assertTrue_1(emptyBool   != sizedBool);
  assertTrue_1(emptyInt    != sizedInt);
  assertTrue_1(emptyReal   != sizedReal);
  assertTrue_1(emptyString != sizedString);

  assertTrue_1(emptyBool   == initedBool);
  assertTrue_1(emptyInt    == initedInt);
  assertTrue_1(emptyReal   == initedReal);
  assertTrue_1(emptyString == initedString);

  assertTrue_1(emptyBool   == copyBool);
  assertTrue_1(emptyInt    == copyInt);
  assertTrue_1(emptyReal   == copyReal);
  assertTrue_1(emptyString == copyString);

  assertTrue_1(emptyBool.size()   == 2);
  assertTrue_1(emptyInt.size()    == 2);
  assertTrue_1(emptyReal.size()   == 2);
  assertTrue_1(emptyString.size() == 2);

  assertTrue_1(emptyBool.elementKnown(0));
  assertTrue_1(emptyBool.elementKnown(1));
  assertTrue_1(emptyInt.elementKnown(0));
  assertTrue_1(emptyInt.elementKnown(1));
  assertTrue_1(emptyReal.elementKnown(0));
  assertTrue_1(emptyReal.elementKnown(1));
  assertTrue_1(emptyString.elementKnown(0));
  assertTrue_1(emptyString.elementKnown(1));

  assertTrue_1(emptyBool.getElement(0, btemp));
  assertTrue_1(btemp == bv[0]);
  assertTrue_1(emptyBool.getElement(1, btemp));
  assertTrue_1(btemp == bv[1]);
  assertTrue_1(emptyInt.getElement(0, itemp));
  assertTrue_1(itemp == iv[0]);
  assertTrue_1(emptyInt.getElement(1, itemp));
  assertTrue_1(itemp == iv[1]);
  assertTrue_1(emptyReal.getElement(0, dtemp));
  assertTrue_1(dtemp == dv[0]);
  assertTrue_1(emptyReal.getElement(1, dtemp));
  assertTrue_1(dtemp == dv[1]);
  assertTrue_1(emptyString.getElement(0, stemp));
  assertTrue_1(stemp == sv[0]);
  assertTrue_1(emptyString.getElement(1, stemp));
  assertTrue_1(stemp == sv[1]);

  return true;
}

bool testResize()
{
  // Empty
  Array<bool>        emptyBool;
  Array<int32_t>     emptyInt;
  Array<double>      emptyReal;
  Array<std::string> emptyString;

  assertTrue_1(emptyBool.size()   == 0);
  assertTrue_1(emptyInt.size()    == 0);
  assertTrue_1(emptyReal.size()   == 0);
  assertTrue_1(emptyString.size() == 0);

  // Sized but uninitialized
  Array<bool>        sizedBool(2);
  Array<int32_t>     sizedInt(2);
  Array<double>      sizedReal(2);
  Array<std::string> sizedString(2);

  assertTrue_1(sizedBool.size()   == 2);
  assertTrue_1(sizedInt.size()    == 2);
  assertTrue_1(sizedReal.size()   == 2);
  assertTrue_1(sizedString.size() == 2);

  assertTrue_1(sizedBool   != emptyBool);
  assertTrue_1(sizedInt    != emptyInt);
  assertTrue_1(sizedReal   != emptyReal);
  assertTrue_1(sizedString != emptyString);

  emptyBool.resize(2);
  emptyInt.resize(2);
  emptyReal.resize(2);
  emptyString.resize(2);

  assertTrue_1(emptyBool.size()   == 2);
  assertTrue_1(emptyInt.size()    == 2);
  assertTrue_1(emptyReal.size()   == 2);
  assertTrue_1(emptyString.size() == 2);

  assertTrue_1(!emptyBool.elementKnown(0));
  assertTrue_1(!emptyBool.elementKnown(1));
  assertTrue_1(!emptyInt.elementKnown(0));
  assertTrue_1(!emptyInt.elementKnown(1));
  assertTrue_1(!emptyReal.elementKnown(0));
  assertTrue_1(!emptyReal.elementKnown(1));
  assertTrue_1(!emptyString.elementKnown(0));
  assertTrue_1(!emptyString.elementKnown(1));

  bool btemp;
  int32_t itemp;
  double dtemp;
  std::string stemp;
  assertTrue_1(!emptyBool.getElement(0, btemp));
  assertTrue_1(!emptyBool.getElement(1, btemp));
  assertTrue_1(!emptyInt.getElement(0, itemp));
  assertTrue_1(!emptyInt.getElement(1, itemp));
  assertTrue_1(!emptyReal.getElement(0, dtemp));
  assertTrue_1(!emptyReal.getElement(1, dtemp));
  assertTrue_1(!emptyString.getElement(0, stemp));
  assertTrue_1(!emptyString.getElement(1, stemp));

  assertTrue_1(sizedBool   == emptyBool);
  assertTrue_1(sizedInt    == emptyInt);
  assertTrue_1(sizedReal   == emptyReal);
  assertTrue_1(sizedString == emptyString);

  // Resize downward
  emptyBool.resize(1);
  emptyInt.resize(1);
  emptyReal.resize(1);
  emptyString.resize(1);

  // Size should not change
  assertTrue_1(emptyBool.size()   == 2);
  assertTrue_1(emptyInt.size()    == 2);
  assertTrue_1(emptyReal.size()   == 2);
  assertTrue_1(emptyString.size() == 2);
  

  // Initialized
  std::vector<bool>        bv(2);
  std::vector<int32_t>     iv(2);
  std::vector<double>      dv(2);
  std::vector<std::string> sv(2);

  bv[0] = false;
  bv[1] = true;

  iv[0] = 42;
  iv[1] = 6;

  dv[0] = 3.14;
  dv[1] = 4.5;

  sv[0] = std::string("yo ");
  sv[1] = std::string("mama");

  Array<bool>        initedBool(bv);
  Array<int32_t>     initedInt(iv);
  Array<double>      initedReal(dv);
  Array<std::string> initedString(sv);

  // Verify initial contents
  assertTrue_1(initedBool.size()   == 2);
  assertTrue_1(initedInt.size()    == 2);
  assertTrue_1(initedReal.size()   == 2);
  assertTrue_1(initedString.size() == 2);

  assertTrue_1(initedBool.elementKnown(0));
  assertTrue_1(initedBool.elementKnown(1));
  assertTrue_1(initedInt.elementKnown(0));
  assertTrue_1(initedInt.elementKnown(1));
  assertTrue_1(initedReal.elementKnown(0));
  assertTrue_1(initedReal.elementKnown(1));
  assertTrue_1(initedString.elementKnown(0));
  assertTrue_1(initedString.elementKnown(1));

  assertTrue_1(initedBool.getElement(0, btemp));
  assertTrue_1(btemp == bv[0]);
  assertTrue_1(initedBool.getElement(1, btemp));
  assertTrue_1(btemp == bv[1]);
  assertTrue_1(initedInt.getElement(0, itemp));
  assertTrue_1(itemp == iv[0]);
  assertTrue_1(initedInt.getElement(1, itemp));
  assertTrue_1(itemp == iv[1]);
  assertTrue_1(initedReal.getElement(0, dtemp));
  assertTrue_1(dtemp == dv[0]);
  assertTrue_1(initedReal.getElement(1, dtemp));
  assertTrue_1(dtemp == dv[1]);
  assertTrue_1(initedString.getElement(0, stemp));
  assertTrue_1(stemp == sv[0]);
  assertTrue_1(initedString.getElement(1, stemp));
  assertTrue_1(stemp == sv[1]);

  initedBool.resize(4);
  initedInt.resize(4);
  initedReal.resize(4);
  initedString.resize(4);

  // Check length
  assertTrue_1(initedBool.size()   == 4);
  assertTrue_1(initedInt.size()    == 4);
  assertTrue_1(initedReal.size()   == 4);
  assertTrue_1(initedString.size() == 4);

  // Check that previous contents were not disturbed
  assertTrue_1(initedBool.elementKnown(0));
  assertTrue_1(initedBool.elementKnown(1));
  assertTrue_1(initedInt.elementKnown(0));
  assertTrue_1(initedInt.elementKnown(1));
  assertTrue_1(initedReal.elementKnown(0));
  assertTrue_1(initedReal.elementKnown(1));
  assertTrue_1(initedString.elementKnown(0));
  assertTrue_1(initedString.elementKnown(1));

  assertTrue_1(initedBool.getElement(0, btemp));
  assertTrue_1(btemp == bv[0]);
  assertTrue_1(initedBool.getElement(1, btemp));
  assertTrue_1(btemp == bv[1]);
  assertTrue_1(initedInt.getElement(0, itemp));
  assertTrue_1(itemp == iv[0]);
  assertTrue_1(initedInt.getElement(1, itemp));
  assertTrue_1(itemp == iv[1]);
  assertTrue_1(initedReal.getElement(0, dtemp));
  assertTrue_1(dtemp == dv[0]);
  assertTrue_1(initedReal.getElement(1, dtemp));
  assertTrue_1(dtemp == dv[1]);
  assertTrue_1(initedString.getElement(0, stemp));
  assertTrue_1(stemp == sv[0]);
  assertTrue_1(initedString.getElement(1, stemp));
  assertTrue_1(stemp == sv[1]);

  // Check that new contents are unknown
  assertTrue_1(!initedBool.elementKnown(2));
  assertTrue_1(!initedBool.elementKnown(3));
  assertTrue_1(!initedInt.elementKnown(2));
  assertTrue_1(!initedInt.elementKnown(3));
  assertTrue_1(!initedReal.elementKnown(2));
  assertTrue_1(!initedReal.elementKnown(3));
  assertTrue_1(!initedString.elementKnown(2));
  assertTrue_1(!initedString.elementKnown(3));

  assertTrue_1(!initedBool.getElement(2, btemp));
  assertTrue_1(!initedBool.getElement(3, btemp));
  assertTrue_1(!initedInt.getElement(2, itemp));
  assertTrue_1(!initedInt.getElement(3, itemp));
  assertTrue_1(!initedReal.getElement(2, dtemp));
  assertTrue_1(!initedReal.getElement(3, dtemp));
  assertTrue_1(!initedString.getElement(2, stemp));
  assertTrue_1(!initedString.getElement(3, stemp));

  // Resize back to 2
  initedBool.resize(2);
  initedInt.resize(2);
  initedReal.resize(2);
  initedString.resize(2);

  // Length should not change
  assertTrue_1(initedBool.size()   == 4);
  assertTrue_1(initedInt.size()    == 4);
  assertTrue_1(initedReal.size()   == 4);
  assertTrue_1(initedString.size() == 4);

  // Nor contents
  assertTrue_1(initedBool.elementKnown(0));
  assertTrue_1(initedBool.elementKnown(1));
  assertTrue_1(initedInt.elementKnown(0));
  assertTrue_1(initedInt.elementKnown(1));
  assertTrue_1(initedReal.elementKnown(0));
  assertTrue_1(initedReal.elementKnown(1));
  assertTrue_1(initedString.elementKnown(0));
  assertTrue_1(initedString.elementKnown(1));

  assertTrue_1(initedBool.getElement(0, btemp));
  assertTrue_1(btemp == bv[0]);
  assertTrue_1(initedBool.getElement(1, btemp));
  assertTrue_1(btemp == bv[1]);
  assertTrue_1(initedInt.getElement(0, itemp));
  assertTrue_1(itemp == iv[0]);
  assertTrue_1(initedInt.getElement(1, itemp));
  assertTrue_1(itemp == iv[1]);
  assertTrue_1(initedReal.getElement(0, dtemp));
  assertTrue_1(dtemp == dv[0]);
  assertTrue_1(initedReal.getElement(1, dtemp));
  assertTrue_1(dtemp == dv[1]);
  assertTrue_1(initedString.getElement(0, stemp));
  assertTrue_1(stemp == sv[0]);
  assertTrue_1(initedString.getElement(1, stemp));
  assertTrue_1(stemp == sv[1]);

  assertTrue_1(!initedBool.elementKnown(2));
  assertTrue_1(!initedBool.elementKnown(3));
  assertTrue_1(!initedInt.elementKnown(2));
  assertTrue_1(!initedInt.elementKnown(3));
  assertTrue_1(!initedReal.elementKnown(2));
  assertTrue_1(!initedReal.elementKnown(3));
  assertTrue_1(!initedString.elementKnown(2));
  assertTrue_1(!initedString.elementKnown(3));

  assertTrue_1(!initedBool.getElement(2, btemp));
  assertTrue_1(!initedBool.getElement(3, btemp));
  assertTrue_1(!initedInt.getElement(2, itemp));
  assertTrue_1(!initedInt.getElement(3, itemp));
  assertTrue_1(!initedReal.getElement(2, dtemp));
  assertTrue_1(!initedReal.getElement(3, dtemp));
  assertTrue_1(!initedString.getElement(2, stemp));
  assertTrue_1(!initedString.getElement(3, stemp));

  return true;
}

static bool testSetters()
{
  // Sized but uninitialized
  Array<bool>        sizedBool(2);
  Array<int32_t>     sizedInt(2);
  Array<double>      sizedReal(2);
  Array<std::string> sizedString(2);

  assertTrue_1(sizedBool.size()   == 2);
  assertTrue_1(sizedInt.size()    == 2);
  assertTrue_1(sizedReal.size()   == 2);
  assertTrue_1(sizedString.size() == 2);

  assertTrue_1(!sizedBool.elementKnown(0));
  assertTrue_1(!sizedBool.elementKnown(1));
  assertTrue_1(!sizedInt.elementKnown(0));
  assertTrue_1(!sizedInt.elementKnown(1));
  assertTrue_1(!sizedReal.elementKnown(0));
  assertTrue_1(!sizedReal.elementKnown(1));
  assertTrue_1(!sizedString.elementKnown(0));
  assertTrue_1(!sizedString.elementKnown(1));

  bool btemp;
  int32_t itemp;
  double dtemp;
  std::string stemp;

  assertTrue_1(!sizedBool.getElement(0, btemp));
  assertTrue_1(!sizedBool.getElement(1, btemp));
  assertTrue_1(!sizedInt.getElement(0, itemp));
  assertTrue_1(!sizedInt.getElement(1, itemp));
  assertTrue_1(!sizedReal.getElement(0, dtemp));
  assertTrue_1(!sizedReal.getElement(1, dtemp));
  assertTrue_1(!sizedString.getElement(0, stemp));
  assertTrue_1(!sizedString.getElement(1, stemp));

  bool bval = true;
  int32_t ival = 69;
  double dval = 2.718;
  std::string sval("yahoo!");

  sizedBool.setElement(0, bval);
  sizedInt.setElement(0, ival);
  sizedReal.setElement(0, dval);
  sizedString.setElement(0, sval);

  assertTrue_1(sizedBool.size()   == 2);
  assertTrue_1(sizedInt.size()    == 2);
  assertTrue_1(sizedReal.size()   == 2);
  assertTrue_1(sizedString.size() == 2);

  assertTrue_1(sizedBool.elementKnown(0));
  assertTrue_1(!sizedBool.elementKnown(1));
  assertTrue_1(sizedInt.elementKnown(0));
  assertTrue_1(!sizedInt.elementKnown(1));
  assertTrue_1(sizedReal.elementKnown(0));
  assertTrue_1(!sizedReal.elementKnown(1));
  assertTrue_1(sizedString.elementKnown(0));
  assertTrue_1(!sizedString.elementKnown(1));

  assertTrue_1(sizedBool.getElement(0, btemp));
  assertTrue_1(btemp == bval);
  assertTrue_1(!sizedBool.getElement(1, btemp));
  assertTrue_1(sizedInt.getElement(0, itemp));
  assertTrue_1(itemp == ival);
  assertTrue_1(!sizedInt.getElement(1, itemp));
  assertTrue_1(sizedReal.getElement(0, dtemp));
  assertTrue_1(dtemp == dval);
  assertTrue_1(!sizedReal.getElement(1, dtemp));
  assertTrue_1(sizedString.getElement(0, stemp));
  assertTrue_1(stemp == sval);
  assertTrue_1(!sizedString.getElement(1, stemp));

  sizedBool.setElement(1, bval);
  sizedInt.setElement(1, ival);
  sizedReal.setElement(1, dval);
  sizedString.setElement(1, sval);

  assertTrue_1(sizedBool.size()   == 2);
  assertTrue_1(sizedInt.size()    == 2);
  assertTrue_1(sizedReal.size()   == 2);
  assertTrue_1(sizedString.size() == 2);

  assertTrue_1(sizedBool.elementKnown(0));
  assertTrue_1(sizedBool.elementKnown(1));
  assertTrue_1(sizedInt.elementKnown(0));
  assertTrue_1(sizedInt.elementKnown(1));
  assertTrue_1(sizedReal.elementKnown(0));
  assertTrue_1(sizedReal.elementKnown(1));
  assertTrue_1(sizedString.elementKnown(0));
  assertTrue_1(sizedString.elementKnown(1));

  assertTrue_1(sizedBool.getElement(0, btemp));
  assertTrue_1(btemp == bval);
  assertTrue_1(sizedBool.getElement(1, btemp));
  assertTrue_1(btemp == bval);
  assertTrue_1(sizedInt.getElement(0, itemp));
  assertTrue_1(itemp == ival);
  assertTrue_1(sizedInt.getElement(1, itemp));
  assertTrue_1(itemp == ival);
  assertTrue_1(sizedReal.getElement(0, dtemp));
  assertTrue_1(dtemp == dval);
  assertTrue_1(sizedReal.getElement(1, dtemp));
  assertTrue_1(dtemp == dval);
  assertTrue_1(sizedString.getElement(0, stemp));
  assertTrue_1(stemp == sval);
  assertTrue_1(sizedString.getElement(1, stemp));
  assertTrue_1(stemp == sval);

  sizedBool.setElementUnknown(0);
  sizedInt.setElementUnknown(0);
  sizedReal.setElementUnknown(0);
  sizedString.setElementUnknown(0);

  assertTrue_1(sizedBool.size()   == 2);
  assertTrue_1(sizedInt.size()    == 2);
  assertTrue_1(sizedReal.size()   == 2);
  assertTrue_1(sizedString.size() == 2);

  assertTrue_1(!sizedBool.elementKnown(0));
  assertTrue_1(sizedBool.elementKnown(1));
  assertTrue_1(!sizedInt.elementKnown(0));
  assertTrue_1(sizedInt.elementKnown(1));
  assertTrue_1(!sizedReal.elementKnown(0));
  assertTrue_1(sizedReal.elementKnown(1));
  assertTrue_1(!sizedString.elementKnown(0));
  assertTrue_1(sizedString.elementKnown(1));

  assertTrue_1(!sizedBool.getElement(0, btemp));
  assertTrue_1(sizedBool.getElement(1, btemp));
  assertTrue_1(btemp == bval);
  assertTrue_1(!sizedInt.getElement(0, itemp));
  assertTrue_1(sizedInt.getElement(1, itemp));
  assertTrue_1(itemp == ival);
  assertTrue_1(!sizedReal.getElement(0, dtemp));
  assertTrue_1(sizedReal.getElement(1, dtemp));
  assertTrue_1(dtemp == dval);
  assertTrue_1(!sizedString.getElement(0, stemp));
  assertTrue_1(sizedString.getElement(1, stemp));
  assertTrue_1(stemp == sval);

  sizedBool.setElementUnknown(1);
  sizedInt.setElementUnknown(1);
  sizedReal.setElementUnknown(1);
  sizedString.setElementUnknown(1);

  assertTrue_1(sizedBool.size()   == 2);
  assertTrue_1(sizedInt.size()    == 2);
  assertTrue_1(sizedReal.size()   == 2);
  assertTrue_1(sizedString.size() == 2);

  assertTrue_1(!sizedBool.elementKnown(0));
  assertTrue_1(!sizedBool.elementKnown(1));
  assertTrue_1(!sizedInt.elementKnown(0));
  assertTrue_1(!sizedInt.elementKnown(1));
  assertTrue_1(!sizedReal.elementKnown(0));
  assertTrue_1(!sizedReal.elementKnown(1));
  assertTrue_1(!sizedString.elementKnown(0));
  assertTrue_1(!sizedString.elementKnown(1));

  assertTrue_1(!sizedBool.getElement(0, btemp));
  assertTrue_1(!sizedBool.getElement(1, btemp));
  assertTrue_1(!sizedInt.getElement(0, itemp));
  assertTrue_1(!sizedInt.getElement(1, itemp));
  assertTrue_1(!sizedReal.getElement(0, dtemp));
  assertTrue_1(!sizedReal.getElement(1, dtemp));
  assertTrue_1(!sizedString.getElement(0, stemp));
  assertTrue_1(!sizedString.getElement(1, stemp));

  return true;
}

bool arrayTest()
{
  runTest(testConstructors);
  runTest(testResize);
  runTest(testSetters);
  return true;
}

