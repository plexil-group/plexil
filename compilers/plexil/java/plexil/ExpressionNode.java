// Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

import plexil.PlexilTreeNode;

public class ExpressionNode
    extends PlexilTreeNode
    implements Expression
{
    // Default type is void.
    // May be overridden in derived classes.
    protected PlexilDataType m_dataType = PlexilDataType.VOID_TYPE;

    //
    // Constructors
    //
    public ExpressionNode()
    {
        super();
    }

    public ExpressionNode(ExpressionNode node)
    {
        super(node);
        m_dataType = node.m_dataType;
    }

    public ExpressionNode(PlexilTreeNode node)
    {
        super(node);
    }

    public ExpressionNode(Token t)
    {
        super(t);
    }

	public Tree dupNode()
	{
		return new ExpressionNode(this);
	}

    protected void setDataType(PlexilDataType dtype)
    {
        m_dataType = dtype;
    }

    // Expression interface
    public PlexilDataType getDataType()
    {
        return m_dataType;
    }

    // Default method.  
    // Variables and array references should override this method.
    public boolean isAssignable()
    {
        return false;
    }

    // Default method.  
    // Variables and array references should override this method.
    public void checkAssignable(NodeContext context, CompilerState myState)
    {
        myState.addDiagnostic(this,
                              "Expression may not be assigned to",
                              Severity.ERROR);
    }

    /**
     * @brief Establish bindings and do initial checks of this node's children.
     * @note Derived classes should override this as applicable.
     */
    @Override
    protected void earlyCheckChildren(NodeContext context, CompilerState state)
    {
        for (int i = 0; i < this.getChildCount(); i++) {
            PlexilTreeNode child = this.getChild(i);
            if (child instanceof CommandNode)
                state.addDiagnostic(child,
                                    "Commands may not be used in expressions",
                                    Severity.ERROR);
            child.earlyCheck(context, state);
        }
    }

    /**
     * @brief Perform a recursive semantic check.
     * @note The top level check comes first because it establishes types for the children.
     * @note Derived classes should override this where appropriate.
     */
    @Override
    public void check(NodeContext context, CompilerState myState)
    {
        super.check(context, myState); // can establish types for children
        checkTypeConsistency(context, myState);
    }

    /**
     * @brief Check the expression for type consistency.
     * @note This is a default method.  Derived classes should override it.
     */
    protected void checkTypeConsistency(NodeContext context, CompilerState myState)
    {
    }

    /**
     * @brief Persuade the expression to assume the specified data type
     * @return true if the expression can consistently assume the specified type, false otherwise.
     * @note This is a default method.  Derived classes should override it.
     */
    protected boolean assumeType(PlexilDataType t, CompilerState myState)
    {
        // If our type is null, void, or error, fail.
        if (!PlexilDataType.isValid(m_dataType))
            return false;

        // If target type is Void, Error, or underspec'd array, fail.
        if (!PlexilDataType.isValid(t)) {
            myState.addDiagnostic(null,
                                  "Internal error: ExpressionNode.assumeType() called with invalid type "
                                  + t,
                                  Severity.FATAL);
            return false;
        }

        // If target type is Any, always succeed.
        if (t == PlexilDataType.ANY_TYPE)
            return true;

        // If our type is Any, assume the requested type but warn
        if (m_dataType == PlexilDataType.ANY_TYPE) {
            myState.addDiagnostic(this,
                                  "Any-type expression coerced to " + t,
                                  Severity.WARNING);
            m_dataType = t;
            return true;
        }

        // Otherwise don't change anything
        return (m_dataType == t)
            || (t.isNumeric() && m_dataType.isNumeric()); // *** FIXME: too general?
    }

}
