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

#ifndef _H_ManagedExecListener
#define _H_ManagedExecListener

#include "ExecListener.hh"

// Forward reference w/o namespace
class TiXmlElement;

namespace PLEXIL
{

  // Forward reference w/o namespace
  class InterfaceManagerBase;

  /**
   * @brief An abstract base class, derived from ExecListener, which supports
   *        automatic management by the InterfaceManager class.
   * @see Class ExecListener
   */
  class ManagedExecListener :
    public ExecListener
  {
  public:

    /**
     * @brief Default constructor, for use when there's no XML
     *        description or Interface Manager involved.
     * @param xml Pointer to the (shared) configuration XML describing this listener.
     * @param mgr Pointer to the owning manager.
     */
    ManagedExecListener();

    /**
     * @brief Constructor from configuration XML and owning manager.
     * @param xml Pointer to the (shared) configuration XML describing this listener.
     * @param mgr A reference to the owning manager.
     */
    ManagedExecListener(const TiXmlElement* xml,
			InterfaceManagerBase & mgr);

    /**
     * @brief Destructor.
     */
    virtual ~ManagedExecListener();

    /**
     * @brief Get the configuration XML of this instance.
     * @return A pointer to the XML element.
     */
    inline const TiXmlElement* getXml() const
    { 
      return m_xml; 
    }

    /**
     * @brief Get the InterfaceManager that owns this instance.
     * @return A InterfaceManagerBase &.
     */
    inline InterfaceManagerBase & getManager() const;

    //
    // API to be implemented by derived classes
    //

    /**
     * @brief Perform listener-specific initialization.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize() = 0;

    /**
     * @brief Perform listener-specific startup.
     * @return true if successful, false otherwise.
     */
    virtual bool start() = 0;

    /**
     * @brief Perform listener-specific actions to stop.
     * @return true if successful, false otherwise.
     */
    virtual bool stop() = 0;

    /**
     * @brief Perform listener-specific actions to reset to initialized state.
     * @return true if successful, false otherwise.
     */
    virtual bool reset() = 0;

    /**
     * @brief Perform listener-specific actions to shut down.
     * @return true if successful, false otherwise.
     */
    virtual bool shutdown() = 0;

  private:
    //
    // Deliberately unimplemented
    //
    ManagedExecListener(const ManagedExecListener&);
    ManagedExecListener& operator= (const ManagedExecListener&);
    
    //
    // Member variables
    //

    /**
     * @brief The configuration XML used at construction time.
     */
    const TiXmlElement* m_xml;

    /**
     * @brief The InterfaceManager instance that owns this listener.
     */
    InterfaceManagerBase* m_manager;
  };

}

#endif // _H_ManagedExecListener
