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

import java.util.Vector;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import net.n3.nanoxml.*;

public class CommandNode extends ExpressionNode
{
	public CommandNode(int ttype)
	{
		super(new CommonToken(ttype, "COMMAND"));
	}

	// AST is:
	// (COMMAND ((COMMAND_NAME NCNAME) | expression) (ARGUMENT_LIST expression*)?)

	public boolean checkSelf(NodeContext context, CompilerState myState)
	{
		boolean success = true;

		// if name is known:
		if (isCommandNameLiteral()) {
			// Check that name is defined as a command
		    PlexilTreeNode nameNode = this.getChild(0).getChild(0);
			String name = nameNode.getText();
			if (!GlobalContext.getGlobalContext().isCommandName(name)) {
				myState.addDiagnostic(nameNode,
									  "Command \"" + name + "\" is not defined",
									  Severity.ERROR);
				return false;
			}

			// Check parameter list against declaration
			GlobalDeclaration cmdDecl = GlobalContext.getGlobalContext().getCommandDeclaration(name);
			Vector<PlexilDataType> parmTypes = cmdDecl.getParameterTypes();
			PlexilTreeNode parmSpecs = this.getParameters();
			if (parmTypes == null) {
				// No parameters expected
				if (parmSpecs != null) {
					myState.addDiagnostic(parmSpecs,
										  "Command \"" + name + "\" expects 0 parameters, but "
										  + String.valueOf(parmSpecs.getChildCount() + " were supplied"),
										  Severity.ERROR);
					success = false;
				}
			}
			else {
				// Parameters expected
				if (parmSpecs == null) {
					// None supplied
					myState.addDiagnostic(nameNode,
										  "Command \"" + name + "\" expects "
										  + String.valueOf(parmTypes.size()) + " parameters, but none were supplied",
										  Severity.ERROR);
					success = false;
				}
				else if (parmSpecs.getChildCount() != parmTypes.size()) {
					// Wrong number supplied
					myState.addDiagnostic(nameNode,
										  "Command \"" + name + "\" expects "
										  + String.valueOf(parmTypes.size()) + " parameters, but "
										  + String.valueOf(parmSpecs.getChildCount()) + " were supplied",
										  Severity.ERROR);
					success = false;
				}
				else {
					// TODO: Check supplied expression types against declared
				}
			}

			// Set return type 
			PlexilDataType retnType = cmdDecl.getReturnType();
			if (retnType != null)
				m_dataType = retnType;

			// TODO: Check resource list
			
		}
		else {
			// Check that name expression returns a string
			
		}

		return success;
	}

	/**
	 * @brief Check the expression for type consistency.
	 * @return true if consistent, false otherwise.
	 */
	protected boolean checkTypeConsistency(NodeContext context, CompilerState myState)
	{
		boolean success = true;
		return success;
	}

	public void constructXML()
	{
		// construct Node XML
		super.constructXML();
		m_xml.setAttribute("NodeType", "Command");
		// set source location to the loc'n of the command name (expression)
		m_xml.setAttribute("LineNo", String.valueOf(this.getChild(0).getLine()));
		m_xml.setAttribute("ColNo", String.valueOf(this.getChild(0).getCharPositionInLine()));

		// construct node body
		IXMLElement nodeBody = new XMLElement("NodeBody");
		m_xml.addChild(nodeBody);

		IXMLElement commandBody = new XMLElement("Command");
		nodeBody.addChild(commandBody);

		// TODO: handle resource list

		// TODO: handle return value

		// Add name (expression)
		PlexilTreeNode commandName = this.getChild(0);
		IXMLElement nameXML = new XMLElement("Name");
		commandBody.addChild(nameXML);
		if (commandName.getType() == PlexilLexer.COMMAND_KYWD) {
			// Literal command name
			IXMLElement stringVal = new XMLElement("StringValue");
			stringVal.setContent(commandName.getChild(0).getText());
			nameXML.addChild(stringVal);
		}
		else {
			// TODO: Command name expression
		}

		// TODO: handle parameters
	}

	protected String getXMLElementName() { return "Node"; }

	// TODO: extend to return true for constant string expressions
	private boolean isCommandNameLiteral()
	{
		return this.getChild(0).getType() == PlexilLexer.COMMAND_KYWD;
	}

	private PlexilTreeNode getParameters()
	{
		if (this.getChildCount() < 2)
			return null;
		return this.getChild(1);
	}

	// TODO: Implement!
	private PlexilTreeNode getReturnVariable()
	{
		return null;
	}

}