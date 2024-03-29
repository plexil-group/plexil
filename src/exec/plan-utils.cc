/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#include "plan-utils.hh"

#include <iostream>
#include <sstream> // std::ostringstream

using std::vector;
using std::cout;
using std::flush;

namespace PLEXIL {

  static void print_aux (std::ostream & s, const vector<Value>& args, bool pretty)
  {
    for (vector<Value>::const_iterator iter = args.begin();
         iter != args.end();
         ++iter) {
      s << *iter;
      s << (pretty ? " " : "");
    }
    if (pretty)
      s << '\n';
    s << flush;
  }

  static Value printToString_aux (const vector<Value>& args, bool pretty)
  {
    std::ostringstream ss;
    print_aux(ss, args, pretty);
    return Value(ss.str());
  }

  void print (const vector<Value>& args)
  {
    print_aux (cout, args, false);
  }

  void pprint (const vector<Value>& args)
  {
    print_aux (cout, args, true);
  }
  
  Value printToString(const std::vector<Value>& args)
  {
    return printToString_aux(args, false);
  }

  Value pprintToString(const std::vector<Value>& args)
  {
    return printToString_aux(args, true);
  }

}
