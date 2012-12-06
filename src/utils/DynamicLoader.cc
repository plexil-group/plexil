/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

// *** KLUDGE ALERT ***
// This macro used to be provided by the Makefile system.
// It *should* be conditional based on the target OS.
// I have not yet found a way to properly pass it in using automake. So punt for now.

#define LIB_EXT ".so"

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
    if (libPath == NULL || *libPath == '\0') {
	  // construct library name from module name
	  libName = "lib" + std::string(moduleName);
	  debugMsg("DynamicLoader:loadModule",
			   " no library name provided for module \""
			   << moduleName << "\", using default value of \""
			   << libName << "\"");
	}
    else {
	  // use provided name
	  libName = libPath;
	}

    std::string funcName = (std::string("init") + moduleName);
    void (*func)();

    // attempt to load the library
    // (technically, attempt to find the named symbol in the spec'd library)
    *(void **)(&func) = DynamicLoader::getDynamicSymbol(libName.c_str(), funcName.c_str());
    if (!func) {
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

  static const char* LIBRARY_EXTENSIONS[] = {".so", ".dylib", NULL};

  void *DynamicLoader::getDynamicSymbol(const char* libName, const char* symbol) 
  {
    void *handle = NULL;
    // Try path verbatim
    handle = dlopen(libName, RTLD_NOW | RTLD_GLOBAL);
    if (handle) {
	  debugMsg("DynamicLoader:verboseLoadModule",
			   " dlopen succeeded for " << libName);
    }
    else {
	  debugMsg("DynamicLoader:verboseLoadModule",
			   " dlopen failed on file " << libName << ": " << dlerror());
      // Try adding the appropriate extension
      size_t i = 0;
      while (!handle && LIBRARY_EXTENSIONS[i]) {
        std::string libPath = libName;
        libPath += LIBRARY_EXTENSIONS[i++];
        handle = dlopen(libPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (handle) {
          debugMsg("DynamicLoader:verboseLoadModule",
                   " dlopen succeeded for " << libPath);
        }
        else {
          debugMsg("DynamicLoader:verboseLoadModule",
                   " dlopen failed on file " << libPath << ": " << dlerror());
        }
      }
      if (!handle) {
        debugMsg("DynamicLoader:loadModule",
                 " unable to open library \"" << libName << "\"");
        return NULL;
      }
	}
    void *func = dlsym(handle, symbol);
    if (!func) { // an error occured in loading the function
	  debugMsg("DynamicLoader:loadModule",
			   " dlsym failed to find symbol \"" << symbol << "\": " << dlerror());
      return NULL;
	}
    return func;
  }

  const char *DynamicLoader::getError() {
    return dlerror();
  }
}
