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

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Element;

public class NodeVariableNode extends ExpressionNode
{
    private void setType ()
    {
        switch (this.getToken().getType()) {
        case PlexilLexer.COMMAND_HANDLE_KYWD:
            m_dataType = PlexilDataType.COMMAND_HANDLE_TYPE;
            break;

        case PlexilLexer.OUTCOME_KYWD:
            m_dataType = PlexilDataType.NODE_OUTCOME_TYPE;
            break;

        case PlexilLexer.STATE_KYWD:
            m_dataType = PlexilDataType.NODE_STATE_TYPE;
            break;

        case PlexilLexer.FAILURE_KYWD:
            m_dataType = PlexilDataType.NODE_FAILURE_TYPE;
            break;

        case PlexilLexer.NODE_TIMEPOINT_VALUE:
            // We can't know yet whether time is Real or Date
            // This is determined in the type checking pass (check())
            m_dataType = PlexilDataType.UNKNOWN_TYPE;
            break;

        default:
            m_dataType = PlexilDataType.ERROR_TYPE;
            break;
        }
    }

    public NodeVariableNode(Token t)
    {
        super(t);
        setType();
    }

    public NodeVariableNode(NodeVariableNode n)
    {
        super(n);
    }

	public Tree dupNode()
	{
		return new NodeVariableNode(this);
	}

    @Override
    protected void checkSelf(NodeContext context, CompilerState state)
    {
        if (this.getToken().getType() == PlexilLexer.NODE_TIMEPOINT_VALUE)
            m_dataType = GlobalContext.getGlobalContext().getTimeType();
    }

    @Override
    protected void checkChildren(NodeContext context, CompilerState state)
    {
        // Verify that referenced node is reachable from context
        PlexilTreeNode nodeRef = this.getChild(0);
        if (nodeRef.getToken().getType() == PlexilLexer.NCNAME) {
			// Check for a unique, reachable node ID
            String nodeName = nodeRef.getToken().getText();
            PlexilTreeNode target = null;
            if (!context.isNodeIdReachable(nodeName)) {
                state.addDiagnostic(nodeRef,
                                    "No reachable node named \"" + nodeName + "\"",
                                    Severity.ERROR);
            }
            else if (!context.isNodeIdUnique(nodeName)) {
                state.addDiagnostic(nodeRef,
                                    "Node id \"" + nodeName + "\" is ambiguous",
                                    Severity.ERROR);
            }
        }
        else
            nodeRef.check(context, state);
    }

    @Override
    protected void constructXML()
    {
        this.constructXMLBase();

		PlexilTreeNode nodeRef = this.getChild(0);
		if (nodeRef.getToken().getType() == PlexilLexer.NCNAME) {
			Element id = CompilerState.newElement("NodeId");
			id.appendChild(CompilerState.newTextNode(this.getChild(0).getText()));
			m_xml.appendChild(id);
		}
		else if (nodeRef instanceof NodeRefNode) {
			// NodeRef
			m_xml.appendChild(nodeRef.getXML());
		}
		else {
			Element err = CompilerState.newElement("_NODE_REF_ERROR_");
			m_xml.appendChild(err);
		}

		if (this.getToken().getType() == PlexilLexer.NODE_TIMEPOINT_VALUE) {
			Element state = CompilerState.newElement("NodeStateValue");
			state.appendChild(CompilerState.newTextNode(this.getChild(1).getText()));
			m_xml.appendChild(state);

			Element tp = CompilerState.newElement("Timepoint");
			tp.appendChild(CompilerState.newTextNode(this.getChild(2).getText()));
			m_xml.appendChild(tp);
		}
    }

    public String getXMLElementName()
    {
		switch (this.getToken().getType()) {
		case PlexilLexer.COMMAND_HANDLE_KYWD:
            return "NodeCommandHandleVariable";

		case PlexilLexer.OUTCOME_KYWD:
            return "NodeOutcomeVariable";

		case PlexilLexer.STATE_KYWD:
            return "NodeStateVariable";

		case PlexilLexer.FAILURE_KYWD:
            return "NodeFailureVariable";

		case PlexilLexer.NODE_TIMEPOINT_VALUE:
			return "NodeTimepointValue";

		default:
			return "ERROR";
		}
    }

}
