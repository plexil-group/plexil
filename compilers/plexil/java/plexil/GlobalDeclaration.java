/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

// *** To do:
//  - add array support
//  - add resource declaration support

package plexil;

import java.util.ArrayList;
import java.util.List;

public class GlobalDeclaration extends PlexilName
{
    protected NameType m_declarationType;
    protected List<VariableName> m_paramSpecs;
    protected VariableName m_returnSpec;

    // TODO: handle resource list
    public GlobalDeclaration(PlexilTreeNode declaration,
                             String myName,
                             NameType declType,
                             List<VariableName> paramSpecs,
                             VariableName returnSpec)
    {
        super(myName, declType, declaration);
        m_declarationType = declType;
        m_paramSpecs = paramSpecs;
        m_returnSpec = returnSpec;
    }

    // returns first return type or null
    public PlexilDataType getReturnType()
    {
        if (m_returnSpec == null)
            return null;
        return m_returnSpec.getVariableType();
    }

    // returns return variable, or null
    public VariableName getReturnVariable()
    {
        if (m_returnSpec == null)
            return null;
        return m_returnSpec;
    }

    // returns vector of parameter types, or null
    public List<PlexilDataType> getParameterTypes()
    {
        if (m_paramSpecs == null)
            return null;
        List<PlexilDataType> result = new ArrayList<PlexilDataType>();
        for (VariableName v : m_paramSpecs)
            result.add(v.getVariableType());
        return result;
    }

    // returns vector of parameter variables, or null
    public List<VariableName> getParameterVariables()
    {
        return m_paramSpecs;
    }

    // returns parameter variable descriptor, or null
    public VariableName getParameterByName(String name)
    {
        if (m_paramSpecs == null)
            return null;
        for (VariableName candidate : m_paramSpecs)
            if (name.equals(candidate.getName()))
                return candidate;
        return null;
    }

    // returns parameter variable descriptor, or null
    public boolean hasParameterNamed(String name)
    {
        return (getParameterByName(name) != null);
    }

}
