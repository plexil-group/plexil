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
#include "Error.hh"
#include "Debug.hh"
#include <stddef.h> // for NULL

//
// Utilities for accessing Java data
//

namespace PLEXIL {

  /**
   * @brief Initialize the JNI utils.
   * @param The JNIEnv pointer.
   */
  void JNIUtils::initialize(JNIEnv *env)
  {
	getJNIEnv() = env;
  }

  /**
   * @brief Returns a freshly allocated copy of the Java string.
   * @param javaString The JNI string object.
   * @return A freshly allocated copy of the Java string, or NULL.
   */
  char* JNIUtils::getJavaStringCopy(jstring javaString)
  {
	JNIEnv* &env(getJNIEnv());
	assertTrueMsg(env != NULL,
				  "getJavaStringCopy called before JNIUtils initialized");

	jsize utflen = env->GetStringUTFLength(javaString);
	char* ourString = new char[utflen + 1];
	env->GetStringUTFRegion(javaString, 0, env->GetStringLength(javaString), ourString);
	ourString[utflen] = '\0'; // to be safe
	return ourString;
  }

  /**
   * @brief Extract the strings from a Java string array in argc/argv format.
   * @param javaArgv The JNI string array.
   * @param argcReturn A reference to an int variable to hold the argument count.
   * @param argvReturn A reference to a char** variable to hold the newly allocated argument vector.
   * @return true if the operation was successful, false otherwise.
   * @note Both the strings and the string array are freshly allocated and must be deleted by the caller.
   */
  bool JNIUtils::getArgcArgv(jobjectArray javaArgv, int &argcReturn, char** &argvReturn)
  {
	JNIEnv* &env(getJNIEnv());
	assertTrueMsg(env != NULL,
				  "getJavaStringCopy called before JNIUtils initialized");

	// Get argv length
	int argc = env->GetArrayLength(javaArgv);

	// Allocate and initialize argv
	char** argv = new char*[argc + 1];
	for (unsigned int i = 0; i <= argc; i++)
	  argv[i] = NULL;
	   
	// Copy the strings
	for (unsigned int i = 0; i < argc; i++) {
	  jstring java_string = (jstring) env->GetObjectArrayElement(javaArgv, i);
	  if (java_string == NULL) {
		debugMsg("JNIUtils:getArgcArgv", "GetObjectArrayElement returned NULL");
		// FIXME: clean up all allocated structures here
		return false;
	  }
	  argv[i] = getJavaStringCopy(java_string);
	}

	argcReturn = argc;
	argvReturn = argv;
	return true;
  }

  /**
   * @brief Get the JNIEnv object.
   * @return A reference to the singleton JNIEnv pointer.
   */
  JNIEnv*& JNIUtils::getJNIEnv()
  {
	static JNIEnv* sl_env = NULL;
	return sl_env;
  }

}
