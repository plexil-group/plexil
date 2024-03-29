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

public class ResourceNode extends PlexilTreeNode
{
	private ExpressionNode m_name = null;
	private ExpressionNode m_upperBound = null;
	private ExpressionNode m_releaseAtTermination = null;
	private ExpressionNode m_priority = null;

	public ResourceNode(Token t)
	{
		super(t);
	}

	public ResourceNode(ResourceNode n)
	{
		super(n);
		m_name = n.m_name;
		m_upperBound = n.m_upperBound;
		m_releaseAtTermination = n.m_releaseAtTermination;
		m_priority = n.m_priority;
	}

	public Tree dupNode()
	{
		return new ResourceNode(this);
	}

    // format is:
    // ^(RESOURCE_KYWD name_expr [ option_kywd value_expr ]* )

    @Override
    protected void earlyCheckChildren(NodeContext context, CompilerState state)
    {
        for (int i = 0; i < this.getChildCount(); i += 2) {
            // Perform early checks on the expressions
            this.getChild(i).earlyCheck(context, state);
        }
    }

    @Override
	protected void earlyCheckSelf(NodeContext context, CompilerState state)
	{
		// format is:
		// ^(RESOURCE_KYWD name_expr [ option_kywd value_expr ]* )
		m_name = (ExpressionNode) this.getChild(0);
		for (int i = 1; i < this.getChildCount(); i += 2) {
			PlexilTreeNode kywd = this.getChild(i);
			ExpressionNode valueExpr = (ExpressionNode) this.getChild(i + 1);
			if (valueExpr == null) {
				if (this.getChild(i + 1) != null) {
					state.addDiagnostic(this.getChild(i + 1),
										"The value supplied for the Resource option "
										+ kywd.getText()
										+ " was not an expression",
										Severity.ERROR);
				}
				else {
					// Parser internal error or syntax/tree mismatch
					state.addDiagnostic(this,
										"Internal error: Resource option "
										+ kywd.getText()
										+ " missing a value expression",
										Severity.FATAL);
				}
			}

			switch (kywd.getType()) {

			case PlexilLexer.UPPER_BOUND_KYWD:
				if (m_upperBound != null) {
					// Repeated keyword error
					state.addDiagnostic(kywd,
										"The " + kywd.getText()
										+ " keyword may only appear once per Resource statement",
										Severity.ERROR);
				}
				m_upperBound = valueExpr;
				break;

			case PlexilLexer.RELEASE_AT_TERM_KYWD:
				if (m_releaseAtTermination != null) {
					// Repeated keyword error
					state.addDiagnostic(kywd,
										"The " + kywd.getText()
										+ " keyword may only appear once per Resource statement",
										Severity.ERROR);
				}
				m_releaseAtTermination = valueExpr;
				break;

			case PlexilLexer.PRIORITY_KYWD:
				if (m_priority != null) {
					state.addDiagnostic(kywd,
										"The " + kywd.getText()
										+ " keyword may only appear once per Resource statement",
										Severity.ERROR);
				}
				m_priority = valueExpr;
				break;

			default:
				// TODO: complain of bogus keyword
				state.addDiagnostic(kywd,
									"Unexpected token " + kywd.getText()
									+ " in a Resource statement",
									Severity.ERROR);
				break;
			}
		}
	}

    @Override
    protected void checkChildren(NodeContext context, CompilerState state)
    {
        for (int i = 0; i < this.getChildCount(); i += 2) {
            this.getChild(i).check(context, state);
        }
    }

    @Override
	protected void checkSelf(NodeContext context, CompilerState state)
	{
		// Type check name
		if (m_name.getDataType() != PlexilDataType.STRING_TYPE) {
			state.addDiagnostic(m_name,
								"Resource name is not a String expression",
								Severity.ERROR);
		}
		
		// Type check bounds, if supplied
		if (m_upperBound != null
			&& !m_upperBound.getDataType().isNumeric()) {
			state.addDiagnostic(m_upperBound,
								"Resource UpperBound value is not a numeric expression",
								Severity.ERROR);
		}

		// Type check release-at-termination
		if (m_releaseAtTermination != null
			&& m_releaseAtTermination.getDataType() != PlexilDataType.BOOLEAN_TYPE) {
			state.addDiagnostic(m_releaseAtTermination, 
								"Resource ReleaseAtTermination value is not a Boolean expression",
								Severity.ERROR);
		}

		// Type check priority, if supplied
		if (m_priority != null
			&& !m_priority.getDataType().isNumeric()) {
			state.addDiagnostic(m_priority,
								"Resource Priority value is not a numeric expression",
								Severity.ERROR);
		}
	}

    @Override
	protected void constructXML(Document root)
	{
		super.constructXMLBase(root);
		Element nameElt = root.createElement("ResourceName");
		nameElt.appendChild(m_name.getXML(root));
		m_xml.appendChild(nameElt);
        if (m_priority != null) {
            Element prio = root.createElement("ResourcePriority");
            prio.appendChild(m_priority.getXML(root));
            m_xml.appendChild(prio);
        }
		if (m_upperBound != null) {
			Element ubound = root.createElement("ResourceUpperBound");
			ubound.appendChild(m_upperBound.getXML(root));
			m_xml.appendChild(ubound);
		}
		if (m_releaseAtTermination != null) {
			Element rat = root.createElement("ResourceReleaseAtTermination");
			rat.appendChild(m_releaseAtTermination.getXML(root));
			m_xml.appendChild(rat);
		}
	}

}
