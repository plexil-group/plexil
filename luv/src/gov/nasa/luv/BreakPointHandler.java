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

import static gov.nasa.luv.Constants.*;

import java.awt.Color;
import java.util.HashMap;
import java.util.Vector;

public class BreakPointHandler 
{

      /** if break has occured, the causal break point object */

      BreakPoint breakPoint = null;

      /** a collect of all breakpoints */

      HashMap<BreakPoint, ModelPath> breakPoints = new HashMap<BreakPoint, ModelPath>();
      
      /** a collect of all breakpoints */

      HashMap<String, Integer> checkBreakPoints = new HashMap<String, Integer>();

      /** breakpoints not found in this plan */

      Vector<BreakPoint> unfoundBreakPoints = new Vector<BreakPoint>();

      /** Return all the breakpoints for a given model. */

      public Vector<BreakPoint> getBreakPoints(Model model)
      {
         Vector<BreakPoint> bps = new Vector<BreakPoint>();
         for (BreakPoint breakPoint: breakPoints.keySet())
            if (model == breakPoint.getModel())
               bps.add(breakPoint);
         return bps;
      }

      /** Add breakpoint to grand list of breakpoints.
       *
       * @param breakPoint breakpoint to add
       * @param model model breakpoint is associated with
       */

      public void addBreakPoint(BreakPoint breakPoint, Model model)
      {
          // check if this breakpoint has already been set before adding
          
          if (!checkBreakPoints.containsKey(breakPoint.toString()))
          {
             checkBreakPoints.put(breakPoint.toString(), null);
              
             ModelPath mp = new ModelPath(model);
             breakPoints.put(breakPoint, mp);
             Luv.statusBarHandler.showStatus("Added break on " + breakPoint, 5000l);
             Luv.luvViewerHandler.refreshView();
          }
      }

      /** Remove breakpoint from grand list of breakpoints.
       *
       * @param breakPoint breakpoint to remove
       */

      public void removeBreakPoint(BreakPoint breakPoint)
      {
         breakPoint.unregister();
         breakPoints.remove(breakPoint);
         Luv.statusBarHandler.showStatus("Removed break on " + breakPoint, 5000l);
         Luv.luvViewerHandler.refreshView();
      }

      /** Remove all breakpoints from grand list of breakpoints. */

      public void removeAllBreakPoints()
      {
         for (BreakPoint breakPoint: breakPoints.keySet())
            breakPoint.unregister();
         breakPoints.clear();

         Luv.statusBarHandler.showStatus("Removed all breakponts.", 5000l);
         Luv.luvViewerHandler.refreshView();
      }
      
      /** Create a breakpoint which fires when the model state
       * changes.
       *
       * @param model the model to watch for state changes
       */
      
      public BreakPoint createChangeBreakpoint(Model model)
      {
         return new LuvBreakPoint(model, Constants.MODEL_STATE)
            {
                  public boolean isBreak()
                  {
                     return !model.getProperty(Constants.MODEL_STATE).equals(oldValue);
                  }
                  
                  public void onBreak()
                  {
                     reason = model.getProperty(Constants.MODEL_NAME) + 
                        " changed from " + oldValue +
                        " to " + model.getProperty(Constants.MODEL_STATE) + ".";
                     oldValue = model.getProperty(Constants.MODEL_STATE);
                     super.onBreak();
                  }

                  public String toString()
                  {
                     return model.getProperty(Constants.MODEL_NAME) +
                        " state changed";
                  }
         };
      }

      /** Create a breakpoint which fires when the model state
       * changes to a specifed state.
       *
       * @param model the model to watch for state changes
       * @param propertyTitle printed name of property
       * @param targetProperty property to watch for break
       * @param targetValue value to watch for break
       */
      
      public BreakPoint createTargetPropertyValueBreakpoint(
         Model model, final String propertyTitle, 
         final String targetProperty, final String targetValue)
      {
         return new LuvBreakPoint(model, targetProperty)
            {
                  public boolean isBreak()
                  {
                     String newValue = model.getProperty(targetProperty);
                     if (newValue != null && 
                         !newValue.equals(oldValue) && 
                         newValue.equals(targetValue))
                     {
                        return true;
                     }
                     else 
                        oldValue = newValue;
                     
                     return false;
                  }
                  
                  public void onBreak()
                  {
                     reason = propertyTitle + " changed to " + 
                        model.getProperty(targetProperty) + ".";
                     oldValue = model.getProperty(targetProperty);
                     super.onBreak();
                  }

                  public String toString()
                  {
                     return propertyTitle + " changed to " + targetValue;
                  }
         };
      }

      public boolean doesViewerBlock()
      {
         String blocksStr = Luv.getLuv().model.getProperty(VIEWER_BLOCKS);
         boolean blocks = blocksStr != null 
            ? Boolean.valueOf(blocksStr)
            : false;

         if (blocks && Luv.getLuv().planPaused && !Luv.getLuv().planStep) 
         {
            Luv.statusBarHandler.showStatus((Luv.breakPointHandler.breakPoint == null
                        ? "Plan execution paused."
                        : Luv.breakPointHandler.breakPoint.getReason()) +
               "  Hit " + 
               Luv.actionHandler.pauseAction.getAcceleratorDescription() +
               " to resume, or " + 
               Luv.actionHandler.stepAction.getAcceleratorDescription() +
               " to step.",
               Color.RED);
            Luv.breakPointHandler.breakPoint = null;

            while (Luv.getLuv().planPaused && !Luv.getLuv().planStep)
            {
               try
               {
                  Thread.sleep(50);
               }
               catch (Exception e)
               {
                  e.printStackTrace();
               }
            }
         }

         Luv.getLuv().planStep = false;

         return blocks;
      }
      
      public void pauseAtStart() 
      {
          if (Luv.pauseAtStart)
          {
              if (!Luv.executedViaLuvViewer)
              {
                  Boolean reset = false;
                  
                  if (!Luv.allowBreaks)
                  {
                      Luv.resetAllowBreaks = true;
                      Luv.allowBreaks = true;
                  }
                  
                  Luv.getLuv().planPaused = true;
                  if (Luv.getLuv().model.getProperty(VIEWER_BLOCKS) == null ||
                      Luv.getLuv().model.getProperty(VIEWER_BLOCKS).equals(FALSE) ||
                      Luv.getLuv().model.getProperty(VIEWER_BLOCKS).equals("false"))
                  {
                      if (!Luv.allowBreaks)
                          Luv.actionHandler.allowBreaksAction.actionPerformed(null);
                      Luv.getLuv().model.setProperty(VIEWER_BLOCKS, TRUE);
                      reset = true;
                  }

                  Luv.breakPointHandler.doesViewerBlock();                          

                  if (reset)
                      Luv.getLuv().model.setProperty(VIEWER_BLOCKS, FALSE);
              }
              Luv.pauseAtStart = false;
          }          
      }
}
