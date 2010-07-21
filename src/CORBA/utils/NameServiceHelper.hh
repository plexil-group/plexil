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

// -*- Mode: c++ -*-
//  Purpose: Utilities for using TAO as a CORBA implementation

#ifndef NAME_SERVICE_HELPER_HH
#define NAME_SERVICE_HELPER_HH

// C++ Standard Library
#include <iostream>
#include <string>

// ACE/TAO includes
#include "tao/corba.h"
#include "orbsvcs/CosNamingC.h"
#include "orbsvcs/Naming/Naming_Client.h" // for TAO_Naming_Client

namespace PLEXIL
{
  /*!
    \brief Singleton class performing common CORBA name service functions via TAO
  */
  class NameServiceHelper
  {
  public:
    /*!
      \brief Accessor for the singleton
    */
    static NameServiceHelper& getInstance();

    /*!
      \brief Destructor
    */
    virtual ~NameServiceHelper();

    /*!
      \brief Returns true if name service initialized, false if not.
    */
    bool isInitialized();

    /*!
      \brief Sets up communication with an external name service;
             returns true if successful, false otherwise.
    */
    virtual bool initialize(CORBA::ORB_ptr orb);

    /*!
      \brief Print a human-readable list of the names in a naming context.
    */
    void
    describeNamingContext(const CosNaming::Name & nom, ostream & strm);

    /*!
      \brief Performs a naming service query;
             caller must check return value for nil.
    */
    CORBA::Object_ptr
    queryNamingServiceForObject(const CosNaming::Name & nom);

    /*!
      \brief Binds the object to the given name.  
             Returns true if successful, false otherwise.
             Rebinds if necessary.
    */
    bool
    nameServiceBind(const CosNaming::Name & nom, 
		    CORBA::Object_ptr obj);

    /*!
      \brief Unbinds the given name.  
             Returns true if successful, false otherwise.
    */
    bool
    nameServiceUnbind(const CosNaming::Name & nom);

    // 
    // Static public helper functions
    //

    /*!
      \brief Parses namestring according to the OMG rules, 
             and returns the corresponding Name
    */
    static CosNaming::Name
    parseName(const std::string & namestring);

    /*!
      \brief Formats the Name onto the stream, 
             in escaped format suitable for parseName()
    */
    static void
    printName(const CosNaming::Name &, ostream &);

    /*!
      \brief Formats the Name into a string suitable for parsing
             with parseName()
    */
    static std::string
    nameToEscapedString(const CosNaming::Name &);

  protected:

    /*!
      \brief Constructor.
    */
    NameServiceHelper();

    //
    // Protected member functions
    //

    /*!
      \brief Bind or rebind the name in the parent context to obj.
             If successful, return true; if failed, return false.
    */

    bool
    nameServiceBindInternal(CosNaming::NamingContext_ptr parentContext,
			    const CosNaming::Name & nom, 
			    CORBA::Object_ptr obj);

    /*!
      \brief Perform one naming service query, and return true
             and store the result in the result object var if
             successful.  If failed, return false, and set can_retry
             flag as appropriate for the kind of failure.
    */
    
    bool
    performNamingServiceQuery(const CosNaming::Name & nom,
			      CORBA::Object_var & result,
			      bool & can_retry);

    /*!
      \brief Find or construct a naming context bound to the 
             supplied name.
    */
    CosNaming::NamingContext_ptr
    ensureNamingContext(const CosNaming::Name & nom);

    //
    // Protected constants
    //

    static const char s_nameSeparatorChar = '/';
    static const char s_nameKindChar = '.';
    static const char s_nameEscapeChar = '\\';
    static const std::string s_specialNameChars; // init'ed in .cc

    //
    // Protected static member functions
    //

    /*!
      \brief Given the "rest of name" from a NotFound exception, 
             and the name being queried to cause the exception,
             returns the name of the parent context
    */
    static CosNaming::Name
    getParentContextName(const CosNaming::Name & exception_rest_of_name,
                         const CosNaming::Name & queryName);

    /*!
      \brief Parses namestring according to the OMG rules, 
             and returns the corresponding NameComponent.  
             idx is updated to just past the terminating character, 
	     or std::string::npos if none found.
    */
    static CosNaming::NameComponent
    parseNameComponent(const std::string & namestring,
		       std::string::size_type & idx);

    /*!
      \brief Returns the index of the character terminating the name
             component which begins at start, or
	     std::string::npos if no terminator found.
    */
    static std::string::size_type
    findNameComponentEnd(const std::string & namestring, 
			 std::string::size_type start = 0);

    /*!
      \brief Allocates and initializes a null-terminated character array
             containing the specified substring, minus any escape characters.
    */
    static char * 
    makeNameComponentString(const std::string & namestring,
			    std::string::size_type start,
			    std::string::size_type end = std::string::npos);

    static void
    printNameComponent(const CosNaming::NameComponent &, ostream &);

  private:

    /*!
      \brief Copy constructor, explicitly not implemented.
    */
    NameServiceHelper(const NameServiceHelper &);

    //
    // Private class data
    //

    static NameServiceHelper* s_instance();

    //
    // Private member data
    //

    /*!
      \brief TAO naming service client side helper
    */
    TAO_Naming_Client m_namingClient;

    /*!
      \brief True if the naming service has been initialized, 
             false otherwise.
    */
    bool m_initialized;

  };

  inline std::ostream & operator<< (std::ostream & strm, const CosNaming::Name & nom)
  {
    NameServiceHelper::printName(nom, strm);
    return strm;
  }

}

#endif // NAME_SERVICE_HELPER_HH
