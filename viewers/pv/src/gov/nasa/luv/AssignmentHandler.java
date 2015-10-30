/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

import org.xml.sax.Attributes;

import static gov.nasa.luv.PlexilSchema.*;

/**
 * The AttributeUpdateHandler class provides mothods for handling when the
 * Plexil Plan is executing and a local variable changes.
 */

public class AssignmentHandler
    extends AbstractDispatchableHandler {

    // Public constant

    //* Top level XML tag name registered with DispatchHandler
    public static final String ASSIGNMENT = "Assignment";

    //
    // Local constants
    //

    // XML tags
    private static final String NODE_ID = "NodeId";
    private static final String VARIABLE_NAME = "VariableName";
    private static final String VALUE = "Value";

	private Node current;
	private String vName;
	private String value;

	public AssignmentHandler() {
		super();

        setElementMap(new java.util.TreeMap<String, LuvElementHandler>() {
                {
                    put(NODE_ID, new LuvElementHandler() {
                            public void elementEnd(String tagName, String tweenerText) {
                                Node candidate = Model.getRoot().findChildByName(tweenerText); // *** FIXME ***
                                if (candidate != null) 
                                    current = candidate;
                            }
                        });
                    put(VARIABLE_NAME, new LuvElementHandler() {
                            public void elementEnd(String tagName, String tweenerText) {
                                vName = tweenerText;
                            }
                        });
                    put(VALUE, new LuvElementHandler() {
                            public void elementEnd(String tagName, String tweenerText) {
                                value = tweenerText;
                            }
                        });
                    put(ASSIGNMENT, new LuvElementHandler() {
                            public void elementStart(String tagName, Attributes attributes) {
                                reset();
                            }
                            public void elementEnd(String tagName, String tweenerText) {
                                if (vName != null && current != null) {
                                    current.setVariable(vName, value);
                                }
                            }
                        });
                }
            });
	}

    private void reset() {
        current = null;
        vName = null;
        value = null;
    }

}
