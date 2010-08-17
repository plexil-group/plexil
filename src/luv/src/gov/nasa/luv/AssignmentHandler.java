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

package gov.nasa.luv;

import javax.swing.JOptionPane;

import org.xml.sax.Attributes;

/**
 * The AttributeUpdateHandler class provides mothods for handling when the
 * Plexil Plan is executing and a local variable changes.
 */

public class AssignmentHandler extends AbstractDispatchableHandler {
	private Model current;
	private String vName;
	private String value;

	/**
	 * Constructs a NodeStateUpdateHandler.
	 */
	public AssignmentHandler() {
		super();
		current = Model.getRoot();
	}

	/**
	 * Handle end of an XML element
	 * 
	 * @param uri
	 *            N/A
	 * @param localName
	 *            the name of the XML tag
	 * @param qName
	 *            N/A
	 */
	public void endElement(String uri, String localName, String qName) {
		String text = getTweenerText();
		// get text between tags
		if (Constants.NODE_ID.equals(qName)) {
			Model candidate = null;
			if ((candidate = current.findChildByName(text)) != null) {
				current = candidate;
			}
		} else if (Constants.VARIABLE_NAME.equals(qName)) {
			vName = text;
		} else if (Constants.VALUE.equals(qName)) {
			value = text;
		}
	}

	/**
	 * Handles the end of the state update document.
	 */
	public void endDocument() {
		// set the variable to the given value
		if (vName != null) {
			current.setVariable(vName, value);
		}
		// Reset to root of model
		current = Model.getRoot();
	}
}
