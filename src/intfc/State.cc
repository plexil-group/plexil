/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "State.hh"

#include "Error.hh" // assertTrue_2 macro

#include <ostream>
#include <sstream>

namespace PLEXIL
{

  State::State()
  {
  }

  State::State(State const &other)
    : m_name(other.m_name),
      m_parameters(other.m_parameters)
  {
  }

  State::State(char const *name, size_t n)
    : m_name(name),
      m_parameters(n)
  {
  }

  State::State(std::string const &name, size_t n)
    : m_name(name),
      m_parameters(n)
  {
  }

  State::State(std::string const &name, Value const &arg0)
    : m_name(name),
      m_parameters(1, arg0)
  {
  }

  State::State(std::string const &name, std::vector<Value> const &args)
    : m_name(name),
      m_parameters(args)
  {
  }


#if __cplusplus >= 201103L
  //
  // Move constructors
  //
  State::State(State &&other)
    : m_name(std::move(other.m_name)),
      m_parameters(std::move(other.m_parameters))
  {
  }

  State::State(std::string &&name, size_t n)
    : m_name(name),
      m_parameters(n)
  {
  }

  State::State(std::string &&name, Value const &arg0)
    : m_name(name),
      m_parameters(1, arg0)
  {
  }

  State::State(std::string const &name, Value &&arg0)
    : m_name(name),
      m_parameters(1, arg0)
  {
  }

  State::State(std::string &&name, Value &&arg0)
    : m_name(name),
      m_parameters(1, arg0)
  {
  }

  State::State(std::string &&name, std::vector<Value> const &args)
    : m_name(name),
      m_parameters(args)
  {
  }

  State::State(std::string const &name, std::vector<Value> &&args)
    : m_name(name),
      m_parameters(args)
  {
  }

  State::State(std::string &&name, std::vector<Value> &&args)
    : m_name(name),
      m_parameters(args)
  {
  }
#endif

  State::~State()
  {
  }

  State &State::operator=(State const &other)
  {
    m_name = other.m_name;
    m_parameters = other.m_parameters;
    return *this;
  }

#if __cplusplus >= 201103L
  State &State::operator=(State &&other)
  {
    m_name = std::move(other.m_name);
    m_parameters = std::move(other.m_parameters);
    return *this;
  }
#endif

  std::string const &State::name() const
  {
    return m_name;
  }

  std::vector<Value> const &State::parameters() const
  {
    return m_parameters;
  }

  size_t State::parameterCount() const
  {
    return m_parameters.size();
  }
   
  bool State::isParameterKnown(size_t n) const
  {
    if (n < m_parameters.size())
      return m_parameters[n].isKnown();
    return false;
  }

  ValueType State::parameterType(size_t n) const
  {
    if (n < m_parameters.size())
      return m_parameters[n].valueType();
    return UNKNOWN_TYPE;
  }

  Value const &State::parameter(size_t n) const
  {
    if (n < m_parameters.size())
      return m_parameters[n];

    // FIXME: This should be a global constant
    static Value const sl_unknown;
    return sl_unknown;
  }

  void State::setName(std::string const &name)
  {
    m_name = name;
  }

  void State::setParameterCount(size_t n)
  {
    m_parameters.resize(n);
  }

  void State::setParameter(size_t i, Value const &val)
  {
    assertTrue_2(i < m_parameters.size(), "State::setParameter: index out of range");
    m_parameters[i] = val;
  }

  void State::print(std::ostream &str) const
  {
    str << m_name << '(';
    size_t i = 0;
    while (i < m_parameters.size()) {
      str << m_parameters[i];
      if (++i < m_parameters.size())
        str << ", ";
    }
    str << ')';
  }

  std::string State::toString() const
  {
    std::ostringstream strm;
    print(strm);
    return strm.str();
  }

  std::ostream &operator<<(std::ostream &str, State const &state)
  {
    state.print(str);
    return str;
  }

  char *State::serialize(char *buf) const
  {
    *buf++ = STATE_TYPE;
    buf = PLEXIL::serialize(m_name, buf);
    // Put 3 bytes of parameter count
    size_t siz = m_parameters.size();
    *buf++ = (char) (0xFF & (siz >> 16));
    *buf++ = (char) (0xFF & (siz >> 8));
    *buf++ = (char) (0xFF & siz);
    for (size_t i = 0; buf != NULL && i < siz; ++i)
      buf = PLEXIL::serialize(m_parameters[i], buf);
    return buf;
  }

  template <>
  char *serialize<State>(State const &val, char *buf)
  {
    return val.serialize(buf);
  }

  char const *State::deserialize(char const *buf)
  {
    if (STATE_TYPE != (ValueType) *buf++)
      return NULL;
    buf = PLEXIL::deserialize(m_name, buf);
    // Get parameter count
    size_t siz = ((size_t) (unsigned char) *buf++) << 8;
    siz = (siz + (size_t) (unsigned char) *buf++) << 8;
    siz = siz + (size_t) (unsigned char) *buf++;
    m_parameters.resize(siz);
    for (size_t i = 0; buf != NULL && i < siz; ++i)
      buf = PLEXIL::deserialize(m_parameters[i], buf);
    return buf;
  }

  template <>
  char const *deserialize<State>(State &val, char const *buf)
  {
    return val.deserialize(buf);
  }

  size_t State::serialSize() const
  {
    size_t result = 4 + PLEXIL::serialSize(m_name);
    for (size_t i = 0; i < m_parameters.size(); ++i)
      result += PLEXIL::serialSize(m_parameters[i]);
    return result;
  }

  template <>
  size_t serialSize<State>(State const &val)
  {
    return val.serialSize();
  }

  bool operator==(State const &sta, State const &stb)
  {
    return sta.name() == stb.name()
      && sta.parameters() == stb.parameters();
  }


  bool operator<(State const &sta, State const &stb)
  {
    if (sta.m_name < stb.m_name)
      return true;
    if (sta.m_name > stb.m_name)
      return false;
    // Same name
    size_t aSize = sta.m_parameters.size();
    if (aSize < stb.m_parameters.size())
      return true;
    if (aSize > stb.m_parameters.size())
      return false;
    // Same # params
    for (size_t i = 0; i < sta.parameterCount(); ++i) {
      Value const &aval = sta.m_parameters[i];
      Value const &bval = stb.m_parameters[i];
      if (aval < bval)
        return true;
      if (aval > bval)
        return false;
    }
    return false; // states are equal
  }

  // Global "constant"
  State const &State::timeState()
  {
    static State const sl_timeState("time");
    return sl_timeState;
  }

} // namespace PLEXIL
