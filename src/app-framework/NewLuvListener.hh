/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#ifndef _H_NewLuvListener
#define _H_NewLuvListener

#include "ConstantMacros.hh"
#include "ManagedExecListener.hh"
#include "EssentialLuvListener.hh"
#include "PlexilPlan.hh"
#include "LabelStr.hh"


//
// Forward references w/o namespace
//

class TiXmlElement;

namespace PLEXIL 
{
  /**
   * @brief A variant of the LuvListener class for use with InterfaceManager.
   * @see Class LuvListener
   */
  class NewLuvListener :
	public EssentialLuvListener,
    public ManagedExecListener 
  {
  public:
    //
    // Public class constants
    //

    // Configuration XML
    DECLARE_STATIC_CLASS_CONST(char*, LUV_HOSTNAME_ATTR, "HostName");
    DECLARE_STATIC_CLASS_CONST(char*, LUV_PORT_ATTR, "Port");
    DECLARE_STATIC_CLASS_CONST(char*, LUV_BLOCKING_ATTR, "Blocking");
    DECLARE_STATIC_CLASS_CONST(char*, IGNORE_CONNECT_FAILURE_ATTR, "IgnoreConnectFailure");

    // Literal strings (yes, this is redundant with LuvFormat.hh)
    DECLARE_STATIC_CLASS_CONST(char*, TRUE_STR, "true");
    DECLARE_STATIC_CLASS_CONST(char*, FALSE_STR, "false");

    //
    // Constructors
    //

    /**
     * @brief Constructor from configuration XML.
     */
    NewLuvListener(const TiXmlElement* xml, InterfaceManagerBase& mgr);

    /**
     * @brief Destructor.
     */
    ~NewLuvListener();

    /**
     * @brief Perform listener-specific initialization.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize();

    /**
     * @brief Perform listener-specific startup.
     * @return true if successful, false otherwise.
     */
    virtual bool start();

    /**
     * @brief Perform listener-specific actions to stop.
     * @return true if successful, false otherwise.
     */
    virtual bool stop();

    /**
     * @brief Perform listener-specific actions to reset to initialized state.
     * @return true if successful, false otherwise.
     */
    virtual bool reset();

    /**
     * @brief Perform listener-specific actions to shut down.
     * @return true if successful, false otherwise.
     */
    virtual bool shutdown();

    /**
     * @brief Notify that a node has changed state.
     * @param prevState The old state.
     * @param node The node that has transitioned.
     * @note The current state is accessible via the node.
     */
    void implementNotifyNodeTransition(const LabelStr& prevState, 
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


    //
    // Public class member functions
    //

    /**
     * @brief Construct the appropriate configuration XML for the desired settings.
     * @param block true if the Exec should block until the user steps forward, false otherwise.
     * @param hostname The host name where the Luv instance is running.
     * @param port The port number for the Luv instance.
     */
    static TiXmlElement* constructConfigurationXml(const bool& block = false,
						   const char* hostname = LUV_DEFAULT_HOSTNAME(),
						   const unsigned int port = LUV_DEFAULT_PORT());

  private:
    //
    // Deliberately unimplemented
    //
    NewLuvListener();
    NewLuvListener(const NewLuvListener&);
    NewLuvListener& operator=(const NewLuvListener&);

    //
    // Member variables
    //
    Socket* m_socket;
    const char* m_hostname;
    uint16_t m_port;
    bool m_block;
    bool m_ignoreConnectFailure;
  };
}

#endif
