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

import plexil.PlexilName;
import plexil.PlexilDataType;

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

    public Element makeGlobalDeclarationElement(String elementType)
    {
        Element result = CompilerState.newElement(elementType);
        if (m_name != null) {
            Element nameXML = CompilerState.newElement("Name");
            nameXML.appendChild(CompilerState.newTextNode(getName()));
            result.appendChild(nameXML);
        }
        Element typeXML = CompilerState.newElement("Type");
        result.appendChild(typeXML);
        if (m_variableType.isArray()) {
            typeXML.appendChild(CompilerState.newTextNode(m_variableType.arrayElementType().typeName()));
            Element sizeXML = CompilerState.newElement("MaxSize");
            sizeXML.appendChild(CompilerState.newTextNode(Integer.toString(m_maxSize)));
            result.appendChild(sizeXML);
        }
        else {
            typeXML.appendChild(CompilerState.newTextNode(getVariableTypeName()));
        }
        return result;
    }


    // For code generation purposes
    public Element makeDeclarationXML()
    {
        Element result = 
            CompilerState.newElement(isArray() ? "DeclareArray" : "DeclareVariable");

        // add source locators
        if (m_declaration != null) {
            result.setAttribute("LineNo", String.valueOf(m_declaration.getLine()));
            result.setAttribute("ColNo", String.valueOf(m_declaration.getCharPositionInLine()));
        }

        Element xname = CompilerState.newElement("Name");
        xname.appendChild(CompilerState.newTextNode(getName()));
        result.appendChild(xname);

        String typeName =
            isArray() ? getArrayElementTypeName()
            : getVariableTypeName(); 
        Element xtype = CompilerState.newElement("Type");
        xtype.appendChild(CompilerState.newTextNode(typeName));
        result.appendChild(xtype);

        if (isArray()) {
            Element xsize = CompilerState.newElement("MaxSize");
            xsize.appendChild(CompilerState.newTextNode(Integer.toString(m_maxSize)));
            result.appendChild(xsize);
        }

        if (m_initialValue != null) {
            Element init = CompilerState.newElement("InitialValue");
            result.appendChild(init);
            if (m_initialValue.getDataType().isArray()) {
                // handle array initial value
                Element val = CompilerState.newElement("ArrayValue");
                val.setAttribute("Type", getArrayElementTypeName());
                for (int i = 0; i < m_initialValue.getChildCount(); i++) {
                    val.appendChild(m_initialValue.getChild(i).getXML());
                }
                init.appendChild(val);
            }
            else {
                init.appendChild(m_initialValue.getXML());
            }
        }

        return result;
    }

}
