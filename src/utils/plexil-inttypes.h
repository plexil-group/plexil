/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_INTTYPES_H
#define PLEXIL_INTTYPES_H

/**
 * \file plexil-inttypes.h
 * \brief Deals with complexities related to C(++) standard versions and compiler quirks
 *        around the C standard inttypes.h include file.
 * \note Search on __STDC_LIMIT_MACROS, __STDC_CONSTANT_MACROS, __STDC_FORMAT_MACROS
 *       to understand the issues being abstracted away here.
 * \ingroup Utils
 */

#include "plexil-config.h"

#if defined(__cplusplus) && defined(HAVE_INTTYPES_H) && !defined(HAVE_GOOD_INTTYPES_H)
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#elif defined(__cplusplus) && defined(HAVE_CINTTYPES)
#include <cinttypes>
#elif defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif

#endif /* PLEXIL_INTTYPES_H */
