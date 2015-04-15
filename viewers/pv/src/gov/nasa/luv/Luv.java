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
import java.util.concurrent.Semaphore;
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
    private boolean planPaused;
    private boolean planStep;
    private boolean isExecuting;
    private boolean extendedViewOn;
    private boolean shouldHighlight;

    // Step/breakpoint semaphore
    private Semaphore stopSem;

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
    private ExecSelectDialog execSelect;
    private RegexNodeFilter regexFilter;
    private int luvPrevPort = 0;
    private int pid;

    //Gantt Viewer
    private OpenGanttViewer openGanttViewer;
    
    //Luv SocketServer
    private LuvSocketServer luvServer;

    // Luv menus
    private JMenu fileMenu;
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
    // *** N.B. Distinct from "the root Model".
    private Model currentPlan;

    // current working instance of luv
    private static Luv theLuv;
    
    // persistent store for preferences
    private Settings settings;

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
     * Universal Executive. Starts socket server with requested port.
     */
    public Luv(String[] args) throws IOException
    {
        theLuv = this;
        Runtime.getRuntime().addShutdownHook(new MyShutdownHook());
        init(args);

        viewHandler.showModelInViewer(currentPlan);
        constructFrame(getContentPane());
        luvServer.startServer(settings.getPort());

        startState();

        // Display the plan if one was named on the command line
        if (settings.getPlanSupplied()
            && settings.getPlanLocation().isFile()) {
            // Pause long enough to see start screen (?)
            try {
                Thread.currentThread().sleep(5000);
            } catch (InterruptedException e) {
            }

            loadPlan(settings.getPlanLocation());
            readyState();
        }
    }

    private void init(String[] cmdLineArgs)
    {
        planPaused = false;
        planStep = false;
        isExecuting = false;
        extendedViewOn = true;
        shouldHighlight = true;

        stopSem = new Semaphore(0, true); // initially blocked

        settings = new Settings();
        settings.load(); // load saved preferences
        settings.parseCommandOptions(cmdLineArgs);
        settings.save(); // save prefs

        // Build debug window early
        debugWindow = new DebugWindow(); // depends on settings

        fileHandler = new FileHandler();
        viewHandler = new ViewHandler();
        statusMessageHandler = new StatusMessageHandler();
        luvBreakPointHandler = new LuvBreakPointHandler();
        executionHandler = new ExecutionHandler();
        regexFilter = new RegexNodeFilter(true);
        luvServer = new LuvSocketServer();

        // Create these on demand
        portGui = null;
        execSelect = null;
        hideOrShowWindow = null;
        sourceWindow = null;
        libraryLoader = null;
        createCFGFileWindow = null;
        openGanttViewer = null;

        currentPlan = new Model();
    }
        
    private void stopServer() {
        if (luvServer.isGood())
            luvServer.stopServer();
    }

    /** 
     * Redefines port argument based upon string argument.
     * Restarts listening server with new port. 
     * Sets port in settings.
     */
    public void changePort(int newPort) {
        if (luvServer != null && LuvSocketServer.portFree(newPort)) {
            // Shut down old
            int oldPort = settings.getPort();
            try {
                stopServer();
            } catch (Exception e) {
                statusMessageHandler.displayErrorMessage(e, "Error occured while stopping server on port " + oldPort);
                return;
            }

            // Launch new
            try {
                luvServer.startServer(newPort);
            } catch (Exception e) {
                statusMessageHandler.displayErrorMessage(e, "Error occured while starting server on port " + newPort);
            }
            settings.setPort(newPort);
            settings.setPortSupplied(true); // because this code only called as a result of user action
            settings.save();
        }
    }

    private void constructFrame (Container frame)
    {
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout());
        setBackground(settings.getColor(PROP_WIN_BCLR));

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
                settings.set(PROP_WIN_LOC, getLocation());
                settings.set(PROP_WIN_SIZE, getSize());
                settings.set(PROP_DBWIN_LOC, debugWindow.getLocation());
                settings.set(PROP_DBWIN_SIZE, debugWindow.getSize());
                settings.save();
            }
        });

        setLocation(settings.getPoint(PROP_WIN_LOC));
        setPreferredSize(settings.getDimension(PROP_WIN_SIZE));

        regexFilter.extendedPlexilView();
        regexFilter.updateRegexList();

        setTitle();
        pack();

        setVisible(true);
    }

    // Called when the user selects or reloads a plan
    public void loadPlan(File plan) {
        PlexilPlanHandler.resetRowNumber();
        Model m = fileHandler.readPlan(plan);
        if (m != null)
            handleNewPlan(m);
        openPlanState();
        statusMessageHandler.showStatus("Plan " + currentPlan.getPlanFile().toString() + " loaded", 1000);
    }

    /** Handles the Plexil plan being loaded into the Luv application. 
     *  Called from PlexilPlanHandler.endElement(), which will be invoked 
     *  by both Luv (directly) and the Universal Executive (via the Luv listener stream).
     *
     * @param plan the Plexil plan to be loaded into the Luv application
     */

    // *** N.B. Called in two circumstances:
    //  1. User has just requested a plan in the configuration dialog, or requested reloading one.
    //  2. Exec has just transmitted the plan it is about to execute.
    // If the Exec is running as a slave of Luv, both events will happen, in that order. In different threads.
    public void handleNewPlan(Model plan) {
        // New plans are added at the end of the model root's children.
        Model other = (Model) Model.getRoot().findChildByName(plan.getNodeName());
        boolean newPlan = false;
        if (plan == other) {
            // brand new 'plan' loaded
            newPlan = true;
        } else if (plan.equivalent(other)) {
            // same 'plan' loaded, so use 'other' previous plan
            Model.getRoot().removeChild(plan);
            plan = other;
        } else {
            // new 'plan' has same root name as 'other' previous plan,
            // but new 'plan' supersedes
            Model.getRoot().removeChild(other);
            newPlan = true;
        }

        if (newPlan) {
            NodeInfoWindow.closeNodeInfoWindow();
            luvBreakPointHandler.removeAllBreakPoints();
            currentPlan = plan;
            Model.getRoot().planChanged();
            viewHandler.showModelInViewer(currentPlan);
        }

        setTitle(); // ??
    }

    // Called when a plan is received from the Exec.
    public void newPlanFromExec(Model plan) {
        handleNewPlan(plan);

        readyState();
        preExecutionState();
        executionState();

        // Determine if the Luv Viewer should pause before executing.
        if (isExecuting && settings.blocksExec()) {
            pausedState();
            runMenu.setEnabled(true);
        }
    }


    /** 
     * Pauses the execution of the Plexil plan by the Universal Executive
     * when directed to by the user. 
     */
    public void blockViewer() {
        statusMessageHandler.showStatus("Plan execution is paused. " +
                                        pauseAction.getAcceleratorDescription() +
                                        " to resume, or " +
                                        stepAction.getAcceleratorDescription() +
                                        " to step",
                                        Color.RED);

        if (luvBreakPointHandler.getActiveBreakPoint() != null && shouldHighlight)
            TreeTableView.getCurrent().highlightRow(luvBreakPointHandler.getActiveBreakPoint().getNode());
        luvBreakPointHandler.clearActiveBreakPoint();

        // wait here for user action
        try {
            stopSem.acquire();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        TreeTableView.getCurrent().unHighlightRow();
        shouldHighlight = true;
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
        if (openGanttViewer == null)
            openGanttViewer = new OpenGanttViewer();
        return openGanttViewer;
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
                new HideOrShowWindow(settings.getStringList(PROP_HIDE_SHOW_LIST));
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
        if (portGui == null)
            portGui = new LuvPortGUI();
        return portGui;
    }
    
    /** Returns the current instance of the Library Loader GUI.
     *  @return the current instance of the Library Loader GUI */
    public LibraryLoader getLibraryLoader() {
        if (libraryLoader == null)
            libraryLoader = new LibraryLoader();
        return libraryLoader;
    }
    
    /** Returns the current instance of the Executive GUI.
     *  @return the current instance of the Executive GUI */
    public ExecSelectDialog getExecSelectDialog() {
        if (execSelect == null)
            execSelect = new ExecSelectDialog(this);
        return execSelect;
    }    

    /** Returns the current instance of the Luv DebugCFGWindow.
     *  @return the current instance of the Luv DebugCFGWindow */
    public CreateCFGFileWindow getCreateCFGFileWindow() {
        if (createCFGFileWindow == null)
            createCFGFileWindow = new CreateCFGFileWindow();
        return createCFGFileWindow;
    }

    /** Returns the current instance of the Luv SourceWindow or null. */
    public SourceWindow getSourceWindow() {
        return sourceWindow;
    }

    /** Returns the existing SourceWindow, or constructs one if none exists. */
    public SourceWindow ensureSourceWindow() {
        if (sourceWindow == null)
            sourceWindow = new SourceWindow();
        return sourceWindow;
    }

    /** Returns the current instance of the Luv RegexNodeFilter.
     *  @return the current instance of the Luv RegexNodeFilter */
    public RegexNodeFilter getRegexNodeFilter() {
        return regexFilter;
    }

    /** Returns the current instance of the Luv settings. */
    public Settings getSettings() {
        return settings;
    }

    public AppType getAppMode() {
        return settings.getAppMode();
    }

    // Used by ExecSelectDialog, others?
    public void setAppMode(AppType mode) {
        if (mode == settings.getAppMode())
            return;
        // *** TODO: actually switch mode ***
    }

    /** Returns the current instance of the Luv Model.
     *  @return the current instance of the Luv Model */
    public Model getCurrentPlan() {
        return currentPlan;
    }

    /** Returns whether the current Plexil plan is stepping.
     *  @return the current instance of planStep */
    public boolean getPlanStep() {
        return planStep;
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
    	return settings.getPort();
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

    /** Returns whether Luv currently allows breaks. */
    public boolean breaksAllowed() {
        return settings.blocksExec();
    }       
    
    /** Returns whether viewer invokes static checker.
     *  @return the current instance of checkPlan */
    public boolean checkPlan() {
        return settings.checkPlan();
    }
    
    /** Returns whether the currently executing Plexil plan should pause. 
     *  @return the whether the flag for pausing a plan is set and the flag 
     *  for stepping a plan is not set */
    public boolean shouldBlock() {
        return planPaused && !planStep;
    }

    /** Sets the flag that indicates whether the Luv application is currently
     *  allowing breaks. Updates items under the Run menu accordingly.
     *  @param value sets the flag that indicates whether breaks are allowed
     */
    public void setBreaksAllowed(boolean value) {
        settings.setBlocksExec(value);
        settings.save();
        updateBlockingMenuItems();
    }
    
    /** Sets the flag that indicates whether the application is currently
     *  statically checking the plan.
     *  @param value sets the flag that indicates plan check
     */
    public void setCheckPlan(boolean value) {
        settings.setCheckPlan(value);
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
            if (settings.blocksExec()) {
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
        fileMenu.add(new LuvAction("Configuration",
                                   "Select an executive and its files.",
                                   VK_E,
                                   META_MASK) {

                public void actionPerformed(ActionEvent e) {
                    getExecSelectDialog().activate();
                }
            }
                     );
        fileMenu.add(new LuvAction("Reload",
                                   "Reload currently loaded files.",
                                   VK_R,
                                   META_MASK) {

                public void actionPerformed(ActionEvent e) {
                    reload();
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
                    if (isExecuting) {
                        planPaused = !planPaused;
                        statusMessageHandler.showStatus((planPaused ? "Pause" : "Resume") + " requested",
                                                        Color.BLACK,
                                                        1000);
                        if (planPaused)
                            pausedState();
                        else {
                            stopSem.release();
                            executionState();
                        }
                    }
                }
            };
        runMenu.add(pauseAction);
        stepAction = new LuvAction("Step",
                                   "Step a plan, pausing it if is not paused.",
                                   VK_SPACE) {

                public void actionPerformed(ActionEvent e) {
                    if (isExecuting) {
                        if (!planPaused) {
                            pausedState();
                            statusMessageHandler.showStatus("Pause requested", Color.BLACK, 1000);
                        } else {
                            stopSem.release();
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
                    if (!isExecuting) {
                        boolean allowBreaks = !settings.blocksExec();
                        settings.setBlocksExec(allowBreaks);
                        settings.save();
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
                    FindWindow.open(settings.get(PROP_SEARCH_LIST));
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
                        getRegexNodeFilter().corePlexilView();
                        viewHandler.refreshRegexView();
                    } else {
                        extendedViewAction.putValue(NAME, "Switch to Core Plexil View");
                        getRegexNodeFilter().extendedPlexilView();
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
                    ensureSourceWindow().open(currentPlan);
                }
            }
                     );
        viewMenu.add(new JSeparator());
        viewMenu.add(new LuvAction("Show Gantt Viewer",
                                   "Show viewer window with timeline/Gantt views of a plan post-execution.",
                                   VK_V,
                                   META_MASK) {
                public void actionPerformed(ActionEvent e) {	
                    getGanttViewer().openURL();
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
                public void actionPerformed(ActionEvent e) {
                    getPortGUI().activate();
                }
            }
                      );
        debugMenu.add(new LuvAction("Create Debug Configuration File...",
                                    "Create and Customize a debug configuration file.",
                                    VK_G,
                                    META_MASK) {

                public void actionPerformed(ActionEvent e) {
                    try {
                        getCreateCFGFileWindow().open();
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
        if (currentPlan != null && currentPlan.getPlanName() != null) {
            setTitle("Plexil Viewer - " + currentPlan.getPlanName());
        }
        else if (isExecuting) {
        	setTitle("Plexil Viewer");
        }
        else {
            setTitle("Plexil Viewer - no plan loaded");
        }
    }

    /** Sets a program wide property 
     *  @note Convenience method
     *  @param key
     *  @param value
     */
    public void setProperty(String key, String value) {
        settings.set(key, value);
    }

    /** Returns a program wide property
     *  @note Convenience method
     *  @param key
     *  @return the program wide property
     */
    public String getProperty(String key) {
        return settings.get(key);
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
        setBreaksAllowed(false); // ??
		isExecuting = false;
		planPaused = false;
        updateBlockingMenuItems();

		Model.getRoot().clear();

		viewHandler.clearCurrentView();
		statusMessageHandler.clearStatusMessageQ();
		luvBreakPointHandler.removeAllBreakPoints();

		// reset all menu items

		if (execSelect != null)
            execSelect.enableSaveButton();

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

        // *** FIXME ***
		PlexilPlanHandler.resetRowNumber();

		setTitle();

		luvBreakPointHandler.clearActiveBreakPoint();

		// set certain menu items

		execAction.putValue(NAME, "Execute Plan");

        if (execSelect != null)
            execSelect.enableSaveButton();

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

        if (execSelect != null)
            execSelect.enableSaveButton();

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
		statusMessageHandler.showChangeOnPort("Listening on port " + settings.getPort());
	}

	/**
	 * Sets the Luv application to a Pre Execution State and occurs just before
	 * the loaded Plexil Plan is about to execute.
	 */
	public void preExecutionState() {
		shouldHighlight = false;
		currentPlan.reset();

        if (execSelect != null)
            execSelect.disableSaveButton();

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
	public void executionState() {
		isExecuting = true;
		
		statusMessageHandler.showIdlePortMessage();
		statusMessageHandler.showStatus("Executing...",
                                        Color.GREEN.darker());

		execAction.putValue(NAME, "Stop Execution");

        if (execSelect != null)
            execSelect.enableSaveButton();

		fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
		runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(true);

		if (settings.blocksExec())
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
	public void openPlanState() {
		luvBreakPointHandler.removeAllBreakPoints();
		currentPlan.reset();
		NodeInfoWindow.closeNodeInfoWindow();
		readyState();
	}

	/**
	 * Sets the Luv application to an Reload Plan State and occurs when a
	 * currently loaded Plexil Plan is refreshed in the Luv application.
	 */
	public void reloadPlanState() {
        if (isExecuting && executionHandler.isAlive())
			try {
				stopExecutionState();
			} catch (IOException e) {
				e.printStackTrace();
			}

		currentPlan.reset();

		readyState();
	}

	/**
	 * Sets the Luv application to an Paused State and occurs when the Luv
	 * application has breaks enabled and is at the beginning of executing a
	 * Plexil Plan or the user manually pauses a currently running Plexil Plan.
	 */
	public void pausedState() {
		planPaused = true;
		planStep = false;
		updateBlockingMenuItems();
	}

	/**
	 * Sets the Luv application to an Step State and occurs when the Luv
	 * application has breaks enabled and the user manually steps through a
	 * currently running Plexil Plan.
	 */
	public void stepState() {
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
	public void disabledBreakingState() {
		settings.setBlocksExec(false);
		allowBreaksAction.putValue(NAME, "Enable Breaks");
		setForeground(lookupColor(NODE_DISABLED_BREAKPOINTS));

		luvBreakPointHandler.disableAllBreakPoints();
		viewHandler.refreshView();
		updateBlockingMenuItems();
	}

	/**
	 * Sets the Luv application to an Enabled Breaking State and occurs when the
	 * Luv application has breaks enabled.
	 */
	public void enabledBreakingState() {
		settings.setBlocksExec(true);
		allowBreaksAction.putValue(NAME, "Disable Breaks");
		setForeground(lookupColor(NODE_ENABLED_BREAKPOINTS));

        luvBreakPointHandler.enableAllBreakPoints();
		viewHandler.refreshView();
		updateBlockingMenuItems();
	}

    // Utility used in startState()
	private void disableAllMenus()
    {
        if (execSelect != null)
            execSelect.disableSaveButton();

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

    public void reload() {		
        if (isExecuting) {
            try {
                stopExecutionState();
                statusMessageHandler.displayInfoMessage("Stopping execution and reloading plan",
                                                        "Stopping execution");
            } catch (IOException ex) {
                statusMessageHandler.displayErrorMessage(ex, "ERROR: exception occurred while reloading plan");
            }
        }        
                
        if (execSelect != null)
            execSelect.refresh();

        File plan = settings.getPlanLocation();
        if (plan == null)
            return;
        
        if (plan.exists()) {
            loadPlan(plan);
        } else {
            statusMessageHandler.displayErrorMessage(null,
                                                     "ERROR: While reloading plan: unable to find plan file "
                                                     + plan.toString());
            reloadPlanState(); // *** FIXME ***
        }
        
        AppType mode = settings.getAppMode();
        if (mode == AppType.PLEXIL_SIM || mode == AppType.PLEXIL_TEST) {
            File script = settings.getScriptLocation();
            if (script != null && script.isFile() && script.canRead()) {
                fileHandler.loadScript(script);
                statusMessageHandler.showStatus("Script \""
                                                + currentPlan.getScriptFile().toString()
                                                + "\" loaded",
                                                1000);
            }
        }

        if (mode == AppType.PLEXIL_SIM || mode == AppType.PLEXIL_EXEC) {
            File config = settings.getConfigLocation();
            if (config == null && config.isFile() && config.canRead()) {
                fileHandler.loadConfig(config);
                statusMessageHandler.showStatus("Config "
                                                + theLuv.getCurrentPlan().getConfigFile().toString()
                                                + " loaded",
                                                1000);
            }
        }

        setTitle();
        if (sourceWindow != null)
            sourceWindow.refresh();
    }

    /* 
     * Clean up at exit
     */
    private class MyShutdownHook extends Thread
    {
    	public void run() {
            settings.save();
            if (luvServer.isGood())
                luvServer.stopServer();
        }
    }

}
