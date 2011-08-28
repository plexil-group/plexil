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

/*
 * DynamicLoader.hh
 *
 * Facilitates loading dynamic libraries and executing functions in them.
 * Linux-only at the moment. Mac may work, but needs testing.
 *
 *  Created on: Jan 27, 2010
 *      Author: jhogins
 */

#ifndef DYNAMICLOADER_HH_
#define DYNAMICLOADER_HH_

#include <cstddef> // for NULL

namespace PLEXIL
{
  class DynamicLoader {
  public:

    //
    // Dynamic library loading utility
    //
    
    /**
     * @brief Dynamically load the shared library containing the module name, using the library name if provided.
     * @param typeName The name of the module
     * @param libPath The library name containing the module; defaults to NULL.
     * @return true if successful, false otherwise.
     * @note If libPath is not provided, attempts to load 'lib<moduleName><LIB_EXT>'.
     * @note Expects to call init<moduleName>() with no args to initialize the freshly loaded module.
     */

    static bool loadModule(const char* moduleName, 
			   const char* libPath = NULL);

    /**
     * @brief Executes the given function in the given dynamic library (loaded and cached
     *    if not already).
     * @param libPath The path to the library to execute funcSymbol.
     * @param funcSymbol The function to execute.
     * @return NULL if loading failed, or a void * pointing to the requested symbol.
     * See getError for more info on NULL errors
     * @note The function must take no arguments and must
     *        have a return type of void.
     */
    static void *getDynamicSymbol(const char* libPath, const char* symbol);

    /**
     * @brief Returns a human readable string describing the most recent error that
     * occurred from runDynamicFunction.
     * @return NULL if no errors have occured since startup; a human readable string
     * describing the most recent error otherwise.
     */
    static const char *getError();

  };

}

#endif /* DYNAMICLOADER_HH_ */
