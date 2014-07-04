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
#include "Debug.hh"
#include "Error.hh"
#include "Id.hh"
#include "iso-8601.hh"
#include "lifecycle-utils.h"
#include "stricmp.h"
#include "TestData.hh"
#include "timespec-utils.hh"
#include "timeval-utils.hh"
#include "XMLUtils.hh"

#include <cassert>
#include <cfloat>
#include <cstring> // for strcmp
#include <iomanip>
#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include <typeinfo>

#ifdef HAVE_SYS_TIME_H 
#include <sys/time.h>
#elif defined(__VXWORKS__)
#include <time.h>
#include <sys/times.h>
#include <sysLib.h> /* for sysClkRateGet() */
#endif

#if S950
// apparently needed for sys950lib
#include <types/vxTypesOld.h>
#include <sys950Lib.h>
#endif

// Utility used only in IdTests
#ifndef PLEXIL_ID_FAST
#define non_fast_only_assert(T) assertTrue(T)
#else
#define non_fast_only_assert(T) //NO-OP
#endif

/**
 * @def assertFalse
 * @brief Test a condition and create an error if true.
 * @param cond Expression that yields a true/false result.
 */
#define assertFalse(cond) { \
  if (cond) { \
    Error(#cond, __FILE__, __LINE__).handleAssert(); \
  } \
}

/**
 * @def handle_error
 * Create an error instance for throwing, printing, etc., per class Error.
 * @param cond Condition that failed (was false), implying an error has occurred.
 * @param msg String describing the error.
 * @param err A specific Error instance.
 */
#define handle_error(cond, msg, err) {         \
  Error(#cond, #msg, err, __FILE__, __LINE__);        \
}

#define runTest(test) { \
  try { \
    std::cout << "      " << #test; \
    bool result = test(); \
    if (result) \
      std::cout << " PASSED." << std::endl; \
    else { \
      std::cout << " UNIT TEST FAILED." << std::endl; \
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
    assertTrue_1(var == 1);
    assertTrue_1(Error::printingErrors());
    assertTrue_1(Error::displayWarnings());
    assertTrue_1(Error::throwEnabled());
    try {
      // These are tests of check_error() and should therefore not be changed
      //   to assertTrue() despite the usual rule for test programs.
      // --wedgingt@email.arc.nasa.gov 2005 Feb 9
      check_error(Error::printingErrors(), "not printing errors by default!");
      check_error(Error::displayWarnings(), "display warnings off by default!");
      check_error_2(Error::printingErrors(), "not printing errors by default!");
      check_error_2(Error::displayWarnings(), "display warnings off by default!");
      check_error(var == 1);
      check_error_1(var == 1);
      check_error(var == 1, "check_error(var == 1)");
      check_error(var == 1, Error("check_error(var == 1)"));
      check_error_2(var == 1, "check_error(var == 1)");
      check_error_2(var == 1, Error("check_error(var == 1)"));
      checkError(var ==1, "Can add " << 1.09 << " and " << 2.81 << " to get " << 1.09 +2.81);
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
#if !defined(PLEXIL_FAST) && !defined(NO_DEBUG_MESSAGE_SUPPORT)
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
#if !defined(PLEXIL_FAST) && !defined(NO_DEBUG_MESSAGE_SUPPORT)
    std::stringstream cfgName;
    cfgName << "debug" << cfgNum << ".cfg";
    std::string cfgFile(cfgName.str());
    cfgName << ".output";
    std::string cfgOut(cfgName.str());

    Error::doNotThrowExceptions();
    Error::doNotDisplayErrors();
    std::ofstream debugOutput(cfgOut.c_str());
    assertTrue(debugOutput.good(), "could not open debug output file");
    setDebugOutputStream(debugOutput);
    std::ifstream debugStream(cfgFile.c_str());
    assertTrue(debugStream.good(), "could not open debug config file",
                DebugErr::DebugConfigError());
    if (!readDebugConfigStream(debugStream))
      handle_error(!readDebugConfigStream(debugStream),
                   "problems reading debug config file",
                   DebugErr::DebugConfigError());
    
    debugMsg("main1", "done opening files");
    condDebugMsg(std::cout.good(), "main1a", "std::cout is good");
    debugStmt("main2a", int s = 0; for (int i = 0; i < 5; i++) { s += i; } debugOutput << "Sum is " << s << '\n'; );
    debugMsg("main2", "primary testing done");
    Error::doThrowExceptions();
    Error::doDisplayErrors();
    setDebugOutputStream(std::cerr);
#endif
  }
};

#ifdef PLEXIL_WITH_THREADS
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
#endif /* PLEXIL_WITH_THREADS */

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
  return(true);
}

bool IdTests::testBasicAllocation() {
#ifndef PLEXIL_ID_FAST
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
  assertTrue(fooList.empty());
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

  // Check the checks
  std::cout << std::endl;

#ifndef PLEXIL_FAST
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
#endif // PLEXIL_FAST

#ifndef PLEXIL_ID_FAST
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
    // Work around 'unused variable' warning
    assertTrue(bingId != bingId , "Id<Bing> bingId = barId; failed to error out.");
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
    __z__(e, Error("m_ptr != 0", "Invalid cast from Id<4Root> to Id<4Bing>.", "Id.hh", 0), success);
  }
#endif // !defined(__CYGWIN__)
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

class TimespecTests
{
public:
  static bool test()
  {
    runTest(testTimespecComparisons);
    runTest(testTimespecArithmetic);
    runTest(testTimespecConversions);
    return true;
  }

private:
  static bool testTimespecComparisons()
  {
    struct timespec a = {1, 0};
    struct timespec a1 = {1, 0};
    struct timespec b = {2, 0};
    struct timespec c = {1, 1};

    assertTrue(a < b, "Timespec operator< failed");
    assertTrue(a < c, "Timespec operator< failed");
    assertTrue(c < b, "Timespec operator< failed");
    assertTrue(!(b < a), "Timespec operator< failed");
    assertTrue(!(c < a), "Timespec operator< failed");
    assertTrue(!(b < c), "Timespec operator< failed");
    assertTrue(!(a1 < a), "Timespec operator< failed");
    assertTrue(!(a < a1), "Timespec operator< failed");

    assertTrue(b > a, "Timespec operator> failed");
    assertTrue(b > c, "Timespec operator> failed");
    assertTrue(c > a, "Timespec operator> failed");
    assertTrue(!(a > b), "Timespec operator> failed");
    assertTrue(!(a > c), "Timespec operator> failed");
    assertTrue(!(c > b), "Timespec operator> failed");
    assertTrue(!(a1 > a), "Timespec operator> failed");
    assertTrue(!(a > a1), "Timespec operator> failed");

    assertTrue(a == a, "Timespec operator== failed - identity");
    assertTrue(a == a1, "Timespec operator== failed - equality");
    assertTrue(!(a == b), "Timespec operator== failed - tv_sec");
    assertTrue(!(a == c), "Timespec operator== failed - tv_nsec");

    return true;
  }

  static bool testTimespecArithmetic()
  {
    struct timespec tsminus1 = {-1, 0};
    struct timespec ts0 = {0, 0};
    struct timespec ts1 = {1, 0};
    struct timespec ts1pt1 = {1, 1};
    struct timespec ts0pt9 = {0, 999999999};
    struct timespec ts2 = {2, 0};

    assertTrue(ts0 == ts0 + ts0, "Timespec operator+ failed - 0 + 0");
    assertTrue(ts1 == ts0 + ts1, "Timespec operator+ failed - 0 + 1");
    assertTrue(ts0 == ts1 + tsminus1, "Timespec operator+ failed - 1 + -1");
    assertTrue(ts0 == tsminus1 + ts1, "Timespec operator+ failed - -1 + 1");
    assertTrue(ts1pt1 == ts0 + ts1pt1, "Timespec operator+ failed - 0 + 1.000000001");
    assertTrue(ts1 == ts1 + ts0, "Timespec operator+ failed - 1 + 0");
    assertTrue(ts2 == ts1 + ts1, "Timespec operator+ failed - 1 + 1");
    assertTrue(ts2 == ts1pt1 + ts0pt9, "Timespec operator+ failed - 1.00000001 + 0.999999999");

    assertTrue(ts0 == ts0 - ts0, "Timespec operator- failed - 0 - 0");
    assertTrue(ts0 == ts1 - ts1, "Timespec operator- failed - 1 - 1");
    assertTrue(ts0 == tsminus1 - tsminus1, "Timespec operator- failed - -1 - -1");
    assertTrue(ts1 == ts1 - ts0, "Timespec operator- failed - 1 - 0");
    assertTrue(tsminus1 == ts0 - ts1, "Timespec operator- failed - 0 - 1");
    assertTrue(ts1 == ts0 - tsminus1, "Timespec operator- failed - 0 - -1");
    assertTrue(ts1pt1 == ts2 - ts0pt9, "Timespec operator- failed - 2 - 0.999999999");
    assertTrue(ts0pt9 == ts2 - ts1pt1, "Timespec operator- failed - 2 - 1.000000001");

    return true;
  }

  static bool testTimespecConversions()
  {
    return true;
  }

};

class TimevalTests
{
public:
  static bool test()
  {
    runTest(testTimevalComparisons);
    runTest(testTimevalArithmetic);
    runTest(testTimevalConversions);
    return true;
  }

private:
  static bool testTimevalComparisons()
  {
    struct timeval a = {1, 0};
    struct timeval a1 = {1, 0};
    struct timeval b = {2, 0};
    struct timeval c = {1, 1};

    assertTrue(a < b, "Timeval operator< failed");
    assertTrue(a < c, "Timeval operator< failed");
    assertTrue(c < b, "Timeval operator< failed");
    assertTrue(!(b < a), "Timeval operator< failed");
    assertTrue(!(c < a), "Timeval operator< failed");
    assertTrue(!(b < c), "Timeval operator< failed");
    assertTrue(!(a1 < a), "Timeval operator< failed");
    assertTrue(!(a < a1), "Timeval operator< failed");

    assertTrue(b > a, "Timeval operator> failed");
    assertTrue(b > c, "Timeval operator> failed");
    assertTrue(c > a, "Timeval operator> failed");
    assertTrue(!(a > b), "Timeval operator> failed");
    assertTrue(!(a > c), "Timeval operator> failed");
    assertTrue(!(c > b), "Timeval operator> failed");
    assertTrue(!(a1 > a), "Timeval operator> failed");
    assertTrue(!(a > a1), "Timeval operator> failed");

    assertTrue(a == a, "Timeval operator== failed - identity");
    assertTrue(a == a1, "Timeval operator== failed - equality");
    assertTrue(!(a == b), "Timeval operator== failed - tv_sec");
    assertTrue(!(a == c), "Timeval operator== failed - tv_nsec");

    return true;
  }

  static bool testTimevalArithmetic()
  {
    struct timeval tsminus1 = {-1, 0};
    struct timeval ts0 = {0, 0};
    struct timeval ts1 = {1, 0};
    struct timeval ts1pt1 = {1, 1};
    struct timeval ts0pt9 = {0, 999999};
    struct timeval ts2 = {2, 0};

    assertTrue(ts0 == ts0 + ts0, "Timeval operator+ failed - 0 + 0");
    assertTrue(ts1 == ts0 + ts1, "Timeval operator+ failed - 0 + 1");
    assertTrue(ts0 == ts1 + tsminus1, "Timeval operator+ failed - 1 + -1");
    assertTrue(ts0 == tsminus1 + ts1, "Timeval operator+ failed - -1 + 1");
    assertTrue(ts1pt1 == ts0 + ts1pt1, "Timeval operator+ failed - 0 + 1.000001");
    assertTrue(ts1 == ts1 + ts0, "Timeval operator+ failed - 1 + 0");
    assertTrue(ts2 == ts1 + ts1, "Timeval operator+ failed - 1 + 1");
    assertTrue(ts2 == ts1pt1 + ts0pt9, "Timeval operator+ failed - 1.000001 + 0.999999");

    assertTrue(ts0 == ts0 - ts0, "Timeval operator- failed - 0 - 0");
    assertTrue(ts0 == ts1 - ts1, "Timeval operator- failed - 1 - 1");
    assertTrue(ts0 == tsminus1 - tsminus1, "Timeval operator- failed - -1 - -1");
    assertTrue(ts1 == ts1 - ts0, "Timeval operator- failed - 1 - 0");
    assertTrue(tsminus1 == ts0 - ts1, "Timeval operator- failed - 0 - 1");
    assertTrue(ts1 == ts0 - tsminus1, "Timeval operator- failed - 0 - -1");
    assertTrue(ts1pt1 == ts2 - ts0pt9, "Timeval operator- failed - 2 - 0.999999");
    assertTrue(ts0pt9 == ts2 - ts1pt1, "Timeval operator- failed - 2 - 1.000001");

    return true;
  }

  static bool testTimevalConversions()
  {
    return true;
  }

};

class ISO8601Tests
{
public:
  static bool test()
  {
    runTest(testPrinting);
#if !defined(__VXWORKS__)
    runTest(testGMTPrinting);
#endif
    runTest(testLocalParsing);
    runTest(testGMTParsing);
    runTest(testOffsetParsing);
    runTest(testCompleteDurationParsing);
    runTest(testAlternativeBasicDurationParsing);
    runTest(testAlternativeExtendedDurationParsing);
    runTest(testDurationPrinting);
    return true;
  }

  static bool testLocalParsing()
  {
    const char* localDate1 = "2012-09-17T16:00:00";
    double localTime1 = 0;
    assertTrue(parseISO8601Date(localDate1, localTime1)
               || localTime1 != 0,
               "Basic date parsing failed");

    std::ostringstream str1;
    printISO8601Date(localTime1, str1);
    assertTrueMsg(0 == strcmp(localDate1, str1.str().c_str()),
                  "Date " << localDate1 << " printed as " << str1.str());

    return true;
  }

  static bool testGMTParsing()
  {
    // Not working on Mac OS X!
    // const char* zuluDate0 = "1970-01-01T00:00:00Z";
    // double zuluTime0 = 0;
    // assertTrueMsg(parseISO8601Date(zuluDate0, zuluTime0)
    //               && zuluTime0 == 0,
    //               "GMT date parsing failed at epoch, returned " << zuluTime0);

    const char* zuluDate1 = "2012-09-17T16:00:00Z";
    double zuluTime1 = 0;
    assertTrueMsg(parseISO8601Date(zuluDate1, zuluTime1)
                  && zuluTime1 != 0,
                  "GMT date parsing failed");

    std::ostringstream str2;
    printISO8601DateUTC(zuluTime1, str2);
    assertTrueMsg(0 == strcmp(zuluDate1, str2.str().c_str()),
                  "Date " << zuluDate1 << " printed as " << str2.str());

    return true;
  }

  static bool testOffsetParsing()
  {
    const char* relDate1 = "2012-09-17T16:00:00+04:00";
    double relTime1 = 0;
    assertTrue(parseISO8601Date(relDate1, relTime1)
               || relTime1 != 0,
               "Offset date parsing failed");

    std::ostringstream str3;
    printISO8601DateUTC(relTime1, str3);
    assertTrueMsg(0 == strcmp("2012-09-17T20:00:00Z", str3.str().c_str()),
                  "Date " << relDate1 << " printed as " << str3.str());

    return true;
  }

  // convenience function
  static void tm_init(struct tm& the_tm,
                      int year,
                      int month,
                      int dayOfMonth,
                      int hour,
                      int min, 
                      int sec,
                      int dstFlag)
  {
    the_tm.tm_year = year - 1900;
    the_tm.tm_mon = month;
    the_tm.tm_mday = dayOfMonth;
    the_tm.tm_hour = hour;
    the_tm.tm_min = min;
    the_tm.tm_sec = sec;
    the_tm.tm_isdst = dstFlag;
  }

  static bool testPrinting()
  {
    // take a date, convert it to time_t, then to double, print it
    struct tm tm1;
    tm_init(tm1, 2012, 6, 16, 5, 30, 0, 1);
    time_t date1 = mktime(&tm1);
    std::ostringstream sstr1;
    printISO8601Date((double) date1, sstr1);
    assertTrue(sstr1.str() == "2012-06-16T05:30:00", 
               "Date printing error");

    std::ostringstream sstr2;
    printISO8601Date(0.5 + (double) date1, sstr2);
    assertTrue(sstr2.str() == "2012-06-16T05:30:00.500", 
               "Date printing error - fractional seconds");

    return true;
  }

#if !defined(__VXWORKS__) /* timegm() not available */
  static bool testGMTPrinting()
  {
    // Broken on Mac OS X
    // std::ostringstream sstr0;
    // printISO8601DateUTC(0.0, sstr0);
    // assertTrueMsg(sstr0.str() == "1970-01-01T00:00:00Z",
    //               "GMT date printing error at epoch, prints as \"" << sstr0.str() << "\"");

    struct tm gmt1;
    tm_init(gmt1, 2012, 6, 16, 5, 30, 0, 0);
    time_t gmtime1 = timegm(&gmt1);
    std::ostringstream sstr3;
    printISO8601DateUTC((double) gmtime1, sstr3);
    assertTrue(sstr3.str() == "2012-06-16T05:30:00Z", 
               "GMT date printing error");

    return true;
  }
#endif

  static bool testCompleteDurationParsing()
  {
    double result = 0;
    
    // Basics
    assertTrue(NULL != parseISO8601Duration("PT20S", result),
               "Complete duration parsing (seconds) failed");
    assertTrueMsg(result == 20.0,
                  "Complete duration parsing (seconds) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT20M", result),
               "Complete duration parsing (minutes) failed");
    assertTrueMsg(result == 1200.0,
                  "Complete duration parsing (minutes) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT20H", result),
               "Complete duration parsing (hours) failed");
    assertTrueMsg(result == 72000.0,
                  "Complete duration parsing (hours) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20D", result),
               "Complete duration parsing (days) failed");
    assertTrueMsg(result == 1728000.0,
                  "Complete duration parsing (days) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20W", result),
               "Complete duration parsing (weeks) failed");
    assertTrueMsg(result == 12096000.0,
                  "Complete duration parsing (weeks) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20M", result),
               "Complete duration parsing (months) failed");
    assertTrueMsg(result == 51840000.0,
                  "Complete duration parsing (months) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20Y", result),
               "Complete duration parsing (years) failed");
    assertTrueMsg(result == 630720000.0,
                  "Complete duration parsing (years) returned wrong result " << result);

    // Combinations
    assertTrue(NULL != parseISO8601Duration("P20DT20S", result),
               "Complete duration parsing (days, seconds) failed");
    assertTrueMsg(result == 1728020.0,
                  "Complete duration parsing (days, seconds) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20DT20M", result),
               "Complete duration parsing (days, minutes) failed");
    assertTrueMsg(result == 1729200.0,
                  "Complete duration parsing (days, minutes) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20DT20M20S", result),
               "Complete duration parsing (days, minutes, seconds) failed");
    assertTrueMsg(result == 1729220.0,
                  "Complete duration parsing (days, minutes, seconds) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20DT20H", result),
               "Complete duration parsing (days, hours) failed");
    assertTrueMsg(result == 1800000.0,
                  "Complete duration parsing (days, hours) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20DT20H20S", result),
               "Complete duration parsing (days, hours, seconds) failed");
    assertTrueMsg(result == 1800020.0,
                  "Complete duration parsing (days, hours, seconds) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20DT20H20M20S", result),
               "Complete duration parsing (days, hours, minutes, seconds) failed");
    assertTrueMsg(result == 1801220.0,
                  "Complete duration parsing (days, hours, minutes, seconds) returned wrong result " << result);

    assertTrue(NULL != parseISO8601Duration("P20M20D", result),
               "Complete duration parsing (months, days) failed");
    assertTrueMsg(result == 53568000.0,
                  "Complete duration parsing (months, days) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20MT20S", result),
               "Complete duration parsing (months, seconds) failed");
    assertTrueMsg(result == 51840020.0,
                  "Complete duration parsing (months, seconds) returned wrong result " << result);

    assertTrue(NULL != parseISO8601Duration("P20Y20D", result),
               "Complete duration parsing (years, days) failed");
    assertTrueMsg(result == 632448000.0,
                  "Complete duration parsing (years) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P20YT20S", result),
               "Complete duration parsing (years, seconds) failed");
    assertTrueMsg(result == 630720020.0,
                  "Complete duration parsing (years, seconds) returned wrong result " << std::setprecision(15) << result);

    // Error checking
    assertTrue(NULL == parseISO8601Duration("P20Y20S", result),
               "Complete duration parsing (years, seconds) failed to detect missing T separator");

    return true;
  }

  static bool testAlternativeBasicDurationParsing()
  {
    double result = 0;
    assertTrue(NULL != parseISO8601Duration("PT000020", result),
               "Alternative basic duration parsing (seconds) failed");
    assertTrueMsg(result == 20.0,
                  "Alternative basic duration parsing (seconds) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT002000", result),
               "Alternative basic duration parsing (minutes) failed");
    assertTrueMsg(result == 1200.0,
                  "Alternative basic duration parsing (minutes) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT0020", result),
               "Alternative basic duration parsing (minutes) failed");
    assertTrueMsg(result == 1200.0,
                  "Alternative basic duration parsing (minutes) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT200000", result),
               "Alternative basic duration parsing (hours) failed");
    assertTrueMsg(result == 72000.0,
                  "Alternative basic duration parsing (hours) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT2000", result),
               "Alternative basic duration parsing (hours) failed");
    assertTrueMsg(result == 72000.0,
                  "Alternative basic duration parsing (hours) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT20", result),
               "Alternative basic duration parsing (hours) failed");
    assertTrueMsg(result == 72000.0,
                  "Alternative basic duration parsing (hours) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P00000020", result),
               "Alternative basic duration parsing (days) failed");
    assertTrueMsg(result == 1728000.0,
                  "Alternative basic duration parsing (days) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P00002000", result),
               "Alternative basic duration parsing (months) failed");
    assertTrueMsg(result == 51840000.0,
                  "Alternative basic duration parsing (months) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P00200000", result),
               "Alternative basic duration parsing (years) failed");
    assertTrueMsg(result == 630720000.0,
                  "Alternative basic duration parsing (years) returned wrong result " << result);
    return true;
  }

  static bool testAlternativeExtendedDurationParsing()
  {
    double result = 0;
    assertTrue(NULL != parseISO8601Duration("PT00:00:20", result),
               "Alternative extended duration parsing (seconds) failed");
    assertTrueMsg(result == 20.0,
                  "Alternative extended duration parsing (seconds) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT00:20:00", result),
               "Alternative extended duration parsing (minutes) failed");
    assertTrueMsg(result == 1200.0,
                  "Alternative extended duration parsing (minutes) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT00:20", result),
               "Alternative extended duration parsing (minutes) failed");
    assertTrueMsg(result == 1200.0,
                  "Alternative extended duration parsing (minutes) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT20:00:00", result),
               "Alternative extended duration parsing (hours) failed");
    assertTrueMsg(result == 72000.0,
                  "Alternative extended duration parsing (hours) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT20:00", result),
               "Alternative extended duration parsing (hours) failed");
    assertTrueMsg(result == 72000.0,
                  "Alternative extended duration parsing (hours) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("PT20", result),
               "Alternative extended duration parsing (hours) failed");
    assertTrueMsg(result == 72000.0,
                  "Alternative extended duration parsing (hours) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P0000-00-20", result),
               "Alternative extended duration parsing (days) failed");
    assertTrueMsg(result == 1728000.0,
                  "Alternative extended duration parsing (days) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P0000-20-00", result),
               "Alternative extended duration parsing (months) failed");
    assertTrueMsg(result == 51840000.0,
                  "Alternative extended duration parsing (months) returned wrong result " << result);
    assertTrue(NULL != parseISO8601Duration("P0020-00-00", result),
               "Alternative extended duration parsing (years) failed");
    assertTrueMsg(result == 630720000.0,
                  "Alternative extended duration parsing (years) returned wrong result " << result);
    return true;
  }

  static bool testDurationPrinting()
  {
    std::ostringstream str0;
    printISO8601Duration(0.0, str0);
    assertTrueMsg(str0.str() == "PT0S",
                  "Wrong result printing zero duration \"" << str0.str() << "\"");

    std::ostringstream str1;
    printISO8601Duration(20.0, str1);
    assertTrueMsg(str1.str() == "PT20S",
                  "Wrong result printing seconds \"" << str1.str() << "\"");

    std::ostringstream str2;
    printISO8601Duration(1200.0, str2);
    assertTrueMsg(str2.str() == "PT20M",
                  "Wrong result printing minutes \"" << str2.str() << "\"");

    std::ostringstream str3;
    printISO8601Duration(72000.0, str3);
    assertTrueMsg(str3.str() == "PT20H",
                  "Wrong result printing hours \"" << str3.str() << "\"");

    std::ostringstream str4;
    printISO8601Duration(1728000.0, str4);
    assertTrueMsg(str4.str() == "P20D",
                  "Wrong result printing days \"" << str4.str() << "\"");

    std::ostringstream str5;
    printISO8601Duration(1728020.0, str5);
    assertTrueMsg(str5.str() == "P20DT20S",
                  "Wrong result printing days and seconds \"" << str5.str() << "\"");

    std::ostringstream str6;
    printISO8601Duration(51840000.0, str6);
    assertTrueMsg(str6.str() == "P1Y7M25D",
                  "Wrong result printing years, months, days \"" << str6.str() << "\"");

    return true;
  }

};

class StricmpTests
{
public:
  static bool test()
  {
    runTest(testNullArgs);
    runTest(testSameCase);
    runTest(testDifferentCases);
    return true;
  }

private:
  static bool testNullArgs()
  {
    assertTrue_1(0 == stricmp(NULL, NULL));
    assertTrue_1(0 == stricmp(NULL, ""));
    assertTrue_1(-1 == stricmp(NULL, " "));
    assertTrue_1(0 == stricmp("", NULL));
    assertTrue_1(1 == stricmp(" ", NULL));
    assertTrue_1(0 == stricmp("", ""));
    return true;
  }

  static bool testSameCase()
  {
    assertTrue_1(0 == stricmp(" ", " "));
    assertTrue_1(1 == stricmp("  ", " "));
    assertTrue_1(-1 == stricmp(" ", "  "));

    assertTrue_1(0 == stricmp("123", "123"));
    assertTrue_1(1 == stricmp("234", "123"));
    assertTrue_1(1 == stricmp("1234", "123"));
    assertTrue_1(-1 == stricmp("123", "1234"));
    assertTrue_1(-1 == stricmp("123", "234"));

    assertTrue_1(0 == stricmp("foo", "foo"));
    assertTrue_1(0 == stricmp("FOO", "FOO"));
    assertTrue_1(-1 == stricmp("foo", "fool"));
    assertTrue_1(1 == stricmp("fool", "foo"));
    assertTrue_1(1 == stricmp("fou", "foo"));
    assertTrue_1(-1 == stricmp("foo", "fou"));

    return true;
  }

  static bool testDifferentCases()
  {
    assertTrue_1(0 == stricmp("foo", "FOO"));
    assertTrue_1(0 == stricmp("FOO", "foo"));

    assertTrue_1(-1 == stricmp("FOO", "fool"));
    assertTrue_1(-1 == stricmp("foo", "FOOL"));

    assertTrue_1(1 == stricmp("fool", "FOO"));
    assertTrue_1(1 == stricmp("FOOL", "foo"));

    assertTrue_1(1 == stricmp("fou", "FOO"));
    assertTrue_1(1 == stricmp("FOU", "foo"));

    assertTrue_1(-1 == stricmp("foo", "FOU"));
    assertTrue_1(-1 == stricmp("FOO", "fou"));

    return true;
  }
};

void UtilModuleTests::runTests(std::string /* path */) 
{
  runTestSuite(ErrorTest::test);
  runTestSuite(DebugTest::test);
  runTestSuite(TimespecTests::test);
  runTestSuite(TimevalTests::test);
  runTestSuite(ISO8601Tests::test);
#ifdef PLEXIL_WITH_THREADS
  runTestSuite(MutexTest::test);
#endif
  runTestSuite(IdTests::test);
  runTestSuite(StricmpTests::test);

  // Do cleanup
  runFinalizers();

  std::cout << "Finished" << std::endl;
}
