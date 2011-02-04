/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

#include "JNIUtils.hh"
#include "ScopedOstreamRedirect.hh"
#include "exec-test-module.hh"
#include <iostream>
#include <fstream>

using PLEXIL::JNIUtils;

std::string *logFileName(const char* dirname)
{
  std::string *result = new std::string(dirname);
  *result = *result + std::string("/") + std::string("date") + std::string(".log");
  return result;
}

extern "C"
jint Java_gov_nasa_plexil_ExecModuleTest_run(JNIEnv *env, jobject /* java_this */)
{
  JNIUtils::initialize(env);

  // Route cout and cerr to a log file.
  std::string* logName = logFileName("/data/data/gov.nasa.plexil/logs");
  std::ofstream log(logName->c_str());
  if (log.fail())
	return -1;
  PLEXIL::ScopedOstreamRedirect coutRedirect(std::cout, log);
  PLEXIL::ScopedOstreamRedirect cerrRedirect(std::cerr, log);

  ExecModuleTests::runTests();
  delete logName;
  return 0;
}

