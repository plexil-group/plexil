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

#ifndef EXPRESSION_FACTORY_HH
#define EXPRESSION_FACTORY_HH

#include "Expression.hh"

// ***********************************
// * TEMPORARY - TO BE REPLACED SOON *
// ***********************************

namespace PLEXIL
{
  // Forward declarations
  class NodeConnector;
  DECLARE_ID(NodeConnector);
  
  class PlexilExpr;
  DECLARE_ID(PlexilExpr);

  /**
   * @brief Factory class for Expressions.
   * This allows you to write, e.g., \<AND\> in XML and have the correct Expression instantiated.
   */
  class ExpressionFactory {
  public:

    /**
     * @brief Creates a new Expression instance with the type associated with the
     *        given expression prototype.
     * @param expr The PlexilExprId to be passed to the Expression constructor.
     * @param node Node for name lookup.
     * @return The Id for the new Expression. May not be unique.
     * @note Convenience wrapper.
     */
    static ExpressionId createInstance(const PlexilExprId& expr,
                                       const NodeConnectorId& node = NodeConnectorId::noId());

    /**
     * @brief Creates a new Expression instance with the type associated with the name and
     *        the given expression prototype.
     * @param name The registered name for the factory.
     * @param expr The PlexilExprId to be passed to the Expression constructor.
     * @param node Node for name lookup.
     * @return The Id for the new Expression.  May not be unique.
     * @note Convenience wrapper.
     */
    static ExpressionId createInstance(const std::string& name,
                                       const PlexilExprId& expr,
                                       const NodeConnectorId& node = NodeConnectorId::noId());

    /**
     * @brief Creates a new Expression instance with the type associated with the
     *        given expression prototype.
     * @param expr The PlexilExprId to be passed to the Expression constructor.
     * @param node Node for name lookup.
     * @return The Id for the new Expression. May not be unique.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @note Convenience wrapper for most common use case.
     */
    static ExpressionId createInstance(const PlexilExprId& expr,
                                       const NodeConnectorId& node,
                                       bool& wasCreated);

    /**
     * @brief Creates a new Expression instance with the type associated with the name and
     *        the given configuration XML.
     * @param name The registered name for the factory.
     * @param expr The PlexilExprId to be passed to the Expression constructor.
     * @param node Node for name lookup.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new Expression.  If wasCreated is set to false, is not unique.
     */
    static ExpressionId createInstance(const std::string& name,
                                       const PlexilExprId& expr,
                                       const NodeConnectorId& node,
                                       bool& wasCreated);

    const std::string& getName() const
    {
      return m_name;
    }

    /**
     * @brief Deallocate all factories
     */
    static void purge();

  protected:

    virtual ~ExpressionFactory()
    {}

    /**
     * @brief Registers an ExpressionFactory with the specific name.
     * @param name The name by which the Expression shall be known.
     * @param factory The ExpressionFactory instance.
     */
    static void registerFactory(const std::string& name, ExpressionFactory* factory);

    virtual ExpressionId create(const PlexilExprId& expr,
                                const NodeConnectorId& node = NodeConnectorId::noId()) const = 0;

    ExpressionFactory(const std::string& name)
    : m_name(name)
    {
      registerFactory(m_name, this);
    }

  private:

    /**
     * @brief The map from names to ConcreteExpressionFactory instances.
     * This pattern of wrapping static data in a static method is to ensure proper loading
     * when used as a shared library.
     */
    static std::map<std::string, ExpressionFactory*>& factoryMap();

    const std::string m_name; /*!< Name used for lookup */
  };

  /**
   * @brief Concrete factory class, templated for each Expression type.
   */
  template<class FactoryType>
  class ConcreteExpressionFactory : public ExpressionFactory {
  public:
    ConcreteExpressionFactory(const std::string& name) : ExpressionFactory(name) {}

  private:
    /**
     * @brief Instantiates a new Expression of the appropriate type.
     * @param expr The PlexilExprId for the instantiated Expression.
     * @param node
     * @return The Id for the new Expression.
     */

    virtual ExpressionId create(const PlexilExprId& expr,
                                const NodeConnectorId& node = NodeConnectorId::noId()) const
    {return (new FactoryType(expr, node))->getId();}
  };


  /**
   * @brief Variant of above for constant values.
   */
  template<class FactoryType>
  class ConstantExpressionFactory : public ExpressionFactory {
  public:
    ConstantExpressionFactory(const std::string& name) : ExpressionFactory(name) {}
  private:
    /**
     * @brief Instantiates a new Expression of the appropriate type.
     * @param expr The PlexilExprId for the instantiated Expression.
     * @param node
     * @return The Id for the new Expression.
     */

    virtual ExpressionId create(const PlexilExprId& expr,
                                const NodeConnectorId& node = NodeConnectorId::noId()) const
    {return (new FactoryType(expr, node, true))->getId();}
  };

}

// Convenience macros
#define REGISTER_EXPRESSION(CLASS,NAME) {new PLEXIL::ConcreteExpressionFactory<CLASS>(#NAME);}
#define REGISTER_CONSTANT_EXPRESSION(CLASS,NAME) {new PLEXIL::ConstantExpressionFactory<CLASS>(#NAME);}

#endif // EXPRESSION_FACTORY_HH
