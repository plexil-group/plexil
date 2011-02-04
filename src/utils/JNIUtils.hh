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

#ifndef PLEXIL_JNI_UTILS_HH
#define PLEXIL_JNI_UTILS_HH

#include <jni.h>

//
// Utilities for accessing Java data
//

namespace PLEXIL {

  /**
   * @brief A data-less class whose static methods allow access to Java objects.
   */ 

  class JNIUtils
  {
  public:

	/**
	 * @brief Initialize the JNI utils.
	 * @param The JNIEnv pointer.
	 */
	static void initialize(JNIEnv *env);

	/**
	 * @brief Returns a freshly allocated copy of the Java string.
	 * @param javaString The JNI string object.
	 * @return A freshly allocated copy of the Java string, or NULL.
	 */
	static char* getJavaStringCopy(jstring javaString);

	/**
	 * @brief Returns a freshly allocated copy of the Java string.
	 * @param javaArgv The JNI string array.
	 * @param argcReturn A reference to an int variable to hold the argument count.
	 * @param argvReturn A reference to a char** variable to hold the argument vector.
	 * Both the strings and the string array are freshly allocated and must be deleted by the caller.
	 * @return true if the operation was successful, false otherwise.
	 */
	static bool getArgcArgv(jobjectArray javaArgv, int &argcReturn, char** &argvReturn);

  private:

	/**
	 * @brief Get the JNIEnv object.
	 * @return A reference to the singleton JNIEnv pointer.
	 */
	static JNIEnv*& getJNIEnv();

	// Deliberately unimplemented
	JNIUtils(); 
	JNIUtils(const JNIUtils&);
	JNIUtils& operator=(const JNIUtils&);
	~JNIUtils();

  }; // class JNIUtils

} // namespace PLEXIL

#endif // PLEXIL_JNI_UTILS_HH
