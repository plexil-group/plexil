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

#include "AdapterExecInterface.hh"

#include "Debug.hh"
#include "Expression.hh"
#include "State.hh"
#include "Value.hh"

#include <cstring>
#include <cstdlib> // for strtod(), strtoll()
#include <iomanip>
#include <sstream>

namespace PLEXIL
{

  /**
   * @brief Default constructor method.
   */
  AdapterExecInterface::AdapterExecInterface()
  {
  }

  /**
   * @brief Destructor method.
   */
  AdapterExecInterface::~AdapterExecInterface()
  {
  }

  //
  // Static utility functions
  //

  std::string AdapterExecInterface::getText(const State& c) 
  {
    std::ostringstream retval;
    retval << c.name() << "(";
    std::vector<Value>::const_iterator it = c.parameters().begin();
    if(it != c.parameters().end()) {
      retval << *it;
      for (++it; it != c.parameters().end(); ++it)
        retval << ", " << *it;
    }
    retval << ")";
    return retval.str();
  }

  Value AdapterExecInterface::stringToValue(const char * rawValue)
  {
    // null / empty check first
    if (rawValue == 0) {
      debugMsg("ExternalInterface:stringToValue",
               " raw value is null pointer");
      return Value();
    }
    else if (strlen(rawValue) == 0) {
      debugMsg("ExternalInterface:stringToValue",
               " raw value is empty string");
      return Value();
    }

    debugMsg("ExternalInterface:stringToValue", " input string = \"" << rawValue << "\"");

    if (*rawValue != '\0') {
      // try converting to integer first
      char * endptr;
      long longResult = strtol(rawValue, &endptr, 10);
      if (*endptr == '\0') {
        // string is valid integer
        debugMsg("ExternalInterface:stringToValue", " result is integer " << longResult);
        return Value((double) longResult);
      }

      debugMsg("ExternalInterface:stringToValue", " result is not valid integer");

      // string is not valid integer --
      // try converting to double
      double doubleResult = strtod(rawValue, &endptr);
      if (*endptr == '\0') {
        // string is valid double
        debugMsg("ExternalInterface:stringToValue", " result is double " << doubleResult);
        return Value(doubleResult);
      }

      debugMsg("ExternalInterface:stringToValue", " result is not valid double");
    }

    // if all else fails, must be a string
    return Value(rawValue);
  }

}
