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

#ifndef COMMAND_NODE_HH
#define COMMAND_NODE_HH

#include "NodeImpl.hh"

namespace PLEXIL
{
  // Forward reference
  class CommandImpl;

  class CommandNode final : public NodeImpl
  {
  public:

    /**
     * @brief The constructor.
     * @param nodeId The name of this node.
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    CommandNode(char const *nodeId, NodeImpl *parent = nullptr);

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    CommandNode(const std::string& type,
                const std::string& name,
                NodeState state,
                NodeImpl *parent = nullptr);

    /**
     * @brief Destructor.  Cleans up this entire part of the node tree.
     */
    virtual ~CommandNode();

    /**
     * @brief Gets the type of this node.
     * @return The type of this node.
     */
    virtual PlexilNodeType getType() const override
    {
      return NodeType_Command;
    }

    /**
     * @brief Get the node's command.
     */
    CommandImpl const *getCommand() const { return m_command.get(); }
    CommandImpl *getCommand() { return m_command.get(); }

    /**
     * @brief Set the command.
     * @param cmd The command.
     * @note Should only be used by plan parser and unit test.
     */
    void setCommand(CommandImpl *cmd);

  protected:

    // Specific behaviors for derived classes
    virtual void specializedCreateConditionWrappers() override;
    virtual void specializedHandleExecution(PlexilExec *exec) override;
    virtual void specializedDeactivateExecutable(PlexilExec *exec) override;

    virtual bool getDestStateFromExecuting() override;
    virtual bool getDestStateFromFinishing() override;
    virtual bool getDestStateFromFailing() override;

    virtual void transitionToExecuting() override;
    virtual void transitionToFinishing() override;
    virtual void transitionToFailing(PlexilExec *exec) override;
    
    virtual void transitionFromExecuting(PlexilExec *exec) override;
    virtual void transitionFromFinishing(PlexilExec *exec) override;
    virtual void transitionFromFailing(PlexilExec *exec) override;

    virtual void printCommandHandle(std::ostream& stream, const unsigned int indent) const override;

    virtual void cleanUpNodeBody() override;

  private:

    // Unit test support
    void initDummyCommand(); // unit test variant

    std::unique_ptr<CommandImpl> m_command; /*<! The command to be performed. */
  };

}

#endif // COMMAND_NODE_HH
