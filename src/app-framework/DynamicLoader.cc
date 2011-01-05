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

#include "DynamicLoader.hh"
#include <dlfcn.h>
#include <string>

#include "Debug.hh"

namespace PLEXIL
{
    /**
     * @brief Dynamically load the shared library containing the module name, using the library name if provided.
     * @param typeName The name of the module
     * @param libPath The library name containing the module, or NULL.
     * @param moduleTypeForDisplay Used in display messages only.
     * @return true if successful, false otherwise.
     * @note Expects to call init<moduleName>() with no args to initialize the freshly loaded module.
     */

  bool DynamicLoader::loadModule(const char* moduleName, 
				 const char* libPath)
  {
    std::string libName;
    if (libPath == NULL)
      {
	// construct library name from module name
	libName = "lib" + std::string(moduleName);
	debugMsg("DynamicLoader:loadModule",
		 " no library name provided for module \""
		 << moduleName << "\", using default value of \""
		 << libName << "\"");
      }
    else
      {
	// use provided name
	libName = libPath;
      }

    // append file extension
    libName += LIB_EXT;

    // attempt to load the library
    // (technically, attempt to find the named symbol in the spec'd library)
    std::string funcName = (std::string("init") + moduleName);
    void (*func)();
    *(void **)(&func) = DynamicLoader::getDynamicSymbol(libName.c_str(), funcName.c_str());
    if (func == 0)
      {
	debugMsg("DynamicLoader:loadModule", 
			 " Failed to load library " << libPath);
	return false;
      }

    // Call init function
    (*func)();

    debugMsg("DynamicLoader:loadModule",
	     " successfully loaded \"" << moduleName << "\"");
    return true;
  }

  void *DynamicLoader::getDynamicSymbol(const char* libPath, const char* symbol) {
    void *handle = dlopen(libPath, RTLD_NOW | RTLD_GLOBAL);
    if (!handle) { //an error occured in loading the library
	  debugMsg("DynamicLoader:loadModule",
			   " dlopen failed on file " << libPath << ": " << dlerror());
      return 0;
	}
    void *func = dlsym(handle, symbol);
    if (!func) { //an error occured in loading the function
	  debugMsg("DynamicLoader:loadModule",
			   " dlsym failed to find symbol \"" << symbol << "\": " << dlerror());
      return 0;
	}
    return func;
  }

  const char *DynamicLoader::getError() {
    return dlerror();
  }
}
