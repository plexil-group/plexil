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

#include "PlexilPlan.hh"
#include "ExecDefs.hh"
#include "Utils.hh"
#include "Debug.hh"
#include "Node.hh"
#include <sstream>

namespace PLEXIL {

   const VarType STRING_TYPE("String");

   // convert plexil varible type (which is basically a string) to a
   // plexil type enum

   PlexilType VarType::plexilType() const
   {
      if (*this == "Integer")
         return INTEGER;
      if (*this == "Real")
         return REAL;
      if (*this == "Boolean")
         return BOOLEAN;
      if (*this == "BLOB")
         return BLOB;
      if (*this == "Array")
         return ARRAY;
      if (*this == "String")
         return STRING;
      if (*this == "Time")
         return TIME;
      
      return UNKNOWN;
   }

   void PlexilState::setName(const std::string& name)
   {
      PlexilValue* pv = new PlexilValue("String", name);
      setNameExpr(pv->getId());
   }
   
   const std::string& PlexilState::name() const
   {
      if (Id<PlexilValue>::convertable(m_nameExpr))
         return ((PlexilValue*)&(*m_nameExpr))->value();
      return m_nameExpr->name();
   }
   
  PlexilNode::PlexilNode() : m_id(this), m_priority(WORST_PRIORITY) {}

  PlexilNode::~PlexilNode() {
    m_id.remove();
    //delete everything here
//     cleanup(m_declarations);
//     cleanup(m_conditions);
//     if(m_nodeBody.isValid())
//       delete m_nodeBody;
//     if(m_intf.isValid())
//       delete m_intf;
  }

 PlexilInterface::~PlexilInterface() {
    m_id.remove();
//     cleanup(m_in);
//     cleanup(m_inOut);
  }

      // find a variable in the set of In variables

   const PlexilVarRef* PlexilInterface::findInVar(const PlexilVarRef* target)
   {
      return findInVar(target->name());
   }
      // find a variable in the set of InOut variables

   const PlexilVarRef* PlexilInterface::findInOutVar(const PlexilVarRef* target)
   {
      return findInOutVar(target->name());
   }
      // find a var in the interface

   const PlexilVarRef* PlexilInterface::findVar(const PlexilVarRef* target)
   {
      return findVar(target->name());
   }
      // find a variable in the set of In variables

   const PlexilVarRef* PlexilInterface::findInVar(const std::string& target)
   {
      for (std::vector<PlexilVarRef*>::const_iterator var = m_in.begin();
           var != m_in.end(); ++var)
      {
         if (target == (*var)->name())
            return *var;
      }
      return NULL;
   }
      // find a variable in the set of InOut variables

   const PlexilVarRef* PlexilInterface::findInOutVar(const std::string& target)
   {
      for (std::vector<PlexilVarRef*>::const_iterator var = m_inOut.begin();
           var != m_inOut.end(); ++var)
      {
         if (target == (*var)->name())
            return *var;
      }
      return NULL;
   }
      // find a var in the interface

   const PlexilVarRef* PlexilInterface::findVar(const std::string& target)
   {
      const PlexilVarRef* var = findInVar(target);
      return var == NULL ? findInOutVar(target) : var;
   }
         

  PlexilValue::PlexilValue(const VarType& type, const std::string& value)
    : PlexilExpr(), m_type(type), m_value(value) {setName(m_type + "Value");}

   PlexilArrayValue::PlexilArrayValue(
      const VarType& type,
      unsigned maxSize,
      const std::vector<std::string>& values)
      : PlexilValue(type), m_maxSize(maxSize), m_values(values)
   {
      setName("ArrayValue");
   }
   
   PlexilVar::PlexilVar(const std::string& name, const VarType& type, 
                        const std::string& value)
      : m_id(this), m_name(name), m_value(new PlexilValue(type, value))
   {
   }
   
   PlexilVar::PlexilVar(const std::string& name, const VarType& type, 
                        PlexilValue* value)
      : m_id(this), m_name(name), m_value(value)
   {
   }
   
   PlexilVar::~PlexilVar()
   {
      m_id.remove();
   }
   

   PlexilArrayVar::PlexilArrayVar(const std::string& name, 
                                  const VarType& type, 
                                  const unsigned maxSize, 
                                  std::vector<std::string>& values)
      : PlexilVar(name, type, new PlexilArrayValue(type, maxSize, values)),
        m_maxSize(maxSize)
   {
      checkError(values.size() <= m_maxSize,
                 "Number of initial values of " << type << 
                 " array variable \'" << name << 
                 "\' exceeds maximun of " << m_maxSize);
   }
   
   PlexilArrayVar::~PlexilArrayVar() 
   {
      //m_id.remove();
   }


  PlexilArrayElement::PlexilArrayElement()
    : PlexilExpr()
  {
    setName("ArrayElement");
  }

  void PlexilArrayElement::setArrayName(const std::string& name)
  {
    m_arrayName = name;
  }


      // wrapper call for link which creates the seen library nodes
      // data structure before calling the recurisive linker

   void PlexilNode::link(const std::vector<PlexilNodeId>& libraries)
   {
      PlexilNodeIdSet* seen = new PlexilNodeIdSet();
      link(libraries, *seen);
      delete seen;
   }
      // resolve links between the plan and a library node, returns the number
      // of unresolved library node calls present after linking is completed
   
   void PlexilNode::link(const std::vector<PlexilNodeId>& libraries, PlexilNodeIdSet& seen)
   {
         // if this is a library node call, find matching node in libraries

      if (nodeType() == Node::LIBRARYNODECALL().toString())
      {
         Id<PlexilLibNodeCallBody> & body = (Id<PlexilLibNodeCallBody> &)m_nodeBody;

            // loop through the provided libraries

         for (std::vector<PlexilNodeId>::const_iterator library = libraries.begin();
              library  != libraries.end(); ++library)
         {
               // if the called node name matches the library node name
            
            if (body->libNodeName() == (*library)->nodeId())
            {
                  // test for a circular library reference

               for (PlexilNodeIdSet::iterator seenLib = seen.begin(); 
                    seenLib != seen.end(); ++seenLib)
               {
                  checkError(**seenLib != *library, "Circular library reference: "
                             << body->libNodeName());
               }
                  // link the the two nodes

               body->setLibNode(*library);

                  // add this to the seen library nodes

               seen.push_back(&(*library));

                  // resolve any library calls in the library

               (*library)->link(libraries, seen);

                  // now remove said item from the seen set  (pop the stack)
               
               seen.pop_back();

                  // return to avoide unresolved library call error

               return;
            }
         }
            // no libary node found, report unresolved library call error
            
         checkError(false, "Unresolved library call: " << body->libNodeName());
      }
         // if this is a list node, recurse into it's children
      
      else if (nodeType() == Node::LIST().toString())
      {
         Id<PlexilListBody> & body = (Id<PlexilListBody> &)m_nodeBody;
         
            // iterate through the list nodes children and check for library calls
         
         const std::vector<PlexilNodeId>& children = body->children();
         for(std::vector<PlexilNodeId>::const_iterator child = children.begin();
             child != children.end(); ++child)
            (*child)->link(libraries, seen);
      }
   }
}
