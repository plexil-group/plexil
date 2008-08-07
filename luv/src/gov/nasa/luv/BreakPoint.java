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

/** An interface to support breakpoints in the plan. */

public interface BreakPoint
{
      /** Test if conditions are correct for a break to occur.  This
       * expected to only be called when state of the unerlying system
       * has chane.
       *
       * @return true if a break should occur
       */

      public boolean isBreak();

      /** Signal that a break has occurred. */

      public void onBreak();

      /** Set enabled state of this breakpoint.
       *
       * @param enabled true if this breakpoint is enabled
       */

      public void setEnabled(boolean enabled);

      /** Get enabled state of this breakpoint.
       *
       * @return true if break point is enabled
       */

      public boolean isEnabled();

      /** Return the reason that this break occurred. 
       *
       * @return the reason the break occurred
       */

      public String getReason();

      /** Set the break point model.
       * 
       * @param model the model to test for breaks
       */

      public void setModel(Model model);

      /** Get the break point model.
       * 
       * @return the model to test for breaks
       */

      public Model getModel();

      /** Disconnect breakpoint from any world events. */

      public void unregister();
 }
