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

class WaitNode extends NodeTreeNode
{
    public WaitNode(Token t)
    {
        super(t);
    }

    public WaitNode(WaitNode n)
    {
        super(n);
    }

    @Override
    public Tree dupNode()
    {
        return new WaitNode(this);
    }
    
    //  N.B. Refactor the following with SynchronousCommandNode.java!
    @Override
    protected void checkChildren(NodeContext parentContext, CompilerState state)
    {
        super.checkChildren(parentContext, state); // NodeTreeNode method

        ExpressionNode delayExp = (ExpressionNode) this.getChild(0);
        PlexilDataType delayType = delayExp.getDataType();
        if (delayType == PlexilDataType.DURATION_TYPE) {
            checkForDuration(state);
        }
        else if (delayType == PlexilDataType.REAL_TYPE
                 || delayType == PlexilDataType.INTEGER_TYPE) {
            checkForReal(state);
        }
        else state.addDiagnostic(delayExp,
                                 "The delay argument to the Wait builtin, \""
                                 + delayExp.getText()
                                 + "\", is not a Duration or number",
                                 Severity.ERROR);
    }
    
    private void checkForDuration(CompilerState state)
    {
        if (this.getChildCount() > 1) {
            ExpressionNode toleranceExp = (ExpressionNode) this.getChild(1);
            PlexilDataType toleranceType = toleranceExp.getDataType();
            if (toleranceType == PlexilDataType.DURATION_TYPE
                || toleranceType == PlexilDataType.REAL_TYPE
                || toleranceType == PlexilDataType.INTEGER_TYPE) {
                // it's good
            }
            else {
                state.addDiagnostic(toleranceExp,
                                    "The tolerance argument to the Wait builtin, \""
                                    + toleranceExp.getText()
                                    + "\", is not compatible with a Duration delay argument.",
                                    Severity.ERROR);
            }
        }
    }

    private void checkForReal(CompilerState state)
    {
        if (this.getChildCount() > 1) {
            ExpressionNode toleranceExp = (ExpressionNode) this.getChild(1);
            PlexilDataType toleranceType = toleranceExp.getDataType();
            if (toleranceType == PlexilDataType.REAL_TYPE
                || toleranceType == PlexilDataType.INTEGER_TYPE) {
                // it's good
            }
            else {
                state.addDiagnostic(toleranceExp,
                                    "The tolerance argument to the Wait builtin, \""
                                    + toleranceExp.getText()
                                    + "\", is not numeric.",
                                    Severity.ERROR);
            }
        }
    }

    @Override
    protected void constructXML(Document root)
    {
        super.constructXMLBase(root);
        Element unitsElt = root.createElement("Units");
        m_xml.appendChild(unitsElt);
        unitsElt.appendChild(this.getChild(0).getXML(root));
        if (this.getChildCount() > 1) {
            Element toleranceElt = root.createElement("Tolerance");
            m_xml.appendChild(toleranceElt);
            toleranceElt.appendChild(this.getChild(1).getXML(root));
        }
    }

    protected String getXMLElementName() { return "Wait"; }

}
