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

package gov.nasa.luv;

import java.io.File;
import java.io.InterruptedIOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import java.util.Set;
import java.util.LinkedHashSet;
import java.util.Properties;
import java.util.Map.Entry;
import java.util.Stack;
import static gov.nasa.luv.Constants.*;

/**
 * The Model class represents a model of a Plexil Plan.
 */

public class Model extends Properties
{
    private static Model TheRootModel;
    private static boolean found;   
    private Model foundChild;    
    private int row_number;    
    private String type;
    private String modelName; 
    private String planName;    
    private String scriptName;
    private String libraryName;
    private boolean unresolvedLibraryCall;               
    private LinkedHashSet<String> libraryFiles;
    private LinkedHashSet<String> missingLibraryNodes;
    private HashMap<Integer, ArrayList> conditionMap;    
    private ArrayList<Variable> variableList;    
    private ArrayList<String> actionList;
    private Model parent;
    private Vector<Model> children;
    private Vector<ChangeListener> changeListeners;
    
    /**
     * Constructs a Plexil Model with the specified type (usually the XML tag) 
     * which identifies what kind of thing this model represents.  All other 
     * features of the model are stored in properties and children.
     * 
     * @param type the type of model or node this Model is
     */
    public Model(String type)
    {
        this(type, -1);
    }
    
    /**
     * Constructs a Plexil Model with the specified type (usually the XML tag) 
     * which identifies what kind of thing this model represents and the row
     * number this Model lies on in the Luv application viewer. All other 
     * features of the model are stored in properties and children.
     * 
     * @param type the type of model or node this Model is
     * @param row the row number this Model is on in the viewer
     */
    public Model(String type, int row)
    {
        this.type = type;
        this.row_number = row;
        
        init();
    }

    /**
     * Copy Constructor
     * @param node
     */
    private Model(Model node)
    {
        init();

        this.addPlanName(node.getAbsolutePlanName());
        this.addScriptName(node.getAbsoluteScriptName());

        for (String action : node.getActionList())
            this.actionList.add(action);

        for (Model child : node.getChildren())
            this.children.add(new Model(child));

        for (int condition_type = 0; condition_type < node.getConditionMap().size(); condition_type++) {
            for (int condition_expr = 0; node.getConditionMap().get(condition_type) != null && condition_expr < node.getConditionMap().get(condition_type).size(); condition_expr++)
                this.addConditionInfo(condition_type, (String)node.getConditionMap().get(condition_type).get(condition_expr));
        }

        this.setLibraryName(node.getLibraryName());

        for (String library : node.getLibraryNames())
            this.libraryFiles.add(library);

        for (String library : node.getMissingLibraries())
            this.missingLibraryNodes.add(library);

        this.setModelName(node.getModelName());
        this.setParent(node.getParent());
        this.setRowNumber(node.getRowNumber());
        this.type = node.getType();
        this.setUnresolvedLibraryCall(node.getUnresolvedLibraryCall());

        for (Variable v : node.getVariableList()) {
            this.variableList.add(v.clone());
        }

        modelName = node.getProperty(NODE_ID);
        setProperty(NODETYPE_ATTR, node.getProperty(NODETYPE_ATTR));
        setProperty(NODETYPE_ATTR_PLX, node.getProperty(NODETYPE_ATTR_PLX));
        setProperty(NODE_ID, node.getProperty(NODE_ID));
        setProperty(MODEL_TYPE, node.getProperty(MODEL_TYPE));
        setProperty(MODEL_OUTCOME, node.getProperty(MODEL_OUTCOME));
        setProperty(MODEL_STATE, node.getProperty(MODEL_STATE));
        setProperty(SKIP_CONDITION, node.getProperty(SKIP_CONDITION));
        setProperty(START_CONDITION, node.getProperty(START_CONDITION));
        setProperty(END_CONDITION, node.getProperty(END_CONDITION));
        setProperty(INVARIANT_CONDITION, node.getProperty(INVARIANT_CONDITION));
        setProperty(PRE_CONDITION, node.getProperty(PRE_CONDITION));
        setProperty(POST_CONDITION, node.getProperty(POST_CONDITION));
        setProperty(REPEAT_CONDITION, node.getProperty(REPEAT_CONDITION));
        setProperty(ANCESTOR_INVARIANT_CONDITION, node.getProperty(ANCESTOR_INVARIANT_CONDITION));
        setProperty(ANCESTOR_END_CONDITION, node.getProperty(ANCESTOR_END_CONDITION));
        setProperty(PARENT_EXECUTING_CONDITION, node.getProperty(PARENT_EXECUTING_CONDITION));
        setProperty(PARENT_FINISHED_CONDITION, node.getProperty(PARENT_FINISHED_CONDITION));
        setProperty(CHILDREN_WAITING_OR_FINISHED, node.getProperty(CHILDREN_WAITING_OR_FINISHED));
        setProperty(ABORT_COMPLETE, node.getProperty(ABORT_COMPLETE));
        setProperty(PARENT_WAITING_CONDITION, node.getProperty(PARENT_WAITING_CONDITION));
        setProperty(COMMAND_HANDLE_RECEIVED_CONDITION, node.getProperty(COMMAND_HANDLE_RECEIVED_CONDITION));
    }
    
    private void init()
    {
        modelName = UNKNOWN;
        planName = UNKNOWN;
        scriptName = UNKNOWN;
        libraryName = null;
        unresolvedLibraryCall = false;
        foundChild = null;
        libraryFiles = new LinkedHashSet<String>();
        missingLibraryNodes = new LinkedHashSet<String>();
        changeListeners = new Vector<ChangeListener>();
        children = new Vector<Model>();
        conditionMap = new HashMap<Integer, ArrayList>();
        variableList = new ArrayList<Variable>();
        actionList = new ArrayList<String>();
    }
    
    /** Returns the type of this Plexil Model/node.
     *  @return the type of this Plexil Model/node */
    public String                       getType()                   { return type; }
    /** Returns the row number of this Plexil Model/node.
     *  @return the row number of this Plexil Model/node */
    public int                          getRowNumber()              { return row_number; }
    /** Returns the name of this Plexil Model/node.
     *  @return the name of this Plexil Model/node */
    public String                       getModelName()              { return modelName; }
    /** Returns the Model parent of this Plexil Model/node.
     *  @return the Model parent of this Plexil Model/node */
    public Model                        getParent()                 { return parent; }    
    /** Returns the Vector of Model children of this Plexil Model/node.
     *  @return the Vector of Model children of this Plexil Model/node */
    public Vector<Model>                getChildren()               { return children; }
    /** Returns the full path of the Plexil Plan of this Plexil Model/node.
     *  @return the full path of the Plexil Plan of this Plexil Model/node */
    public String                       getAbsolutePlanName()       { return planName; }
    /** Returns the full path of the Plexil Script of this Plexil Model/node.
     *  @return the full path of the Plexil Script of this Plexil Model/node */
    public String                       getAbsoluteScriptName()     { return scriptName; }
    /** Returns the HashMap of Conditions for this Plexil Model/node.
     *  @return the HashMap of Conditions for this Plexil Model/node */
    public HashMap<Integer, ArrayList>  getConditionMap()           { return conditionMap; }
    /** Returns the ArrayList of local variables for this Plexil Model/node.
     *  @return the ArrayList of local variables for this Plexil Model/node */
    public ArrayList<Variable>     getVariableList()           { return variableList; }
    /** Returns the ArrayList of actions for this Plexil Model/node.
     *  @return the ArrayList of actions for this Plexil Model/node */
    public ArrayList<String>            getActionList()             { return actionList; }
    /** Returns whether this Plexil Model/node has an unresolved library call.
     *  @return whether this Plexil Model/node has an unresolved library call */
    public boolean                      getUnresolvedLibraryCall()  { return unresolvedLibraryCall; }
    /** Returns the Library Name for this Plexil Model/node.
     *  @return the Library Name for this Plexil Model/node */
    public String                       getLibraryName()            { return libraryName; }
    /** Returns the Set of Library Names for this Plexil Model/node.
     *  @return the Set of Library Names for this Plexil Model/node */
    public Set<String>                  getLibraryNames()           { return libraryFiles; }
    /** Returns the Set of missing Libraries for this Plexil Model/node.
     *  @return the Set of missing Libraries for this Plexil Model/node */
    public Set<String>                  getMissingLibraries()       { return missingLibraryNodes; }
    
    /**
     * Returns the specified Model child for this Model.
     * @param i the index of the child for this Model
     * @return the Model child of this model
     */
    public Model getChild(int i)
    {
	return children.get(i);
    }
    
    /**
     * Returns the Plexil Plan name without the path.
     * @return the Plexil Plan name
     */
    public String getPlanName()
    {
        if (!planName.equals(UNKNOWN))
            return planName.substring(planName.lastIndexOf("/") + 1, 
                   planName.length());
        else
            return planName;
    }
    
    /**
     * Returns the Plexil Script name without the path.
     * @return the Plexil Script name
     */
    public String getScriptName()
    {
        if (!scriptName.equals(UNKNOWN))
            return scriptName.substring(scriptName.lastIndexOf("/") + 1, 
                   scriptName.length());
        else
            return scriptName;
    }
    
    /**
     * Returns the top node or root of this Plexil Model.
     * @return the root Model
     */
    public static Model getRoot()
    {
	if (TheRootModel == null) 
        {
	    TheRootModel = new Model("dummy");
	    TheRootModel.setModelName("_The_Root_Model_");
	}
        
	return TheRootModel;
    }
    
    /**
     * Returns whether or not this Model has local variables.
     * @return whether or not this Model has local variables
     */
    public boolean hasVariables()
    {
        return !variableList.isEmpty();
    }
    
    /**
     * Returns whether or not this Model has actions.
     * @return whether or not this Model has actions
     */
    public boolean hasAction()
    {
        return !actionList.isEmpty();
    }
    
    /**
     * Returns whether or not this Model has conditions.
     * @return whether or not this Model has conditions
     */
    public boolean hasConditions()
    {
        return !conditionMap.isEmpty();
    }
    
    /**
     * Returns whether or not this Model has the specified condition.
     * @return whether or not this Model has the specified condition
     */
    public boolean hasCondition(String condition)
    {
        return conditionMap.containsKey(getConditionNum(condition));
    }
    
    /**
     * Returns whether or not this Model is the root Model.
     * @return whether or not this Model is the root Model
     */
    public boolean isRoot()
    {
	return this == getRoot(); 
    }
    
    /**
     * Returns whether or not this Model's type is a Node.
     * @return whether or not this Model's type is a Node
     */
    public boolean isNode()
    {
	return type.equals(NODE);
    }

    /**
     * Sets specified the row number for this Model.
     * @param row the row number for this Model
     */
    public void setRowNumber(int row)
    {
        row_number = row;      
    }

    /**
     * Sets the specified name for this Model.
     * @param name the name for this Model
     */
    public void setModelName(String name)
    {
	modelName = name;
    }
    
    /**
     * Sets the specified Model as this Model's parent.
     * @param newParent the parent Model of this Model
     */
    public void setParent(Model newParent)
    {
	parent = newParent;
    }
    
    /**
     * Sets whether this Model has an unresolved library call with this specified boolean value.
     * @param val the  boolean value to set whether or not this Model has an unresolved library call
     */
    public void setUnresolvedLibraryCall(boolean val)
    {
	unresolvedLibraryCall = val;
    }
    
    /**
     * Sets the specified string for the library name of this Model.
     * @param libname the library name for this Model
     */
    public void setLibraryName(String libname)
    {
	libraryName = libname;
    }
    
    /**
     * Sets the specified property value with key to this Model.
     * @param key the key to the specified value
     * @param value the value of a property for this Model
     * @return an object result
     */
    public Object setProperty(String key, String value)
    {
	if (value == null)
        {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, 
                    "ERROR: error occured while setting model property " +
                    "(key: " + key + ", type: " + type + ", value: NULL)");        
        }

	Object result = super.setProperty(key, value);

	if (value != result && value.equals(result) == false) {
            for (ChangeListener cl: changeListeners)
		cl.propertyChange(this, key);
	}
         
	return result;
    }


	public void setVariable(String vName, String value) {
		boolean found = false;
		if (vName != null) {
			Model walker = this;
			if(getVariableList().isEmpty())
				while(!found && (walker = walker.parent) != null)
					if(!walker.getVariableList().isEmpty())
						for(Variable v : walker.getVariableList())
							if(ArrayVariable.getBaseName(vName).equals(ArrayVariable.getBaseName(v.getName())))
							{
								found = true;
								break;								
							}
			Variable var = null;
			if(walker != null)
			for (Variable v : walker.getVariableList()) {
				//needs some other logic if there is dynamic support for assigning a set of values to array
				if (v instanceof ArrayVariable && vName.contains(((ArrayVariable)v).getBaseName())){
					((ArrayVariable) v).setArrayIndexVariable(vName, value);					
					var = v;
					break;
				}
				else if (vName.equals(v.getName())) {
					var = v;
					break;
				}				
			}
			if (var != null) {
				if(!(var instanceof ArrayVariable))
					var.setValue(value);
				System.out.println("Assigned " + value + " to variable " + vName);
				for (ChangeListener cl : walker.changeListeners)
					cl.variableAssigned(this, vName);
			}
		}
	}
    
    /**
     * Resets the flag indicating that the child of this Model was either found or not.
     */
    public void resetFoundChildFlag()
    {
        found = false;
    }
    
    /**
     * Adds the specified condition and condition equation to the ArrayList of conditions for this Model.
     * @param condition the condition type to add
     * @param conditionEquation the equation of the condition to add
     */
    public void addConditionInfo(int condition, String conditionEquation)
    { 
        ArrayList<String> equationHolder = 
                ConditionsTab.formatCondition(conditionEquation);

        conditionMap.put(condition, equationHolder);
    }
    
    /**
     * Adds the specified local variable Stack to the list of variables for this Model. Converts into a Variable first
     * @param variable the local variable to add to this Model's Stack of local variables
     */
    public void addVariableInfo(Stack variable)
    { 

        String value;
        String type;
        String name;
        String in_inout;
            
        if (variable.size() == 4)
        {
            value = (String) variable.pop();
            type = (String) variable.pop();
            name = (String) variable.pop();
            in_inout = (String) variable.pop(); 
        }
        else
        {
            value = UNKNOWN;
            type = UNKNOWN;
            name = UNKNOWN;
            in_inout = UNKNOWN;
        }
        
        if(!type.equals("UNKNOWN") && name.matches(".*\\[[0-9]*\\]"))
        {   
        	ArrayVariable m_array = new ArrayVariable("--", name, type, value); 
        	variableList.add(m_array);
        }
        else
        	variableList.add(new Variable(in_inout, name, type, value));
    }
    
    /**
     * Adds the given variable to the list of variables for this Model.
     * @param v The local variable to add.
     */
    public void addVariableInfo(Variable v) {
    	variableList.add(v);
    }
    
    /**
     * Adds the specified action to the ArrayList of actions for this Model.
     * @param action  the action to add
     */
    public void addActionInfo(String action)
    { 
        String formattedAction = ActionTab.formatAction(action);
  
        actionList.add(formattedAction);
    }

    /**
     * Specifies the Plexil Plan name for this Model.
     * @param planName the Plexil Plan name
     */
    public void addPlanName(String planName)
    {
        this.planName = planName;
	for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).planNameAdded(this, planName);
    }
      
    /**
     * Specifies the Plexil Script name for this Model.
     * @param scriptName the Plexil Plan name
     */
    public void addScriptName(String scriptName)
    {
        this.scriptName = scriptName;
	for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).scriptNameAdded(this, scriptName);
    }

    /**
     * Specifies the Plexil Library name for this Model.
     * @param libraryName the Plexil Library name
     */
    public void addLibraryName(String libraryName)
    {
	if (libraryFiles.add(libraryName))
        {
            for (int i = 0; i < changeListeners.size(); ++i)
                changeListeners.get(i).libraryNameAdded(this, libraryName);
        }
    }

    /**
     * Notifies the top level node that a library was not found and adds the
     * library to the list of missing libraries.
     * 
     * @param nodeName the name of the missing library
     */
    public void addMissingLibrary(String nodeName)
    {
	missingLibraryNodes.add(nodeName);
    }

    /**
     * Notifies the top level node that a library was found and removes the
     * library from the list of missing libraries.
     * 
     * @param nodeName the name of the found library
     */
    private boolean missingLibraryFound(String nodeName) 
    {
	return missingLibraryNodes.remove(nodeName);
    }
    
    /**
     * Creates and adds the specified Model child to this Model.
     * @param child the model child
     */
    public void addChild(Model child)
    {
	assert child.isNode();
    
	children.add(child);
	child.setParent(this);
    }
    
    /**
     * Removes the specified Model child from this Model.
     * @param child the Model child
     * @return whether or the child was removed
     */
    public boolean removeChild(Model child)
    {
	// Can't use Vector.remove(Object) because it uses equals(),
	// and two models are often equal without being identical!
	boolean removed = false;
	for (Iterator<Model> it = children.iterator(); it.hasNext(); ) {
	    if (it.next() == child) {
		it.remove();
		child.parent = null;
		removed = true;
		break;
	    }
	}
	return removed;
    }
    
    /**
     * Clears this Model of all children and properties.
     */
    public void removeChildren()
    {
	for (Model child: children)
            child.parent = null;
	children.clear();
    }
    
    /**
     * Returns the top level ancestor of this Model. 
     * @return the top level ancestor
     */
    public Model topLevelNode()
    {
	if (this == getRoot())
	    return null;
	if (parent == getRoot())
	    return this;
	return parent.topLevelNode(); 
    }

    /**
     * Returns the Node pathway starting from the specified Model to the root Model.
     * @param node the model to return the pathway for
     * @return the pathway
     */
    public Stack<String> pathToNode(Model node)
    {
        Stack<String> node_path = new Stack<String>();
        while (!node.isRoot())
        {
            if (!AbstractModelFilter.isModelFiltered(node))
            {
                node_path.push(node.getModelName());
            }
            
            node = node.getParent();
        }
        return node_path;
    }
    
    /**
     * Compares this Model with the specified Model to see if they are the same.
     * @param other the model to compare with
     * @return whether or not these two Models are equivalent
     */
    public boolean equivalent(Model other)
    {
	if (other == null)
	    return false;
	if (other == this)
	    return true;
	if (!modelName.equals(other.modelName))
	    return false;
	if (!type.equals(other.type))
	    return false;  
	if (!childrenEquivalent(other))
	    return false;        
        if (libraryFiles.size() != other.libraryFiles.size())
            return false;
        else
        {
            Iterator itr1 = libraryFiles.iterator();
            Iterator itr2 = other.libraryFiles.iterator();
            
            while(itr1.hasNext() && itr2.hasNext())
            {
                if (!itr1.next().equals(itr2.next()))
                    return false;
            }
        }
        
	return true;
    }

    /**
     * Compares this Model's children with the specified Model's children to see if they are the same.
     * @param other the model to compare with
     * @return whether or not these two Model's children are equivalent
     */
    private boolean childrenEquivalent(Model other)
    {
	if (children.isEmpty()) 
        {
	    if (other.children.isEmpty())
		return true;
	    else
		return false;
	}
	else if (other.children.isEmpty())
	    return false;
	else if (children.size() != other.children.size())
	    return false;
	for (int i = 0; i < children.size(); i++) 
        {
	    if (!children.get(i).equivalent(other.children.get(i)))
		return false;
	}
        
	return true;
    }

    /**
     * Signals that a new Plexil Plan has been installed under this Model.
     */
    public void planChanged()
    {
	for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).planChanged(this);
    }
    
    /**
     * Returns the Model that matches with the specified name.
     * @param name the name to match the Model to
     * @return the matching Model
     */
    public Model findChildByName(String name)
    {
	for (Model child : children)
	    if (child.modelName.equals(name))
		return child;
	return null;
    }
    
    /**
     * Returns the Model that matches with the specified row number.
     * @param row the row number to match the Model to
     * @return the matching Model
     */
    public Model findChildByRowNumber(int row)
    {
        for (Model child : children)
        {
            if (!found)
            {
                if (child.row_number == row)
                {
                    foundChild = child;
                    found = true;
                }
                else
                    foundChild = child.findChildByRowNumber(row);
            }
        }        
        
	return foundChild;
    }

    /**
     * Locates the Library node with the specified name based on whether the user
     * allows it.
     * @param name the name of the missing library
     * @param askUser flags whether or not the user wants Luv to search for the library
     * @return the located Library
     * @throws java.io.InterruptedIOException
     */
    public Model findLibraryNode(String name, boolean askUser) 
            throws InterruptedIOException
    {
	Model result = TheRootModel.findChildByName(name);

	if (result == null || result.getAbsolutePlanName().equals(UNKNOWN)) {
	    if (askUser) {
		// ask user for a file of missing library, and try to load it
		File library = null;
		try {
		    library = Luv.getLuv().getFileHandler().searchForLibrary(name);
		}
		catch (InterruptedIOException e) {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, 
                            "ERROR: exception occurred while finding library node");
		}

		if (library != null) {
		    result = TheRootModel.findChildByName(name);
		}
	    }
	}
        
	return result;
    }

    /**
     * Links the specified library into this Model and makes the appropriate 
     * annotations in the top level Model.
     * @param library the library for this Model
     * @return whether or not the library was linked
     */
    public boolean linkLibrary(Model library)
    {
        if (type.equals(NODE) &&
            getProperty(NODETYPE_ATTR).equals(LIBRARYNODECALL)&&
            libraryName.equals(library.modelName)) {
            addChild(library);
            unresolvedLibraryCall = false;
            Model topLevelNode = topLevelNode();
            if (topLevelNode != null) {
                topLevelNode.addLibraryName(library.planName);
                missingLibraryFound(library.modelName);
                return true;
            }
        }
        
        return false;
    }
    
    /** {@inheritDoc} */
    public String toString()
    {
	String name = getModelName();
	if (name == null)
            name = type;
	StringBuffer s = new StringBuffer(name);
         
	s.append("(");
	for (Entry<Object, Object> property: entrySet())
            s.append(" " + property.getKey() + " = " + property.getValue());
	s.append(")");
         
	if (children.size() > 0)
        {
            s.append("[");
            for (Model child: children)
                {
                    s.append(child.toString());
                    if (child != children.lastElement())
                        s.append(", ");
                }
            s.append("]");
        }
        
	return s.toString();
    }   
    
    /**
     * Resets all the properties of this Model and the Model's children properties 
     * to the beginning values (pre-execution).
     */
    public void resetMainAttributesOfAllNodes()
    {
        this.setMainAttributesOfNode();
        
        for (Model child : this.getChildren())
        {
            child.resetMainAttributesOfAllNodes();
        }
    }
         
    /**
     * Resets all the properties of this Model to the beginning values (pre-execution).
     */
    public void setMainAttributesOfNode()
    {
        String rawType = getProperty(NODETYPE_ATTR);
        String polishedtype = rawType != null ? NODE_TYPES.get(rawType) : null;
        if (polishedtype == null)
            polishedtype = rawType;

        modelName = getProperty(NODE_ID);

        setProperty(MODEL_TYPE, polishedtype);
        setProperty(MODEL_OUTCOME, UNKNOWN);
        setProperty(MODEL_STATE, INACTIVE);        
        setProperty(SKIP_CONDITION, UNKNOWN);                     
        setProperty(START_CONDITION, UNKNOWN);               
        setProperty(END_CONDITION, UNKNOWN); 
        setProperty(INVARIANT_CONDITION, UNKNOWN); 
        setProperty(PRE_CONDITION, UNKNOWN); 
        setProperty(POST_CONDITION, UNKNOWN); 
        setProperty(REPEAT_CONDITION, UNKNOWN); 
        setProperty(ANCESTOR_INVARIANT_CONDITION, UNKNOWN); 
        setProperty(ANCESTOR_END_CONDITION, UNKNOWN);        
        setProperty(PARENT_EXECUTING_CONDITION, UNKNOWN);    
        setProperty(PARENT_FINISHED_CONDITION, UNKNOWN);    
        setProperty(CHILDREN_WAITING_OR_FINISHED, UNKNOWN); 
        setProperty(ABORT_COMPLETE, UNKNOWN);           
        setProperty(PARENT_WAITING_CONDITION, UNKNOWN);         
        setProperty(COMMAND_HANDLE_RECEIVED_CONDITION, UNKNOWN); 
    }
            
    /**
     * Adds a property change listener to this Model. 
     * @param listener the property change listener
     */
    public void addChangeListener(ChangeListener listener)
    {
	changeListeners.add(listener);
    }

    /**
     * Removes a property change listener to this Model. 
     * @param listener the property change listener
     */
    public void removeChangeListener(ChangeListener listener)
    {
	changeListeners.remove(listener);
    }

    /**
     * The ChangeListener is an abstract class that is signaled when the Plexil Model 
     * is changed in some way.
     */
    public abstract static class ChangeListener
    {
	abstract public void propertyChange(Model model, String property);

	abstract public void planCleared(Model model);

	abstract public void planChanged(Model model);

	abstract public void planNameAdded(Model model, String planName);
            
	abstract public void scriptNameAdded(Model model, String scriptName);

	abstract public void libraryNameAdded(Model model, String libraryName);
	
	abstract public void variableAssigned(Model model, String variableName);
    }

    /**
     * The ChangeAdapter class is signaled when the Plexil Model is changed in some way.
     */
    public static class ChangeAdapter extends ChangeListener
    {
	public void propertyChange(Model model, String property){}

	public void planCleared(Model model){}

	public void planChanged(Model model){}

	public void planNameAdded(Model model, String planName){}
            
	public void scriptNameAdded(Model model, String scriptName){}

	public void libraryNameAdded(Model model, String libraryName){}
	
	public void variableAssigned(Model model, String variableName){}
    }
}
