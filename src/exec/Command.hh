/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

#include "ExecDefs.hh"
#include "LabelStr.hh"

namespace PLEXIL
{

  // *** TODO: replace ResourceMap and ResourceValues with structs or classes
  typedef std::map<std::string, ExpressionId> ResourceMap;
  typedef std::vector<ResourceMap> ResourceList;
  typedef std::map<std::string, double> ResourceValues;
  typedef std::vector<ResourceValues> ResourceValuesList;

  class Command 
  {
  public:
    Command(const ExpressionId nameExpr, 
			const std::list<ExpressionId>& args, 
            const VariableId dest,
            const LabelStr& dest_name,
			const VariableId ack,
			const std::vector<ExpressionId>& garbage,
            const ResourceList& resource,
			const NodeId& parent);
    ~Command();

    CommandId& getId() {return m_id;}
    VariableId& getDest() {return m_dest;}
    VariableId& getAck() {return m_ack;}
    const std::list<double>& getArgValues() const {return m_argValues;}
    const ResourceValuesList& getResourceValues() const {return m_resourceValuesList;}
	const NodeId& getNode() const { return m_node; }
	LabelStr getName() const;
    const std::string& getDestName() const;

    void activate();
    void deactivate();

  protected:
    friend class CommandNode;

    void fixValues();
    void fixResourceValues();

  private:
    CommandId m_id;
    ExpressionId m_nameExpr;
    std::list<ExpressionId> m_args;
    VariableId m_dest;
    LabelStr m_destName;
    VariableId m_ack;
    std::vector<ExpressionId> m_garbage;
    std::list<double> m_argValues;
    ResourceList m_resourceList;
    ResourceValuesList m_resourceValuesList;
	NodeId m_node; // backpointer to parent
  };

}

#endif // PLEXIL_COMMAND_HH
