/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

#ifndef _H_PlexilPlan
#define _H_PlexilPlan

#include <list>
#include <vector>
#include <map>
#include <string>
#include "Id.hh"
#include "LabelStr.hh"
#include "ExecDefs.hh"
#include "PlexilResource.hh"

// Take care of annoying VxWorks macro
#undef UPDATE

#define isType(p,type) (dynamic_cast<type*>(p) != NULL)

namespace PLEXIL 
{

  class PlexilInterface;
  class PlexilVar;
  class PlexilArrayVar;
  class PlexilValue;
  class PlexilNodeBody;
  class PlexilVarRef;
  class PlexilNode;
  class PlexilState;
  class PlexilUpdate;
  class PlexilNodeRef;
  class PlexilInternalVar;

  DECLARE_ID(PlexilNode);
  DECLARE_ID(PlexilInterface);
  DECLARE_ID(PlexilState);
  DECLARE_ID(PlexilVar);
  DECLARE_ID(PlexilArrayVar);
  DECLARE_ID(PlexilNodeBody);
  DECLARE_ID(PlexilUpdate);
  DECLARE_ID(PlexilNodeRef);
  DECLARE_ID(PlexilInternalVar);

  typedef std::vector<const PlexilNode*> PlexilNodeSet;
  typedef std::map<double, PlexilExprId>  PlexilAliasMap;

  enum PlexilNodeType
    {
      NodeType_uninitialized = 0,
      NodeType_NodeList,
      NodeType_Command,
      NodeType_Assignment,
      NodeType_Update,
      NodeType_Empty,
      NodeType_LibraryNodeCall,
      NodeType_error
    };

  class PlexilParser 
  {
  public:
    //node types
    DECLARE_STATIC_CLASS_CONST(std::string, LIST, "NodeList");
    DECLARE_STATIC_CLASS_CONST(std::string, COMMAND, "Command");
    DECLARE_STATIC_CLASS_CONST(std::string, ASSIGNMENT, "Assignment");
    DECLARE_STATIC_CLASS_CONST(std::string, UPDATE, "Update");
    DECLARE_STATIC_CLASS_CONST(std::string, EMPTY, "Empty");
    DECLARE_STATIC_CLASS_CONST(std::string, LIBRARYNODECALL, "LibraryNodeCall");

    // value types
    DECLARE_STATIC_CLASS_CONST(std::string, INTEGER_STR, "Integer");
    DECLARE_STATIC_CLASS_CONST(std::string, REAL_STR, "Real");
    DECLARE_STATIC_CLASS_CONST(std::string, BOOL_STR, "Boolean");
    DECLARE_STATIC_CLASS_CONST(std::string, BLOB_STR, "BLOB");
    DECLARE_STATIC_CLASS_CONST(std::string, ARRAY_STR, "Array");
    DECLARE_STATIC_CLASS_CONST(std::string, STRING_STR, "String");
    DECLARE_STATIC_CLASS_CONST(std::string, TIME_STR, "Time");
    DECLARE_STATIC_CLASS_CONST(std::string, NODE_STATE_STR, "NodeState");
    DECLARE_STATIC_CLASS_CONST(std::string, NODE_OUTCOME_STR, "NodeOutcome");
    DECLARE_STATIC_CLASS_CONST(std::string, NODE_FAILURE_STR, "NodeFailure");
    DECLARE_STATIC_CLASS_CONST(std::string, NODE_COMMAND_HANDLE_STR, "NodeCommandHandle");

    // condition names
    DECLARE_STATIC_CLASS_CONST(std::string, START_CONDITION_TAG, "StartCondition");
    DECLARE_STATIC_CLASS_CONST(std::string, REPEAT_CONDITION_TAG, "RepeatCondition");
    DECLARE_STATIC_CLASS_CONST(std::string, PRE_CONDITION_TAG, "PreCondition");
    DECLARE_STATIC_CLASS_CONST(std::string, POST_CONDITION_TAG, "PostCondition");
    DECLARE_STATIC_CLASS_CONST(std::string, INVARIANT_CONDITION_TAG, "InvariantCondition");
    DECLARE_STATIC_CLASS_CONST(std::string, END_CONDITION_TAG, "EndCondition");
    DECLARE_STATIC_CLASS_CONST(std::string, EXIT_CONDITION_TAG, "ExitCondition");
    DECLARE_STATIC_CLASS_CONST(std::string, SKIP_CONDITION_TAG, "SkipCondition");

    DECLARE_STATIC_CLASS_CONST(std::string, UNKNOWN_STR, "UNKNOWN");

    // miscellaneous constants
    DECLARE_STATIC_CLASS_CONST(std::string, WHITESPACE_CHARS, " \t\n");

    static PlexilNodeType parseNodeType(const std::string & typeName);
    static const std::string& nodeTypeString(PlexilNodeType nodeType);


    static PlexilType parseValueTypePrefix(const std::string & str, 
                                           std::string::size_type prefixLen);
    inline static PlexilType parseValueType(const std::string& typeStr)
    {
      return parseValueTypePrefix(typeStr, typeStr.length());
    }
    static const std::string& valueTypeString(const PlexilType& type);

  private:
    // Deliberately not implemented because this class can't be instantiated,
    // only derived from.
    PlexilParser();
    PlexilParser(const PlexilParser&);
    PlexilParser& operator=(const PlexilParser&);
    ~PlexilParser();

  };

  class PlexilNode 
  {
  public:
    PlexilNode();
    ~PlexilNode();

    const PlexilNodeId& getId() const {return m_id;}
    
    const std::string& fileName() const {return m_fileName;}
    int lineNo() const {return m_lineNo;}
    int colNo() const {return m_colNo;}

    const std::string& nodeId() const {return m_nodeId;}
    PlexilNodeType nodeType() const {return m_nodeType;}
    const std::string& nodeTypeString() const {return PlexilParser::nodeTypeString(m_nodeType);}
    double priority() const {return m_priority;}
    const PlexilInterfaceId& interface() const {return m_intf;}
    const std::vector<PlexilVarId>& declarations() const {return m_declarations;}
    const std::map<std::string, PlexilExprId>& conditions() const {return m_conditions;}
    const PlexilNodeBodyId& body() const {return m_nodeBody;}

    /**
     * @brief Get the names of all library nodes referenced by this node and its descendants.
     * @return A vector of library node names.
     */
    std::vector<std::string> getLibraryReferences() const;
    void getLibraryReferences(std::vector<std::string>& refs) const;

    void setFileName(const std::string& fname) {m_fileName = fname;}
    void setFileName(const char* fname) {m_fileName = fname;}
    void setLineNo(int n) {m_lineNo = n;}
    void setColNo(int n) {m_colNo = n;}

    void setNodeId(const std::string& id) {m_nodeId = id;}
    void setNodeType(PlexilNodeType type) {m_nodeType = type;}
    void setPriority(double priority) {m_priority = priority;}
    void addVariable(const PlexilVarId& var) {m_declarations.push_back(var);}
    void addCondition(const std::string& name, const PlexilExprId& expr)
    {m_conditions.insert(std::make_pair(name, expr));}
    void setBody(const PlexilNodeBodyId& body) {m_nodeBody = body;}
    void setInterface(const PlexilInterfaceId& intf) { m_intf = intf;}

    /**
     * @brief Recurse into given node and link library calls found therein.
     */
    bool link(const std::map<std::string, PlexilNodeId>& libraries);

  private:

    // Internal methods
    bool link(const std::map<std::string, PlexilNodeId>& libraries, PlexilNodeSet& seen);

    PlexilNodeType m_nodeType;
    int m_lineNo;
    int m_colNo;
    double m_priority;

    PlexilNodeId m_id;
    PlexilInterfaceId m_intf;
    PlexilNodeBodyId m_nodeBody;

    std::string m_fileName;
    std::string m_nodeId;
    std::vector<PlexilVarId> m_declarations;
    std::map<std::string, PlexilExprId> m_conditions;
  };
  
  class PlexilInterface
  {
  public:
    PlexilInterface() : m_id(this) {}
    ~PlexilInterface();
    const PlexilInterfaceId& getId() const {return m_id;}
    const std::vector<PlexilVarRef*>& in() const {return m_in;}
    const std::vector<PlexilVarRef*>& inOut() const {return m_inOut;}
    const PlexilVarRef* findVar(const PlexilVarRef* target);
    const PlexilVarRef* findInVar(const PlexilVarRef* target);
    const PlexilVarRef* findInOutVar(const PlexilVarRef* target);
    const PlexilVarRef* findVar(const std::string& target);
    const PlexilVarRef* findInVar(const std::string& target);
    const PlexilVarRef* findInOutVar(const std::string& target);
        
    void addIn(PlexilVarRef* var) {m_in.push_back(var);}
    void addInOut(PlexilVarRef* var) {m_inOut.push_back(var);}

  private:
    PlexilInterfaceId m_id;
    std::vector<PlexilVarRef*> m_in;
    std::vector<PlexilVarRef*> m_inOut;
  };

  class PlexilExpr {
  public:
    PlexilExpr() : m_lineNo(0), m_colNo(0), m_id(this) {}

    virtual ~PlexilExpr() 
    {
      for (std::vector<PlexilExprId>::iterator it = m_subExprs.begin();
           it != m_subExprs.end();
           ++it)
        delete (PlexilExpr*) *it;
      m_subExprs.clear();
      m_id.remove();
    }
    const PlexilExprId& getId() const {return m_id;}
    const std::string& name() const {return m_name;}
    const std::vector<PlexilExprId>& subExprs() const {return m_subExprs;}
    int lineNo() const {return m_lineNo;}
    int colNo() const {return m_colNo;}

    void setName(const std::string& name);
    void addSubExpr(PlexilExprId expr) {m_subExprs.push_back(expr);}
    void setLineNo(int n) {m_lineNo = n;}
    void setColNo(int n) {m_colNo = n;}

  private:
    int m_lineNo;
    int m_colNo;
    PlexilExprId m_id;
    std::string m_name;
    std::vector<PlexilExprId> m_subExprs;
  };

  class PlexilVarRef : public PlexilExpr
  {
  public:
    PlexilVarRef() :
      PlexilExpr(),
      m_typed(false),
      m_type(PLEXIL::UNKNOWN_TYPE),
      m_defaultValue(PlexilExprId::noId())
    {}

    ~PlexilVarRef();

    bool typed() const {return m_typed;}
    const PlexilType& type() const {return m_type;}
    const PlexilExprId& defaultValue() const {return m_defaultValue;}
    const PlexilVarId& variable() const {return m_variable;}
         
    void setDefaultValue(const PlexilExprId& defaultValue)
    {
      m_defaultValue = defaultValue;
    }

    void setType(const PlexilType& type)
    {
      m_type = type; 
      m_typed = true;
    }
    void setVariable(const PlexilVarId& var);

  private:
    bool m_typed;
    PlexilType m_type;
    PlexilExprId m_defaultValue;
    PlexilVarId m_variable;
  };

  class PlexilOp : public PlexilExpr {
  public:
    PlexilOp() : PlexilExpr() {}
    const std::string& getOp() const {return m_op;}

    void setOp(const std::string& op) {m_op = op; setName(op);}
  private:
    std::string m_op;
  };

  class PlexilArrayElement : public PlexilExpr {
  public:
    PlexilArrayElement();

    const std::string& getArrayName() const
    {
      return m_arrayName;
    }

    void setArrayName(const std::string& name);

  private:
    std::string m_arrayName;
  };

  class PlexilState {
  public:
    PlexilState() : m_lineNo(0), m_colNo(0), m_id(this) {}

    ~PlexilState()
    {
      for (std::vector<PlexilExprId>::iterator it = m_args.begin();
           it != m_args.end();
           ++it)
        delete (PlexilExpr*) *it;
      m_args.clear();
      if (m_nameExpr.isId())
        delete (PlexilExpr*) m_nameExpr;
      m_id.remove();
    }
    const PlexilStateId& getId() const {return m_id;}
    const std::vector<PlexilExprId>& args() const {return m_args;}
    const std::string& name() const;
    const PlexilExprId& nameExpr() const {return m_nameExpr;}
    int lineNo() const {return m_lineNo;}
    int colNo() const {return m_colNo;}

    void addArg(const PlexilExprId& arg) {m_args.push_back(arg);}
    void setName(const std::string& name);
    void setNameExpr(const PlexilExprId& nameExpr) {m_nameExpr = nameExpr;}
    void setLineNo(int n) {m_lineNo = n;}
    void setColNo(int n) {m_colNo = n;}

  private:
    int m_lineNo;
    int m_colNo;
    PlexilStateId m_id;
    PlexilExprId m_nameExpr;
    std::vector<PlexilExprId> m_args;
  };

  class PlexilLookup : public PlexilExpr {
  public:
    PlexilLookup() : PlexilExpr() {}

    virtual ~PlexilLookup()
    {
      if (m_state.isId())
        delete (PlexilState*) m_state;
    }

    const PlexilStateId& state() const {return m_state;}
    void setState(const PlexilStateId& state) {m_state = state;}
  private:
    PlexilStateId m_state;
  };


  class PlexilLookupNow : public PlexilLookup {
  public:
    PlexilLookupNow() : PlexilLookup() {setName("LookupNow");}
  };

  class PlexilChangeLookup : public PlexilLookup {
  public:
    PlexilChangeLookup() : PlexilLookup() {setName("LookupOnChange");}

    ~PlexilChangeLookup()
    {
      for (std::vector<PlexilExprId>::iterator it = m_tolerances.begin();
           it != m_tolerances.end();
           ++it)
        delete (PlexilExpr*) *it;
      m_tolerances.clear();
    }

    const std::vector<PlexilExprId>& tolerances() const {return m_tolerances;}
    void addTolerance(const PlexilExprId& tolerance) {m_tolerances.push_back(tolerance);}
  private:
    std::vector<PlexilExprId> m_tolerances;
  };

  class PlexilValue : public PlexilExpr {
  public:
    PlexilValue(const PlexilType& type, const std::string& value = "UNKNOWN");
    const PlexilType& type() const {return m_type;}
    const std::string& value() const {return m_value;}
  private:
    PlexilType m_type;
    std::string m_value;
  };

  class PlexilArrayValue : public PlexilValue
  {
  public:
    PlexilArrayValue(const PlexilType& type, unsigned maxSize,
                     const std::vector<std::string>& values);
    ~PlexilArrayValue() {} 
    const std::vector<std::string>& values() const {return m_values;}
    unsigned maxSize() const {return m_maxSize;}
  private:
    unsigned m_maxSize;
    std::vector<std::string> m_values;
  };

  class PlexilVar : public PlexilExpr {
  public:
    PlexilVar(const std::string& name, const PlexilType& type);
    PlexilVar(const std::string& name, const PlexilType& type, const std::string& value);
    PlexilVar(const std::string& name, const PlexilType& type, PlexilValue* value);
    virtual ~PlexilVar();

    virtual bool isArray() const {return false;}

    const PlexilVarId& getId() const {return m_varId;}
    virtual const PlexilType& type() const {return m_type;}
    const PlexilValue* value() const {return m_value;}

  protected:
    PlexilType m_type;

  private:
    PlexilVarId m_varId;
    PlexilValue* m_value;
  };
  
  class PlexilArrayVar : public PlexilVar {
  public:
    PlexilArrayVar(const std::string& name, 
                   const PlexilType& type, 
                   const unsigned maxSize);
    PlexilArrayVar(const std::string& name, 
                   const PlexilType& type, 
                   const unsigned maxSize, 
                   std::vector<std::string>& values);
    ~PlexilArrayVar();

    // override PlexilVar method
     
    virtual const PlexilType& type() const
    { 
      static PlexilType s_arrayType = ARRAY;
      return s_arrayType;
    }
    virtual bool isArray() const {return true;}
    const PlexilType& elementType() const {return m_type;}
    virtual unsigned maxSize() const {return m_maxSize;}

  private:
    unsigned m_maxSize;
  };
  
  class PlexilNodeBody {
  public:
    PlexilNodeBody() : m_lineNo(0), m_colNo(0), m_id(this) {}
    virtual ~PlexilNodeBody() {m_id.remove();}
    const PlexilNodeBodyId& getId() const {return m_id;}
    int lineNo() const {return m_lineNo;}
    int colNo() const {return m_colNo;}
    void setLineNo(int n) {m_lineNo = n;}
    void setColNo(int n) {m_colNo = n;}

  private:
    int m_lineNo;
    int m_colNo;
    PlexilNodeBodyId m_id;
  };

  class PlexilActionBody : public PlexilNodeBody {
  public:
    PlexilActionBody() : PlexilNodeBody() {}
    virtual ~PlexilActionBody()
    {
      for (std::vector<PlexilExpr*>::iterator it = m_dest.begin();
           it != m_dest.end();
           ++it)
        delete *it;
      m_dest.clear();
    }
    const std::vector<PlexilExpr*>& dest() const {return m_dest;}

    void addDestVar(PlexilExpr* ref) {m_dest.push_back(ref);}
  private:
    std::vector<PlexilExpr*> m_dest;
  };

  //rhs may need to end up being a list!
  class PlexilAssignmentBody : public PlexilActionBody {
  public:
    PlexilAssignmentBody()
      : PlexilActionBody(),
        m_type(PLEXIL::UNKNOWN_TYPE)
    {}

    ~PlexilAssignmentBody()
    {
      if (m_rhs.isId())
        delete (PlexilExpr*) m_rhs;
    }

    const PlexilExprId& RHS() const {return m_rhs;}
    const PlexilType& type() const {return m_type;}

    void setRHS(const PlexilExprId& rhs) {m_rhs = rhs;}
    void setType(const PlexilType& type) {m_type = type;}
  private:
    PlexilExprId m_rhs;
    PlexilType m_type;
  };

  class PlexilCommandBody : public PlexilActionBody {
  public:
    PlexilCommandBody() : PlexilActionBody() {}

    ~PlexilCommandBody()
    {
      for (std::vector<PlexilResourceId>::iterator it = m_resource.begin();
           it != m_resource.end();
           ++it)
        delete (PlexilResource*) *it;
      m_resource.clear();
      if (m_state.isId())
        delete (PlexilState*) m_state;
    }

    const PlexilStateId& state() const {return m_state;}
    const std::vector<PlexilResourceId>& getResource() const {return m_resource;}

    void setState(const PlexilStateId& state) {m_state = state;}
    void setResource(const std::vector<PlexilResourceId>& resource) {m_resource=resource;}
  private:
    PlexilStateId m_state;
    std::vector<PlexilResourceId> m_resource;
  };

  class PlexilNodeRef {
  public:
    enum Direction {
      SELF = 0,
      PARENT,
      CHILD,
      SIBLING,
      // Used internally, not user-specified
      GRANDPARENT,
      UNCLE,
      NO_DIR
    };

    PlexilNodeRef(Direction dir)
      : m_id(this), m_name(), m_lineNo(0), m_colNo(0), m_generation(0), m_dir(dir)
    {}
    PlexilNodeRef(Direction dir, const char* name)
      : m_id(this), m_name(name), m_lineNo(0), m_colNo(0), m_generation(0), m_dir(dir)
    {}

    ~PlexilNodeRef() {m_id.remove();}

    const PlexilNodeRefId& getId() const {return m_id;}
    const Direction& dir() const {return m_dir;}
    const std::string& name() const {return m_name;}

    int generation() const { return m_generation; }
    void incrementGeneration() 
    {
      ++m_generation;
      switch (m_dir) {
        case SELF: m_dir = PARENT; break;
        case PARENT: m_dir = GRANDPARENT; break;
        case CHILD: m_dir = SIBLING; break;
        case SIBLING: m_dir = UNCLE; break;

        case GRANDPARENT:
        case UNCLE:
          break; // no change

        default: // includes NO_DIR
          assertTrueMsg(ALWAYS_FAIL, "PlexilNodeRef::incrementGeneration(): invalid direction");
        }
    }

    int lineNo() const {return m_lineNo;}
    int colNo() const {return m_colNo;}
    void setLineNo(int n) {m_lineNo = n;}
    void setColNo(int n) {m_colNo = n;}

  private:
    PlexilNodeRefId m_id;
    std::string m_name;
    int m_lineNo;
    int m_colNo;
    int m_generation;
    Direction m_dir;
  };

  class PlexilInternalVar : public PlexilVarRef {
  public:
    PlexilInternalVar() : PlexilVarRef() {}
 
    virtual ~PlexilInternalVar()
    {
      if (m_ref.isId())
        delete (PlexilNodeRef*) m_ref;
    }

    const PlexilNodeRefId& ref() const {return m_ref;}

    void setRef(const PlexilNodeRefId& ref) {m_ref = ref;}
  private:
    PlexilNodeRefId m_ref;
  };

  class PlexilOutcomeVar : public PlexilInternalVar {
  public:
    PlexilOutcomeVar() : PlexilInternalVar() {setName("outcome");}
  };

  class PlexilFailureVar : public PlexilInternalVar {
  public:
    PlexilFailureVar() : PlexilInternalVar() {setName("failure_type");}
  };

  class PlexilStateVar : public PlexilInternalVar {
  public:
    PlexilStateVar() : PlexilInternalVar() {setName("state");}
  };

  class PlexilCommandHandleVar : public PlexilInternalVar {
  public:
    PlexilCommandHandleVar() : PlexilInternalVar() {setName("command_handle");}
  };

  class PlexilTimepointVar : public PlexilInternalVar {
  public:
    PlexilTimepointVar() : PlexilInternalVar() {setName("@Timepoint");}
    const std::string& state() const {return m_state;}
    const std::string& timepoint() const {return m_timepoint;}

    void setState(const std::string& state) {m_state = state;}
    void setTimepoint(const std::string& timepoint) {m_timepoint = timepoint;}
  private:
    // *** Both of these should be represented by enums.
    std::string m_state;
    std::string m_timepoint;
  };

  class PlexilUpdate {
  public:
    PlexilUpdate() : m_lineNo(0), m_colNo(0), m_id(this) {}

    ~PlexilUpdate()
    {
      for (std::vector<std::pair<std::string, PlexilExprId> >::iterator it = m_map.begin();
           it != m_map.end();
           ++it)
        delete (PlexilExpr*) it->second;
      m_map.clear();
      m_id.remove();
    }

    const PlexilUpdateId& getId() const {return m_id;}
    const std::vector<std::pair<std::string, PlexilExprId> >& pairs() const {return m_map;}

    void addPair(const std::string& name, const PlexilExprId& value)
    { m_map.push_back(std::make_pair(name, value));}
    int lineNo() const {return m_lineNo;}
    int colNo() const {return m_colNo;}
    void setLineNo(int n) {m_lineNo = n;}
    void setColNo(int n) {m_colNo = n;}
  private:
    int m_lineNo;
    int m_colNo;
    PlexilUpdateId m_id;
    std::vector<std::pair<std::string, PlexilExprId> > m_map;
  };


  class PlexilUpdateBody : public PlexilNodeBody {
  public:
    PlexilUpdateBody() : PlexilNodeBody() {}

    ~PlexilUpdateBody()
    {
      if (m_update.isId())
        delete (PlexilUpdate*) m_update;
    }

    const PlexilUpdateId& update() const {return m_update;}

    void setUpdate(const PlexilUpdateId& update) {m_update = update;}
  private:
    PlexilUpdateId m_update;
  };

  class PlexilListBody : public PlexilNodeBody 
  {
  public:
    PlexilListBody() : PlexilNodeBody() {}

    ~PlexilListBody()
    {
      for (std::vector<PlexilNodeId>::iterator it = m_children.begin();
           it != m_children.end();
           ++it)
        delete (PlexilNode*) *it;
      m_children.clear();
    }

    void addChild(const PlexilNodeId& child) {m_children.push_back(child);}
    const std::vector<PlexilNodeId>& children() const {return m_children;}
  private:
    std::vector<PlexilNodeId> m_children;
  };
  // library node call body

  class PlexilLibNodeCallBody : public PlexilNodeBody 
  {
  public:

    // construct a library node call body with the name of the
    // library node
        
    PlexilLibNodeCallBody(const std::string& libNodeName) : PlexilNodeBody() 
    {
      setLibNodeName(libNodeName);
    }

    ~PlexilLibNodeCallBody()
    {
      for (PlexilAliasMap::iterator it = m_aliases.begin();
           it != m_aliases.end();
           it = m_aliases.begin()) {
        delete (PlexilExpr*) it->second;
        m_aliases.erase(it);
      }
      if (m_libNode.isId())
        delete (PlexilNode*) m_libNode;
    }

    // getter for library node name

    const std::string& libNodeName() const {return m_libNodeName;}

    // getter for library node

    const PlexilNodeId& libNode() const {return m_libNode;}

    // getter for all aliases
        
    const PlexilAliasMap& aliases() const
    {
      return m_aliases;
    }
    // add an alias pair to the library call

    void addAlias(const std::string& param, PlexilExprId value)
    {
      PlexilExprId &alias = m_aliases[LabelStr(param)];
      checkError(!alias.isId(), "Alias '" << param
                 << "' apears more then once in call to "
                 << m_libNodeName);
      alias = value;
    }
    // setter for library node name

    void setLibNodeName(const std::string& libNodeName) 
    {
      m_libNodeName = libNodeName;
    }
    // setter for the called library node

    void setLibNode(const PlexilNodeId& libNode) {m_libNode = libNode;}

  private:
    std::string m_libNodeName;
    PlexilNodeId m_libNode;
    PlexilAliasMap m_aliases;
  };
}
#endif
