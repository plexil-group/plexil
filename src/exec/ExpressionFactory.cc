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

#include "ExpressionFactory.hh"
#include "Debug.hh"
#include "Expression.hh"
#include "Node.hh"
#include "Variable.hh"

namespace PLEXIL
{


  void ExpressionFactory::registerFactory(const LabelStr& name, ExpressionFactory* factory) {
    check_error(factory != NULL);
    checkError(factoryMap().find(name) == factoryMap().end(),
	       "Error:  Attempted to register a factory for name '" << name.toString() <<
	       "' twice.");
    factoryMap()[name] = factory;
    debugMsg("ExpressionFactory:registerFactory",
	     "Registered factory for name '" << name.toString() << "'");
  }


   ExpressionId ExpressionFactory::createInstance(const LabelStr& name,
                                                  const PlexilExprId& expr,
                                                  const NodeConnectorId& node)
   {
     bool dummy;
     return createInstance(name, expr, node, dummy);
   }

   ExpressionId ExpressionFactory::createInstance(const LabelStr& name,
                                                  const PlexilExprId& expr,
                                                  const NodeConnectorId& node,
                                                  bool& wasCreated)
   {
      // if this is a variable ref, look it up
      
      if (Id<PlexilVarRef>::convertable(expr)) 
      {
         checkError(node.isValid(), "Need a valid Node argument to find a Variable");
         ExpressionId retval = node->findVariable(expr);         
	 checkError(retval.isValid(), "Unable to find variable '" << expr->name() << "'");
         wasCreated = false;
         return retval;
      }

      // otherwise look up factory
      
      std::map<double, ExpressionFactory*>::const_iterator it = factoryMap().find(name);
      checkError(it != factoryMap().end(),
                 "Error: No factory registered for name '" << name.toString() << "'.");
      ExpressionId retval = it->second->create(expr, node);
      debugMsg("ExpressionFactory:createInstance", "Created " << retval->toString());
      wasCreated = true;
      return retval;
   }

  std::map<double, ExpressionFactory*>& ExpressionFactory::factoryMap() {
    static std::map<double, ExpressionFactory*>* sl_map = NULL;
    if (sl_map == NULL)
      sl_map = new std::map<double, ExpressionFactory*>();

    return *sl_map;
  }

  void ExpressionFactory::purge() {
    for(std::map<double, ExpressionFactory*>::iterator it = factoryMap().begin();
	it != factoryMap().end(); ++it)
      delete it->second;
    factoryMap().clear();
  }

}
