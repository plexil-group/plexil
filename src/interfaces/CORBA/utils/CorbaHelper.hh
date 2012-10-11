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

//  Purpose: Utilities for using TAO as a CORBA implementation

#ifndef CORBA_HELPER_HH
#define CORBA_HELPER_HH

// C++ Standard Library
#include <iostream>
#include <string>

// ACE/TAO includes
#include "tao/corba.h"
#include "tao/PortableServer/PortableServer.h"
#include "orbsvcs/CosNamingC.h"

namespace PLEXIL
{
  /*!
    \brief Singleton class performing common CORBA functions via TAO
  */
  class CorbaHelper
  {
  public:
    /*!
      \brief Accessor for the singleton
    */
    static CorbaHelper& getInstance();

    /*!
      \brief Destructor
    */
    virtual ~CorbaHelper();

    /*!
      \brief Returns a pointer to the ORB
    */
    CORBA::ORB_ptr getOrb();

    /*!
      \brief Returns true if ORB initialized, false if not.
    */
    bool isOrbInitialized();

    /*!
      \brief Sets up communication with the ORB;
             returns true if successful, false otherwise.
    */
    virtual bool initializeOrb(int argc, char** argv);

    /*!
      \brief Returns true if name service initialized, false if not.
    */
    bool isNameServiceInitialized();

    /*!
      \brief Sets up communication with an external name service;
             returns true if successful, false otherwise.
    */
    virtual bool initializeNameService();

    /*!
      \brief Returns true if Portable Object Adapter (POA) initialized,
             false if not.
    */
    bool isPOAInitialized();

    /*!
      \brief Sets up the Portable Object Adapter (POA);
             returns true if successful, false otherwise.
    */
    virtual bool initializePOA();

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

    // 
    // Static public helper functions
    // Moved to NameServiceHelper class
    //

    /*!
      \brief Parses namestring according to the OMG rules, 
             and returns the corresponding Name
    */
//     static CosNaming::Name
//     parseName(const std::string & namestring);

    /*!
      \brief Formats the Name onto the stream, 
             in escaped format suitable for parseName()
    */
//     static void
//     printName(const CosNaming::Name &, ostream &);

    /*!
      \brief Formats the Name into a string suitable for parsing
             with parseName()
    */
//     static std::string
//     nameToEscapedString(const CosNaming::Name &);

  protected:

    /*!
      \brief Constructor.  
             The argc and argv parameters are for TAO initialization.
    */
    CorbaHelper(int argc, char ** argv);

    //
    // Protected static member functions
    //

    /*!
      \brief Top level function for ORB event thread.
    */
    static void * orbEventThread(void * CorbaHelperAsVoidPointer);

    //
    // Protected member data, for use by subclasses (if any)
    //

    int m_argc;
    char ** m_argv;

  private:

    /*!
      \brief Default constructor, explicitly not implemented.
    */
    CorbaHelper();

    /*!
      \brief Copy constructor, explicitly not implemented.
    */
    CorbaHelper(const CorbaHelper &);

    //
    // Private class data
    //

    static CorbaHelper* s_instance();

    //
    // Private member data
    //

    /*!
      \brief Cached ORB object reference
    */
    CORBA::ORB_var m_orb;

    /*!
      \brief Cached POA object reference
    */
    PortableServer::POA_var m_poa;

    /*!
      \brief True if the ORB has been initialized, false otherwise.
    */
    bool m_orbInitialized;

    /*!
      \brief True if the POA has been initialized, 
             false otherwise.
    */
    bool m_POAInitialized;

  };

  // Moved to NameServiceHelper.hh
//   inline std::ostream & operator<< (std::ostream & strm, const CosNaming::Name & nom)
//   {
//     CorbaHelper::printName(nom, strm);
//     return strm;
//   }

}

#endif // CORBA_HELPER_HH
