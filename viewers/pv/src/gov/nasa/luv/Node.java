/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

import java.awt.Color;
import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Stack;
import java.util.Vector;

import static gov.nasa.luv.Constants.*;

public class Node
    extends java.util.Properties
    implements Cloneable {

    /** A collection of all the possible Plexil Plan node types. */
    public static final Map<String, String> NODE_TYPES =
        new HashMap<String, String>(6) {
            {
                put(NODELIST,        "List Node");
                put(COMMAND,         "Command Node");
                put(ASSN,            "Assignment Node");
                put(EMPTY,           "Empty Node");
                put(UPDATE,          "Update Node");
                put(LIBRARYNODECALL, "Library Node");
            }
        };

    protected String nodeName; 
    protected String type;
    protected String state;
    protected String outcome;
    protected String failureType;

    /** Library name for a LibraryNodeCall. */
    protected String libraryName;
    protected Node parent;
    protected Vector<Node> children;
    private HashMap<Integer, ArrayList> conditionMap;
    private ArrayList<Variable> variableList;
    private ArrayList<String> actionList;
    private Vector<LuvBreakPoint> breakPoints;
    protected Vector<ChangeListener> changeListeners;
    private int row_number;
    private boolean unresolvedLibraryCall;               

    /** Standard constructor when loading a plan. */
    public Node(String typ, int row) {
        super();
        nodeName = null;
        type = typ;
        state = INACTIVE;
        outcome = null;
        failureType = null;
        libraryName = null;
        parent = null;
        if (typ == null // special global root node
            || NODELIST.equals(typ) 
            || LIBRARYNODECALL.equals(typ))
            children = new Vector<Node>();
        else
            children = null;
        conditionMap = new HashMap<Integer, ArrayList>();
        variableList = new ArrayList<Variable>();
        actionList = new ArrayList<String>();
        breakPoints = new Vector<LuvBreakPoint>();
        changeListeners = new Vector<ChangeListener>();
        row_number = row;
        unresolvedLibraryCall = false;
    }

    /** Copy constructor. Used by library linker. */
    public Node(Node orig) {
        super(orig); // copies properties
        nodeName = orig.nodeName;
        type = orig.type;
        state = orig.state;
        outcome = orig.outcome;
        failureType = orig.failureType;
        libraryName = orig.libraryName;
        // parent will be supplied by caller
        conditionMap = orig.conditionMap;
        variableList = new ArrayList<Variable>();
        for (Variable v : orig.variableList) {
            variableList.add(v.clone());
        }

        actionList = orig.actionList;
        breakPoints = new Vector<LuvBreakPoint>();
        changeListeners = new Vector<ChangeListener>();
        row_number = orig.row_number;
        unresolvedLibraryCall = orig.unresolvedLibraryCall;

        // Recurse over children
        if (orig.children != null) {
            children = new Vector<Node>(orig.children.size());
            for (Node n : orig.children)
                addChild(n.clone());
        }
    }

    public Node clone() {
        return new Node(this);
    }

    /** Returns the name of this Plexil Node.
     *  @return the name */
    public String                       getNodeName()              { return nodeName; }

    /** Returns the type of this Plexil Node.
     *  @return the type */
    public String                       getType()                   { return type; }

    /** Returns the row number of this Plexil Node.
     *  @return the row number */
    public int                          getRowNumber()              { return row_number; }

    /** Returns the parent of this Plexil Node.
     *  @return the parent */
    public Node                        getParent()                 { return parent; }    

    /** Returns the Vector of Node children of this Node.
     *  @return the children */
    public Vector<Node>                getChildren()               { return children; }

    //
    // Breakpoint handling
    //

    public Vector<LuvBreakPoint> getBreakPoints() {
        return breakPoints;
    }

    public void clearBreakPoints() {
        breakPoints.clear();
    }

    public void addBreakPoint(LuvBreakPoint bp) {
        for (LuvBreakPoint l : breakPoints) {
            if (l.toString().equals(bp.toString())) {
                Luv.getLuv().getStatusMessageHandler().showStatus("Breakpoint \"" + bp 
                                                                  + "\" already set", Color.RED, 5000l);
                return;
            }
        }

        breakPoints.add(bp);
        addChangeListener(bp); // *** redundant? ***
        // do UI bookkeeping last
        Luv.getLuv().getLuvBreakPointHandler().addBreakPoint(bp);
    }

    // Called from node menu
    public void removeBreakPoint(LuvBreakPoint bp) {
        // do UI bookkeeping first
        Luv.getLuv().getLuvBreakPointHandler().removeBreakPoint(bp);

        removeChangeListener(bp); // *** redundant? ***
        breakPoints.remove(bp);
    }

    // Same, but from global menu - no UI bookkeeping to do.
    public void deleteBreakPoint(LuvBreakPoint bp) {
        removeChangeListener(bp); // *** redundant? ***
        breakPoints.remove(bp);
    }

    public String getState() {
        return state;
    }

    public void setState(String newval) {
        String oldState = state;
        state = newval;
        if (oldState == null || !oldState.equals(state))
            for (ChangeListener cl: changeListeners)
                cl.propertyChange(this, NODE_STATE);
    }

    public String getOutcome() {
        return outcome;
    }

    public void setOutcome(String newval) {
        String oldOutcome = outcome;
        outcome = newval;
        if (oldOutcome == null || !oldOutcome.equals(outcome))
            for (ChangeListener cl: changeListeners)
                cl.propertyChange(this, NODE_OUTCOME);
    }

    public String getFailureType() {
        return failureType;
    }

    public void setFailureType(String newval) {
        String oldFailureType = failureType;
        failureType = newval;
        if (oldFailureType == null || !oldFailureType.equals(failureType))
            for (ChangeListener cl: changeListeners)
                cl.propertyChange(this, NODE_FAILURE_TYPE);
    }

    /** Returns the HashMap of Conditions for this Node.
     *  @return the Conditions */
    public HashMap<Integer, ArrayList>  getConditionMap()           { return conditionMap; }

    /** Returns the ArrayList of local variables for this Node.
     *  @return the variables */
    public ArrayList<Variable>     getVariableList()           { return variableList; }

    /** Returns the ArrayList of actions for this Node.
     *  @return the actions */
    public ArrayList<String>            getActionList()             { return actionList; }

    /** Returns whether this Node has an unresolved library call.
     *  @return true if unresolved, false otherwise */
    public boolean                      getUnresolvedLibraryCall()  { return unresolvedLibraryCall; }
    
    /** Returns the Library Name for this Plexil Model/node.
     *  @return the Library Name for this Plexil Model/node */
    public String                       getLibraryName()            { return libraryName; }

    /**
     * Returns the specified child of this Node.
     * @param i the index of the child for this Node
     * @return the child or null
     */
    public Node getChild(int i)
    {
        if (children == null)
            return null;
        return children.get(i);
    }

    public boolean hasChildren() {
        return children != null
            && !children.isEmpty();
    }

    public boolean isRoot() {
        return parent == Model.getRoot();
    }

    /**
     * Returns the top level ancestor of this Node.
     * @return the top level ancestor
     */
    public Model topLevelNode() {
        if (this == Model.getRoot())
            return null;
        if (parent == Model.getRoot())
            return (Model) this;
        return parent.topLevelNode();
    }
    
    /**
     * Returns whether or not this Node has local variables.
     * @return whether or not this Node has local variables
     */
    public boolean hasVariables()
    {
        return !variableList.isEmpty();
    }
    
    /**
     * Returns whether or not this Node has actions.
     * @return whether or not this Node has actions
     */
    public boolean hasAction()
    {
        return !actionList.isEmpty();
    }
    
    /**
     * Returns whether or not this Node has conditions.
     * @return whether or not this Node has conditions
     */
    public boolean hasConditions()
    {
        return !conditionMap.isEmpty();
    }
    
    /**
     * Returns whether or not this Node has the specified condition.
     * @return whether or not this Node has the specified condition
     */
    public boolean hasCondition(String condition)
    {
        return conditionMap.containsKey(getConditionNum(condition));
    }

    /**
     * Sets the specified name for this Node.
     * @param name the name for this Node
     */
    public void setNodeName(String name) {
        nodeName = name;
    }

    /**
     * Sets specified the row number for this Node.
     * @param row the row number for this Node
     */
    public void setRowNumber(int row)
    {
        row_number = row;      
    }
    
    /**
     * Sets the specified Node as this Node's parent.
     * @param newParent the parent Node of this Node
     */
    public void setParent(Node newParent) {
        parent = newParent;
    }
    
    /**
     * Sets whether this Node has an unresolved library call with this specified boolean value.
     * @param val the  boolean value to set whether or not this Node has an unresolved library call
     */
    public void setUnresolvedLibraryCall(boolean val) {
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
     * Sets the specified property value with key to this Node.
     * @param key the key to the specified value
     * @param value the value of a property for this Node
     * @return an object result
     */
    public Object setProperty(String key, String value) {
        if (key == null || value == null) {
            // Properties.setProperty() throws an exception if either arg is null
            Luv.getLuv().getStatusMessageHandler().showStatus("Warning: attempt to set property "
                                                              + (nodeName == null ? "" : " of node " + nodeName)
                                                              + "(key: " + key + ", type: " + type + ") to null");
            return null;
        }

        Object result = super.setProperty(key, value);
        if (value == null || value != result && value.equals(result) == false) {
            for (ChangeListener cl: changeListeners)
                cl.propertyChange(this, key);
        }
         
        return result;
    }

	public void setVariable(String vName, String value) {
		boolean found = false;
		if (vName != null) {
			Node walker = this;
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
     * Adds the specified condition and condition equation to the ArrayList of conditions for this Node.
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
     * Adds the specified local variable Stack to the list of variables for this Node. Converts into a Variable first
     * @param variable the local variable to add to this Node's Stack of local variables
     */
    public void addVariableInfo(Stack variable) {
        String value;
        String type;
        String name;
        String in_inout;
            
        if (variable.size() == 4) {
            value = (String) variable.pop();
            type = (String) variable.pop();
            name = (String) variable.pop();
            in_inout = (String) variable.pop(); 
        }
        else {
            value = UNKNOWN;
            type = UNKNOWN;
            name = UNKNOWN;
            in_inout = UNKNOWN;
        }
        
        if (!type.equals("UNKNOWN") && name.matches(".*\\[[0-9]*\\]"))
            variableList.add(new ArrayVariable("--", name, type, value));
        else
        	variableList.add(new Variable(in_inout, name, type, value));
    }
    
    /**
     * Adds the given variable to the list of variables for this Node.
     * @param v The local variable to add.
     */
    public void addVariableInfo(Variable v) {
    	variableList.add(v);
    }
    
    /**
     * Adds the specified action to the ArrayList of actions for this Node.
     * @param action  the action to add
     */
    public void addActionInfo(String action) { 
        actionList.add(ActionTab.formatAction(action));
    }
    
    /**
     * Creates and adds the specified Node child to this Node.
     * @param child the node child
     */
    public void addChild(Node child) {
        children.add(child);
        child.setParent(this);
    }
    
    /**
     * Removes the specified Node child from this Node.
     * @param child the Node child
     * @return whether or the child was removed
     * @note Only called on the model root.
     */
    public boolean removeChild(Node child)
    {
        // Can't use Vector.remove(Object) because it uses equals(),
        // and two nodes are often equal without being identical!
        boolean removed = false;
        for (Iterator<Node> it = children.iterator(); it.hasNext(); ) {
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
     * Returns the Node pathway starting from the specified Node to the root Node.
     * @param node the node to return the pathway for
     * @return the pathway
     */
    public Stack<String> pathToNode(Node node) {
        Stack<String> node_path = new Stack<String>();
        while (!node.isRoot()) {
            if (!AbstractNodeFilter.isNodeFiltered(node))
                node_path.push(node.getNodeName());
            node = node.getParent();
        }
        return node_path;
    }
    
    /**
     * Compares this Node with the specified Node to see if they are the same.
     * @param other the node to compare with
     * @return whether or not these two Nodes are equivalent
     */
    public boolean equivalent(Node other) {
        if (other == this)
            return true; // identity
        if (other == null) {
            System.out.println("Not equivalent because other is null");
            return false;
        }
        if (!type.equals(other.type)) {
            System.out.println("Not equivalent because other has different type");
            return false;
        }
        if (!nodeName.equals(other.nodeName)) {
            System.out.println("Not equivalent because node IDs differ; this = " + nodeName
                               + " other = " + other.nodeName);
            return false;
        }
        if (children == null) {
            if (other.children != null) {
                System.out.println("Not equivalent because other has children, this doesn't");
                return false;
            }
            return true;
        }
        if (children != null && other.children == null) {
            System.out.println("Not equivalent because this has children, other doesn't");
            return false;
        }
        if (children.size() != other.children.size()) {
            System.out.println("Not equivalent because number of children differs");
            return false;
        }
        for (int i = 0; i < children.size(); i++) {
            if (!children.get(i).equivalent(other.children.get(i))) {
                System.out.println("Not equivalent because children differ");
                return false;
            }
        }
        return true;
    }
    
    /**
     * Returns the Node that matches with the specified name.
     * @param name the name to match the Node to
     * @return the matching Node
     */
    public Node findChildByName(String name) {
        if (children == null)
            return null;
        for (Node child : children)
            if (child.nodeName.equals(name))
                return child;
        return null;
    }
    
    /**
     * Returns the Node that matches with the specified row number.
     * @param row the row number to match the Node to
     * @return the matching Node
     */
    public Node findChildByRowNumber(int row) {
        if (children == null)
            return null;
        // Simple depth-first search
        for (Node child : children) {
            if (child.row_number == row)
                return child;
            Node foundChild = child.findChildByRowNumber(row);
            if (foundChild != null)
                return foundChild;
        }        
        return null;
    }
            
    /**
     * Adds a property change listener to this Node. 
     * @param listener the property change listener
     */
    public void addChangeListener(ChangeListener listener) {
        changeListeners.add(listener);
    }

    /**
     * Removes a property change listener to this Node. 
     * @param listener the property change listener
     */
    public void removeChangeListener(ChangeListener listener) {
        changeListeners.remove(listener);
    }
         
    /**
     * Resets all the properties of this Node to the beginning values (pre-execution).
     */
    public void reset() {
        setMainAttributesOfNode();
        if (children == null)
            return;
        for (Node child : children)
            child.reset();
    }

    public void setMainAttributesOfNode()
    {
        String rawType = getProperty(NODETYPE_ATTR);
        if (rawType != null) {
            String polishedType = NODE_TYPES.get(rawType);
            if (polishedType != null)
            setProperty(NODE_TYPE, polishedType);
        }

        nodeName = getProperty(NODE_ID);
        state = INACTIVE;
        outcome = null;

        setProperty(SKIP_CONDITION, UNKNOWN);                     
        setProperty(START_CONDITION, UNKNOWN);               
        setProperty(END_CONDITION, UNKNOWN); 
        setProperty(EXIT_CONDITION, UNKNOWN);
        setProperty(INVARIANT_CONDITION, UNKNOWN);
        setProperty(PRE_CONDITION, UNKNOWN);
        setProperty(POST_CONDITION, UNKNOWN);
        setProperty(REPEAT_CONDITION, UNKNOWN);
        setProperty(ANCESTOR_INVARIANT_CONDITION, UNKNOWN);
        setProperty(ANCESTOR_END_CONDITION, UNKNOWN);
        setProperty(ANCESTOR_EXIT_CONDITION, UNKNOWN);
        setProperty(ACTION_COMPLETE, UNKNOWN);
        setProperty(ABORT_COMPLETE, UNKNOWN);
    }

    public String toString() {
        StringBuilder s = new StringBuilder();
        toStringInternal(s);
        return s.toString();
    }

    protected void toStringInternal(StringBuilder s) {
        if (nodeName == null)
            s.append(type);
        else
            s.append(nodeName);

        s.append("(");
        for (Entry<Object, Object> property: entrySet()) {
            s.append(" ");
            s.append(property.getKey());
            s.append(" = ");
            s.append(property.getValue());
        }
        s.append(")");

        if (children != null && !children.isEmpty()) {
            Node lastChild = children.lastElement();
            s.append("[");
            for (Node child: children) {
                child.toStringInternal(s);
                if (child != lastChild)
                    s.append(", ");
            }
            s.append("]");
        }
    }
        
    /**
     * Links the specified library into this Node and makes the appropriate 
     * annotations in the top level Model.
     * @param library the library for this Node
     * @return whether or not the library was linked
     */
    public boolean linkLibrary(Model library) {
        if (type.equals(NODE)
            && getProperty(NODETYPE_ATTR).equals(LIBRARYNODECALL)
            && libraryName.equals(library.nodeName)) {
            addChild(new Model(library));
            unresolvedLibraryCall = false;
            Model topLevelNode = topLevelNode();
            if (topLevelNode != null) {
                topLevelNode.addLibraryFile(library.getPlanFile());
                Model.getRoot().missingLibraryFound(library.getNodeName());
                return true;
            }
        }
        
        return false;
    }
      
    /**
     * Creates a LuvBreakPoint which fires when the specified node state changes at all.
     * 
     * @param node the node on which the LuvBreakPoint fires
     * @return the LuvBreakPoint
     */
    public LuvBreakPoint createChangeBreakpoint()
    {
        return new LuvBreakPoint(this,
                                 new StateChangeFilter());
    }
      
    /**
     * Creates a LuvBreakPoint which fires when the specified node reaches the target state.
     * 
     * @param node the node on which the LuvBreakPoint fires
     * @return the LuvBreakPoint
     */
    public LuvBreakPoint createNodeStateValueBreakpoint(final String targetState) {
        return new LuvBreakPoint(this,
                                 new StateValueFilter(targetState));
    }
      
    /**
     * Creates a LuvBreakPoint which fires when the specified node reaches the target outcome.
     * 
     * @param node the node on which the LuvBreakPoint fires
     * @return the LuvBreakPoint
     */
    public LuvBreakPoint createNodeOutcomeValueBreakpoint(final String targetOutcome) {
        return new LuvBreakPoint(this,
                                 new OutcomeValueFilter(targetOutcome));
    }

    /**
     * Creates a LuvBreakPoint which fires when the node reaches the target failure type.
     * 
     * @param node the node on which the LuvBreakPoint fires
     * @return the LuvBreakPoint
     */
    public LuvBreakPoint createNodeFailureValueBreakpoint(final String targetFailureType) {
        return new LuvBreakPoint(this,
                                 new FailureTypeValueFilter(targetFailureType));
    }
    
    /**
     * The ChangeListener is an abstract class that is signaled when the Plexil Node 
     * is changed in some way.
     */
    public static interface ChangeListener {
        public void propertyChange(Node node, String property);

        public void planChanged(Model model);

        public void planFileAdded(Model model, File planName);
            
        public void scriptFileAdded(Model model, File scriptName);

        public void libraryFileAdded(Model model, File libName);
	
        public void variableAssigned(Node node, String variableName);
    }

    /**
     * The ChangeAdapter class is signaled when the Plexil Model is changed in some way.
     */
    public static class ChangeAdapter implements ChangeListener {
        public void propertyChange(Node node, String property) {}

        public void planChanged(Model model) {}

        public void planFileAdded(Model model, File planName) {}

        public void scriptFileAdded(Model model, File scriptName) {}

        public void libraryFileAdded(Model model, File libName) {}
	
        public void variableAssigned(Node node, String variableName) {}
    }

    public static interface PropertyChangeFilter {
        public boolean eventMatches(Node n, String property);
        public String getDescription();
    }

    public static class StateChangeFilter
        implements PropertyChangeFilter {

        StateChangeFilter() {
        }

        public boolean eventMatches(Node n, String property) {
            return NODE_STATE.equals(property);
        }

        public String getDescription() {
            return " state changed";
        }
    }

    public static class StateValueFilter
        implements PropertyChangeFilter {

        private final String targetValue;
        private final String description;

        StateValueFilter(final String target) {
            targetValue = target;
            description = " state changed to " + target;
        }

        public boolean eventMatches(Node n, String property) {
            if (!NODE_STATE.equals(property))
                return false;
            return targetValue.equals(n.getState());
        }

        public String getDescription() {
            return description;
        }
    }

    public static class OutcomeValueFilter
        implements PropertyChangeFilter {

        private final String targetValue;
        private final String description;

        OutcomeValueFilter(final String target) {
            targetValue = target;
            description = " outcome changed to " + target;
        }

        public boolean eventMatches(Node n, String property) {
            if (!NODE_OUTCOME.equals(property))
                return false;
            return targetValue.equals(n.getOutcome());
        }

        public String getDescription() {
            return description;
        }
    }

    public static class FailureTypeValueFilter
        implements PropertyChangeFilter {

        private final String targetValue;
        private final String description;

        FailureTypeValueFilter(final String target) {
            targetValue = target;
            description = " failure type changed to " + target;
        }

        public boolean eventMatches(Node n, String property) {
            if (!NODE_FAILURE_TYPE.equals(property))
                return false;
            return targetValue.equals(n.getFailureType());
        }

        public String getDescription() {
            return description;
        }
    }
    
}
