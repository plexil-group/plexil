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

#ifndef PLEXIL_COMMAND_OPERATOR_HH
#define PLEXIL_COMMAND_OPERATOR_HH

#include "Listenable.hh" // ListenableUnaryOperator
#include "ValueType.hh"

namespace PLEXIL
{
  // Forward references
  class CommandImpl;
  class Value;

  // TODO:
  // - Support printing

  //! \class CommandOperator
  //! \brief A virtual base class for operators used by CommandFunction.
  //! \see CommandFunction
  //! \ingroup External-Interface
  class CommandOperator
  {
  public:

    //! \brief Virtual destructor.
    virtual ~CommandOperator() = default;

    //! \brief Get the operator's name.
    //! \return Const reference to the name string.
    std::string const &getName() const
    {
      return m_name;
    }

    //! \brief Get the type of the operator's return value.
    //! \return The ValueType.
    virtual ValueType valueType() const = 0;

    //! \brief Compute the result of applying this operator to the Command
    //!        and store it in the appropriately typed result variable.
    //! \param result Reference to the result variable.
    //! \param arg Const pointer to the Command.
    //! \return true if the result is known, false if not.
    //! \note Only Boolean operators implemented to date.
    virtual bool operator()(Boolean &result, CommandImpl const *arg) const = 0;

    //! \brief Is the value of this operator applied to this command known?
    //! \param command Const pointer to the Command.
    //! \return true if known, false if unknown.
    virtual bool isKnown(CommandImpl const *command) const = 0;

    //! \brief Print the value of this operator applied to this command.
    //! \param s The output stream.
    //! \param command Const pointer to the Command.
    virtual void printValue(std::ostream &s, CommandImpl const *command) const = 0;

    //! \brief Get the value of this operator applied to this command.
    //! \param command Const pointer to the command.
    //! \return The Value.
    virtual Value toValue(CommandImpl const *command) const = 0;

    //! \brief Map the given operator onto all propagation sources of the command.
    //! \param command Pointer to the Command.
    //! \param oper Const reference to the operator.
    //! \note Helper for CommandFunction::doSubexprs.
    virtual void doPropagationSources(CommandImpl *command,
                                      ListenableUnaryOperator const &oper) const = 0;

  protected:

    //! \brief Protected constructor.  Only accessible to derived classes.
    //! \param name Const reference to the operator's name.
    CommandOperator(std::string const &name)
      : m_name(name)
    {
    }

    //! \brief The name of this operator.
    std::string const m_name;

  private:

    // Default, copy, move constructors, assignment operators unimplemented
    CommandOperator() = delete;
    CommandOperator(CommandOperator const &) = delete;
    CommandOperator(CommandOperator &&) = delete;
    CommandOperator &operator=(CommandOperator const &) = delete;
    CommandOperator &operator=(CommandOperator &&) = delete;
  };

} // namespace PLEXIL

//! \brief Helper macro, intended to implement "boilerplate" singleton accessors
//!        for classes derived from CommandOperator.
//! \param CLASS The class of the object returned from the singleton accessor.
//! \ingroup External-Interface
#define DECLARE_COMMAND_OPERATOR_STATIC_INSTANCE(CLASS) \
  static PLEXIL::CommandOperator const *instance() \
  { \
    static CLASS const sl_instance; \
    return static_cast<PLEXIL::CommandOperator const *>(&sl_instance); \
  }

#endif // PLEXIL_COMMAND_OPERATOR_HH
