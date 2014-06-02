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

#include "lifecycle-utils.h"
#include "TestSupport.hh"

extern bool aliasTest();
extern bool arithmeticTest();
extern bool arrayConstantTest();
extern bool arrayOperatorsTest();
extern bool arrayReferenceTest();
extern bool arrayTest();
extern bool arrayVariableTest();
extern bool booleanOperatorsTest();
extern bool comparisonsTest();
extern bool constantsTest();
extern bool functionsTest();
extern bool listenerTest();
extern bool stateTest();
extern bool stringTest();
extern bool valueTest();
extern bool variablesTest();

using namespace PLEXIL;

int main(int argc, char *argv[])
{
  runTestSuite(listenerTest);
  runTestSuite(constantsTest);
  runTestSuite(variablesTest);
  runTestSuite(arrayTest);
  runTestSuite(valueTest);
  runTestSuite(stateTest);
  runTestSuite(arrayConstantTest);
  runTestSuite(arrayVariableTest);
  runTestSuite(arrayReferenceTest);
  runTestSuite(aliasTest);
  runTestSuite(functionsTest);
  runTestSuite(comparisonsTest);
  runTestSuite(booleanOperatorsTest);
  runTestSuite(arithmeticTest);
  runTestSuite(stringTest);
  runTestSuite(arrayOperatorsTest);

  // clean up
  runFinalizers();

  std::cout << "Finished" << std::endl;
  return 0;
}
