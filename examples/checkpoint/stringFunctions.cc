/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

// Functions for manipulating strings in StringAdapter

#include "parser-utils.hh"
#include "stricmp.h"
#include "Value.hh"

#include <algorithm> 
#include <limits>
#include <sstream>

using std::cerr;
using std::endl;
using std::numeric_limits;
using std::ostringstream;
using std::string;
using std::transform;
using std::vector;

using PLEXIL::Boolean;
using PLEXIL::BOOLEAN_TYPE;
using PLEXIL::Integer;
using PLEXIL::INTEGER_TYPE;
using PLEXIL::Real;
using PLEXIL::REAL_TYPE;
using PLEXIL::Value;

Value toStringFunction(const vector<Value> &args)
{
  switch (args.size()) {
  case 0:
    return Value("");
  case 1:
    return Value(args[0].valueToString());

  default:
    {
      ostringstream s;
      for (Value const &v : args)
        v.printValue(s);
      return Value(s.str());
    }
  }
}

Value stringToIntegerFunction(const vector<Value> &args)
{
  if (args.size() == 1) {
    if (args[0].valueType() == INTEGER_TYPE) {
      return Value(args[0]);
    }
    const string s = args[0].valueToString();
    try {
      long n = std::stol(s);
      if (n >= numeric_limits<Integer>::min()
          && n <= numeric_limits<Integer>::max()) {
        return Value((Integer) n);
      }
      else {
        cerr << "StringToInteger: \"" << s << "\" is out of range for an Integer"
             << endl;
        return Value();
      }
    }
    catch (std::invalid_argument const & /* exc */) {
      cerr << "StringToInteger: \"" << s << "\" cannot be parsed as an Integer"
           << endl;
      return Value();
    }
    catch (std::out_of_range const & /* exc */) {
      cerr << "StringToInteger: \"" << s << "\" is out of range for an Integer"
           << endl;
      return Value();
    }
  }
  else {
    cerr << "StringToInteger: expected 1 argument, received " << args.size()
         << endl;
    return Value();
  }
}

Value stringToRealFunction(const vector<Value> &args)
{
  if (args.size() == 1) {
    if (args[0].valueType() == REAL_TYPE) {
      return Value(args[0]);
    }
    const string s = args[0].valueToString();
    try {
      return Value(std::stod(s));
    }
    catch (std::invalid_argument const & /* exc */) {
      cerr << "StringToReal: \"" << s << "\" cannot be parsed as a Real"
           << endl;
      return Value();
    }
    catch (std::out_of_range const & /* exc */) {
      cerr << "StringToReal: \"" << s << "\" is out of range for a Real"
           << endl;
      return Value();
    }
  }
  else {
    cerr << "StringToReal: expected 1 argument, received " << args.size()
         << endl;
    return Value();
  }
}

Value stringToBooleanFunction(const vector<Value> &args)
{
  if (args.size() == 1) {
    if (args[0].valueType() == BOOLEAN_TYPE) {
      return Value(args[0]);
    }
    const string s = args[0].valueToString();
    if (s == "1" || !stricmp(s.c_str(), "true"))
      return Value(true);
    else if (s == "0" || !stricmp(s.c_str(), "false"))
      return Value(false);
    else {
      cerr << "StringToBoolean: \"" << s << "\" cannot be parsed as a Boolean"
           << endl;
      return Value();
    }
  }
  else {
    cerr << "StringToReal: expected 1 argument, received " << args.size()
         << endl;
    return Value();
  }
}

Value substrFunction(const vector<Value> &args)
{
  if (args.size() < 1 || args.size() > 3) {
    cerr << "substr: expected 1 to 3 arguments, received " << args.size()
         << endl;
    return Value();
  }
  const string s = args[0].valueToString();
  Integer pos = 0;
  Integer count = s.size();
  if (args.size() > 1) {
    if (args[1].getValue(pos)) {
      if (pos < 0) {
        cerr << "substr: Illegal negative value " << args[1] << " for position argument"
             << endl;
        return Value();
      }
      else if (pos > count) {
        cerr << "substr: Position argument " << args[1] << " is greater than length of string"
             << s.size() << endl;
        return Value();
      }
    }
    else {
      cerr << "substr: The position argument, \"" << args[2] << "\", was not an Integer"
           << endl;
      return Value();
    }
    if (args.size() > 2) {
      if (args[2].getValue(count)) {
        if (pos < 0) {
          cerr << "substr: Illegal negative value " << args[2] <<  " for count argument"
               << endl;
          return Value();
        }
      }
      else {
        cerr << "substr: The count argument, \"" << args[2] << "\", was not an Integer"
             << endl;
        return Value();
      }
    }
  }
  return Value(s.substr(pos, count));
}

Value strlwrFunction(const vector<Value> &args)
{
  if (args.size() != 1) {
    cerr << "strlwr: expected 1 argument, received " << args.size() << endl;
    return Value();
  }
  string data = args[0].valueToString();
  std::transform(data.begin(), data.end(), data.begin(), ::tolower);
  return Value(data);
}

Value struprFunction(const vector<Value> &args)
{
  if (args.size() != 1) {
    cerr << "strupr: expected 1 argument, received " << args.size() << endl;
    return Value();
  }
  string data = args[0].valueToString();
  std::transform(data.begin(), data.end(), data.begin(), ::toupper);
  return Value(data);
}

// strindex(s,i,[v]) acts the same as s[i] = v, or s[i] if v is not specified
Value strindexFunction(const vector<Value> &args)
{
  if (args.size () < 2 || args.size() > 3) {
    cerr << "strindex: Expected 2 or 3 arguments, received " << args.size()
         << endl;
    return Value();
  }

  string data = args[0].valueToString();
  Integer pos;
  if (args[1].getValue(pos)) {
    if (pos < 0) {
      cerr << "strindex: Illegal negative value " << args[1] << " for position argument"
           << endl;
      return Value();
    }
    else if (pos > data.size()) {
      cerr << "strindex: Position argument " << args[1] << " is greater than length of string"
           << data.size() << endl;
      return Value();
    }
  }
  else {
    cerr << "strindex: The position argument, \"" << args[2] << "\", was not an Integer"
         << endl;
    return Value();
  }
  if (args.size() == 2)
    // Return the character at the index
    return Value(string(1, data[pos]));

  // Insert the 3rd argument at the index and return the result
  return Value(data.insert(pos, args[2].valueToString()));
}

Value find_first_of_Function(const vector<Value> &args)
{
  if (args.size () < 2 || args.size() > 3) {
    cerr << "find_first_of: Expected 2 or 3 arguments, received " << args.size()
         << endl;
    return Value();
  }

  const string data = args[0].valueToString();
  const string toSearchFor = args[1].valueToString();
  Integer pos = 0;
  if (args.size() == 3) {
    if (args[2].getValue(pos)) {
      if (pos < 0) {
        cerr << "find_first_of: Illegal negative value " << args[1] << " for position argument"
             << endl;
        return Value();
      }
      else if (pos > data.size()) {
        cerr << "find_first_of: Position argument " << args[1] << " is greater than length of string"
             << data.size() << endl;
        return Value();
      }
    }
  }
  return Value((Integer) data.find_first_of(toSearchFor,pos));
}

Value find_last_of_Function(const vector<Value> &args)
{
  if (args.size () < 2 || args.size() > 3) {
    cerr << "find_last_of: Expected 2 or 3 arguments, received " << args.size()
         << endl;
    return Value();
  }

  const string data = args[0].valueToString();
  const string toSearchFor = args[1].valueToString();
  Integer pos = 0;
  if (args.size() == 3) {
    if (args[2].getValue(pos)) {
      if (pos < 0) {
        cerr << "find_last_of: Illegal negative value " << args[1] << " for position argument"
             << endl;
        return Value();
      }
      else if (pos > data.size()) {
        cerr << "find_last_of: Position argument " << args[1] << " is greater than length of string"
             << data.size() << endl;
        return Value();
      }
    }
  }
  return Value((Integer) data.find_last_of(toSearchFor,pos));
}
