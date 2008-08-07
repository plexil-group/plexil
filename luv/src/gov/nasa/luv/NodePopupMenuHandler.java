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

import java.awt.event.ActionEvent;
import java.util.Vector;
import javax.swing.JMenu;
import static gov.nasa.luv.Constants.*;

import javax.swing.JPopupMenu;
import javax.swing.JSeparator;

public class NodePopupMenuHandler 
{
      JMenu stateMenu;
      JMenu outcomeMenu;
      JMenu failureTypeMenu;
      JPopupMenu popup;
      
      static int anyStateChangeCount;
      static int targetStateChangeCount;
      static int targetOutcomeCount;
      static int failureTypeCount;
              
      public NodePopupMenuHandler() 
      {
          anyStateChangeCount = 0;
          targetStateChangeCount = 0;
          targetOutcomeCount = 0;
          failureTypeCount = 0;
      }
      
      /** Construct a node popup menu on the fly given the it is
       * associated with.
       *
       * @param model model that this popup is about
       * @return the freshly constructed popup menue
       */

      public JPopupMenu constructNodePopupMenu(final Model model)
      {
         // get the model name

         final String name = model.getProperty(MODEL_NAME);
         
         // get the break points for this model
         
         final Vector<BreakPoint> BreakPoints = Luv.breakPointHandler.getBreakPoints(model);

         // construct the node popup menu
         
         popup = new JPopupMenu(NODE_POPUP_MENU);         
         setInitialPopupContent(name, model);

         // add enable/disable & remove item for each breakpoint
         
         if (!BreakPoints.isEmpty())
             updateBreakpointInfo(BreakPoints, name);        

         return popup;
      }
      
         public void setInitialPopupContent(final String name, final Model model) 
         {
             // add node state change breakpoint
             
             setAnyStateChangeBreakpoint(name, model);
             setTargetStateChangeBreakpoint(name, model);
             setTargetOutcomeBreakpoint(name, model);
             setFailureTypeBreakpoint(name, model);
         }
         
         public void setAnyStateChangeBreakpoint(final String name, final Model model)
         {
             popup.add(new LuvAction(
                          "Add Break Point for " + name + " State Change",
                          "Add a break point any time " + name + " changes state.")
                {
                      public void actionPerformed(ActionEvent e)
                      {
                         Luv.breakPointHandler.createChangeBreakpoint(model);
                      }
                });     
         }

         // add target state break points menu

         public void setTargetStateChangeBreakpoint(final String name, final Model model)
         {
             stateMenu = new JMenu("Add Break Point for " + name + " State");
             stateMenu.setToolTipText(
                "Add a break point which pauses execution when " + name + 
                " reaches a specified state.");
             popup.add(stateMenu);
             for (final String state: NODE_STATES)
             {
                stateMenu.add(new LuvAction(
                                 state,
                                 "Add a break point when " + name + 
                                 " reaches the " + state + " state.")
                   {
                         public void actionPerformed(ActionEvent e)
                         {
                            Luv.breakPointHandler.createTargetPropertyValueBreakpoint(
                               model, name + " state", MODEL_STATE, state);
                         }
                   });
             }
         }

         // add target outcome break points menu

         public void setTargetOutcomeBreakpoint(final String name, final Model model)
         {
             outcomeMenu = new JMenu("Add Break Point for " + name + " Outcome");
             outcomeMenu.setToolTipText("Add a break point which pauses execution when " + name + " reaches a specified outcome.");
             popup.add(outcomeMenu);
             for (final String outcome: NODE_OUTCOMES)
                outcomeMenu.add(new LuvAction(
                                   outcome,
                                   "Add a break point when " + name + 
                                   " reaches the " + outcome + " outcome.")
                   {
                         public void actionPerformed(ActionEvent e)
                         {
                            Luv.breakPointHandler.createTargetPropertyValueBreakpoint(
                               model, name + " outcome", MODEL_OUTCOME, outcome);
                         }
                   }); 
         }

         // add target failure type break points menu

         public void setFailureTypeBreakpoint(final String name, final Model model)
         {
             failureTypeMenu = new JMenu("Add Break Point for " + name + " Failure Type");
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
                            Luv.breakPointHandler.createTargetPropertyValueBreakpoint(
                               model, name + " failure type", 
                               MODEL_FAILURE_TYPE, failureType);
                         }
                   });  
         }
         
         public void updateBreakpointInfo(final Vector<BreakPoint> BreakPoints, final String name) 
         {
            // add the breakpoints

            popup.add(new JSeparator());
            for (final BreakPoint BreakPoint: BreakPoints)
            {
               String action = BreakPoint.isEnabled() ? DISABLE : ENABLE;
               popup.add(new LuvAction(
                            action + " " + BreakPoint,
                            action + " the breakpoint " + 
                            BreakPoint + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           BreakPoint.setEnabled(!BreakPoint.isEnabled());
                           Luv.luvViewerHandler.refreshView();
                        }
                  }); 
            }

            // add the breakpoints

            popup.add(new JSeparator());
            for (final BreakPoint BreakPoint: BreakPoints)
            {
               popup.add(new LuvAction(
                            "Remove " + BreakPoint,
                            "Permanently remove the breakpoint " + 
                            BreakPoint + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           Luv.breakPointHandler.removeBreakPoint(BreakPoint);
                        }
                  }); 
            }

            // add the remove all action

            popup.add(new JSeparator());
            popup.add(new LuvAction(
                         "Remove All Break Points From " + name,
                         "Permanently remove all breakpoints from " + name + ".")
               {
                     public void actionPerformed(ActionEvent e)
                     {
                        for (final BreakPoint BreakPoint: BreakPoints)
                           Luv.breakPointHandler.removeBreakPoint(BreakPoint);

                        Luv.luvViewerHandler.updateRunMenu();
                     }
               });
         }

}
