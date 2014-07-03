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

#ifndef PLEXIL_COMMAND_HH
#define PLEXIL_COMMAND_HH

#include "CommandHandleVariable.hh"
#include "State.hh"
#include "UserVariable.hh"
#include "Value.hh"

namespace PLEXIL
{
  // TODO:
  // - Move type names to common file shared with ResourceArbitrationInterface
  // - replace ResourceMap and ResourceValues with structs or classes

  // Forward references
  class Command;
  DECLARE_ID(Command);

  // FIXME: conflicts with same name in ResourceArbiterInterface
  typedef std::map<std::string, ExpressionId> ResourceMap;

  typedef std::vector<ResourceMap> ResourceList;
  typedef std::map<std::string, Value> ResourceValues;
  typedef std::vector<ResourceValues> ResourceValuesList;

  class Command 
  {
    friend class CommandNode;
    friend class CommandHandleVariable;

  public:
    Command(const ExpressionId nameExpr, 
            const std::vector<ExpressionId>& args, 
            const std::vector<ExpressionId>& garbage,
            Assignable *dest,
            const ResourceList& resource,
            std::string const &nodeName);
    ~Command();

    const CommandId &getId() {return m_id;}
    ExpressionId getDest() const;
    ExpressionId getAck() {return m_ack.getId();}
    ExpressionId getAbortComplete() {return m_abortComplete.getId();}
    State const &getCommand() const;
    std::string const &getName() const;
    std::vector<Value> const &getArgValues() const;
    const ResourceValuesList &getResourceValues() const;
    CommandHandleValue getCommandHandle() const {return (CommandHandleValue) m_commandHandle;}

    void activate();
    void deactivate();
    void reset();

  protected:

    void fixValues();
    void fixResourceValues();

  private:
    // Deliberately not implemented
    Command();
    Command(const Command&);
    Command& operator=(const Command&);

    CommandId m_id;
    CommandHandleVariable m_ack;
    BooleanVariable m_abortComplete;
    State m_command;
    ExpressionId m_nameExpr;
    Assignable *m_dest;
    std::vector<ExpressionId> m_garbage;
    std::vector<ExpressionId> m_args;
    ResourceList m_resourceList;
    ResourceValuesList m_resourceValuesList;
    uint16_t m_commandHandle;
    bool m_fixed, m_resourceFixed;
  };

}

#endif // PLEXIL_COMMAND_HH
