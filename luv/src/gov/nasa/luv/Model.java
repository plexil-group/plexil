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
 * The model of a Plexil plan.
 */

public class Model extends Properties
{
    /** the type (usually the XML tag) which identifies what kind of
     * thing this model represents.  All other features of the model
     * are stored in properties and children. */
    
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
    private ArrayList<Stack<String>> variableList;    
    private ArrayList<String> actionList;
    private Model parent;
    private Vector<Model> children;
    private Vector<ChangeListener> changeListeners;
    
    public Model(String type)
    {
        this(type, -1);
    }
    
    public Model(String type, int row)
    {
	this.type = type;
        this.row_number = row;
        
        init();
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
        variableList = new ArrayList<Stack<String>>();
        actionList = new ArrayList<String>();
    }
    
    public String                       getType()                   { return type; }
    public int                          getRowNumber()              { return row_number; }
    public String                       getModelName()              { return modelName; }
    public Model                        getParent()                 { return parent; }    
    public Vector<Model>                getChildren()               { return children; }
    public String                       getAbsolutePlanName()       { return planName; }
    public String                       getAbsoluteScriptName()     { return scriptName; }
    public HashMap<Integer, ArrayList>  getConditionMap()           { return conditionMap; }
    public ArrayList<Stack<String>>     getVariableList()           { return variableList; }
    public ArrayList<String>            getActionList()             { return actionList; }
    public boolean                      getUnresolvedLibraryCall()  { return unresolvedLibraryCall; }
    public String                       getLibraryName()            { return libraryName; }
    public Set<String>                  getLibraryNames()           { return libraryFiles; }
    public Set<String>                  getMissingLibraries()       { return missingLibraryNodes; }
    
    public Model getChild(int i)
    {
	return children.get(i);
    }
    
    public String getPlanName()
    {
        if (!planName.equals(UNKNOWN))
            return planName.substring(planName.lastIndexOf("/") + 1, 
                   planName.length());
        else
            return planName;
    }

    public String getScriptName()
    {
        if (!scriptName.equals(UNKNOWN))
            return scriptName.substring(scriptName.lastIndexOf("/") + 1, 
                   scriptName.length());
        else
            return scriptName;
    }
    
    public static Model getRoot()
    {
	if (TheRootModel == null) 
        {
	    TheRootModel = new Model("dummy");
	    TheRootModel.setModelName("_The_Root_Model_");
	}
        
	return TheRootModel;
    }
    
    public boolean hasVariables()
    {
        return !variableList.isEmpty();
    }
    
    public boolean hasAction()
    {
        return !actionList.isEmpty();
    }
    
    public boolean hasConditions()
    {
        return !conditionMap.isEmpty();
    }
    
    public boolean hasCondition(String condition)
    {
        return conditionMap.containsKey(getConditionNum(condition));
    }
    
    public boolean isRoot()
    {
	return this == getRoot(); 
    }
    
    public boolean isNode()
    {
	return type.equals(NODE);
    }

    public void setRowNumber(int row)
    {
        row_number = row;      
    }

    public void setModelName(String name)
    {
	modelName = name;
    }
    
    public void setParent(Model newParent)
    {
	parent = newParent;
    }
    
    public void setUnresolvedLibraryCall(boolean val)
    {
	unresolvedLibraryCall = val;
    }
    
    public void setLibraryName(String libname)
    {
	libraryName = libname;
    }
    
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
    
    public void resetFoundChildFlag()
    {
        found = false;
    }
    
    public void addConditionInfo(int condition, String conditionEquation)
    { 
        ArrayList<String> equationHolder = 
                ConditionsTab.formatCondition(conditionEquation);

        conditionMap.put(condition, equationHolder);
    }
    
    public void addVariableInfo(Stack variable)
    { 
        Stack<String> copyVariable = new Stack<String>();
        
        Object[] obj = variable.toArray();
        
        for (int i = 0; i < obj.length; i++)
        {
            copyVariable.push((String) obj[i]);
        }
        
        variableList.add(copyVariable);
    }
    
    public void addActionInfo(String action)
    { 
        String formattedAction = ActionTab.formatAction(action);
  
        actionList.add(formattedAction);
    }

    // specify the plan file name
    public void addPlanName(String planName)
    {
        this.planName = planName;
	for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).planNameAdded(this, planName);
    }
      
    // specify the script file name.
    public void addScriptName(String scriptName)
    {
        this.scriptName = scriptName;
	for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).scriptNameAdded(this, scriptName);
    }

    // specify a library name
    public void addLibraryName(String libraryName)
    {
	if (libraryFiles.add(libraryName))
        {
            for (int i = 0; i < changeListeners.size(); ++i)
                changeListeners.get(i).libraryNameAdded(this, libraryName);
        }
    }

    // notify top level node that a library was not found
    public void addMissingLibrary(String nodeName)
    {
	missingLibraryNodes.add(nodeName);
    }

    // notify that a missing library has been found
    private boolean missingLibraryFound(String nodeName) 
    {
	return missingLibraryNodes.remove(nodeName);
    }
    
    // create and add a child to this model.
    public void addChild(Model child)
    {
	assert child.isNode();

	children.add(child);
	child.setParent(this);
    }
    
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
    
    // clear node of all children and properties
    public void removeChildren()
    {
	for (Model child: children)
            child.parent = null;
	children.clear();
    }
    
    // return the top level ancestor of this node. 
    public Model topLevelNode()
    {
	if (this == getRoot())
	    return null;
	if (parent == getRoot())
	    return this;
	return parent.topLevelNode(); 
    }

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
    
    // model-to-model comparison
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

    // signal that a new plan has been installed under this model
    public void planChanged()
    {
	for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).planChanged(this);
    }
    
    public Model findChildByName(String name)
    {
	for (Model child : children)
	    if (child.modelName.equals(name))
		return child;
	return null;
    }
    
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

    //* Link the given library into this node and make the appropriate annotations in the top level node.
    //  @param library The library node
    //  @return true if the library satisfied an unresolved call, false otherwise.
    //  @note

    public boolean linkLibrary(Model library)
    {
	boolean result = linkLibraryInternal(library);
	if (result) {
	    Model topLevelNode = topLevelNode();
	    if (topLevelNode != null) {
		topLevelNode.addLibraryName(library.planName);
		missingLibraryFound(library.modelName);

		// Inherit the libraries called by this one
		for (String libfile: library.libraryFiles)
		    topLevelNode.addLibraryName(libfile);
		for (String libnode: library.missingLibraryNodes)
		    topLevelNode.addMissingLibrary(libnode);
	    }
	}
        
	return result;
    }

    //* Link the given library into this node.
    //  @param library The library node
    //  @return true if the library satisfied an unresolved call, false otherwise.

    private boolean linkLibraryInternal(Model library)
    {
	boolean result = linkLibraryLocal(library);
	if (!result) {
	    for (Model child : children) {
		result = result || child.linkLibraryInternal(library);
	    }
	}
        
	return result;
    }

    //* Link the given library into this library call node.
    //  @param library The library node
    //  @return true if the library satisfied an unresolved call, false otherwise.

    private boolean linkLibraryLocal(Model library)
    {
	if (type.equals(NODE)
	    && getProperty(NODETYPE_ATTR).equals(LIBRARYNODECALL)
	    && libraryName.equals(library.modelName)) {
	    addChild((Model) library.clone());
	    unresolvedLibraryCall = false;
	    return true;
	}
        
	return false;
    }
    
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
    
    public void resetMainAttributesOfAllNodes()
    {
        this.setMainAttributesOfNode();
        
        for (Model child : this.getChildren())
        {
            child.resetMainAttributesOfAllNodes();
        }
    }
            
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
            
    // add a property change listener to this model. 
    public void addChangeListener(ChangeListener listener)
    {
	changeListeners.add(listener);
    }

    // remove a property change listener frome this model. 
    public void removeChangeListener(ChangeListener listener)
    {
	changeListeners.remove(listener);
    }

    // listener which is signaled when a the model is changed in some way
    public abstract static class ChangeListener
    {
	abstract public void propertyChange(Model model, String property);

	abstract public void planCleared(Model model);

	abstract public void planChanged(Model model);

	abstract public void planNameAdded(Model model, String planName);
            
	abstract public void scriptNameAdded(Model model, String scriptName);

	abstract public void libraryNameAdded(Model model, String libraryName);
    }

    /** An adapter which is signaled when the model is changed in some way.
     */

    public static class ChangeAdapter extends ChangeListener
    {
	public void propertyChange(Model model, String property){}

	public void planCleared(Model model){}

	public void planChanged(Model model){}

	public void planNameAdded(Model model, String planName){}
            
	public void scriptNameAdded(Model model, String scriptName){}

	public void libraryNameAdded(Model model, String libraryName){}
    }
}
