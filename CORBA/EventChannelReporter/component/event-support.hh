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

#ifndef Event_Support_H
#define Event_Support_H

#include <string>
#include <sstream>

namespace PLEXIL {

// A quick, simple, composable function to generate an XML element
// string with up to 9 content elements/strings.

std::string element (const std::string& name,
                     const std::string& c1 = "",
                     const std::string& c2 = "",
                     const std::string& c3 = "",
                     const std::string& c4 = "",
                     const std::string& c5 = "",
                     const std::string& c6 = "",
                     const std::string& c7 = "",
                     const std::string& c8 = "",
                     const std::string& c9 = "");

// Hack (?) to obtain string representation of objects.
template<class T> std::string to_string (T x)
{
  std::ostringstream s;
  s << x;
  return s.str();
}

// Specialization for doubles (e.g. time)
template<> std::string to_string (double x);

// Return a unique ID.
std::string event_id ();

}

#endif
