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

#include "CorbaHelper.hh"
#include "NameServiceHelper.hh"

// Other PLEXIL utils
#include "Debug.hh"
#include "Error.hh"

// C++ Standard Library
#include <sstream>
#include <vector>

// ACE/TAO
#include "ace/Thread_Manager.h"

// *** TO DO:
//  - Improve error checking, reporting
//  - Efficiency hacks

namespace PLEXIL
{

  /*!
    \brief Instance accessor.
  */
  CorbaHelper& CorbaHelper::getInstance() {
    static CorbaHelper sl_instance;
    return sl_instance;
  }

  /*!
    \brief Constructor.
  */
  CorbaHelper::CorbaHelper()
    : m_orbInitialized(false),
      m_POAInitialized(false)
  {
  }

  /*!
    \brief Destructor.
  */
  CorbaHelper::~CorbaHelper()
  {
    // Seems to be a bad idea
//     if (this->isPOAInitialized())
//       {
// 	m_poa->destroy(1, 1);
//       }
//     if (this->isOrbInitialized())
//       {
// 	m_orb->destroy();
//       }
  }
  
  /*!
    \brief Returns true if ORB initialized, false if not.
  */
  bool CorbaHelper::isOrbInitialized()
  {
    return this->m_orbInitialized;
  }

  /*!
    \brief Sets up communication with the ORB;
    returns true if successful, false otherwise.
  */
  bool CorbaHelper::initializeOrb(int argc, char** argv)
  {
    try
      {
	this->m_orb = CORBA::ORB_init(argc, argv, "UE Orb");
      }
    catch (CORBA::Exception & e)
      {
	// report exception here
	std::cerr << "Error: CORBA exception while initializing ORB: " << e << std::endl;
	this->m_orbInitialized = false;
	return false;
      }

    this->m_orbInitialized = true;
    return true;
  }

  /*!
    \brief Returns a pointer to the ORB
  */
  CORBA::ORB_ptr CorbaHelper::getOrb()
  {
    return CORBA::ORB::_duplicate(m_orb.in());
  }

  /*!
    \brief Returns true if name service initialized, false if not.
  */
  bool CorbaHelper::isNameServiceInitialized()
  {
    return NameServiceHelper::getInstance().isInitialized();
  }

  /*!
    \brief Sets up communication with an external name service;
    returns true if successful, false otherwise.
  */
  bool CorbaHelper::initializeNameService()
  {
    return NameServiceHelper::getInstance().initialize(m_orb.in());
  }

  /*!
    \brief Returns true if POA initialized, false if not.
  */
  bool CorbaHelper::isPOAInitialized()
  {
    return this->m_POAInitialized;
  }

  /*!
    \brief Sets up communication;
    returns true if successful, false otherwise.
  */
  bool CorbaHelper::initializePOA()
  {
    checkError(this->isOrbInitialized(), "initializePOA: ORB not initialized");
    try
      {
	debugMsg("CorbaHelper:initializePOA", " initializing POA");
	CORBA::Object_var poa_object =
	  m_orb->resolve_initial_references("RootPOA");
	m_poa = PortableServer::POA::_narrow(poa_object.in());
	PortableServer::POAManager_var poa_manager =
	  m_poa->the_POAManager();
	poa_manager->activate();
	debugMsg("CorbaHelper:initializePOA", " POA manager activated");

	// now spawn a thread to run the ORB event loop
	ACE_Thread_Manager::instance()->spawn(orbEventThread, this);
	debugMsg("CorbaHelper:initializePOA", " ORB event loop thread spawned");
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "CORBA exception while initializing POA: " << e << std::endl;
	this->m_POAInitialized = false;
	return false;
      }

    // success!
    this->m_POAInitialized = true;
    return true;
  }

  /*!
    \brief Binds the object to the given name.  
           Rebinds if necessary.
  */
  bool
  CorbaHelper::nameServiceBind(const CosNaming::Name & nom, 
			       CORBA::Object_ptr obj)
  {
    return NameServiceHelper::getInstance().nameServiceBind(nom, obj);
  }

  

  //
  // Protected static member functions
  //

  /*!
    \brief Top level function for the ORB event thread.
  */
  void * CorbaHelper::orbEventThread(void * corbaHelperAsVoidPointer)
  {
    CorbaHelper * self = (CorbaHelper *) corbaHelperAsVoidPointer;
    self->m_orb->run();
    return 0;
  }
  
}
