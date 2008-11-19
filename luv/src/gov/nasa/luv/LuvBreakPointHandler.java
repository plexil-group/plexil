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

import java.awt.Color;
import javax.swing.JMenu;
import javax.swing.JSeparator;
import javax.swing.JPopupMenu;

import java.util.Vector;
import java.util.HashMap;
import java.util.Map;

import java.awt.event.ActionEvent;

import static gov.nasa.luv.Constants.*;

public class LuvBreakPointHandler 
{
    
      // breakpoint variables
      
      private BreakPoint                        breakPoint = null;                                     // if break has occured, the causal break point object
      private HashMap<BreakPoint, ModelPath>    breakPointMap = new HashMap<BreakPoint, ModelPath>();  // collection of all breakpoints
      private Vector<BreakPoint>                unfoundBreakPoints = new Vector<BreakPoint>();         // breakpoints not found in current plan   
      private Vector<String>                    breakPointList = new Vector<String>();                 // list of breakpoints to check for repeats
      
      public void setBreakPoint(BreakPoint bp)
      {
          breakPoint = bp;
      }
      
      public BreakPoint getBreakPoint()
      {
          return breakPoint;
      }
      
      public String getBreakPointNodeName()
      {
          return breakPoint.getModel().getModelName();
      }
      
      public HashMap<BreakPoint, ModelPath> getBreakPointMap()
      {
          return breakPointMap;
      }
      
      public Vector<BreakPoint> getUnfoundBreakPoints()
      {
          return unfoundBreakPoints;
      }
      
      public void clearBreakPoint()
      {
          breakPoint = null;
      }
      
      public boolean breakpointsExist()
      {
          return !breakPointList.isEmpty();
      }
      
      /** Add breakpoint to grand list of breakpoints.
       *
       * @param breakPoint breakpoint to add
       * @param model model breakpoint is associated with
       */

      public void addBreakPoint(BreakPoint breakPoint, Model model)
      {
         ModelPath mp = new ModelPath(model);
         
         if (!breakPointList.contains(breakPoint.toString()))
         {
             breakPointList.add(breakPoint.toString());

             breakPointMap.put(breakPoint, mp);
             
             Luv.getLuv().enableRemoveBreaksMenuItem(true);

             Luv.getLuv().showStatus("Added break on " + breakPoint, 5000l);
             Luv.getLuv().getViewHandler().refreshView();       
         }
         else
             Luv.getLuv().showStatus("\"" + breakPoint + "\" breakpoint has already been added", Color.RED, 5000l);
      }

      /** Remove breakpoint from grand list of breakpoints.
       *
       * @param breakPoint breakpoint to remove
       */

      public void removeBreakPoint(BreakPoint breakPoint)
      {
         breakPoint.unregister();
         breakPointMap.remove(breakPoint);
         breakPointList.remove(breakPoint.toString());
         
         if (!breakpointsExist())
             Luv.getLuv().enableRemoveBreaksMenuItem(false);
         
         Luv.getLuv().showStatus("Removed break on " + breakPoint, 5000l);
         Luv.getLuv().getViewHandler().refreshView();
      }

      /** Remove all breakpoints from grand list of breakpoints. */

      public void removeAllBreakPoints()
      {
         for (BreakPoint bp: breakPointMap.keySet())
            bp.unregister();
         breakPointMap.clear();
         breakPointList.clear();
         unfoundBreakPoints.clear();
         breakPoint = null;
         Luv.getLuv().enableRemoveBreaksMenuItem(false);
         Luv.getLuv().getViewHandler().refreshView();
      }
      
      /** Create a breakpoint which fires when the model state
       * changes.
       *
       * @param model the model to watch for state changes
       */
      
      public BreakPoint createChangeBreakpoint(Model model)
      {
         return new LuvBreakPoint(model, MODEL_STATE)
            {
                  public boolean isBreak()
                  {
                     return !model.getProperty(MODEL_STATE).equals(oldValue);
                  }
                  
                  public void onBreak()
                  {
                     reason = model.getModelName() + 
                        " changed from " + oldValue +
                        " to " + model.getProperty(MODEL_STATE) + ".";
                     oldValue = model.getProperty(MODEL_STATE);
                     super.onBreak();
                  }

                  public String toString()
                  {
                     return model.getModelName();
                  }
         };
      }
      
      // map all the breakpoints into the new model
      
      public void mapBreakPointsToNewModel(Model model)
      {
	    getUnfoundBreakPoints().clear();
         
	    for (Map.Entry<BreakPoint, ModelPath> pair: getBreakPointMap().entrySet()) 
            {
		BreakPoint bp = pair.getKey();
		ModelPath path = pair.getValue();

		Model target = path.find(model);
		if (target != null)
                {
                    bp.setModel(target);
		}
		else
		    getUnfoundBreakPoints().add(bp);
	    }
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
      
      /** Construct a node popup menu on the fly given the it is associated with.
       *
       * @param model model that this popup is about
       * @return the freshly constructed popup menue
       */

      public JPopupMenu constructNodePopupBreakPointMenu(final Model model)
      {
         // get the model name

         final String name = model.getModelName();

         // construct the node popup menu
         
         JPopupMenu popup = new JPopupMenu("Node Popup Menu");

         // add node state change breakpoint

         popup.add(new LuvAction(
                      "Add Break Point for " + name + " State Change",
                      "Add a break point any time " + name + " changes state.")
            {
                  public void actionPerformed(ActionEvent e)
                  {
                     createChangeBreakpoint(model);
                  }
            });
         
         // add target state break points menu

         JMenu stateMenu = new JMenu(
            "Add Break Point for " + name + " State");
         stateMenu.setToolTipText(
            "Add a break point which pauses execution when " + name + 
            " reaches a specified state.");
         popup.add(stateMenu);
         for (final String state: NODE_STATES)
            stateMenu.add(new LuvAction(
                             state,
                             "Add a break point when " + name + 
                             " reaches the " + state + " state.")
               {
                     public void actionPerformed(ActionEvent e)
                     {
                        createTargetPropertyValueBreakpoint(
                           model, name + " state", MODEL_STATE, state);
                     }
               });

         // add target outcome break points menu

         JMenu outcomeMenu = new JMenu(
            "Add Break Point for " + name + " Outcome");
         outcomeMenu.setToolTipText(
            "Add a break point which pauses execution when " + name + 
            " reaches a specified outcome.");
         popup.add(outcomeMenu);
         for (final String outcome: NODE_OUTCOMES)
            outcomeMenu.add(new LuvAction(
                               outcome,
                               "Add a break point when " + name + 
                               " reaches the " + outcome + " outcome.")
               {
                     public void actionPerformed(ActionEvent e)
                     {
                        createTargetPropertyValueBreakpoint(
                           model, name + " outcome", MODEL_OUTCOME, outcome);
                     }
               }); 

         // add target failure type break points menu

         JMenu failureTypeMenu = new JMenu(
            "Add Break Point for " + name + " Failure Type");
         failureTypeMenu.setToolTipText(
            "Add a break point which pauses execution when " + name + 
            " reaches a specified failure type.");
         popup.add(failureTypeMenu);
         for (final String failureType: NODE_FAILURE_TYPES)
            failureTypeMenu.add(new LuvAction(
                                   failureType,
                                   "Add a break point when " + name + 
                                   " reaches the " + failureType +
                                   " failure type.")
               {
                     public void actionPerformed(ActionEvent e)
                     {
                        createTargetPropertyValueBreakpoint(
                           model, name + " failure type", 
                           MODEL_FAILURE_TYPE, failureType);
                     }
               }); 

         // get the break points for this model
         
         final Vector<BreakPoint> bps = getBreakPoints(model);

         // if we got any add enable/disable & remove item for each one

         if (bps.size() > 0 && !Luv.getLuv().getIsExecuting())
         {
            // add the breakpoints

            popup.add(new JSeparator());
            for (final BreakPoint bp: bps)
            {
               String action = bp.isEnabled() ? "Disable" : "Enable";
               popup.add(new LuvAction(
                            action + " " + bp,
                            action + " the breakpoint " + 
                            bp + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           bp.setEnabled(!bp.isEnabled());
                           Luv.getLuv().getViewHandler().refreshView();
                        }
                  }); 
            }

            // add the breakpoints

            popup.add(new JSeparator());
            for (final BreakPoint bp2: bps)
            {
               popup.add(new LuvAction(
                            "Remove " + bp2,
                            "Permanently remove the breakpoint " + 
                            bp2 + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           removeBreakPoint(bp2);
                        }
                  }); 
            }
         }

         // if there is more then one break point add a remove all item

         if (bps.size() > 1)
         {
            // add the remove all action
            
            popup.add(new JSeparator());
            popup.add(new LuvAction(
                         "Remove All Break Points From " + name,
                         "Permanently remove all breakpoints from " + name + ".")
               {
                     public void actionPerformed(ActionEvent e)
                     {
                        for (final BreakPoint bp3: bps)
                           removeBreakPoint(bp3);
                     }
               });
         }

         // return our freshly created popup menu

         return popup;
      }

      /** Return all the breakpoints for a given model. */

      public Vector<BreakPoint> getBreakPoints(Model model)
      {
         Vector<BreakPoint> bps = new Vector<BreakPoint>();
         for (BreakPoint bp: breakPointMap.keySet())
            if (model == bp.getModel())
               bps.add(bp);
         return bps;
      }
}
