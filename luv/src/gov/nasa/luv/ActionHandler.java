/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package gov.nasa.luv;

import java.awt.Color;
import static gov.nasa.luv.Constants.*;

import java.awt.event.ActionEvent;
import java.io.IOException;
import javax.swing.JOptionPane;
import static java.awt.event.KeyEvent.*;

public class ActionHandler 
{
    public ActionHandler() {}
      /** Action to open and view a plan. */

      LuvAction openAction = new LuvAction(
         OPEN_PLAN, "Open a plan for viewing.", VK_O, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  Luv.fileHandler.choosePlan();
               }
         };
         
      /** Action to exit the program. */

      LuvAction exitAction = new LuvAction(
         EXIT, "Terminate this program.", VK_ESCAPE)
         {
               public void actionPerformed(ActionEvent e)
               {
                   Object[] options = 
                     {
                        YES,
                        NO,
                     };
                   
                   int exitLuv = JOptionPane.showOptionDialog(
                           Luv.luvViewerHandler,
                     "Are you sure you want to exit?",
                     "Exit Luv Viewer",
                     JOptionPane.YES_NO_CANCEL_OPTION,
                     JOptionPane.WARNING_MESSAGE,
                     null,
                     options,
                     options[0]);
                   
                   if (exitLuv == 0)
                       Luv.getLuv().exit();
               }
         };
        

      /** Action to disable all breakpoints. */

      LuvAction disableBreakpointsAction = new LuvAction(
         "Disable All Break Points", 
         "Disable every breakpoint in the system.",
         VK_B, META_MASK | SHIFT_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  for (BreakPoint bp: Luv.breakPointHandler.breakPoints.keySet())
                     bp.setEnabled(false);
                  Luv.luvViewerHandler.refreshView();
               }
         };

      /** Action to remove all breakpoints. */

      LuvAction removeAllBreakpointsAction = new LuvAction(
         "Remove All Break Points", 
         "Permanently remove all breakpoint from the system.")
         {
               public void actionPerformed(ActionEvent e)
               {
                  Luv.breakPointHandler.removeAllBreakPoints();
               }
         };
         

      /** Action to enable all breakpoints. */

      LuvAction enableBreakpointsAction = new LuvAction(
         "Enable All Break Points", 
         "Enable every breakpoint in the system.",
         VK_B, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  for (BreakPoint bp: Luv.breakPointHandler.breakPoints.keySet())
                     bp.setEnabled(true);
                  Luv.luvViewerHandler.refreshView();
               }
         };
         
      
      /** Action to load a script for Execution. */
      LuvAction openScriptAction = new LuvAction(
         OPEN_SCRIPT, "Open a script for execution.", VK_O, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  Luv.fileHandler.chooseScript();
               }
         };

      /** Action to reload a plan. */

      LuvAction reloadAction = new LuvAction(
         RELOAD_PLAN, "Reload current plan file.", VK_R, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  Luv.fileHandler.loadRecentPlan(1);
               }
         };

      /** Action to show the debugging window. */

      LuvAction debugWindowAction = new LuvAction(
         SHOW_DEBUG, "Show window with debug text.", VK_D, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  Luv.luvViewerHandler.debugWindow.setVisible(!Luv.luvViewerHandler.debugWindow.isVisible());
               }
         };

      /** Action to allow breakpoints. */
         
      LuvAction allowBreaksAction = new LuvAction(
         BREAKING_ENABLED, "Select this to allow breakpoints.", VK_F2)
	 {
             public void actionPerformed(ActionEvent actionEvent)
             {
                 Luv.allowBreaks = !Luv.allowBreaks;
                 
                 if (Luv.allowBreaks)
                 {
                     Luv.luvViewerHandler.runMenu.getItem(BREAK_MENU_ITEM).setText(BREAKING_ENABLED);
                     Luv.statusBarHandler.showStatus(BREAKING_ENABLED, Color.GREEN.darker());
                 }
                 else
                 {
                     Luv.luvViewerHandler.runMenu.getItem(BREAK_MENU_ITEM).setText(BREAKING_DISABLED);
                     Luv.statusBarHandler.showStatus(BREAKING_DISABLED, Color.RED);
                 }
             }
	 };
         
      /** Action to execute a plexil plan. */

      LuvAction execAction = new LuvAction(
         EXECUTE_PLAN, 
         "Execute plan currently loaded.",
         VK_F1)
         {
             public void actionPerformed(ActionEvent actionEvent)
             {
                try {
                    if (!Luv.planPaused && !Luv.isExecuting)
                    {
                        Luv.executedViaLuvViewer = true;
                        Luv.statusBarHandler.showStatus("Executing...", Color.GREEN.darker(), 1000);
                        Luv.executionHandler.runExec();
                    }
                } catch (IOException ex) {
                    System.err.println("Error: " + ex.getMessage());
                }
             }
      };

      LuvAction pauseAction = new LuvAction(
         PAUSE_OR_RESUME_PLAN, 
         "Pause or resume an executing plan, if it is blocking.",
         VK_SPACE)
         {
               public void actionPerformed(ActionEvent e)
               {  
                   if (Luv.allowBreaks && Luv.isExecuting)
                   {
                       if (Luv.resetAllowBreaks)
                           Luv.allowBreaks = false;
                       Luv.planPaused = !Luv.planPaused;
                       Luv.statusBarHandler.showStatus((Luv.planPaused ? PAUSE : RESUME) + " requested.", Color.BLACK, 1000);
                   }
               }
         };

      /** Action to step a paused plexil plan. */

      LuvAction stepAction = new LuvAction(
         STEP, 
         "Step a plan, pausing it if is not paused.",
         VK_ENTER)
         {
               public void actionPerformed(ActionEvent e)
               {
                  if (!Luv.planPaused)
                  {
                     Luv.planPaused = true;
                     Luv.statusBarHandler.showStatus("Step requested.", Color.BLACK, 1000);
                  }
                  else
                  {
                     Luv.planStep = true;
                     Luv.statusBarHandler.showStatus("Step plan.", Color.BLACK, 1000);
                  }
               }
         };
 
      /** Action show node types in different ways. */
      
      LuvAction showHidePrlNodes = new LuvAction(
         "Toggle Plexil Lisp Nodes", 
         "Show or hide nodes that start with \"plexillisp_\".",
         VK_P, 
         META_MASK)
         {
               RegexModelFilter filter = 
                  new RegexModelFilter(
                     Luv.getLuv().properties.getBoolean(PROP_VIEW_HIDE_PLEXILLISP),
                     "^plexilisp_.*", 
                     MODEL_NAME);

               {
                  filter.addListener(
                     new AbstractModelFilter.Listener()
                     {
                           @Override public void filterChanged(
                              AbstractModelFilter filter) 
                           {
                              Luv.luvViewerHandler.resetView();
                           }
                     });
               }

               public void actionPerformed(ActionEvent e)
               {
                  filter.setEnabled(!filter.isEnabled());
                  Luv.getLuv().properties.set(PROP_VIEW_HIDE_PLEXILLISP,
                                 filter.isEnabled());
               }
         };
}
