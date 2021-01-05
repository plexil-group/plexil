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

#ifndef INTERFACE_ADAPTER_H
#define INTERFACE_ADAPTER_H

#include "pugixml.hpp"

namespace PLEXIL
{
  // forward references 
  class AdapterConfiguration;
  class AdapterExecInterface;

  //!
  // @brief An abstract base class for constructing the handlers which 
  //        interface the PLEXIL Universal Exec to external systems,
  //        and maintaining their shared state as the application requires.
  //
  // InterfaceAdapter is an optional feature of the PLEXIL Application Framework.
  // It is intended to serve these purposes:
  //
  // * Centralizing shared state required to access an external system
  //   with multiple commands and/or states
  // * A channel for publishing external data to the Exec
  //
  // @see AdapterConfiguration
  // @see CommandHandler
  // @see LookupHandler
  // @see PlannerUpdateHandler
  //

  // ISSUES:
  // * 

  class InterfaceAdapter
  {
  public:

    /**
     * @brief Default constructor.
     */

    /**
     * @brief Constructor from configuration XML.
     * @param xml The const XML element (handle) describing this adapter
     *
     * @note The instance maintains a shared reference to the XML, though
     *       perhaps this should be passed in via the initialize() method.
     */
    InterfaceAdapter(AdapterExecInterface &intf, pugi::xml_node const xml)
      : m_xml(xml),
        m_interface(intf)
    {
    }

    /**
     * @brief Destructor.
     */
    virtual ~InterfaceAdapter() = default;

    //
    // API to ExecApplication
    //

    /**
     * @brief Construct the appropriate handler objects as specified in the
     *        configuration XML, and register them with the AdapterConfiguration
     *        instance.
     * @param config Pointer to the AdapterConfiguration interface registry.
     * @return true if successful, false otherwise.
     * 
     * @note The default method simply returns true.
     */
    virtual bool initialize(AdapterConfiguration * /* config */)
    {
      return true;
    }

    /**
     * @brief Start the interface.
     * @return true if successful, false otherwise.
     * 
     * @note The default method simply returns true.
     */
    virtual bool start()
    {
      return true;
    }

    /**
     * @brief Stop the interface.
     * @return true if successful, false otherwise.
     * 
     * @note The default method simply returns true.
     */
    virtual bool stop()
    {
      return true;
    }

    AdapterExecInterface &getInterface()
    {
      return m_interface;
    }

    /**
     * @brief Get the configuration XML for this instance.
     */
    pugi::xml_node const getXml()
    {
      return m_xml;
    }

  private:

    // Deliberately unimplemented
    InterfaceAdapter() = delete;
    InterfaceAdapter(const InterfaceAdapter &) = delete;
    InterfaceAdapter(InterfaceAdapter &&) = delete;

    InterfaceAdapter &operator=(const InterfaceAdapter &) = delete;
    InterfaceAdapter &operator=(InterfaceAdapter &&) = delete;

    //
    // Member variables
    //

    const pugi::xml_node m_xml;
    AdapterExecInterface &m_interface;
  };

}

#endif // INTERFACE_ADAPTER_H
