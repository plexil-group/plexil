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
import javax.swing.border.EmptyBorder;
import javax.swing.JMenu;
import javax.swing.JSeparator;
import java.awt.Container;
import java.awt.Color;
import java.awt.BorderLayout;
import java.util.ArrayList;
import static gov.nasa.luv.Constants.*;
import static java.lang.System.*;
import static java.awt.BorderLayout.*;
import static javax.swing.JFileChooser.*;

/**
 * The containing frame for the Lightweight UE Viewer.
 */

public class Luv extends JFrame
{
    // boolean variables to help determine Luv state
    private static boolean allowBreaks;     
    private static boolean planPaused;         
    private static boolean planStep;           
    private static boolean isExecuting;         
    private boolean newPlan;
    private boolean shouldHighlight;      
    
    // class instances to manage Luv features
    private static FileHandler fileHandler;
    private static StatusMessageHandler statusMessageHandler; 
    private static LuvBreakPointHandler luvBreakPointHandler; 
    private static ExecutionHandler executionHandler;
    private static ViewHandler viewHandler;   
    private static LuvStateHandler luvStateHandler;
    private static NodeInfoWindow nodeInfoWindow;
    private static ConditionsTab conditionsTab;
    private static VariablesTab variablesTab;
    private static ActionTab actionTab;   
    private static HideOrShowWindow hideOrShowWindow;
    private DebugWindow debugWindow;     
    private DebugCFGWindow debugCFGWindow; 
    private RegexModelFilter regexFilter;   
    
    // Luv menus
    private JMenu fileMenu;  
    private JMenu recentRunMenu;
    private JMenu runMenu;   
    private JMenu viewMenu;
    private JMenu debugMenu; 
   
    private Model currentPlan; // the current model/plan    
    private SocketServer s; // server listening for events     
    private static Luv theLuv; // current working instance of luv        
    private Properties properties; // persistent properties for luv viewer
    
    public static void main(String[] args)
    {
	runApp();
    }
      
    public static void runApp()
    {
	// if we're on a mac, use mac style menus         
	System.setProperty("apple.laf.useScreenMenuBar", "true");
         
	try 
        {
            new Luv();
	}
	catch (Exception e) 
        {
            e.printStackTrace();
	}
    }
      
    // constructor      
    public Luv()
    {
	init();

        viewHandler.showModelInViewer(currentPlan);

	constructFrame(getContentPane());   
         
        luvStateHandler.startState();

	s = new SocketServer(properties.getInteger(PROP_NET_SERVER_PORT),
			     new LuvSocketWranglerFactory());

    }
    
    public void init()
    {
        theLuv = this;
        
        currentPlan = new Model("dummy");
        
        properties = new Properties(PROPERTIES_FILE_LOCATION)
	{
	    {    
		define(PROP_FILE_RECENT_COUNT, PROP_FILE_RECENT_COUNT_DEF);
		define(PROP_WIN_LOC,  PROP_WIN_LOC_DEF);
		define(PROP_WIN_SIZE, PROP_WIN_SIZE_DEF);
		define(PROP_WIN_BCLR, PROP_WIN_BCLR_DEF);
		define(PROP_DBWIN_LOC,     PROP_DBWIN_LOC_DEF);
		define(PROP_DBWIN_SIZE,    PROP_DBWIN_SIZE_DEF);
                
                define(PROP_NODEINFOWIN_LOC,     PROP_NODEINFOWIN_LOC_DEF);
		define(PROP_NODEINFOWIN_SIZE,    PROP_NODEINFOWIN_SIZE_DEF);
                define(PROP_FINDWIN_LOC,     PROP_FINDWIN_LOC_DEF);
		define(PROP_FINDWIN_SIZE,    PROP_FINDWIN_SIZE_DEF);
                define(PROP_HIDESHOWWIN_LOC,     PROP_HIDESHOWWIN_LOC_DEF);
		define(PROP_HIDESHOWWIN_SIZE,    PROP_HIDESHOWWIN_SIZE_DEF);
                define(PROP_CFGWIN_LOC,     PROP_CFGWIN_LOC_DEF);
                define(PROP_CFGWIN_SIZE,    PROP_CFGWIN_SIZE_DEF);
                
		define(PROP_TOOLTIP_DISMISS, PROP_TOOLTIP_DISMISS_DEF);
		define(PROP_NET_SERVER_PORT,  PROP_NET_SERVER_PORT_DEF);
		define(PROP_NET_RECENT_HOST,  PROP_NET_RECENT_HOST_DEF);
		define(PROP_NET_AUTO_CONNECT, PROP_NET_AUTO_CONNECT_DEF);
		define(PROP_FILE_RECENT_PLAN_DIR, 
                        getProperty(PROP_FILE_RECENT_PLAN_BASE + 1, UNKNOWN));
		define(PROP_FILE_RECENT_SCRIPT_DIR, 
                        getProperty(PROP_FILE_RECENT_SCRIPT_BASE + 1, UNKNOWN));
                
                define(PROP_HIDE_SHOW_LIST, 
                        getProperty(PROP_HIDE_SHOW_LIST, UNKNOWN).equals
                        (UNKNOWN) ? "" : getProperty(PROP_HIDE_SHOW_LIST));
                define(PROP_SEARCH_LIST, 
                        getProperty(PROP_SEARCH_LIST, UNKNOWN).equals
                        (UNKNOWN) ? "" : getProperty(PROP_SEARCH_LIST));
	    }
	};
        
        allowBreaks = false;
        planPaused = false;
        planStep = false;
        isExecuting = false;
        newPlan = false;
        shouldHighlight = true;    
        fileHandler = new FileHandler();
        statusMessageHandler = new StatusMessageHandler();
        luvBreakPointHandler = new LuvBreakPointHandler();
        executionHandler = new ExecutionHandler();
        luvStateHandler = new LuvStateHandler();
        viewHandler = new ViewHandler();      
        nodeInfoWindow = new NodeInfoWindow();
        conditionsTab = new ConditionsTab();
        variablesTab = new VariablesTab();
        actionTab = new ActionTab();
        hideOrShowWindow = new HideOrShowWindow();
        debugWindow = new DebugWindow(this);  
            
        debugCFGWindow = new DebugCFGWindow();     
        regexFilter = new RegexModelFilter(true);     
        fileMenu = new JMenu("File");
        recentRunMenu = new JMenu("Recent Runs");
        runMenu = new JMenu("Run");   
        viewMenu = new JMenu("View");
        debugMenu = new JMenu("Debug");
    }
    
    private void constructFrame(Container frame)
    {
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);      
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
         
	setLocation(properties.getPoint(PROP_WIN_LOC));
	setPreferredSize(properties.getDimension(PROP_WIN_SIZE));
        
        regexFilter.updateRegexList();
            
        setTitle();    
	pack();
	setVisible(true);
    }

    // Called from PlexilPlanHandler::endElement(),
    // which will be invoked by both Luv (directly)
    // and the exec (via the Luv listener stream)
    public void handleNewPlan(Model plan)
    {
	// *** N.B. This depends on that new plans are always added at the end
	Model other = Model.getRoot().findChildByName(plan.getModelName());
        
	if (plan == other) 
        {
	    // brand new 'plan' loaded
            nodeInfoWindow.closeNodeInfoWindow();
	}
	else if (plan.equivalent(other) && !newPlan) 
        {
	    // same 'plan' loaded, so use 'other' previous plan
	    Model.getRoot().removeChild(plan);
	    plan = other;
	}
	else 
        {
	    // new 'plan' has same root name as 'other' previous plan, 
            // but new 'plan' supersedes
	    Model.getRoot().removeChild(other);
	}

	if (!plan.equivalent(currentPlan) || newPlan)
        {    
            luvBreakPointHandler.removeAllBreakPoints();
            currentPlan = plan;
	    Model.getRoot().planChanged();
            viewHandler.showModelInViewer(currentPlan);
	}

        luvStateHandler.preExecutionState();
        
        if (isExecuting)
        {
            luvStateHandler.executionState();
            LoadRecentAction.addRunToRecentRunList();
        }       
        
        setTitle();
                        
	// Determine if the Luv Viewer should pause before executing.                                                
	if (isExecuting && allowBreaks) 
        {
	    luvStateHandler.pausedState(); 
	    runMenu.setEnabled(true);
        }
    }

    // NOT BEING USED CURRENTLY - need more info from UE
    // Called from PlexilPlanHandler::endElement()
    /*public void handleNewLibrary(Model library)
    {
	// See if this library was referenced before it was defined
	for (Model child : Model.getRoot().getChildren()) {
	    if (child.getMissingLibraries().contains(library.getModelName())) {
		// link in the new library
		child.linkLibrary(library);
	    }
	}
	
	Model.getRoot().planChanged();
    }*/

    public void blockViewer()
    {
	if (shouldBlock()) 
        {
            statusMessageHandler.showStatus((luvBreakPointHandler.getBreakPoint() == null
					     ? "Stopped"
					     : "Stopped at " + luvBreakPointHandler.getBreakPoint()) +
					    " - " + 
					    LuvActionHandler.pauseAction.getAcceleratorDescription() +
					    " to resume, or " + 
					    LuvActionHandler.stepAction.getAcceleratorDescription() +
					    " to step",
					    Color.RED); 
           
            if (luvBreakPointHandler.getBreakPoint() != null && shouldHighlight)
            {
                TreeTableView.getCurrent().highlightRow(luvBreakPointHandler.getBreakPoint().getModel());
            }

	    luvBreakPointHandler.clearBreakPoint();
                        
	    // wait here for user action
	    while (shouldBlock()) 
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
            
            TreeTableView.getCurrent().unHighlightRow();
            shouldHighlight = true;
	}
    }
      
    // accessors for local luv variables  
    public static Luv             getLuv()                    { return theLuv; }                   
    public ViewHandler            getViewHandler()            { return viewHandler; }           
    public FileHandler            getFileHandler()            { return fileHandler; }              
    public ExecutionHandler       getExecutionHandler()       { return executionHandler; }  
    public LuvBreakPointHandler   getLuvBreakPointHandler()   { return luvBreakPointHandler; }    
    public StatusMessageHandler   getStatusMessageHandler()   { return statusMessageHandler; } 
    public LuvStateHandler        getLuvStateHandler()        { return luvStateHandler; }
    public HideOrShowWindow       getHideOrShowWindow()       { return hideOrShowWindow; } 
    public NodeInfoWindow         getNodeInfoWindow()         { return nodeInfoWindow; } 
    public ConditionsTab          getConditionsTab()          { return conditionsTab; }              
    public VariablesTab           getVariablesTab()           { return variablesTab; }               
    public ActionTab              getActionTab()              { return actionTab; }
    public DebugWindow            getDebugWindow()            { return debugWindow; }
    public DebugCFGWindow         getDebugCFGWindow()         { return debugCFGWindow; }     
    public RegexModelFilter       getRegexModelFilter()       { return regexFilter; }         
    public Properties             getProperties()             { return properties; }             
    public Model                  getCurrentPlan()            { return currentPlan; }    
    public JMenu                  getViewMenu()               { return viewMenu; }   
    public JMenu                  getFileMenu()               { return fileMenu; }   
    public JMenu                  getRunMenu()                { return runMenu; }  
    public JMenu                  getDebugMenu()              { return debugMenu; }  
    public JMenu                  getRecentRunMenu()          { return recentRunMenu; }   
    public boolean                getPlanStep()               { return planStep; }  
    public boolean                getPlanPaused()             { return planPaused; }
    public boolean                getIsExecuting()            { return isExecuting; }  
    public boolean                breaksAllowed()             { return allowBreaks; }
    
    public boolean                shouldBlock()               
    { 
        return planPaused && !planStep; 
    }
    
    // mutators for local luv variables    
    public void setIsExecuting(boolean value) 
    {
	isExecuting = value;
	updateBlockingMenuItems();
    }
    
    public void setIsPaused(boolean value)
    {
        planPaused = value;
    }
    
    public void setIsStepped(boolean value)
    {
        planStep = value;
    }
    
    public void setNewPlan(boolean value)
    {
        newPlan = value;
    }

    public void setBreaksAllowed(boolean value)
    {
	allowBreaks = value;
	updateBlockingMenuItems();
    }
    
    public void setShouldHighlight(boolean value)
    {
        shouldHighlight = value;
    }

    // Modify the state of certain menu items based on whether the exec 
    // is running and whether it blocks.
    public void updateBlockingMenuItems()
    {
        // Pause/resume not useful if exec isn't listening            
	if (isExecuting) 
        {            
	    if (allowBreaks) 
            {
		runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(true);
		runMenu.getItem(STEP_MENU_ITEM).setEnabled(true);
	    }
	    else 
            {
		runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
		runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
	    }
            runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
            runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(false);
	}
	else 
        {
	    runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
	    runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
            runMenu.getItem(BREAK_MENU_ITEM).setEnabled(true);
            
            if (luvBreakPointHandler.breakpointsExist())
                runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(true);
            else
                runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(false);
	}      
    }
    
    public void enableRemoveBreaksMenuItem(boolean value)
    {
        if (runMenu.getComponents().length > 0)
            runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(value);
    }

    public void createMenuBar(JMenuBar menuBar)
    {       
	menuBar.add(fileMenu);
	fileMenu.add(LuvActionHandler.openPlanAction);
	fileMenu.add(LuvActionHandler.openScriptAction);
	LoadRecentAction.updateRecentMenu();
	fileMenu.add(recentRunMenu);
	fileMenu.add(LuvActionHandler.reloadAction);
	fileMenu.add(new JSeparator());
	fileMenu.add(LuvActionHandler.exitAction);

	menuBar.add(runMenu);
	runMenu.add(LuvActionHandler.pauseAction);
	runMenu.add(LuvActionHandler.stepAction);
	runMenu.add(LuvActionHandler.allowBreaksAction);
	runMenu.add(LuvActionHandler.removeAllBreaksAction);
        runMenu.add(new JSeparator());      
        runMenu.add(LuvActionHandler.execAction);

        menuBar.add(viewMenu);
        viewMenu.add(LuvActionHandler.expandAll);
        viewMenu.add(LuvActionHandler.collapseAll);
        viewMenu.add(LuvActionHandler.hideOrShowNodes);
        viewMenu.add(LuvActionHandler.findNode);

	menuBar.add(debugMenu);
        debugMenu.add(LuvActionHandler.luvDebugWindowAction);
        debugMenu.add(LuvActionHandler.createDebugCFGFileAction);
        debugMenu.add(LuvActionHandler.aboutWindowAction);
    }
      
    public void disableAllMenus()
    {
	fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(false);
	fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(false);
	fileMenu.setEnabled(false);
          
	runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
	runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
	runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);	
        runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(false);
        runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);  
	runMenu.setEnabled(false);
 
	if (viewMenu.getMenuComponentCount() > 0) 
        {
	    viewMenu.getItem(EXPAND_MENU_ITEM).setEnabled(false);
	    viewMenu.getItem(COLLAPSE_MENU_ITEM).setEnabled(false);
	    viewMenu.getItem(HIDE_OR_SHOW_NODES_MENU_ITEM).setEnabled(false);  
            viewMenu.getItem(FIND_MENU_ITEM).setEnabled(false);
	}
	viewMenu.setEnabled(false);
        
        debugMenu.setEnabled(false);
    }
      
    // set title of the luv viewer
    public void setTitle()
    {  
        if (currentPlan != null && !currentPlan.getPlanName().equals(UNKNOWN))
        {
            String title = "Luv Viewer  -  " + currentPlan.getPlanName();
            
            if (!currentPlan.getScriptName().equals(UNKNOWN))
                title += " + " + currentPlan.getScriptName();
            
            setTitle(title);
        }
        else if (isExecuting)
        {
            setTitle("Luv Viewer  -  Remote Execution");
        }
        else
        {
            setTitle("Luv Viewer");
        }
    }

    // set a program wide property
    public void setProperty(String key, String value)
    {
	properties.setProperty(key, value);
    }
    
    public void setProperty(String key, ArrayList<String> value)
    {
	properties.define(key, value);
    }

    // get a program wide property
    public String getProperty(String key)
    {
	return properties.getProperty(key, UNKNOWN);
    }
}
