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

// Functions shared by several formatter classes in this directory.

#include "event-support.hh"
#include "LabelStr.hh"
#include <string>

namespace PLEXIL 
{

using std::string;
using std::ostringstream;

  //
  // A quick, simple, composable function to generate an XML element
  // string with up to 9 content elements/strings.
  //
string element (const string& name,
                const string& c1,
                const string& c2,
                const string& c3,
                const string& c4,
                const string& c5,
                const string& c6,
                const string& c7,
                const string& c8,
                const string& c9)
{
  // optimization for empty element
  if (c1.empty() && c2.empty() && c3.empty()
      && c4.empty() && c5.empty() && c6.empty()
      && c7.empty() && c8.empty() && c9.empty())
    return "<" + name + "/>";

  return "<" + name + ">"
    + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9
    + "</" + name + ">" ;
}

// specialization for doubles (e.g. time)
// *** do label strings come through this path?
template<> string to_string (double x)
{
  //
  // Hack (?) to obtain string representation of objects.
  //
  if (LabelStr::isString(x))
    {
      return string(LabelStr(x).toString());
    }

  ostringstream s;
  //  s << setprecision(15) << x;
  s << x;
  return s.str();
}

string event_id ()
{
  //
  // Generate a unique ID.
  //
  static int id = 0;
  return to_string<int> (id++);
}

}
