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

#ifndef _H_TestSupport
#define _H_TestSupport


#include "Error.hh"
#include "Utils.hh"

#ifdef PLEXIL_ID_FAST
#define ID_TABLE_SIZE (0)
#define ID_TABLE_OUTPUT(dummy)
#else
#define ID_TABLE_SIZE PLEXIL::IdTable::size()
#define ID_TABLE_OUTPUT(str) PLEXIL::IdTable::output(str)
#endif


#define runTest(test) {	\
  try { \
  std::cout << "   " << #test << " "; \
  bool result = test(); \
  unsigned int id_count = ID_TABLE_SIZE; \
  if (result && ID_TABLE_SIZE <= id_count) \
    std::cout << " PASSED." << std::endl; \
  else \
    if (result) { \
      std::cout << " FAILED = DID NOT CLEAN UP ALLOCATED IDs:\n"; \
      ID_TABLE_OUTPUT(std::cout); \
      std::cout << "\tWere " << id_count << " IDs before; " << ID_TABLE_SIZE << " now"; \
      std::cout << std::endl; \
      throw Error::GeneralMemoryError(); \
    } else { \
      std::cout << "      " << " FAILED TO PASS UNIT TEST." << std::endl; \
      throw Error::GeneralUnknownError(); \
    } \
  } \
  catch (Error &err){ \
   err.print(std::cout); \
  }\
  }

#define runTestSuite(test) { \
  try{ \
  std::cout << #test << "******************************" << std::endl;\
  if (test()) \
    std::cout << #test << " PASSED." << std::endl; \
  else \
    std::cout << #test << " FAILED." << std::endl; \
  }\
  catch (Error &err){\
   err.print(std::cout);\
  }\
  }
#endif


