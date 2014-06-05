/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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
#include "NodeConnector.hh"
#include "PlexilExpr.hh"

namespace PLEXIL
{
  ExpressionFactory::ExpressionFactory(const std::string& name)
    : m_name(name)
  {
    registerFactory(m_name, this);
  }

  ExpressionFactory::~ExpressionFactory()
  {
  }

  const std::string& ExpressionFactory::getName() const
  {
    return m_name;
  }

  void ExpressionFactory::registerFactory(const std::string& name, ExpressionFactory* factory) 
  {
    check_error(factory != NULL);
    checkError(factoryMap().find(name) == factoryMap().end(),
               "Error:  Attempted to register a factory for name \"" << name <<
               "\" twice.");
    factoryMap()[name] = factory;
    debugMsg("ExpressionFactory:registerFactory",
             "Registered factory for name \"" << name << "\"");
  }

  ExpressionId ExpressionFactory::createInstance(const PlexilExprId& expr,
                                                 const NodeConnectorId& node)
  {
    const std::string& name = expr->name();
    bool dummy;
    return createInstance(name, expr, node, dummy);
  }

  ExpressionId ExpressionFactory::createInstance(const std::string& name,
                                                 const PlexilExprId& expr,
                                                 const NodeConnectorId& node)
  {
    bool dummy;
    return createInstance(name, expr, node, dummy);
  }

  ExpressionId ExpressionFactory::createInstance(const PlexilExprId& expr,
                                                 const NodeConnectorId& node,
                                                 bool& wasCreated)
  {
    const std::string& name = expr->name();
    return createInstance(name, expr, node, wasCreated);
  }

  ExpressionId ExpressionFactory::createInstance(const std::string& name,
                                                 const PlexilExprId& expr,
                                                 const NodeConnectorId& node,
                                                 bool& wasCreated)
  {
    // if this is a variable ref, look it up
    if (Id<PlexilVarRef>::convertable(expr)) {
      assertTrueMsg(node.isValid(),
                    "Need a valid Node argument to find a Variable");
      ExpressionId retval = node->findVariable(expr);         
      assertTrueMsg(retval.isValid(),
                    "Unable to find variable \"" << expr->name() << "\"");
      wasCreated = false;
      return retval;
    }

    // otherwise look up factory
    std::map<std::string, ExpressionFactory*>::const_iterator it = factoryMap().find(name);
    assertTrueMsg(it != factoryMap().end(),
                  "Error: No factory registered for name \"" << name << "\".");
    ExpressionId retval = it->second->create(expr, node);
    debugMsg("ExpressionFactory:createInstance", "Created " << retval->toString());
    wasCreated = true;
    return retval;
  }

  std::map<std::string, ExpressionFactory*>& ExpressionFactory::factoryMap()
  {
    static std::map<std::string, ExpressionFactory*> sl_map;
    return sl_map;
  }

  void ExpressionFactory::purge()
  {
    for (std::map<std::string, ExpressionFactory*>::iterator it = factoryMap().begin();
         it != factoryMap().end();
         ++it) {
      ExpressionFactory* tmp = it->second;
      it->second = NULL;
      delete tmp;
    }
    factoryMap().clear();
  }

}
