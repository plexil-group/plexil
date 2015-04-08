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

import java.util.HashMap;
import java.util.Map;
import static gov.nasa.luv.Constants.*;

/**
 * The NodeStateUpdateHandler class provides mothods for handling when the 
 * Plexil Plan is executing and sending updates to properties of the Plexil Model.
 */

public class NodeStateUpdateHandler extends AbstractDispatchableHandler
{
    private Node current;
    private String state;
    private String outcome;
    private String failureType;
    private HashMap<String, String> conditions;

    /**
     * Constructs a NodeStateUpdateHandler.
     */
    public NodeStateUpdateHandler() {
        super();
        current = Model.getRoot();
        conditions = new HashMap<String, String>();
    }

    /**
     * Handle end of an XML element or an update to an element in the Plexil Plan.
     * @param uri N/A
     * @param localName the name of the XML tag
     * @param qName N/A
     */
    public void endElement(String uri, String localName, String qName)
    {
        // get text between tags
        String text = getTweenerText();

        // if this is the id of a path element, move down model tree
        if (localName.equals(NODE_ID)) {
            Node candidate;
            if ((candidate = current.findChildByName(text)) != null) {
                current = candidate;
            }
        }

        // if this is the node state, record the state
        else if (localName.equals(NODE_STATE))
            state = text;

        // if this is the node outcome, record the outcome
        else if (localName.equals(NODE_OUTCOME))
            outcome = text;

        // if this is the node failure type, record the failure type
        else if (localName.equals(NODE_FAILURE_TYPE))
            failureType = text;        

        // if this is the node state update, update the node state
        else if (localName.equals(NODE_STATE_UPDATE)) {
            if (!state.equals(current.getState()))
                current.setState(state);
            
            if (!outcome.equals(UNKNOWN) || current.getOutcome() != null)
                current.setOutcome(outcome);
            
            if (!failureType.equals(UNKNOWN) || current.getFailureType() != null)
                current.setFailureType(failureType);
          
            if (current.hasConditions()) {
                for (Map.Entry<String, String> condition: conditions.entrySet()) {
                    if (current.hasCondition(condition.getKey()))
                        current.setProperty(condition.getKey(), condition.getValue());
                }
            }
        }

        // if this is one of the conditions, record it
        else if (localName.endsWith("Condition"))
            conditions.put(localName, text);
    }

    /**
     * Handles the end of the state update document.
     */
    public void endDocument() {
        // Reset to root of model
        current = Model.getRoot();

        // pause if single stepping
        if (Luv.getLuv().getPlanStep()) {
            Luv.getLuv().pausedState();
        }
    }
}
