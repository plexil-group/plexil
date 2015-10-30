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

import org.xml.sax.Attributes;

import static gov.nasa.luv.Constants.*;

/**
 * The NodeStateUpdateHandler class provides mothods for handling when the 
 * Plexil Plan is executing and sending updates to properties of the Plexil Model.
 */

public class NodeStateUpdateHandler
    extends AbstractDispatchableHandler
{
    //* Top level XML tag name registered with DispatchHandler
    public static final String NODE_STATE_UPDATE = "NodeStateUpdate";

    // XML tags
    private static final String NODE_ID = "NodeId";

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
        conditions = new HashMap<String, String>();

        setElementMap(new HashMap<String, LuvElementHandler>() {
                {
                    put(NODE_ID, new LuvElementHandler() {
                            public void elementEnd(String tagName, String tweenerText) {
                                Node candidate = current.findChildByName(tweenerText);
                                if (candidate != null)
                                    current = candidate;
                            }
                        });
                    put(NODE_STATE, new LuvElementHandler() {
                            public void elementEnd(String tagName, String tweenerText) {
                                state = tweenerText;
                            }
                        });
                    put(NODE_OUTCOME, new LuvElementHandler() {
                            public void elementEnd(String tagName, String tweenerText) {
                                outcome = tweenerText;
                            }
                        });
                    put(NODE_FAILURE_TYPE, new LuvElementHandler() {
                            public void elementEnd(String tagName, String tweenerText) {
                                failureType = tweenerText;
                            }
                        });
                    put(NODE_STATE_UPDATE, new LuvElementHandler() {
                            public void elementStart(String tagName, Attributes attributes) {
                                reset();
                            }
                            public void elementEnd(String tagName, String tweenerText) {
                                if (!state.equals(current.getState()))
                                    current.setState(state);
            
                                if (!outcome.equals(UNKNOWN) || current.getOutcome() == null)
                                    current.setOutcome(outcome);
            
                                if (!failureType.equals(UNKNOWN) || current.getFailureType() == null)
                                    current.setFailureType(failureType);
          
                                if (current.hasConditions()) {
                                    for (Map.Entry<String, String> condition: conditions.entrySet()) {
                                        if (current.hasCondition(condition.getKey()))
                                            current.setProperty(condition.getKey(), condition.getValue());
                                    }
                                }
                            }
                        });

                    LuvElementHandler conditionHandler = new LuvElementHandler() {
                            public void elementEnd(String tagName, String tweenerText) {
                                conditions.put(tagName, tweenerText);
                            }
                        };
                    for (String c: PlexilSchema.ALL_CONDITIONS)
                        put(c, conditionHandler);
                }
            });
    }

    /**
     * Handles the end of the state update document.
     */
    public void endDocument() {
        // pause if single stepping
        if (Luv.getLuv().getPlanStep()) {
            Luv.getLuv().pausedState();
        }
    }

    private void reset()
    {
        current = Model.getRoot();
        state = null;
        outcome = null;
        failureType = null;
        conditions.clear();
    }
}
