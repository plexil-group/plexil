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

// *** To do:
//  - add array support

package plexil;

import java.util.Vector;
import java.util.Iterator;

import antlr.collections.AST;

import net.n3.nanoxml.*;

import plexil.PlexilDataType;
import plexil.PlexilName;
import plexil.PlexilTokenTypes;

public class PlexilGlobalDeclaration extends PlexilName
{
    protected PlexilNameType declarationType;
    protected Vector<PlexilVariableName> paramSpecs;
    protected Vector<PlexilVariableName> returnSpecs;

    public PlexilGlobalDeclaration(String myName,
				   PlexilNameType declType,
				   Vector<PlexilVariableName> parms,
				   Vector<PlexilVariableName> returns)
    {
	super(myName, declType);
	declarationType = declType;
	paramSpecs = parms;
        returnSpecs = returns;
	if (plexil.Parse.debugWriter != null)
	    try 
		{
		    plexil.Parse.debugWriter.write("Constructed global declaration '");
		    plexil.Parse.debugWriter.write(myName);
    		    plexil.Parse.debugWriter.write("' of type ");
		    plexil.Parse.debugWriter.write(declarationType.name());
		}
	    catch (Exception e)
		{
		    System.out.println("Exception " + e.getClass() +
				       " caught while writing to debug file");
		}
    }

    public PlexilGlobalDeclaration(String myName,
				   PlexilNameType declType,
				   AST parmAST,
				   AST returnAST)
    {
	super(myName, declType);
	declarationType = declType;
	paramSpecs = null;
	if (parmAST != null)
	    {
		paramSpecs = new Vector<PlexilVariableName>();
		AST parm = parmAST.getFirstChild();
		int parmIdx = 1;
		while (parm != null)
		    {
			AST typeName = parm.getFirstChild();
			AST parmName = typeName.getNextSibling();
			String nam = (parmName == null) ? ("_" + declarationType.plexilName() + "_param_" + parmIdx)
                                                        : parmName.getText();
			PlexilVariableName newParmVar = null;
			switch (parm.getType())
			    {
			    case PlexilTokenTypes.PARAMETER:
				newParmVar = new PlexilVariableName(nam,
								    PlexilDataType.findByName(typeName.getText()));
				break;
				
			    case PlexilTokenTypes.IN_KYWD:
				newParmVar = new PlexilInterfaceVariableName(nam,
									     false,
									     PlexilDataType.findByName(typeName.getText()));
				break;

			    case PlexilTokenTypes.IN_OUT_KYWD:
				newParmVar = new PlexilInterfaceVariableName(nam,
									     true,
									     PlexilDataType.findByName(typeName.getText()));
				break;

			    default:
				System.out.println("Invalid parameter descriptor type " + parm.getType());
				break;
			    }

			paramSpecs.add(newParmVar);
			parm = parm.getNextSibling();
			parmIdx++;
		    }
	    }
	returnSpecs = null;
	if (returnAST != null)
	    {
		returnSpecs = new Vector<PlexilVariableName>();
		AST retn = returnAST.getFirstChild();
		int retnIdx = 1;
		while (retn != null)
		    {
			AST typeName = retn.getFirstChild();
			AST retnName = typeName.getNextSibling();
			String nam = (retnName == null) ? ("_" + declarationType.plexilName() + "_return_" + retnIdx)
			                                : retnName.getText();
			returnSpecs.add(new PlexilVariableName(nam,
							       PlexilDataType.findByName(typeName.getText())));
			retn = retn.getNextSibling();
		    }
	    }
	if (plexil.Parse.debugWriter != null)
	    try 
		{
		    plexil.Parse.debugWriter.write("Constructed global declaration '");
		    plexil.Parse.debugWriter.write(myName);
    		    plexil.Parse.debugWriter.write("' of type ");
		    plexil.Parse.debugWriter.write(declarationType.name());
		    plexil.Parse.debugWriter.write("\n");
		}
	    catch (Exception e)
		{
		    System.out.println("Exception " + e.getClass() +
				       " caught while writing to debug file");
		}
    }

    // returns first return type or null
    public PlexilDataType getReturnType()
    {
	if (returnSpecs == null)
	    return null;
	return returnSpecs.firstElement().getVariableType();
    }

    // returns vector of return types, or null
    public Vector<PlexilDataType> getReturnTypes()
    {
	if (returnSpecs == null)
	    return null;
	Vector<PlexilDataType> result = new Vector<PlexilDataType>();
	Iterator<PlexilVariableName> e = returnSpecs.iterator();
	while (e.hasNext())
	    result.add(e.next().getVariableType());
	return result;
    }

    // returns vector of return variables, or null
    public Vector<PlexilVariableName> getReturnVariables()
    {
	if (returnSpecs == null)
	    return null;
	Vector<PlexilVariableName> result = new Vector<PlexilVariableName>();
	Iterator<PlexilVariableName> e = returnSpecs.iterator();
	while (e.hasNext())
	    result.add(e.next());
	return result;
    }

    // returns vector of parameter types, or null
    public Vector<PlexilDataType> getParameterTypes()
    {
	if (returnSpecs == null)
	    return null;
	Vector<PlexilDataType> result = new Vector<PlexilDataType>();
	Iterator<PlexilVariableName> e = paramSpecs.iterator();
	while (e.hasNext())
	    result.add(e.next().getVariableType());
	return result;
    }

    // returns vector of parameter variables, or null
    public Vector<PlexilVariableName> getParameterVariables()
    {
	if (paramSpecs == null)
	    return null;
	Vector<PlexilVariableName> result = new Vector<PlexilVariableName>();
	Iterator<PlexilVariableName> e = paramSpecs.iterator();
	while (e.hasNext())
	    result.add(e.next());
	return result;
    }

    // returns parameter variable descriptor, or null
    public PlexilVariableName getParameterByName(String name)
    {
	if (paramSpecs == null)
	    return null;
	Iterator<PlexilVariableName> e = paramSpecs.iterator();
	while (e.hasNext())
	    {
		PlexilVariableName candidate = e.next();
		if (name.equals(candidate.getName()))
		    return candidate;
	    }
	return null;
    }

    // returns parameter variable descriptor, or null
    public boolean hasParameterNamed(String name)
    {
	return (getParameterByName(name) != null);
    }

    public IXMLElement getXml()
    {
	String declTypeName = getType().plexilName();
	IXMLElement result = new XMLElement(declTypeName + "Declaration");
	// name
	IXMLElement nameXml = new XMLElement("Name");
	nameXml.setContent(getName());
	result.addChild(nameXml);
	// return(s)
	if (returnSpecs != null)
	    for (Iterator<PlexilVariableName> returnIt = returnSpecs.iterator(); returnIt.hasNext(); )
		{
		    PlexilVariableName returnSpec = returnIt.next();
		    IXMLElement returnXml = returnSpec.makeGlobalDeclarationElement("Return");
		    result.addChild(returnXml);
		}

	// param(s)
	if (paramSpecs != null)
	    for (Iterator<PlexilVariableName> paramIt = paramSpecs.iterator(); paramIt.hasNext(); )
		{
		    PlexilVariableName paramSpec = paramIt.next();
		    IXMLElement paramXml = paramSpec.makeGlobalDeclarationElement("Parameter");
		    result.addChild(paramXml);
		}
	return result;
    }

}
