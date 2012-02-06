/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

#include "Expression.hh"
#include "LabelStr.hh"
#include "StoredArray.hh"
#include "types.hh"

using std::string;
using std::vector;
using PLEXIL::LabelStr;

// Simple types have very similar enconding and decoding functions, so using a
// macro to define them.

#define defSimpleType(type, ctype) \
type encode##type (ctype x) \
{ \
  return (type) x; \
} \
ctype decode##type (type x) \
{ \
  return (ctype) x;  \
} 

defSimpleType(Int, int)
defSimpleType(Real, double)
defSimpleType(Bool, bool)


// The remaining types are handled individually.

String encodeString (const string& x)
{
  return LabelStr (x);
}

string decodeString (String x)
{
  return string (LabelStr(x).toString());
}

Array encodeArray (const vector<Any>& x)
{
  // This should be memory-safe.
  PLEXIL::StoredArray arr (x);
  return arr.getKey();
}

vector<Any>& decodeArray (Array x)
{
  return PLEXIL::StoredArray(x).getArray();
}
