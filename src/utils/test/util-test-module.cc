/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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
#include "ItemStore.hh"
#include "ItemTable.hh"
#include "KeySource.hh"
#include "LabelStr.hh"
#include "StoredArray.hh"
#include "StoredItem.hh"
#include "TestData.hh"
#include "timespec-utils.hh"
#include "timeval-utils.hh"
#include "TwoWayStore.hh"
#include "TwoWayTable.hh"
#include "Value.hh"
#include "XMLUtils.hh"

#include <cfloat>
#include <cstring> // for strcmp
#include <iomanip>
#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include <typeinfo>

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

class KeySourceTests
{
private:
  template<typename key_t, typename key_source_t>
  static bool testKeyAllocation()
  {
    key_source_t keygen;
    size_t keySpace = KeySource<key_t>::totalKeys();
    std::cout << "key space: " << keySpace << std::endl;
    key_t lastKey = key_source_t::unassigned();
    size_t n = 0;
    try {
      std::cout << std::endl;
      Error::doThrowExceptions();
      for (size_t i = 0; i < keySpace + 1; ++i) {
        key_t next = keygen.next();
        if (next == lastKey) {
          std::cout << "Error: non-unique key " << next << std::endl;
          return false;
        }
        lastKey = next;
        ++n;
      }
      // If we fall through from the loop,
      // we have failed to detect exhaustion
      std::cout << "Error: failed to detect keyspace exhaustion;\n keyMax = "
                << std::setprecision(15) << key_source_t::keyMax()
                << ", last key = " << std::setprecision(15) << lastKey
                << std::endl;
      return false;
    }
    catch (Error &e) {
      std::cout << "\nCaught expected exception: ";
      e.print(std::cout);
      std::cout << std::endl;
      if (n != keySpace) {
        std::cout << "Error: # of keys generated, " << n
                  << ", does not equal totalKeys(), " << keySpace 
                  << std::endl;
        return false;
      }
      std::cout << " done." << std::endl;
      return true;
    }

    std::cout << "Error: should never get here" << std::endl;
    return false;
  }

  template<typename key_t>
  static bool testSpecialAllocation()
  {
    PartitionedKeySource<key_t> keygen;
    size_t keySpace = PartitionedKeySource<key_t>::totalKeys();
    size_t specialKeySpace = PartitionedKeySource<key_t>::totalSpecialKeys();
    std::cout << "key space: " << keySpace
              << " special key space: " << specialKeySpace
              << " unassigned: " << std::setprecision(8) << PartitionedKeySource<key_t>::unassigned()
              << " special max: " << std::setprecision(8) << PartitionedKeySource<key_t>::specialMax()
              << std::endl;
    key_t lastKey = PartitionedKeySource<key_t>::unassigned();
    size_t n = 0;
    try {
      std::cout << std::endl;
      Error::doThrowExceptions();
      size_t i = 0;
      for (; i < specialKeySpace; ++i) {
        key_t next = keygen.next(true);
        if (next == lastKey) {
          std::cout << "Error: non-unique key " << next << std::endl;
          return false;
        }
        if (!PartitionedKeySource<key_t>::isSpecial(next)) {
          std::cout << "Error: non-special key " << std::setprecision(15) << next
                    << " before total special keys exhausted, n = " << n << std::endl;
          return false;
        }
        lastKey = next;
        ++n;
      }

      for (; i <= keySpace; ++i) {
        key_t next = keygen.next(true); // push past special limit
        if (next == lastKey) {
          std::cout << "Error: non-unique key " << next << std::endl;
          return false;
        }
        if (PartitionedKeySource<key_t>::isSpecial(next)) {
          std::cout << "Error: special key " << std::setprecision(15) << next
                    << " after total special keys exhausted" << std::endl;
          return false;
        }
        lastKey = next;
        ++n;
      }
      
      // If we fall through from the loop,
      // we have failed to detect overall exhaustion
      std::cout << "Error: failed to detect keyspace exhaustion;\n keyMax = "
                << std::setprecision(15) << PartitionedKeySource<key_t>::keyMax()
                << ", last key = " << std::setprecision(15) << lastKey
                << std::endl;
      return false;
    }
    catch (Error &e) {
      std::cout << "\nCaught expected exception: ";
      e.print(std::cout);
      std::cout << std::endl;
      if (n != keySpace) {
        std::cout << "Error: # of keys generated, " << n
                  << ", does not equal totalKeys(), " << keySpace 
                  << std::endl;
        return false;
      }
      std::cout << " done." << std::endl;
      return true;
    }

    std::cout << "Error: should never get here" << std::endl;
    return false;
  }

public:
  static bool test()
  {
    return testKeyAllocation<unsigned char, KeySource<unsigned char> >()
      && testKeyAllocation<signed char, KeySource<signed char> >()
      && testKeyAllocation<unsigned short, KeySource <unsigned short> >()
      && testKeyAllocation<float, KeySource<float> >()
      && testKeyAllocation<float, KeySource<float, NegativeDenormKeyTraits<float> > >()
      // && testKeyAllocation<double, KeySource<double> >() // too many keys to exhaust in reasonable time!
      && testSpecialAllocation<unsigned short>()
      && testSpecialAllocation<float>()
      // && testSpecialAllocation<double>() // too many keys to exhaust in reasonable time!
      ;
  }
};

class ItemTableTests
{
private:
  template <typename key_t, typename key_source_t>
  static bool testItemTableKeys()
  {
    ItemTable<key_t, std::string> tbl;
    const size_t n = 10000;
    // Populate table
    std::cout << "Testing linear insertion... " << std::flush;
    {
      key_source_t keygen1;
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen1.next();
        std::ostringstream ostream;
        ostream << i;
        ItemStoreEntry<std::string>* entry = new ItemStoreEntry<std::string>;
        entry->refcount = 1;
        entry->item = ostream.str();
        tbl.insertEntry(key, entry);
      }
      assertTrueMsg(tbl.size() == n,
                    "Error populating table; size should be " << n << ", is " << tbl.size());
      std::cout << " done." << std::endl;
    }
    // Check contents
    std::cout << "Checking contents... " << std::flush;
    {
      key_source_t keygen2;
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen2.next();
        ItemStoreEntry<std::string>* entry = tbl.get(key);
        assertTrueMsg(entry != NULL, "Error: No table entry found for key " << key);
        assertTrueMsg(entry->refcount == 1, "get error: entry refcount != 1");
        std::istringstream str(entry->item);
        size_t j;
        str >> j;
        assertTrueMsg(i == j, "Item at key " << key << " should be " << i << ", is " << j);
      }
      std::cout << " done." << std::endl;
    }

    // Clear table by key
    std::cout << "Testing linear deletion... " << std::flush;
    {
      key_source_t keygen3;
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen3.next();
        ItemStoreEntry<std::string>* entry = tbl.get(key);
        assertTrueMsg(entry != NULL, "Error: No table entry found for key " << key);
        tbl.removeEntry(key);
        entry = tbl.get(key);
        assertTrueMsg(entry == NULL, "Error: removeEntry failed");
      }
    }
    assertTrueMsg(tbl.empty(), "Error: Table not empty after clearing");
    assertTrueMsg(tbl.size() == 0, "Error: Table size not zero after clearing");
    std::cout << " done." << std::endl;

    return true;
  }

public:
  static bool test()
  {
    return testItemTableKeys<unsigned int, KeySource<unsigned int> >()
      && testItemTableKeys<double, KeySource<double> >()
      && testItemTableKeys<double, KeySource<double, NegativeDenormKeyTraits<double> > >();
  }
};

class ItemStoreTests
{
private:
  template <typename key_t, typename key_source_t>
  static bool testItemStoreFunctions()
  {
    ItemStore<key_t, std::string, key_source_t, ItemTable<key_t, std::string> > store;
    assertTrueMsg(store.size() == 1,
                  "Error: At construction, ItemStore size is " << store.size() << " should be 1");
    const size_t n = 10000;

    // Populate store
    std::cout << "Testing linear insertion... " << std::flush;
    for (size_t i = 1; i <= n; ++i) {
      std::ostringstream ostream;
      ostream << i;
      store.storeItem(ostream.str());
    }
    assertTrueMsg(store.size() == n + 1,
                  "Error populating store; size is " << store.size()
                  << ", should be " << n + 1);
    std::cout << " done." << std::endl;

    // Check contents
    std::cout << "Checking contents... " << std::flush;
    {
      key_source_t keygen1;
      keygen1.next(); // for effect, to account for empty item
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen1.next();
        std::string* item = store.getItem(key);
        assertTrueMsg(item != NULL, "Error: No item found for key " << key);
        std::istringstream str(*item);
        size_t j;
        str >> j;
        assertTrueMsg(i == j, "Item at key " << key << " should be " << i << ", is " << j);
      }
      std::cout << " done." << std::endl;
    }

    // Mark as re-referenced
    std::cout << "Incrementing reference count... " << std::flush;
    {
      key_source_t keygen2;
      keygen2.next(); // for effect, to account for empty item
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen2.next();
        assertTrueMsg(store.newReference(key),
                      "Error: newReference failed for key " << key);
      }
      assertTrueMsg(store.size() == n + 1,
                    "Error: size is " << store.size()
                    << ", should be " << n + 1);
      std::cout << " done." << std::endl;
    }

    // Remove one reference
    std::cout << "Decrementing reference count... " << std::flush;
    {
      key_source_t keygen3;
      keygen3.next(); // for effect, to account for empty item
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen3.next();
        store.deleteReference(key);
        assertTrueMsg(NULL != store.getItem(key),
                      "Error: item deleted prematurely for key " << key);
      }
      assertTrueMsg(store.size() == n + 1,
                    "Error: size is " << store.size()
                    << ", should be " << n + 1);
      std::cout << " done." << std::endl;
    }

    // Remove 2nd reference
    std::cout << "Removing last reference... " << std::flush;
    {
      key_source_t keygen4;
      keygen4.next(); // for effect, to account for empty item
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen4.next();
        store.deleteReference(key);
        assertTrueMsg(NULL == store.getItem(key), "Error: item not deleted for key " << key);
      }
    }
    assertTrueMsg(store.size() == 1,
                  "Error: After clearing, ItemStore size is " << store.size() << " should be 1");
    std::cout << " done." << std::endl;

    return true;
  }

public:
  static bool test()
  {
    return testItemStoreFunctions<unsigned int, KeySource<unsigned int> >()
      && testItemStoreFunctions<double, KeySource<double> >()
      && testItemStoreFunctions<double, KeySource<double, NegativeDenormKeyTraits<double> > >();
  }
};

class StoredItemTests
{
private:
  template <typename key_t, typename item_store_t>
  static bool testStoredItemFunctions()
  {
    std::cout << "Testing basics ... " << std::flush;
    typedef StoredItem<key_t, std::string, item_store_t>
      stored_item_t;

    assertTrueMsg(stored_item_t::getSize() == 1,
                  "Error: At construction, StoredItem::getSize() is "
                  << stored_item_t::getSize() << ", should be 1");

    // Test basic functionality
    {
      stored_item_t mt1, mt2;                       // default constructor
      // constructor from item_t (const char* promoted to std::string)
      stored_item_t foo("foo");
      stored_item_t bar(foo.getKey());              // constructor from key_t
      stored_item_t baz = bar;                      // copy constructor
      stored_item_t bletch = std::string("bletch"); // constructor from item_t (?)

      assertTrueMsg(stored_item_t::getSize() == 3,
                    "Error: StoredItem::getSize() is "
                    << stored_item_t::getSize() << ", should be 3");

      assertTrue(mt1 == mt2);
      assertTrue(mt1 != foo);
      assertTrue(foo == bar);
      assertTrue(foo == baz);
      assertTrue(bar == baz);
      assertTrue(baz != bletch);

      baz = bletch;              // assignment from stored_item_t
      assertTrue(baz == bletch);
      assertTrue(bar != baz);

      bar = bletch.getKey();     // assignment from key_t
      assertTrue(bar == bletch);
      assertTrue(foo != bar);

      mt1 = foo.getKey();        // assignment to empty item from key_t
      assertTrue(mt1 == foo);
      assertTrue(mt1 != mt2);

      mt1 = std::string("arf");  // assignment from value type
      assertTrue(mt1 != foo);
      assertTrue(mt1 != mt2);

      assertTrueMsg(stored_item_t::getSize() == 4,
                    "Error: StoredItem::getSize() is "
                    << stored_item_t::getSize() << ", should be 4");

      mt1 = std::string();       // assignment from empty value type
      // "arf" should be deleted
      assertTrue(mt1 == mt2);

      assertTrueMsg(stored_item_t::getSize() == 3,
                    "Error: StoredItem::getSize() is "
                    << stored_item_t::getSize() << ", should be 3");

      foo = bletch;           // assignment from stored_item_t
      // "foo" should be deleted

      assertTrueMsg(stored_item_t::getSize() == 2,
                    "Error: StoredItem::getSize() is "
                    << stored_item_t::getSize() << ", should be 2");

    } // above instances should all be deleted here
    assertTrueMsg(stored_item_t::getSize() == 1,
                  "Error: StoredItem::getSize() is "
                  << stored_item_t::getSize() << ", should be 1");
    std::cout << " done." << std::endl;

    {
      size_t n = 10000;
      std::vector<stored_item_t> vec(n);
      assertTrueMsg(stored_item_t::getSize() == 1,
                    "Error: StoredItem::getSize() is "
                    << stored_item_t::getSize() << ", should be 1");

      // Populate items in vector
      std::cout << "Populating vector of StoredItem ..." << std::flush;
      for (size_t i = 0; i < n; ++i) {
        std::ostringstream ostream;
        ostream << i;
        vec[i] = stored_item_t(ostream.str());
      }
      assertTrueMsg(stored_item_t::getSize() == n + 1,
                    "Error: StoredItem::getSize() is "
                    << stored_item_t::getSize()
                    << ", should be " << n + 1);
      std::cout << " done." << std::endl;

      std::cout << "Checking vector of StoredItem ..." << std::flush;
      for (size_t i = 0; i < n; ++i) {
        std::istringstream str(vec[i].getItem());
        size_t j;
        str >> j;
        assertTrueMsg(i == j, "Item should be " << i << ", is " << j);
      }
      std::cout << " done." << std::endl;

      std::cout << "Checking assignment in vector ..." << std::flush;
      for (size_t i = 1; i < n; ++i) {
        vec[i] = vec[0];
      }
      assertTrueMsg(stored_item_t::getSize() == 2,
                    "Error: StoredItem::getSize() is "
                    << stored_item_t::getSize() << ", should be 2");
      std::cout << " done." << std::endl;
    } // vector is deleted here

    assertTrueMsg(stored_item_t::getSize() == 1,
                  "Error: StoredItem::getSize() is "
                  << stored_item_t::getSize() << ", should be 1");

    return true;
  }

public:
  static bool test()
  {
    return testStoredItemFunctions<unsigned int, 
                                   ItemStore<unsigned int,
                                             std::string,
                                             KeySource<unsigned int>, 
                                             ItemTable<unsigned int, std::string> > >()
       && testStoredItemFunctions<double, 
                                  ItemStore<double,
                                            std::string,
                                            KeySource<double, NegativeDenormKeyTraits<double> >, 
                                            ItemTable<double, std::string> > >();
  }
};

class StoredArrayTests
{
private:
  static bool testStoredArrayBasics()
  {
    assertTrueMsg(StoredArray::getSize() == 1,
                  "Error: At construction, StoredArray::getSize() is " << StoredArray::getSize()
                  << ", should be 1");

    std::cout << "Testing basics ... " << std::flush;
    {
      StoredArray sa0;
      assertTrue(sa0.size() == 0);

      StoredArray sa1(10, UNKNOWN());
      assertTrue(sa1.size() == 10);
      sa1[0] = 3.3;
      sa1[1] = 9.9;
      assertTrue(sa1[2].isUnknown());
      assertTrueMsg(StoredArray::getSize() == 2,
                    "Error: StoredArray::getSize() is " << StoredArray::getSize()
                    << ", should be 2");

      StoredArray sa2(sa1.getKey());
      assertTrue(sa2.size() == 10);
      assertTrue(sa2[0] == 3.3);
      assertTrue(sa2[1] == 9.9);
      assertTrue(sa2[2].isUnknown());
      assertTrueMsg(StoredArray::getSize() == 2,
                    "Error: StoredArray::getSize() is " << StoredArray::getSize()
                    << ", should be 2");

      sa2 = sa0;
      assertTrue(sa2.size() == 0);
      assertTrueMsg(StoredArray::getSize() == 2,
                    "Error: StoredArray::getSize() is " << StoredArray::getSize()
                    << ", should be 2");
    }
    assertTrueMsg(StoredArray::getSize() == 1,
                  "Error: After cleanup, StoredArray::getSize() is " << StoredArray::getSize()
                  << ", should be 1");


    std::cout << " done." << std::endl;
    return true;
  }

  static bool testMemory()
  {
    std::cout << std::endl;

    size_t width = 1000;
    size_t testSize = 1000;
    size_t updateSize = 100;

    // preallocate the vector to the appropriate size
    std::vector<double> keys;
    keys.reserve(testSize);

    // create AND delete a whole bunch of StoredArray
    time_t startTotal = startTime();
    time_t start = startTime();
    for (int32_t i = 0; i < testSize; ++i) {
      StoredArray sa(width, i);
      if ((i + 1) % updateSize == 0)
        std::cout << "creating StoredArray: " << (i + 1)
                  << " key: " << sa.getKey() << "\r" << std::flush;
      keys.push_back(sa.getKey());
    }
    std::cout << std::endl;
    stopTime(start);
         
    // check that all these keys are invalid
    start = startTime();
    for (size_t i = 0; i < keys.size(); ++i) {
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

    size_t width = 10;
    size_t testSize = 100000; // was 2000000
    size_t updateSize = 10000; // was 100000
    // preallocate vectors to the appropriate size
    std::vector<double> keys;
    keys.reserve(testSize);
    std::vector<StoredArray*> arrays;
    arrays.reserve(testSize);

    // create a whole bunch of StoredArray
    time_t startTotal = startTime();
    time_t start = startTime();
    for (int32_t i = 0; i < testSize; ++i) {
      if ((i + 1) % updateSize == 0)
        std::cout << "creating StoredArray: " << (i + 1) <<
          "\r" << std::flush;
      StoredArray* sa = new StoredArray(width, i);
      arrays.push_back(sa);
      keys.push_back(sa->getKey());
    }
    std::cout << std::endl;
    stopTime(start);
         
    // change the values of each of the vectors
    start = startTime();
    for (size_t i = 0; i < keys.size(); ++i) {
      if ((i + 1) % updateSize == 0)
        std::cout << "changing elements in StoredArray: " 
                  << (i + 1) << "\r" << std::flush;
            
      StoredArray sa(keys[i]);
      for (int32_t j = 0; j < sa.size(); ++j)
        sa[j] = sa[j].getIntValue() + j;
    }
    std::cout << std::endl;
    stopTime(start);
         
    // test the values of each of the vectors
    start = startTime();
    for (int32_t i = 0; i < keys.size(); ++i) {
      if ((i + 1) % updateSize == 0)
        std::cout << "testing elements of StoredArray: " << 
          (i + 1) << "\r" << std::flush;
            
      StoredArray sa(keys[i]);
      assertTrueMsg((StoredArray::isKey(keys[i])),
                    "item key mismatch for index " << i);
            
      for (int32_t j = 0; j < sa.size(); ++j)
        assertTrueMsg(sa[j].getIntValue() == i + j,
                      "value " << sa[j].getIntValue() << " != " << (i + j));
    }
    std::cout << std::endl;
    stopTime(start);

    // delete everything
    start = startTime();
    for (size_t i = 0; i < arrays.size(); ++i) {
      if ((i + 1) % updateSize == 0)
        std::cout << "deleting StoredArray: " << 
          (i + 1) << "\r" << std::flush;
            
      StoredArray* sa = arrays[i];
      arrays[i] = NULL;
      delete sa;
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

public:
  static bool test()
  {
    return testStoredArrayBasics()
      && testMemory()
      && testSpeed();
  }
};

class TwoWayTableTests
{
private:
  template <typename key_t, typename key_source_t>
  static bool testTwoWayTableKeys()
  {
    TwoWayTable<key_t, std::string> tbl;
    const size_t n = 10000;
    // Populate table
    std::cout << "Testing linear insertion... " << std::flush;
    {
      key_source_t keygen1;
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen1.next();
        std::ostringstream ostream;
        ostream << i;
        ItemStoreEntry<std::string>* entry = new ItemStoreEntry<std::string>;
        entry->refcount = 1;
        entry->item = ostream.str();
        tbl.insertEntry(key, entry);
      }
      assertTrueMsg(tbl.size() == n,
                    "Error populating table; size should be " << n << ", is " << tbl.size());
      std::cout << " done." << std::endl;
    }
    // Check contents
    std::cout << "Checking contents... " << std::flush;
    {
      key_source_t keygen2;
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen2.next();
        ItemStoreEntry<std::string>* entryByKey = tbl.getByKey(key);
        assertTrueMsg(entryByKey != NULL, "No table entry found for key " << key);
        assertTrueMsg(entryByKey->refcount == 1, "getByKey error: entry refcount != 1");
        std::istringstream str(entryByKey->item);
        size_t j;
        str >> j;
        assertTrueMsg(i == j, "Item at key " << key << " should be " << i << ", is " << j);

        key_t itemKey;
        assertTrueMsg(tbl.getItemKey(entryByKey->item, itemKey),
                      "getItemKey error: no key found for item " << entryByKey->item);
        assertTrueMsg(itemKey == key,
                      "getItemKey error: returned wrong key");

        ItemStoreEntry<std::string>* entryByItem = tbl.getByItem(entryByKey->item);
        assertTrueMsg(entryByKey != NULL, "No table entry found for item " << key);
        assertTrueMsg(entryByKey == entryByItem,
                      "getByItem error: Entries differ for key " << key << " and item " << entryByKey->item);
      }
      std::cout << " done." << std::endl;
    }

    // Clear table by key
    std::cout << "Testing linear deletion... " << std::flush;
    {
      key_source_t keygen3;
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen3.next();
        ItemStoreEntry<std::string>* entryByKey = tbl.getByKey(key);
        assertTrueMsg(entryByKey != NULL, "Error: No table entry found for key " << key);
        tbl.removeEntry(key);
        entryByKey = tbl.getByKey(key);
        assertTrueMsg(entryByKey == NULL, "Error: removeEntry failed");
      }
    }
    assertTrueMsg(tbl.empty(), "Error: Table not empty after clearing");
    assertTrueMsg(tbl.size() == 0, "Error: Table size not zero after clearing");
    std::cout << " done." << std::endl;

    return true;
  }

public:
  static bool test()
  {
    return testTwoWayTableKeys<unsigned int, KeySource<unsigned int> >()
      && testTwoWayTableKeys<double, KeySource<double> >()
      && testTwoWayTableKeys<double, KeySource<double, NegativeDenormKeyTraits<double> > >();
  }
};

class TwoWayStoreTests
{
private:
  template <typename key_t, typename key_source_t>
  static bool testTwoWayStoreFunctions()
  {
    TwoWayStore<key_t, std::string, key_source_t, TwoWayTable<key_t, std::string> > store;
    assertTrueMsg(store.size() == 1,
                  "Error: At construction, store size is " << store.size() << " should be 1");
    const size_t n = 10000;

    // Populate store
    std::cout << "Testing linear insertion... " << std::flush;
    for (size_t i = 1; i <= n; ++i) {
      std::ostringstream ostream;
      ostream << i;
      store.storeItem(ostream.str());
    }
    assertTrueMsg(store.size() == n + 1 ,
                  "Error populating store: size is " << store.size() << ", should be " << n + 1);
    std::cout << " done." << std::endl;

    // Check contents
    std::cout << "Checking contents by key... " << std::flush;
    {
      key_source_t keygen1;
      keygen1.next(true); // account for empty item
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen1.next();
        std::string* item = store.getItem(key);
        assertTrueMsg(item != NULL, "Error: No item found for key " << key);
        std::istringstream str(*item);
        size_t j;
        str >> j;
        assertTrueMsg(i == j, "Item at key " << key << " should be " << i << ", is " << j);
      }
      std::cout << " done." << std::endl;
    }

    // Mark as re-referenced by key
    std::cout << "Incrementing reference count... " << std::flush;
    {
      key_source_t keygen2;
      keygen2.next(true); // account for empty item
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen2.next();
        assertTrueMsg(store.newReference(key),
                      "Error: newReference failed for key " << key);
      }
      assertTrueMsg(store.size() == n + 1 ,
                    "Error: store size is " << store.size() << ", should be " << n + 1);
      std::cout << " done." << std::endl;
    }

    // Mark as re-referenced by item
    std::cout << "Incrementing reference count by item... " << std::flush;
    {
      for (size_t i = 1; i <= n; ++i) {
        std::ostringstream ostream;
        ostream << i;
        store.storeItem(ostream.str());
      }
      assertTrueMsg(store.size() == n + 1 ,
                    "Error: store size is " << store.size() << ", should be " << n + 1);
      std::cout << " done." << std::endl;
    }

    // Remove one reference
    std::cout << "Decrementing reference count... " << std::flush;
    {
      key_source_t keygen3;
      keygen3.next(true); // account for empty item
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen3.next();
        store.deleteReference(key);
        assertTrueMsg(NULL != store.getItem(key),
                      "Error: item deleted prematurely for key " << key);
      }
      assertTrueMsg(store.size() == n + 1 ,
                    "Error: store size is " << store.size() << ", should be " << n + 1);
      std::cout << " done." << std::endl;
    }

    // Remove one reference
    std::cout << "Decrementing reference count again... " << std::flush;
    {
      key_source_t keygen4;
      keygen4.next(true); // account for empty item
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen4.next();
        store.deleteReference(key);
        assertTrueMsg(NULL != store.getItem(key),
                      "Error: item deleted prematurely for key " << key);
      }
      assertTrueMsg(store.size() == n + 1 ,
                    "Error: store size is " << store.size() << ", should be " << n + 1);
      std::cout << " done." << std::endl;
    }

    // Remove 2nd reference
    std::cout << "Removing last reference... " << std::flush;
    {
      key_source_t keygen5;
      keygen5.next(true); // account for empty item
      for (size_t i = 1; i <= n; ++i) {
        key_t key = keygen5.next();
        store.deleteReference(key);
        assertTrueMsg(NULL == store.getItem(key), "Error: item not deleted for key " << key);
      }
    }
    assertTrueMsg(store.size() == 1,
                  "Error: After clearing, store size is " << store.size() << " should be 1");
    std::cout << " done." << std::endl;

    return true;
  }

public:
  static bool test()
  {
    return testTwoWayStoreFunctions<unsigned int, PartitionedKeySource<unsigned int> >()
      && testTwoWayStoreFunctions<double, PartitionedKeySource<double> >();
  }
};

class LabelStrTests {
public:
  static bool test()
  {
    runTest(testLabelStrBasics);
    runTest(testElementCounting);
    runTest(testComparisons);
    return true;
  }

private:
  static bool testLabelStrBasics()
  {
    // Test basic allocation/deallocation
    assertTrueMsg(LabelStr::getSize() == 1,
                  "Error: LabelStr::getSize() is " << LabelStr::getSize() << ", should be 1");
    {
      // Test constructors
      LabelStr label0;                        // default constructor
      assertTrueMsg(LabelStr::getSize() == 1,
                    "Error: LabelStr::getSize() is " << LabelStr::getSize() << ", should be 1");

      LabelStr label1("label1");              // char* constructor
      LabelStr label2(std::string("label2")); // std::string constructor
      LabelStr label3 = label2;               // copy constructor
      assertTrueMsg(LabelStr::getSize() == 3,
                    "Error: LabelStr::getSize() is " << LabelStr::getSize() << ", should be 3");

      // Equality, inequality
      assertTrue(label2 == label3);           // equality test
      assertTrue(!(label0 == label1));
      assertTrue(label1 != label2);           // inequality test
      assertTrue(!(label2 != label3));

      label0 = label1;                        // assignment operator from LabelStr
      assertTrue(label0 == label1);

      return true;
    }

    {
      size_t n = 10000;
      std::vector<LabelStr> vec(n);
      assertTrueMsg(LabelStr::getSize() == 1,
                    "Error: LabelStr::getSize() is "
                    << LabelStr::getSize() << ", should be 1");

      // Populate items in vector
      std::cout << "Populating vector of LabelStr ..." << std::flush;
      for (size_t i = 0; i < n; ++i) {
        std::ostringstream ostream;
        ostream << i;
        vec[i] = ostream.str();
      }
      assertTrueMsg(LabelStr::getSize() == n + 1,
                    "Error: LabelStr::getSize() is "
                    << LabelStr::getSize()
                    << ", should be " << n + 1);
      std::cout << " done." << std::endl;

      std::cout << "Checking vector of LabelStr ..." << std::flush;
      for (size_t i = 0; i < n; ++i) {
        std::istringstream str(vec[i].toString());
        size_t j;
        str >> j;
        assertTrueMsg(i == j, "Item should be " << i << ", is " << j);
      }
      std::cout << " done." << std::endl;

      std::cout << "Checking assignment in vector ..." << std::flush;
      for (size_t i = 1; i < n; ++i) {
        vec[i] = vec[0];
      }
      assertTrueMsg(LabelStr::getSize() == 2,
                    "Error: LabelStr::getSize() is "
                    << LabelStr::getSize() << ", should be 2");
      std::cout << " done." << std::endl;
    } // vector is deleted here

    assertTrueMsg(LabelStr::getSize() == 1,
                  "Error: LabelStr::getSize() is " << LabelStr::getSize() << ", should be 1");
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

class ValueTests
{
public:
  static bool test()
  {
    runTest(testValueBasics);
    runTest(testValueRefcount);
    return true;
  }

private:
  static bool testValueBasics()
  {
    // Default constructor
    Value u;
    assertTrue(u.isUnknown());
    assertFalse(u.isString());
    assertFalse(u.isArray());

    // Bool constructor
    const bool b = true;
    Value bv(b);
    assertFalse(bv.isUnknown());
    assertFalse(bv.isString());
    assertFalse(bv.isArray());
    assertTrue(b == bv.getBoolValue());

    // Int constructor
    const int32_t i = 42;
    Value iv(i);
    assertFalse(iv.isUnknown());
    assertFalse(iv.isString());
    assertFalse(iv.isArray());
    assertTrue(i == iv.getIntValue());

    // Double constructor
    const double d = 3.1415;
    Value dv(d);
    assertFalse(dv.isUnknown());
    assertFalse(dv.isString());
    assertFalse(dv.isArray());
    assertTrue(d == dv.getDoubleValue());

    // LabelStr constructor
    const LabelStr l("Ellesse");
    Value lv(l);
    assertFalse(lv.isUnknown());
    assertTrue(lv.isString());
    assertFalse(lv.isArray());
    assertTrue(l == lv);

    // String constructor
    const std::string s = "Ess";
    Value sv(s);
    assertFalse(sv.isUnknown());
    assertTrue(sv.isString());
    assertFalse(sv.isArray());
    assertTrue(s == sv.getStringValue());

    // char* constructor
    const char* c = "See";
    Value cv(c);
    assertFalse(cv.isUnknown());
    assertTrue(cv.isString());
    assertFalse(cv.isArray());
    assertTrue(0 == strcmp(c, cv.c_str()));

    // StoredArray constructor
    const StoredArray sa(2, UNKNOWN());
    Value sav(sa);
    assertFalse(sav.isUnknown());
    assertFalse(sav.isString());
    assertTrue(sav.isArray());
    assertTrue(sa == sav.getStoredArrayValue());

    // Array constructor
    const StoredArray_value_t a(3);
    Value av(a);
    assertFalse(av.isUnknown());
    assertFalse(av.isString());
    assertTrue(av.isArray());
    assertTrue(a == av.getArrayValue()); 

    return true;
  }

  static bool testAssignment()
  {
    Value v;
    v = true;
    assertFalse(v.isUnknown());
    assertFalse(v.isString());
    assertFalse(v.isArray());
    assertTrue(v.getBoolValue());
    v = false;
    assertFalse(v.getBoolValue());
    v.setUnknown();
    assertTrue(v.isUnknown());

    int32_t i = 42;
    v = i;
    assertFalse(v.isUnknown());
    assertFalse(v.isString());
    assertFalse(v.isArray());
    assertTrue(v.getIntValue() == i);
    v.setUnknown();

    double d = 3.1415;
    v = d;
    assertFalse(v.isUnknown());
    assertFalse(v.isString());
    assertFalse(v.isArray());
    assertTrue(v.getDoubleValue() == d);
    v.setUnknown();
    
    LabelStr l("elle");
    v = l;
    assertFalse(v.isUnknown());
    assertTrue(v.isString());
    assertFalse(v.isArray());
    assertTrue(v == l);
    v.setUnknown();

    std::string s("esse");
    v = s;
    assertFalse(v.isUnknown());
    assertTrue(v.isString());
    assertFalse(v.isArray());
    assertTrue(v.getStringValue() == s);
    v.setUnknown();

    const char* c = "sea";
    v = c;
    assertFalse(v.isUnknown());
    assertTrue(v.isString());
    assertFalse(v.isArray());
    assertTrue(0 == strcmp(v.c_str(), c));
    v.setUnknown();

    StoredArray sa(3, 69.0);
    v = sa; // assign reference
    assertFalse(v.isUnknown());
    assertFalse(v.isString());
    assertTrue(v.isArray());
    assertTrue(v.getArrayValue() == sa.getConstArray());
    assertTrue(v.getStoredArrayValue() == sa);
    v.setUnknown();
    
    v.copyArray(sa); // assign copy
    assertFalse(v.isUnknown());
    assertFalse(v.isString());
    assertTrue(v.isArray());
    assertTrue(v.getArrayValue() == sa.getConstArray());
    assertFalse(v.getStoredArrayValue() == sa);
    v.setUnknown();

    StoredArray_value_t a(1, 0.0);
    v = a;
    assertFalse(v.isUnknown());
    assertFalse(v.isString());
    assertTrue(v.isArray());
    assertTrue(v.getArrayValue() == a);

    return true;
  }

  static bool testValueRefcount()
  {
    Value lv;
    double lvv;
    {
      LabelStr l("ell");
      lv = l;
      assertTrue(lv.isString());
      assertTrue(lv.getStringValue() == l.toString());
      assertTrue(lv.getRawValue() == l.getKey());
      lvv = lv.getRawValue();
    } // l deleted
    assertTrue(LabelStr::isString(lvv));
    lv.setUnknown();
    assertFalse(LabelStr::isString(lvv));

    {
      StoredArray s(3, 1.415);
      lv = s;
      assertTrue(lv.isArray());
      assertTrue(lv.getArrayValue() == s.getConstArray());
      assertTrue(lv.getRawValue() == s.getKey());
      lvv = lv.getRawValue();
    } // s deleted
    assertTrue(StoredArray::isKey(lvv));
    lv.setUnknown();
    assertFalse(StoredArray::isKey(lvv));

    return true;
  }

};

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
    runTest(testGMTPrinting);
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
    assertTrue(parseISO8601Date(zuluDate1, zuluTime1)
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

void UtilModuleTests::runTests(std::string /* path */) 
{
  runTestSuite(ErrorTest::test);
  runTestSuite(DebugTest::test);
  runTestSuite(TimespecTests::test);
  runTestSuite(TimevalTests::test);
  runTestSuite(ISO8601Tests::test);
  runTestSuite(MutexTest::test);
  runTestSuite(IdTests::test);
  runTestSuite(KeySourceTests::test);
  runTestSuite(ItemTableTests::test);
  runTestSuite(ItemStoreTests::test);
  runTestSuite(StoredItemTests::test);
  runTestSuite(StoredArrayTests::test);
  runTestSuite(TwoWayTableTests::test);
  runTestSuite(TwoWayStoreTests::test);
  runTestSuite(LabelStrTests::test);
  runTestSuite(ValueTests::test);

  std::cout << "Finished" << std::endl;
}
