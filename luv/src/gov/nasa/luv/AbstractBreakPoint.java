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

/** An abstract break point that provides some basic enablement
 * functionality, and model storage. */

public abstract class AbstractBreakPoint implements BreakPoint
{
      /** enabeld state of this breakpoint jsdhcb */

      boolean enabled = true;

      /** the model on which the break point operates */

      Model model;

      /** change listener for the model */

      Model.ChangeListener listener;

      /** Construct an abstract break point. 
       *
       * @param model the model on which the break point operates
       */

      public AbstractBreakPoint(Model model)
      {
         setModel(model);
      }

      /** { @inheritDoc } */

      public void setEnabled(boolean enabled)
      {
         this.enabled = enabled;
      }

      /** { @inheritDoc } */

      public boolean isEnabled()
      {
         return enabled;
      }

      /** { @inheritDoc } */

      public void unregister()
      {
         // remove listener from model

         if (listener != null)
            model.removeChangeListener(listener);
      }

      /** { @inheritDoc } */

      public void setModel(Model model)
      {
         // unregister breakpoint

         unregister();

         // assign new listener
         
         listener = new Model.ChangeAdapter()
            {
                  @Override public void propertyChange(Model model, String property)
                  {
                     if (isEnabled() && isBreak())
                        onBreak();
                  }
            };
         model.addChangeListener(listener);

         // assign the new model

         this.model = model;
      }

      /** { @inheritDoc } */

      public Model getModel()
      {
         return model;
      }
}
