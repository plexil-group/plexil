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

//
// Where to find the GNU hash_map class
//

#ifndef PLEXIL_GNU_HASH_MAP_HH
#define PLEXIL_GNU_HASH_MAP_HH

// Which files are included is dependent on the version of the 
// GNU libstdc++ installation, not the compiler.
// The macro __GLIBCXX__ (or __GLIBCPP__ for older versions) contains that information.

#ifdef PLEXIL_ANDROID
// Oddball Android 2.2 (maybe earlier as well)
# define _GLIBCXX_PERMIT_BACKWARD_HASH 1
# include <ext/hash_map>
# include <backward/hash_fun.h>
#else

// Location of the required include files depends on the libstdc++ version.
#include <cstdlib>
# if defined (__GLIBCXX__)
// +++ ADD LATEST REVISIONS HERE +++
#  if __GLIBCXX__ >= 20080306 && __GLIBCXX__ != 20080519 && __GLIBCXX__ != 20080704
// GCC 4.3.0 (20080306) through at least 4.5.2
#   include <backward/hash_map>
#   include <backward/hash_fun.h>
#  else
// GCC 3.4.0 through 4.2.4 (20080519)
// Red Hat uses a bastard version of 4.1.2 (20080704)
#   include <ext/hash_map>
#   include <ext/hash_fun.h>
#  endif
# elif defined (__GLIBCPP__)
// GCC 3.00 through 3.3.6 (20050503)
#  include <ext/hash_map>
#  include <ext/stl_hash_fun.h>
# else
#  error "Unable to determine GNU libstdc++ version."
# endif // defined (__GLIBCXX__)
#endif

namespace __gnu_cxx
{

// Define hash functions for double and std::string

template <>
struct hash<std::string>
{
  size_t operator()(const std::string& __val) const
  {
    return hash<const char*>()(__val.c_str());
  }
};

template<>
struct hash<double>
{
  size_t
  operator()(double __x) const
  {
    // stolen from __stl_hash_string - see ext/hash_fun.h
    unsigned long __h = 0;
    const char* __s = reinterpret_cast<const char*>(&__x);
    for (size_t i = 0; i < sizeof(__x); ++i, ++__s)
      __h = 5 * __h + *__s;
    return size_t(__h);
  }
};

} // end namespace __gnu_cxx
    
#endif // PLEXIL_GNU_HASH_MAP_HH
