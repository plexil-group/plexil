// Copyright (c) 2006-2010, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package plexil;

import java.util.Set;
import java.util.TreeSet;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import net.n3.nanoxml.*;

public class CommandDeclarationNode extends PlexilTreeNode
{
	public CommandDeclarationNode(Token t)
	{
		super(t);
	}

	// structure is:
	// ^(COMMAND_KYWD NCNAME paramsSpec? returnsSpec? resourcesList?)

	public void earlyCheckSelf(NodeContext context, CompilerState myState)
	{
		// check that name is not already defined
		String cmdName = this.getChild(0).getText();
		if (GlobalContext.getGlobalContext().isCommandName(cmdName)) {
			// TODO: handle overloaded def'n
			// Report duplicate definition
			myState.addDiagnostic(this.getChild(0),
								  "Command \"" + cmdName + "\" is already defined",
								  Severity.ERROR);
		}

		// Define in global environment
		GlobalContext.getGlobalContext().addCommandName(this, cmdName, getParameters(), getReturn());
	}

	public void checkSelf(NodeContext context, CompilerState myState)
	{
		String cmdName = this.getChild(0).getText();

		// Check that parameters have no name conflicts
		PlexilTreeNode parms = getParameters();
		if (parms != null) {
			Set<String> names = new TreeSet<String>();
			for (int i = 0; i < parms.getChildCount(); i++) {
				PlexilTreeNode nameSpec = parms.getChild(i).getChild(0);
				// Parameter specs need not be named
				if (nameSpec != null) {
					String thisName = nameSpec.getText();
					if (names.contains(thisName)) {
						// Report duplicate name warning
						myState.addDiagnostic(nameSpec,
											  "Command \"" + cmdName + "\": Parameter name \"" + thisName + "\" is already defined",
											  Severity.WARNING);
					}
					else {
						names.add(thisName);
					}
				}
			}
		}

		// TODO: handle resource list
	}

	public void constructXML()
	{
		super.constructXML();

		// set source location to the loc'n of the Command keyword
		m_xml.setAttribute("LineNo", String.valueOf(this.getLine()));
		m_xml.setAttribute("ColNo", String.valueOf(this.getCharPositionInLine()));

		// add name
		PlexilTreeNode nameTree = this.getChild(0);
		IXMLElement nameXML = new XMLElement("Name");
		nameXML.setContent(nameTree.getText());
		m_xml.addChild(nameXML);

		if (this.getChildCount() > 1) {
			// Add return spec(s) if provided
			PlexilTreeNode returnSpec = getReturn();
			if (returnSpec != null) {
				for (int i = 0; i < returnSpec.getChildCount(); i++) {
					PlexilTreeNode thisSpec = returnSpec.getChild(i);
					IXMLElement returnXML = new XMLElement("Return");
					returnXML.setAttribute("Type", thisSpec.getToken().getText());
					returnXML.setAttribute("LineNo", String.valueOf(thisSpec.getLine()));
					returnXML.setAttribute("ColNo", String.valueOf(thisSpec.getCharPositionInLine()));
					if (thisSpec.getChildCount() > 0) {
						returnXML.setContent(thisSpec.getChild(0).getText());
					}
					m_xml.addChild(returnXML);
				}
			}

			// TODO: Add parameter spec(s) if provided
			PlexilTreeNode parametersSpec = getParameters();
			if (parametersSpec != null) {
				for (int i = 0; i < parametersSpec.getChildCount(); i++) {
					PlexilTreeNode thisSpec = parametersSpec.getChild(i);
					IXMLElement paramXML = new XMLElement("Parameter");
					paramXML.setAttribute("Type", thisSpec.getToken().getText());
					if (thisSpec.getChildCount() > 0) {
						paramXML.setContent(thisSpec.getChild(0).getText());
					}
					m_xml.addChild(paramXML);
				}
			}

			// TODO: add resource list if provided
			PlexilTreeNode resourceList = getResourceList();
			if (resourceList != null) {
			}
		}
	}

	public String getXMLElementName() { return "CommandDeclaration"; }

	protected PlexilTreeNode getParameters()
	{
		if (this.getChildCount() < 2)
			return null;
		if (this.getChild(1).getType() != PlexilLexer.PARAMETERS)
			return null;
		return this.getChild(1);
	}

	protected PlexilTreeNode getReturn()
	{
		if (this.getChildCount() < 2)
			return null;
		for (int i = 1; i < this.getChildCount(); i++) {
			if (this.getChild(i).getType() == PlexilLexer.RETURNS_KYWD)
				return this.getChild(i);
		}
		return null; // not found
	}

	protected PlexilTreeNode getResourceList()
	{
		// *** TODO: implement!
		return null;
	}

}
