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

#ifndef _H_ManagedExecListener
#define _H_ManagedExecListener

#include "ExecListener.hh"

// Forward reference w/o namespace
class TiXmlElement;

namespace PLEXIL
{
  class ManagedExecListener :
    public ExecListener
  {
  public:

    /**
     * @brief Constructor from configuration XML.
     */
    ManagedExecListener(const TiXmlElement* xml);

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

    //
    // API to be implemented by derived classes
    //

    /**
     * @brief Perform listener-specific initialization.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool initialize() = 0;

    /**
     * @brief Perform listener-specific startup.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool start() = 0;

    /**
     * @brief Perform listener-specific actions to stop.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool stop() = 0;

    /**
     * @brief Perform listener-specific actions to reset to initialized state.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool reset() = 0;

    /**
     * @brief Perform listener-specific actions to shut down.
     * @return true if successful, false otherwise.
     * @note Default method provided as a convenience for backward compatibility.
     */
    virtual bool shutdown() = 0;

  private:
    //
    // Deliberately unimplemented
    //
    ManagedExecListener();
    ManagedExecListener(const ManagedExecListener&);
    ManagedExecListener& operator= (const ManagedExecListener&);
    
    //
    // Member variables
    //

    /**
     * @brief The configuration XML used at construction time.
     */
    const TiXmlElement* m_xml;
  };

  class ManagedExecListenerFilter :
    public ExecListenerFilter
  {
  public:
    /**
     * @brief Constructor from configuration XML.
     */
    ManagedExecListenerFilter(TiXmlElement* xml);

    /**
     * @brief Destructor from configuration XML.
     */
    virtual ~ManagedExecListenerFilter();

    /**
     * @brief Get the configuration XML of this instance.
     * @return A pointer to the XML element.
     */
    inline const TiXmlElement* getXml() const
    {
      return m_xml;
    }

  private:
    //
    // Deliberately unimplemented
    //
    ManagedExecListenerFilter();
    ManagedExecListenerFilter(const ManagedExecListenerFilter &);
    ManagedExecListenerFilter& operator=(const ManagedExecListenerFilter &);

    //
    // Member variables
    //
    const TiXmlElement* m_xml;
  };


}

#endif // _H_ManagedExecListener
