/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

// #include <dlfcn.h> // included in above
#include <string>

#include "Debug.hh"

namespace PLEXIL
{
  /**
   * @brief Dynamically load the shared library containing the module name, using the library name if provided.
   * @param typeName The name of the module
   * @param libPath The library name containing the module, or NULL.
   * @return true if successful, false otherwise.
   * @note Expects to call init<moduleName>() with no args to initialize the freshly loaded module.
   */

  bool DynamicLoader::loadModule(const char* moduleName, 
				 const char* libPath)
  {
    // Try to initialize it, in hopes it's already loaded
    if (initModule(moduleName)) {
      debugMsg("DynamicLoader:loadModule", " for " << moduleName << " succeeded");
      return true;
    }

    // Try to load it.
    std::string libName;
    if (libPath == NULL || *libPath == '\0') {
      // construct library name from module name
      libName = "lib" + std::string(moduleName);
      debugMsg("DynamicLoader:loadModule",
	       " no library name provided for module \""
	       << moduleName << "\", using default value of \""
	       << libName << "\"");
      libPath = libName.c_str();
    }

    void *dl_handle = loadLibrary(libPath);
    if (!dl_handle) {
      debugMsg("DynamicLoader:loadModule",
	       " for " << moduleName << " failed; library " << libPath << " not found");
      return false;
    }

    debugMsg("DynamicLoader:loadModule",
	     "for " << moduleName << ", found library " << libPath);

    // Try to initialize it again
    if (initModule(moduleName, dl_handle)) {
      debugMsg("DynamicLoader:loadModule", " for " << moduleName << " succeeded");
      return true;
    }

    debugMsg("DynamicLoader:loadModule",
	     " unable to initialize \"" << moduleName << '\"');
    return false;
  }

  void *DynamicLoader::findSymbol(char const *symName, void *dl_handle)
  {
    void *sym = dlsym(dl_handle, symName);
    if (!sym) {
      // error,  or is symbol actually NULL?
      char const *err = dlerror();
      if (err) {
	debugMsg("DynamicLoader:findSymbol",
		 " failed; symbol \"" << symName << "\" not found: " << err);
	return NULL;
      }
    }
    debugMsg("DynamicLoader:findSymbol",
	     " succeeded for \"" << symName << '"');
    return sym;
  }

  bool DynamicLoader::initModule(const char *moduleName, void *dl_handle)
  {
    std::string funcName = (std::string("init") + moduleName);
    void (*func)() = NULL;
    *(void **)(&func) = findSymbol(funcName.c_str(), dl_handle);
    if (!func) {
      debugMsg("DynamicLoader:initModule",
	       " failed; init function for module " << moduleName << " not found");
      return false;
    }

    // FIXME - Could blow up spectacularly, how to defend?
    (*func)();

    debugMsg("DynamicLoader:initModule",
	     " for module " << moduleName << " succeeded");
    return true;
  }

  static const char* LIBRARY_EXTENSIONS[] = {".so", ".dylib", NULL};

  void *DynamicLoader::loadLibrary(const char *libName)
  {
    // Try path verbatim
    void *handle = dlopen(libName, RTLD_NOW | RTLD_GLOBAL);
    if (handle) {
      debugMsg("DynamicLoader:loadLibrary",
	       " dlopen succeeded for " << libName);
      return handle;
    }

    debugMsg("DynamicLoader:verboseLoadLibrary",
	     " dlopen failed on file " << libName << ": " << dlerror());
    // Try adding the appropriate extension
    size_t i = 0;
    while (!handle && LIBRARY_EXTENSIONS[i]) {
      std::string libPath = libName;
      libPath += LIBRARY_EXTENSIONS[i++];
      handle = dlopen(libPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
      if (handle) {
	debugMsg("DynamicLoader:loadLibrary",
		 " dlopen succeeded for " << libPath);
	return handle;
      }
      else {
	debugMsg("DynamicLoader:verboseLoadLibrary",
		 " dlopen failed on file " << libPath << ": " << dlerror());
      }
    }

    debugMsg("DynamicLoader:loadLibrary",
	     " unable to open library \"" << libName << "\"");
    return NULL;
  }
}
