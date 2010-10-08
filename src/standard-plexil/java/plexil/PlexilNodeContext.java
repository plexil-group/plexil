// -*- Mode: Java; -*-

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

package plexil;

import java.util.Iterator;
import java.util.Vector;
import antlr.collections.AST;
import net.n3.nanoxml.IXMLElement;

import plexil.PlexilVariableName;
import plexil.PlexilInterfaceVariableName;

//
// A class to represent the context of one PLEXIL node in the translator.
//

//
// *** To do:
//

public class PlexilNodeContext
{

    protected PlexilNodeContext parentContext;
    protected Vector<PlexilVariableName> variables = new Vector<PlexilVariableName>();
    protected Vector<PlexilNodeContext> childNodes = new Vector<PlexilNodeContext>();
    protected String nodeName;
    protected AST resourcePriorityAST = null;
    protected IXMLElement resourcePriorityXML = null;
    protected Vector<AST> resources = new Vector<AST>();

    public PlexilNodeContext(PlexilNodeContext previous, String name)
    {
	parentContext = previous;
	nodeName = name; // *** may be null 
	if (previous != null)
	    {
		previous.addChildNode(this);
	    }

	if (plexil.Parse.debugWriter != null)
	    try 
		{
		    plexil.Parse.debugWriter.write("New PlexilNodeContext ");
		    if (name == null)
			plexil.Parse.debugWriter.write("(unnamed)");
		    else
			plexil.Parse.debugWriter.write(name);
		    plexil.Parse.debugWriter.write('\n');		
		}
	    catch (Exception e)
		{
		    System.out.println("Exception " + e.getClass() +
				       " caught while writing to debug file");
		}
    }

    public boolean isGlobalContext()
    {
	return false;
    }

    public PlexilNodeContext getParentContext()
    {
	return parentContext;
    }

    public void addChildNode(PlexilNodeContext child)
    {
	childNodes.add(child);
    }

	public String getNodeName()
    {
		return nodeName;
    }

    // get the root of this context tree
    protected PlexilNodeContext getRootContext()
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
    public PlexilNodeContext findNode(String name)
    {
	if (name == null)
	    return null;
	PlexilNodeContext result = null;
	try
	    {
		result = getRootContext().findNodeInternal(name);
	    }
	catch (Exception e)
	    {
	    }
	return result;
    }

    protected PlexilNodeContext findNodeInternal(String name)
    {
	// check self
	if ((nodeName != null)
	    && nodeName.equals(name))
	    return this;

	// recurse down children
	Iterator<PlexilNodeContext> childIt = childNodes.iterator();
	while (childIt.hasNext())
	    {
		PlexilNodeContext result = 
		    childIt.next().findNodeInternal(name);
		if (result != null)
		    return result;
	    }
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
	int childCount = childNodes.size() + 1;
	return ((nodeName == null) ? "__ANONYMOUS_NODE" : nodeName)
               + "__CHILD__" + childCount;
    }

    //
    // Resources and resource priority
    //

    public Vector<AST> getResources()
    {
        return resources;
    }

    public void addResource(AST resourceAST)
    {
        resources.add(resourceAST);
    }

    public AST getResourcePriorityAST()
    {
        return resourcePriorityAST;
    }

    public void setResourcePriorityAST(AST priority)
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

    public void declareInterfaceVariable(String name, 
					 boolean isInOut, 
					 PlexilDataType typ)
	throws antlr.SemanticException
    {
	// For library nodes, just add the declaration
	if (isLibraryNode())
	    {
		addInterfaceVariable(name, isInOut, typ, null, true);
	    }
	else
	    {
		// Not a library node -- find original definition, if any
		// N.B. getInheritedVariable() can throw a SemanticException if 
		// the variable is previously declared In and now is redeclared InOut
		PlexilVariableName ext =
		    this.getInheritedVariable(name, isInOut); // has side effect of declaring up the tree
		if (ext == null)
		    {
			throw new antlr.SemanticException("declareInterfaceVariable: Interface variable '"
							  + name
							  + "' unknown");
		    }
		else
		    {
			// If type supplied, check it for consistency
			if ((typ != null) && (ext.getVariableType() != typ))
			    throw new antlr.SemanticException("Interface variable '" + name +
							      "' declared as type " +
							      typ.typeName() +
							      ", but is actually of type " +
							      ext.getVariableType());

			addInterfaceVariable(name, isInOut, typ, ext, true);
		    }
	    }
    }

    protected PlexilInterfaceVariableName addInterfaceVariable(String name,
							       boolean isInOut,
							       PlexilDataType typ,
							       PlexilVariableName original,
							       boolean isDeclared)
	throws antlr.SemanticException
    {
	PlexilInterfaceVariableName result = null;
	if (isLibraryNode())
	    {
		result = 
		    new PlexilInterfaceVariableName(name, isInOut, typ);
	    }
	else if (original == null)
	    {
		// no such variable
		throw new antlr.SemanticException("addInterfaceVariable: Interface variable '"
						  + name
						  + "' unknown");
	    }
	else
	    {
		result = 
		    new PlexilInterfaceVariableName(name, isInOut, original, isDeclared);
	    }
	variables.add(result);
	if (plexil.Parse.debugWriter != null)
	    try 
		{
		    plexil.Parse.debugWriter.write("Added ");
		    if (isDeclared)
			plexil.Parse.debugWriter.write("explicit ");
		    else
			plexil.Parse.debugWriter.write("implicit ");
		    if (isInOut)
			plexil.Parse.debugWriter.write("inOut");
		    else
			plexil.Parse.debugWriter.write("in");
		    plexil.Parse.debugWriter.write(" interface variable ");
		    plexil.Parse.debugWriter.write(name);
		    plexil.Parse.debugWriter.write(" of type ");
		    plexil.Parse.debugWriter.write(result.getVariableType().typeName());
		    plexil.Parse.debugWriter.write(" to node ");
		    if (nodeName == null)
			plexil.Parse.debugWriter.write("(unnamed)");
		    else
			plexil.Parse.debugWriter.write(nodeName);
		    plexil.Parse.debugWriter.write('\n');		
		}
	    catch (Exception e)
		{
		    System.out.println("Exception " + e.getClass() +
				       " caught while writing to debug file");
		}
	return result;
    }

    // Caller is responsible for creating the 3 vectors
    public void getNodeVariables(Vector<PlexilVariableName> localVarsResult,
				 Vector<PlexilInterfaceVariableName> inVarsResult,
				 Vector<PlexilInterfaceVariableName> inOutVarsResult)
    {
	localVarsResult.removeAllElements();
	inVarsResult.removeAllElements();
	inOutVarsResult.removeAllElements();
	for (Iterator<PlexilVariableName> varIt = variables.iterator(); varIt.hasNext(); )
	    {
		PlexilVariableName var = varIt.next();
		if (var.isLocal())
		    {
			localVarsResult.add(var);
		    }
		else
		    {
			if (var.isAssignable())
			    {
				inOutVarsResult.add((PlexilInterfaceVariableName) var);
			    }
			else
			    {
				inVarsResult.add((PlexilInterfaceVariableName) var);
			    }
		    }
	    }
    }

    // Variant for surface parser, where we don't need to know initial value

    public void addVariableName(String name, 
				PlexilDataType varType)
	throws antlr.SemanticException
    {
	addVariableName(name, varType, null);
    }

    public void addVariableName(String name, 
				PlexilDataType varType,
				Object initialValue)
	throws antlr.SemanticException
    {
        checkVariableName(name); // for effect
	variables.add(new PlexilVariableName(name, varType, initialValue));
	if (plexil.Parse.debugWriter != null)
	    try {
                plexil.Parse.debugWriter.write("Added variable ");
                plexil.Parse.debugWriter.write(name);
                plexil.Parse.debugWriter.write(" of type ");
                plexil.Parse.debugWriter.write(varType.typeName());
                if (initialValue != null) {
                    plexil.Parse.debugWriter.write(" with initial value ");
                    if (initialValue instanceof String)
                        plexil.Parse.debugWriter.write((String) initialValue);
                }
                plexil.Parse.debugWriter.write(" to node ");
                if (nodeName == null)
                    plexil.Parse.debugWriter.write("(unnamed)");
                else
                    plexil.Parse.debugWriter.write(nodeName);
                plexil.Parse.debugWriter.write('\n');		
            }
	    catch (Exception e) {
                System.out.println("Exception " + e.getClass() +
                                   " caught while writing to debug file");
            }
    }

    //
    // Array variables
    //

    public void addArrayVariableName(String name, 
                                     PlexilDataType elementType,
                                     String maxSize)
	throws antlr.SemanticException
    {
	addArrayVariableName(name, elementType, maxSize, null);
    }

    public void addArrayVariableName(String name, 
                                     PlexilDataType elementType,
                                     String maxSize,
                                     Object initialValue)
	throws antlr.SemanticException
    {
        checkVariableName(name); // for effect
        PlexilDataType arrayType = elementType.arrayType();
        if (arrayType == null)
            throw new antlr.SemanticException("Array variable "
                                              + name
                                              + ": Element type "
                                              + elementType.typeName()
                                              + " has no associated array type");
	variables.add(new PlexilVariableName(name, arrayType, maxSize, initialValue));
	if (plexil.Parse.debugWriter != null)
	    try {
                plexil.Parse.debugWriter.write("Added array variable ");
                plexil.Parse.debugWriter.write(name);
                plexil.Parse.debugWriter.write(" of element type ");
                plexil.Parse.debugWriter.write(elementType.typeName());
                plexil.Parse.debugWriter.write(" and size ");
                plexil.Parse.debugWriter.write(maxSize);
                if (initialValue != null) {
                    plexil.Parse.debugWriter.write(" with initial value(s) ");
                    if (initialValue instanceof String)
                        plexil.Parse.debugWriter.write((String) initialValue);
                }
                plexil.Parse.debugWriter.write(" to node ");
                if (nodeName == null)
                    plexil.Parse.debugWriter.write("(unnamed)");
                else
                    plexil.Parse.debugWriter.write(nodeName);
                plexil.Parse.debugWriter.write('\n');		
            }
	    catch (Exception e) {
                System.out.println("Exception " + e.getClass() +
                                   " caught while writing to debug file");
            }
    }

    //
    // Called for effect in addVariableName
    //

    protected void checkVariableName(String name)
        throws antlr.SemanticException
    {
	PlexilVariableName existing = findLocalVariable(name);
	if (existing != null)
	    // error - duplicate variable name in node
	    {
		if (existing.isLocal())
		    // duplicate local def'n
		    throw new antlr.SemanticException("Variable '" + name
						      + "' has already been declared locally");
		else
		    // overriding explicit interface def'n
		    throw new antlr.SemanticException("Local variable '" + name
						      + "' has same name as a declared interface variable");
	    }
	if (parentContext != null)
	    {
		PlexilVariableName shadowedVar =
		    parentContext.findInheritedVariable(name);
		if (shadowedVar != null)
		    // warn of conflict
		    // *** to do: allow user to turn off warning 
		    System.out.println("Local variable '" + name
				       + "' has same name as a declared interface variable");
	    }
    }

    protected PlexilVariableName findLocalVariable(String name)
    {
        for (Iterator<PlexilVariableName> iter = variables.iterator() ; iter.hasNext(); ) {
            PlexilVariableName candidate = iter.next();
            if (candidate.getName().equals(name))
                return candidate;
        }
        return null; 
    }

    // Look up an inherited variable with the given name.  
    // Returns the first instance found up the tree.
    // If none exists, return null.

    protected PlexilVariableName findInheritedVariable(String name)
    {
	if (parentContext == null)
	    return null;
	PlexilVariableName vn = parentContext.findLocalVariable(name);
	if (vn != null)
	    return vn;
	else
	    return parentContext.findInheritedVariable(name);
    }

    public PlexilVariableName findVariable(String name)
    {
	PlexilVariableName result = findLocalVariable(name);
	if (result != null)
	    return result;
	return findInheritedVariable(name);
    }

    // May return either a PlexilVariableName or PlexilInterfaceVariableName instance.

    // As a side effect, may create a local PlexilInterfaceVariableName
    // to represent a new implicit interface variable.

    protected PlexilVariableName getVariable(String name)
	throws antlr.SemanticException
    {
	return getVariable(name, true); // InOut by default
    }

    protected PlexilVariableName getVariable(String name, boolean isInOut)
	throws antlr.SemanticException
    {
	PlexilVariableName result = findLocalVariable(name);
	if (result != null)
	    {
		if (!result.isLocal())
		    {
			// Interface variable
			if (!result.isAssignable() && isInOut)
			    {
				// coerce to assignable
				PlexilInterfaceVariableName ifResult = 
				    (PlexilInterfaceVariableName) result;
				// N.B. can throw exception here
				ifResult.makeAssignable();
			    }
		    }
		if (plexil.Parse.debugWriter != null)
		    try 
			{
			    plexil.Parse.debugWriter.write("getVariable: Returning local variable '");
			    plexil.Parse.debugWriter.write(result.getName());
			    plexil.Parse.debugWriter.write("' from ");
			    if (nodeName == null)
				plexil.Parse.debugWriter.write("anonymous node\n");
			    else
				{
				    plexil.Parse.debugWriter.write("node '");
				    plexil.Parse.debugWriter.write(nodeName);
				    plexil.Parse.debugWriter.write("'\n");
				}
			}
		    catch (Exception e)
			{
			    System.out.println("Exception " + e.getClass() +
					       " caught while writing to debug file");
			}
		return result;
	    }
	// If some interface variable is in scope,
	// create a local implicit InOut interface declaration for it. 
	PlexilVariableName ifvar = getInheritedVariable(name, isInOut);
	if (ifvar == null)
	    {
		if (plexil.Parse.debugWriter != null)
		    try 
			{
			    plexil.Parse.debugWriter.write("getVariable: Returning null from ");
			    if (nodeName == null)
				plexil.Parse.debugWriter.write("anonymous node\n");
			    else
				{
				    plexil.Parse.debugWriter.write("node '");
				    plexil.Parse.debugWriter.write(nodeName);
				    plexil.Parse.debugWriter.write("'\n");
				}
			}
		    catch (Exception e)
			{
			    System.out.println("Exception " + e.getClass() +
					       " caught while writing to debug file");
			}
		return null;
	    }
	result = addInterfaceVariable(name, true, ifvar.getVariableType(), ifvar, false);
	if (plexil.Parse.debugWriter != null)
	    try 
		{
		    plexil.Parse.debugWriter.write("getVariable: Returning new interface variable '");
		    plexil.Parse.debugWriter.write(result.getName());
		    plexil.Parse.debugWriter.write("' from ");
		    if (nodeName == null)
			plexil.Parse.debugWriter.write("anonymous node\n");
		    else
			{
			    plexil.Parse.debugWriter.write("node '");
			    plexil.Parse.debugWriter.write(nodeName);
			    plexil.Parse.debugWriter.write("'\n");
			}
		}
	    catch (Exception e)
		{
		    System.out.println("Exception " + e.getClass() +
				       " caught while writing to debug file");
		}
	return result;
    }

    // Get the inherited variable and create any implicit declarations.
    protected PlexilVariableName getInheritedVariable(String name, boolean isInOut)
	throws antlr.SemanticException
    {
	// if this is root context,
	// there can't be any implicit interface variables.
	if (parentContext == null)
	    return null;
	// can create a local implicit interface declaration for the parent
	PlexilVariableName ifvar = parentContext.getVariable(name, isInOut);
	return ifvar;
    }

    //
    // Simple queries w/o side effects
    //

    public boolean isVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null) ;
    }

    public boolean isArrayVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null) && vn.isArray();
    }

    public boolean isAssignableVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null) && vn.isAssignable() ;
    }

    public boolean isBooleanVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return
	    (vn != null)
	    && (vn.getVariableType() == PlexilDataType.BOOLEAN_TYPE);
    }

    public boolean isIntegerVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return
	    (vn != null)
	    && (vn.getVariableType() == PlexilDataType.INTEGER_TYPE);
    }

    public boolean isBlobVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
	    && (vn.getVariableType() == PlexilDataType.BLOB_TYPE);
    }

    public boolean isRealVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
	    && (vn.getVariableType() == PlexilDataType.REAL_TYPE);
    }

    public boolean isStringVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
	    && (vn.getVariableType() == PlexilDataType.STRING_TYPE);
    }

    public boolean isTimeVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
	    && (vn.getVariableType() == PlexilDataType.TIME_TYPE);
    }

    public boolean isBooleanArrayVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
            && vn.isArray()
            && (vn.getArrayElementType() == PlexilDataType.BOOLEAN_TYPE);
    }

    public boolean isIntegerArrayVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
            && vn.isArray()
            && (vn.getArrayElementType() == PlexilDataType.INTEGER_TYPE);
    }

    public boolean isBlobArrayVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
            && vn.isArray()
            && (vn.getArrayElementType() == PlexilDataType.BLOB_TYPE);
    }

    public boolean isRealArrayVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
            && vn.isArray()
            && (vn.getArrayElementType() == PlexilDataType.REAL_TYPE);
    }

    public boolean isStringArrayVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
            && vn.isArray()
            && (vn.getArrayElementType() == PlexilDataType.STRING_TYPE);
    }

    public boolean isTimeArrayVariableName(String name)
    {
	PlexilVariableName vn = findVariable(name);
	return (vn != null)
            && vn.isArray()
            && (vn.getArrayElementType() == PlexilDataType.TIME_TYPE);
    }

    public boolean isBooleanInterfaceVariable(String name)
    {
	PlexilVariableName vn = findInheritedVariable(name);
	return
	    (vn != null)
	    && (vn.getVariableType() == PlexilDataType.BOOLEAN_TYPE);
    }

    public boolean isIntegerInterfaceVariable(String name)
    {
	PlexilVariableName vn = findInheritedVariable(name);
	return
	    (vn != null)
	    && (vn.getVariableType() == PlexilDataType.INTEGER_TYPE);
    }

    public boolean isBlobInterfaceVariable(String name)
    {
	PlexilVariableName vn = findInheritedVariable(name);
	return (vn != null)
	    && (vn.getVariableType() == PlexilDataType.BLOB_TYPE);
    }

    public boolean isRealInterfaceVariable(String name)
    {
	PlexilVariableName vn = findInheritedVariable(name);
	return (vn != null)
	    && (vn.getVariableType() == PlexilDataType.REAL_TYPE);
    }

    public boolean isStringInterfaceVariable(String name)
    {
	PlexilVariableName vn = findInheritedVariable(name);
	return (vn != null)
	    && (vn.getVariableType() == PlexilDataType.STRING_TYPE);
    }

    public boolean isTimeInterfaceVariable(String name)
    {
	PlexilVariableName vn = findInheritedVariable(name);
	return (vn != null)
	    && (vn.getVariableType() == PlexilDataType.TIME_TYPE);
    }

}
