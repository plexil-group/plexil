/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

package plexil;

import java.util.Vector;

import net.n3.nanoxml.IXMLElement;

//
// A class to represent the context of one PLEXIL node in the translator.
//

//
// *** To do:
//

public class NodeContext
{

    protected NodeContext parentContext;
    protected Vector<VariableName> m_variables = new Vector<VariableName>();
    protected Vector<NodeContext> children = new Vector<NodeContext>();
    protected String nodeName = null;
    protected PlexilTreeNode resourcePriorityAST = null;
    protected IXMLElement resourcePriorityXML = null;
    protected Vector<PlexilTreeNode> resources = new Vector<PlexilTreeNode>();

    public NodeContext(NodeContext previous, String name)
    {
        parentContext = previous;
		nodeName = name;
        if (previous != null) {
			previous.addChildNode(this);
		}
    }

    public boolean isGlobalContext()
    {
        return false;
    }

    public NodeContext getParentContext()
    {
        return parentContext;
    }

    public void addChildNode(NodeContext child)
    {
        children.add(child);
    }

	public void setNodeName(String name)
	{
		nodeName = name;
	}

    public String getNodeName()
    {
        return nodeName;
    }

    // get the root of this context tree
    protected NodeContext getRootContext()
        throws Exception
    {
        if (parentContext == null)
            // is global context -- error
            throw new Exception("getRootContext() called on global context");
        else if (parentContext.isGlobalContext())
            return this;
        return parentContext.getRootContext();
    }

    protected boolean isRootContext()
    {
        return (parentContext != null) && parentContext.isGlobalContext();
    }

    public boolean isLibraryNode()
    {
        return (nodeName != null) && isRootContext();
    }

    // *** this won't find library nodes!
    // Only finds nodes in the current tree.
    public NodeContext findNode(String name)
    {
        if (name == null)
            return null;
        NodeContext result = null;
        try {
			result = getRootContext().findNodeInternal(name);
		}
        catch (Exception e) {
		}
        return result;
    }

    protected NodeContext findNodeInternal(String name)
    {
        // check self
        if ((nodeName != null)
            && nodeName.equals(name))
            return this;

        // recurse down children
        for (NodeContext child : children) {
			NodeContext result = 
				child.findNodeInternal(name);
			if (result != null)
				return result;
		}
        return null;
    }

	public NodeContext getChildContext(String name) {
		for (NodeContext child : children)
			if (child.getNodeName() == name)
				return child;
		return null;
	}

    public boolean isNodeName(String name)
    {
        if (name == null)
            return false;

        return (findNode(name) != null);
    }

    // Creates a locally unique node name based on this node's name
    public String generateChildNodeName()
    {
        int childCount = children.size() + 1;
        return ((nodeName == null) ? "__ANONYMOUS_NODE" : nodeName)
            + "__CHILD__" + childCount;
    }

    //
    // Resources and resource priority
    //

    public Vector<PlexilTreeNode> getResources()
    {
        return resources;
    }

    public void addResource(PlexilTreeNode resourceAST)
    {
        resources.add(resourceAST);
    }

    public PlexilTreeNode getResourcePriorityAST()
    {
        return resourcePriorityAST;
    }

    public void setResourcePriorityAST(PlexilTreeNode priority)
    {
        resourcePriorityAST = priority;
    }

    public IXMLElement getResourcePriorityXML()
    {
        return resourcePriorityXML;
    }

    public void setResourcePriorityXML(IXMLElement priority)
    {
        resourcePriorityXML = priority;
    }

    public boolean declareInterfaceVariable(PlexilTreeNode declaration,
											PlexilTreeNode nameNode, 
											boolean isInOut, 
											PlexilDataType typ)
    {
        // For library nodes, just add the declaration
        if (isLibraryNode()) {
			return addInterfaceVariable(declaration, nameNode, isInOut, typ, null, true);
		}
        else {
			boolean success = true;
			// Not a library node -- find original definition, if any
			// N.B. getInheritedVariable can issue a diagnostic if 
			// the variable is previously declared In and now is redeclared InOut
			VariableName ext = findInheritedVariable(nameNode.getText());
			if (ext == null) {
				CompilerState.getCompilerState().addDiagnostic(nameNode, 
															   "Interface variable \""
															   + nameNode.getText()
															   + "\" was not found",
															   Severity.ERROR);
				success = false;
			}
			else {
				// If type supplied, check it for consistency
				if ((typ != null) && (ext.getVariableType() != typ)) {
					CompilerState.getCompilerState().addDiagnostic(nameNode,
																   "Interface variable \"" + nameNode.getText() +
																   "\" declared as type " +
																   typ.typeName() +
																   ", but is actually of type " +
																   ext.getVariableType(),
																   Severity.ERROR);
					success = false;
				}
				success = addInterfaceVariable(declaration, nameNode, isInOut, typ, ext, true);
			}
			return success;
		}
    }

    protected boolean addInterfaceVariable(PlexilTreeNode declaration,
										   PlexilTreeNode nameNode,
										   boolean isInOut,
										   PlexilDataType typ,
										   VariableName original,
										   boolean isDeclared)
    {
        InterfaceVariableName var = null;
        if (isLibraryNode()) {
			var = new InterfaceVariableName(declaration, nameNode.getText(), isInOut, typ);
		}
        else if (original == null) {
			// no such variable
			CompilerState.getCompilerState().addDiagnostic(declaration,
														   "Interface variable \""
														   + nameNode.getText()
														   + "\" is not a known variable",
														   Severity.ERROR);
			return false;
		}
        else {
			var = 
				new InterfaceVariableName(declaration, nameNode.getText(), isInOut, original, isDeclared);
		}
		if (var != null)
			m_variables.add(var);
        return var != null;
    }

    // Caller is responsible for creating the 3 vectors
    public void getNodeVariables(Vector<VariableName> localVarsResult,
                                 Vector<InterfaceVariableName> inVarsResult,
                                 Vector<InterfaceVariableName> inOutVarsResult)
    {
        localVarsResult.removeAllElements();
        inVarsResult.removeAllElements();
        inOutVarsResult.removeAllElements();
        for (VariableName var : m_variables) {
			if (var.isLocal())
				localVarsResult.add(var);
			else if (var.isAssignable())
				inOutVarsResult.add((InterfaceVariableName) var);
			else
				inVarsResult.add((InterfaceVariableName) var);
		}
    }

    public VariableName declareVariable(PlexilTreeNode declaration,
										PlexilTreeNode nameNode,
										PlexilDataType varType,
										ExpressionNode initialValueExpr)
    {
        if (checkVariableName(nameNode)) {
			VariableName result = new VariableName(declaration, nameNode.getText(), varType, initialValueExpr);
			m_variables.add(result);
			return result;
		}
		else 
			return null;
    }

    //
    // Array variables
    //

    public VariableName declareArrayVariable(PlexilTreeNode declaration,
											 PlexilTreeNode nameNode, 
											 PlexilDataType arrayType,
											 String maxSize,
											 ExpressionNode initialValueExpr)
    {
		if (checkVariableName(nameNode)) {
			VariableName result = 
				new VariableName(declaration, nameNode.getText(), arrayType, maxSize, initialValueExpr);
			m_variables.add(result);
			return result;
		}
		else 
			return null;
    }

	// Returns true if name is not in direct conflict with other names in this context.
	// Adds diagnostics to myState if required
    public boolean checkVariableName(PlexilTreeNode nameNode)
    {
		boolean success = true;
		String name = nameNode.getText();
        VariableName existing = findLocalVariable(name);
        if (existing != null) {
			// error - duplicate variable name in node
			CompilerState.getCompilerState().addDiagnostic(nameNode,
														   "Variable name \"" + name
														   + "\" is already declared locally",
														   Severity.ERROR);
			CompilerState.getCompilerState().addDiagnostic(existing.getDeclaration(),
														   "Variable \"" + name
														   + "\" previously declared here",
														   Severity.NOTE);
			success = false;
		}
        if (parentContext != null) {
			VariableName shadowedVar =
				parentContext.findInheritedVariable(name);
			if (shadowedVar != null)
				// warn of conflict
				CompilerState.getCompilerState().addDiagnostic(nameNode,
															   "Local variable \"" + name
															   + "\" shadows an inherited variable",
															   Severity.WARNING);
		}
		return success;
    }

    protected VariableName findLocalVariable(String name)
    {
        for (VariableName candidate : m_variables)
            if (candidate.getName().equals(name))
                return candidate;
        return null; 
    }

    // Look up an inherited variable with the given name.  
    // Returns the first instance found up the tree.
    // If none exists, return null.

    protected VariableName findInheritedVariable(String name)
    {
        if (parentContext == null)
            return null;
        VariableName vn = parentContext.findLocalVariable(name);
        if (vn != null)
            return vn;
        else
            return parentContext.findInheritedVariable(name);
    }

    public VariableName findVariable(String name)
    {
        VariableName result = findLocalVariable(name);
        if (result != null)
            return result;
        return findInheritedVariable(name);
    }

    //
    // Simple queries w/o side effects
    //

    public boolean isVariableName(String name)
    {
        VariableName vn = findVariable(name);
        return (vn != null) ;
    }

}
