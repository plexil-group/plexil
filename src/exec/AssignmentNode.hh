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

#ifndef ASSIGNMENT_NODE_HH
#define ASSIGNMENT_NODE_HH

#include "Node.hh"

namespace PLEXIL
{

  class AssignmentNode : public Node
  {
  public:

    /**
     * @brief The constructor.  Will construct all conditions and child nodes.
     * @param node The PlexilNodeId for this node and all of its children.
     * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    AssignmentNode(const PlexilNodeId& node, const ExecConnectorId& exec, const NodeId& parent = NodeId::noId());

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    AssignmentNode(const LabelStr& type, const LabelStr& name, const NodeState state,
				   const bool skip, const bool start, const bool pre,
				   const bool invariant, const bool post, const bool end, const bool repeat,
				   const bool ancestorInvariant, const bool ancestorEnd, const bool parentExecuting,
				   const bool childrenFinished, const bool commandAbort, const bool parentWaiting,
				   const bool parentFinished, const bool cmdHdlRcvdCondition,
				   const ExecConnectorId& exec = ExecConnectorId::noId());

    /**
     * @brief Destructor.  Cleans up this entire part of the node tree.
     */
    virtual ~AssignmentNode();

    /**
     * @brief Accessor for the assigned variable.
     */
    const VariableId& getAssignmentVariable() const;

	// Called from the transition handler
    virtual void abort();
	
  protected:

	// Specific behaviors for derived classes
	virtual void specializedPostInit();
	virtual void createSpecializedConditions();
	virtual void specializedHandleExecution();
    virtual void specializedDeactivateExecutable();
	virtual void cleanUpNodeBody();

  private:

    void createAssignment(const PlexilAssignmentBody* body);
    void createDummyAssignment(); // unit test variant

    AssignmentId m_assignment;
  };

  class Assignment {
  public:
    Assignment(const VariableId lhs,
			   const ExpressionId rhs, 
			   const VariableId ack,
			   const LabelStr& lhsName,
			   const bool deleteLhs, 
			   const bool deleteRhs);
    ~Assignment();
    AssignmentId& getId() {return m_id;}
    VariableId& getDest() {return m_lhs;}
    ExpressionId& getAck() {return m_ack;}
    double getValue(){return m_value;}
    void activate();
    void deactivate();
    const std::string& getDestName();

  protected:
    friend class AssignmentNode;
    void fixValue();

  private:
    AssignmentId m_id;
	VariableId m_lhs;
    ExpressionId m_rhs, m_ack;
    double m_value;
    LabelStr m_destName;
    bool m_deleteLhs, m_deleteRhs;
  };

}

#endif // ASSIGNMENT_NODE_HH
