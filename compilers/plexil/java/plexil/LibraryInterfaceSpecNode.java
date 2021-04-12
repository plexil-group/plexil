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

import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Element;

public class LibraryInterfaceSpecNode extends PlexilTreeNode
{
    private List<VariableName> m_parameterSpecs = null;

    public LibraryInterfaceSpecNode(Token t)
    {
        super(t);
    }

    public LibraryInterfaceSpecNode(LibraryInterfaceSpecNode n)
    {
        super(n);
        m_parameterSpecs = n.m_parameterSpecs;
    }

    public Tree dupNode()
    {
        return new LibraryInterfaceSpecNode(this);
    }

    // Format is (LIBRARY_INTERFACE ( (IN_KYWD | IN_OUT_KYWD) baseTypeName NCNAME INT? )* )
    @Override
    protected void earlyCheckChildren(NodeContext context, CompilerState state)
    {
        Set<String> names = new TreeSet<String>();
        if (this.getChildCount() > 0)
            m_parameterSpecs = new ArrayList<VariableName>();

        for (int parmIdx = 0; parmIdx < this.getChildCount(); parmIdx++) {
            String typeName = null;
            PlexilTreeNode nameSpec = null;
            String sizeSpec = null;

            PlexilTreeNode parm = this.getChild(parmIdx);
            int parmType = parm.getType();
            if (parmType == PlexilLexer.IN_KYWD
                || parmType == PlexilLexer.IN_OUT_KYWD) {
                // ^((In | InOut) typename NCNAME INT?)
                typeName = parm.getChild(0).getText();
                nameSpec = parm.getChild(1);
                if (parm.getChild(2) != null) {
                    sizeSpec = parm.getChild(2).getText(); // array dimension
                    // check spec'd size
                    int arySiz = LiteralNode.parseIntegerValue(sizeSpec);
                    if (arySiz < 0) {
                        state.addDiagnostic(parm.getChild(1),
                                            "Array maximum size must not be negative",
                                            Severity.ERROR);
                    }
                }
            }

            String nam =
                (nameSpec == null) ? null : nameSpec.getText();

            // check for duplicate names and warn
            if (nam != null) {
                if (names.contains(nam)) {
                    // Report duplicate name error
                    state.addDiagnostic(nameSpec,
                                        "Library parameter name \"" + nam
                                        + "\" was used more than once",
                                        Severity.ERROR);
                }
                else {
                    names.add(nam);
                }
            }

            // Construct parameter spec
            VariableName newParmVar = null;
            switch (parmType) {
                // In and InOut are the only valid types
            case PlexilLexer.IN_KYWD:
            case PlexilLexer.IN_OUT_KYWD:
                if (sizeSpec != null) {
                    // Array case
                    newParmVar = 
                        new InterfaceVariableName(parm,
                                                  nam,
                                                  parmType == PlexilLexer.IN_OUT_KYWD,
                                                  PlexilDataType.findByName(typeName).arrayType(),
                                                  sizeSpec,
                                                  null);
                }
                else {
                    newParmVar =
                        new InterfaceVariableName(parm,
                                                  nam,
                                                  parmType == PlexilLexer.IN_OUT_KYWD,
                                                  PlexilDataType.findByName(typeName));
                }
                break;

            default:
                state.addDiagnostic(parm, 
                                    "Invalid parameter descriptor \"" + parm.getText()
                                    + "\" in a LibraryNode declaration",
                                    Severity.ERROR);
                break;
            }

            m_parameterSpecs.add(newParmVar);
        }
    }

    public List<VariableName> getParameterList() { return m_parameterSpecs; }

    // For command and lookup
    public void constructParameterXML(Element parent)
    {
        if (m_parameterSpecs != null) {
            for (VariableName vn : m_parameterSpecs) 
                parent.appendChild(vn.makeGlobalDeclarationElement("Parameter"));
        }
    }

    @Override
    protected void constructXML()
    {
        m_xml = CompilerState.newElement(this.getXMLElementName()); // no source locators desired
        if (m_parameterSpecs != null) {
            List<VariableName> inVars = new ArrayList<VariableName>();
            List<VariableName> inOutVars = new ArrayList<VariableName>();
            for (VariableName vn : m_parameterSpecs) {
                if (vn.isAssignable())
                    inOutVars.add(vn);
                else
                    inVars.add(vn);
            }
            if (inVars.size() > 0) {
                Element inXML = CompilerState.newElement("In");
                m_xml.appendChild(inXML);
                for (VariableName inVar : inVars)
                    inXML.appendChild(inVar.makeDeclarationXML());
            }
            if (inOutVars.size() > 0) {
                Element inOutXML = CompilerState.newElement("InOut");
                m_xml.appendChild(inOutXML);
                for (VariableName inOutVar : inOutVars)
                    inOutXML.appendChild(inOutVar.makeDeclarationXML());
            }
        }
    }

    public String getXMLElementName() { return "Interface"; }

}
