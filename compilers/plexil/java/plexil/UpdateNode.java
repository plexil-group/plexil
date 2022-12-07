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

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class UpdateNode extends NodeTreeNode
{

    //
    // Constructors
    //

	public UpdateNode(Token t)
	{
		super(t);
	}

	public UpdateNode(UpdateNode n)
	{
		super(n);
	}

	public Tree dupNode()
	{
		return new UpdateNode(this);
	}

    @Override
	protected void earlyCheckChildren(NodeContext parentContext, CompilerState state)
	{
        super.earlyCheckChildren(parentContext, state); // NodeTreeNode method

        List<String> tags = new ArrayList<String>();
		for (int i = 0; i < this.getChildCount(); i += 2) {
            // Check that tags are unique
            String tag = this.getChild(i).getText();
            if (tags.contains(tag)) {
                state.addDiagnostic(this.getChild(i),
                                    "Duplicate tag \"" + tag + "\" in Update node",
                                    Severity.ERROR);
            }
            else {
                tags.add(tag);
            }
		}
	}

    @Override
	protected void constructXML(Document root)
	{
		super.constructXMLBase(root);
		m_xml.setAttribute("NodeType", "Update");

        // construct node body
        Element nodeBody = root.createElement("NodeBody");
        m_xml.appendChild(nodeBody);

		// construct update
        Element update = root.createElement("Update");
        nodeBody.appendChild(update);

		for (int i = 0; i < this.getChildCount(); i += 2) {
			Element pair = root.createElement("Pair");
			Element name = root.createElement("Name");
			name.appendChild(root.createTextNode(this.getChild(i).getText()));
			pair.appendChild(name);
			pair.appendChild(this.getChild(i + 1).getXML(root));
			update.appendChild(pair);
		}
	}

    protected String getXMLElementName() { return "Node"; }

}
