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

namespace PLEXIL
{
  class DynamicLoader {
  public:
    /**
     * @breif Executes the given function in the given dynamic library (loaded and cached
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
     * @breif Returns a human readable string describing the most recent error that
     * occurred from runDynamicFunction.
     * @return NULL if no errors have occured since startup; a human readable string
     * describing the most recent error otherwise.
     */
    static char *getError();

  };

}

#endif /* DYNAMICLOADER_HH_ */
