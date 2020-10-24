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

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

// 
// A specialized AST node that does code generation for string literals.
// 

public class StringLiteralNode extends LiteralNode
{
    // Needed for serializable
    private static final long serialVersionUID = -8806474295262342033L;

    // Cache for parent syntax/semantic checkers and code generation
    private String m_stringValue;

    public StringLiteralNode(Token t)
    {
        super(t);
    }

	public StringLiteralNode(StringLiteralNode n)
	{
		super(n);
        m_stringValue = n.m_stringValue;
	}

    // Override LiteralNode method
    @Override
    protected void setInitialDataTypeFromTokenType()
    {
        m_dataType = PlexilDataType.STRING_TYPE;
    }

	public Tree dupNode()
	{
        return new StringLiteralNode(this);
	}

    public String getStringValue()
    {
        return m_stringValue;
    }

    @Override
    protected void earlyCheckSelf(NodeContext context, CompilerState state)
    {
        // Check that format is correct, and extract actual value w/o escapes
        parseStringValue(state);
    }

    // Override LiteralNode method
    @Override
    protected void constructXML()
    {
        super.constructXMLBase();
        m_xml.appendChild(CompilerState.newTextNode(m_stringValue));
    }

    //
    // Static helper methods
    //

    static private boolean isOctalDigit(char c)
    {
        return (c >= '0' && c <= '7');
    }

    static private boolean isHexDigit(char c)
    {
        return (c >= '0' && c <= '9')
            || (c >= 'a' && c <= 'f')
            || (c >= 'A' && c <= 'F');
    }

    // Handle all 'digits' -
    // all inputs are already range checked by caller
    static private int digitToInt(char c)
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

    // Subroutine of parseStringValue
    // Process one escape and add it to myContent.
    // Index points to the character after the \.
    // Return index of first character after the escaped character(s),
    // or -1 in event of parse error
    private int parseEscape(String myText,
                            int index,
                            StringBuilder myContent,
                            CompilerState state)
    {
        char escaped = myText.charAt(index++);
        int charcode = 0;
        switch (escaped) {
            // \[0-3]([0-7][0-7]?)?
            // - Octal escape
        case '0':
        case '1':
        case '2':
        case '3':
            // 1-3 chars
            charcode = digitToInt(escaped);
            escaped = myText.charAt(index++);
            if (isOctalDigit(escaped)) {
                charcode = (charcode * 8) + digitToInt(escaped);
                escaped = myText.charAt(index++);
                if (isOctalDigit(escaped))
                    charcode = (charcode * 8) + digitToInt(escaped);
                else
                    index--;
            }
            else {
                index--;
            }
            myContent.append((char) charcode);
            break;
                        
            // \[4-7][0-7]?
            // - Octal escape
        case '4':
        case '5':
        case '6':
        case '7':
            // 1-2 chars
            charcode = digitToInt(escaped);
            escaped = myText.charAt(index++);
            if (isOctalDigit(escaped))
                charcode = (charcode * 8) + digitToInt(escaped);
            else
                index--;
            myContent.append((char) charcode);
            break;

            // \ U [0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]
            // - Unicode long escape
        case 'U':
            // check for hex digits and compose character
            for (int i = 0; i < 8; i++) {
                char hex = myText.charAt(index++);
                if (isHexDigit(hex)) {
                    charcode = (charcode * 16) + digitToInt(hex);
                }
                else {
                    state.addDiagnostic(this,
                                        "Invalid Unicode escape format in string literal \""
                                        + myText + "\"",
                                        Severity.ERROR);
                    return -1;
                }
            }
            myContent.appendCodePoint(charcode);
            break;

            // \b - backspace
        case 'b':
            myContent.append('\b');
            break;

            // \f - form feed
        case 'f':
            myContent.append('\f');
            break;

            // \n - newline
        case 'n':
            myContent.append('\n');
            break;

            // \t - tab
        case 't':
            myContent.append('\t');
            break;

            // \ u [0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]
            // - Unicode escape
        case 'u':
            // Check for hex digits and compose character
            for (int i = 0; i < 4; i++) {
                char hex = myText.charAt(index++);
                if (isHexDigit(hex)) {
                    charcode = (charcode * 16) + digitToInt(hex);
                }
                else {
                    state.addDiagnostic(this,
                                        "Invalid Unicode escape format in string literal \""
                                        + myText + "\"",
                                        Severity.ERROR);
                    return -1;
                }
            }
            myContent.appendCodePoint(charcode);
            break;

            // \x[0-9a-fA-F]+
            // - hex escape
        case 'x':
            {
                // check for at least one hex digit and compose character
                char hex = myText.charAt(index++);
                if (isHexDigit(hex)) {
                    charcode = digitToInt(hex);
                }
                else {
                    state.addDiagnostic(this,
                                        "Invalid hexadecimal escape format in string literal \""
                                        + myText + "\"",
                                        Severity.ERROR);
                    return -1;
                }
                while (isHexDigit(hex = myText.charAt(index++)))
                    charcode = (charcode * 16) + digitToInt(hex);
                // Back up one for first non-hex
                index--;
                // Store result
                myContent.appendCodePoint(charcode); // ???
            }
            break;

            // verbatim (includes backslash, single quote, double quote)
        default:
            myContent.append(escaped);
            break;
        }
        return index;
    }

    // Check that string is properly formed
    // Extract escape characters to get the real value
    // and cache it for later use
    private void parseStringValue(CompilerState state)
    {
        String myText = getText();
        // These errors shouldn't sneak past the ANTLR-generated lexer,
        // but just in case... 
        char initialQuote = myText.charAt(0);
        if (initialQuote != '\'' && initialQuote != '"') {
            state.addDiagnostic(this,
                                "String literal initial character '" + initialQuote
                                + "' is neither a single-quote nor double-quote",
                                Severity.ERROR);
            return;
        }
        if (initialQuote != myText.charAt(myText.length() - 1)) {
            state.addDiagnostic(this,
                                "String literal initial quote character '" + initialQuote
                                + "' doesn't match closing quote character '"
                                + myText.charAt(myText.length() - 1) + "'",
                                Severity.ERROR);
            return;
        }

        StringBuilder myContent = new StringBuilder(myText.length());
        int index = 1; // start past opening quote
        while (index < myText.length()) {
            int nextIdx = myText.indexOf('\\', index);
            if (nextIdx != -1) {
                // copy string up to \ escape char
                if (index != nextIdx)
                    myContent.append(myText, index, nextIdx);
                // handle escapes
                nextIdx = parseEscape(myText, nextIdx + 1, myContent, state);
                // Return w/o setting m_stringValue in event of parse error
                if (nextIdx == -1) 
                    return;
            }
            else {
                // find end quote
                nextIdx = myText.indexOf(initialQuote, index);
                // Check for lexer error
                if (nextIdx == -1) {
                    state.addDiagnostic(this,
                                        "Internal error: can't find closing quote for string literal "
                                        + myText,
                                        Severity.FATAL);
                    return;
                }
                // copy to end
                myContent.append(myText, index, nextIdx);
                nextIdx++;
            }
            index = nextIdx;
        }
        // All good, cache result
        m_stringValue = myContent.toString();
    }

}
