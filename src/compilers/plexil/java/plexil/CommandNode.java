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
	private GlobalDeclaration m_commandDeclaration = null;

	public CommandNode(int ttype)
	{
		super(new CommonToken(ttype, "COMMAND"));
	}

	// AST is:
	// (COMMAND ((COMMAND_NAME NCNAME) | expression) (ARGUMENT_LIST expression*)?)

	public void earlyCheckSelf(NodeContext context, CompilerState state)
	{
		PlexilTreeNode nameAST = this.getChild(0);
		if (nameAST.getType() == PlexilLexer.COMMAND_KYWD) {
			// Literal command name - 
			// Check that name is defined as a command
			PlexilTreeNode nameNode = nameAST.getChild(0);
			String name = nameNode.getText();
			if (!GlobalContext.getGlobalContext().isCommandName(name)) {
				state.addDiagnostic(nameNode,
									"Command \"" + name + "\" is not defined",
									Severity.ERROR);
			}

			// get declaration for type info
			m_commandDeclaration = GlobalContext.getGlobalContext().getCommandDeclaration(name);
			if (m_commandDeclaration == null) {
				// We can't make any assumptions about return type
				m_dataType = PlexilDataType.ANY_TYPE;
			}
			else {
				// We know this command
				// Set return type 
				PlexilDataType retnType = m_commandDeclaration.getReturnType();
				if (retnType != null)
					m_dataType = retnType;

				// We have a valid command declaration
				// Check parameter list
				Vector<PlexilDataType> parmTypes = m_commandDeclaration.getParameterTypes();
				PlexilTreeNode parmsAST = this.getParameters();
				if (parmTypes == null) {
					// No parameters expected
					if (parmsAST != null) {
						state.addDiagnostic(parmsAST,
											"Command \"" + name + "\" expects 0 parameters, but "
											+ String.valueOf(parmsAST.getChildCount() + " were supplied"),
											Severity.ERROR);
					}
				}
				else {
					// Parameters expected
					if (parmsAST == null) {
						// None supplied
						state.addDiagnostic(nameNode,
											"Command \"" + name + "\" expects "
											+ String.valueOf(parmTypes.size()) + " parameters, but none were supplied",
											Severity.ERROR);
					}
					else if (parmsAST.getChildCount() != parmTypes.size()) {
						// Wrong number supplied
						state.addDiagnostic(nameNode,
											"Command \"" + name + "\" expects "
											+ String.valueOf(parmTypes.size()) + " parameters, but "
											+ String.valueOf(parmsAST.getChildCount()) + " were supplied",
											Severity.ERROR);
					}
					// Parameter type checking done in checkTypeConsistency() below
				}

				// TODO: Check resource list (?)
			
			}
		}
	}

	public void check(NodeContext context, CompilerState state)
	{
		PlexilTreeNode nameAST = this.getChild(0);

		// if name is not literal
		if (nameAST.getType() != PlexilLexer.COMMAND_KYWD) {
 			// Check that name expression returns a string
			ExpressionNode nameExp = (ExpressionNode) nameAST;
			if (nameExp.getDataType() != PlexilDataType.STRING_TYPE) {
				state.addDiagnostic(nameExp,
									"Command name expression is not a string expression",
									Severity.ERROR);
			}
		}

		if (m_commandDeclaration != null) {
			// We have a valid command declaration
			// Check parameter list
			Vector<PlexilDataType> parmTypes = m_commandDeclaration.getParameterTypes();
			PlexilTreeNode parmsAST = this.getParameters();
			if (parmTypes != null
				&& parmsAST != null
				&& parmsAST.getChildCount() == parmTypes.size()) {
				// Check supplied expression types against declared
				for (int i = 0; i < parmsAST.getChildCount(); i++) {
					ExpressionNode parm = (ExpressionNode) parmsAST.getChild(i);
					if (!parm.assumeType(parmTypes.elementAt(i), state)) {
						state.addDiagnostic(parm,
											"Command parameter is not of expected type "
											+ parmTypes.elementAt(i).typeName(),
											Severity.ERROR);
					}
				}
			}
		}

		// TODO: Check resources

		// Perform recursive checks on subexprs
		this.checkChildren(context, state);
	}

	/**
	 * @brief Persuade the expression to assume the specified data type
	 * @return true if the expression can consistently assume the specified type, false otherwise.
	 */
	protected boolean assumeType(PlexilDataType t, CompilerState myState)
	{
		// If we have a known type already, the usual rules apply
		if (m_commandDeclaration != null)
			return super.assumeType(t, myState);
		// If not, take on whatever type the user expects
		m_dataType = t;
		return true;
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