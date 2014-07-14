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

#ifndef _H_PlexilPlan
#define _H_PlexilPlan

#include "ConstantMacros.hh"
#include "ExecDefs.hh"
#include "PlexilExpr.hh"
#include "PlexilLookup.hh"
#include "PlexilNodeType.hh"
#include "PlexilResource.hh"
#include "PlexilUpdate.hh"
#include "ValueType.hh"

#include <vector>
#include <map>
#include <string>

// Take care of annoying VxWorks macro
#undef UPDATE

#define isType(p,type) (dynamic_cast<type*>(p) != NULL)

namespace PLEXIL 
{

  class PlexilInterface;
  class PlexilNodeBody;
  class PlexilNode;
  class PlexilUpdate;
  class PlexilNodeRef;
  class PlexilInternalVar;

  DECLARE_ID(PlexilNode);
  DECLARE_ID(PlexilInterface);
  DECLARE_ID(PlexilNodeBody);
  DECLARE_ID(PlexilUpdate);
  DECLARE_ID(PlexilNodeRef);
  DECLARE_ID(PlexilInternalVar);

  typedef std::vector<const PlexilNode*> PlexilNodeSet;
  typedef std::map<std::string, PlexilExprId>  PlexilAliasMap;

  class PlexilParser 
  {
  public:
    //node types
    // see src/expr/PlexilNodeType.{hh,cc}

    // value types
    // see src/expr/ValueType.hh

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

    static ValueType parseValueTypePrefix(const std::string & str, 
                                           std::string::size_type prefixLen);
    inline static ValueType parseValueType(const std::string& typeStr)
    {
      return parseValueTypePrefix(typeStr, typeStr.length());
    }

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
    double priority() const {return m_priority;}
    const PlexilInterfaceId& interface() const {return m_intf;}
    const std::vector<PlexilVar *>& declarations() const {return m_declarations;}
    const std::vector<std::pair <PlexilExprId, std::string> >& conditions() const {return m_conditions;}
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
    void addVariable(PlexilVar *var) {m_declarations.push_back(var);}
    void addCondition(const std::string& name, const PlexilExprId& expr)
    {m_conditions.push_back(std::make_pair(expr, name));}
    void setBody(const PlexilNodeBodyId& body) {m_nodeBody = body;}
    void setInterface(const PlexilInterfaceId& intf) { m_intf = intf;}

    /**
     * @brief Recurse into given node and link library calls found therein.
     */
    bool link(const std::map<std::string, PlexilNodeId>& libraries);

  private:

    // Internal methods
    bool link(const std::map<std::string, PlexilNodeId>& libraries, PlexilNodeSet& seen);

    // 8 byte alignment on 32 bit, 16 on 64
    PlexilNodeId m_id;
    PlexilInterfaceId m_intf;
    PlexilNodeBodyId m_nodeBody;

    // 8 byte alignment
    double m_priority;

    // 4 byte alignment on 32 bit, 8 on 64
    std::string m_fileName;
    std::string m_nodeId;
    std::vector<PlexilVar *> m_declarations;
    std::vector<std::pair<PlexilExprId, std::string> > m_conditions;

    // 4 byte alignment on 32 and 64 (?)
    int m_lineNo;
    int m_colNo;
    PlexilNodeType m_nodeType;
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
  
  class PlexilNodeBody {
  public:
    PlexilNodeBody() : m_id(this), m_lineNo(0), m_colNo(0) {}
    virtual ~PlexilNodeBody() {m_id.remove();}
    const PlexilNodeBodyId& getId() const {return m_id;}
    int lineNo() const {return m_lineNo;}
    int colNo() const {return m_colNo;}
    void setLineNo(int n) {m_lineNo = n;}
    void setColNo(int n) {m_colNo = n;}

  private:
    PlexilNodeBodyId m_id;
    int m_lineNo;
    int m_colNo;
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
    ValueType type() const {return m_type;}

    void setRHS(const PlexilExprId& rhs) {m_rhs = rhs;}
    void setType(ValueType type) {m_type = type;}
  private:
    PlexilExprId m_rhs;
    ValueType m_type;
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

    // N.B. Only caller is PlexilXmlParser::getNodeRefInternal.
    void setGeneration(int gen)
    {
      m_generation = gen;
      if (gen == 0)
        return;

      switch (m_dir) {
        case SELF:
          if (gen == 1)
            m_dir = PARENT;
          else
            m_dir = GRANDPARENT;
          break;

        case PARENT: m_dir = GRANDPARENT; break;

        case CHILD:
          if (gen == 1)
            m_dir = SIBLING;
          else 
            m_dir = UNCLE;
          break;

        case SIBLING: m_dir = UNCLE; break;

        default: // includes NO_DIR, GRANDPARENT, UNCLE
          assertTrueMsg(ALWAYS_FAIL, "PlexilNodeRef::setGeneration(): invalid direction");
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

  class PlexilInternalVar : public PlexilVarRef 
  {
  public:
    PlexilInternalVar(std::string const &varName, ValueType type, PlexilNodeRefId const &ref)
      : PlexilVarRef(varName, type),
        m_ref(ref)
    {
    }
 
    virtual ~PlexilInternalVar()
    {
      if (m_ref.isId())
        delete (PlexilNodeRef*) m_ref;
    }

    const PlexilNodeRefId& ref() const {return m_ref;}

  private:
    PlexilNodeRefId m_ref;
  };

  class PlexilOutcomeVar : public PlexilInternalVar 
  {
  public:
    PlexilOutcomeVar(PlexilNodeRefId const &ref)
    : PlexilInternalVar("outcome", OUTCOME_TYPE, ref)
    {
      setName("NodeOutcomeVariable");
    }
  };

  class PlexilFailureVar : public PlexilInternalVar
  {
  public:
    PlexilFailureVar(PlexilNodeRefId const &ref)
    : PlexilInternalVar("failure_type", FAILURE_TYPE, ref)
    {
      setName("NodeFailureVariable");
    }
  };

  class PlexilStateVar : public PlexilInternalVar 
  {
  public:
    PlexilStateVar(PlexilNodeRefId const &ref)
    : PlexilInternalVar("state", NODE_STATE_TYPE, ref)
    {
      setName("NodeStateVariable");
    }
  };

  class PlexilCommandHandleVar : public PlexilInternalVar {
  public:
    PlexilCommandHandleVar(PlexilNodeRefId const &ref) 
    : PlexilInternalVar("command_handle", COMMAND_HANDLE_TYPE, ref)
    {
      setName("NodeCommandHandleVariable");
    }
  };

  class PlexilTimepointVar : public PlexilInternalVar {
  public:
    PlexilTimepointVar(PlexilNodeRefId const &ref, std::string const &state, std::string const &timept)
      : PlexilInternalVar("@Timepoint", DATE_TYPE, ref),
        m_state(state),
        m_timepoint(timept)
    {
      setName("NodeTimepointValue");
    }
    const std::string& state() const {return m_state;}
    const std::string& timepoint() const {return m_timepoint;}

  private:
    // FIXME: State should be represented by an enum.
    // Timepoint could be represented by a bool.
    std::string m_state;
    std::string m_timepoint;
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
      PlexilExprId &alias = m_aliases[param];
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
