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

#ifndef PLEXIL_CONCRETE_EXPRESSION_FACTORY_HH
#define PLEXIL_CONCRETE_EXPRESSION_FACTORY_HH

#include "ExpressionFactory.hh"
#include "NodeConnector.hh"
#include "PlexilExpr.hh"

namespace PLEXIL
{
  // Forward references
  template <typename T> class ArrayVariable;
  template <typename T> class Constant;
  template <typename T> class UserVariable;

  class ArrayReference;
  class MutableArrayReference;

  template <class EXPR>
  class ConcreteExpressionFactory : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const *expr,
                         NodeConnector *node,
                         bool &wasCreated) const = 0;

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const = 0;

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  template <typename T>
  class ConcreteExpressionFactory<Constant<T> > : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const *expr,
                         NodeConnector *node,
                         bool &wasCreated) const;

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const;

  protected:
    Expression *create(PlexilValue const *expr) const;

    Expression *create(pugi::xml_node const &expr) const;

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  // Specialization for array constants

  template <typename T>
  class ConcreteExpressionFactory<Constant<ArrayImpl<T> > > : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const *expr,
                         NodeConnector *node,
                         bool &wasCreated) const;

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const;

  protected:
    Expression *create(PlexilArrayValue const *expr) const;

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  template <typename T>
  class ConcreteExpressionFactory<UserVariable<T> > : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const *expr,
                         NodeConnector *node,
                         bool &wasCreated) const;

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const;

  protected:
    Expression *create(PlexilVar const *expr,
                       NodeConnector *node) const;

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  template <typename T>
  class ConcreteExpressionFactory<ArrayVariable<T> > : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(const std::string& name)
      : ExpressionFactory(name) 
    {
    }

    ~ConcreteExpressionFactory()
    {
    }

    Expression *allocate(PlexilExpr const *expr,
                         NodeConnector *node,
                         bool &wasCreated) const;

    Expression *allocate(pugi::xml_node const &expr,
                         NodeConnector *node,
                         bool &wasCreated) const;

  protected:
    Expression *create(PlexilArrayVar const *expr,
                       NodeConnector *node) const;

  private:
    // Default, copy, assign all prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(const ConcreteExpressionFactory &);
    ConcreteExpressionFactory &operator=(const ConcreteExpressionFactory &);
  };

  template <>
  class ConcreteExpressionFactory<ArrayReference> : public ExpressionFactory
  {
  public:
    ConcreteExpressionFactory(std::string const &name)
      : ExpressionFactory(name)
    {
    }
    ~ConcreteExpressionFactory()
    {
    }

    virtual Expression *allocate(PlexilExpr const *expr,
                                 NodeConnector *node,
                                 bool & wasCreated) const;

    virtual Expression *allocate(pugi::xml_node const &expr,
                                 NodeConnector *node,
                                 bool & wasCreated) const;

  private:
    // Default, copy, assign prohibited
    ConcreteExpressionFactory();
    ConcreteExpressionFactory(ConcreteExpressionFactory const &);
    ConcreteExpressionFactory &operator=(ConcreteExpressionFactory const &);
  };

  class VariableReferenceFactory : public ExpressionFactory
  {
  public:
    VariableReferenceFactory(std::string const &name)
      : ExpressionFactory(name)
    {
    }

    ~VariableReferenceFactory()
    {
    }

    virtual Expression *allocate(PlexilExpr const *expr,
                                 NodeConnector *node,
                                 bool & wasCreated) const;

    virtual Expression *allocate(pugi::xml_node const &expr,
                                 NodeConnector *node,
                                 bool & wasCreated) const;

  private:
    // Default, copy, assign prohibited
    VariableReferenceFactory();
    VariableReferenceFactory(VariableReferenceFactory const &);
    VariableReferenceFactory &operator=(VariableReferenceFactory const &);
  };

} // namespace PLEXIL

// Convenience macros
#define ENSURE_EXPRESSION_FACTORY(CLASS) template class PLEXIL::ConcreteExpressionFactory<CLASS >;
#define REGISTER_EXPRESSION(CLASS,NAME) {new PLEXIL::ConcreteExpressionFactory<CLASS >(#NAME);}

#endif // PLEXIL_CONCRETE_EXPRESSION_FACTORY_HH
