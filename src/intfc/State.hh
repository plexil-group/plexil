// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_STATE_HH
#define PLEXIL_STATE_HH

#include "Value.hh"

namespace PLEXIL
{
  //! \class State
  //! \brief Represents the ground values at a particular instant
  //!        of the name and arguments of a Lookup or Command. 
  //! \ingroup External-Interface
  class State final
  {
  public:
    //! \brief Default constructor.
    State();

    //! \brief Copy constructor.
    //! \param orig Const reference to the State to copy.
    State(State const &orig);

    //! \brief Move constructor.
    //! \param orig Rvalue reference to the State to copy.
    State(State &&orig);

    //! \brief Constructor from a null terminated state name and number of arguments.
    //! \param name Const pointer to the name.
    //! \param n The number of arguments.
    State(char const *name, size_t n = 0);

    //! \brief Constructor from a std::string state name and number of arguments.
    //! \param name Const reference to the name.
    //! \param n The number of arguments.
    State(std::string const &name, size_t n = 0);

    //! \brief Move constructor from a std::string and a number of arguments.
    //! \param name Lvalue reference to the name string.
    //! \param n The number of arguments.
    State(std::string &&name, size_t n = 0);

    //
    // Convenience variants
    //

    //! \brief Construct a State with one argument from a state name and the argument value. 
    //! \param name Const reference to the name.
    //! \param arg0 Const reference to the Value.
    State(std::string const &name, Value const &arg0);

    //! \brief Move constructor from a std::string and one argument value.
    //! \param name Lvalue reference to the name string.
    //! \param arg0 Const reference to the argument Value.
    State(std::string &&name, Value const &arg0);

    //! \brief Construct a State with one argument from a state name and the argument value. 
    //! \param name Const reference to the name.
    //! \param arg0 Const reference to the first Value.
    //! \param arg1 Const reference to the second Value.
    State(std::string const &name, Value const &arg0, Value const &arg1);

    //! \brief Construct a State with one argument from a state name and the argument value. 
    //! \param name Lvalue reference to the name.
    //! \param arg0 Const reference to the first Value.
    //! \param arg1 Const reference to the second Value.
    State(std::string &&name, Value const &arg0, Value const &arg1);

    //! \brief Construct a State from a state name and a vector of argument values.
    //! \param name Const reference to the name.
    //! \param args Const reference to the vector of Value instances.
    State(std::string const &name, std::vector<Value> const &args);

    //! \brief Move constructor from a std::string and a vector of argument values.
    //! \param name Lvalue reference to the name string.
    //! \param arg0 Const reference to the vector of Value instances.
    State(std::string &&name, std::vector<Value> const &args);

    //! \brief Move constructor from a std::string and a vector of argument values.
    //! \param name Const reference to the name string.
    //! \param arg0 Lvalue reference to the vector of Value instances.
    State(std::string const &name, std::vector<Value> &&args);

    //! \brief Move constructor from a std::string and a vector of argument values.
    //! \param name Lvalue reference to the name string.
    //! \param arg0 Lvalue reference to the vector of Value instances.
    State(std::string &&name, std::vector<Value> &&args);

    //! \brief Destructor.
    ~State() = default;

    //! \brief Copy assignment operator.
    //! \param other Const reference to the State being copied.
    State &operator=(State const &other);
 
    //! \brief Move assignment operator.
    //! \param other Lvalue reference to the other State.
    State &operator=(State &&other);

    //! \brief Get the name of the State.
    //! \return Const reference to the name.
    std::string const &name() const;

    //! \brief Get the State's parameters.
    //! \return Const reference to the parameter vector.
    std::vector<Value> const &parameters() const;

    //! \brief Get the number of parameters.
    //! \return The count.
    size_t parameterCount() const;

    //! \brief Is the requested parameter known?
    //! \param n The parameter index.
    //! \return true if the parameter is known, false if unknown.
    bool isParameterKnown(size_t n) const;

    //! \brief Get the ValueType of the requested parameter.
    //! \param n The parameter index.
    //! \return The ValueType.
    ValueType parameterType(size_t n) const;

    //! \brief Get the value of the requested parameter.
    //! \param n The parameter index.
    //! \return Const reference to the Value.
    Value const &parameter(size_t n) const;

    //! \brief Set this State's name.
    //! \param name Const reference to the new name string.
    void setName(std::string const &name);

    //! \brief Set the number of parameters of this State.
    //! \param n The new parameter count.
    void setParameterCount(size_t n);
    
    //! \brief Set the requested parameter to a new value.
    //! \param i The index of the parameter to set.
    //! \param val Const reference to the new Value.
    void setParameter(size_t i, Value const &val);

    //! \brief Print this State to an output stream.
    //! \param s Reference to the stream.
    void print(std::ostream &s) const;

    //! \brief Get a printed representation of this State as a string.
    //! \return The string.
    std::string toString() const;

    //! \brief Singleton accessor to the "time" state.
    //! \return Const reference to the state.
    static State const &timeState();

    //
    // Serialization support
    //

    //! \brief Write a serial representation of this object to the given
    //!        character array buffer.
    //! \param b First character of the buffer to write to.
    //! \return Pointer to the character after the last character written.
    char *serialize(char *b) const;

    //! \brief Read a serial representation from a buffer into this object.
    //! \param b Pointer to first character of the serial representation.
    //! \return Pointer to the character after the last character read.
    char const *deserialize(char const *b);

    //! \brief Get the number of bytes required by a serial
    //!        representation of this object.
    //! \return The size.
    size_t serialSize() const;

  private:

    friend bool operator<(State const &, State const &);

    //! \brief The state name.
    std::string m_name;

    //! \brief The vector of parameter values.
    std::vector<Value> m_parameters;
  };

  //! \brief Overloaded equality comparison operator.
  //! \param sta Const reference to a state.
  //! \param stb Const reference to another state.
  //! \return true if the two states are identical in name and parameters,
  //!         false if not.
  //! \ingroup External-Interface
  bool operator==(State const &, State const &);

  //! \brief Overloaded inequality comparison operator.
  //! \param a Const reference to a state.
  //! \param b Const reference to another state.
  //! \return false if the two states are identical in name and parameters,
  //!         true if not.
  //! \ingroup External-Interface
  inline bool operator!=(State const &a, State const &b)
  {
    return !(a == b);
  }

  //
  // Comparisons for use with (e.g.) std::map
  //

  //! \brief Overloaded less-than comparison operator.
  //! \param sta Const reference to a state.
  //! \param stb Const reference to another state.
  //! \return true if sta is strictly less than stb,
  //!         false if not.
  //! \ingroup External-Interface
  bool operator<(State const &, State const &);

  //! \brief Overloaded greater-than comparison operator.
  //! \param a Const reference to a state.
  //! \param b Const reference to another state.
  //! \return true if a is strictly greater than b,
  //!         false if not.
  //! \ingroup External-Interface
  inline bool operator>(State const &a, State const &b)
  {
    return b < a;
  }

  //! \brief Overloaded greater-or-equal comparison operator.
  //! \param a Const reference to a state.
  //! \param b Const reference to another state.
  //! \return true if a is greater than or equal to b,
  //!         false if not.
  //! \ingroup External-Interface
  inline bool operator>=(State const &a, State const &b)
  {
    return !(a < b);
  }

  //! \brief Overloaded less-or-equal comparison operator.
  //! \param a Const reference to a state.
  //! \param b Const reference to another state.
  //! \return true if a is less than or equal to b,
  //!         false if not.
  //! \ingroup External-Interface
  inline bool operator<=(State const &a, State const &b)
  {
    return !(b < a);
  }

  //! \brief Overloaded formatted output operator.
  //! \param str Reference to the output stream.
  //! \param state Const reference to the State.
  //! \return Reference to the output stream.
  //! \ingroup External-Interface
  std::ostream &operator<<(std::ostream &, State const &);

} // namespace PLEXIL

#endif // PLEXIL_STATE_HH
