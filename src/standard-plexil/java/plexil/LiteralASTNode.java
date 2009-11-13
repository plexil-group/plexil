/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

import net.n3.nanoxml.*;

// 
// A specialized AST node that does code generation for literals.
// The data type should be specified by the parser from the context.
// 

public class LiteralASTNode extends PlexilASTNode
{
    // Needed for serializable
    static final long serialVersionUID = 3820020334365921089L;

    public LiteralASTNode() 
    {
	super();
    }

    public IXMLElement getXmlElement()
    {
        if (getDataType().isArray()) {
            IXMLElement result = 
                new XMLElement(getDataType().arrayElementType().typeName() + "ArrayValue");
            LiteralASTNode child = (LiteralASTNode) getFirstChild();
            while (child != null) {
                result.addChild(child.getXmlElement());
                child = (LiteralASTNode) child.getNextSibling();
            }
            return result;
        }
        else {
            return getXmlElement(getDataType().typeName() + "Value");
        }
    }

    public IXMLElement getXmlElement(String elementType)
    {
        IXMLElement result = new XMLElement(elementType);
        result.setContent(getText());
        return result;
    }

    // Helper methods to support parsing literals

    static protected boolean isQuadDigit(char c)
    {
        return (c >= '0' && c <= '3');
    }

    static protected boolean isOctalDigit(char c)
    {
        return (c >= '0' && c <= '7');
    }

    static protected boolean isDigit(char c)
    {
        return (c >= '0' && c <= '9');
    }

    static protected boolean isHexDigit(char c)
    {
        return (c >= '0' && c <= '9')
            || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F');
    }

    static protected int digitToInt(char c)
    {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        System.err.println("Error: '" + c + "' is not a digit");
        return -1;
    }

    static protected int hexDigitToInt(char c)
    {
        if (c >= '0' && c <= '9')
            return c - '0';
        else if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        System.err.println("Error: '" + c + "' is not a hex digit");
        return -1;
    }

}
