/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

import java.awt.Container;
import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Set;
import javax.swing.ImageIcon;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JMenuBar;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.border.EmptyBorder;
import javax.swing.JMenu;
import javax.swing.JSeparator;
import static gov.nasa.luv.Constants.*;
import static java.awt.BorderLayout.*;
import static java.awt.event.KeyEvent.*;
import static java.lang.System.*;

/**
 * The Luv class is the starting point of the Luv application and creates a 
 * containing frame for the Lightweight Universal Executive Viewer.
 */
public class Luv extends JFrame {

    //
    // Local constants
    //

    /** Represents index of the Executive Select under the File menu in the Luv application. */
    private static final int EXEC_SELECT_ITEM      		= 0;
    /** Represents index of the Reload item under the File menu in the Luv application. */
    private static final int RELOAD_MENU_ITEM              = 1;
    /** Represents index of the Exit item under the File menu in the Luv application. */ 
    private static final int EXIT_MENU_ITEM                = 3;    

    /** Represents index of the Pause/Resume item under the Run menu in the Luv application. */ 
    private static final int PAUSE_RESUME_MENU_ITEM        = 0;
    /** Represents index of the Step item under the Run menu in the Luv application. */
    private static final int STEP_MENU_ITEM                = 1;
    /** Represents index of the Break item under the Run menu in the Luv application. */
    private static final int BREAK_MENU_ITEM               = 2;
    /** Represents index of the Remove Breaks item under the Run menu in the Luv application. */
    private static final int REMOVE_BREAKS_MENU_ITEM       = 3;      
    /** Represents index of the Execute item under the Run menu in the Luv application. */
    private static final int EXECUTE_MENU_ITEM             = 5;
      
    /** Represents index of the Expand All item under the View menu in the Luv application. */
    private static final int EXPAND_MENU_ITEM              = 0;
    /** Represents index of the Collapse All item under the View menu in the Luv application. */
    private static final int COLLAPSE_MENU_ITEM            = 1;
    /** Represents index of the Hide Or Show... item under the View menu in the Luv application. */
    private static final int HIDE_OR_SHOW_NODES_MENU_ITEM  = 2;
    /** Represents index of the Find... item under the View menu in the Luv application. */
    private static final int FIND_MENU_ITEM                = 3;
    
    private static final int EPX_VIEW_MENU_ITEM            = 5;
    
    private static final int VIEW_SOURCE_MENU_ITEM         = 7;

    // boolean variables to help determine Luv state	
    private boolean allowBreaks;
    private int appMode;
    private boolean checkPlan;
    private boolean planPaused;
    private boolean planStep;
    private boolean isExecuting;
    private boolean extendedViewOn;
    private boolean newPlan;
    private boolean shouldHighlight;

    // instances to manage Luv features
    private FileHandler fileHandler;
    private StatusMessageHandler statusMessageHandler;
    private LuvBreakPointHandler luvBreakPointHandler;
    private ExecutionHandler executionHandler;
    private ViewHandler viewHandler;
    private HideOrShowWindow hideOrShowWindow;
    private DebugWindow debugWindow;
    private CreateCFGFileWindow createCFGFileWindow;
    private SourceWindow sourceWindow;
    private LuvPortGUI portGui;
    private LibraryLoader libraryLoader;
    private ExecSelect execSelect;
    private RegexModelFilter regexFilter;
    private int luvPort;
    private int luvPrevPort = 0;
    private int pid;

    //Gantt Viewer
    private OpenGanttViewer openGanttViewer;
    
    //Luv SocketServer
    private LuvSocketServer luvServer;

    // Luv menus
    private JMenu fileMenu;
    private JMenu recentRunMenu;
    private JMenu runMenu;
    private JMenu viewMenu;
    private JMenu debugMenu;

    // UI action listeners
    private LuvAction allowBreaksAction;
    private LuvAction execAction;
    private LuvAction extendedViewAction;
    private LuvAction luvGetDebugWindowAction;
    private LuvAction pauseAction;
    private LuvAction stepAction;

    // the current model/plan
    private Model currentPlan;

    // current working instance of luv
    private static Luv theLuv;   
    
    // current port file associated with luv
    private static LuvTempFile portFile;
    
    // persistent properties for Plexil viewer
    private Properties properties;

    /** Entry point for the Luv application. */
    public static void main(String[] args) {    	
        runApp(args);
    }

    /** Creates a new instance of the Luv application. */
    private static void runApp(String[] args) {
        // In MacOS, don't use system menu bar
        System.setProperty("apple.laf.useScreenMenuBar", "false");
        try {
            new Luv(args);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /** 
     * Constructs Luv and initializes the many features of the Luv application
     * such as creating the main viewing window, putting Luv in a 'start state'
     * and initializing a SocketServer to start listening for events from the 
     * Universal Executive.  Assigns port argument in to socket server and
     * creates a port temp file with the relevant port in use.
     */
    public Luv(String[] args) throws IOException
    {
        init(args);
        viewHandler.showModelInViewer(currentPlan);
        constructFrame(getContentPane());
        startState();
        handlePort(args);
    }
    
    /** Manages initial port configuration for viewer.
     * 
     * @param port argument from command line 
     */
    private void handlePort(String[] args)
    {
    	LuvTempFile.cleanupPorts();
    	luvPort = definePort(args);
        luvServer = new LuvSocketServer(luvPort);
        //Script handles socket connections, temp file only for timing difference
        LuvTempFile.deleteTempFile();
        portFile = new LuvTempFile();
    }
    
    /** class which Deletes temp file assocated with viewer port prior to shutting down.
     */
    private class MyShutdownHook extends Thread
    {
    	public void run() { LuvTempFile.deleteTempFile(); }
    }

    /** 
     * Redefines port argument based upon string argument.
     * Restarts listening server with new port. Sets
     * up new port temp file.
     */
    public void changePort(String port)
    {
    	String[] tempArry = {""};
    	tempArry[0] = port;
    	luvPrevPort = luvPort;
    	int tempPort = definePort(tempArry);
    	try {
            LuvSocketServer temp = new LuvSocketServer(tempPort);
            if (!LuvTempFile.checkPort(tempPort) && temp != null) {
                luvServer.stopServer();
                LuvTempFile.deleteTempFile();
                luvPort = tempPort;
                luvServer = temp;
                portFile = new LuvTempFile();
            }
    	}
        catch(Exception e)
            {    		
    		statusMessageHandler.displayErrorMessage(e, "Error occured while changing to port " + tempPort);    		
            }
    }
    
    /** 
     * Tests port argument or uses default
     */    
    private int definePort(String[] args)
    {    	
    	int port = 0;
    	int def_port = properties.getInteger(PROP_NET_SERVER_PORT);
    	
    	if(args.length == 0)
    		port = portGui.getPick();
    	if(args.length > 0)
    	{
    		try{    			
    			port = Integer.parseInt(args[0]);
    		}catch(NumberFormatException e){
    			statusMessageHandler.displayErrorMessage(e, "Port " + args[0] + " is invalid");    			
    	    }
    	}
    	if(port < 1 && !portGui.isEmpty())
    	{
    		port = portGui.getPick();
    		statusMessageHandler.showChangeOnPort("Re-routing to port " + port, 2000);
    	}
    	else if(port < 1)
    	{
    		port = def_port;
    		statusMessageHandler.showChangeOnPort("Attempting last resort port " + port, 2000);
    	}
    	    	
    	statusMessageHandler.showChangeOnPort("Listening on port " + port);    	     	
    	return port;    	
    }

    private void init(String[] cmdLineArgs)
    {
        theLuv = this;
        Runtime.getRuntime().addShutdownHook(new MyShutdownHook());
        currentPlan = new Model("dummy");
        properties = new Properties (PROPERTIES_FILE_LOCATION)
            {
                {
                    define(PROP_FILE_RECENT_COUNT, PROP_FILE_RECENT_COUNT_DEF);
                    define(PROP_ARRAY_MAX_CHARS, PROP_ARRAY_MAX_CHARS_DEF);
                    define(PROP_WIN_LOC, PROP_WIN_LOC_DEF);
                    define(PROP_WIN_SIZE, PROP_WIN_SIZE_DEF);
                    define(PROP_WIN_BCLR, PROP_WIN_BCLR_DEF);
                    define(PROP_DBWIN_LOC, PROP_DBWIN_LOC_DEF);
                    define(PROP_DBWIN_SIZE, PROP_DBWIN_SIZE_DEF);

                    define(PROP_NODEINFOWIN_LOC, PROP_NODEINFOWIN_LOC_DEF);
                    define(PROP_NODEINFOWIN_SIZE, PROP_NODEINFOWIN_SIZE_DEF);
                    define(PROP_FINDWIN_LOC, PROP_FINDWIN_LOC_DEF);
                    define(PROP_FINDWIN_SIZE, PROP_FINDWIN_SIZE_DEF);
                    define(PROP_HIDESHOWWIN_LOC, PROP_HIDESHOWWIN_LOC_DEF);
                    define(PROP_HIDESHOWWIN_SIZE, PROP_HIDESHOWWIN_SIZE_DEF);
                    define(PROP_CFGWIN_LOC, PROP_CFGWIN_LOC_DEF);
                    define(PROP_CFGWIN_SIZE, PROP_CFGWIN_SIZE_DEF);

                    define(PROP_NET_SERVER_PORT, PROP_NET_SERVER_PORT_DEF);
                
                    define(PROP_FILE_EXEC_RECENT_PLAN_DIR,
                           getProperty(PROP_FILE_EXEC_RECENT_PLAN_BASE + 1, UNKNOWN));
                    define(PROP_FILE_EXEC_RECENT_CONFIG_DIR,
                           getProperty(PROP_FILE_EXEC_RECENT_CONFIG_BASE + 1, UNKNOWN));
                    define(PROP_FILE_TEST_RECENT_PLAN_DIR,
                           getProperty(PROP_FILE_TEST_RECENT_PLAN_BASE + 1, UNKNOWN));
                    define(PROP_FILE_TEST_RECENT_SCRIPT_DIR,
                           getProperty(PROP_FILE_TEST_RECENT_SCRIPT_BASE + 1, UNKNOWN));
                    define(PROP_FILE_SIM_RECENT_PLAN_DIR,
                           getProperty(PROP_FILE_SIM_RECENT_PLAN_BASE + 1, UNKNOWN));
                    define(PROP_FILE_SIM_RECENT_SCRIPT_DIR,
                           getProperty(PROP_FILE_SIM_RECENT_SCRIPT_BASE + 1, UNKNOWN));
                    define(PROP_HIDE_SHOW_LIST,
                           getProperty(PROP_HIDE_SHOW_LIST, UNKNOWN).equals(UNKNOWN) ? ""
                           : getProperty(PROP_HIDE_SHOW_LIST));
                    define(PROP_SEARCH_LIST,
                           getProperty(PROP_SEARCH_LIST, UNKNOWN).equals(UNKNOWN) ? ""
                           : getProperty(PROP_SEARCH_LIST));
                }
            };

        allowBreaks = false;
        appMode = PLEXIL_TEST; //default is testExec
        checkPlan = true;
        planPaused = false;
        planStep = false;
        isExecuting = false;
        extendedViewOn = true;
        newPlan = false;
        shouldHighlight = true;        
        
        fileHandler = new FileHandler(cmdLineArgs);
        statusMessageHandler = new StatusMessageHandler();
        luvBreakPointHandler = new LuvBreakPointHandler();
        executionHandler = new ExecutionHandler();
        viewHandler = new ViewHandler();
        hideOrShowWindow = null; // create on demand
        debugWindow = new DebugWindow();

        libraryLoader = new LibraryLoader("Libraries", cmdLineArgs);
        execSelect = new ExecSelect();
        portGui = new LuvPortGUI();    

        //Gantt Viewer
        openGanttViewer = new OpenGanttViewer();

        createCFGFileWindow = new CreateCFGFileWindow();
        sourceWindow = new SourceWindow();
        regexFilter = new RegexModelFilter(true);
        recentRunMenu = new JMenu("Recent Runs");
        execSelect.loadFromPersistence();
    }

    private void constructFrame (Container frame)
    {
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout());
        setBackground(properties.getColor(PROP_WIN_BCLR));

        // add view panel with start logo
        JPanel logoPane = new JPanel();
        logoPane.setLayout(new BorderLayout());
        logoPane.setBackground(Color.WHITE);
        JLabel title = new JLabel("<html>                         " + "<font size=+1>Plan Execution Interchange Language</font><br>" 
        		+ Constants.PLEXIL_VERSION + "<br><br>" + Constants.PLEXIL_WEBSITE + "<br><br>" + Constants.PLEXIL_COPYRIGHT 
        		+ "<br></html>", SwingConstants.CENTER);
        JLabel startLogo = new JLabel(getIcon(START_LOGO), SwingConstants.CENTER);
        //JLabel startCopyRight = new JLabel(Constants.PLEXIL_COPYRIGHT, SwingConstants.CENTER);
        logoPane.add(startLogo, BorderLayout.PAGE_START);
        logoPane.add(title, BorderLayout.CENTER);
        //logoPane.add(startCopyRight, BorderLayout.PAGE_END);
        viewHandler.getViewPanel().add(logoPane);        
        frame.add(viewHandler.getViewPanel(), CENTER);

        // create a menu bar
        JMenuBar menuBar = new JMenuBar();
        setJMenuBar(menuBar);
        createMenuBar(menuBar);

        //allocate area for status bars
        JPanel infoBar = new JPanel();
        frame.add(infoBar, SOUTH);
        GridBagLayout gridbag = new GridBagLayout();
        infoBar.setLayout(gridbag);
        GridBagConstraints c = new GridBagConstraints();
                
        // create the status bar
        final JLabel statusBar = new JLabel(" ");
        statusBar.setBorder(new EmptyBorder(2, 2, 2, 2));        
        infoBar.add(statusBar);
        c.fill = GridBagConstraints.HORIZONTAL;
        c.weightx = 1.0;
        gridbag.setConstraints(statusBar, c);
        statusMessageHandler.startMsgStatusBarThread(statusBar);
        
        // create port message bar
        final JLabel portBar = new JLabel(" ");
        portBar.setBorder(new EmptyBorder(2, 2, 2, 2));
        infoBar.add(portBar);
        c.weightx = 0.02;
        gridbag.setConstraints(portBar,c);
        statusMessageHandler.startPortStatusBarThread(portBar);                

        // save preferred window sizes when Luv application closes
        Runtime.getRuntime().addShutdownHook(new Thread() {

            public void run() {
                properties.set(PROP_WIN_LOC, getLocation());
                properties.set(PROP_WIN_SIZE, getSize());
                properties.set(PROP_DBWIN_LOC, debugWindow.getLocation());
                properties.set(PROP_DBWIN_SIZE, debugWindow.getSize());
            }
        });

        setLocation(properties.getPoint(PROP_WIN_LOC));
        setPreferredSize(properties.getDimension(PROP_WIN_SIZE));

        regexFilter.extendedPlexilView();
        regexFilter.updateRegexList();

        setTitle();
        pack();
        setVisible(true);
    }

    public void loadPlan(File plan)
    {
        fileHandler.loadPlan(plan);
        openPlanState();
        statusMessageHandler.showStatus("Plan \"" + currentPlan.getAbsolutePlanName() + "\" loaded", 1000);
    }

    /** Handles the Plexil plan being loaded into the Luv application. 
     *  Called from PlexilPlanHandler.endElement(), which will be invoked 
     *  by both Luv (directly) and the Universal Executive (via the Luv listener stream).
     *
     * @param plan the Plexil plan to be loaded into the Luv application
     */
    public void handleNewPlan(Model plan) {
        // *** N.B. This depends on that new plans are always added at the end
        Model other = Model.getRoot().findChildByName(plan.getModelName());

        if (plan == other) {
            // brand new 'plan' loaded
            NodeInfoWindow.closeNodeInfoWindow();
        } else if (plan.equivalent(other) && !newPlan) {
            // same 'plan' loaded, so use 'other' previous plan
            Model.getRoot().removeChild(plan);
            plan = other;
        } else {
            // new 'plan' has same root name as 'other' previous plan,
            // but new 'plan' supersedes
            Model.getRoot().removeChild(other);
        }

        if (!plan.equivalent(currentPlan) || newPlan) {
            luvBreakPointHandler.removeAllBreakPoints();
            currentPlan = plan;
            Model.getRoot().planChanged();
            viewHandler.showModelInViewer(currentPlan);
        }

        preExecutionState();

        if (isExecuting) {
            executionState();
            LoadRecentAction.addRunToRecentRunList();
        }

        setTitle();

        // Determine if the Luv Viewer should pause before executing.
        if (isExecuting && allowBreaks) {
            pausedState();
            runMenu.setEnabled(true);
        }
    }

    /** 
     * Pauses the execution of the Plexil plan by the Universal Executive
     * when directed to by the user. 
     */
    public void blockViewer() {
        if (shouldBlock()) {
            statusMessageHandler.showStatus("Plan execution is paused. " +
                    pauseAction.getAcceleratorDescription() +
                    " to resume, or " +
                    stepAction.getAcceleratorDescription() +
                    " to step",
                    Color.RED);

            if (luvBreakPointHandler.getBreakPoint() != null && shouldHighlight) {
                TreeTableView.getCurrent().highlightRow(luvBreakPointHandler.getBreakPoint().getModel());
            }

            luvBreakPointHandler.clearBreakPoint();

            // wait here for user action
            while (shouldBlock()) {
                try {
                    Thread.sleep(50);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }

            TreeTableView.getCurrent().unHighlightRow();
            shouldHighlight = true;
        }
    }

    /** Returns the current instance of the Luv application. 
     *  @return the current instance of the Luv application */
    public static Luv getLuv() {
        return theLuv;
    }
    
    /** Gantt Viewer
     *  Opens the Gantt Viewer in a browser.
     *  @return OpenGanttViewer */
    public OpenGanttViewer getGanttViewer() {
	return openGanttViewer;
    }

    /** Returns the current instance of the Luv temp port file. 
     *  @return the current instance of the Luv temp port file */
    public static LuvTempFile getPortFile() {
        return portFile;
    }

    /** Returns the current instance of the Luv ViewHandler.
     *  @return the current instance of the Luv ViewHandler */
    public ViewHandler getViewHandler() {
        return viewHandler;
    }

    /** Returns the current instance of the Luv FileHandler.
     *  @return the current instance of the Luv FileHandler */
    public FileHandler getFileHandler() {
        return fileHandler;
    }

    /** Returns the current instance of the Luv ExecutionHandler.
     *  @return the current instance of the Luv ExecutionHandler */
    public ExecutionHandler getExecutionHandler() {
        return executionHandler;
    }

    /** Returns the current instance of the Luv LuvBreakPointHandler.
     *  @return the current instance of the Luv LuvBreakPointHandler */
    public LuvBreakPointHandler getLuvBreakPointHandler() {
        return luvBreakPointHandler;
    }

    /** Returns the current instance of the Luv StatusMessageHandler.
     *  @return the current instance of the Luv StatusMessageHandler */
    public StatusMessageHandler getStatusMessageHandler() {
        return statusMessageHandler;
    }

    /** Returns the current instance of the Luv HideOrShowWindow.
     *  @return the current instance of the Luv HideOrShowWindow */
    public HideOrShowWindow getHideOrShowWindow()
    {
        if (hideOrShowWindow == null)
            hideOrShowWindow =
                new HideOrShowWindow(properties.getProperty(PROP_HIDE_SHOW_LIST, UNKNOWN));
        return hideOrShowWindow;
    }

    /** Returns the current instance of the Luv DebugWindow.
     *  @return the current instance of the Luv DebugWindow */
    public DebugWindow getDebugWindow() {
        return debugWindow;
    }    
    
    /** Returns the current instance of the Listener Port GUI.
     *  @return the current instance of the Listener Port GUI */
    public LuvPortGUI getPortGUI() {
        return portGui;
    }
    
    /** Returns the current instance of the Library Loader GUI.
     *  @return the current instance of the Library Loader GUI */
    public LibraryLoader getLibraryLoader() {
        return libraryLoader;
    }
    
    /** Returns the current instance of the Executive GUI.
     *  @return the current instance of the Executive GUI */
    public ExecSelect getExecSelect() {
        return execSelect;
    }    

    /** Returns the current instance of the Luv DebugCFGWindow.
     *  @return the current instance of the Luv DebugCFGWindow */
    public CreateCFGFileWindow getCreateCFGFileWindow() {
        return createCFGFileWindow;
    }

    /** Returns the current instance of the Luv SourceWindow.
     *  @return the current instance of the Luv SourceWindow */
    public SourceWindow getSourceWindow() {
        return sourceWindow;
    }

    /** Returns the current instance of the Luv RegexModelFilter.
     *  @return the current instance of the Luv RegexModelFilter */
    public RegexModelFilter getRegexModelFilter() {
        return regexFilter;
    }

    /** Returns the current instance of the Luv Properties.
     *  @return the current instance of the Luv Properties */
    public Properties getProperties() {
        return properties;
    }

    /** Returns the current instance of the Luv Model.
     *  @return the current instance of the Luv Model */
    public Model getCurrentPlan() {
        return currentPlan;
    }

    /** Returns the current instance of the Luv viewMenu.
     *  @return the current instance of the Luv viewMenu */
    public JMenu getViewMenu() {
        return viewMenu;
    }

    /** Returns the current instance of the Luv fileMenu.
     *  @return the current instance of the Luv fileMenu */
    public JMenu getFileMenu() {
        return fileMenu;
    }

    /** Returns the current instance of the Luv runMenu.
     *  @return the current instance of the Luv runMenu */
    public JMenu getRunMenu() {
        return runMenu;
    }

    /** Returns the current instance of the Luv debugMenu.
     *  @return the current instance of the Luv debugMenu */
    public JMenu getDebugMenu() {
        return debugMenu;
    }

    /** Returns the current instance of the Luv recentRunMenu.
     *  @return the current instance of the Luv recentRunMenu */
    public JMenu getRecentRunMenu() {
        return recentRunMenu;
    }

    /** Returns whether the current Plexil plan is stepping.
     *  @return the current instance of planStep */
    public boolean getPlanStep() {
        return planStep;
    }

    /** Returns whether the current Plexil plan is paused.
     *  @return the current instance of planPaused */
    public boolean getPlanPaused() {
        return planPaused;
    }

    /** Returns whether the current Plexil plan is executing.
     *  @return the current instance of isExecuting */
    public boolean getIsExecuting() {
        return isExecuting;
    }

    public boolean getIsExtendedViewOn() {
        return extendedViewOn;
    }
    
    public int getPort() {
    	return luvPort;
    }

    public int getPrevPort() {
    	return luvPrevPort;
    }
    
  	public int getPid() {
  		return pid;
  	}
  	
  	public void setPid(int pid) {
  		this.pid = pid;
  	}

    /** Returns whether Luv currently allows breaks.
     *  @return the current instance of allowBreaks */
    public boolean breaksAllowed() {
        return allowBreaks;
    }       

    /** Returns Which application is in use.
     *  @return the current instance of mode */
    public int getAppMode() {
        return appMode;
    }      
    
    /** Returns whether viewer invokes static checker.
     *  @return the current instance of checkPlan */
    public boolean checkPlan() {
        return checkPlan;
    }      
    
    /** Returns whether the currently executing Plexil plan should pause. 
     *  @return the whether the flag for pausing a plan is set and the flag 
     *  for stepping a plan is not set */
    public boolean shouldBlock() {
        return planPaused && !planStep;
    }

    /** Sets the flag that indicates whether or not the current Plexil plan is 
     *  executing.
     *  @param value sets the flag that indicates whether the current Plexil 
     *  plan is executing or not
     */
    public void setIsExecuting(boolean value) {
        isExecuting = value;
    }

    /** Sets the flag that indicates whether or not the current Plexil plan is 
     *  paused. 
     *  @param value sets the flag that indicates whether the current Plexil 
     *  plan is paused or not
     */
    public void setIsPaused(boolean value) {
        planPaused = value;
    }

    /** Sets the flag that indicates whether a new Plexil plan has been loaded. 
     *  @param value sets the flag that indicates whether there is a new Plexil 
     *  plan
     */
    public void setNewPlan(boolean value) {
        newPlan = value;
    }

    /** Sets the flag that indicates whether the Luv application is currently
     *  allowing breaks. Updates items under the Run menu accordingly.
     *  @param value sets the flag that indicates whether breaks are allowed
     */
    public void setBreaksAllowed(boolean value) {
        allowBreaks = value;
        updateBlockingMenuItems();
    }
    
    /** Sets the flag that indicates whether the Luv application is currently
     *  using TestExec or Universal Exec.
     *  @param value sets the flag that indicates whether TestExec or Universal Exec
     */
    public void setAppMode(int mode) {
        appMode = mode;        
    }
    
    /** Sets the flag that indicates whether the application is currently
     *  statically checking the plan.
     *  @param value sets the flag that indicates plan check
     */
    public void setCheckPlan(boolean value) {
        checkPlan = value;        
    }

    /** Sets the flag that indicates whether the Luv application should 
     *  highlist rows in pink in the model tree.
     *  @param value sets the flag that indicates whether rows can be highlighted
     */
    public void setShouldHighlight(boolean value) {
        shouldHighlight = value;
    }

    public void setExtendedViewOn(boolean value) {
        extendedViewOn = value;
    }       

    /** Modifies the state of certain menu items based on whether the Universal 
     *  Executive is running and whether it blocks.
     * */
    public void updateBlockingMenuItems() {
        // Pause/resume not useful if exec isn't listening            
        if (isExecuting) {
            if (allowBreaks) {
                runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(true);
                runMenu.getItem(STEP_MENU_ITEM).setEnabled(true);
            } else {
                runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
                runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
            }
            runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
            runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(false);
        } else {
            runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
            runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
            runMenu.getItem(BREAK_MENU_ITEM).setEnabled(true);

            if (luvBreakPointHandler.breakpointsExist()) {
                runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(true);
            } else {
                runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(false);
            }
        }
    }

    /** Enables or Disables the Remove Breaks menu item.
     *  @param value to enable or disable the Remove Breaks menu item
     * */
    public void enableRemoveBreaksMenuItem(boolean value) {
        if (runMenu.getComponents().length > 0) {
            runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(value);
        }
    }

    private void createMenuBar(JMenuBar menuBar)
    {
        fileMenu = new JMenu("File");
        menuBar.add(fileMenu);        
        LoadRecentAction.updateRecentMenu();
        fileMenu.add(new LuvAction("Configuration",
                                   "Window to Change Executive.",
                                   VK_E,
                                   META_MASK) {

                // *** FIXME!! ***
                public void actionPerformed(ActionEvent e) {
                    JComponent newContentPane = execSelect; 
                    newContentPane.setOpaque(true);    			
                    execSelect.getFrame().setContentPane(newContentPane);
                    execSelect.loadExecSelect();
                    execSelect.getFrame().pack();    			
                    execSelect.getFrame().setVisible(true);
                    execSelect.backupNames();
                }
            }
                     );
        fileMenu.add(new LuvAction("Reload",
                                   "Reload currently loaded files.",
                                   VK_R,
                                   META_MASK) {

                public void actionPerformed(ActionEvent e) {
                    execSelect.reload();
                }
            }
                     );
        fileMenu.add(new JSeparator());
        fileMenu.add(new LuvAction("Exit", "Terminate this program.", VK_ESCAPE) {
                public void actionPerformed(ActionEvent e) {                  
                    System.exit(0);                    
                }
            }
            );

        runMenu = new JMenu("Run");
        menuBar.add(runMenu);
        pauseAction = new LuvAction("Pause/Resume plan",
                      "Pause or resume an executing plan, if it is blocking.",
                      VK_ENTER) {
                public void actionPerformed(ActionEvent e) {
                    if (getIsExecuting()) {
                        setIsPaused(!planPaused);
                        statusMessageHandler.showStatus((planPaused ? "Pause" : "Resume") + " requested",
                                                        Color.BLACK,
                                                        1000);
                        if (planPaused)
                            pausedState();
                        else
                            executionState();
                    }
                }
            };
        runMenu.add(pauseAction);
        stepAction = new LuvAction("Step",
                                   "Step a plan, pausing it if is not paused.",
                                   VK_SPACE) {

                public void actionPerformed(ActionEvent e) {
                    if (getIsExecuting()) {
                        if (!planPaused) {
                            pausedState();
                            statusMessageHandler.showStatus("Pause requested", Color.BLACK, 1000);
                        } else {
                            stepState();
                            statusMessageHandler.showStatus("Step plan", Color.BLACK, 1000);
                        }
                    }
                }
            };
        runMenu.add(stepAction);
        allowBreaksAction = new LuvAction("Enable Breaks",
                                          "Select this to enable or disable breakpoints.",
                                          VK_F5) {

                public void actionPerformed(ActionEvent e) {
                    if (!getIsExecuting()) {
                        setBreaksAllowed(!allowBreaks);
                        if (allowBreaks) {
                            enabledBreakingState();
                            statusMessageHandler.showStatus("Enabled breaks", Color.GREEN.darker(), 1000);
                        } else {
                            disabledBreakingState();
                            statusMessageHandler.showStatus("Disabled breaks", Color.RED, 1000);
                        }
                    }
                }
            };
        runMenu.add(allowBreaksAction);
        runMenu.add(new LuvAction("Remove All Breakpoints",
                                  "Remove all breakpoints from this plan.") {

                public void actionPerformed(ActionEvent e) {
                    luvBreakPointHandler.removeAllBreakPoints();
                    statusMessageHandler.showStatus("All breakpoints have been removed", 1000);
                }
            }
                    );                
        runMenu.add(new JSeparator());
        execAction = new LuvAction("Execute Plan",
                                   "Execute currently loaded plan.",
                                   VK_F6) {

                public void actionPerformed(ActionEvent e) {
                    try {
                        if (!isExecuting) {
                            if (!executionHandler.runExec()) {
                                statusMessageHandler.showStatus("Stopped execution", Color.lightGray, 1000);
                                readyState();
                            }
                        } else {
                            stopExecutionState();
                        }
                    } catch (IOException ex) {
                        statusMessageHandler.displayErrorMessage(ex, "ERROR: exception occurred while executing plan");
                    }
                }
            };
        runMenu.add(execAction);

        viewMenu = new JMenu("View");
        menuBar.add(viewMenu);
        viewMenu.add(new LuvAction("Expand All",
                                   "Expand all tree nodes.",
                                   VK_EQUALS) {

                public void actionPerformed(ActionEvent e) {
                    TreeTableView.getCurrent().expandAllNodes();
                    viewHandler.refreshRegexView();
                }
            }
                     );
        viewMenu.add(new LuvAction("Collapse All",
                                   "Collapse all tree nodes.",
                                   VK_MINUS) {

                public void actionPerformed(ActionEvent e) {
                    TreeTableView.getCurrent().collapseAllNodes();
                    viewHandler.refreshRegexView();
                }
            }
                     );
        viewMenu.add(new LuvAction("Hide/Show Nodes...",
                                   "Hide or Show specific nodes by full or partial name.",
                                   VK_H,
                                   META_MASK) {

                public void actionPerformed(ActionEvent e) {
                    getHideOrShowWindow().open();
                }
            }
                     );
        viewMenu.add(new LuvAction("Find...",
                                   "Find node by name.",
                                   VK_F,
                                   META_MASK) {
                public void actionPerformed(ActionEvent e) {
                    FindWindow.open(properties.getProperty(PROP_SEARCH_LIST, UNKNOWN));
                }
            }
                     );
        viewMenu.add(new JSeparator());
        extendedViewAction = new LuvAction("Switch to Core Plexil View",
                                   "Switches between Normal or Core Plexil views. Normal is the default.",
                                   VK_F8) {
                public void actionPerformed(ActionEvent e) {
                    if (extendedViewOn) {
                        extendedViewAction.putValue(NAME, "Switch to Normal Plexil View");
                        getRegexModelFilter().corePlexilView();
                        viewHandler.refreshRegexView();
                    } else {
                        extendedViewAction.putValue(NAME, "Switch to Core Plexil View");
                        getRegexModelFilter().extendedPlexilView();
                        viewHandler.refreshRegexView(); 
                    }
                    setExtendedViewOn(!extendedViewOn);
                }
            };
        viewMenu.add(extendedViewAction);
        viewMenu.add(new JSeparator());
        viewMenu.add(new LuvAction("View Source Files",
                                   "Displays Source Files.",
                                   VK_F9) {

                public void actionPerformed(ActionEvent e) {
                    try {
                        sourceWindow.open(currentPlan);
                    } catch (FileNotFoundException ex) {
                        statusMessageHandler.displayErrorMessage(ex, "ERROR: exception occurred while opening source window");
                    }
                }
            }
                     );
        viewMenu.add(new JSeparator());
        viewMenu.add(new LuvAction("Show Gantt Viewer",
                                   "Show viewer window with timeline/Gantt views of a plan post-execution.",
                                   VK_V,
                                   META_MASK) {
                public void actionPerformed(ActionEvent e) {	
                    openGanttViewer.openURL();
                }
            }
                     );

        debugMenu = new JMenu("Debug");
        menuBar.add(debugMenu);
        luvGetDebugWindowAction = new LuvAction("Show Debug Window",
                                    "Show window with status and debugging information.",
                                    VK_D,
                                    META_MASK) {
                public void actionPerformed(ActionEvent e) {
                    debugWindow.setVisible(!debugWindow.isVisible());
                    if (debugWindow.isVisible()) {
                        luvGetDebugWindowAction.putValue(NAME, "Hide Debug Window");
                    } else {
                        luvGetDebugWindowAction.putValue(NAME, "Show Debug Window");
                    }
                }
            };
        debugMenu.add(luvGetDebugWindowAction);
        debugMenu.add(new LuvAction("Change Server port",
                                    "Change viewer server listening port.",
                                    VK_S,
                                    META_MASK) {

                // *** FIXME ***
                public void actionPerformed(ActionEvent e) {
                    JComponent newContentPane = portGui; 
                    newContentPane.setOpaque(true);    			
                    portGui.getFrame().setContentPane(newContentPane);
                    portGui.getFrame().pack();
                    portGui.refresh();
                    portGui.getFrame().setVisible(true);
                    if(portGui.isEmpty())
                        statusMessageHandler.displayErrorMessage(null, "ERROR: No ports avaliable.  Close an open instance and try again");
                }
            }
                      );
        debugMenu.add(new LuvAction("Create Debug Configuration File...",
                                    "Create and Customize a debug configuration file.",
                                    VK_G,
                                    META_MASK) {

                public void actionPerformed(ActionEvent e) {
                    try {
                        createCFGFileWindow.open();
                    } catch (FileNotFoundException ex) {
                        statusMessageHandler.displayErrorMessage(ex, "ERROR: exception occurred while opening CFG Debug window");
                    }
                }
            }
                      );
        debugMenu.add(new LuvAction("About Plexil Viewer Window",
                                    "Show window with Plexil Viewer about information.") {

                public void actionPerformed(ActionEvent e) {
                    String info =
                        "Application:   PLEXIL " + Constants.PLEXIL_VERSION + " " + Constants.PLEXIL_COPYRIGHT +
                        "Website:   "+ Constants.PLEXIL_WEBSITE + "\n" +
                        "Java:        " + System.getProperty("java.version") + "; " + System.getProperty("java.vm.name") + " " + System.getProperty("java.vm.version") + "\n" +
                        "System:    " + System.getProperty("os.name") + " version " + System.getProperty("os.version") + " running on " + System.getProperty("os.arch") + "\n" +
                        "Userdir:    " + System.getProperty("user.dir") + "\n";

                    ImageIcon icon = getIcon(ABOUT_LOGO);

                    JOptionPane.showMessageDialog(theLuv,
                                                  info,
                                                  "About Plexil Viewer",
                                                  JOptionPane.INFORMATION_MESSAGE,
                                                  icon);
                }
            }
                      );   
    }

    /** Sets the title of the Luv application. */
    public void setTitle() {
        if (currentPlan != null && !currentPlan.getPlanName().equals(UNKNOWN)) {
            String title = "Plexil Viewer - " + currentPlan.getPlanName();;            

            if (!currentPlan.getScriptName().equals(UNKNOWN)) {
                title += " + " + currentPlan.getScriptName();
            }

            setTitle(title);
        }
        else if (isExecuting) {
        	setTitle("Plexil Viewer");
        }
        else {
            setTitle("Plexil Viewer - no plan loaded");
        }
    }

    /** Sets a program wide property 
     *  
     *  @param key
     *  @param value
     */
    public void setProperty(String key, String value) {
        properties.setProperty(key, value);
    }

    /** Sets a program wide property 
     * 
     *  @param key
     *  @param value
     */
    public void setProperty(String key, ArrayList<String> value) {
        properties.define(key, value);
    }

    /** Returns a program wide property
     * 
     *  @param key
     *  @return the program wide property
     */
    public String getProperty(String key) {
        return properties.getProperty(key, UNKNOWN);
    }

    //
    // State management
    //

	/**
	 * Sets the Luv application to a Start State, this occurs as when the Luv
	 * application opens for the first time.
     * @note Should only be called from within this class.
	 */
	private void startState()
    {
		disableAllMenus();
		allowBreaks = false;
		isExecuting = false;
		planPaused = false;
        updateBlockingMenuItems();

		Model.getRoot().clear();

		viewHandler.clearCurrentView();
		statusMessageHandler.clearStatusMessageQ();
		luvBreakPointHandler.removeAllBreakPoints();

		// reset all menu items

		execSelect.getSaveBut().setEnabled(true);
		fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(true);
		fileMenu.setEnabled(true);

		updateBlockingMenuItems();
		allowBreaksAction.putValue(NAME, "Enable Breaks");
		runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
		runMenu.getItem(EXEC_SELECT_ITEM).setEnabled(true);
		runMenu.setEnabled(true);

		viewMenu.setEnabled(true);
		debugMenu.setEnabled(true);
	}

	/**
	 * Sets the Luv application to a Ready State.
	 */
	public void readyState()
    {
		// set only certain luv viewer variables
		planPaused = false;
		planStep = false;
		fileHandler.setStopSearchForMissingLibs(false);

		PlexilPlanHandler.resetRowNumber();

		setTitle();

		luvBreakPointHandler.clearBreakPoint();

		// set certain menu items

		execAction.putValue(NAME, "Execute Plan");

		execSelect.getSaveBut().setEnabled(true);
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
			viewMenu.getItem(HIDE_OR_SHOW_NODES_MENU_ITEM).setEnabled(true);
			viewMenu.getItem(FIND_MENU_ITEM).setEnabled(true);
			viewMenu.getItem(EPX_VIEW_MENU_ITEM).setEnabled(true);
			viewMenu.setEnabled(true);
		} else
			viewMenu.setEnabled(false);

		debugMenu.setEnabled(true);
	}

	/**
	 * Sets the Luv application to a Finished Execution State and occurs when
	 * EOF on the LuvListener stream is received.
	 */
	public void finishedExecutionState() {
		isExecuting = false;
		planPaused = false;
		planStep = false;
		fileHandler.setStopSearchForMissingLibs(false);

		// set certain menu items

		execAction.putValue(NAME, "Execute Plan");

		execSelect.getSaveBut().setEnabled(true);
		fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
		fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(true);
		fileMenu.setEnabled(true);

		updateBlockingMenuItems();

		runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(true);
		runMenu.setEnabled(true);

		if (viewMenu.getMenuComponentCount() > 0) {			
			viewMenu.getItem(EXPAND_MENU_ITEM).setEnabled(
					true);
			viewMenu.getItem(COLLAPSE_MENU_ITEM).setEnabled(
					true);
			viewMenu.getItem(HIDE_OR_SHOW_NODES_MENU_ITEM)
					.setEnabled(true);
			viewMenu.getItem(FIND_MENU_ITEM).setEnabled(true);
			viewMenu.getItem(EPX_VIEW_MENU_ITEM).setEnabled(
					true);
			viewMenu.setEnabled(true);
		} else
			viewMenu.setEnabled(false);

		debugMenu.setEnabled(true);

		statusMessageHandler.showStatus("Execution stopped", Color.BLUE);
		statusMessageHandler.showChangeOnPort("Listening on port " + getPort());
	}

	/**
	 * Sets the Luv application to a Pre Execution State and occurs just before
	 * the loaded Plexil Plan is about to execute.
	 */
	public void preExecutionState()
    {
		shouldHighlight = false;
		currentPlan.resetMainAttributesOfAllNodes();

		execSelect.getSaveBut().setEnabled(false);
		fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(false);
		runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
		runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(
				false);
		runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);
	}

	/**
	 * Sets the Luv application to an Execution State and occurs while the
	 * loaded Plexil Plan is executing.
	 */
	public void executionState()
    {
		isExecuting = true;
		
		statusMessageHandler.showIdlePortMessage();
		statusMessageHandler.showStatus("Executing...",
				Color.GREEN.darker());

		execAction.putValue(NAME, "Stop Execution");

		execSelect.getSaveBut().setEnabled(true);
		fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
		runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(true);

		if (allowBreaks)
			enabledBreakingState();
		else
			disabledBreakingState();

		updateBlockingMenuItems();
	}

	/**
	 * Sets the Luv application to a Stopped Execution State and occurs when the
	 * user manually stops the execution of a Plexil Plan.
	 */
	public void stopExecutionState() throws IOException {
		executionHandler.killUEProcess();

		planPaused = false;
		planStep = false;
	}

	/**
	 * Sets the Luv application to an Open Plan State and occurs when a new
	 * Plexil Plan is newly opened in the Luv application.
	 */
	public void openPlanState()
    {
		luvBreakPointHandler.removeAllBreakPoints();
		currentPlan.resetMainAttributesOfAllNodes();
		currentPlan.addScriptName(UNKNOWN);
		NodeInfoWindow.closeNodeInfoWindow();
		readyState();
	}

	/**
	 * Sets the Luv application to an Load Recent Run State and occurs when a
	 * reccently loaded Plexil Plan is newly opened in the Luv application from
	 * the recently run menu.
	 */
	public void loadRecentRunState()
    {
		luvBreakPointHandler.removeAllBreakPoints();

		currentPlan.resetMainAttributesOfAllNodes();

		NodeInfoWindow.closeNodeInfoWindow();

		readyState();
	}

	/**
	 * Sets the Luv application to an Reload Plan State and occurs when a
	 * currently loaded Plexil Plan is refreshed in the Luv application.
	 */
	public void reloadPlanState()
    {
		if (isExecuting && executionHandler.isAlive())
			try {
				stopExecutionState();
			} catch (IOException e) {
				e.printStackTrace();
			}

		currentPlan.resetMainAttributesOfAllNodes();

		readyState();
	}

	/**
	 * Sets the Luv application to an Paused State and occurs when the Luv
	 * application has breaks enabled and is at the beginning of executing a
	 * Plexil Plan or the user manually pauses a currently running Plexil Plan.
	 */
	public void pausedState()
    {
		planPaused = true;
		planStep = false;
		updateBlockingMenuItems();
	}

	/**
	 * Sets the Luv application to an Step State and occurs when the Luv
	 * application has breaks enabled and the user manually steps through a
	 * currently running Plexil Plan.
	 */
	public void stepState()
    {
		planPaused = false;
		planStep = true;
		updateBlockingMenuItems();
	}

	//
	// Sub-states
	//

	/**
	 * Sets the Luv application to an Disabled Breaking State and occurs when
	 * the Luv application has breaks disabled.
	 */
	public void disabledBreakingState()
    {
		allowBreaks = false;

		allowBreaksAction.putValue(NAME, "Enable Breaks");

		setForeground(lookupColor(MODEL_DISABLED_BREAKPOINTS));

		Set<LuvBreakPoint> breakPoints =
            luvBreakPointHandler.getBreakPointMap().keySet();

		for (BreakPoint bp : breakPoints)
			bp.setEnabled(allowBreaks);

		viewHandler.refreshView();

		updateBlockingMenuItems();
	}

	/**
	 * Sets the Luv application to an Enabled Breaking State and occurs when the
	 * Luv application has breaks enabled.
	 */
	public void enabledBreakingState()
    {
		allowBreaks = true;
		allowBreaksAction.putValue(NAME, "Disable Breaks");
		setForeground(lookupColor(MODEL_ENABLED_BREAKPOINTS));

		Set<LuvBreakPoint> breakPoints =
            luvBreakPointHandler.getBreakPointMap().keySet();

		for (LuvBreakPoint bp : breakPoints)
			if (!bp.getReserveBreakStatus())
				bp.setEnabled(allowBreaks);

		viewHandler.refreshView();

		updateBlockingMenuItems();
	}

    // Utility used in startState()
	private void disableAllMenus()
    {
		execSelect.getSaveBut().setEnabled(false);
		fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(false);
		fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(false);
		fileMenu.setEnabled(false);

		runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
		runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
		runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
		runMenu.getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(false);		
		runMenu.getItem(EXEC_SELECT_ITEM).setEnabled(false);
		runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);
		runMenu.setEnabled(false);

		if (viewMenu.getMenuComponentCount() > 0) {			
			viewMenu.getItem(EXPAND_MENU_ITEM).setEnabled(false);
			viewMenu.getItem(COLLAPSE_MENU_ITEM).setEnabled(false);
			viewMenu.getItem(HIDE_OR_SHOW_NODES_MENU_ITEM) .setEnabled(false);
			viewMenu.getItem(FIND_MENU_ITEM) .setEnabled(false);
			viewMenu.getItem(EPX_VIEW_MENU_ITEM).setEnabled(false);
		}
		viewMenu.setEnabled(false);
		debugMenu.setEnabled(false);
	}

}
