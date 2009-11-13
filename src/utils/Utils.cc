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

// N.B.: This #include *must* come before any other #include statements!
#include "Utils.hh"

#include <sstream>

DEFINE_GLOBAL_CONST(bool, g_alwaysFails, false);

namespace PLEXIL {

  std::string toString(double value) {
    std::stringstream s;
    s << value;
    return(s.str());
  }

   bool compareIgnoreCase(const std::string & s1,
                          const std::string & s2)
   {
     std::string::size_type s1_size = s1.size();
     if (s1_size != s2.size())
       return false;

     // s1 is known to be same length as s2 at this point
     for (std::string::size_type i = 0; i < s1_size; ++i)
       if (tolower(s1[i]) != tolower(s2[i]))
         return false;

     return true;
   }

  void tokenize(const std::string& str,
		std::vector<std::string>& tokens,
		const std::string& delimiters)  {
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
      {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
      }
  }


  //
  // *** BEWARE!! ***
  // INT_MAX != LONG_MAX on most 64-bit systems!
  // The integer quantities here are meant to be 32 bit!
  // 
  DEFINE_GLOBAL_CONST(int32_t, g_maxInt, (INT32_MAX/8)); /*!< Same as max finite time */
  DEFINE_GLOBAL_CONST(int32_t, g_infiniteTime, (g_maxInt() + 1));
  DEFINE_GLOBAL_CONST(int32_t, g_noTime, 0);
  DEFINE_GLOBAL_CONST(double, g_epsilon, 0.00001);
  DEFINE_GLOBAL_CONST(double, g_maxReal, ((double) DBL_MAX));
}
