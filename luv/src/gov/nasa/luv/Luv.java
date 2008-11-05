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

import javax.swing.JFrame;
import javax.swing.JMenuBar;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.border.EmptyBorder;
import javax.swing.JMenu;
import javax.swing.JSeparator;

import java.awt.Container;
import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;

import java.io.File;
import java.io.IOException;
import java.io.InterruptedIOException;

import static gov.nasa.luv.Constants.*;

import static java.lang.System.*;
import static java.awt.BorderLayout.*;
import static java.awt.event.KeyEvent.*;
import static javax.swing.JFileChooser.*;

/**
 * The containing frame for the Lightweight UE Viewer.
 */

public class Luv extends JFrame
{
    // variables

    private static boolean allowBreaks                 = false;        // does instance of luv have breaks enabled?
    private static boolean planPaused                  = false;        // is instance of luv currently paused?    
    private static boolean planStep                    = false;        // is instance of luv currently stepping?    
    private static boolean isExecuting                 = false;        // is instance of luv currently executing?          
      
    // handler instances
      
    private static FileHandler                  fileHandler                   = new FileHandler();                 // handles all file operations
    private static StatusMessageHandler         statusMessageHandler          = new StatusMessageHandler();        // handles all status messages
    private static LuvBreakPointHandler         luvBreakPointHandler          = new LuvBreakPointHandler();        // handles all break points
    private static ExecutionViaLuvViewerHandler executionViaLuvViewerHandler  = new ExecutionViaLuvViewerHandler();// handles when user executes plan via Luv Viewer and not a terminal
    private static ViewHandler                  viewHandler                   = new ViewHandler();                 // handles all file operations
      
    private JMenu fileMenu                = new JMenu("File");  
    private JMenu recentRunMenu           = new JMenu("Recent Runs");
    private JMenu runMenu                 = new JMenu("Run");   
    private JMenu viewMenu                = new JMenu("View");
    private JMenu windowMenu              = new JMenu("Windows");
      
    private DebugWindow luvViewerDebugWindow; 
      
    SocketServer s;
      
    // current working instance of luv
      
    private static Luv theLuv;         

    // persistent properties for luv viewer 

    private Properties properties = new Properties(PROPERTIES_FILE_LOCATION)
	{
	    {    
		define(PROP_FILE_RECENT_COUNT, PROP_FILE_RECENT_COUNT_DEF);
		define(PROP_WIN_LOC,  PROP_WIN_LOC_DEF);
		define(PROP_WIN_SIZE, PROP_WIN_SIZE_DEF);
		define(PROP_WIN_BCLR, PROP_WIN_BCLR_DEF);
		define(PROP_DBWIN_LOC,     PROP_DBWIN_LOC_DEF);
		define(PROP_DBWIN_SIZE,    PROP_DBWIN_SIZE_DEF);
		define(PROP_DBWIN_VISIBLE, PROP_DBWIN_VISIBLE_DEF);
		define(PROP_TOOLTIP_DISMISS, PROP_TOOLTIP_DISMISS_DEF);
		define(PROP_NET_SERVER_PORT,  PROP_NET_SERVER_PORT_DEF);
		define(PROP_NET_RECENT_HOST,  PROP_NET_RECENT_HOST_DEF);
		define(PROP_NET_AUTO_CONNECT, PROP_NET_AUTO_CONNECT_DEF);
		define(PROP_VIEW_HIDE_PLEXILLISP, PROP_VIEW_HIDE_PLEXILLISP_DEF);
		define(PROP_FILE_RECENT_PLAN_DIR, getProperty(PROP_FILE_RECENT_PLAN_BASE + 1, UNKNOWN));
		define(PROP_FILE_RECENT_SCRIPT_DIR, getProperty(PROP_FILE_RECENT_SCRIPT_BASE + 1, UNKNOWN));
	    }
	};
    
    // the current model

    private Model currentPlan = null;

    // entry point for this program

    public static void main(String[] args)
    {
	runApp();
    }
      
    public static void runApp()
    {
	// if we're on a mac, use mac style menus
         
	System.setProperty("apple.laf.useScreenMenuBar", "true");
         
	try {
            new Luv();
	}
	catch (Exception e) {
            e.printStackTrace();
	}
    }
      
    // constructor for luv viewer
      
    public Luv()
    {
	// record instance of self

	theLuv = this;

	// app exits when frame is closed

	setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

	// construct the frame
         
	constructFrame(getContentPane());    
         
	// create title
         
	setTitle();
         
         // luv will be in this state only when you first start it (minimal options available)
         
	startState();

	// start the server listening for events

	s = new SocketServer(properties.getInteger(PROP_NET_SERVER_PORT),
			     new LuvSocketWranglerFactory());

    }


    // Called from PlexilPlanHandler::endElement(),
    // which will be invoked by both Luv (directly)
    // and the exec (via the Luv listener stream)
    public void handleNewPlan(Model plan)
    {
	// check if plan is a duplicate
	// *** N.B. This depends on the fact that new plans are always added at the end
	Model other = Model.getRoot().findChildByName(plan.getModelName());
	if (plan == other) {
	    // it's really new
	}
	else if (plan.equivalent(other)) {
	    // it's a duplicate, ignore
	    Model.getRoot().removeChild(plan);
	    plan = other;
	}
	else {
	    // it has same name as original, new one supersedes
	    Model.getRoot().removeChild(other);
	}

	if (!plan.equivalent(currentPlan)) {
            currentPlan = plan;
	    Model.getRoot().planChanged();            
            viewHandler.focusView(currentPlan);
	}
        
        setTitle();

        if (isExecuting) {
            executionState();
            addRunToRecentRunList();  // save plan and script to recent run list
	}
                        
	refreshConditionWindow();
                        
	// Determine if the Luv Viewer should pause before executing. 
                                                  
	if (isExecuting && allowBreaks) {
	    pausedState(); 
	    runMenu.setEnabled(true);
	}
    }

    // Called from PlexilPlanHandler::endElement()
    public void handleNewLibrary(Model library)
    {
	Model.getRoot().planChanged();
    }

    public boolean shouldBlock()
    {
	return planPaused && !planStep;
    }

    public void blockViewer()
    {
	if (shouldBlock()) {
	    statusMessageHandler.showStatus((luvBreakPointHandler.getBreakPoint() == null
					     ? "Stopped at beginning of plan"
					     : "Stopped at " + luvBreakPointHandler.getBreakPoint()) +
					    " - " + 
					    pauseAction.getAcceleratorDescription() +
					    " to resume, or " + 
					    stepAction.getAcceleratorDescription() +
					    " to step.",
					    Color.GRAY);

	    luvBreakPointHandler.clearBreakPoint();
                        
	    // wait here for user action
	    while (shouldBlock()) {
		try {
		    Thread.sleep(50);
		}
		catch (Exception e) {
		    e.printStackTrace();
		}
	    }
	}
    }
      
    public ViewHandler            getViewHandler()            { return viewHandler; }             // get current view handler

    public FileHandler            getFileHandler()            { return fileHandler; }             // get current file handler
      
    public LuvBreakPointHandler   getLuvBreakPointHandler()   { return luvBreakPointHandler; }    // get current breakpoint handler
      
    public Properties             getProperties()             { return properties; }              // get persistent properties for luv viewer
    
    public static Luv             getLuv()                    { return theLuv; }                  // get current active instance of luv viewer
      
    //
    // Accessors for local luv variables
    //

    public Model getCurrentPlan()
    {
	return currentPlan;
    }

    public boolean getPlanStep()
    {
	return planStep;
    }

    public boolean getIsExecuting()
    {
	return isExecuting;
    }

    public void setIsExecuting(boolean value)
    {
	isExecuting = value;
	updateBlockingMenuItems();
    }

    public boolean breaksAllowed()
    {
	return allowBreaks;
    }
    
    public void setBreaksAllowed(boolean value)
    {
	allowBreaks = value;
	updateBlockingMenuItems();
    }

    //
    // State transitions
    //

    public void startState()
    {
	disableAllMenus();
          
	allowBreaks = false;
	isExecuting = false;     
	planPaused = false;
          
	Model.getRoot().clear();  
          
	viewHandler.clearCurrentView();
	statusMessageHandler.clearStatusMessageQ();
        luvBreakPointHandler.removeAllBreakPoints();
          
	// reset all menu items
                 
	fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);
        fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(true);
	fileMenu.setEnabled(true);
          
	updateBlockingMenuItems();
	allowBreaksAction.putValue(NAME, ENABLE_BREAKS);
          
	windowMenu.getItem(SHOW_LUV_DEBUG_MENU_ITEM).setEnabled(true);
	windowMenu.setEnabled(true);
    }
      
    public void readyState()
    {
	// set only certain luv viewer variables
          
	planPaused = false;
	planStep = false;
	fileHandler.setStopSearchForMissingLibs(false); 
        
        setTitle();
        
	// set certain menu items
          
	execAction.putValue(NAME, EXECUTE_PLAN);

	fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);          
	fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(true);
	fileMenu.setEnabled(true);
 
	updateBlockingMenuItems();

        if (isExecuting)
            runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);
        else
            runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(true);
            
	runMenu.setEnabled(true);

	if (viewMenu.getMenuComponentCount() > 0) {
	    viewMenu.getItem(EXPAND_MENU_ITEM).setEnabled(true);
	    viewMenu.getItem(COLLAPSE_MENU_ITEM).setEnabled(true);
	    viewMenu.getItem(TOGGLE_TEXT_TYPES_MENU_ITEM).setEnabled(true);
	    viewMenu.getItem(TOGGLE_LISP_NODES_MENU_ITEM).setEnabled(true); 
	    viewMenu.setEnabled(true);
	}
	else
	    viewMenu.setEnabled(false);

	windowMenu.getItem(SHOW_LUV_DEBUG_MENU_ITEM).setEnabled(true);
	windowMenu.setEnabled(true);
    }

    //* Called when we receive EOF on the LuvListener stream. 
    public void finishedExecutionState()
    {
        // set only certain luv viewer variables
          
	planPaused = false;
	planStep = false;	 
	isExecuting = false;
        
        fileHandler.setStopSearchForMissingLibs(false);
  
	// set certain menu items
          
	execAction.putValue(NAME, EXECUTE_PLAN);

	fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);          
	fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(true);
	fileMenu.setEnabled(true);
 
	updateBlockingMenuItems();
	runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(true);
	runMenu.setEnabled(true);

	if (viewMenu.getMenuComponentCount() > 0)
	    {
		viewMenu.getItem(EXPAND_MENU_ITEM).setEnabled(true);
		viewMenu.getItem(COLLAPSE_MENU_ITEM).setEnabled(true);
		viewMenu.getItem(TOGGLE_TEXT_TYPES_MENU_ITEM).setEnabled(true);
		viewMenu.getItem(TOGGLE_LISP_NODES_MENU_ITEM).setEnabled(true); 
		viewMenu.setEnabled(true);
	    }
	else
	    viewMenu.setEnabled(false);

	windowMenu.getItem(SHOW_LUV_DEBUG_MENU_ITEM).setEnabled(true);
	windowMenu.setEnabled(true);

	showStatus("Execution complete.", Color.BLUE);

    }
      
    public void preExecutionState()
    {
        currentPlan.resetMainAttributesOfAllNodes();
        
	fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(false);
	runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
	runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);
          
	showStatus("Preparing to execute...", Color.lightGray);
    }
      
    public void executionState()
    {
	isExecuting = true;
          
	showStatus("Executing...", Color.GREEN.darker());
        
        execAction.putValue(NAME, STOP_EXECUTION);
        
        fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);
	fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
        
        runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(true);
        
        if (allowBreaks) 
            enabledBreakingState();
        else
            disabledBreakingState();
          
	updateBlockingMenuItems();
    }     
          
    public void stopExecution() throws IOException
    {
        showStatus("Canceling execution...", Color.lightGray);
        
	planPaused = false;
        planStep = false;
	executionViaLuvViewerHandler.killUEProcess();
    }
    
    public void openPlanState()
    {
        luvBreakPointHandler.removeAllBreakPoints();
        
        currentPlan.resetMainAttributesOfAllNodes();

        if (TreeTableView.getCurrent() != null &&
            TreeTableView.getCurrent().isConditionWindowOpen())
            TreeTableView.getCurrent().closeConditionWindow();                

        readyState();
    }
    
    private void reloadPlanState()
    {
        currentPlan.resetMainAttributesOfAllNodes();
        
        if (TreeTableView.getCurrent() != null && 
            TreeTableView.getCurrent().isConditionWindowOpen())
            refreshConditionWindow();

        readyState();
    }
    
    public void pausedState()
    {
	allowBreaks = true;
	planPaused = true;
	planStep = false;
          
	// *** modify these? see updateBlockingMenuItems()
	runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(true);
	runMenu.getItem(STEP_MENU_ITEM).setEnabled(true);
    }
      
    public void stepState()
    {
	allowBreaks = true;
	isExecuting = true;
	planPaused = false;
	planStep = true;
          
	// *** modify these? see updateBlockingMenuItems()
	runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(true);
	runMenu.getItem(STEP_MENU_ITEM).setEnabled(true);
    }

    //
    // Sub-states
    //
      
    public void disabledBreakingState()
    {
	allowBreaks = false;
	luvBreakPointHandler.removeAllBreakpointsAction.actionPerformed(null);
	allowBreaksAction.putValue(NAME, ENABLE_BREAKS);
	updateBlockingMenuItems();
    }
      
    public void enabledBreakingState()
    {
	allowBreaks = true;
	allowBreaksAction.putValue(NAME, DISABLE_BREAKS);
	updateBlockingMenuItems();
    }

    //* Modify the state of certain menu items based on whether the exec is running and whether it blocks.

    private void updateBlockingMenuItems()
    {
        // Pause/resume not useful if exec isn't listening
            
	if (isExecuting) {
	    runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
	    if (allowBreaks) {
		runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(allowBreaks);
		runMenu.getItem(STEP_MENU_ITEM).setEnabled(allowBreaks);
	    }
	    else {
		runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
		runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
	    }
	}
	else {
	    runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
	    runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
	    runMenu.getItem(BREAK_MENU_ITEM).setEnabled(true);
	}
    }

      
    public void refreshConditionWindow()
    {
        if(TreeTableView.getCurrent() != null && 
           TreeTableView.getCurrent().isConditionWindowOpen())
        {
            Model node = Model.getRoot();

            for (int i = TreeTableView.getCurrent().getPathToNode().size() - 2; i >= 0; i--) {
                String name = TreeTableView.getCurrent().getPathToNode().get(i);
                if (node != null)
                    node = node.findChildByName(name);
                else 
                    break;
            }

            if (node != null)
                TreeTableView.getCurrent().resetConditionWindow(node, node.getModelName());
            else
                TreeTableView.getCurrent().closeConditionWindow();
        }
    }

    // place all visible elements into the container in the main frame of the application.
  
    public void constructFrame(Container frame)
    {
	// set layout and background color

	setLayout(new BorderLayout());
	setBackground(properties.getColor(PROP_WIN_BCLR));
         
	// add view panel with start logo
         
	JLabel startLogo = new JLabel(getIcon(START_LOGO));
	viewHandler.getViewPanel().add(startLogo);
	frame.add(viewHandler.getViewPanel(), CENTER);
         
	// create a menu bar

	JMenuBar menuBar = new JMenuBar();
	setJMenuBar(menuBar);        
	createMenuBar(menuBar);
         
	// create the status bar

	final JLabel statusBar = new JLabel(" ");
	statusBar.setBorder(new EmptyBorder(2, 2, 2, 2));
	frame.add(statusBar, SOUTH);
	statusMessageHandler.startStatusBarThread(statusBar);       
         
	// set size and location of frame

	setLocation(properties.getPoint(PROP_WIN_LOC));
	setPreferredSize(properties.getDimension(PROP_WIN_SIZE));

	// create the debug window
         
	luvViewerDebugWindow = new DebugWindow(this);
	luvViewerDebugWindow.setTitle("Luv Viewer Debug Window");                

	// set size and location off frame

	luvViewerDebugWindow.setLocation(properties.getPoint(PROP_DBWIN_LOC));
	luvViewerDebugWindow.setPreferredSize(properties.getDimension(PROP_DBWIN_SIZE));
	luvViewerDebugWindow.pack();
         
         // reset "Show/Hide debug window" menu item when closing window
         
	luvViewerDebugWindow.addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent winEvt) {
		    // Perhaps ask user if they want to save any unsaved files first.
		    luvDebugWindowAction.actionPerformed(null);   
		}
	    });
                
	// make the frame visible
         
	pack();
	setVisible(true);
    }
      
    public void createMenuBar(JMenuBar menuBar)
    {       
	// create file menu
         
	menuBar.add(fileMenu);
	fileMenu.add(theLuv.openPlanAction);
	fileMenu.add(theLuv.openScriptAction);
	updateRecentMenu();
	fileMenu.add(recentRunMenu);
	fileMenu.add(theLuv.reloadAction);
	fileMenu.add(new JSeparator());
	fileMenu.add(theLuv.exitAction);

	// create and update exec menu
         
	menuBar.add(runMenu);
	runMenu.add(theLuv.pauseAction);
	runMenu.add(theLuv.stepAction);
	runMenu.add(theLuv.allowBreaksAction);
	runMenu.add(theLuv.execAction);

	// add view menu

	menuBar.add(viewMenu);

	// show window menu
 
	menuBar.add(windowMenu);
	windowMenu.add(theLuv.luvDebugWindowAction);
        windowMenu.add(theLuv.aboutWindowAction);
    }
      
    public JMenu getViewMenu()
    {
	return viewMenu;
    }
      
    public void disableAllMenus()
    {
	// disable all file menu elements
        
	fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(false);
	fileMenu.setEnabled(false);
          
	// disable all run menu elements
        
	runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
	runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
	runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
	runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);         
	runMenu.setEnabled(false);
 
	// disable all view menu elements if there are any

	if (viewMenu.getMenuComponentCount() > 0) {
	    viewMenu.getItem(EXPAND_MENU_ITEM).setEnabled(false);
	    viewMenu.getItem(COLLAPSE_MENU_ITEM).setEnabled(false);
	    viewMenu.getItem(TOGGLE_TEXT_TYPES_MENU_ITEM).setEnabled(false);
	    viewMenu.getItem(TOGGLE_LISP_NODES_MENU_ITEM).setEnabled(false);              
	}
	viewMenu.setEnabled(false);
          
	// disable window menu
          
	windowMenu.setEnabled(false);
    }
      
    /** Given a recent plan index, the description used for the recent menu item tooltip. 
     *
     * @param recentIndex the index of the recent plan
     *
     * @return the description of what gets loaded
     */

    public String getRecentMenuDescription(int index)
    {
        String plan = properties.getProperty(PROP_FILE_RECENT_PLAN_BASE + index);
        String script = properties.getProperty(PROP_FILE_RECENT_SCRIPT_BASE + index);
	
	String description = "Load " + plan + " + " + script;

	return description;
    }
      
    // Add a file to the recently opened file list. 

    private void addRunToRecentRunList()
    {
	// put newest file at the top of the list
        
        String planName = currentPlan.getPlanName();
        String scriptName = currentPlan.getScriptName();
        
        String currPlan = planName;
        String currScript = scriptName;
        
        if (planName != null && scriptName != null &&
            !planName.equals(UNKNOWN) && !scriptName.equals(UNKNOWN))
        {
            int count = properties.getInteger(PROP_FILE_RECENT_COUNT);

            for (int i = 1; i <= count && planName != null; ++i) 
            {
                if (planName != null) 
                {
                    planName = (String)properties.setProperty(PROP_FILE_RECENT_PLAN_BASE + i, planName);
                    scriptName = (String)properties.setProperty(PROP_FILE_RECENT_SCRIPT_BASE + i, scriptName);
                    
                    // if this run already existed in the list, we can stop

                    if (planName != null && planName.equals(currPlan) &&
                        scriptName != null && scriptName.equals(currScript))
                        break;
                }
            }

            // update the recent menu

            updateRecentMenu();
        }
    }
    
    public String getRecentPlanName(int index)
    {
        File recentPlan = new File(UNKNOWN);
        
        if (properties.getProperty(PROP_FILE_RECENT_PLAN_BASE + index) != null)
            recentPlan = new File(properties.getProperty(PROP_FILE_RECENT_PLAN_BASE + index));  
        
        return recentPlan.getName();
    }
    
    public String getRecentScriptName(int index)
    {
        File recentScript = new File(UNKNOWN);
        
        if (properties.getProperty(PROP_FILE_RECENT_SCRIPT_BASE + index) != null)
            recentScript = new File(properties.getProperty(PROP_FILE_RECENT_SCRIPT_BASE + index));  
        
        return recentScript.getName();
    }
    
    public String getRecentPlan(int index)
    {
        String recentPlan = UNKNOWN;
        
        if (properties.getProperty(PROP_FILE_RECENT_PLAN_BASE + index) != null)
            recentPlan = properties.getProperty(PROP_FILE_RECENT_PLAN_BASE + index);  
        
        return recentPlan;
    }
    
    public String getRecentScript(int index)
    {
        String recentScript = UNKNOWN;
        
        if (properties.getProperty(PROP_FILE_RECENT_SCRIPT_BASE + index) != null)
            recentScript = properties.getProperty(PROP_FILE_RECENT_SCRIPT_BASE + index);  
        
        return recentScript;
    }
      
    /** Update the recently loaded files menu. */

    private void updateRecentMenu()
    {
	recentRunMenu.removeAll();
	int count = properties.getInteger(PROP_FILE_RECENT_COUNT);
        
	if (getRecentPlanName(1) == null ||
            getRecentPlanName(1).equals(UNKNOWN)) {
	    recentRunMenu.add("No recent runs");
	}
	else {
	    for (int i = 0; i < count; ++i)
		if ( getRecentPlanName(i + 1) != null &&
                    !getRecentPlanName(i + 1).equals(UNKNOWN))
		    recentRunMenu.add(new LoadRecentAction(i + 1, '1' + i, META_MASK));
	}

	// this menu is only enabled when there are items in it
         
	recentRunMenu.setEnabled(recentRunMenu.getMenuComponentCount() > 0);
    }
      
    // set title of the luv viewer

    private void setTitle()
    {  
        if (currentPlan != null && !currentPlan.getPlanName().equals(UNKNOWN))
        {
            String title = "Luv Viewer  -  " + currentPlan.getPlanNameSansPath();
            
            if (!currentPlan.getScriptName().equals(UNKNOWN))
                title += " + " + currentPlan.getScriptNameSansPath();
            
            setTitle(title);
        }
        else
            setTitle("Luv Viewer");
    }

    // set a program wide property

    public void setProperty(String key, String value)
    {
	properties.setProperty(key, value);
    }

    // get a program wide property

    public String getProperty(String key)
    {
	return properties.getProperty(key, UNKNOWN);
    }
      
    public void showStatus(String message)
    {
	statusMessageHandler.showStatus(message);           
    }
      
    public void showStatus(String message, Color color)
    {
	statusMessageHandler.showStatus(message, color);
    }
      
    public void showStatus(String message, long autoClearTime)
    {
	statusMessageHandler.showStatus(message, autoClearTime);
    }
      
    public void showStatus(String message, Color color, long autoClearTime)
    {
	statusMessageHandler.showStatus(message, color, autoClearTime);
    }
      
    public void clearStatus()
    {
	statusMessageHandler.clearStatus();
    }

    // exit this program.

    private void exit()
    { 
	System.exit(0); 
    }
      
    private String createCommandLine() throws IOException
    {
        String command = PROP_UE_EXEC + " -v";

	if (allowBreaks)
	    command += " -b";                
  
        // get plan
        
	if (currentPlan != null && 
            currentPlan.getPlanName() != null &&
            !currentPlan.getPlanName().equals(UNKNOWN))
        {
            if (new File(currentPlan.getPlanName()).exists())
            {
                command += " " + currentPlan.getPlanName(); 
            }
            else
                return "Error: unable to identify plan.";
        }
        else
            return "Error: unable to identify plan.";
        
        // get script
        
        if (currentPlan != null &&
            currentPlan.getScriptName() != null &&
            !currentPlan.getScriptName().equals(UNKNOWN))
        {
            if (new File(currentPlan.getScriptName()).exists())
            {
                command += " " + currentPlan.getScriptName(); 
            }
            else
                return "Error: unable to identify script.";
        }
        else if (fileHandler.searchForScript() != null)
        {
            command += " " + currentPlan.getScriptName();
        }
        else
            return "Error: unable to identify script.";
             
        // get libraries

        if (!currentPlan.getMissingLibraries().isEmpty()) {
            // try to find libraries
            for (String libName : currentPlan.getMissingLibraries()) {
                Model lib = findLibraryNode(libName, true);
                if (lib == null) {
                    return "Error: library \"" + libName + "\" not found.";
                }
                else {
                    currentPlan.addLibraryName(lib.getPlanName());
                    currentPlan.missingLibraryFound(libName);
                }
            }
        }

        if (!currentPlan.getLibraryNames().isEmpty()) {
            for (String libFile : currentPlan.getLibraryNames()) {
                // double check that library still exists
                if (new File(libFile).exists()) {
                    command += " -l ";
                    command += libFile;
                }
                else {
                    return "Error: library file " + libFile + " does not exist.";
                }
            }
        }       

	return command;
    }

    public Model findLibraryNode(String name, boolean askUser) throws InterruptedIOException
    {
	Model result = Model.getRoot().findChildByName(name);

	if (result == null) {
	    if (askUser) {
		// Prompt user for a file containing the missing library, and (try to) load it
		File library = null;
		try {
		    library = fileHandler.searchForLibrary(name);
		}
		catch (InterruptedIOException e) {
                    JOptionPane.showMessageDialog(theLuv, 
                                                  "Error finding library node. Please see Debug Window.", 
                                                  "Error", 
                                                  JOptionPane.ERROR_MESSAGE);
			    System.err.println("Error: " + e.getMessage());
		}

		if (library != null) {
		    result = Model.getRoot().findChildByName(name);
		}
	    }
	}
	return result;
    }
      
      
    /***************** List of Actions ********************/

    /** Action to open and view a plan. */

    LuvAction openPlanAction = 
	new LuvAction("Open Plan",
		      "Open a plexil plan file.",
		      VK_O, 
		      META_MASK)
	{
	    public void actionPerformed(ActionEvent e)
	    {                   
		// Loading done in the file handler at present
		int option = fileHandler.choosePlan();

		if (option == APPROVE_OPTION) {
		    // Do these things only if we loaded a plan
		    if (isExecuting) {
			try {
			    stopExecution();
			    JOptionPane.showMessageDialog(theLuv, 
                                                          "Stopping execution and opening a new plan", 
                                                          "Stopping Execution", 
                                                          JOptionPane.INFORMATION_MESSAGE);
			}
			catch (IOException ex) {
			    JOptionPane.showMessageDialog(theLuv, 
                                                          "Error stopping execution. Please see Debug Window.", 
                                                          "Error", 
                                                          JOptionPane.ERROR_MESSAGE);
			    System.err.println("Error: " + ex.getMessage());
			}
		    } 
                    
                    openPlanState();
		}
	    }
	};
      
    /** Action to load a script for Execution. */
         
    LuvAction openScriptAction = 
	new LuvAction("Open Script", 
		      "Open a plexil script file.", 
		      VK_E, 
		      META_MASK)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		int option = fileHandler.chooseScript();
		if (option == APPROVE_OPTION) {
		    if (isExecuting) {
			try {
			    stopExecution();
			    JOptionPane.showMessageDialog(theLuv,
							  "Stopping execution and opening script",
							  "Stopping Execution",
							  JOptionPane.INFORMATION_MESSAGE);
			}
			catch (IOException ex) {                       
			    JOptionPane.showMessageDialog(theLuv,
							  "Error stopping execution. Please see Debug Window.",
							  "Error",
							  JOptionPane.ERROR_MESSAGE);
			    System.err.println("Error: " + ex.getMessage());
			}
		    }
                    
                    readyState();
		}
	    }
	};

    /** Action to reload a plan. */

    LuvAction reloadAction = 
	new LuvAction("Reload",
		      "Reload currently loaded files.",
		      VK_R, 
		      META_MASK)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		if (isExecuting) {
		    try {
			stopExecution();
			JOptionPane.showMessageDialog(theLuv,
						      "Stopping execution and reloading plan",
						      "Stopping Execution",
						      JOptionPane.INFORMATION_MESSAGE);
		    }
		    catch (IOException ex) {
			JOptionPane.showMessageDialog(theLuv,
						      "Error reloading plan. Please see Debug Window.",
						      "Error",
						      JOptionPane.ERROR_MESSAGE);
			System.err.println("Error: " + ex.getMessage());
		    }
		}               

                if (currentPlan != null && !currentPlan.getPlanName().equals(UNKNOWN))
                {                  
                    fileHandler.loadPlan(new File(currentPlan.getPlanName()));
                    if (!currentPlan.getScriptName().equals(UNKNOWN))
                        fileHandler.loadScript(new File(currentPlan.getScriptName()));
                    reloadPlanState(); 
                }
                else
                {
                    JOptionPane.showMessageDialog(theLuv,
                                                  "Error: unable to identify plan.",
                                                  "Error",
                                                  JOptionPane.ERROR_MESSAGE);
                    System.err.println("Error: unable to identify plan.");
                }

	    }
	};

    /** Action to show the debugging window. */

    LuvAction luvDebugWindowAction = 
	new LuvAction("Show Luv Viewer Debug Window",
		      "Show window with luv viewer debug text.",
		      VK_L, 
		      META_MASK)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		luvViewerDebugWindow.setVisible(!luvViewerDebugWindow.isVisible());
                   
		if (luvViewerDebugWindow.isVisible())
		    luvDebugWindowAction.putValue(NAME, "Hide Luv Viewer Debug Window");
		else
		    luvDebugWindowAction.putValue(NAME, "Show Luv Viewer Debug Window");
	    }
	};
        
    /** Action to show the About Luv Viewer window. */
        
    LuvAction aboutWindowAction = 
        new LuvAction("About Luv Viewer Window",
		      "Show window with luv viewer about information.",
		      VK_A, 
		      META_MASK)
	{
	    public void actionPerformed(ActionEvent e)
	    {
                if (AboutWindow.isAboutWindowOpen())
                {
                    AboutWindow.closeAboutWindow();
                }
                else
                {
                    try 
                    {
                        AboutWindow.openAboutWindow();
                    } catch (IOException ex) {
                        JOptionPane.showMessageDialog(theLuv,
						      "Error opening About Window. Please see Debug Window.",
						      "Error",
						      JOptionPane.ERROR_MESSAGE);
                        System.err.println("Error: " + ex.getMessage());
                    } catch (InterruptedException ex) {
                        JOptionPane.showMessageDialog(theLuv,
						      "Error opening About Window. Please see Debug Window.",
						      "Error",
						      JOptionPane.ERROR_MESSAGE);
                        System.err.println("Error: " + ex.getMessage());
                    }
                }
	    }
	};
         
    /** Action to allow breakpoints. */
         
    LuvAction allowBreaksAction =
	new LuvAction(ENABLE_BREAKS,
		      "Select this to allow breakpoints.",
		      VK_F2)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		if (!isExecuting) {
		    allowBreaks = !allowBreaks;

		    if (allowBreaks) {
			enabledBreakingState();
                        statusMessageHandler.showStatus("Enabled breaks", Color.GREEN.darker(), 1000);
		    }
		    else {
			disabledBreakingState();
                        statusMessageHandler.showStatus("Disabled breaks", Color.RED, 1000);
		    }
		}
	    }
	};
         
    /** Action to execute a plexil plan. */

    LuvAction execAction = 
	new LuvAction(EXECUTE_PLAN, 
		      "Execute currently loaded plan.",
		      VK_F1)
	{
	    public void actionPerformed(ActionEvent e)
	    {
                try {

                    if (!isExecuting) {
                        preExecutionState();
                        String command = createCommandLine();
                        if (!command.contains("Error")) {                           
                            executionViaLuvViewerHandler.runExec(command);
			}
                        else {
                            JOptionPane.showMessageDialog(theLuv, command, "Error", JOptionPane.ERROR_MESSAGE);
                            showStatus("Stopped execution", Color.lightGray, 1000);
                            readyState();
                        }
                    }
                    else {                       
                        stopExecution();
                    }
                    
                } 
		catch (IOException ex) {
                    JOptionPane.showMessageDialog(theLuv,
						  "Error executing plan. Please see Debug Window.",
						  "Error", 
						  JOptionPane.ERROR_MESSAGE);
                    System.err.println("Error: " + ex.getMessage());
                }
	    }
	};

    LuvAction pauseAction = 
	new LuvAction(PAUSE_OR_RESUME_PLAN, 
		      "Pause or resume an executing plan, if it is blocking.",
		      VK_ENTER)
	{
	    public void actionPerformed(ActionEvent e)
	    {  
		if (isExecuting) {
		    planPaused = !planPaused;

		    statusMessageHandler.showStatus((planPaused ? "Pause" : "Resume") + " requested.", 
						    Color.BLACK, 
						    1000);

		    if (planPaused)
			pausedState();
		    else
			executionState();
                         
		}
	    }
	};

    /** Action to step a paused plexil plan. */

    LuvAction stepAction = 
	new LuvAction(STEP, 
		      "Step a plan, pausing it if is not paused.",
		      VK_SPACE)
	{
	    public void actionPerformed(ActionEvent e)
	    {
		if (isExecuting) {
		    if (!planPaused) {
			pausedState();
			statusMessageHandler.showStatus("Pause requested.", Color.BLACK, 1000);
		    }
		    else {
			stepState();
			statusMessageHandler.showStatus("Step plan.", Color.BLACK, 1000);
		    }
		}                     
	    }
	};
 
    /** Action show node types in different ways. */
      
    LuvAction showHidePrlNodes = 
	new LuvAction("Toggle Plexil Lisp Nodes", 
		      "Show or hide nodes that start with \"plexillisp_\".",
		      VK_P, 
		      META_MASK)
	{
	    RegexModelFilter filter = 
		new RegexModelFilter(properties.getBoolean(PROP_VIEW_HIDE_PLEXILLISP),
				     "^plexilisp_.*");

	    {
		filter.addListener(
				   new AbstractModelFilter.Listener()
				   {
				       @Override public void filterChanged(AbstractModelFilter filter) 
				       {
					   viewHandler.resetView();
				       }
				   });
	    }

	    public void actionPerformed(ActionEvent e)
	    {
		filter.setEnabled(!filter.isEnabled());
		properties.set(PROP_VIEW_HIDE_PLEXILLISP, filter.isEnabled());
	    }
	};

    /** Action to exit the program. */

    LuvAction exitAction = 
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
		    JOptionPane.showOptionDialog(theLuv,
						 "Are you sure you want to exit?",
						 "Exit Luv Viewer",
						 JOptionPane.YES_NO_CANCEL_OPTION,
						 JOptionPane.WARNING_MESSAGE,
						 null,
						 options,
						 options[0]);
                   
		if (exitLuv == 0) {                 
		    exit();
		}
	    }
	};
}
