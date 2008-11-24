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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import java.util.Set;
import java.util.LinkedHashSet;
import java.util.Properties;
import java.util.Map.Entry;

import static gov.nasa.luv.Constants.*;

/**
 * The model of a Plexil plan.
 */

public class Model extends Properties
{
    /** the type (usually the XML tag) which identifies what kind of
     * thing this model represents.  All other features of the model
     * are stored in properties and children. */

    private String type = UNKNOWN;
      
    private String pathToNode = UNKNOWN;

    private String modelName = UNKNOWN;
    
    private String planName = UNKNOWN;
    
    private String scriptName = UNKNOWN;
    
    private int row = 0;
    
    private int totalRows = 0;
    
    private boolean rowFound = false;

    private LinkedHashSet<String> libraryFiles = new LinkedHashSet<String>();

    private LinkedHashSet<String> missingLibraryNodes = new LinkedHashSet<String>();

    /** property change listeners registered for this model */ 

    private Vector<ChangeListener> changeListeners = new Vector<ChangeListener>();

    /** parent of this node */

    private Model parent;

    /** children of this node */

    private Vector<Model> children = new Vector<Model>();
      
    // condition info holders

    private HashMap<Integer, ArrayList> conditionMap = new HashMap<Integer, ArrayList>();
    
    private ArrayList<String> variableList = new ArrayList<String>();
      
    static HashMap<String, String> typeLut = new HashMap<String, String>()
    {
	{
	    put(NODELIST,        "List Node");
	    put(COMMAND,         "Command Node");
	    put(ASSN,            "Assignment Node");
	    put(EMPTY,           "Empty Node");
	    put(FUNCCALL,        "Function Call Node");
	    put(UPDATE,          "Update Node");
	    put(LIBRARYNODECALL, "Library Node");
	}
    };

    private static Model TheRootModel = null;

    /** Construct a Model.
     *
     * @param type string representation of the of object this is modeling
     */

    public Model(String type)
    {
	this.type = type;
    }
    
    public HashMap<Integer, ArrayList> getConditionMap()
    {
        return conditionMap;
    }
    
    public ArrayList<String> getVariableList()
    {
        return variableList;
    }
    
    public boolean hasVariables()
    {
        return !variableList.isEmpty();
    }
    
    public boolean hasConditions()
    {
        return !conditionMap.isEmpty();
    }
      
    public void setPathToNode()
    {
	Model node = this;
        
        pathToNode = "";
          
	while (node.parent != null && !node.parent.isRoot())
        {
            pathToNode += "--->" + node.parent.modelName;
            node = node.parent;
        }
          
	pathToNode = modelName + pathToNode;
    }
    
    public void addTotalNumberOfRows(Model model)
    {
        totalRows++;
        
        for (int i = 0; i < model.getChildCount(); i++)
        {
            addTotalNumberOfRows(model.getChild(i));
        }
    }
    
    public int getRowNumberOfNode(Model node, String nodeName)
    { 
        if (node.getModelName().equals(nodeName))
        {
            row++;
            rowFound = true;
        }
        else if (!rowFound)
        { 
            row++;
            for (int i = 0; i < node.getChildCount(); i++)
            {
                getRowNumberOfNode(node.getChild(i), nodeName);
            }
        }
 
        if (row == totalRows && !rowFound)
            return -1;
        else
            return row;
    }
    
    public void resetRowNumber()
    {
        row = 0;
        rowFound = false;
    }
      
    public String getPathToNode()
    {
	return pathToNode;
    }

    public String getModelName()
    {
	return modelName;
    }
    
    public String getPlanName()
    {
        return planName;
    }

    public String getScriptName()
    {
        return scriptName;
    }
    
    public String getPlanNameSansPath()
    {
        if (!planName.equals(UNKNOWN))
            return planName.substring(planName.lastIndexOf("/") + 1, planName.length());
        else
            return planName;
    }

    public String getScriptNameSansPath()
    {
        if (!scriptName.equals(UNKNOWN))
            return scriptName.substring(scriptName.lastIndexOf("/") + 1, scriptName.length());
        else
            return scriptName;
    }

    public void setModelName(String name)
    {
	modelName = name;
    }

    public static Model getRoot()
    {
	if (TheRootModel == null) {
	    TheRootModel = new Model("dummy");
	    TheRootModel.setModelName("_The_Root_Model_");
	}
	return TheRootModel;
    }

    /** Clone a model.
     *
     * @return the clone of this model
     */

    // *** Is this complete?!? ***

    public Object clone()
    {
	Model clone = (Model)super.clone();

	// deeply clone the children of this model

	clone.children = new Vector<Model>();
	for (Model child: children)
	    clone.addChild((Model)child.clone());

	// return the copy, is this ethical? :)
         
	return clone;
    }

    // Model-to-model comparison

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
	if (!pathToNode.equals(other.pathToNode))
	    return false;        
        //if (!super.equals(other)) // compare properties
	    //return false;
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
	if (children.isEmpty()) {
	    if (other.children.isEmpty())
		return true;
	    else
		return false;
	}
	else if (other.children.isEmpty())
	    return false;
	else if (children.size() != other.children.size())
	    return false;
	for (int i = 0; i < children.size(); i++) {
	    if (!children.get(i).equivalent(other.children.get(i)))
		return false;
	}
	return true;
    }

    /** Accesor for model type.
     *
     * @return the type of the model
     */

    public final String getType()
    {
	return type;
    }

    /** Signal that a new plan has been installed under this model. */

    public void planChanged()
    {
	for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).planChanged(this);
    }

    /** Accesor for model children.
     *
     * @return the children of the model
     */

    public final Vector<Model> getChildren()
    {
	return children;
    }

    /** Create and add a child to this model.
     *
     * @param child child to add to this model
     */

    public void addChild(Model child)
    {
	assert child.isNode();

	children.add(child);
	child.setParent(this);
    }
      
    public void addConditionInfo(int condition, ArrayList<String> equationHolder)
    { 
	if (!equationHolder.isEmpty())
	    conditionMap.put(condition, equationHolder);
    }
    
    public void addVariableInfo(String variable)
    { 
        String formattedVariable = formatVariable(variable);
  
        if (!formattedVariable.equals(""))
            variableList.add(formattedVariable);
    }
 
    // determine if the variable for this node is singular or an array
    // and teh format it appropriately for display purposes
    private String formatVariable(String variable)
    {
        String formattedVariable = "COULD NOT IDENTIFY VARIABLE";
        String array[] = variable.split(":");      // : is used as a place holder in between variable elements
        
        if (array.length > 0)
        {
            if (array[0].equals(VAR))
            {
                if (array.length == 3)
                {
                    formattedVariable = array[2] + " " + array[1];
                }
                else if (array.length == 4)
                {
                    formattedVariable = array[2] + " " + array[1] + " = " + array[3];
                }
            }
            else if (array[0].equals(ARRAY))
            {
                if (array.length == 3)
                    ;
                else if (array.length == 4)
                {
                    formattedVariable = array[2] + " " +  array[1] + "[" + array[3] + "]";
                }
                else if (array.length > 4)
                {
                    formattedVariable = array[2] + " " +  array[1] + "[" + array[3] + "] = {";
                    
                    for (int i = 4; i < array.length; i++)
                    {
                        formattedVariable += array[i];
                        if (i + 1 < array.length)
                            formattedVariable += ",";
                    }
                    
                    formattedVariable += "}";
                }
            }
        }
        
        return formattedVariable;
    }

    /** Add a parent node to this node. 
     *
     * @param parent node of which this is a child
     */

    public void setParent(Model newParent)
    {
	parent = newParent;
    }

    /** Test if a given model is the root of a tree.
     *
     * @return true if this model is the root of a model tree
     */

    public boolean isRoot()
    {
	return this == getRoot(); 
    }

    /** Get the parent node to this node. 
     *
     * @return the parent of this node or null if it is a root node
     */

    public Model getParent()
    {
	return parent;
    }

    /** Clear node of all children and properties. */

    public void clear()
    {
	removeChildren();
	super.clear();
	for (ChangeListener cl: changeListeners)
            cl.planCleared(this);
	libraryFiles.clear();
	missingLibraryNodes.clear();
    }

    /** Specify the plan file name.
     *
     * @param planName name of plan file
     */

    public void addPlanName(String planName)
    {
        this.planName = planName;
	for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).planNameAdded(this, planName);
    }
      
    /** Specify the script file name.
     *
     * @param scriptName name of script file
     */

    public void addScriptName(String scriptName)
    {
        this.scriptName = scriptName;
	for (int i = 0; i < changeListeners.size(); ++i)
            changeListeners.get(i).scriptNameAdded(this, scriptName);
    }

    /** Specify a library name.
     *
     * @param libraryName name of library file
     */

    public void addLibraryName(String libraryName)
    {
	if (libraryFiles.add(libraryName))
        {
            for (int i = 0; i < changeListeners.size(); ++i)
                changeListeners.get(i).libraryNameAdded(this, libraryName);
        }
    }

    /** Get a list of library names recorded in this model.
     *
     * @return a vector of library names, which might be empty
     */

    public Set<String> getLibraryNames()
    {
	return libraryFiles;
    }


    //* Notify that a library was not found.
    //  @param nodeName The name of the missing library node.

    public void addMissingLibrary(String nodeName)
    {
	missingLibraryNodes.add(nodeName);
    }

    //* Notify that a missing library has been found.
    //  @param nodeName The name of the formerly missing library node.
    //  @return true if the node name was marked missing, false otherwise.

    public boolean missingLibraryFound(String nodeName) 
    {
	return missingLibraryNodes.remove(nodeName);
    }

    //* Get names of library nodes missing from this model.
    //  @return Vector of node names
    public Set<String> getMissingLibraries()
    {
	return missingLibraryNodes;
    }

    /** Join all data from the two models together.
     *
     * @param other other node to join data into this one
     */

    public void join(Model other)
    {
	changeListeners.addAll(other.changeListeners);
	children.addAll(other.children);
	putAll(other);
    }

    /** Clear node of all children and properties. */

    public void removeChildren()
    {
	for (Model child: children)
            child.parent = null;
	children.clear();
    }
     
    public Model getChild(int i)
    {
	return children.get(i);
    }

    public int getChildCount()
    {
	return children.size();
    }
      
    public Model findChildByName(String name)
    {
	for (Model child: children)
	    if (child.modelName.equals(name))
		return child;
	return null;
    }

    /** Find all children of a given type. 
     *
     * @return a list of children of a given type.
     */

    public Vector<Model> findChildren(String type)
    {
	Vector<Model> matches = new Vector<Model>();
	for (Model child: children)
            if (child.type.equalsIgnoreCase(type))
		matches.add(child);
	return matches;
    }

    public Model findChild(String type)
    {
	for (Model child: children)
            if (child.type.equalsIgnoreCase(type))
		return child;
	return null;
    }

    public Model findChild(String property, String value)
    {
	for (Model child: children)
        {
            String prop = child.getProperty(property);
            if (prop != null && prop.equals(value))
                return child;
        }
        
	return null;
    }

    public Model removeChild(String type)
    {
	for (Model child: children)
            if (child.type.equalsIgnoreCase(type))
		{
		    removeChild(child);
		    return child;
		}
	return null;
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

    public Object setProperty(String key, String value)
    {
	if (value == null)
        {
            Luv.getLuv().displayErrorMessage(null, "ERROR: error occured while setting model property " +
                    "(key: " + key + ", type: " + type + ", value: NULL)");        
        }

	Object result = super.setProperty(key, value);

	if (value != result && value.equals(result) == false) {
            for (ChangeListener cl: changeListeners)
		cl.propertyChange(this, key);
	}
         
	return result;
    } 

    public boolean isNode()
    {
	return type.equals(NODE);
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
	String polishedtype = rawType != null ? typeLut.get(rawType) : null;
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
            
    /** Add a property change listener to this model. 
     *
     * @param listener the listener which will be added to the set of
     * listeners which will be signaled when a property change occurs
     */

    public void addChangeListener(ChangeListener listener)
    {
	changeListeners.add(listener);
    }

    /** Remove a property change listener frome this model. 
     *
     * @param listener the listener which will be removed to the set
     * of listeners which will be signaled when a property change
     * occurs
     */

    public void removeChangeListener(ChangeListener listener)
    {
	changeListeners.remove(listener);
    }

    /** A listener which is signaled when a the model is changed in some way.
     */

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
