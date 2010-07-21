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
#include "PlexilPlan.hh"
#include "LabelStr.hh"
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

//
// Forward references w/o namespace
//

class Socket;
class TiXmlElement;

namespace PLEXIL 
{
  /**
   * @brief A variant of the LuvListener class for use with InterfaceManager.
   * @see Class LuvListener
   */
  class NewLuvListener :
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

    // Defaults
    DECLARE_STATIC_CLASS_CONST(char*, LUV_DEFAULT_HOSTNAME, "localhost");
    DECLARE_STATIC_CLASS_CONST(unsigned int, LUV_DEFAULT_PORT, 9787);

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
				       const NodeId& node) const;

    /**
     * @brief Notify that a plan has been received by the Exec.
     * @param plan The intermediate representation of the plan.
     * @param parent The name of the parent node under which this plan will be inserted.
     */
    void implementNotifyAddPlan(const PlexilNodeId& plan, 
				const LabelStr& parent) const;

    /**
     * @brief Notify that a library node has been received by the Exec.
     * @param libNode The intermediate representation of the plan.
     * @note The default method is deprecated and will go away in a future release.
     */
    void implementNotifyAddLibrary(const PlexilNodeId& libNode) const;

    /**
     * @brief Notify that a variable assignment has been performed.
     * @param dest The Expression being assigned to.
     * @param destName A string naming the destination.
     * @param value The value (in internal Exec representation) being assigned.
     */
    void implementNotifyAssignment(const ExpressionId & dest,
				   const std::string& destName,
				   const double& value) const;

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

  protected:

    //
    // Internal class constants
    //

    // Literal strings
    DECLARE_STATIC_CLASS_CONST(char*, TRUE_STR, "true");
    DECLARE_STATIC_CLASS_CONST(char*, FALSE_STR, "false");

    // XML tags
    DECLARE_STATIC_CLASS_CONST(char*, PLAN_INFO_TAG, "PlanInfo");
    DECLARE_STATIC_CLASS_CONST(char*, PLEXIL_PLAN_TAG, "PlexilPlan");
    DECLARE_STATIC_CLASS_CONST(char*, PLEXIL_LIBRARY_TAG, "PlexilLibrary");
    DECLARE_STATIC_CLASS_CONST(char*, VIEWER_BLOCKS_TAG, "ViewerBlocks");

    DECLARE_STATIC_CLASS_CONST(char*, NODE_ID_TAG, "NodeId");
    DECLARE_STATIC_CLASS_CONST(char*, NODE_PATH_TAG, "NodePath");

    DECLARE_STATIC_CLASS_CONST(char*, NODE_STATE_UPDATE_TAG, "NodeStateUpdate");
    DECLARE_STATIC_CLASS_CONST(char*, NODE_STATE_TAG, "NodeState");
    DECLARE_STATIC_CLASS_CONST(char*, NODE_OUTCOME_TAG, "NodeOutcome");
    DECLARE_STATIC_CLASS_CONST(char*, NODE_FAILURE_TYPE_TAG, "NodeFailureType");
    DECLARE_STATIC_CLASS_CONST(char*, CONDITIONS_TAG, "Conditions");

    DECLARE_STATIC_CLASS_CONST(char*, ASSIGNMENT_TAG, "Assignment");
    DECLARE_STATIC_CLASS_CONST(char*, VARIABLE_TAG, "Variable");
    DECLARE_STATIC_CLASS_CONST(char*, VARIABLE_NAME_TAG, "VariableName");
    DECLARE_STATIC_CLASS_CONST(char*, VARIABLE_VALUE_TAG, "Value");

    // End-of-message marker
    DECLARE_STATIC_CLASS_CONST(char, LUV_END_OF_MESSAGE, (char)4);

    //
    // Static helper methods
    //

    // N.B. These allocate the first argument if it is NULL.
    static TiXmlNode* constructNodePath(TiXmlNode* path,
                                        const NodeId& node);
    static TiXmlNode* constructConditions(TiXmlNode* conditions,
                                          const NodeId& node);

    //
    // Internal methods
    //

    void sendPlanInfo() const;
    void sendMessage(const TiXmlNode& xml) const;
    void sendMessage(const std::string& message) const;
    void waitForAcknowledge() const;

    // *** these don't seem to be used anywhere ***
    void sendTaggedStream(std::istream& stream, 
                          const std::string& tag) const;
    void sendStream(std::istream& stream) const;

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
