/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Universities Space Research Association nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package plexil;

//
// A specialization of VariableName to support interface variables.
//

public class InterfaceVariableName extends VariableName
{
    protected boolean isInOut;
    protected VariableName parentVariable;

    // This variant is for variables that already exist.
    public InterfaceVariableName(PlexilTreeNode declaration,
                                 String myName, 
                                 boolean is_inOut,
                                 VariableName parent)
    {
        super(declaration, myName, parent.getVariableType());
        parentVariable = parent;
        isInOut = is_inOut;
    }

    // use this variant for scalar library node interface variables
    public InterfaceVariableName(PlexilTreeNode declaration,
                                 String myName, 
                                 boolean is_inOut,
                                 PlexilDataType varType)
    {
        super(declaration, myName, varType);
        parentVariable = null;
        isInOut = is_inOut;
    }

    // use this variant for array library node interface variables
    public InterfaceVariableName(PlexilTreeNode declaration,
                                 String myName, 
                                 boolean is_inOut,
                                 PlexilDataType varType,
                                 String maxSize,
                                 ExpressionNode initVal)
    {
        super(declaration, myName, varType, maxSize, initVal);
        parentVariable = null;
        isInOut = is_inOut;
    }

    public boolean isLocal()
    {
        return false;
    }

    public boolean isAssignable()
    {
        return isInOut;
    }

    public boolean hasParentVariable()
    {
        return (parentVariable != null);
    }

}
