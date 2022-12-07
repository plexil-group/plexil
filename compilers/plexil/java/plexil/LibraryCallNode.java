// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class LibraryCallNode extends NodeTreeNode
{
    private GlobalDeclaration m_libDecl = null;
    
    public LibraryCallNode(Token t)
    {
        super(t);
    }

    public LibraryCallNode(LibraryCallNode n)
    {
        super(n);
        m_libDecl = n.m_libDecl;
    }

    @Override
    public Tree dupNode()
    {
        return new LibraryCallNode(this);
    }

    //
    // format is:
    // (LIBRARY_CALL_KYWD NCNAME (ALIAS NCNAME expression)* )
    //

    @Override
    protected void earlyCheckSelf(NodeContext parentContext, CompilerState state)
    {
        super.earlyCheckSelf(parentContext, state); // NodeTreeNode method

        // Add library node as child so conditions in containing BlockNode
        // can reference it
        m_context.addChildNodeId(this.getChild(0));

        // Check that library node is declared
        String libName = this.getChild(0).getText();
        m_libDecl = state.getGlobalContext().getLibraryNodeDeclaration(libName);
        if (m_libDecl == null) {
            state.addDiagnostic(this.getChild(0),
                                "Library node \"" + libName + "\" is not declared",
                                Severity.ERROR);
        }
    }

    @Override
    protected void earlyCheckChildren(NodeContext context, CompilerState state)
    {
        if (this.getChildCount() > 1) {
            // Check aliases
            List<String> aliasNames = new ArrayList<String>();
            for (PlexilTreeNode alias : this.getChildren()) {
                // Skip over library name
                if (alias.getType() == PlexilLexer.NCNAME)
                    continue;

                // (ALIAS NCNAME expression)
                // Check for duplicate names
                String aliasName = alias.getChild(0).getText();
                if (aliasNames.contains(aliasName)) {
                    state.addDiagnostic(alias.getChild(0),
                                        "Alias name \"" + aliasName +
                                        "\" occurs multiple times in LibraryCall",
                                        Severity.ERROR);
                }
                else {
                    aliasNames.add(aliasName);
                }
                // Early checks on expression
                alias.getChild(1).earlyCheck(m_context, state);
            }
        }

        // Checks to run on parameters
        if (m_libDecl != null)
            earlyCheckParameters(state);
    }

    private void earlyCheckParameters(CompilerState state)
    {
        // Check number and in/out status of aliases
        List<VariableName> paramSpecs = m_libDecl.getParameterVariables();
        int argument_count = this.getChildCount() - 1;
        if (paramSpecs == null) {
            if (argument_count > 0) {
                state.addDiagnostic(this.getChild(1),
                                    "Library node \"" + m_libDecl.getName()
                                    + "\" expects 0 arguments, but "
                                    + Integer.toString(argument_count)
                                    + " were supplied",
                                    Severity.ERROR);
            }
        }
        else if (argument_count == 0) {
            // Count parameters w/o default values
            int minArgs = 0;
            for (VariableName v : paramSpecs)
                if (v.getInitialValue() == null)
                    minArgs++;
            if (minArgs > 0) {
                state.addDiagnostic(this,
                                    "Library node \"" + m_libDecl.getName()
                                    + "\" expects at least "
                                    + Integer.toString(minArgs) 
                                    + " arguments, but 0 were supplied",
                                    Severity.ERROR);
            }
        }
        else {
            // Match up aliases with parameters
            // Do type and writable variable checking in check() below
            Set<String> used = new TreeSet<String>();
            for (PlexilTreeNode alias : this.getChildren()) {
                // Skip over library name
                if (alias.getType() == PlexilLexer.NCNAME)
                    continue;

                String paramName = alias.getChild(0).getText();
                VariableName param = m_libDecl.getParameterByName(paramName);

                if (param == null) {
                    state.addDiagnostic(alias.getChild(0),
                                        "Library node \"" + m_libDecl.getName()
                                        + "\" has no parameter named \""
                                        + paramName + "\"",
                                        Severity.ERROR);
                    continue;
                }
                used.add(paramName);
                if (param.isAssignable()) {
                    if (!((ExpressionNode) alias.getChild(1)).isAssignable()) {
                        state.addDiagnostic(alias,
                                            "Library node parameter \"" + paramName
                                            + "\" is declared InOut, but is aliased to a read-only expression",
                                            Severity.ERROR);
                    }
                }
            }

            // See if any required params went unused
            Set<String> required = new TreeSet<String>();
            for (VariableName v : paramSpecs) 
                if (!used.contains(v.getName()) && v.getInitialValue() != null)
                    required.add(v.getName());
            if (required.size() != 0) {
                String missingNames = null;
                for (String s : required)
                    if (missingNames == null)
                        missingNames = s;
                    else
                        missingNames = missingNames + ", " + s;
                state.addDiagnostic(this,
                                    "Library node \"" + m_libDecl.getName()
                                    + "\" required paramater(s) "
                                    + missingNames + " were not supplied",
                                    Severity.ERROR);
            }
        }
    }
                                      

    @Override
    protected void checkChildren(NodeContext parentContext, CompilerState state)
    {
        // Check alias expressions, if provided
        for (PlexilTreeNode alias : this.getChildren()) {
            // Skip over library name
            if (alias.getType() == PlexilLexer.NCNAME)
                continue;

            alias.getChild(1).check(m_context, state);
        }

        if (m_libDecl != null) {
            List<VariableName> paramSpecs = m_libDecl.getParameterVariables();
            if (paramSpecs != null && this.getChildCount() > 1) {
                // Check types of supplied aliases
                for (PlexilTreeNode alias : this.getChildren()) {
                    // Skip over library name
                    if (alias.getType() == PlexilLexer.NCNAME)
                        continue;

                    VariableName param = m_libDecl.getParameterByName(alias.getChild(0).getText());
                    ExpressionNode valueExp = (ExpressionNode) alias.getChild(1);
                    if (param != null) // error already reported in earlyCheck
                        valueExp.assumeType(param.getVariableType(), state); // for effect
                }
            }
        }
    }

    @Override
    protected void constructXML(Document root)
    {
        this.constructXMLBase(root);
        m_xml.setAttribute("NodeType", "LibraryNodeCall");
        Element bodyXML = root.createElement("NodeBody");
        m_xml.appendChild(bodyXML);
        Element callXML = root.createElement("LibraryNodeCall");
        callXML.setAttribute("LineNo", String.valueOf(this.getLine()));
        callXML.setAttribute("ColNo", String.valueOf(this.getCharPositionInLine()));
        bodyXML.appendChild(callXML);
        Element aliasXML = null;
        for (PlexilTreeNode child : this.getChildren()) {
            if (child.getType() == PlexilLexer.NCNAME) {
                // process node ID
                Element idXML = root.createElement("NodeId");
                idXML.appendChild(root.createTextNode(this.getChild(0).getText()));
                callXML.appendChild(idXML);
            }
            else {
                // process an alias
                aliasXML = root.createElement("Alias");
                Element nameXML = root.createElement("NodeParameter");
                nameXML.appendChild(root.createTextNode(child.getChild(0).getText()));
                aliasXML.appendChild(nameXML);
                aliasXML.appendChild(child.getChild(1).getXML(root));
                callXML.appendChild(aliasXML);
            }
        }
    }

    @Override
    protected String getXMLElementName()
    {
        return "Node";
    }

}
