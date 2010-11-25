/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

package plexil;

import java.util.Vector;

import net.n3.nanoxml.*;

public class GlobalDeclaration extends PlexilName
{
    protected NameType m_declarationType;
    protected Vector<VariableName> m_paramSpecs;
    protected Vector<VariableName> m_returnSpecs;

	// TODO: handle resource list
    public GlobalDeclaration(PlexilTreeNode declaration,
							 String myName,
                             NameType declType,
                             PlexilTreeNode parmAST,
                             PlexilTreeNode returnAST)
    {
        super(myName, declType, declaration);
        m_declarationType = declType;
        m_paramSpecs = null;
        if (parmAST != null) {
			m_paramSpecs = new Vector<VariableName>();
			for (int parmIdx = 0; parmIdx < parmAST.getChildCount(); parmIdx++) {
				PlexilTreeNode parm = parmAST.getChild(parmIdx);
				String typeName = parm.getToken().getText();
				PlexilTreeNode parmName = parm.getChild(1);
				String nam =
					(parmName == null) ? ("_" + m_declarationType.plexilName + "_param_" + parmIdx)
					: parmName.getText();
				VariableName newParmVar = null;
				switch (parm.getType()) {
					// In and InOut are only valid for library node declarations
				case PlexilLexer.IN_KYWD:
					newParmVar = new InterfaceVariableName(parm,
														   nam,
														   false,
														   PlexilDataType.findByName(typeName));
					break;

				case PlexilLexer.IN_OUT_KYWD:
					newParmVar = new InterfaceVariableName(parm,
														   nam,
														   true,
														   PlexilDataType.findByName(typeName));
					break;

				case PlexilLexer.BOOLEAN_KYWD:
				case PlexilLexer.INTEGER_KYWD:
				case PlexilLexer.REAL_KYWD:
				case PlexilLexer.STRING_KYWD:
					newParmVar = new VariableName(parm,
												  nam,
												  PlexilDataType.findByName(typeName));
					break;

				default:
					System.out.println("Invalid parameter descriptor token type " + parm.getType());
					break;
				}

				m_paramSpecs.add(newParmVar);
			}
		}
        m_returnSpecs = null;
        if (returnAST != null) {
			m_returnSpecs = new Vector<VariableName>();
			for (int retnIdx = 0; retnIdx < returnAST.getChildCount(); retnIdx++) {
				PlexilTreeNode retn = returnAST.getChild(retnIdx);
				String typeName = retn.getToken().getText();
				String nam = "_" + m_declarationType.plexilName + "_return_" + retnIdx;
				m_returnSpecs.add(new VariableName(retn,
												   nam,
												   PlexilDataType.findByName(typeName)));
			}
		}
    }

    // returns first return type or null
    public PlexilDataType getReturnType()
    {
        if (m_returnSpecs == null)
            return null;
        return m_returnSpecs.firstElement().getVariableType();
    }

    // returns vector of return types, or null
    public Vector<PlexilDataType> getReturnTypes()
    {
        if (m_returnSpecs == null)
            return null;
        Vector<PlexilDataType> result = new Vector<PlexilDataType>();
        for (VariableName v : m_returnSpecs)
            result.add(v.getVariableType());
        return result;
    }

    // returns vector of return variables, or null
    public Vector<VariableName> getReturnVariables() { return m_returnSpecs;}

    // returns vector of parameter types, or null
    public Vector<PlexilDataType> getParameterTypes()
    {
        if (m_paramSpecs == null)
            return null;
        Vector<PlexilDataType> result = new Vector<PlexilDataType>();
		for (VariableName v : m_paramSpecs)
            result.add(v.getVariableType());
        return result;
    }

    // returns vector of parameter variables, or null
    public Vector<VariableName> getParameterVariables() { return m_paramSpecs; }

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
