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

#ifndef PLEXIL_COMMAND_OPERATOR_HH
#define PLEXIL_COMMAND_OPERATOR_HH

#include "ValueType.hh"

namespace PLEXIL
{
  // Forward references
  class ListenableUnaryOperator;
  class Command;
  class Value;

  // TODO:
  // - Support printing

  // Type-independent components of CommandOperator
  class CommandOperator
  {
  public:
    virtual ~CommandOperator()
    {
    }

    std::string const &getName() const
    {
      return m_name;
    }

    // Delegated to CommandOperatorImpl by default
    virtual ValueType valueType() const = 0;
    virtual void *allocateCache() const = 0;
    virtual void deleteCache(void *Ptr) const = 0;

    // Default methods assert
    virtual bool operator()(Boolean &result, Command const *arg) const;
    // Only Boolean operators implemented to date
    // virtual bool operator()(uint16_t &result, Command const *arg) const;
    // virtual bool operator()(Integer &result, Command const *command) const;
    // virtual bool operator()(Real &result, Command const *command) const;
    // virtual bool operator()(String &result, Command const *command) const;

    // Not needed yet
    // virtual bool operator()(Array &result, Command const *command) const;
    // virtual bool operator()(BooleanArray &result, Command const *command) const;
    // virtual bool operator()(IntegerArray &result, Command const *command) const;
    // virtual bool operator()(RealArray &result, Command const *command) const;
    // virtual bool operator()(StringArray &result, Command const *command) const;

    virtual bool isKnown(Command const *command) const = 0;
    virtual void printValue(std::ostream &s, Command const *command) const = 0;
    virtual Value toValue(Command const *command) const = 0;

    // Helper for notification network
    virtual void doPropagationSources(Command *command,
                                      ListenableUnaryOperator const &oper) const = 0;

  protected:
    CommandOperator(std::string const &name)
      : m_name(name)
    {
    }

    std::string const m_name;

  private:
    // unimplemented
    CommandOperator();
    CommandOperator(CommandOperator const &);
    CommandOperator &operator=(CommandOperator const &);
  };

} // namespace PLEXIL

#endif // PLEXIL_COMMAND_OPERATOR_HH
