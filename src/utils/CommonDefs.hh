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

#ifndef _H_CommonDefs
#define _H_CommonDefs

#include <stdint.h> // for int32_t
#include <cassert>

#ifdef __BEOS__
#include <debugger.h>
#endif

#include <sstream>

#include "Error.hh"


/**
 * @file CommonDefs.hh
 * @brief Declares a variety of common utilities
 * @author unknown
 * @ingroup Utils
 */

/**
 * @def DECLARE_STATIC_CLASS_CONST(TYPE,NAME)
 * @brief Declare and define class scoped constant to ensure initialization
 * occurs before use with all linkers.
 */
#define DECLARE_STATIC_CLASS_CONST(TYPE, NAME, VALUE) \
  static const TYPE& NAME() { \
    static const TYPE *sl_data = NULL; \
    if (sl_data == NULL) \
      sl_data = new const TYPE(VALUE); \
    return *sl_data; \
  }

/**
 * @def DECLARE_GLOBAL_CONST(TYPE,NAME)
 * @brief Declare a global constant via a global function to ensure initialization
 * occurs before use with all linkers.
 */
#define DECLARE_GLOBAL_CONST(TYPE, NAME) \
  extern const TYPE& NAME();

/**
 * @def DEFINE_GLOBAL_CONST(TYPE,NAME,VALUE)
 * @brief Define a global constant to have the given value via a
 * global function to ensure initialization occurs before use with all
 * linkers.
 */
#define DEFINE_GLOBAL_CONST(TYPE, NAME, VALUE) \
  const TYPE& NAME() { \
    static const TYPE *sl_data = NULL; \
    if (sl_data == NULL) \
      sl_data = new TYPE(VALUE); \
    return *sl_data; \
  }

/**
 * @def DEFINE_GLOBAL_EMPTY_CONST(TYPE,NAME)
 * @brief Define a global constant via a global function to ensure
 * initialization occurs before use with all linkers.
 */
#define DEFINE_GLOBAL_EMPTY_CONST(TYPE, NAME) \
  const TYPE& NAME() { \
    static const TYPE *sl_data = NULL; \
    if (sl_data == NULL) \
      sl_data = new const TYPE(); \
    return *sl_data; \
  }

namespace PLEXIL {
  DECLARE_GLOBAL_CONST(int32_t, g_maxInt);
  DECLARE_GLOBAL_CONST(int32_t, g_infiniteTime);
  DECLARE_GLOBAL_CONST(double, g_epsilon);
  DECLARE_GLOBAL_CONST(double, g_maxReal);
}

#define MAX_INT (PLEXIL::g_maxInt())

#define PLUS_INFINITY (PLEXIL::g_infiniteTime())

#define MINUS_INFINITY (-PLEXIL::g_infiniteTime())

#define REAL_PLUS_INFINITY (PLEXIL::g_maxReal())

#define REAL_MINUS_INFINITY (-PLEXIL::g_maxReal())

/**
 * @def EPSILON
 * Used when computing differences and comparing real numbers:
 * smallest recognized increment.
 */
#define EPSILON (PLEXIL::g_epsilon())

#endif
