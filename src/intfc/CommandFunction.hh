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

#ifndef PLEXIL_COMMAND_FUNCTION_HH
#define PLEXIL_COMMAND_FUNCTION_HH

#include "Expression.hh"
#include "Propagator.hh"

namespace PLEXIL
{
  class CommandImpl;
  class CommandOperator;

  /**
   * @class CommandFunction
   * @brief An abstract base class.
   * Represents a function whose value depends on some property of a command or its neighbors.
   */

  class CommandFunction
    : public Expression,
      public Propagator
  {
  public:
    CommandFunction(CommandOperator const *op, CommandImpl &cmd);
    virtual ~CommandFunction();

    //
    // Expression API
    //

    const char *exprName() const;
    ValueType valueType() const;
    bool isKnown() const;
    void printValue(std::ostream &s) const;
    virtual void doSubexprs(ListenableUnaryOperator const &oper);

    Value toValue() const;

    /**
     * @brief Retrieve the value of this Expression in its native form.
     * @param The appropriately typed place to put the result.
     * @return True if result known, false if unknown.
     */
    bool getValue(Boolean &result) const;
    // Only Boolean operators implemented to date
    // bool getValue(uint16_t &result) const;
    // bool getValue(Integer &result) const;
    // bool getValue(Real &result) const;
    // bool getValue(String &result) const;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    // Only Boolean operators implemented to date
    // bool getValuePointer(String const *&ptr) const;

    // Array variants not implemented

  protected:

    CommandOperator const *m_op;

  private:
    // Not implemented
    CommandFunction();
    CommandFunction(const CommandFunction &);
    CommandFunction& operator=(const CommandFunction &);

    CommandImpl &m_command;
  };

} // namespace PLEXIL

#endif // PLEXIL_COMMAND_FUNCTION_HH
