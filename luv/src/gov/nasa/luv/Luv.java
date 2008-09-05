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
import javax.swing.ToolTipManager;

import java.awt.Container;
import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;

import java.io.IOException;
import java.io.ByteArrayInputStream;

import java.io.File;
import static gov.nasa.luv.Constants.*;
import static gov.nasa.luv.LuvSplashScreen.*;

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
    
      private static int stepToStart                     = 0;            // if using step option at beginning
    
      private static boolean allowBreaks                 = false;        // is current instance of luv have breaks enabled?
      private static boolean executedViaLuvViewer        = false;        // was current instance of luv executed via the viewer itself?
      private static boolean executedViaCommandPrompt    = false;        // was current instance of luv executed via the command prompt?
      private static boolean isExecuting                 = false;        // is instance of luv currently executing?   
      private static boolean atStartScreen               = true;         // is current instance of luv only just started with no plan loaded?
      private static boolean dontLoadScriptAgain         = false;        // is script already loaded? if so, do not waste time loading it again
      private static boolean stopSearchForMissingLibs    = false;        // is library found? if so, stop searching for missing libraries 
      private static boolean openedPlanViaLuvViewer      = false;        // is current instance of luv executing a plan that was opened via the viewer itself?
      private static boolean planPaused                  = false;        // is instance of luv currently paused?    
      private static boolean planStep                    = false;        // is instance of luv currently stepping?
      private static boolean showConditions              = false;     
      private static boolean stopExecution               = false;
      private static boolean resetBreaks                 = false;
      
      // handler instances
      
      private static FileHandler                  fileHandler                   = new FileHandler();                 // handles all file operations
      private static StatusMessageHandler         statusMessageHandler          = new StatusMessageHandler();        // handles all status messages
      private static LuvBreakPointHandler         luvBreakPointHandler          = new LuvBreakPointHandler();        // handles all break points
      private static ExecutionViaLuvViewerHandler executionViaLuvViewerHandler  = new ExecutionViaLuvViewerHandler();// handles when user executes plan via Luv Viewer itself
      private static ViewHandler                  viewHandler                   = new ViewHandler();                 // handles all file operations
      private static MenuHandler                  menuHandler                   = new MenuHandler();                 // handles all menu operations
      private static LibraryHandler               libraryHandler                = new LibraryHandler();              // handles all library operations
      
      private DebugWindow luvViewerDebugWindow;         
      private ShellDebugWindow cmdPrmptDebugWindow;    
      
      Server s;
      
      // current working instance of luv
      
      private static Luv theLuv;         
      
      // the current model being displayed 

      private final Model model = new Model("dummy")
         {
               {
                  setProperty(MODEL_NAME, "root");
               }

               @Override public void clear()
               {
                  String name = getProperty(MODEL_NAME);
                  super.clear();
                  setProperty(MODEL_NAME, name);
               }
         };

      // persistant properties for luv viewer 

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

      // entry point for this program

      public static void main(String[] args)
      {         
        // LuvSplashScreen.enableSplashScreen(true);   
         
         runApp();
      }
      
      public static void runApp()
      {
         // if we're on a mac, us mac style menus
         
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
      
            // constructor for luv viewer
      
      public Luv()
      {
         // record instance of self

         theLuv = this;

         // set tooltip display time

         ToolTipManager.sharedInstance().setDismissDelay(properties.getInteger(PROP_TOOLTIP_DISMISS));

         // app exits when frame is closed

         setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

         // things to do when app is closed

         Runtime.getRuntime().addShutdownHook(new Thread()
            {
            @Override
                  public void run()
                  {
                     properties.set(PROP_WIN_LOC, getLocation());
                     properties.set(PROP_WIN_SIZE, getSize());
                     properties.set(PROP_DBWIN_LOC, luvViewerDebugWindow.getLocation());
                     properties.set(PROP_DBWIN_SIZE, luvViewerDebugWindow.getSize());
                     properties.set(PROP_DBWIN_VISIBLE, luvViewerDebugWindow.isVisible());
                     luvViewerDebugWindow.setVisible(false);
                     //cmdPrmptDebugWindow.setVisible(false);
                     viewHandler.setViewProperties(properties);
                  }
            });
            
         // construct the frame
         
         constructFrame(getContentPane());    
         
         // luv will be in this state only when you first start it (minimal options available)
         
         setLuvViewerState(START_STATE);

         // start the server listening for events

         s = new Server(properties.getInteger(PROP_NET_SERVER_PORT))
            {
                  @Override public void handleMessage(final String message) 
                  {                
                      
                     // parse the message
                     
                     boolean isPlan = fileHandler.parseXml(new ByteArrayInputStream(message.getBytes()), model);

                     // if this is a plan (or possibly a library)

                     if (isPlan)
                     {
                        if(!executedViaLuvViewer)
                        {
                            setLuvViewerState(CMD_PROMPT_EXECUTION_STATE);
                            //execAction.putValue(NAME, STOP_EXECUTION);
                        }
                        
                        viewHandler.resetView();
                        
                        if(TreeTableView.getCurrent().isNodeInfoWindowOpen())
                            refreshPopUpNodeWindow();
                        
                        // Determine if the Luv Viewer should pause before executing. 
                                                  
                        if (executedViaCommandPrompt && model.getProperty(VIEWER_BLOCKS).equals("true"))
                        {
                            setLuvViewerState(PAUSED_STATE);                             
                            doesViewerBlock();
                            menuHandler.getMenu(FILE_MENU).getItem(RELOAD_MENU_ITEM).setEnabled(false);
                            menuHandler.getMenu(RUN_MENU).getItem(EXECUTE_MENU_ITEM).setEnabled(false);
                        }                           
                     }                    
                  }
                  
                  @Override public boolean doesViewerBlock()
                  {
                     String blocksStr = model.getProperty(VIEWER_BLOCKS);
                     boolean blocks = blocksStr != null 
                        ? Boolean.valueOf(blocksStr)
                        : false;
                     
                     if (blocks && planPaused && !planStep) 
                     {
                        statusMessageHandler.showStatus((luvBreakPointHandler.getBreakPoint() == null
                                    ? "Plan execution paused."
                                    : luvBreakPointHandler.getBreakPoint().getReason()) +
                           "  Hit " + 
                           pauseAction.getAcceleratorDescription() +
                           " to resume, or " + 
                           stepAction.getAcceleratorDescription() +
                           " to step.",
                           Color.RED);

                        luvBreakPointHandler.clearBreakPoint();
                        
                        menuHandler.getMenu(RUN_MENU).getItem(EXECUTE_MENU_ITEM).setEnabled(false);
                        
                        while (planPaused && !planStep)
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

                     if (stepToStart >= 2)
                         planStep = false;
                     else
                         stepToStart++;

                     return blocks;
                  }
            };
    
            // clear splash screen
            
          //  LuvSplashScreen.enableSplashScreen(false);
      }
      
      public ViewHandler            getViewHandler()            { return viewHandler; }             // get current view handler
      
      public LibraryHandler         getLibraryHandler()         { return libraryHandler; }          // get current library handler
      
      public MenuHandler            getMenuHandler()            { return menuHandler; }             // get current menu handler

      public FileHandler            getFileHandler()            { return fileHandler; }             // get current file handler
            
      public StatusMessageHandler   getStatusMessageHandler()   { return statusMessageHandler; }    // get current status message handler
      
      public LuvBreakPointHandler   getLuvBreakPointHandler()   { return luvBreakPointHandler; }    // get current breakpoint handler
      
      public Model                  getModel()                  { return model; }                   // get current model being displayed
      
      public Properties             getProperties()             { return properties; }              // get persistant properties for luv viewer
      
      public static Luv             getLuv()                    { return theLuv; }                  // get current active instance of luv viewer
            
      // assign value to local luv boolean variables of current luv instance
      
      public void setBoolean(int boolVariable, boolean value)
      {
          switch (boolVariable)
          {
              case ALLOW_BREAKS:
                  allowBreaks = value;   
                  break;
              case EXEC_VIA_LUV:
                  executedViaLuvViewer = value;
                  break;
              case EXEC_VIA_CMD_PRMPT:
                  executedViaCommandPrompt = value;
                  break;
              case IS_EXECUTING:
                  isExecuting = value;  
                  break;
              case AT_START_SCREEN:
                  atStartScreen = value;
                  break;
              case DONT_LOAD_SCRIPT_AGAIN:
                  dontLoadScriptAgain = value;
                  break;
              case STOP_SRCH_LIBS:
                  stopSearchForMissingLibs = value; 
                  break;
              case OPEN_PLN_VIA_LUV:
                  openedPlanViaLuvViewer = value;
                  break;
              case PLAN_PAUSED:
                  planPaused = value;   
                  break;
              case PLAN_STEP:
                  planStep = value;
                  break;
              default:
                 ; //error
          }
      }
      
      // get the value of local luv boolean variables of current luv instance
      
      public boolean getBoolean(int boolVariable)
      {
          switch (boolVariable)
          {
              case ALLOW_BREAKS:
                  return allowBreaks;                 
              case EXEC_VIA_LUV:
                  return executedViaLuvViewer;
              case EXEC_VIA_CMD_PRMPT:
                  return executedViaCommandPrompt;
              case IS_EXECUTING:
                  return isExecuting;   
              case AT_START_SCREEN:
                  return atStartScreen;
              case DONT_LOAD_SCRIPT_AGAIN:
                  return dontLoadScriptAgain;
              case STOP_SRCH_LIBS:
                  return stopSearchForMissingLibs; 
              case OPEN_PLN_VIA_LUV:
                  return openedPlanViaLuvViewer;
              case PLAN_PAUSED:
                  return planPaused;    
              case PLAN_STEP:
                  return planStep;
              case SHOW_CONDITIONS:
                  return showConditions;
              case STOPPED_EXECUTION:
                  return stopExecution;
              default:
                  return false; //error 
          }
      }

      public void startState()
      {
          // reset all luv viewer variables
          
          stepToStart = 0;

          allowBreaks = false;
          executedViaLuvViewer = false;
          isExecuting = false;   
          atStartScreen = true;
          dontLoadScriptAgain = false;
          stopSearchForMissingLibs = false;
          openedPlanViaLuvViewer = false;
          executedViaCommandPrompt = false;
          planPaused = false;
          //planStep = false;
          
          model.clear();  
          
          fileHandler.clearCurrentFile(PLAN); 
          fileHandler.clearCurrentFile(SCRIPT); 
          fileHandler.clearCurrentFile(DEBUG);

          viewHandler.clearCurrentView();
          statusMessageHandler.clearStatusMessageQ();
          libraryHandler.resetLibraryInfo();
          luvBreakPointHandler.clearBreakPoint();
          luvBreakPointHandler.clearBreakPointMap();
          luvBreakPointHandler.clearUnfoundBreakPoints();
          
          // reset all menu items
          
          menuHandler.getMenu(FILE_MENU).getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(FILE_MENU).getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(FILE_MENU).getItem(EXIT_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(FILE_MENU).setEnabled(true);
          
          menuHandler.getMenu(WINDOW_MENU).getItem(SHOW_CONDITIONS_MENU_ITEM).setEnabled(false);
          menuHandler.getMenu(WINDOW_MENU).getItem(SHOW_LUV_DEBUG_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(WINDOW_MENU).setEnabled(true);
      }
      
      public void readyState()
      {
          // set only certain luv viewer variables
          
          stepToStart = 0;
          
          //executedViaLuvViewer = false;
          //executedViaCommandPrompt = false;
          planPaused = false;
          planStep = false;
          atStartScreen = false;
          stopSearchForMissingLibs = false;           
  
          // set certain menu items

          menuHandler.getMenu(FILE_MENU).getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(FILE_MENU).getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(FILE_MENU).getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);          
          menuHandler.getMenu(FILE_MENU).getItem(RELOAD_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(FILE_MENU).getItem(EXIT_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(FILE_MENU).setEnabled(true);
 
          if (allowBreaks)
          {
              if (isExecuting)
                  menuHandler.getMenu(RUN_MENU).getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(true);
              else
                  menuHandler.getMenu(RUN_MENU).getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
              menuHandler.getMenu(RUN_MENU).getItem(STEP_MENU_ITEM).setEnabled(true);
          }
          else
          {
              menuHandler.getMenu(RUN_MENU).getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
              menuHandler.getMenu(RUN_MENU).getItem(STEP_MENU_ITEM).setEnabled(false);
          }
          
          if (isExecuting)
              menuHandler.getMenu(RUN_MENU).getItem(BREAK_MENU_ITEM).setEnabled(false);
          else
              menuHandler.getMenu(RUN_MENU).getItem(BREAK_MENU_ITEM).setEnabled(true);
              
          menuHandler.getMenu(RUN_MENU).getItem(EXECUTE_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(RUN_MENU).setEnabled(true);

          if (menuHandler.getMenu(VIEW_MENU).getMenuComponentCount() > 0)
          {
              menuHandler.getMenu(VIEW_MENU).getItem(EXPAND_MENU_ITEM).setEnabled(true);
              menuHandler.getMenu(VIEW_MENU).getItem(COLLAPSE_MENU_ITEM).setEnabled(true);
              menuHandler.getMenu(VIEW_MENU).getItem(TOGGLE_TEXT_TYPES_MENU_ITEM).setEnabled(true);
              menuHandler.getMenu(VIEW_MENU).getItem(TOGGLE_LISP_NODES_MENU_ITEM).setEnabled(true); 
              menuHandler.getMenu(VIEW_MENU).setEnabled(true);
          }
          else
              menuHandler.getMenu(VIEW_MENU).setEnabled(false);

          menuHandler.getMenu(WINDOW_MENU).getItem(SHOW_CONDITIONS_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(WINDOW_MENU).getItem(SHOW_LUV_DEBUG_MENU_ITEM).setEnabled(true);
          menuHandler.getMenu(WINDOW_MENU).setEnabled(true);
      }
      
      public void executionState()
      {
          isExecuting = true;
          stopExecution = false;
          
          readyState();
          
          menuHandler.getMenu(RUN_MENU).getItem(BREAK_MENU_ITEM).setEnabled(false);
          
          if (executedViaCommandPrompt)
          {
              menuHandler.getMenu(FILE_MENU).getItem(RELOAD_MENU_ITEM).setEnabled(false);
              menuHandler.getMenu(RUN_MENU).getItem(EXECUTE_MENU_ITEM).setEnabled(false);
          }             
          
          statusMessageHandler.showStatus("Executing...", Color.GREEN.darker(), 1000);          
      }
      
      public void pausedState()
      {
          isExecuting = true;
          
          readyState();
          
          if (!allowBreaks)
              enabledBreakingState();
          
          planPaused = true;
          menuHandler.getMenu(RUN_MENU).getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(true);
          
          if (executedViaCommandPrompt)
          {
              menuHandler.getMenu(FILE_MENU).getItem(RELOAD_MENU_ITEM).setEnabled(false);
              menuHandler.getMenu(RUN_MENU).getItem(EXECUTE_MENU_ITEM).setEnabled(false);
          }
      }
      
      public void disabledBreakingState()
      {
          readyState();
          
          allowBreaks = false;          
          
          menuHandler.getMenu(RUN_MENU).getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
          menuHandler.getMenu(RUN_MENU).getItem(STEP_MENU_ITEM).setEnabled(false);
          allowBreaksAction.putValue(NAME, ENABLE_BREAKS);
          
          statusMessageHandler.showStatus(DISABLE_BREAKS, Color.RED, 1000);
      }
      
      public void enabledBreakingState()
      {
          readyState();
          
          allowBreaks = true;
          
          menuHandler.getMenu(RUN_MENU).getItem(STEP_MENU_ITEM).setEnabled(true);
          allowBreaksAction.putValue(NAME, DISABLE_BREAKS);
          
          statusMessageHandler.showStatus(ENABLE_BREAKS, Color.GREEN.darker(), 1000);
      }
      
      public void endState()
      {
          isExecuting = false;
          execAction.putValue(NAME, EXECUTE_PLAN);
          readyState();
      }
      
      public void setLuvViewerState(int state)
      {
          menuHandler.disableAllMenus();
          
          switch (state)
          {
              case NULL_STATE:
                  break;
              case START_STATE:
                  startState();
                  break;
              case READY_STATE:
                  readyState();
                  break;
              case EXECUTION_STATE:
                  executionState();
                  break;
              case LUV_VIEWER_EXECUTION_STATE:
                  executionState();
                  executedViaLuvViewer = true;
                  openedPlanViaLuvViewer = true;
                  break;
              case CMD_PROMPT_EXECUTION_STATE:
                  executionState();
                  executedViaCommandPrompt = true; 
                  openedPlanViaLuvViewer = false;      
                  menuHandler.getMenu(FILE_MENU).getItem(RELOAD_MENU_ITEM).setEnabled(false);
                  menuHandler.getMenu(RUN_MENU).getItem(EXECUTE_MENU_ITEM).setEnabled(false);
                  break;
              case PAUSED_STATE:
                  pausedState();
                  break;
              case DISABLED_BREAKING_STATE:
                  disabledBreakingState();
                  break;
              case ENABLED_BREAKING_STATE:
                  enabledBreakingState();
                  break;
              case END_STATE:  
                  endState();
                  break;
          }       
      }
      
      public void refreshPopUpNodeWindow()
      {
         Model node = model;

         for (int i = TreeTableView.getCurrent().getPathToInfoWindowNode().size() - 2; i >= 0; i--)
         {
             String name = TreeTableView.getCurrent().getPathToInfoWindowNode().get(i);
             if (node != null)
                 node = node.findChildByName(name);
             else 
                 break;
         }

         if (node != null)
             TreeTableView.getCurrent().resetNodeInfoWindow(node, node.getProperty(MODEL_NAME));
         else
             TreeTableView.getCurrent().closeNodeInfoWindow();
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
         menuHandler.createMenuBar(menuBar);
         
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
         
         luvViewerDebugWindow.addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(WindowEvent winEvt) {
                // Perhaps ask user if they want to save any unsaved files first.
                luvDebugWindowAction.actionPerformed(null);   
            }
         });
         
         // create the command prompt debug window
         
         //cmdPrmptDebugWindow = new cmdPrmptDebugWindow(this);
         //cmdPrmptDebugWindow.setTitle("Command Prompt Debug Window");
         
         // set size and location off frame

         //cmdPrmptDebugWindow.setLocation(properties.getPoint(PROP_DBWIN_LOC));
         //cmdPrmptDebugWindow.setPreferredSize(properties.getDimension(PROP_DBWIN_SIZE));
         //cmdPrmptDebugWindow.pack();
         
         // when this frame get's focus, get it's menu bar back from the debug window

         addWindowFocusListener(new WindowAdapter()
            {
            @Override
                  public void windowGainedFocus(WindowEvent e)
                  {
                     JMenuBar mb = luvViewerDebugWindow.getJMenuBar();
                     if (mb != null) setJMenuBar(mb);
                  }
            });
            
         setTitle();
         
         // make the frame visible
         
         pack();
         setVisible(true);
      }
      
      // set title of the luv viewer

      public void setTitle()
      {          
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
         return properties.getProperty(key);
      }

      // exit this program.

      public void exit() { System.exit(0); }
      
      
      
      
      
      /***************** List of Actions ********************/

      /** Action to open and view a plan. */

      LuvAction openAction = new LuvAction(
         OPEN_PLAN, "Open a plan for viewing.", VK_O, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  fileHandler.choosePlan();
                  openedPlanViaLuvViewer = true;
                  isExecuting = false;
                  executedViaCommandPrompt = false;
               }
         };
      
      /** Action to load a script for Execution. */
         
      LuvAction openScriptAction = new LuvAction(
         OPEN_SCRIPT, "Open a script for execution.", VK_O, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  fileHandler.chooseScript();
               }
         };

      /** Action to reload a plan. */

      LuvAction reloadAction = new LuvAction(
         RELOAD_PLAN, "Reload current plan file.", VK_R, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                   if (executedViaLuvViewer || openedPlanViaLuvViewer)
                   {
                      setLuvViewerState(READY_STATE);
                      fileHandler.loadRecentPlan(1);
                      if(TreeTableView.getCurrent().isNodeInfoWindowOpen())
                            refreshPopUpNodeWindow();
                   }
                   else
                   {
                        JOptionPane.showMessageDialog(
                           theLuv,
                           "Error reloading plan. The Luv Viewer cannot store a plan that was loaded by command prompt.\n" +
                           "You must load the plan via the Luv Viewer in order to reload.",
                           "Reload Error",
                           JOptionPane.ERROR_MESSAGE);
                        statusMessageHandler.showStatus("Unable to reload plan that was loaded by command prompt.", 1000);
                   }
               }
         };

      /** Action to show the debugging window. */

      LuvAction luvDebugWindowAction = new LuvAction(
         SHOW_LUV_VIEWER_DEBUG_WINDOW, "Show window with luv viewer debug text.", VK_L, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                   luvViewerDebugWindow.setVisible(!luvViewerDebugWindow.isVisible());
                   
                   if (luvViewerDebugWindow.isVisible())
                       luvDebugWindowAction.putValue(NAME, HIDE_LUV_VIEWER_DEBUG_WINDOW);
                   else
                       luvDebugWindowAction.putValue(NAME, SHOW_LUV_VIEWER_DEBUG_WINDOW);
                   
                   
               }
         };
         
      LuvAction commandPromptDebugWindowAction = new LuvAction(
         SHOW_CMD_PROMPT_DEBUG_WINDOW, "Show window with command prompt debug text.", VK_P, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                   cmdPrmptDebugWindow.setVisible(!cmdPrmptDebugWindow.isVisible());
                   
                   if (cmdPrmptDebugWindow.isVisible())
                       commandPromptDebugWindowAction.putValue(NAME, HIDE_CMD_PROMPT_DEBUG_WINDOW);
                   else
                       commandPromptDebugWindowAction.putValue(NAME, SHOW_CMD_PROMPT_DEBUG_WINDOW);
                   
               }
         };
         
      /** Action to allow breakpoints. */
         
      LuvAction allowBreaksAction = new LuvAction(
         ENABLE_BREAKS, "Select this to allow breakpoints.", VK_F2)
	 {
             public void actionPerformed(ActionEvent e)
             {
                 if (!isExecuting)
                 {
                     allowBreaks = !allowBreaks;

                     if (allowBreaks)
                     {
                         setLuvViewerState(ENABLED_BREAKING_STATE);
                     }
                     else
                     {
                         setLuvViewerState(DISABLED_BREAKING_STATE);
                     }
                 }
             }
	 };
         
      /** Action to execute a plexil plan. */

      LuvAction execAction = new LuvAction(
         EXECUTE_PLAN, 
         "Execute plan currently loaded.",
         VK_F1)
         {
             public void actionPerformed(ActionEvent e)
             {
                try {
                    
                    if (!isExecuting && !executedViaCommandPrompt)
                    {                        
                        setLuvViewerState(LUV_VIEWER_EXECUTION_STATE);
                        executionViaLuvViewerHandler.runExec();
                        execAction.putValue(NAME, STOP_EXECUTION);
                    }
                    else if (isExecuting && !executedViaCommandPrompt)
                    {
                        statusMessageHandler.showStatus("Execution stopped.", Color.RED, 1000);
                        stopExecution = true;
                        s.setHaltExecution(true);           
                        execAction.putValue(NAME, EXECUTE_PLAN);
                        isExecuting = false;                                       
                        fileHandler.loadPlan(fileHandler.getCurrentFile(PLAN));                  
                        reloadAction.actionPerformed(e);
                        stopExecution = false;
                    }
                    else if (isExecuting && executedViaCommandPrompt)
                    {
                      /*  statusMessageHandler.showStatus("Execution stopped.", Color.RED, 1000);
                        stopExecution = true;
                        s.setHaltExecution(true);
                        execAction.putValue(NAME, EXECUTE_PLAN);
                        isExecuting = false;
                        
                        menuHandler.getMenu(FILE_MENU).getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
                        menuHandler.getMenu(FILE_MENU).getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
                        menuHandler.getMenu(FILE_MENU).getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);          
                        menuHandler.getMenu(FILE_MENU).getItem(RELOAD_MENU_ITEM).setEnabled(false);
                        menuHandler.getMenu(FILE_MENU).getItem(EXIT_MENU_ITEM).setEnabled(true);
                        menuHandler.getMenu(FILE_MENU).setEnabled(true);

                        menuHandler.getMenu(RUN_MENU).getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
                        menuHandler.getMenu(RUN_MENU).getItem(STEP_MENU_ITEM).setEnabled(false);
                        menuHandler.getMenu(RUN_MENU).getItem(BREAK_MENU_ITEM).setEnabled(false);
                        menuHandler.getMenu(RUN_MENU).getItem(EXECUTE_MENU_ITEM).setEnabled(false);
                        menuHandler.getMenu(RUN_MENU).setEnabled(false);*/
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
                   if (allowBreaks && isExecuting)
                   {
                       planPaused = !planPaused;

                       statusMessageHandler.showStatus((planPaused ? PAUSE : RESUME) + " requested.", Color.BLACK, 1000);

                       if (planPaused)
                           setLuvViewerState(PAUSED_STATE);
                       else
                           setLuvViewerState(EXECUTION_STATE);
                         
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
                   if (isExecuting)
                   {
                      if (!planPaused)
                      {
                         planPaused = true;
                         statusMessageHandler.showStatus("Step requested.", Color.BLACK, 1000);
                      }
                      else
                      {
                         planStep = true;
                         statusMessageHandler.showStatus("Step plan.", Color.BLACK, 1000);
                      }
                   }
                   else
                   {
                       stepToStart = 0;
                       execAction.actionPerformed(e);
                       planStep = true;
                       stepAction.actionPerformed(e);
                       stepToStart++;
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
               RegexModelFilter filter = new RegexModelFilter(
                     properties.getBoolean(PROP_VIEW_HIDE_PLEXILLISP),
                     "^plexilisp_.*", 
                     MODEL_NAME);

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
                           theLuv,
                     "Are you sure you want to exit?",
                     "Exit Luv Viewer",
                     JOptionPane.YES_NO_CANCEL_OPTION,
                     JOptionPane.WARNING_MESSAGE,
                     null,
                     options,
                     options[0]);
                   
                   if (exitLuv == 0)
                   {                 
                       exit();
                   }
               }
         };
         
      LuvAction conditionsAction = new LuvAction(
         "Enable Conditions Window", "Allow conditions window to open.", VK_C, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                   showConditions = !showConditions;
                   
                   if (showConditions)
                       conditionsAction.putValue(NAME, "Disable Conditions Window");
                   else
                   {
                       conditionsAction.putValue(NAME, "Enable Conditions Window");  
                       ConditionsWindow.closeConditonsWindow();
                   }
               }
         };
}
