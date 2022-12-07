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

import java.time.Duration;
import java.time.Instant;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.time.OffsetDateTime;
import java.time.OffsetTime;

import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;
import java.time.format.ResolverStyle;

import java.time.temporal.TemporalAccessor;

import org.antlr.runtime.*;
import org.antlr.runtime.tree.*;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

// 
// A specialized AST node that does code generation
// for literals representing dates and durations.
// 

public class TemporalLiteralNode extends LiteralNode
{
    public TemporalLiteralNode(Token t)
    {
        super(t);
    }
    
    public TemporalLiteralNode(TemporalLiteralNode n)
    {
        super(n);
    }

	public Tree dupNode()
	{
		return new TemporalLiteralNode(this);
	}

    // Overrides method on PlexilTreeNode
    @Override
    public void earlyCheck(NodeContext context, CompilerState state)
    {
        super.earlyCheck(context, state);

        // Do syntax checking on children as required
        switch (this.getType()) {

        case PlexilLexer.DATE_LITERAL:
        case PlexilLexer.DATE_KYWD:
            if (this.getChild(0).getType() == PlexilLexer.STRING) {
                String val = ((StringLiteralNode) this.getChild(0)).getStringValue();
                if (!val.isEmpty()) {
                    try {
                        DateTimeFormatter parser =
                            DateTimeFormatter.ISO_DATE_TIME.withResolverStyle(ResolverStyle.LENIENT);
                        TemporalAccessor protodate =
                            parser.parseBest(val,
                                             Instant::from,
                                             OffsetDateTime::from,
                                             LocalDateTime::from,
                                             OffsetTime::from,
                                             LocalTime::from,
                                             LocalDate::from);
                    } catch (DateTimeParseException dte) {
                        state.addDiagnostic(this.getChild(0),
                                            "Date literal \"" + val
                                            + "\" is not a legal ISO 8601 date-time value:\n"
                                            + dte.toString(),
                                            Severity.ERROR);
                    }
                }
            }
            else {
                // Parser error, or I don't know what I'm doing
                state.addDiagnostic(this.getChild(0),
                                    "Internal error: Duration literal value is not a String",
                                    Severity.FATAL);
            }
            break;

        case PlexilLexer.DURATION_LITERAL:
        case PlexilLexer.DURATION_KYWD:
            if (this.getChild(0).getType() == PlexilLexer.STRING) {
                String val = ((StringLiteralNode) this.getChild(0)).getStringValue();
                if (!val.isEmpty()) {
                    try {
                        Duration dur = Duration.parse(val);
                    } catch (DateTimeParseException dte) {
                        state.addDiagnostic(this.getChild(0),
                                            "Duration literal \"" + val
                                            + "\" is not a legal ISO 8601 duration value",
                                            Severity.ERROR);
                    }
                }
            }
            else {
                // Parser error, or I don't know what I'm doing
                state.addDiagnostic(this.getChild(0),
                                    "Internal error: Duration literal value is not a String",
                                    Severity.FATAL);
            }
            break;

            // Extend later?
        default:
            break;
        }
    }

    // Override LiteralNode method
    @Override
    protected void earlyCheckSelf(NodeContext context, CompilerState state)
    {
        switch (this.getType()) {

        case PlexilLexer.DATE_LITERAL:
        case PlexilLexer.DATE_KYWD:
            m_dataType = PlexilDataType.DATE_TYPE;
            break;

        case PlexilLexer.DURATION_LITERAL:
        case PlexilLexer.DURATION_KYWD:
            m_dataType = PlexilDataType.DURATION_TYPE;
            break;

        default:
            // debug aid
            state.addDiagnostic(this,
                                "Internal error: TemporalLiteralNode has no data type for \""
                                + this.getText() + "\"",
                                Severity.FATAL);
            m_dataType = PlexilDataType.ERROR_TYPE;
            break;
        }
    }

    /**
     * @brief Persuade the expression to assume the specified data type
     * @return true if the expression can consistently assume the specified type, false otherwise.
     */
    @Override
    protected boolean assumeType(PlexilDataType t, CompilerState state)
    {
        // If our type is null, void, or error, fail.
        if (!PlexilDataType.isValid(m_dataType))
            return false;

        // If target type is Void, Error, or underspec'd array, fail.
        if (!PlexilDataType.isValid(t)) {
            state.addDiagnostic(null,
                                "Internal error: TemporalLiteralNode.assumeType called with invalid type "
                                + t,
                                Severity.FATAL);
            return false;
        }

        // If we are already the right type, succeed.
        if (m_dataType == t)
            return true;

        // If target type is Any, succeed.
        if (t == PlexilDataType.ANY_TYPE)
            return true;

        // If target type is Real, succeed.
        if (t == PlexilDataType.REAL_TYPE)
            return true;

        // else fail
        return false;
    }

    // Override LiteralNode method
    @Override
    protected void constructXML(Document root)
    {
        super.constructXMLBase(root);
        m_xml.appendChild(root.createTextNode(((StringLiteralNode) this.getChild(0)).getStringValue()));
    }

}
