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

#ifndef PLEXIL_COMMAND_FUNCTION_HH
#define PLEXIL_COMMAND_FUNCTION_HH

#include "Expression.hh"
#include "Propagator.hh"

namespace PLEXIL
{
  class CommandImpl;
  class CommandOperator;

  //! \class CommandFunction
  //! \brief An abstract base class representing a function whose value depends on
  //!        some property or properties of a command.
  //! \ingroup External-Interface
  class CommandFunction
    : public Expression,
      public Propagator
  {
  public:
    //! \brief Constructor.
    //! \param op Const pointer to the operator.
    //! \param cmd Reference to the command.
    CommandFunction(CommandOperator const *op, CommandImpl &cmd);

    //! \brief Virtual destructor.
    virtual ~CommandFunction() = default;

    //
    // Expression API
    //

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual const char *exprName() const;

    //! \brief Return the type of the expression's value.
    //! \return The value type.
    virtual ValueType valueType() const;

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const;

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream &s) const;

    //! \brief Get the value of this expression as a Value instance.
    //! \return The value.
    virtual Value toValue() const;

    //! \brief Copy the value of this object to a result variable.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    //! \note The value is not copied if the return value is false.
    //! \note Only Boolean operators implemented to date.
    virtual bool getValue(Boolean &result) const;

  protected:

    //
    // Listenable API
    //

    //! \brief Call a function on all subexpressions of this object.
    //! \param oper A functor; it must implement an operator() method of one argument, a Listenable, returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &oper);

    CommandOperator const *m_op;

  private:

    // Default, copy, move constructors, assignment not implemented
    CommandFunction() = delete;
    CommandFunction(CommandFunction const &) = delete;
    CommandFunction(CommandFunction &&) = delete;
    CommandFunction &operator=(CommandFunction const &) = delete;
    CommandFunction &operator=(CommandFunction &&) = delete;

    //! \brief Reference to the parent Command.
    CommandImpl &m_command;
  };

} // namespace PLEXIL

#endif // PLEXIL_COMMAND_FUNCTION_HH
