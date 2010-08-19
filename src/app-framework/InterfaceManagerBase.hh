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

#ifndef INTERFACE_MANAGER_BASE_HH
#define INTERFACE_MANAGER_BASE_HH

#include "Id.hh"
#include "Expression.hh"
#include <string>

namespace PLEXIL
{
  // forward references
  class InterfaceManagerBase;
  typedef Id<InterfaceManagerBase> InterfaceManagerBaseId;


  /**
   * @brief An abstract base class representing the InterfaceManager API
   *        from an ExecListener's point of view.
   * @note This class exists so that ManagedExecListener and its derived classes
   *       need not be aware of the implementation details of InterfaceManager.
   */

  class InterfaceManagerBase
  {
  public:

    //
    // API for all related objects
    //

    /**
     * @brief Associate an arbitrary object with a string.
     * @param name The string naming the property.
     * @param thing The property value as an untyped pointer.
     */
    virtual void setProperty(const std::string& name, void * thing) = 0;

    /**
     * @brief Fetch the named property.
     * @param name The string naming the property.
     * @return The property value as an untyped pointer.
     */
    virtual void* getProperty(const std::string& name) = 0;

    //
    // Static utility functions
    //
    
    inline static std::string valueToString(const double val) {
	  return Expression::valueToString(val);
	};
    static double stringToValue(const char * rawValue);

  protected:
    InterfaceManagerBase();

    virtual ~InterfaceManagerBase();
    
    InterfaceManagerBaseId m_baseId;

  private:
    // Deliberately unimplemented
    InterfaceManagerBase(const InterfaceManagerBase&);
    InterfaceManagerBase& operator=(const InterfaceManagerBase&);

  };

}

#endif // INTERFACE_MANAGER_BASE_HH
