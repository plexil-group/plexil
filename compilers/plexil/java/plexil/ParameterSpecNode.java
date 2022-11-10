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
import java.util.Set;
import java.util.TreeSet;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

// ParameterSpecNode is used in Lookup and Command declarations.
// Structure:
// (PARAMETERS ( (paramTypeName NCNAME?)
//             | (ARRAY_TYPE baseTypeName INT NCNAME?) )*
//             ELLIPSIS?)

public class ParameterSpecNode extends PlexilTreeNode
{
    private List<VariableName> m_parameterSpecs = null;

    public ParameterSpecNode(Token t)
    {
        super(t);
    }

    public ParameterSpecNode(ParameterSpecNode n)
    {
        super(n);
        m_parameterSpecs = n.m_parameterSpecs;
    }

    public Tree dupNode()
    {
        return new ParameterSpecNode(this);
    }

    @Override
    protected void earlyCheckChildren(NodeContext context, CompilerState state)
    {
        boolean ellipsisSeen = false;
        Set<String> names = new TreeSet<String>();
        if (this.getChildCount() > 0)
            m_parameterSpecs = new ArrayList<VariableName>();

        int parmIdx = 0; // only used to generate placeholders for erroneous decls
        for (PlexilTreeNode parm : this.getChildren()) {
            String typeName = null;
            PlexilTreeNode nameSpec = null;
            String sizeSpec = null;

            int parmType = parm.getType();
            if (parmType == PlexilLexer.ARRAY_TYPE) {
                // ^(ARRAY_TYPE eltTypeName INT NCNAME?)
                typeName = parm.getChild(0).getText();
                sizeSpec = parm.getChild(1).getText();
                nameSpec = parm.getChild(2); // may be null
                // check spec'd size
                int arySiz = LiteralNode.parseIntegerValue(sizeSpec);
                if (arySiz < 0) {
                    state.addDiagnostic(parm.getChild(1),
                                        "Array size must not be negative",
                                        Severity.ERROR);
                }
            }
            else {
                // typeName or ^(typeName NCNAME)
                typeName = parm.getToken().getText();
                if (parm.getChildCount() > 0)
                    nameSpec = parm.getChild(0);
            }

            // check for duplicate names and warn
            String nam = null;
            if (nameSpec != null) {
                nam = nameSpec.getText();
                if (names.contains(nam)) {
                    // Report duplicate name error
                    state.addDiagnostic(nameSpec,
                                        "Parameter name \"" + nam + "\" was used more than once",
                                        Severity.ERROR);
                }
                else {
                    names.add(nam);
                }
            }

            if (ellipsisSeen) {
                state.addDiagnostic(parm, 
                                    "Wildcard (ellipsis) parameter must be the last parameter",
                                    Severity.ERROR);
                ellipsisSeen = false; // turn off for subsequent parameters
            }

            // Construct parameter spec
            VariableName newParmVar = null;
            switch (parmType) {

            case PlexilLexer.ANY_KYWD:
            case PlexilLexer.BOOLEAN_KYWD:
            case PlexilLexer.INTEGER_KYWD:
            case PlexilLexer.REAL_KYWD:
            case PlexilLexer.STRING_KYWD:
            case PlexilLexer.DATE_KYWD:
            case PlexilLexer.DURATION_KYWD:
                newParmVar = new VariableName(parm,
                                              nam,
                                              PlexilDataType.findByName(typeName));
                break;

            case PlexilLexer.ARRAY_TYPE:
                newParmVar = new VariableName(parm,
                                              nam,
                                              PlexilDataType.findByName(typeName).arrayType(),
                                              sizeSpec,
                                              null);
                break;

            case PlexilLexer.ELLIPSIS:
                newParmVar = new WildcardVariableName(parm);
                ellipsisSeen = true;
                break;

            default:
                state.addDiagnostic(parm, 
                                    "Invalid parameter descriptor \"" + parm.getText() + "\"",
                                    Severity.ERROR);
                // create a placeholder so other parsing can continue
                newParmVar = new VariableName(parm, 
                                              "INVALID_PARM_" + Integer.toString(parmIdx),
                                              PlexilDataType.ANY_TYPE);
                break;
            }

            m_parameterSpecs.add(newParmVar);
            ++parmIdx;
        }
    }

    public List<VariableName> getParameterList() { return m_parameterSpecs; }

    public boolean containsAnyType()
    {
        if (m_parameterSpecs == null)
            return false;
        for (VariableName vn : m_parameterSpecs)
            if (vn.getVariableType() == PlexilDataType.ANY_TYPE)
                return true;
        return false;
    }

    public void constructParameterXML(Document root, Element parent)
    {
        if (m_parameterSpecs != null) {
            for (VariableName vn : m_parameterSpecs) 
                parent.appendChild(vn.makeGlobalDeclarationElement(root, "Parameter"));
        }
    }

    public String getXMLElementName() { return "Parameters"; }

}
