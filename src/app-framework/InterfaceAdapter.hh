/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

#ifndef INTERFACE_ADAPTER_H
#define INTERFACE_ADAPTER_H

#include "ExecDefs.hh"
#include "pugixml.hpp"

#include <list>
#include <map>
#include <set>
#include <vector>

namespace PLEXIL
{
  // forward references
  class InterfaceAdapter;
  typedef Id<InterfaceAdapter> InterfaceAdapterId;

  class Expression;
  typedef Id<Expression> ExpressionId;

  class LabelStr;

  class AdapterExecInterface;

  /**
   * @brief An abstract base class for interfacing the PLEXIL Universal Exec
   *        to other systems.
   * @see InterfaceManager, AdapterExecInterface
   */
  class InterfaceAdapter
  {

  public:

    //
    // Class constants
    //

    /**
     * @brief Constructor.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     */
    InterfaceAdapter(AdapterExecInterface& execInterface);

    /**
     * @brief Constructor from configuration XML.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     * @param xml Const reference to the XML element describing this adapter
     * @note The instance maintains a shared reference to the XML.
     */
    InterfaceAdapter(AdapterExecInterface& execInterface, 
                     const pugi::xml_node& xml);

    /**
     * @brief Destructor.
     */
    virtual ~InterfaceAdapter();

    //
    // API to ExecApplication
    //

    /**
     * @brief Initializes the adapter, possibly using its configuration data.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize() = 0;

    /**
     * @brief Starts the adapter, possibly using its configuration data.  
     * @return true if successful, false otherwise.
     */
    virtual bool start() = 0;

    /**
     * @brief Stops the adapter.  
     * @return true if successful, false otherwise.
     */
    virtual bool stop() = 0;

    /**
     * @brief Resets the adapter.  
     * @return true if successful, false otherwise.
     * @note Adapters should provide their own methods.  The default method simply returns true.
     */
    virtual bool reset() = 0;

    /**
     * @brief Shuts down the adapter, releasing any of its resources.
     * @return true if successful, false otherwise.
     * @note Adapters should provide their own methods.  The default method simply returns true.
     */
    virtual bool shutdown() = 0;

    /**
     * @brief Perform an immediate lookup on an existing state.
     * @param state The state.
     * @return The current value for the state.
     * @note Adapters should provide their own methods.  The default method raises an assertion.
     */
    virtual double lookupNow(const State& state);

    /**
     * @brief Inform the interface that it should report changes in value of this state.
     * @param state The state.
     * @note Adapters should provide their own methods.  The default method raises an assertion.
     */
    virtual void subscribe(const State& state);

    /**
     * @brief Inform the interface that a lookup should no longer receive updates.
     * @note Adapters should provide their own methods.  The default method raises an assertion.
     */
    virtual void unsubscribe(const State& state);

    /**
     * @brief Advise the interface of the current thresholds to use when reporting this state.
     * @param state The state.
     * @param hi The upper threshold, at or above which to report changes.
     * @param lo The lower threshold, at or below which to report changes.
     * @note Adapters should provide their own methods as appropriate.  The default method does nothing.
     */
    virtual void setThresholds(const State& state, double hi, double lo);

    /**
     * @brief Send the name of the supplied node, and the supplied value pairs, to the planner.
     * @param node The Node requesting the update.
     * @param valuePairs A map of <LabelStr key, value> pairs.
     * @param ack The expression in which to store an acknowledgement of completion.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void sendPlannerUpdate(const NodeId& node,
                                   const std::map<double, double>& valuePairs,
                                   ExpressionId ack);

    /**
     * @brief Execute a command with the requested arguments.
     * @param cmd The Command instance.
     * @note The default method calls the method below.
     */
    virtual void executeCommand(CommandId cmd);

    /**
     * @brief Execute a command with the requested arguments.
     * @param name The LabelString representing the command name.
     * @param args The command arguments expressed as doubles.
     * @param dest The expression in which to store any value returned from the command.
     * @param ack The expression in which to store an acknowledgement of command transmission.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void executeCommand(const LabelStr& name,
                                const std::list<double>& args,
                                ExpressionId dest,
                                ExpressionId ack);

    /**
     * @brief Abort the pending command.
     * @note Derived classes may implement this method. The default method calls the compatibility method below.
     */
    virtual void invokeAbort(const CommandId& name);

    /**
     * @brief Abort the pending command with the supplied name and arguments.
     * @param name The LabelString representing the command name.
     * @param args The command arguments expressed as doubles.
     * @param abort_ack The expression in which to store an acknowledgement of command abort.
     * @param cmd_ack The original acknowledgment for this command (for identification purposes)
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */
    virtual void invokeAbort(const LabelStr& name, 
                             const std::list<double>& args, 
                             ExpressionId abort_ack,
                             ExpressionId cmd_ack);

    /**
     * @brief Register this adapter based on its XML configuration data.
     * @note The adapter is presumed to be fully initialized and working at the time of this call.
     * @note This is a default method; adapters are free to override it.
     */
    virtual void registerAdapter();

    /**
     * @brief Get the ID of this instance.
     */
    InterfaceAdapterId getId()
    {
      return m_id;
    }

    /**
     * @brief Get the configuration XML for this instance.
     */
    const pugi::xml_node& getXml()
    {
      return m_xml;
    }

    /**
     * @brief Get the AdapterExecInterface for this instance.
     */
    AdapterExecInterface& getExecInterface()
    {
      return m_execInterface;
    }

  protected:

    //
    // API which all subclasses must implement
    //

    AdapterExecInterface& m_execInterface;

  private:

    // Deliberately unimplemented
    InterfaceAdapter();
    InterfaceAdapter(const InterfaceAdapter &);
    InterfaceAdapter & operator=(const InterfaceAdapter &);

    //
    // Member variables
    //

    const pugi::xml_node m_xml;
    InterfaceAdapterId m_id;
  };

  typedef Id<InterfaceAdapter> InterfaceAdapterId;
}

#endif // INTERFACE_ADAPTER_H
