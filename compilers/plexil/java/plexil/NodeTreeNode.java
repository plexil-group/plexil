// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

//* @class NodeTreeNode
//* NodeTreeNode is an implementation class representing any parse subtree
//* which could represent a PLEXIL Node.  It is a superclass of several
//* instantiable tree node classes, but is not meant to be instantiated
//* by itself.  It implements common behavior required by all Node types.

public class NodeTreeNode
    extends PlexilTreeNode
    implements PlexilNode
{
    protected NodeContext m_context = null;

    protected NodeTreeNode(Token t)
    {
        super(t);
    }

    protected NodeTreeNode(NodeTreeNode n)
    {
        super(n);
        m_context = n.m_context;
    }

    //
    // PlexilNode API
    //

    public NodeContext getContext()
    {
        return m_context;
    }

    public boolean hasNodeId()
    {
        PlexilTreeNode parent = this.getParent();
        return parent != null && parent instanceof ActionNode
            && ((ActionNode) parent).hasNodeId();
    }

    public boolean inheritsParentContext()
    {
        PlexilTreeNode parent = this.getParent();
        if (parent == null)
            return false; // no parent - shouldn't happen
        if (!(parent instanceof ActionNode))
            return false; // shouldn't happen
        if (((ActionNode) parent).hasNodeId())
            return false;
        PlexilTreeNode grandparent = parent.getParent();
        if (grandparent == null)
            return false; // either no grandparent or not a BlockNode
        if (grandparent instanceof BlockNode)
            return ((BlockNode) grandparent).isSimpleNode();
        return false;
    }

    public void initializeContext(NodeContext parentContext)
    {
        if (this.inheritsParentContext()) {
            m_context = parentContext;
        }
        else {
            String nodeId = null;
            if (this.hasNodeId()) {
                nodeId = ((ActionNode) this.getParent()).getNodeId();
            }
            m_context = new NodeContext(parentContext, nodeId);
        }
    }

    @Override
    protected void earlyCheckSelf(NodeContext parentContext, CompilerState state)
    {
        initializeContext(parentContext);
    }

    @Override
    protected void earlyCheckChildren(NodeContext parentContext, CompilerState state)
    {
        for (PlexilTreeNode child : this.getChildren())
            child.earlyCheck(m_context, state);
    }

    @Override
    protected void checkChildren(NodeContext parentContext, CompilerState state)
    {
        for (PlexilTreeNode child : this.getChildren())
            child.check(m_context, state);
    }

}
