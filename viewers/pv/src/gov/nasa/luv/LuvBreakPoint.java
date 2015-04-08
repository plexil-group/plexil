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

/** 
 * The LuvBreakPoint class is an abstract class that provides methods for 
 * breaking action in the event that a breakpoint is signaled.  Derived classes 
 * are expected to provide the conditions underwich the break is eligible to fire. 
 */

public class LuvBreakPoint
    extends Node.ChangeAdapter {

    private Node.PropertyChangeFilter filter;

    /** enabled state of this breakpoint jsdhcb */
    private boolean enabled = true;

    private boolean breakStatus = false;

    // The following are protected for the convenience of derived classes.

    /** the node on which the break point operates */
    protected Node node;
    
    /** Old value, use to test for changes. */
    protected String oldValue = "";

    /** 
     * Constructs a Luv specific break point with the specified Plexil node.
     *
     * @param node the node on which the break point operates
     */
    public LuvBreakPoint(Node node, Node.PropertyChangeFilter f) {
        filter = f;
        setNode(node);
    }

    /** {@inheritDoc} */
      
    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    /** {@inheritDoc} */

    public boolean isEnabled() {
        return enabled;
    }

    public String toString() {
        return node.getNodeName() + filter.getDescription();
    }

    /** detatch from node */
    // N.B. this is initiated from GUI on node
    public void unregister() {
        if (node != null)
            node.removeBreakPoint(this);
        node = null;
    }

    /** {@inheritDoc} */

    public Node getNode() {
        return node;
    }

    /** {@inheritDoc} */
    public void onBreak() {
        Luv.getLuv().pausedState();
        Luv.getLuv().getLuvBreakPointHandler().setActiveBreakPoint(this);
    }

    /** {@inheritDoc} */

    public void setNode(Node n) {
        // unregister from previous node, if any
        unregister();

        // assign the new node
        node = n;
        node.addBreakPoint(this);
    }
    
    public void reserveBreakStatus(boolean value) {
        breakStatus = value;
    }

    public boolean getReserveBreakStatus() {
        return breakStatus;
    }

    //
    // Node.ChangeListener API
    //

    public void propertyChange(Node node, String property) {
        if (!enabled)
            return;
        if (filter.eventMatches(node, property))
            onBreak();
    }

}
