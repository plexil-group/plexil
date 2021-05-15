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

#include "Configuration.hh"

#include "pugixml.hpp"

#include <memory>

namespace PLEXIL
{
  // forward references 
  class AdapterConfiguration;
  class AdapterExecInterface;

  //! @class InterfaceAdapter
  //! An abstract base class for constructing the handlers which
  //! interface the PLEXIL Universal Exec to external systems, and
  //! maintaining their shared state as the application requires.

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

  class InterfaceAdapter
  {
  public:

    //1 Constructor.
    //! @param intf Reference to the application's AdapterExecInterface.
    //! @param conf Pointer to the AdapterConf object describing this
    //!             adapter; may be null.
    InterfaceAdapter(AdapterExecInterface &intf, AdapterConf *conf)
      : m_conf(conf),
        m_interface(intf)
    {
    }

    //! Virtual destructor.
    virtual ~InterfaceAdapter() = default;

    //
    // API to ExecApplication
    //

    //! Initialize the handler, construct the handler objects as
    //! specified in the adapter's AdapterConf instance, and register
    //! them with the AdapterConfiguration.
    //! @param config Pointer to the AdapterConfiguration interface registry.
    //! @return true if successful, false otherwise.
    //! @note The default method simply returns true as a convenience.
    virtual bool initialize(AdapterConfiguration * /* config */)
    {
      return true;
    }

    //! Start the interface.
    //! @return true if successful, false otherwise.
    //! @note The default method simply returns true as a convenience.
    virtual bool start()
    {
      return true;
    }

    //! Stop the interface.
    //! @note The default method does nothing.
    virtual void stop()
    {
    }

    //! Get a reference to the AdapterExecInterface.
    //! @return The AdapterExecInterface reference.
    AdapterExecInterface &getInterface()
    {
      return m_interface;
    }

    //! Get a const reference to the AdapterExecInterface.
    //! @return The AdapterExecInterface reference.
    AdapterExecInterface const &getInterface() const
    {
      return m_interface;
    }

    //! Get the parsed configuration for this adapter.
    //! @return Const reference to the configuration object.
    AdapterConf const &configuration()
    {
      static AdapterConf const sl_empty_conf = AdapterConf();
      if (m_conf)
        return *m_conf;
      else
        return sl_empty_conf;
    }

    //! Get the configuration XML used to create this instance.
    //! @return The XML.
    pugi::xml_node const getXml()
    {
      return configuration().xml;
    }

  private:

    // Deliberately unimplemented
    InterfaceAdapter() = delete;
    InterfaceAdapter(const InterfaceAdapter &) = delete;
    InterfaceAdapter(InterfaceAdapter &&) = delete;

    InterfaceAdapter &operator=(const InterfaceAdapter &) = delete;
    InterfaceAdapter &operator=(InterfaceAdapter &&) = delete;

    //
    // Private member variables
    //

    std::unique_ptr<AdapterConf> m_conf;
    AdapterExecInterface &m_interface;
  };

}

#endif // INTERFACE_ADAPTER_H
