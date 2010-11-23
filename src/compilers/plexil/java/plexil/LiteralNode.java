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

import org.antlr.runtime.Token;

import net.n3.nanoxml.*;

// 
// A specialized AST node that does code generation for literals.
// The data type should be specified by the parser from the content.
// 

public class LiteralNode extends ExpressionNode
{
    public LiteralNode(Token t) 
    {
		super(t);
		setDataTypeFromContent();
    }

	private void setDataTypeFromContent()
	{
		switch (this.getToken().getType()) {

		case PlexilLexer.INT:
			this.setDataType(PlexilDataType.INTEGER_TYPE);
			break;

		case PlexilLexer.DOUBLE:
			this.setDataType(PlexilDataType.REAL_TYPE);
			break;

		case PlexilLexer.STRING:
			this.setDataType(PlexilDataType.STRING_TYPE);
			break;

		case PlexilLexer.TRUE_KYWD:
		case PlexilLexer.FALSE_KYWD:
			this.setDataType(PlexilDataType.BOOLEAN_TYPE);
			break;

			// internal data types

		case PlexilLexer.EXECUTING_STATE_KYWD:
		case PlexilLexer.FAILING_STATE_KYWD:
		case PlexilLexer.FINISHED_STATE_KYWD:
		case PlexilLexer.FINISHING_STATE_KYWD:
		case PlexilLexer.INACTIVE_STATE_KYWD:
		case PlexilLexer.ITERATION_ENDED_STATE_KYWD:
		case PlexilLexer.WAITING_STATE_KYWD:
			this.setDataType(PlexilDataType.NODE_STATE_TYPE);
			break;


		case PlexilLexer.SUCCESS_OUTCOME_KYWD:
		case PlexilLexer.FAILURE_OUTCOME_KYWD:
		case PlexilLexer.SKIPPED_OUTCOME_KYWD:
			this.setDataType(PlexilDataType.NODE_OUTCOME_TYPE);
			break;


		case PlexilLexer.PRE_CONDITION_FAILED_KYWD:
		case PlexilLexer.POST_CONDITION_FAILED_KYWD:
		case PlexilLexer.INVARIANT_CONDITION_FAILED_KYWD:
		case PlexilLexer.PARENT_FAILED_KYWD:
			this.setDataType(PlexilDataType.NODE_FAILURE_TYPE);
			break;

		case PlexilLexer.COMMAND_ABORTED_KYWD:
		case PlexilLexer.COMMAND_ABORT_FAILED_KYWD:
		case PlexilLexer.COMMAND_ACCEPTED_KYWD:
		case PlexilLexer.COMMAND_DENIED_KYWD:
		case PlexilLexer.COMMAND_FAILED_KYWD:
		case PlexilLexer.COMMAND_RCVD_KYWD:
		case PlexilLexer.COMMAND_SENT_KYWD:
		case PlexilLexer.COMMAND_SUCCESS_KYWD:
			this.setDataType(PlexilDataType.COMMAND_HANDLE_TYPE);
			break;

		case PlexilLexer.ARRAY_LITERAL:
			this.setDataType(PlexilDataType.UNKNOWN_ARRAY_TYPE);
			break;

		default:
			this.setDataType(PlexilDataType.ERROR_TYPE);
		}
	}


	/**
	 * @brief Persuade the expression to assume the specified data type
	 * @return true if the expression can consistently assume the specified type, false otherwise.
	 */
	protected boolean assumeType(PlexilDataType t, CompilerState myState)
	{
		if (m_dataType == t)
			return true;
		else if (m_dataType == PlexilDataType.INTEGER_TYPE
				 && t == PlexilDataType.REAL_TYPE) {
			// Promote to real
			m_dataType = t;
			return true;
		}
		else if (t == PlexilDataType.BOOLEAN_TYPE
				 && m_dataType == PlexilDataType.INTEGER_TYPE) {
			String txt = this.getToken().getText();
			if (txt.equals("0") || txt.equals("1")) {
				// OK to coerce it to boolean
				m_dataType = t;
				return true;
			}
			// else fall through to failure
		}
		else if (t.isArray()) {
			boolean success = true;
			if (m_dataType == PlexilDataType.UNKNOWN_ARRAY_TYPE) {
				// Can this array be coerced to the desired type?
				PlexilDataType eltType = t.arrayElementType();
				// Check that children are type consistent
				for (int childIdx = 0; childIdx < this.getChildCount(); childIdx++) {
					LiteralNode child = (LiteralNode) this.getChild(childIdx);
					if (!child.assumeType(eltType, myState)) {
						myState.addDiagnostic(child,
											  "Array element type " + child.m_dataType.typeName()
											  + " does not match required array literal element type " + t.typeName(),
											  Severity.ERROR);
						success = false;
					}
				}
				if (success) {
					m_dataType = t;
				}
			}
			// TODO: in all cases, perform semantic check of entire array
			return success;
		}

		// fall-through return
		return false;
	}


	public boolean checkTypeConsistency(NodeContext context, CompilerState myState)
	{
		boolean success = true;
		if (m_dataType == PlexilDataType.UNKNOWN_ARRAY_TYPE) {
			myState.addDiagnostic(this,
								  "Translator internal error: Array element type has not been resolved",
								  Severity.ERROR);
		}
        else if (m_dataType.isArray()) {
			// TODO: ???
		}
		else if (m_dataType == PlexilDataType.INTEGER_TYPE) {
			// TODO: format check
			// TODO: range check
		}
		else if (m_dataType == PlexilDataType.REAL_TYPE) {
			// TODO: range check
		}
		return success;
	}

	// Utility for use from various contexts
	static public int parseIntegerValue(String txt)
	{
		int radix = 10;
		// Look for prefixes
		if (txt.length() > 2
			&& txt.charAt(0) == '0'
			&& !isDigit(txt.charAt(1))) {
			switch (txt.charAt(1)) {
			case 'b':
			case 'B':
				// Binary
				radix = 2;
				txt = txt.substring(2);
				break;

			case 'o':
			case 'O':
				// Octal
				radix = 8;
				txt = txt.substring(2);
				break;

			case 'x':
			case 'X':
				// Hexadecimal
				radix = 16;
				txt = txt.substring(2);
				break;

			default:
				// *** should never get here ***
				System.err.println("In integer value \"" + txt
								   + "\": Unknown numeric prefix \"" + txt.substring(0, 2)
								   + "\", parsing as decimal");
			}
		}
		return Integer.parseInt(txt, radix);
	}

    public void constructXML()
    {
		super.constructXML();
        if (m_dataType.isArray()) {
			m_xml.setAttribute("Type", m_dataType.typeName());
			for (int childIdx = 0; childIdx < this.getChildCount(); childIdx++) {
				LiteralNode child = (LiteralNode) this.getChild(childIdx);
				m_xml.addChild(child.getXML());
            }
        }
        else {
			String txt = this.getToken().getText();
			if (getToken().getType() == PlexilLexer.INT) {
				m_xml.setContent(Integer.toString(parseIntegerValue(txt)));
			}
			else {
				m_xml.setContent(txt);
			}
        }
    }

	public String getXMLElementName()
	{
        if (m_dataType.isArray()) 
			return "ArrayValue";
		else
			return m_dataType.typeName() + "Value";
	}

	// Literal nodes do not support source locators.
	protected void addSourceLocatorAttributes() {}

	// *** is this still necessary?? ***
    public IXMLElement getXML(String elementType)
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
