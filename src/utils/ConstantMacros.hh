/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

// Macros for defining global and class constants
#include "lifecycle-utils.h"

/**
 * @def DECLARE_STATIC_CLASS_CONST(TYPE, NAME, VALUE)
 * @brief Declare and define class scoped constant to ensure initialization
 * occurs before use with all linkers.
 */
#define DECLARE_STATIC_CLASS_CONST(TYPE, NAME, VALUE) \
  static const TYPE& NAME() { \
    static const TYPE sl_data(VALUE); \
    return sl_data; \
  }

/**
 * @def DECLARE_STATIC_CLASS_CONST_WITH_CLEANUP(TYPE, NAME, VALUE)
 * @brief Declare and define class scoped constant to ensure initialization
 * occurs before use with all linkers, and cleanup happens later.
 */
#define DECLARE_STATIC_CLASS_CONST_WITH_CLEANUP(TYPE, NAME, VALUE) \
  static TYPE* ensure__ ## NAME() { \
    static TYPE *sl_ptr; \
    static bool sl_inited; \
    if (!sl_inited) { \
      sl_ptr = new TYPE(VALUE); \
      sl_inited = true; \
      addFinalizer(&NAME ## __destroy); \
    } \
    return sl_ptr; \
  } \
  static void NAME ## __destroy() { \
    delete ensure__ ## NAME(); \
  } \
  static const TYPE& NAME() { \
    return *ensure__ ## NAME(); \
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
    static const TYPE sl_data(VALUE); \
    return sl_data; \
  }

/**
 * @def DEFINE_GLOBAL_CONST_WITH_CLEANUP(TYPE,NAME,VALUE)
 * @brief Define a global constant to have the given value via a
 * global function to ensure initialization occurs before use with all
 * linkers.
 */
#define DEFINE_GLOBAL_CONST_WITH_CLEANUP(TYPE, NAME, VALUE) \
  static void NAME ## __destroy(); \
  static TYPE* ensure__ ## NAME() { \
    static TYPE *sl_ptr; \
    static bool sl_inited; \
    if (!sl_inited) { \
      sl_ptr = new TYPE(VALUE); \
      sl_inited = true; \
      addFinalizer(&NAME ## __destroy); \
    } \
    return sl_ptr; \
  } \
  static void NAME ## __destroy() { \
    delete ensure__ ## NAME(); \
  } \
  const TYPE& NAME() { \
    return *ensure__ ## NAME(); \
  }

/**
 * @def DEFINE_GLOBAL_EMPTY_CONST(TYPE,NAME)
 * @brief Define a global constant via a global function to ensure
 * initialization occurs before use with all linkers.
 */
#define DEFINE_GLOBAL_EMPTY_CONST(TYPE, NAME) \
  const TYPE& NAME() { \
    static const TYPE sl_data; \
    return sl_data; \
  }


/**
 * @def DEFINE_GLOBAL_EMPTY_CONST_WITH_CLEANUP(TYPE,NAME)
 * @brief Define a global constant to have the given value via a
 * global function to ensure initialization occurs before use with all
 * linkers.
 */
#define DEFINE_GLOBAL_EMPTY_CONST_WITH_CLEANUP(TYPE, NAME) \
  static void NAME ## __destroy(); \
  static TYPE* ensure__ ## NAME() { \
    static TYPE *sl_ptr; \
    static bool sl_inited; \
    if (!sl_inited) { \
      sl_ptr = new TYPE(); \
      sl_inited = true; \
      addFinalizer(&NAME ## __destroy); \
    } \
    return sl_ptr; \
  } \
  static void NAME ## __destroy() { \
    delete ensure__ ## NAME(); \
  } \
  const TYPE& NAME() { \
    return *ensure__ ## NAME(); \
  }
