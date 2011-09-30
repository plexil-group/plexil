/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software.

/**
   @file util-test-module.cc
   @author Will Edgington

   @brief A small test of classes Error and TestData and the related
   macros.
   
   CMG: Added test for id's and entities
*/

#include "util-test-module.hh"
#include "ConstantMacros.hh"
#include "Error.hh"
#include "Debug.hh"
#include "LabelStr.hh"
#include "TestData.hh"
#include "Id.hh"
#include "XMLUtils.hh"
#include "StoredArray.hh"

#include <list>
#include <sstream>
#include <iostream>
#include <fstream>
#include <typeinfo>
//#include <cmath>
#include <cfloat>

#if S950
// apparently needed for sys950lib
#include <types/vxTypesOld.h>
#include <sys950Lib.h>
#endif

#ifndef PLEXIL_FAST
#define non_fast_only_assert(T) assertTrue(T)
#else
#define non_fast_only_assert(T) //NO-OP
#endif

#define runTest(test) { \
  try { \
    std::cout << "      " << #test; \
    bool result = test(); \
    if (result) \
      std::cout << " PASSED." << std::endl; \
    else { \
      std::cout << " FAILED TO PASS UNIT TEST." << std::endl; \
      throw Error::GeneralUnknownError(); \
    } \
  } \
  catch (Error &err) { \
    err.print(std::cout); \
  } \
}

#define runTestSuite(test) { \
  try{ \
  std::cout << #test << "***************" << std::endl; \
  if (test()) \
    std::cout << #test << " PASSED." << std::endl; \
  else \
    std::cout << #test << " FAILED." << std::endl; \
  }\
  catch (Error &err){\
   err.print(std::cout);\
  }\
  }

using namespace PLEXIL;

class TestError {
public:
  DECLARE_STATIC_CLASS_CONST(char*, TEST_CONST, "TestData");
  DECLARE_ERROR(BadThing);
};

class ErrorTest {
public:
  static bool test() {
    runTest(testExceptions);
    return true;
  }
private:
  static bool testExceptions() {
    assertTrue(strcmp(TestError::TEST_CONST(), "TestData") == 0);
    bool success = true;
    Error::doThrowExceptions();
    int var = 1;
    assertTrue(var == 1);
    assertTrue(Error::printingErrors());
    assertTrue(Error::displayWarnings());
    assertTrue(Error::throwEnabled());
    try {
      // These are tests of check_error() and should therefore not be changed
      //   to assertTrue() despite the usual rule for test programs.
      // --wedgingt@email.arc.nasa.gov 2005 Feb 9
      check_error(Error::printingErrors(), "not printing errors by default!");
      check_error(Error::displayWarnings(), "display warnings off by default!");
      check_error(var == 1);
      check_error(var == 1, "check_error(var == 1)");
      check_error(var == 1, Error("check_error(var == 1)"));
      checkError(var ==1, "Can add " << 1.09 << " and " << 2.81 << " to get " << 1.09 +2.81);
      condWarning(var == 1, "var is not 1");
      std::cout << std::endl;
      Error::setStream(std::cout);
      warn("Warning messages working");
      Error::setStream(std::cerr);
    } 
    catch (Error &e) {
      __x__(e);
      success = false;
    }
    // check_error will not throw the errors for PLEXIL_FAST
#if !defined(PLEXIL_FAST) && !defined(__CYGWIN__)
    assertTrue(Error::throwEnabled());
    /* Do not print errors that we are provoking on purpose to ensure they are noticed. */
    try {
      Error::doNotDisplayErrors();
      check_error(var == 2);
      __y__("check_error(var == 2) did not throw an exception");
      success = false;
    } 
    catch (Error &e) {
      Error::doDisplayErrors();
      __z__(e, Error("var == 2", __FILE__, __LINE__ - 5), success);
    }
    try {
      Error::doNotDisplayErrors();
      check_error(var == 2, "check_error(var == 2)");
      __y__("check_error(var == 2, blah) did not throw an exception");
      success = false;
    } 
    catch (Error &e) {
      Error::doDisplayErrors();
      __z__(e, Error("var == 2", "check_error(var == 2)", __FILE__, __LINE__ - 5), success);
    }
    try {
      Error::doNotDisplayErrors();
      check_error(var == 2, Error("check_error(var == 2)"));
      __y__("check_error(var == 2, Error(blah)) did not throw an exception");
      success = false;
    } 
    catch (Error &e) {
      Error::doDisplayErrors();
      __z__(e, Error("var == 2", "check_error(var == 2)", __FILE__, __LINE__ - 5), success);
    }
    try {
      Error::doNotDisplayErrors();
      check_error(var == 2, "check_error(var == 2)", TestError::BadThing());
      __y__("check_error(var == 2, TestError::BadThing()) did not throw an exception");
      success = false;
    }
    catch (Error &e) {
      Error::doDisplayErrors();
      //!!Should, perhaps, be:
      //__z__(e, Error(TestError::BadThing(), __FILE__, __LINE__ - 7), success);
      // ... but is actually:
      __z__(e, Error("var == 2", "check_error(var == 2)", __FILE__, __LINE__ - 9), success);
    }
#endif
    return(success);
  }
};

class DebugTest {
public:
  static bool test() {
    runTest(testDebugError);
    runTest(testDebugFiles);
    return true;
  }
private:

  static bool testDebugError() {
    bool success = true;
    // check_error will not throw the errors for PLEXIL_FAST
#if !defined(PLEXIL_FAST) && defined(DEBUG_MESSAGE_SUPPORT)
    Error::doThrowExceptions();
    assertTrue(Error::throwEnabled());
    //!!Add a test of DebugMessage that should throw an error here.
    //!!  Skipped for lack of time presently. --wedgingt@email.arc.nasa.gov
    Error::doNotThrowExceptions();
    assertTrue(!Error::throwEnabled());
#endif
    return(success);
  }

  static bool testDebugFiles() {
    for (int i = 1; i < 7; i++)
      runDebugTest(i);
    return(true);
  }

  static void runDebugTest(int cfgNum) {
#if !defined(PLEXIL_FAST) && defined(DEBUG_MESSAGE_SUPPORT)
    std::stringstream cfgName;
    cfgName << "debug" << cfgNum << ".cfg";
    std::string cfgFile(cfgName.str());
    cfgName << ".output";
    std::string cfgOut(cfgName.str());

    Error::doNotThrowExceptions();
    Error::doNotDisplayErrors();
    std::ofstream debugOutput(cfgOut.c_str());
    assertTrue(debugOutput.good(), "could not open debug output file");
    DebugMessage::setStream(debugOutput);
    std::ifstream debugStream(cfgFile.c_str());
    assertTrue(debugStream.good(), "could not open debug config file",
                DebugErr::DebugConfigError());
    if (!DebugMessage::readConfigFile(debugStream))
      handle_error(!DebugMessage::readConfigFile(debugStream),
                   "problems reading debug config file",
                   DebugErr::DebugConfigError());
    
    debugMsg("main1", "done opening files");
    condDebugMsg(std::cout.good(), "main1a", "std::cout is good");
    debugStmt("main2a", int s = 0; for (int i = 0; i < 5; i++) { s += i; } debugOutput << "Sum is " << s << '\n'; );
    debugMsg("main2", "primary testing done");
    Error::doThrowExceptions();
    Error::doDisplayErrors();
    DebugMessage::setStream(std::cerr);
#endif
  }
};

class MutexTest
{
public:
  static bool test()
  {
    runTest(testGuard);
    return true;
  }

  static bool testGuard()
  {
    bool result = true;
    ThreadMutex m;
    try {
      ThreadMutexGuard mg(m);
      Error::doThrowExceptions();
      assertTrue(0 == 1, "This assertion is supposed to fail");
      std::cout << "ERROR: Failed to throw exception" << std::endl;
      result = false;
    }
    catch (Error& e) {
      std::cout << "Caught expected exception" << std::endl;
      if (m.trylock())
	result = result && true;
      else {
	std::cout << "Throwing failed to run guard destructor" << std::endl;
	result = false;
      }
      m.unlock();
    }
    return result;
  }

};

/**
 * Support classes to enable testing
 * Foo: Basic allocation and deallocation.
 * Bar: Casting behaviour
 * EmbeddedClass: Error handling and checking for release and alloction patterns with embedded id's.
 */
class Root {
public:
  Root() {
  }

  virtual ~Root() {
  }
};

class Foo: public Root {
public:
  Foo() {
    counter++;
  }

  virtual ~Foo() {
    counter--;
  }

  void increment() {
    counter++;
  }

  void decrement() {
    counter--;
  }

  bool doConstFunc() const {
    return(true);
  }

  static int getCount() {
    return(counter);
  }

private:
  static int counter;
};

int Foo::counter(0);

class Bar: public Foo {
public:
  Bar() {
  }
};

class Baz: public Foo
{
public:
  Baz(){
  }
};

class Bing: public Root 
{
public:
  Bing(){
  }
};

class Poot : public virtual Root
{
public:
  Poot() {} 
};

class Doot : public virtual Root
{
public:
  Doot() {} 
};

class PootDoot : public Poot, public Doot
{
public:
  PootDoot() {}
};

class DootPoot : public Doot, public Poot
{
public:
  DootPoot() {}
};

void overloadFunc(const Id<Bing>& /* arg */) {
  assertTrue(true);
}

void overloadFunc(const Id<Foo>& /* arg */) {
  assertTrue(true);
}

class IdTests {
public:
  static bool test();

private:
  static bool testBasicAllocation();
  static bool testTypicalConversionsAndComparisons();
  static bool testCollectionSupport();
  static bool testDoubleConversion();
  static bool testCastingSupport();
  static bool testBadAllocationErrorHandling();
  static bool testVirtualInheritance();
  static bool testBadIdUsage();
  static bool testIdConversion();
  static bool testConstId();
};

bool IdTests::test() {
  //LockManager::instance().lock();
  runTest(testBasicAllocation);
  runTest(testCollectionSupport);
  runTest(testDoubleConversion);
  runTest(testCastingSupport);
  runTest(testTypicalConversionsAndComparisons);
  runTest(testBadAllocationErrorHandling);
  runTest(testVirtualInheritance);
  runTest(testBadIdUsage);
  runTest(testIdConversion);
  runTest(testConstId);
  //LockManager::instance().unlock();
  return(true);
}

bool IdTests::testBasicAllocation() {
#ifndef PLEXIL_FAST
  unsigned int initialSize = IdTable::size();
#endif
  Foo *fooPtr = new Foo();
  Id<Foo> fId1(fooPtr);
  assert(fId1.isId());
  assertTrue(Foo::getCount() == 1);
  non_fast_only_assert(IdTable::size() == initialSize + 1);

  fId1->increment();
  assertTrue(Foo::getCount() == 2);
  fId1->decrement();
  assertTrue(Foo::getCount() == 1);

  Id<Foo> fId2 = fId1;
  assertTrue(Foo::getCount() == 1);

  assertTrue(fId1.isValid() && fId2.isValid());
  assertTrue(!fId1.isInvalid() && !fId2.isInvalid());

  fId2.release();
  assertTrue(Foo::getCount() == 0);
  non_fast_only_assert( fId1.isInvalid() &&  fId2.isInvalid());
  return true;
}


bool IdTests::testTypicalConversionsAndComparisons()
{
  Foo* foo1 = new Foo();
  Id<Foo> fId1(foo1);
  Id<Foo> fId2(fId1);
  assertTrue(fId1 == fId2); // Equality operator
  assertTrue(&*fId1 == &*fId2); // Dereferencing operator
  assertTrue(foo1 == &*fId2); // Dereferencing operator
  assertTrue(foo1 == (Foo*) fId2); // Dereferencing operator
  assertTrue(foo1 == fId2.operator->());
  assertTrue( ! (fId1 > fId2));
  assertTrue( ! (fId1 < fId2));

  Foo* foo2 = new Foo();
  Id<Foo> fId3(foo2);
  assertTrue(fId1 != fId3);

  fId1.release();
  fId3.release();
  return true;
}

bool IdTests::testCollectionSupport()
{
  // Test inclusion in a collection class - forces compilation test
  std::list< Id<Foo> > fooList;
  assertTrue(fooList.size() == 0);
  return true;
}

bool IdTests::testDoubleConversion()
{
  Id<Foo> fId(new Foo());
  double fooAsDouble = (double) fId;
  Id<Foo> idFromDbl(fooAsDouble);
  assertTrue(idFromDbl == fId);
  fId.release();
  return true;
}

bool IdTests::testCastingSupport()
{
  Foo* foo = new Foo();
  Id<Foo> fId(foo);
  Foo* fooByCast = (Foo*) fId;
  assertTrue(foo == fooByCast);

  assertTrue(Id<Bar>::convertable(fId) == false);
  fId.release();

  Foo* bar = new Bar();
  Id<Bar> bId((Bar*) bar);
  fId = bId;
  assertTrue(Id<Bar>::convertable(fId) == true);
  bId.release();

  bId = Id<Bar>(new Bar());
  double ptrAsDouble = bId; // Cast to double

  const Id<Bar>& cbId(ptrAsDouble);
  assertTrue(cbId.isValid());
  assertTrue(cbId == bId);
  bId.release();
  non_fast_only_assert(cbId.isInvalid());

  Id<Baz> fId1(new Baz());
  // DOES NOT COMPILE: overloadFunc(fId1);
  fId1.release();
  return true;
}

bool IdTests::testBadAllocationErrorHandling()
{
  std::cout << std::endl;
  bool success = true;
  // check_error (inside class Id) will not throw the errors when compiled with PLEXIL_FAST.
#ifndef PLEXIL_FAST
  // Ensure allocation of a null pointer triggers error
  //LabelStr expectedError = IdErr::IdMgrInvalidItemPtrError();
  Error::doThrowExceptions();
#if !defined(__CYGWIN__)
  // This exception simply isn't being caught on Cygwin for some reason.
  try {
    Error::doNotDisplayErrors();
    Id<Foo> fId0((Foo*) 0);
    assertTrue(false, "Id<Foo> fId0((Foo*) 0); failed to error out.");
    success = false;
  }
  catch (Error &e) {
    Error::doDisplayErrors();
    // Path of Id.hh may vary depending on where test is run from.
    // Match only the filename and not the full path
    std::string pathMsg = e.getFile();
    int end = pathMsg.length();
    std::string name = "Id.hh";
    int start = pathMsg.find(name);
    if (start >= 0) {
      std::string fileMsg = pathMsg.substr(start, end);
      e.setFile(fileMsg);
    }
    __z__(e, Error("ptr != 0", "Cannot generate an Id<3Foo> for 0 pointer.", "Id.hh", 0), success);
  }
#endif
  Error::doNotThrowExceptions();

  Foo* foo = new Foo();
  Id<Foo> fId1(foo);
  fId1.remove();
  Id<Foo> fId3(foo);
  fId3.release();
#endif

  return(success);
}

bool IdTests::testVirtualInheritance()
{
  bool success = true;

  PootDoot* pootdoot = new PootDoot();

  // ID of base 
  Id<Root> pootdootRoot(dynamic_cast<Root*>(pootdoot));
  assertTrue(pootdootRoot.isValid());

  // ID of supers
  Id<Poot> pootdootPoot(dynamic_cast<Poot*>(pootdoot), pootdootRoot);
  assertTrue(pootdootPoot.isValid());
  Id<Doot> pootdootDoot(dynamic_cast<Doot*>(pootdoot), pootdootRoot);
  assertTrue(pootdootDoot.isValid());

  // ID of derived class
  Id<PootDoot> pootdootPootDoot(pootdoot, pootdootRoot);
  assertTrue(pootdootPootDoot.isValid());

#ifndef PLEXIL_FAST
  // Check the checks
  std::cout << std::endl;

  // Basic allocation
  Error::doThrowExceptions();
  try {
    Error::doNotDisplayErrors();
    Id<PootDoot> pdId0((PootDoot*) 0, Id<Root>::noId());
    assertTrue(false, "Id<PootDoot> pdId0((PootDoot*) 0, Id<Root>::noId()); failed to error out.");
    success = false;
    Error::doDisplayErrors();
  }
  catch (Error& e) {
    Error::doDisplayErrors();
    // Path of Id.hh may vary depending on where test is run from.
    // Match only the filename and not the full path
    std::string pathMsg = e.getFile();
    int end = pathMsg.length();
    std::string name = "Id.hh";
    int start = pathMsg.find(name);
    if (start >= 0) {
      std::string fileMsg = pathMsg.substr(start, end);
      e.setFile(fileMsg);
    }
    __z__(e, Error("ptr != 0", "Cannot generate an Id<8PootDoot> for 0 pointer.", "Id.hh", 0), success);
  }

  // Invalid base Id
  try {
    Error::doNotDisplayErrors();
	Id<PootDoot> bogusBase(pootdoot, Id<Root>::noId());
	assertTrue(false, "Id<PootDoot> bogusBase(pootdoot, Id<Root>::noId()); failed to throw an error.");
	success = false;
	Error::doDisplayErrors();
  }
  catch (Error& e) {
    Error::doDisplayErrors();
    // Path of Id.hh may vary depending on where test is run from.
    // Match only the filename and not the full path
    std::string pathMsg = e.getFile();
    int end = pathMsg.length();
    std::string name = "Id.hh";
    int start = pathMsg.find(name);
    if (start >= 0) {
      std::string fileMsg = pathMsg.substr(start, end);
      e.setFile(fileMsg);
    }
    __z__(e, Error("baseId.isValid()", "Cannot generate an Id<8PootDoot> when Id of base class object is invalid.", "Id.hh", 0), success);
  }

  // Detection of duplicate Id
  // This can fail if the superclass pointer is equal to the derived pointer.
  try {
    Error::doNotDisplayErrors();
	Id<PootDoot> duplicateId(pootdoot, pootdootDoot);
	assertTrue(false, "Id<PootDoot> duplicateId(pootdoot, pootdootDoot); failed to throw an error.");
	success = false;
	Error::doDisplayErrors();
  }
  catch (Error& e) {
    Error::doDisplayErrors();
    // Path of Id.hh may vary depending on where test is run from.
    // Match only the filename and not the full path
    std::string pathMsg = e.getFile();
    int end = pathMsg.length();
    std::string name = "Id.hh";
    int start = pathMsg.find(name);
    if (start >= 0) {
      std::string fileMsg = pathMsg.substr(start, end);
      e.setFile(fileMsg);
    }
    __z__(e, Error("m_key != 0", "Cannot generate an Id<8PootDoot> for a pointer that has not been cleaned up.", "Id.hh", 0), success);
  }
  Error::doNotThrowExceptions();
  std::cout << std::endl;
#endif

  // Upcasts to root
  assertTrue(pootdootRoot == Id<Root>(pootdootPoot));
  assertTrue(pootdootRoot == Id<Root>(pootdootDoot));
  assertTrue(pootdootRoot == Id<Root>(pootdootPootDoot));

  // To supers
  assertTrue(pootdootPoot == Id<Poot>(pootdootPootDoot));
  assertTrue(pootdootDoot == Id<Doot>(pootdootPootDoot));

  // Downcasts

  // Base to supers
  assertTrue(pootdootPoot == Id<Poot>(pootdootRoot));
  assertTrue(pootdootDoot == Id<Doot>(pootdootRoot));

  // All supers to derived
  assertTrue(pootdootPootDoot == Id<PootDoot>(pootdootRoot));
  assertTrue(pootdootPootDoot == Id<PootDoot>(pootdootPoot));
  assertTrue(pootdootPootDoot == Id<PootDoot>(pootdootDoot));

  // Remove
  pootdootPootDoot.removeDerived(pootdootRoot);
  assertTrue(pootdootPootDoot.isNoId());
  pootdootDoot.removeDerived(pootdootRoot);
  assertTrue(pootdootDoot.isNoId());
  pootdootPoot.removeDerived(pootdootRoot);
  assertTrue(pootdootPoot.isNoId());

  // Release
  pootdoot = 0; // zero pointer here to be safe

  pootdootRoot.release();
  assertTrue(pootdootRoot.isNoId());

  return success;
}

bool IdTests::testBadIdUsage() {
  bool success = true;
  Id<Root> barId(new Bar());
  Error::doThrowExceptions();
#if !defined(__CYGWIN__)
  // This exception isn't being caught on Cygwin.
  try {
    Error::doNotDisplayErrors();
    Id<Bing> bingId = barId;
    assertTrue(false, "Id<Bing> bingId = barId; failed to error out.");
    success = false;
  }
  catch (Error &e) {
    Error::doDisplayErrors();
    if (e.getType() == "Error")
      assertTrue(false);
  }
  catch (IdErr &idErr) {
    Error::doDisplayErrors();
    std::cerr << "Caught expected IdErr::IdMgrInvalidItemPtrError" << std::endl;
    // No operator==() implemented ...
    // __z__(idErr, IdErr::IdMgrInvalidItemPtrError(), success);
  }
#endif
  Error::doNotThrowExceptions();
  barId.release();
  return(success);
}

bool IdTests::testIdConversion()
{
  int count = Foo::getCount();
  Id<Foo> fooId(new Bar());
  Id<Bar> barId(fooId);
  barId.release();
  Id<Foo> fooId3(new Bar());
  Id<Bar> barId3;
  barId3 = fooId3;
  barId3.release();
  assertTrue(Foo::getCount() == count);
  return true;
}

bool IdTests::testConstId()
{
  Id<Foo> fooId(new Foo());
  const Id<const Foo> constFooId(fooId);
  assertTrue(constFooId->doConstFunc());
  fooId->increment();
  fooId.remove();
  return true;
}

class LabelTests {
public:
  static bool test(){
    runTest(testBasicAllocation);
    runTest(testElementCounting);
    runTest(testElementAccess);
    runTest(testComparisons);
    return true;
  }

private:
  static bool compare(const LabelStr& str1, const LabelStr& str2){
    return str1 == str2;
  }

  static bool testBasicAllocation(){
    LabelStr lbl1("");
    LabelStr lbl2("This is a char*");
    LabelStr lbl3(lbl2.toString());
    assertTrue(lbl3 == lbl2);
    std::string labelStr2("This is another char*");
    assertFalse(LabelStr::isString(labelStr2));
    LabelStr lbl4(labelStr2);
    assertTrue(LabelStr::isString(labelStr2));
    assertTrue(lbl4 != lbl2, lbl4.toString() + " != " + lbl2.toString());

    double key = lbl2.getKey();
    LabelStr lbl5(key);
    assertTrue(lbl5 == lbl2);
    assertTrue(LabelStr::isString(key));
    assertFalse(LabelStr::isString(1));

    assertTrue(compare(lbl3, lbl2));
    assertTrue(compare("This is another char*", "This is another char*"));
    return true;
  }

  static bool testElementCounting(){
    LabelStr lbl1("A 1B 1C 1D EFGH");
    assertTrue(lbl1.countElements("1") == 4);
    assertTrue(lbl1.countElements(" ") == 5);
    assertTrue(lbl1.countElements("B") == 2);
    assertTrue(lbl1.countElements(":") == 1);

    LabelStr lbl2("A:B:C:D:");
    assertTrue(lbl2.countElements(":") == 4);
    return true;
  }

  static bool testElementAccess(){
    LabelStr lbl1("A 1B 1C 1D EFGH");
    LabelStr first(lbl1.getElement(0, " "));
    assertTrue(first == LabelStr("A"));

    LabelStr last(lbl1.getElement(3, "1"));
    assertTrue(last == LabelStr("D EFGH"));
    return true;
  }

  static bool testComparisons(){
    LabelStr lbl1("A");
    LabelStr lbl2("G");
    LabelStr lbl3("B");
    LabelStr lbl4("B");
    assertTrue(lbl1 < lbl2);
    assertTrue(lbl2 > lbl4);
    assertTrue(lbl2 != lbl4);
    assertTrue(lbl4 == lbl3);

    LabelStr lbl5("ABCDEFGH");

    assertTrue(lbl5.contains("A"));
    assertTrue(lbl5.contains("H"));
    assertTrue(lbl5.contains("FG"));
    assertTrue(lbl5.contains(lbl5));
    assertFalse(lbl5.contains("I"));
    return true;
  }
};

#define UNKNOWN DBL_MAX

class StoredArrayTests
{
   public:
      static bool test()
      {
         // this test is commented out as it raises and error when the
         // keyspace is exhausted and therefore does run cleanly

         runTest(testKeyspace);
         runTest(testBasics);
         runTest(testSpeed);
         runTest(testMemory);
         return true;
      }

      static bool testBasics()
      {
	StoredArray sa0;

         StoredArray sa1(10, UNKNOWN);
         sa1[0] = 3.3;
         sa1[1] = 9.9;

         StoredArray sa2(sa1.getKey());
         assert(sa2[0] == 3.3);
         assert(sa2[1] == 9.9);
         assert(sa2[2] == UNKNOWN);
         return true;
      }

      static bool testKeyspace()
      {
#define KEYSPACE_KEY_T uint8_t	
	size_t keySpace = KeySource<KEYSPACE_KEY_T>::totalKeys();
	std::cout << "key space: " << keySpace << std::endl;
	try
	  {
            std::cout << std::endl;
            Error::doThrowExceptions();
            for (size_t i = 0; i < keySpace + 1; ++i)
	      {
		double j = 7;
		StoredItem<KEYSPACE_KEY_T, double> x(j);
		// cut down on output a bit
		if ((i & 0xFF) == 0)
		  {
		    std::cout << "created key: " << (i + 1)
			      << " available: " << KeySource<KEYSPACE_KEY_T>::availableKeys()
			      << "\r" << std::flush;
		  }
	      }
	  }
	catch (Error &e)
	  {
            std::cout << "Caught expected exception: ";
            e.print(std::cout);
	    // cleanup
	    std::cout << "\nCleaning up...";
	    for (size_t i = KeySource<KEYSPACE_KEY_T>::keyMin(); i < KeySource<KEYSPACE_KEY_T>::keyMax(); ++i)
	      {
		if (!StoredItem<KEYSPACE_KEY_T, double>::isKey(i))
		  break;
		StoredItem<KEYSPACE_KEY_T, double>x(i);
		x.unregister();
	      }
            std::cout << " done." << std::endl;
            return true;
	  }

	// should never get here
	return false;
#undef KEYSPACE_KEY_T
      }

      
      static bool testMemory()
      {
         std::cout << std::endl;

         unsigned width = 1000;
         unsigned testSize = 100000; // was 1000000
         unsigned updateSize = 10000;
	 // preallocate the vector to the appropriate size
         std::vector<double> keys;
	 keys.reserve(testSize);

#ifdef STORED_ITEM_REUSE_KEYS
         size_t availableKeys = KeySource<double>::availableKeys();
#endif         
         // create AND unregister a whole bunch of StoredArray
         
         time_t startTotal = startTime();
         time_t start = startTime();
         for (unsigned i = 0; i < testSize; ++i)
         {
            StoredArray sa(width, i);

#ifdef STORED_ITEM_REUSE_KEYS
            assertTrue(KeySource<double>::availableKeys() == availableKeys - 1,
                   "availableKeys count mismatch");
#endif           
            if ((i + 1) % updateSize == 0)
               std::cout << "creating StoredArray: " << (i + 1)
                         << " key: " << sa.getKey() << "\r" << std::flush;
            
            keys.push_back(sa.getKey());
            sa.unregister();

#ifdef STORED_ITEM_REUSE_KEYS
            assertTrue(KeySource<double>::availableKeys() == availableKeys,
                   "availableKeys count mismatch");
#endif
         }
         std::cout << std::endl;
         stopTime(start);
         
         // check that all these keys are invalid
         
         start = startTime();
         for (unsigned i = 0; i < keys.size(); ++i)
         {
            if ((i + 1) % updateSize == 0)
               std::cout << "testing StoredArray: " 
                    << (i + 1) << "\r" << std::flush;
            assert(!StoredArray::isKey(keys[i]));
         }
         std::cout << std::endl;
         stopTime(start);
         stopTime(startTotal);
         return true;
      }

      
      static bool testSpeed()
      {
         std::cout << std::endl;

         unsigned width = 10;
         unsigned testSize = 100000; // was 2000000
         unsigned updateSize = 10000; // was 100000
	 // preallocate the vector to the appropriate size
         std::vector<double> keys;
	 keys.reserve(testSize);
         
         // create a whole bunch of StoredArray
         
         time_t startTotal = startTime();
         time_t start = startTime();
         for (unsigned i = 0; i < testSize; ++i)
         {
            if ((i + 1) % updateSize == 0)
               std::cout << "creating StoredArray: " << (i + 1) <<
                  "\r" << std::flush;
            
            StoredArray sa(width, i);
            keys.push_back(sa.getKey());
         }
         std::cout << std::endl;
         stopTime(start);
         
         // change the values of each of the vectors
         
         start = startTime();
         for (unsigned i = 0; i < keys.size(); ++i)
         {
            if ((i + 1) % updateSize == 0)
               std::cout << "changing elements in StoredArray: " 
                    << (i + 1) << "\r" << std::flush;
            
            StoredArray sa(keys[i]);
            for (unsigned j = 0; j < sa.size(); ++j)
               sa[j] += j;
         }
         std::cout << std::endl;
         stopTime(start);
         
         // test the values of each of the vectors
         
         start = startTime();
         for (unsigned i = 0; i < keys.size(); ++i)
         {
            if ((i + 1) % updateSize == 0)
               std::cout << "testing elements of StoredArray: " << 
                  (i + 1) << "\r" << std::flush;
            
            StoredArray sa(keys[i]);
            assertTrueMsg((StoredItem<double, ArrayStorage>::isKey(keys[i])),
			  "item key mismatch for index " << i);
            
            for (unsigned j = 0; j < sa.size(); ++j)
               assertTrueMsg(sa[j] == i + j, "value " << sa[j] << " != " << (i + j));
         }
         std::cout << std::endl;
         stopTime(start);

	 // delete everything
	 
         start = startTime();
         for (unsigned i = 0; i < keys.size(); ++i)
         {
	   if ((i + 1) % updateSize == 0)
	     std::cout << "deleting StoredArray: " << 
	       (i + 1) << "\r" << std::flush;
            
	   StoredArray sa(keys[i]);
	   sa.unregister();
	   keys[i] = 0.0;
         }
         std::cout << std::endl;
         stopTime(start);

         stopTime(startTotal);
         return true;
      }

      static time_t startTime()
      {
         std::cout << "timer started" << std::endl;
         return clock();
      }
      
      static double stopTime(time_t start)
      {
         time_t end = clock();
         double diff = (end - start) / (double)CLOCKS_PER_SEC;
         std::cout << "duration: " << diff << " seconds" << std::endl;
         return diff;
      }
};


void UtilModuleTests::runTests(std::string /* path */) 
{
  runTestSuite(ErrorTest::test);
  runTestSuite(DebugTest::test);
  runTestSuite(MutexTest::test);
  runTestSuite(IdTests::test);
  runTestSuite(StoredArrayTests::test);
  runTestSuite(LabelTests::test);

  std::cout << "Finished" << std::endl;
}
