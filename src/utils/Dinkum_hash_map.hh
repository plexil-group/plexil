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

#ifndef DINKUM_HASH_MAP_HH
#define DINKUM_HASH_MAP_HH

#include <hash_map>
#include "hash-functions.h"

namespace PLEXIL {

  template <class T>
  inline size_t plexil_hash_value(const T& _Keyval)
  {
    return std::hash_value(_Keyval);
  }

  template <>
  inline size_t plexil_hash_value(const float& _Keyval)
  {
    return std::hash_value(reinterpret_cast<const uint32_t &>(_Keyval));
  }

  template <>
  inline size_t plexil_hash_value(const double& _Keyval)
  {
    return (size_t) HashDouble(&_Keyval); // Paul Hsieh's algorithm
  }

  // Override for const char *
  inline size_t plexil_hash_value(const char * _Keyval)
  {
    return (size_t) PaulHsiehHash(_Keyval, strlen(_Keyval));
  }

  // Override for string
  template <class _Elem,
	    class _Traits,
	    class _Alloc >
  inline size_t plexil_hash_value(const std::basic_string<_Elem, _Traits, _Alloc > &_Keyval)
  {
    return (size_t) PaulHsiehHash(reinterpret_cast<const char *>(_Keyval.c_str()),
				  sizeof(_Elem) * _Keyval.size());
  }

  // Wrapper around std::hash_compare to use our preferred hashing functions.
  template<class _Kty,
	   class _Pr = std::less<_Kty> >
  class hash_compare : public std::hash_compare<_Kty, _Pr >
  {	// traits class for hash containers
  public:
    // construct with default comparator
    hash_compare()
      : std::hash_compare<_Kty, _Pr >()
    {
    }

    // construct with _Pred comparator
    hash_compare(_Pr _Pred)
      : std::hash_compare<_Kty, _Pr> (_Pred)
    {
    }

    // Hashing operator
    inline size_t operator()(const _Kty& _Keyval) const
    {
      return plexil_hash_value(_Keyval);
    }

    // Comparison operator
    inline bool operator()(const _Kty& _Keyval1, const _Kty& _Keyval2) const
    {	// test if _Keyval1 ordered before _Keyval2
      return std::hash_compare<_Kty, _Pr>::operator()(_Keyval1, _Keyval2);
    }

  };

}

#endif // DINKUM_HASH_MAP_HH
