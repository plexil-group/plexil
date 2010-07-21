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

//  Purpose: Implementation of NameServiceHelper class

#include "NameServiceHelper.hh"

// Other PLEXIL utils
#include "Debug.hh"
#include "Error.hh"

// C++ Standard Library
#include <sstream>
#include <vector>

// *** TO DO:
//  - Improve error checking, reporting
//  - Efficiency hacks

namespace PLEXIL
{
  // Initialize static class variable

  const std::string NameServiceHelper::s_specialNameChars = std::string("/.\\");

  /*!
    \brief Instance accessor.
  */
  NameServiceHelper& NameServiceHelper::getInstance() {
    static NameServiceHelper sl_instance;
    return sl_instance;
  }

  /*!
    \brief Constructor.
  */
  NameServiceHelper::NameServiceHelper()
    : m_initialized(false)
  {
  }

  /*!
    \brief Destructor.
  */
  NameServiceHelper::~NameServiceHelper()
  {
  }

  /*!
    \brief Returns true if name service initialized, false if not.
  */
  bool NameServiceHelper::isInitialized()
  {
    return this->m_initialized;
  }

  /*!
    \brief Sets up communication with an external name service;
    returns true if successful, false otherwise.
  */
  bool NameServiceHelper::initialize(CORBA::ORB_ptr orb)
  {
    if (m_namingClient.init(orb) != 0)
      {
	ACE_ERROR_RETURN ((LM_ERROR,
			   "[CLIENT] Process/Thread Id : (%P/%t) Unable to initialize "
			   "the TAO_Naming_Client. \n"),
			  false);
      }
    m_initialized = true;
    return true;
  }

  /*!
    \brief Print a human-readable list of the names in a naming context.
  */
  void
  NameServiceHelper::describeNamingContext(const CosNaming::Name & nom,
                                           ostream & strm)
  {
    CORBA::Object_var obj = 
      queryNamingServiceForObject(nom);
    if (CORBA::is_nil(obj.in()))
      strm << "No object named \"" << nom << "\" found" << std::endl;

    // Got an object, try to narrow it to a naming context
    ACE_DECLARE_NEW_ENV;
    ACE_TRY
      {
	strm << "Naming context " << nom << ":" << std::endl;
	CosNaming::NamingContext_var ctxt =
	  CosNaming::NamingContext::_narrow(obj.in()
					    ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	if (CORBA::is_nil(ctxt.in()))
	  {
	    strm << "ERROR: not a naming context!" << std::endl;
	    return;
	  }

	// Have context, now iterate over its bindings
	CosNaming::BindingList_var dummy;
	CosNaming::BindingIterator_var it;
	ctxt->list(0, dummy, it);
	ACE_TRY_CHECK;

	CosNaming::Binding_var binding;
	while (it->next_one(binding))
	  {
	    ACE_TRY_CHECK;
	    // Display the contents of one binding
	    strm << " " << binding->binding_name
		 << " type = ";
	    switch (binding->binding_type)
	      {
	      case CosNaming::nobject: 
		strm << "object"; 
		break;

	      case CosNaming::ncontext:
		strm << "context";
		break;

	      default:
		strm << "*** unknown type " << binding->binding_type << "***";
	      }
	    strm << std::endl;
	  }
	ACE_TRY_CHECK;
	it->destroy();
	ACE_TRY_CHECK;
      }
    ACE_CATCHANY
      {
	ACE_PRINT_EXCEPTION(ACE_ANY_EXCEPTION,
			    "NameServiceHelper::describeNamingContext");
	return;
      }
    ACE_ENDTRY;
  }

  /*!
    \brief Performs a naming service query;
    caller must check return value for nil
  */
  CORBA::Object_ptr
  NameServiceHelper::queryNamingServiceForObject(const CosNaming::Name & nom)
  {
    debugMsg("NameServiceHelper:queryNamingService",
	     " for '" << nameToEscapedString(nom) << "'");
    // return root context if name is empty
    if (nom.length() == 0)
      return m_namingClient.get_context();

    if (!this->isInitialized())
      {
	std::cerr << "NameServiceHelper::queryNamingServiceForObject: name service not initialized!"
		  << std::endl;
	return CORBA::Object::_nil();
      }
	  
    CORBA::Object_var obj;
    bool can_retry = true; // or we never try the 1st time!
    int max_retries = 3;
    for (int tries = 0; ((tries < max_retries) && can_retry);  tries ++)
      {
	if (this->performNamingServiceQuery(nom, obj, can_retry))
	  return CORBA::Object::_duplicate(obj.in());
      }

    // Fall-thru return
    return CORBA::Object::_nil();

  }


  /*!
    \brief Binds the object to the given name.  
    Rebinds if necessary.
  */
  bool
  NameServiceHelper::nameServiceBind(const CosNaming::Name & nom, 
                                     CORBA::Object_ptr obj)
  {
    checkError(this->isInitialized(),
	       "nameServiceBind: naming service not initialized");
    checkError(nom.length() > 0,
	       "nameServiceBind: attempt to bind a null name");

    // Check that parent naming contexts (if any) exist;
    // create new ones if needed
    CosNaming::Name parentName = nom;
    parentName.length(nom.length() - 1);
    CosNaming::NamingContext_var parentContext =
      this->ensureNamingContext(parentName);
    if (CORBA::is_nil(parentContext.in()))
      {
	debugMsg("NameServiceHelper:nameServiceBind", 
		 " unable to find or construct parent naming context '" 
		 << nameToEscapedString(parentName)
		 << "'");
      }
    else
      {
	// bind the tail of the name in the parent context
	CosNaming::Name localName(1);
	localName.length(1);
	localName[0] = nom[nom.length() - 1];
	if (this->nameServiceBindInternal(parentContext.in(), localName, obj))
	  return true;
      }
    
    // error finding/creating parent context, or failure binding in parent -
    // try binding full name in root context instead
    parentContext = m_namingClient.get_context();
    return this->nameServiceBindInternal(parentContext.in(), nom, obj);
  }


  bool
  NameServiceHelper::nameServiceBindInternal(CosNaming::NamingContext_ptr parentContext,
                                             const CosNaming::Name & nom, 
                                             CORBA::Object_ptr obj)
  {
    checkError(nom.length() > 0,
	       "nameServiceBindInternal: attempt to bind a null name");
    checkError(!CORBA::is_nil(parentContext),
	       "nameServiceBindInternal: parent context is null");

    // bind the name
    try
      {
	parentContext->bind(nom, obj);
      }
    catch (CosNaming::NamingContext::AlreadyBound & ab)
      {
	// rebind
	debugMsg("NameServiceHelper:nameServiceBind", 
		 " name " 
		 << nameToEscapedString(nom)
		 << " already bound, will attempt rebinding");
	try
	  {
	    parentContext->rebind(nom, obj);
	  }
	catch (CORBA::Exception & e)
	  {
	    checkError(ALWAYS_FAIL,
		       "nameServiceBind: unexpected CORBA exception "
		       << e
		       << " while rebinding name "
		       << nameToEscapedString(nom));
	  }
      }
    catch (CosNaming::NamingContext::InvalidName & inv)
      {
	debugMsg("NameServiceHelper:nameServiceBind",
		 " attempt to bind to invalid name "
		 << nameToEscapedString(nom));
	return false;
      }
    catch (CORBA::Exception & e)
      {
	std::cerr << "nameServiceBind: unexpected CORBA exception "
		  << e
		  << "\n while attempting to bind name '"
		  << nameToEscapedString(nom) << "'"
		  << std::endl;
	return false;
      }

    debugMsg("NameServiceHelper:nameServiceBind", 
	     " successfully bound name '" 
	     << nameToEscapedString(nom)
	     << "'");
    return true;
  }

  /*!
	\brief Unbinds the given name.  
	Returns true if successful, false otherwise.
  */
  bool
  NameServiceHelper::nameServiceUnbind(const CosNaming::Name & nom) {
    checkError(nom.length() > 0,
			   "nameServiceUnbind: attempt to unbind a null name");
	try {
	  m_namingClient.get_context()->unbind(nom);
	}
	catch (CosNaming::NamingContext::NotFound & nf) {
	  // We don't care if it wasn't bound in the first place
	  debugMsg("NameServiceHelper:nameServiceUnbind",
			   " name " << nameToEscapedString(nom) << " not found, ignoring");
	  return true;
	}
    catch (CosNaming::NamingContext::InvalidName & inv) {
	  debugMsg("NameServiceHelper:nameServiceUnbind",
			   " invalid name exception for " << nameToEscapedString(nom));
	  return false;
	}
    catch (CORBA::Exception & e) {
	  checkError(ALWAYS_FAIL,
				 "ensureNamingContext: unexpected CORBA exception " << e);
	  return false;
	}
	return true;
  }

  

  //
  // Protected member functions
  //

  /*!
    \brief Perform one naming service query, and return true
    and store the result in the result object var if
    successful.  If failed, return false, and set can_retry
    flag as appropriate for the kind of failure.
  */
    
  bool
  NameServiceHelper::performNamingServiceQuery(const CosNaming::Name & nom,
                                               CORBA::Object_var & result,
                                               bool & can_retry)
  {
    // *** TO DO:
    //  - handle other exceptions gracefully
    ACE_DECLARE_NEW_ENV;
    ACE_TRY
      {
	result =
	  this->m_namingClient->resolve(nom
					ACE_ENV_ARG_PARAMETER);
	ACE_TRY_CHECK;

	return true;
      }
    ACE_CATCH(CosNaming::NamingContext::NotFound, exc)
      {
	can_retry = false;
	std::cerr << "Warning: Name '" << nom
		  << "' not found, reason: ";
	switch (exc.why)
	  {
	  case CosNaming::NamingContext::missing_node:
	    std::cerr << "missing node";
	    break;

	  case CosNaming::NamingContext::not_context:
	    std::cerr << "not a context";
	    break;

	  case CosNaming::NamingContext::not_object:
	    std::cerr << "not an object";
	    break;

	  default:
	    std::cerr << "(unknown reason code " << exc.why << ")";
	    break;
	  }
	std::cerr << ",\n rest of name: '" << exc.rest_of_name
		  << "'" << std::endl;

	// Describe parent context (as a debug aid)
 	// this->describeNamingContext(getParentContextName(exc.rest_of_name, nom),
        //			       std::cerr);
      }
    ACE_CATCH(CORBA::COMM_FAILURE, exc)
      {
	can_retry = true;
	std::cerr << "WARNING: Non-fatal exception while querying name "
		  << nom << std::endl;
	ACE_PRINT_EXCEPTION(exc, "NameServiceHelper::performNamingServiceQuery");
      }
    ACE_CATCHANY
      {
	can_retry = false;
	std::cerr << "ERROR: Unhandled exception while querying name "
		  << nom << std::endl;
	ACE_PRINT_EXCEPTION(ACE_ANY_EXCEPTION,
			    "NameServiceHelper::queryNamingServiceForObject");

      }
    ACE_ENDTRY;

    return false;
  }


  /*!
    \brief Find or construct a naming context bound to the 
    supplied name.
  */
  CosNaming::NamingContext_ptr
  NameServiceHelper::ensureNamingContext(const CosNaming::Name & nom)
  {
    // handle trivial case
    if (nom.length() == 0)
      return m_namingClient.get_context();

    // try the naive way first
    CORBA::Object_var tmp;
    CosNaming::NamingContext_var ctxt;
    
    try
      {
	debugMsg("NameServiceHelper:nameServiceBind",
		 " attempting to resolve naming context '" << nom << "'");
	tmp = m_namingClient->resolve(nom);
	// tmp is a valid object, but is it a naming context?
	debugMsg("NameServiceHelper:nameServiceBind",
		 " resolved '" << nom << "', trying to narrow to naming context");
	try
	  {
	    ctxt = CosNaming::NamingContext::_narrow(tmp.in());
	  }
	catch (CORBA::Exception & e)
	  {
	    std::cerr << "ensureNamingContext: unexpected CORBA exception " << e
		      << " while narrowing to naming context "
		      << nameToEscapedString(nom)
		      << std::endl;
	  }
	if (CORBA::is_nil(ctxt.in()))
	  {
	    debugMsg("NameServiceHelper:nameServiceBind",
		     " parent name already bound to a non-context object");
	  }
	else
	  {
	    debugMsg("NameServiceHelper:nameServiceBind",
		     " successfully narrowed naming context '"
		     << nameToEscapedString(nom) << "'");
	  }
      }
    catch (CosNaming::NamingContext::NotFound & nf)
      {
	// create the missing intervening contexts
	CosNaming::Name parentName =
	  getParentContextName(nf.rest_of_name, nom);
	debugMsg("NameServiceHelper:nameServiceBind",
		 " will attempt to create missing context '"
		 << nf.rest_of_name
		 << "' below '"
		 << nameToEscapedString(parentName)
		 << "'");
	try
	  {
	    CosNaming::NamingContext_var parentCtxt;
	    if (parentName.length() == 0)
	      {
		parentCtxt = m_namingClient.get_context();
	      }
	    else
	      {
		tmp = m_namingClient->resolve(parentName);
		parentCtxt = CosNaming::NamingContext::_narrow(tmp.in());
	      }
	    checkError(!CORBA::is_nil(parentCtxt.in()),
		       "NameServiceHelper:ensureNamingContext: internal error: can't find parent context '"
		       << nameToEscapedString(parentName)
		       << "'");

	    // create and bind new context
	    CosNaming::NamingContext_var newCtxt;
	    try
	      {
		newCtxt = parentCtxt->bind_new_context(nf.rest_of_name);
	      }
	    catch (CosNaming::NamingContext::InvalidName & inv)
	      {
		debugMsg("NameServiceHelper:nameServiceBind",
			 " invalid name exception for '" << nameToEscapedString(nf.rest_of_name)
			 <<"':\n" << inv);
		return CosNaming::NamingContext::_nil();
	      }
	    catch (CosNaming::NamingContext::AlreadyBound & ab)
	      {
		debugMsg("NameServiceHelper:nameServiceBind",
			 " name '" << nameToEscapedString(nf.rest_of_name)
			 << "' is already bound:\n" << ab);
		return CosNaming::NamingContext::_nil();
	      }
	    catch (CORBA::Exception & e)
	      {
		std::cerr << "ensureNamingContext: unexpected CORBA exception " << e
			  << " while binding new naming context '"
			  << nameToEscapedString(nf.rest_of_name) << "'"
			  << std::endl;
		return CosNaming::NamingContext::_nil();
	      }
	      
	    debugMsg("NameServiceHelper:nameServiceBind",
		     " successfully created naming context '"
		     << nameToEscapedString(nom)
		     << "'");
	    return CosNaming::NamingContext::_duplicate(newCtxt.in());
	  }
	catch (CORBA::Exception & e)
	  {
	    checkError(ALWAYS_FAIL,
		       "ensureNamingContext: unexpected CORBA exception " << e
		       << " while creating naming context "
		       << nameToEscapedString(nf.rest_of_name));
	  }
      }
    catch (CosNaming::NamingContext::InvalidName & inv)
      {
	debugMsg("NameServiceHelper:nameServiceBind",
		 " invalid name exception for " << nameToEscapedString(nom));
      }
    catch (CORBA::Exception & e)
      {
	checkError(ALWAYS_FAIL,
		   "ensureNamingContext: unexpected CORBA exception " << e);
      }

    return CosNaming::NamingContext::_duplicate(ctxt.in());
  }

  //
  // Protected static member (helper) fns
  //

  /*!
    \brief Given a NamingContext::NotFound exception's idea of the rest of the name,
    and the name being queried, returns the name of the parent context
  */

  CosNaming::Name 
  NameServiceHelper::getParentContextName(const CosNaming::Name & rest,
                                          const CosNaming::Name & queryName)
  {
    int queryLen = queryName.length();
    int parentLen = queryLen - rest.length();
    if (parentLen <= 0)
      return CosNaming::Name();
    CosNaming::Name result(parentLen);
    result.length(parentLen);
    for (int i = 0; i < parentLen; i++)
      result[i] = queryName[i];
    return result;
  }


  /*!
    \brief Parses namestring according to the OMG rules, 
    and returns the corresponding Name
  */

  // *** TO DO:
  //  - Improve error checking/handling
  //  - Performance improvements

  CosNaming::Name
  NameServiceHelper::parseName(const std::string & namestring)
  {
    if (namestring.length() == 0)
      {
	std::cerr << "WARNING: NameServiceHelper::parseName: empty string to parse" << std::endl;
	return CosNaming::Name();
      }

    std::string::size_type idx = 0;
    std::vector<CosNaming::NameComponent> vec;
    while (idx != std::string::npos)
      {
	vec.push_back(parseNameComponent(namestring, idx));
      }

    // Copy from temp to new Name instance
    CosNaming::Name result(vec.size());
    result.length(vec.size());
    for (unsigned int i = 0; i < vec.size(); i++)
      {
	result[i] = vec[i];
      }

    return result;
  }

  /*!
    \brief Parses namestring according to the OMG rules, 
    and returns the corresponding NameComponent.  
    idx is updated to just past the terminating character, 
    or std::string::npos if none found.
  */

  CosNaming::NameComponent
  NameServiceHelper::parseNameComponent(const std::string & namestring,
                                        std::string::size_type & idx)
  {
    if (idx >= namestring.length())
      return CosNaming::NameComponent();

    std::string::size_type term = 
      findNameComponentEnd(namestring, idx);
    if (term == idx)
      {
	if (namestring.at(term) == s_nameSeparatorChar)
	  // empty name component
	  // *** should this be an error? ***
	  {
	    idx++;
	    return CosNaming::NameComponent();
	  }
	else 
	  {
	    // name component with kind only
	    idx = term + 1;
	    term = findNameComponentEnd(namestring, idx);
	    if (term == std::string::npos)
	      {
		// Whole remaining string is the type
		// *** Should check for redundant type chars ***
		CosNaming::NameComponent result;
		result.kind =
		  makeNameComponentString(namestring, idx);
		idx = std::string::npos;
		return result;
	      }
	    else if (term == idx)
	      {
		// Empty type too
		idx = term + 1;
		return CosNaming::NameComponent();
	      }
	    else 
	      {
		// General case for type
		CosNaming::NameComponent result;
		result.kind =
		  makeNameComponentString(namestring, idx, term);
		idx = term + 1;
		return result;
	      }
	  }
      }
    else if (term == std::string::npos)
      {
	// Whole remaining string is an id
	CosNaming::NameComponent result;
	result.id =
	  makeNameComponentString(namestring, idx);
	idx = std::string::npos;
	return result;
      }
    else
      {
	// General case.  
	// Extract the ID part.
	CosNaming::NameComponent result;
	result.id =
	  makeNameComponentString(namestring, idx, term);
	idx = term + 1;

	if (namestring.at(term) == s_nameKindChar)
	  {
	    // Extract kind part
	    term = findNameComponentEnd(namestring, idx);
	    if (term == std::string::npos)
	      {
		// Whole remaining string is the kind
		// *** Should check for redundant kind chars ***
		result.kind =
		  makeNameComponentString(namestring, idx);
		idx = std::string::npos;
	      }
	    else if (term == idx)
	      {
		// Empty type
		idx = term + 1;
	      }
	    else 
	      {
		// General case for kind
		result.kind =
		  makeNameComponentString(namestring, idx, term);
		idx = term + 1;
	      }
	  }
	return result;

      }
  }

  /*!
    \brief Allocates and initializes a null-terminated character array
    containing the specified substring, minus any escape characters.
  */
  char * 
  NameServiceHelper::makeNameComponentString(const std::string & namestring,
                                             std::string::size_type start, 
                                             std::string::size_type end)
  {
    if (end == std::string::npos)
      end = namestring.length();
    std::string::size_type len = 0;
    char * result = 0;
    
    // Determine length needed by searching for escape chars
    std::string::size_type next =
      namestring.find(s_nameEscapeChar, start);
    if ((next == std::string::npos) // none anywhere
	|| (next > end)) // none in segment
      {
	// Easy case - no escapes found in segment to copy
	len = end - start;
	result = CORBA::string_alloc(len + 1);
	ACE_OS::memcpy(result,
		       namestring.c_str() + start,
		       len);
      }
    else
      {
	// Escape char found in segment to be copied
	std::string::size_type prev = start;
	len = next - prev;
	prev = next + 1; // skip escape
	while (prev < end)
	  {
	    next = namestring.find(s_nameEscapeChar, prev + 1);
	    if ((next == std::string::npos) || (next >= end))
	      {
		len += end - prev;
		break;
	      }
	    else 
	      len += next - prev;
	    prev = next + 1; // skip escape
	  }

	// Allocate result array
	result = CORBA::string_alloc(len + 1);

	// Copy the pieces -- 
	// must scan for escapes again
	char * writePtr = result;
	std::string::size_type segLen;
	if (namestring.at(start) == s_nameEscapeChar)
	  prev = start + 1;
	else
	  prev = start;
	while (prev < end)
	  {
	    next = namestring.find(s_nameEscapeChar, prev + 1);
	    if ((next == std::string::npos) || (next >= end))
	      segLen = end - prev;
	    else
	      segLen = next - prev;
	    ACE_OS::memcpy(writePtr,
			   namestring.c_str() + prev,
			   segLen);
	    if ((next == std::string::npos) || (next >= end))
	      break;
	    writePtr += segLen;
	    prev = next + 1;
	  }
      }

    // Terminate with a null and return it
    result[len] = '\0';
    return result;
  }

  /*!
    \brief Returns the index of the character terminating the name
    component which begins at start, or
    std::string::npos if no terminator found.
  */

  // *** N.B. Can return 0 also if first char is a separator or type char!

  std::string::size_type
  NameServiceHelper::findNameComponentEnd(const std::string & namestring, 
                                          std::string::size_type start)
  {
    if (namestring.length() <= start)
      return 0;
    bool terminated = false;
    std::string::size_type result = start;
    while (!terminated && (result != std::string::npos))
      {
	result = namestring.find_first_of(s_specialNameChars, start);
	if (result != std::string::npos)
	  {
	    // Skip over escape and following character
	    if (namestring.at(result) == s_nameEscapeChar)
	      start = result + 2;
	    // Exit if we found some other special character
	    else
	      terminated = true;
	  }
      }
    return result;
  }

  /*!
    \brief Formats the Name onto the stream, 
    in escaped format suitable for parseName()
  */
  void
  NameServiceHelper::printName(const CosNaming::Name & nom, ostream & str)
  {
    unsigned int i = 0;
    while (i < nom.length())
      {
	printNameComponent(nom[i], str);
	i++;
	if (i < nom.length())
	  str << s_nameSeparatorChar;
      }
  }

  /*!
    \brief Formats the Name into a string suitable for parsing
    with parseName()
  */
  std::string
  NameServiceHelper::nameToEscapedString(const CosNaming::Name & nom)
  {
    std::ostringstream strm;
    printName(nom, strm);
    return std::string(strm.str());
  }

  void
  NameServiceHelper::printNameComponent(const CosNaming::NameComponent & comp,
                                        ostream & strm)
  {
    if (strlen(comp.id) != 0)
      {
	// Print ID, respecting escape chars
	int len = strlen(comp.id);
	for (int i = 0; i < len; i++)
	  {
	    char c = comp.id[i];
	    if ((c == s_nameSeparatorChar)
		|| (c == s_nameKindChar)
		|| (c == s_nameEscapeChar))
	      strm.put(s_nameEscapeChar);
	    strm.put(c);
	  }
      }
    if (strlen(comp.kind) != 0)
      {
	strm << s_nameKindChar;
	// print kind, respecting escape chars
	int len = strlen(comp.kind);
	for (int i = 0; i < len; i++)
	  {
	    char c = comp.kind[i];
	    if ((c == s_nameSeparatorChar)
		|| (c == s_nameKindChar)
		|| (c == s_nameEscapeChar))
	      strm.put(s_nameEscapeChar);
	    strm.put(c);
	  }
      }
  }
  
}
