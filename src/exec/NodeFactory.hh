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

#ifndef NODE_FACTORY_HH
#define NODE_FACTORY_HH

#include "NodeConstants.hh"
#include "PlexilNodeType.hh"
#include "PlexilPlan.hh"

namespace PLEXIL
{

  class Node;

  /**
   * @brief Abstract factory class for Node instances.
   */

  class NodeFactory
  {
  public:

    // *** TO BE DELETED ***
    /**
     * @brief Primary factory method.
     */
    static Node *createNode(PlexilNode const *nodeProto, 
                            Node *parent = NULL);

    /**
     * @brief New primary factory method.
     */
    static Node *createNode(char const *name,
                            char const *type,
                            Node *parent = NULL);

    /**
     * @brief Alternate factory method.  Used only by Exec test module.
     */
    static Node *createNode(const std::string& type, 
                            const std::string& name, 
                            NodeState state,
                            Node *parent = NULL);

    static void ensureNodeFactoriesRegistered();

    static void purge();

  protected:
    NodeFactory(PlexilNodeType nodeType);
    virtual ~NodeFactory();

    /**
     * @brief Primary factory method.
     */
    virtual Node *create(PlexilNode const *node, 
                         Node *parent = NULL) const = 0;

    /**
     * @brief Alternate factory method.  Used only by Exec test module.
     */

    virtual Node *create(const std::string& type,
                         const std::string& name,
                         NodeState state,
                         Node *parent = NULL) const = 0;

    PlexilNodeType m_nodeType;

  private:
    // Deliberately unimplemented
    NodeFactory();
    NodeFactory(const NodeFactory&);
    NodeFactory& operator=(const NodeFactory&);

    static NodeFactory** factoryMap(); // reference to array of pointers was just too hard!
  };

  template<class NODE_TYPE>
  class ConcreteNodeFactory : public NodeFactory
  {
  public:
    ConcreteNodeFactory(PlexilNodeType nodeType)
    : NodeFactory(nodeType)
    {
    }

    virtual ~ConcreteNodeFactory()
    {
    }

  private:
    // Deliberately unimplemented
    ConcreteNodeFactory();
    ConcreteNodeFactory(const ConcreteNodeFactory&);
    ConcreteNodeFactory& operator=(const ConcreteNodeFactory&);

    Node *create(PlexilNode const *nodeProto, 
                 Node *parent) const;

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */

    Node *create(const std::string& type,
                 const std::string& name, 
                 NodeState state,
                 Node *parent) const;

  };

}

#endif // NODE_FACTORY_HH
