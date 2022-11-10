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

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

import plexil.xml.DOMUtils;

public class SynchronousCommandNode extends NodeTreeNode
{
    private ExpressionNode m_timeout = null;
    private ExpressionNode m_tolerance = null;
    private boolean m_checked = false;
    
    public SynchronousCommandNode(Token t)
    {
        super(t);
        m_timeout = null;
        m_tolerance = null;
        m_checked = false;
    }

    public SynchronousCommandNode(SynchronousCommandNode n)
    {
        super(n);
        m_timeout = n.m_timeout;
        m_tolerance = n.m_tolerance;
        m_checked = n.m_checked;
    }

    public Tree dupNode()
    {
        return new SynchronousCommandNode(this);
    }

    //
    // Format is
    // (SYNCHRONOUS_COMMAND_KYWD (assignment | commandInvocation)
    //   ( CHECKED_KYWD | (timeout-expression tolerance-expression?) )*
    // The grammar allows Checked and Timeout options in either order,
    // but only one of each.
    //

    // Timeout and tolerance expressions can be cached here
    @Override
    protected void earlyCheckChildren(NodeContext parentContext, CompilerState state)
    {
        super.earlyCheckChildren(parentContext, state); // NodeTreeNode method
        int childCount = this.getChildCount();
        if (childCount > 1) {
            // Process options
            // Tolerance expression must directly follow timeout expression.
            int i = 1;
            boolean expectingTolerance = false;
            while (i < childCount) {
                PlexilTreeNode child = this.getChild(i);
                if (child.getType() == PlexilParser.CHECKED_KYWD) {
                    m_checked = true;
                    // Ensure that if timeout w/o tolerance has been seen,
                    // any additional expressions are treated as errors
                    if (m_timeout != null)
                        expectingTolerance = false;
                }
                // From here down, grammar ensures child is an expression
                else if (m_timeout == null) {
                    m_timeout = (ExpressionNode) child;
                    expectingTolerance = true;
                }
                else if (expectingTolerance) {
                    m_tolerance = (ExpressionNode) child;
                    expectingTolerance = false;
                }
                else {
                    state.addDiagnostic(child,
                                        "Internal error: unexpected expression in SynchronousCommand AST",
                                        Severity.FATAL);
                    return;
                }
                i++;
            }
        }
    }
    
    // TODO:
    //  Refactor common timeout functionality with WaitNode.java ?

    @Override
    protected void checkChildren(NodeContext parentContext, CompilerState state)
    {
        super.checkChildren(parentContext, state); // NodeTreeNode method

        // Options were partitioned in earlyCheckChildren()
        if (m_timeout != null) {
            if (m_timeout.assumeType(PlexilDataType.DURATION_TYPE, state)) {
                if (m_tolerance != null)
                    checkToleranceForDuration(state);
            }
            else if (m_timeout.assumeType(PlexilDataType.REAL_TYPE, state) ||
                     m_timeout.assumeType(PlexilDataType.INTEGER_TYPE, state)) {
                if (m_tolerance != null)
                    checkToleranceForReal(state);
            }
            else {
                state.addDiagnostic(m_timeout,
                                    "The timeout argument to SynchronousCommand, \""
                                    + m_timeout.getText()
                                    + "\", is not a Duration or number",
                                    Severity.ERROR);
            }
        }
    }

    private void checkToleranceForDuration(CompilerState state)
    {
        if (m_tolerance instanceof LiteralNode
            && m_tolerance.assumeType(PlexilDataType.DURATION_TYPE, state)) {
            // it's good
        }
        else if (m_tolerance instanceof VariableNode
                 // simple variable reference:
                 && m_tolerance.getType() == PlexilLexer.NCNAME 
                 && m_tolerance.getDataType() == PlexilDataType.DURATION_TYPE) {
            // that's good too
        }
        else {
            state.addDiagnostic(m_tolerance,
                                "The tolerance argument to SynchronousCommand, \""
                                + m_tolerance.getText()
                                + "\", is not a Duration value or variable.",
                                Severity.ERROR);
        }
    }

    private void checkToleranceForReal(CompilerState state)
    {
        if (m_tolerance instanceof LiteralNode
            && m_tolerance.assumeType(PlexilDataType.REAL_TYPE, state)) {
            // it's good
        }
        else if (m_tolerance instanceof VariableNode
                 // simple variable reference:
                 && m_tolerance.getType() == PlexilLexer.NCNAME 
                 && m_tolerance.getDataType() == PlexilDataType.REAL_TYPE) {
            // that's good too
        }
        else {
            state.addDiagnostic(m_tolerance,
                                "The tolerance argument to SynchronousCommand, \""
                                + m_tolerance.getText()
                                + "\", is not a Real value or variable.",
                                Severity.ERROR);
        }
    }

    @Override
    protected void constructXML(Document root)
    {
        super.constructXMLBase(root);

        // Generate XML for checked option if supplied
        if (m_checked) {
            m_xml.appendChild(root.createElement("Checked"));
        }

        // Generate XML for timeout if supplied
        if (m_timeout != null) {
            Element timeoutXML = root.createElement("Timeout");
            m_xml.appendChild(timeoutXML);
            timeoutXML.appendChild(m_timeout.getXML(root));
            if (m_tolerance != null) {
                Element tolXML = root.createElement("Tolerance");
                timeoutXML.appendChild(tolXML);
                tolXML.appendChild(m_tolerance.getXML(root));
            }
        }
        
        // Construct command XML by extracting from command node XML
        Element commandNodeXML = this.getChild(0).getXML(root);
        // command is inside NodeBody element
        Element commandXML =
            (Element) DOMUtils.getFirstElementNamed(commandNodeXML, "NodeBody").getFirstChild();
        m_xml.appendChild(commandXML);
    }

}
