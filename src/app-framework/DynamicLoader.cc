/*
 * DynamicLoader.cc
 *
 *  Created on: Jan 27, 2010
 *      Author: jhogins
 */

#include <dlfcn.h>
#include "DynamicLoader.hh"

namespace PLEXIL
{
  void *DynamicLoader::getDynamicSymbol(const char* libPath, const char* symbol) {
    //test to see if it is loaded already
    void *handle = dlopen(libPath, RTLD_NOLOAD);
    void *func;
    if (!handle) //load if not already
      handle = dlopen(libPath, RTLD_NOW);
    if (!handle) //an error occured in loading the library
      return 0;
    func = dlsym(handle, symbol);
    if (!func) //an error occured in loading the function
      return 0;
    return func;
  }


  char *DynamicLoader::getError() {
    return dlerror();
  }
}
