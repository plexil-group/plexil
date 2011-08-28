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

#ifndef COMMAND_NODE_HH
#define COMMAND_NODE_HH

#include "Node.hh"

namespace PLEXIL
{
  class CommandNode : public Node
  {
  public:
    /**
     * @brief The constructor.  Will construct all conditions and child nodes.
     * @param node The PlexilNodeId for this node and all of its children.
     * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    CommandNode(const PlexilNodeId& node, const ExecConnectorId& exec, const NodeId& parent = NodeId::noId());

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    CommandNode(const LabelStr& type, const LabelStr& name, const NodeState state,
				const bool skip, const bool start, const bool pre,
				const bool invariant, const bool post, const bool end, const bool repeat,
				const bool ancestorInvariant, const bool ancestorEnd, const bool parentExecuting,
				const bool childrenFinished, const bool commandAbort, const bool parentWaiting,
				const bool parentFinished, const bool cmdHdlRcvdCondition,
				const ExecConnectorId& exec = ExecConnectorId::noId());

    /**
     * @brief Destructor.  Cleans up this entire part of the node tree.
     */
    virtual ~CommandNode();

	// Called from the transition handler
    virtual void abort();

  protected:

	// Specific behaviors for derived classes
	virtual void specializedPostInit();
	virtual void createSpecializedConditions();
	virtual void createConditionWrappers();
	virtual void specializedActivateInternalVariables();
	virtual void specializedHandleExecution();
	virtual void specializedDeactivateExecutable();
	virtual void specializedReset();

	virtual NodeState getDestStateFromExecuting();
	virtual NodeState getDestStateFromFailing();

	virtual void transitionFromExecuting(NodeState toState);
	virtual void transitionFromFailing(NodeState toState);

	virtual void transitionToExecuting();
	virtual void transitionToFailing();

	virtual void printCommandHandle(std::ostream& stream, const unsigned int indent, bool always = false) const;

	// Not useful if called from base class destructor!
	virtual void cleanUpNodeBody();

	// Used in command handle processing - to be deleted.
    double getAcknowledgementValue() const;

  private:

    void createCommand(const PlexilCommandBody* body);
	// Unit test support
    void createDummyCommand(); // unit test variant

    CommandId m_command; /*<! The command to be performed. */
    VariableId m_ack; /*<! The destination for acknowledgement of the command.  DON'T FORGET TO RESET THIS VALUE IN REPEAT-UNTILs! */
	VariableId m_commandHandleVariable;
  };

}

#endif // COMMAND_NODE_HH
