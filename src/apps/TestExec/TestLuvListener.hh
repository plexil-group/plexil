/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

#ifndef Test_Luv_Listener_HH
#define Test_Luv_Listener_HH

#include "ExecListener.hh"
#include "EssentialLuvListener.hh"

namespace PLEXIL 
{
  class TestLuvListener : 
	public EssentialLuvListener,
	public ExecListener
  {
  public:

	//* Constructor.
	TestLuvListener(const std::string& host, 
					const uint16_t port, 
					const bool block = false);

	//* Destructor.
	~TestLuvListener();

    /**
     * @brief Notify that a node has changed state.
     * @param prevState The old state.
     * @param node The node that has transitioned.
     * @note The current state is accessible via the node.
     */
    void implementNotifyNodeTransition(NodeState prevState, 
									   const NodeId& node) const
	{
	  EssentialLuvListener::implementNotifyNodeTransition(prevState, node);
	}

    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the parent node under which this plan will be inserted.
     */
    void implementNotifyAddPlan(const PlexilNodeId& plan, 
								const LabelStr& parent) const
	{
	  EssentialLuvListener::implementNotifyAddPlan(plan, parent);
	}

    /**
     * @brief Notify that a library node has been received by the Exec.
     * @param libNode The intermediate representation of the plan.
     * @note The default method is deprecated and will go away in a future release.
     */
    void implementNotifyAddLibrary(const PlexilNodeId& libNode) const
	{
	  EssentialLuvListener::implementNotifyAddLibrary(libNode);
	}

    /**
     * @brief Notify that a variable assignment has been performed.
     * @param dest The Expression being assigned to.
     * @param destName A string naming the destination.
     * @param value The value (in internal Exec representation) being assigned.
     */
    void implementNotifyAssignment(const ExpressionId & dest,
								   const std::string& destName,
								   const double& value) const
	{
	  EssentialLuvListener::implementNotifyAssignment(dest, destName, value);
	}


  private:
	//
	// Deliberately unimplemented
	//
	TestLuvListener();
	TestLuvListener(const TestLuvListener&);
	TestLuvListener& operator=(const TestLuvListener&);
  };
}

#endif // Test_Luv_Listener_HH
