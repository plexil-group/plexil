/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

import plexil.PlexilName;
import plexil.PlexilDataType;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class VariableName extends PlexilName
{
    protected PlexilDataType m_variableType;
    protected ExpressionNode m_initialValue;
    protected int m_maxSize = -1; // for arrays only, -1 = uninitialized

    // this variant used only in global declarations

    public VariableName(PlexilTreeNode decl, String myName, PlexilDataType varType)
    {
        super(myName, NameType.VARIABLE_NAME, decl);
        m_variableType = varType;
        m_initialValue = null;
        m_maxSize = -1;
    }

    // used in local variable declarations in tree parser

    public VariableName(PlexilTreeNode decl, String myName, PlexilDataType varType, ExpressionNode initVal)
    {
        super(myName, NameType.VARIABLE_NAME, decl);
        m_variableType = varType;
        m_initialValue = initVal;
        m_maxSize = -1;
    }

    // used in local array variable declarations

    public VariableName(PlexilTreeNode decl,
                        String myName, 
                        PlexilDataType varType, 
                        String maxSize,
                        ExpressionNode initVal)
    {
        super(myName, NameType.VARIABLE_NAME, decl);
        m_variableType = varType;
        m_maxSize = LiteralNode.parseIntegerValue(maxSize);
        m_initialValue = initVal;
    }

    //
    // Basic queries
    //

    public PlexilDataType getVariableType()
    {
        return m_variableType;
    }

    public PlexilTreeNode getInitialValue()
    {
        return m_initialValue;
    }

    public int getMaxSize()
    {
        return m_maxSize;
    }

    public String getVariableTypeName()
    {
        return getVariableType().typeName();
    }

    public boolean isPrimitiveType()
    {
        return m_variableType.isPrimitive();
    }

    public boolean isArray()
    {
        return m_variableType.isArray();
    }

    public PlexilDataType getArrayElementType()
    {
        if (isArray())
            return m_variableType.arrayElementType();
        else
            return null;
    }

    public String getArrayElementTypeName()
    {
        if (isArray())
            return m_variableType.arrayElementType().typeName();
        else
            return null;
    }

    // Subclasses may override this method
    public boolean isLocal()
    {
        return true;
    }

    // Subclasses may override this method
    public boolean isAssignable()
    {
        return true;
    }

    // Subclasses may override this method
    public boolean isExplicit()
    {
        return true; // all local variables are explicitly declared
    }

    // For code generation purposes
    // Subclasses may override this method

    public Element makeGlobalDeclarationElement(Document root, String elementType)
    {
        Element result = root.createElement(elementType);
        if (m_name != null) {
            Element nameXML = root.createElement("Name");
            nameXML.appendChild(root.createTextNode(getName()));
            result.appendChild(nameXML);
        }
        Element typeXML = root.createElement("Type");
        result.appendChild(typeXML);
        if (m_variableType.isArray()) {
            typeXML.appendChild(root.createTextNode(m_variableType.arrayElementType().typeName()));
            Element sizeXML = root.createElement("MaxSize");
            sizeXML.appendChild(root.createTextNode(Integer.toString(m_maxSize)));
            result.appendChild(sizeXML);
        }
        else {
            typeXML.appendChild(root.createTextNode(getVariableTypeName()));
        }
        return result;
    }


    // For code generation purposes
    public Element makeDeclarationXML(Document root)
    {
        Element result = 
            root.createElement(isArray() ? "DeclareArray" : "DeclareVariable");

        // add source locators
        if (m_declaration != null) {
            result.setAttribute("LineNo", String.valueOf(m_declaration.getLine()));
            result.setAttribute("ColNo", String.valueOf(m_declaration.getCharPositionInLine()));
        }

        Element xname = root.createElement("Name");
        xname.appendChild(root.createTextNode(getName()));
        result.appendChild(xname);

        String typeName =
            isArray() ? getArrayElementTypeName()
            : getVariableTypeName(); 
        Element xtype = root.createElement("Type");
        xtype.appendChild(root.createTextNode(typeName));
        result.appendChild(xtype);

        if (isArray()) {
            Element xsize = root.createElement("MaxSize");
            xsize.appendChild(root.createTextNode(Integer.toString(m_maxSize)));
            result.appendChild(xsize);
        }

        if (m_initialValue != null) {
            Element init = root.createElement("InitialValue");
            result.appendChild(init);
            if (m_initialValue.getDataType().isArray()) {
                // handle array initial value
                Element val = root.createElement("ArrayValue");
                val.setAttribute("Type", getArrayElementTypeName());
                for (int i = 0; i < m_initialValue.getChildCount(); i++) {
                    val.appendChild(m_initialValue.getChild(i).getXML(root));
                }
                init.appendChild(val);
            }
            else {
                init.appendChild(m_initialValue.getXML(root));
            }
        }

        return result;
    }

}
