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

/** 
 * The LuvBreakPoint class is an abstract class that provides methods for 
 * breaking action in the event that a breakpoint is signaled.  Derived classes 
 * are expected to provide the conditions underwich the break is eligible to fire. 
 */

public abstract class LuvBreakPoint extends AbstractBreakPoint
{
    private boolean breakStatus = false;
    
    /** Old value, use to test for changes. */
    public String oldValue = "";

    /** Target model property to watch. */
    private String targetProperty;

    /** Storage for reason for current break point occurrance. */
    public String reason = "NO break has occurred yet. This should NEVER be seen!";

    /** 
     * Constructs a Luv specific break point with the specified Plexil model
     * and target property.
     *
     * @param model the model on which the break point operates
     * @param targetProperty the model property on whgich to watch for
     */
    public LuvBreakPoint(Model model, String targetProperty)
    {
       super(model);
       this.targetProperty = targetProperty;
       oldValue = model.getProperty(targetProperty);
       Luv.getLuv().getLuvBreakPointHandler().addBreakPoint(this, model);
    }

    /** {@inheritDoc} */

    public void onBreak()
    {
       Luv.getLuv().getLuvStateHandler().pausedState();
       Luv.getLuv().getLuvBreakPointHandler().setBreakPoint(this);
       oldValue = model.getProperty(targetProperty);
    }

    /** {@inheritDoc} */

    public String getReason()
    {
       return reason;
    }

    /** {@inheritDoc} */

    public void setModel(Model model)
    {
       super.setModel(model);
       if (targetProperty != null)
          oldValue = model.getProperty(targetProperty);
    }
    
    public void reserveBreakStatus(boolean value)
    {
        breakStatus = value;
    }

    public boolean getReserveBreakStatus()
    {
        return breakStatus;
    }
}
