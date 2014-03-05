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

//
// Abstracting out the various kinds of C++ hash tables for basic uses
//

#ifndef PLEXIL_GENERIC_HASH_MAP_HH
#define PLEXIL_GENERIC_HASH_MAP_HH

#include "plexil-config.h"

#if defined(HAVE_UNORDERED_MAP)
// C++11 unordered_map
#include <unordered_map>
#define PLEXIL_HASH_MAP(key_t,item_t) std::unordered_map<key_t, item_t >
#elif defined(HAVE_TR1_UNORDERED_MAP) 
// C++0x TR1 unordered_map
#include <tr1/unordered_map>
#define PLEXIL_HASH_MAP(key_t,item_t) std::tr1::unordered_map<key_t, item_t >
#elif defined(HAVE_EXT_HASH_MAP) || defined(HAVE_BACKWARD_HASH_MAP)
// GNU libstdc++ hash_map
#include "GNU_hash_map.hh"
#define PLEXIL_HASH_MAP(key_t,item_t) __gnu_cxx::hash_map<key_t, item_t >
#elif defined(HAVE_HASH_MAP)
// Dinkumware or original SGI hash_map
#include "Dinkum_hash_map.hh"
#define PLEXIL_HASH_MAP(key_t,item_t) std::hash_map<key_t, item_t, PLEXIL::hash_compare<key_t> >
#else
# error "Unable to find hash_map or equivalent class for this platform."
#endif

#endif // PLEXIL_GENERIC_HASH_MAP_HH
