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

#ifndef PLEXIL_EXPR_PROTO_HH
#define PLEXIL_EXPR_PROTO_HH

#include "Id.hh"
#include "ValueType.hh"

#include <string>

namespace PLEXIL
{
  // Forward references
  class PlexilVar;
  DECLARE_ID(PlexilVar);

  class PlexilExpr;
  DECLARE_ID(PlexilExpr);

  class PlexilExpr {
  public:
    PlexilExpr(std::string const &name = "", 
               ValueType typ = UNKNOWN_TYPE);
    virtual ~PlexilExpr();
    const PlexilExprId& getId() const;

    virtual const std::string& name() const;
    bool typed() const;
    virtual ValueType type() const; // can be overridden
    int lineNo() const;
    int colNo() const;

    void setName(const std::string& name);
    void setType(ValueType type);
    void setLineNo(int n);
    void setColNo(int n);

  private:
    PlexilExprId m_id;

  protected:
    std::string m_name;

  private:
    int m_lineNo;
    int m_colNo;

  protected:
    ValueType m_type;
    bool m_typed;
  };

  class PlexilVarRef : public PlexilExpr
  {
  public:
    PlexilVarRef();
    ~PlexilVarRef();

    const PlexilExprId& defaultValue() const;
    const PlexilVarId& variable() const;
         
    void setDefaultValue(const PlexilExprId& defaultValue);
    void setVariable(const PlexilVarId& var);

  private:
    PlexilExprId m_defaultValue;
    PlexilVarId m_variable;
  };

  class PlexilOp : public PlexilExpr {
  public:
    PlexilOp(std::string const &op = "", 
             ValueType typ = UNKNOWN_TYPE);
    virtual ~PlexilOp();

    const std::string& getOp() const;
    void setOp(const std::string& op);

    const std::vector<PlexilExprId>& subExprs() const;
    void addSubExpr(PlexilExprId expr);

  private:
    std::vector<PlexilExprId> m_subExprs;
  };

  class PlexilArrayElement : public PlexilExpr {
  public:
    PlexilArrayElement();
    virtual ~PlexilArrayElement();

    const std::string& getArrayName() const;
    void setArrayName(const std::string& name);

    const std::vector<PlexilExprId>& subExprs() const;
    void addSubExpr(PlexilExprId expr);

  private:
    std::string m_arrayName;
    std::vector<PlexilExprId> m_subExprs;
  };

  class PlexilValue : public PlexilExpr
  {
  public:
    PlexilValue(ValueType type, const std::string& value = "UNKNOWN");
    ~PlexilValue();

    const std::string& value() const;
    virtual std::string const & name() const;

  private:
    std::string m_value;
  };

  class PlexilArrayValue : public PlexilValue
  {
  public:
    PlexilArrayValue(ValueType eltType,
                     unsigned maxSize,
                     const std::vector<std::string>& values);
    ~PlexilArrayValue();
    const std::vector<std::string>& values() const;
    virtual ValueType type() const;
    ValueType elementType() const;
    unsigned maxSize() const;

  private:
    unsigned m_maxSize;
    std::vector<std::string> m_values;
  };

  class PlexilVar : public PlexilExpr
  {
  public:
    PlexilVar(const std::string& name, ValueType type);
    PlexilVar(const std::string& name, ValueType type, const std::string& value);
    PlexilVar(const std::string& name, ValueType type, PlexilValue* value);
    virtual ~PlexilVar();

    virtual bool isArray() const;

    const PlexilVarId& getId() const;
    virtual const std::string& factoryTypeString() const;
    const PlexilValue* value() const;

  private:
    PlexilVarId m_varId;
    PlexilValue* m_value;
  };
  
  class PlexilArrayVar : public PlexilVar 
  {
  public:
    PlexilArrayVar(const std::string& name, 
                   ValueType eltType, 
                   const unsigned maxSize);
    PlexilArrayVar(const std::string& name, 
                   ValueType eltType, 
                   const unsigned maxSize, 
                   std::vector<std::string>& values);
    ~PlexilArrayVar();

    // override PlexilVar method

    virtual const std::string& factoryTypeString() const;
    virtual bool isArray() const;
    ValueType type() const;
    ValueType elementType() const;
    virtual unsigned maxSize() const;

  private:
    unsigned m_maxSize;
  };

  DECLARE_ID(PlexilArrayVar);

} // namespace PLEXIL

#endif // PLEXIL_EXPR_PROTO_HH

