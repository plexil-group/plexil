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
import java.awt.event.ActionEvent;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import static java.awt.event.KeyEvent.*;
import static javax.swing.JFileChooser.*;
import static gov.nasa.luv.Constants.*;

/**
 * LuvActionHandler manages features that are found in Luv menus
 */

public class LuvActionHandler 
{
    /** Action to open and view a plan. */

    static LuvAction openPlanAction = 
	new LuvAction("Open Plan",
		      "Open a plexil plan file.",
		      VK_O, 
		      META_MASK)
	{
	    public void actionPerformed(ActionEvent e)
	    {  
                Luv.getLuv().setNewPlan(true); 
                PlexilPlanHandler.resetRowNumber();
                
		// Loading done in the file handler at present
		int option = Luv.getLuv().getFileHandler().choosePlan();

		if (option == APPROVE_OPTION) 
                {
 		    // Do these things only if we loaded a plan
		    if (Luv.getLuv().getIsExecuting()) 
                    {
			try 
                        {
			    Luv.getLuv().getLuvStateHandler().stopExecutionState();
                            Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Stopping execution and opening a new plan");
			}
			catch (IOException ex) 
                        {
                            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while stopping execution");
			}
		    } 
                    
                    Luv.getLuv().getLuvStateHandler().openPlanState();
		}
                
                Luv.getLuv().setNewPlan(false); 
	    }
	};
      
    /** Action to load a script for Execution. */
         
    static LuvAction openScriptAction = 
	new LuvAction("Open Script", 
		      "Open a plexil script file.", 
		      VK_E, 
		      META_MASK)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		int option = Luv.getLuv().getFileHandler().chooseScript();
		if (option == APPROVE_OPTION) 
                {
		    if (Luv.getLuv().getIsExecuting()) 
                    {                     
			try 
                        {
			    Luv.getLuv().getLuvStateHandler().stopExecutionState();
                            Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Stopping execution and opening a new script");
			}
			catch (IOException ex) 
                        {                       
			    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while stopping execution");
			}
		    }
                    
                    Luv.getLuv().getLuvStateHandler().readyState();
		}
	    }
	};

    /** Action to reload a plan. */

    static LuvAction reloadAction = 
	new LuvAction("Reload",
		      "Reload currently loaded files.",
		      VK_R, 
		      META_MASK)
	{
	    public void actionPerformed(ActionEvent e) 
	    {
                Luv.getLuv().setNewPlan(true);
                PlexilPlanHandler.resetRowNumber();
                
                if (Luv.getLuv().getIsExecuting()) 
                {
		    try 
                    {
			Luv.getLuv().getLuvStateHandler().stopExecutionState();
                        Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Stopping execution and reloading plan");
		    }
		    catch (IOException ex) 
                    {
                        Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while reloading plan");
		    }
		}                             
          
                File plan = new File(Luv.getLuv().getCurrentPlan().getAbsolutePlanName());
                if (plan.exists())
                {
                    Luv.getLuv().getFileHandler().loadPlan(new File(Luv.getLuv().getCurrentPlan().getAbsolutePlanName()));
                    Luv.getLuv().getStatusMessageHandler().showStatus("Plan \"" + Luv.getLuv().getCurrentPlan().getAbsolutePlanName() + "\" loaded", 1000);
                    Luv.getLuv().getLuvStateHandler().openPlanState(); 
                }
                else
                {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: trying to reload and unknown plan. Are you trying to reload a plan you originally executed remotely?");
                    Luv.getLuv().getLuvStateHandler().reloadPlanState();
                }
                Luv.getLuv().setNewPlan(false);
	    }
	};

    /** Action to show the debugging window. */

    static LuvAction luvDebugWindowAction = 
	new LuvAction("Show Debug Window",
		      "Show window with status and debugging information.",
		      VK_D, 
		      META_MASK)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Luv.getLuv().getDebugWindow().setVisible(!Luv.getLuv().getDebugWindow().isVisible());
                   
		if (Luv.getLuv().getDebugWindow().isVisible())
		    luvDebugWindowAction.putValue(NAME, "Hide Debug Window");
		else
		    luvDebugWindowAction.putValue(NAME, "Show Debug Window");
	    }
	};
        
    /** Action to show the About Luv Viewer window. */
        
    static LuvAction aboutWindowAction = 
        new LuvAction("About Luv Viewer Window",
		      "Show window with luv viewer about information.")
	{
	    public void actionPerformed(ActionEvent e)
	    {
                String info = 
                "Product:   Luv Viewer Version 1.0 beta 4 (c) 2008 NASA Ames Research Center\n" + 
                "Website:   http://plexil.wiki.sourceforge.net/Luv\n" + 
                "Java:        " + System.getProperty("java.version") + "; " + System.getProperty("java.vm.name") + " " + System.getProperty("java.vm.version") + "\n" + 
                "System:    " + System.getProperty("os.name") + " version " + System.getProperty("os.version") + " running on " + System.getProperty("os.arch") + "\n" +
                "Userdir:    " + System.getProperty("user.dir") + "\n";
        
                ImageIcon icon = getIcon(ABOUT_LOGO);

                JOptionPane.showMessageDialog(Luv.getLuv(),
                                              info,
                                              "About Luv Viewer",
                                              JOptionPane.INFORMATION_MESSAGE,
                                              icon);
	    }
	};
         
    /** Action to allow breakpoints. */
         
    static LuvAction allowBreaksAction =
	new LuvAction("Enable Breaks",
		      "Select this to enable or disable breakpoints.",
		      VK_F5)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		if (!Luv.getLuv().getIsExecuting()) 
                {
		    Luv.getLuv().setBreaksAllowed(!Luv.getLuv().breaksAllowed());

		    if (Luv.getLuv().breaksAllowed()) 
                    {
			Luv.getLuv().getLuvStateHandler().enabledBreakingState();
                        Luv.getLuv().getStatusMessageHandler().showStatus("Enabled breaks", Color.GREEN.darker(), 1000);
		    }
		    else 
                    {
			Luv.getLuv().getLuvStateHandler().disabledBreakingState();
                        Luv.getLuv().getStatusMessageHandler().showStatus("Disabled breaks", Color.RED, 1000);
		    }
		}
	    }
	};
        
        /** Action to allow breakpoints. */
         
    static LuvAction removeAllBreaksAction =
	new LuvAction("Remove All Breakpoints",
		      "Remove all breakpoints from this plan.")
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Luv.getLuv().getLuvBreakPointHandler().removeAllBreakPoints();
                Luv.getLuv().getStatusMessageHandler().showStatus("All breakpoints have been removed", 1000);
	    }
	};
         
    /** Action to execute a plexil plan. */

    static LuvAction execAction = 
	new LuvAction("Execute Plan", 
		      "Execute currently loaded plan.",
		      VK_F6)
	{
	    public void actionPerformed(ActionEvent e)
	    {
                try 
                {
                    if (!Luv.getLuv().getIsExecuting()) 
                    {
                        if (!Luv.getLuv().getExecutionHandler().runExec())
                        {
                            Luv.getLuv().getStatusMessageHandler().showStatus("Stopped execution", Color.lightGray, 1000);
                            Luv.getLuv().getLuvStateHandler().readyState();
                        }
                    }
                    else 
                    {                       
                        Luv.getLuv().getLuvStateHandler().stopExecutionState();
                    }                    
                } 
		catch (IOException ex) 
                {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while executing plan");
                }
	    }
	};

    static LuvAction pauseAction = 
	new LuvAction("Pause/Resume plan", 
		      "Pause or resume an executing plan, if it is blocking.",
		      VK_ENTER)
	{
	    public void actionPerformed(ActionEvent e)
	    {  
		if (Luv.getLuv().getIsExecuting()) {
		    Luv.getLuv().setIsPaused(!Luv.getLuv().getPlanPaused());

		    Luv.getLuv().getStatusMessageHandler().showStatus((Luv.getLuv().getPlanPaused() ? "Pause" : "Resume") + " requested", 
						    Color.BLACK, 
						    1000);

		    if (Luv.getLuv().getPlanPaused())
			Luv.getLuv().getLuvStateHandler().pausedState();
		    else
			Luv.getLuv().getLuvStateHandler().executionState();
                         
		}
	    }
	};

    /** Action to step a paused plexil plan. */

    static LuvAction stepAction = 
	new LuvAction("Step", 
		      "Step a plan, pausing it if is not paused.",
		      VK_SPACE)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		if (Luv.getLuv().getIsExecuting()) {
		    if (!Luv.getLuv().getPlanPaused()) {
			Luv.getLuv().getLuvStateHandler().pausedState();
			Luv.getLuv().getStatusMessageHandler().showStatus("Pause requested", Color.BLACK, 1000);
		    }
		    else {
			Luv.getLuv().getLuvStateHandler().stepState();
			Luv.getLuv().getStatusMessageHandler().showStatus("Step plan", Color.BLACK, 1000);
		    }
		}                     
	    }
	};
        
    /** Action to hide or show nodes. */
      
    static LuvAction hideOrShowNodes = 
	new LuvAction("Hide/Show Nodes...", 
		      "Hide or Show specific nodes by full or partial name.",
                      VK_H, 
		      META_MASK)
	{            
            public void actionPerformed(ActionEvent e)
	    {
                Luv.getLuv().getHideOrShowWindow().open();
            }   
	};
        
    /** Action to for a node by name nodes. */
      
    static LuvAction findNode = 
	new LuvAction("Find...", 
		      "Find node by name.",
                      VK_F, 
		      META_MASK)
	{            
            public void actionPerformed(ActionEvent e)
	    {
                FindWindow.open(Luv.getLuv().getProperties().getProperty(PROP_SEARCH_LIST, UNKNOWN));
            }   
	};
        
    /** Action to for a node by name nodes. */
      
    static LuvAction createDebugCFGFileAction = 
	new LuvAction("Create Debug Configuration File...", 
		      "Create and Customize a debug configuration file.",
                      VK_G, 
		      META_MASK)
	{            
            public void actionPerformed(ActionEvent e)
	    {
                try 
                {
                    Luv.getLuv().getDebugCFGWindow().open();
                } 
                catch (FileNotFoundException ex) 
                {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while opening CFG Debug window.");
                }
            }   
	};

    /** Action to exit the program. */

    static LuvAction exitAction = 
	new LuvAction("Exit", "Terminate this program.", VK_ESCAPE)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Object[] options = 
		    {
                        "Yes",
                        "No",
		    };
                   
		int exitLuv = 
		    JOptionPane.showOptionDialog(Luv.getLuv(),
						 "Are you sure you want to exit?",
						 "Exit Luv Viewer",
						 JOptionPane.YES_NO_CANCEL_OPTION,
						 JOptionPane.WARNING_MESSAGE,
						 null,
						 options,
						 options[0]);
                   
		if (exitLuv == 0) 
                {                 
		    System.exit(0);
		}
	    }
	};
        
        /** Action to fully expand tree. */

      static LuvAction expandAll = new LuvAction(
         "Expand All", "Expand all tree nodes.", VK_EQUALS)
         {
               public void actionPerformed(ActionEvent e)
               {                   
                   TreeTableView.getCurrent().expandAllNodes();
                   Luv.getLuv().getViewHandler().refreshRegexView(Luv.getLuv().getCurrentPlan());
               }
         };

      /** Action to fully collapse tree. */

      static LuvAction collapseAll = new LuvAction(
         "Collapse All", "Collapse all tree nodes.", VK_MINUS)
         {
               public void actionPerformed(ActionEvent e)
               {
                   TreeTableView.getCurrent().collapseAllNodes();
                   Luv.getLuv().getViewHandler().refreshRegexView(Luv.getLuv().getCurrentPlan());
               }
         };
}
