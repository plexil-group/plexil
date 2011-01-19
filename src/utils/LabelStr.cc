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

#include "LabelStr.hh"
#include "Error.hh"
#include <string.h>
#include <limits>

namespace PLEXIL
{

   
   //to handle creation of the UNKNOWN label string
   class LabelStrLocalStatic 
   {
      public:
         LabelStrLocalStatic()
         {
            static bool sl_called = false;

            //check_error(sl_called, "Should only register UNKNOWN string once.");
            if(!sl_called) 
            {
	      StoredString::insertItemAtKey(new std::string("UNKNOWN"),
					    KeySource<double>::infinity());
	      sl_called = true;
            }
         }
   };
   
   // create the UNKNOWN label string

   LabelStrLocalStatic s_labelStr;

   //LabelStr Unknown("UNKNOWN");

   // define the empty label

   DEFINE_GLOBAL_CONST(LabelStr, EMPTY_LABEL, "");
   DEFINE_GLOBAL_CONST(LabelStr, UNKNOWN_STR, KeySource<double>::infinity());

   LabelStr::LabelStr()
   {
#ifndef PLEXIL_FAST
      m_chars = empty().c_str();
#endif
   }

   /**
    * Construction must obtain a key that is efficient to use for later
    * calculations in the domain and must maintain the ordering defined
    * by the strings.
    */
   LabelStr::LabelStr(const std::string& label) : 
      StoredString(&label, true)
   {
#ifndef PLEXIL_FAST
      m_chars = label.c_str();
#endif
   }

   LabelStr::LabelStr(const char* label):
      StoredString(new std::string(label), false)
   {
#ifndef PLEXIL_FAST
      m_chars = label;
#endif
   }

   LabelStr::LabelStr(double key):
      StoredString(key)
   {
      check_error(isKey(key), "Invalid key provided.");

#ifndef PLEXIL_FAST
      m_chars = toString().c_str();
#endif
   }

   const std::string& LabelStr::toString() const
   {
      return getItem();
   }

   const char* LabelStr::c_str() const
   {
      return toString().c_str();
   }

#ifndef PLEXIL_FAST

   LabelStr::LabelStr(const LabelStr& org):
      StoredString(org.getKey())
   {
      m_chars = org.m_chars;
   }

   LabelStr::operator double () const
   {
      return getKey();
   }

#endif

   bool LabelStr::operator<(const LabelStr& lbl) const
   {
      return toString() < lbl.toString();
   }

   bool LabelStr::operator>(const LabelStr& lbl) const
   {
      return toString() > lbl.toString();
   }

//    bool LabelStr::operator==(const LabelStr& lbl) const
//    {
//       return getKey() == lbl.getKey();
//    }

   unsigned int LabelStr::getSize()
   {
      return StoredString::getSize();
   }

   bool LabelStr::isString(double key)
   {
      return StoredString::isKey(key);
   }

   bool LabelStr::isString(const std::string& candidate)
   {
      return StoredString::isItem(&candidate);
   }

   bool LabelStr::contains(const LabelStr& lblStr) const
   {
      const std::string& thisStr = toString();
      int index = thisStr.find(lblStr.c_str());
      return (index >= 0);
   }


   unsigned int LabelStr::countElements(const char* delimiter) const
   {
      check_error(delimiter != NULL && delimiter != 0 && delimiter[0] != '\0', "'NULL' and '\\0' are not valid delimiters");

      //allocate a results vector
      std::vector<std::string> tokens;

      // Get a std string from the LabelStr
      const std::string& srcStr = toString();

      //create a std string of the delimiter
      std::string delim(delimiter);

      tokenize(srcStr, tokens, delim);

      return tokens.size();
   }

   LabelStr LabelStr::getElement(unsigned int index, const char* delimiter) const
   {
      check_error(delimiter != NULL && delimiter != 0 && delimiter[0] != '\0', "'NULL' and '\\0' are not valid delimiters");

      //allocate a results vector
      std::vector<std::string> tokens;

      // Get a std string from the LabelStr
      const std::string& srcStr = toString();

      //create a std string of the delimiter
      std::string delim(delimiter);

      tokenize(srcStr, tokens, delim);

      LabelStr result(tokens[index]);

      return result;
   }


  /**
   * @brief Utility function to tokenzie a std string 
   */

  void LabelStr::tokenize(const std::string& str, 
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

}
