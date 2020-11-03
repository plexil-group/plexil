// Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

import org.w3c.dom.Element;

public class CommandNode extends ExpressionNode
{
    private GlobalDeclaration m_commandDeclaration = null;
    private ArgumentListNode m_parameters = null;

    public CommandNode(int ttype)
    {
        super(new CommonToken(ttype, "COMMAND"));
    }

    public CommandNode(CommandNode n)
    {
        super(n);
		m_commandDeclaration = n.m_commandDeclaration;
		m_parameters = n.m_parameters;
    }

	public Tree dupNode()
	{
		return new CommandNode(this);
	}

    // N.B. Only valid after earlyCheckSelf()
    public GlobalDeclaration getCommand()
    {
        return m_commandDeclaration;
    }

    // AST is:
    // (COMMAND ((COMMAND_KYWD NCNAME) | expression) (ARGUMENT_LIST expression*)?)

    @Override
    protected void earlyCheckSelf(NodeContext context, CompilerState state)
    {
        PlexilTreeNode nameAST = this.getChild(0);
        if (this.getChildCount() > 1)
            m_parameters = (ArgumentListNode) this.getChild(1);

        if (nameAST.getType() == PlexilLexer.COMMAND_KYWD) {
            // Literal command name - 
            // Check that name is defined as a command
            PlexilTreeNode nameNode = nameAST.getChild(0);
            String name = nameNode.getText();
            // Set source locators
            this.getToken().setLine(nameNode.getLine());
            this.getToken().setCharPositionInLine(nameNode.getCharPositionInLine());

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
                Vector<VariableName> parmSpecs = m_commandDeclaration.getParameterVariables();
                if (parmSpecs == null) {
                    // No parameters expected
                    if (m_parameters != null) {
                        state.addDiagnostic(m_parameters,
                                            "Command \"" + name + "\" expects 0 parameters, but "
                                            + String.valueOf(m_parameters.getChildCount() + " were supplied"),
                                            Severity.ERROR);
                    }
                }
                else {
                    // No parameters given
                    if (m_parameters == null) {

                        // No parameters required, do nothing
                        if(parmSpecs.size() == 1 &&
                           parmSpecs.elementAt(0) instanceof WildcardVariableName){

                        }

                         else{
                            state.addDiagnostic(nameNode,
                                                "Command \"" + name + "\" expects "
                                                + String.valueOf(parmSpecs.size()) + " parameters, but none were supplied",
                                                Severity.ERROR);
                        }
                    }
                    else {
                        m_parameters.earlyCheckArgumentList(context,
                                                            state,
                                                            "Command",
                                                            name,
                                                            parmSpecs);
                    }
                    // Parameter type checking done in checkTypeConsistency() below
                }
                // Resource list is handled by BlockNode
            }
        }
        else {
            // Set source locators to expression
            this.getToken().setLine(getChild(0).getLine());
            this.getToken().setCharPositionInLine(getChild(0).getCharPositionInLine());
        }
    }

    @Override
    protected void checkSelf(NodeContext context, CompilerState state)
    {
        PlexilTreeNode nameAST = this.getChild(0);
        if (nameAST.getType() != PlexilLexer.COMMAND_KYWD) {
            // if name is not literal, 
            // check that name expression returns a string
            ExpressionNode nameExp = (ExpressionNode) nameAST;
            if (!nameExp.assumeType(PlexilDataType.STRING_TYPE, state)) {
                state.addDiagnostic(nameExp,
                                    "Command name expression is not a string expression",
                                    Severity.ERROR);
            }
        }

        if (m_commandDeclaration != null) {
            // Check parameter list against declaration
            String cmdName = m_commandDeclaration.getName();
            Vector<VariableName> parmSpecs = m_commandDeclaration.getParameterVariables();
            if (parmSpecs != null && m_parameters != null)
                m_parameters.checkArgumentList(context, state, "command", cmdName, parmSpecs);
        }
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

    @Override
    protected void constructXML()
    {
        // construct Node XML
        super.constructXMLBase();
        m_xml.setAttribute("NodeType", "Command");

        // construct node body
        Element nodeBody = CompilerState.newElement("NodeBody");
        m_xml.appendChild(nodeBody);

        Element commandBody = CompilerState.newElement("Command");
        // set source location to the loc'n of the command name (expression)
        commandBody.setAttribute("LineNo", String.valueOf(this.getChild(0).getLine()));
        commandBody.setAttribute("ColNo", String.valueOf(this.getChild(0).getCharPositionInLine()));

        nodeBody.appendChild(commandBody);

        // BlockNode handles resource list

        // Add name (expression)
        PlexilTreeNode commandName = this.getChild(0);
        Element nameXML = CompilerState.newElement("Name");
        commandBody.appendChild(nameXML);
        if (commandName.getType() == PlexilLexer.COMMAND_KYWD) {
            // Literal command name
            Element stringVal = CompilerState.newElement("StringValue");
            stringVal.appendChild(CompilerState.newTextNode(commandName.getChild(0).getText()));
            nameXML.appendChild(stringVal);
        }
        else {
            // Command name expression
            nameXML.appendChild(commandName.getXML());
        }

        // Handle parameters
        if (m_parameters != null) {
            commandBody.appendChild(m_parameters.getXML());
        }
    }

    protected String getXMLElementName() { return "Node"; }

    // TODO: extend to return true for constant string expressions
    private boolean isCommandNameLiteral()
    {
        return this.getChild(0).getType() == PlexilLexer.COMMAND_KYWD;
    }

}
