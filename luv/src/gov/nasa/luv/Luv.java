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

import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JFrame;
import javax.swing.JMenuBar;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.border.EmptyBorder;

import java.awt.Container;
import java.awt.Color;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;

import java.io.IOException;
import java.io.ByteArrayInputStream;

import java.io.File;
import java.util.HashMap;
import javax.swing.JMenu;
import javax.swing.JSeparator;
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
      private static boolean stopExecution               = false;
      private static boolean cancelPlanLoading           = false;
      
      // handler instances
      
      private static FileHandler                  fileHandler                   = new FileHandler();                 // handles all file operations
      private static StatusMessageHandler         statusMessageHandler          = new StatusMessageHandler();        // handles all status messages
      private static LuvBreakPointHandler         luvBreakPointHandler          = new LuvBreakPointHandler();        // handles all break points
      private static ExecutionViaLuvViewerHandler executionViaLuvViewerHandler  = new ExecutionViaLuvViewerHandler();// handles when user executes plan via Luv Viewer itself
      private static ViewHandler                  viewHandler                   = new ViewHandler();                 // handles all file operations
      private static ConditionHandler             conditionHandler;             // saves node's condition information
      
      private JMenu fileMenu                = new JMenu("File");  
      private JMenu recentRunMenu           = new JMenu("Recent Runs");
      private JMenu runMenu                 = new JMenu("Run");   
      private JMenu viewMenu                = new JMenu("View");
      private JMenu windowMenu              = new JMenu("Windows");
      
      private HashMap<String, String> libraryNames = new HashMap<String, String>();
      
      private DebugWindow luvViewerDebugWindow;   
      
      Server s;
      
      // current working instance of luv
      
      private static Luv theLuv;         
      
      // the current model being displayed 

      private Model model = new Model("dummy")
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

         // app exits when frame is closed

         setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            
         // construct the frame
         
         constructFrame(getContentPane());    
         
         // create title
         
         setTitle();
         
         // luv will be in this state only when you first start it (minimal options available)
         
         setLuvViewerState(START_STATE);

         // start the server listening for events

         startServer();
            
      }
      
      public ViewHandler            getViewHandler()            { return viewHandler; }             // get current view handler

      public FileHandler            getFileHandler()            { return fileHandler; }             // get current file handler
      
      public LuvBreakPointHandler   getLuvBreakPointHandler()   { return luvBreakPointHandler; }    // get current breakpoint handler
      
      public ConditionHandler       getConditionHandler()       { return conditionHandler; }        // get current condition handler
      
      public Model                  getModel()                  { return model; }                   // get current model being displayed
      
      public Properties             getProperties()             { return properties; }              // get persistant properties for luv viewer
      
      public static Luv             getLuv()                    { return theLuv; }                  // get current active instance of luv viewer
      
      public void startServer()
      {
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
                            conditionHandler = new ConditionHandler((Model) model.clone());
                        }
                        else
                            addRunToRecentRunList();  // save plan and script to recent run list

                        viewHandler.resetView();
                        
                        if(TreeTableView.getCurrent().isNodeInfoWindowOpen())
                            refreshPopUpNodeWindow();
                        
                        // Determine if the Luv Viewer should pause before executing. 
                                                  
                        if (executedViaCommandPrompt)
                        {
                            if (model.getProperty(VIEWER_BLOCKS).equals("true"))
                            {
                                setLuvViewerState(PAUSED_STATE); 
                                runMenu.setEnabled(true);
                                doesViewerBlock();
                            }
                            else
                            {
                                setLuvViewerState(DISABLED_BREAKING_STATE);
                            }
                        } 
                        
                        executedViaLuvViewer = false;
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
      }
            
      // assign value to local luv boolean variables of current luv instance
      
      public void setBoolean(int boolVariable, boolean value)
      {
          switch (boolVariable)
          {
              case ALLOW_BREAKS: allowBreaks = value; break;
              case EXEC_VIA_CMD_PRMPT: executedViaCommandPrompt = value; break;
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
              case CANCEL_PLAN_LOADING:
                  cancelPlanLoading = value;
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
              case STOPPED_EXECUTION:
                  return stopExecution;
              case CANCEL_PLAN_LOADING:
                  return cancelPlanLoading;
              default:
                  return false; //error 
          }
      }

      public void startState()
      {
          // reset all luv viewer variables
          
          disableAllMenus();
          
          stepToStart = 0;

          allowBreaks = false;
          isExecuting = false;   
          atStartScreen = true;
          dontLoadScriptAgain = false;
          stopSearchForMissingLibs = false;     
          executedViaLuvViewer = false;
          planPaused = false;
          
          model.clear();  
          conditionHandler = new ConditionHandler((Model) model.clone());
          
          fileHandler.clearCurrentFile(PLAN); 
          fileHandler.clearCurrentFile(SCRIPT); 
          fileHandler.clearCurrentFile(DEBUG);

          viewHandler.clearCurrentView();
          statusMessageHandler.clearStatusMessageQ();
          luvBreakPointHandler.clearBreakPoint();
          luvBreakPointHandler.clearBreakPointMap();
          luvBreakPointHandler.clearUnfoundBreakPoints();
          
          // reset all menu items
          
          fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
          fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
          fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);
          fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(false);
          fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(true);
          fileMenu.setEnabled(true);
          
          runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
          runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
          allowBreaksAction.putValue(NAME, ENABLE_BREAKS);
          
          windowMenu.getItem(SHOW_LUV_DEBUG_MENU_ITEM).setEnabled(true);
          windowMenu.setEnabled(true);
          
          if (stopExecution && !executedViaCommandPrompt)
          {
              fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
          }
      }
      
      public void readyState()
      {
          // set only certain luv viewer variables
          
          stepToStart = 0;
          planPaused = false;
          planStep = false;
          atStartScreen = false;
          stopSearchForMissingLibs = false; 
          isExecuting = false;
          stopExecution = false;        
          executedViaLuvViewer = false;
  
          // set certain menu items
          
          execAction.putValue(NAME, EXECUTE_PLAN);

          fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
          fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
          fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);          
          fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
          fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(true);
          fileMenu.setEnabled(true);
 
          if (allowBreaks && isExecuting)
          {
              runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(true);
              runMenu.getItem(STEP_MENU_ITEM).setEnabled(true);
          }
          else
          {
              runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
              runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
          }             
          
          if (isExecuting)
              runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
          else
              runMenu.getItem(BREAK_MENU_ITEM).setEnabled(true);

          if (executedViaCommandPrompt)
          {       
              fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(false);
              runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
              runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
              runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);
          }
          else
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
      }
      
      public void finishedExecutionState()
      {
          // set only certain luv viewer variables
          
          stepToStart = 0;
          planPaused = false;
          planStep = false;
          atStartScreen = false;
          stopSearchForMissingLibs = false; 
          isExecuting = false;
          stopExecution = false;        
          executedViaLuvViewer = false;
          cancelPlanLoading = false;
  
          // set certain menu items
          
          execAction.putValue(NAME, EXECUTE_PLAN);

          fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
          fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
          fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);          
          fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
          fileMenu.getItem(EXIT_MENU_ITEM).setEnabled(true);
          fileMenu.setEnabled(true);
 

          runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
          runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
          runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
          runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);         
          runMenu.setEnabled(false);

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
      }
      
      public void preExecutionState()
      {
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
          stopExecution = false;
          
          showStatus("Executing...", Color.GREEN.darker());
          
          runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
          
          if (allowBreaks)
          {
              runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(true);
              runMenu.getItem(STEP_MENU_ITEM).setEnabled(true);
          }
          
          if (executedViaCommandPrompt)
          {
              fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(false);
              runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);
          } 
      }
      
      public void pausedState()
      {
          allowBreaks = true;
          isExecuting = true;
          planPaused = true;
          planStep = false;
          
          runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(true);
          runMenu.getItem(STEP_MENU_ITEM).setEnabled(true);
      }
      
      public void disabledBreakingState()
      {
          allowBreaks = false;
          
          luvBreakPointHandler.removeAllBreakpointsAction.actionPerformed(null);
          
          runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
          runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
          allowBreaksAction.putValue(NAME, ENABLE_BREAKS);
      }
      
      public void enabledBreakingState()
      {
          allowBreaks = true;
          
          allowBreaksAction.putValue(NAME, DISABLE_BREAKS);
      }
      
      public void setLuvViewerState(int state)
      {
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
                  execAction.putValue(NAME, STOP_EXECUTION);
                  fileMenu.getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
                  fileMenu.getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
                  fileMenu.getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);
                  fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(true);
                  runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
                  runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(true);
                  executedViaLuvViewer = true;
                  executedViaCommandPrompt = false; 
                  openedPlanViaLuvViewer = true;
                  break;
              case CMD_PROMPT_EXECUTION_STATE:
                  executionState();
                  executedViaCommandPrompt = true; 
                  executedViaLuvViewer = false;
                  openedPlanViaLuvViewer = false;      
                  fileMenu.getItem(RELOAD_MENU_ITEM).setEnabled(false);
                  runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);
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
              case FINISHED_EXECUTION_STATE:
                  finishedExecutionState();
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
         
         luvViewerDebugWindow.addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(WindowEvent winEvt) {
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
         fileMenu.add(Luv.getLuv().openAction);
         fileMenu.add(Luv.getLuv().openScriptAction);
         updateRecentMenu();
         fileMenu.add(recentRunMenu);
         fileMenu.add(Luv.getLuv().reloadAction);
         fileMenu.add(new JSeparator());
         fileMenu.add(Luv.getLuv().exitAction);

         // create and update exec menu
         
         menuBar.add(runMenu);
         runMenu.add(Luv.getLuv().pauseAction);
         runMenu.add(Luv.getLuv().stepAction);
         runMenu.add(Luv.getLuv().allowBreaksAction);
         runMenu.add(Luv.getLuv().execAction);

         // add view menu

         menuBar.add(viewMenu);

         // show window menu
 
         menuBar.add(windowMenu);
         windowMenu.add(Luv.getLuv().luvDebugWindowAction);
      }
      
      public JMenu getMenu(int menu)
      {
          switch (menu)
          {
              case FILE_MENU:
                  return fileMenu;
              case RECENT_FILE_MENU:
                  return recentRunMenu;
              case RUN_MENU:
                  return runMenu;
              case VIEW_MENU:
                  return viewMenu;
              case WINDOW_MENU:
                  return windowMenu;
              default:
                  return null; //error
          }
      }
      
      public void removeAllFromMenu(int menu)
      {
          switch (menu)
          {
              case FILE_MENU:
                  fileMenu.removeAll();
                  break;
              case RECENT_FILE_MENU:
                  recentRunMenu.removeAll();
                  break;
              case RUN_MENU:
                  runMenu.removeAll();
                  break;
              case VIEW_MENU:
                  viewMenu.removeAll();
                  break;
              case WINDOW_MENU:
                  windowMenu.removeAll();
                  break;
              default:
                  ; //error
          }          
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

          if (viewMenu.getMenuComponentCount() > 0)
          {
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

      public String getRecentMenuDescription(int recentIndex)
      {
         File recentPlan = new File(Luv.getLuv().getFileHandler().getRecentPlanName(recentIndex));
         StringBuffer description = new StringBuffer("Load " + recentPlan.getName());
         
         description.append(".");
         return description.toString();
      }
      
      // Add a file to the recently opened file list. 

      public void addRunToRecentRunList()
      {
         // put newest file at the top of the list
          
         File plan = fileHandler.getCurrentFile(PLAN);
         File script = fileHandler.getCurrentFile(SCRIPT);

         String currentPlan = plan.getAbsolutePath();
         String currentScript = script.getAbsolutePath();

         String planName = currentPlan;
         String scriptName = currentScript;
         
         int count = properties.getInteger(PROP_FILE_RECENT_COUNT);
         
         for (int i = 1; i <= count && planName != null; ++i)
         {
            if (planName != null)
            {

               planName = (String)properties.setProperty(PROP_FILE_RECENT_PLAN_BASE + i, planName);
               scriptName = (String)properties.setProperty(PROP_FILE_RECENT_SCRIPT_BASE + i, scriptName);
               // if this run already existed in the list, we can stop
               // it already appears at the top

               if (planName != null && planName.equals(currentPlan))
                  break;
            }
         }

         // update the recent menu

         updateRecentMenu();
      }
      
      /** Update the recently loaded files menu. */

      public void updateRecentMenu()
      {
         recentRunMenu.removeAll();
         int count = properties.getInteger(PROP_FILE_RECENT_COUNT);
       
         if (fileHandler.getRecentPlanName(1) == null)
         {
             recentRunMenu.add("No recent runs");
         }
         else
         {
             for (int i = 0; i < count; ++i)
             {
                if (fileHandler.getRecentPlanName(i + 1) != null)
                   recentRunMenu.add(new LoadRecentAction(i + 1, '1' + i, META_MASK));
             }
         }

         // this menu is only enabled when there are items in it
         
         recentRunMenu.setEnabled(recentRunMenu.getMenuComponentCount() > 0);
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

      public void exit() { System.exit(0); }
      
      public String getCommandLine() throws IOException
      {
          String command = PROP_UE_EXEC;
          
          command += " -v";
          
          if (Luv.getLuv().getBoolean(ALLOW_BREAKS))
              command += " -b";                
          
          // double check that plan still exists
          
          if (new File(model.getPlanName()).exists())
          {
              command += " " +  model.getPlanName(); 
              
              if (fileHandler.getScript() != null)
              {
                  // double check that script still exists
                  
                  if (new File(fileHandler.getScript().getAbsolutePath()).exists())
                  {
                      command += " " +  fileHandler.getScript().getAbsolutePath();
                  }
                  else
                      command = "Error: " + fileHandler.getScript().getAbsolutePath() + " does not exist.";
              }
              else
                  command = "Error: script does not exist.";

              if (libraryNames.size() > 0)
              {
                  for (String libName : libraryNames.values())
                  {
                      // double check that library still exists
                      if (new File(libName).exists())
                      {
                          command += " -l ";
                          command += libName.toString();
                      }
                      else
                      {
                          command = "Error: " + libName + " does not exist.";
                          break;
                      }
                  }
              }
          }
          else
              command = "Error: " + model.getPlanName() + " does not exist.";

          return command;
      }   
      
      public void addLibraryName(String name, String path)
      {
          libraryNames.put(name, path);
      }
      
      public HashMap<String, String> getLibraryNames()
      {
          return libraryNames;
      }
      
      public void clearLibraryNames()
      {
          libraryNames.clear();
      }
      
      public void stopExecution() throws IOException
      {
          executionViaLuvViewerHandler.killUEProcess();
          pauseAction.actionPerformed(null);
          stopExecution = true;
          executedViaLuvViewer = true;
      }
      
      
      
      /***************** List of Actions ********************/

      /** Action to open and view a plan. */

      LuvAction openAction = new LuvAction(
         "Open Plan", "Open a plan for viewing.", VK_O, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  if (isExecuting)
                  {
                      try 
                      {
                          stopExecution();
                          JOptionPane.showMessageDialog(theLuv, "Stopping execution and opening a new plan", "Stopping Execution", JOptionPane.INFORMATION_MESSAGE);
                      }
                      catch (IOException ex) 
                      {
                          JOptionPane.showMessageDialog(theLuv, "Error opening plan. Please see Debug Window.", "Error", JOptionPane.ERROR_MESSAGE);
                          System.err.println("Error: " + ex.getMessage());
                      }
                  }
                  
                  clearLibraryNames();
                  int option = fileHandler.choosePlan();  
                  if (option != CANCEL_OPTION)
                  {
                      if (!cancelPlanLoading)
                      {
                          executedViaCommandPrompt = false;
                          setLuvViewerState(READY_STATE);                     
                          luvBreakPointHandler.clearBreakPoint();
                          luvBreakPointHandler.clearBreakPointMap();
                          luvBreakPointHandler.clearUnfoundBreakPoints();
                          conditionHandler = new ConditionHandler((Model) model.clone());
                      }
                      else
                          setLuvViewerState(START_STATE);
                  }
               }
         };
      
      /** Action to load a script for Execution. */
         
      LuvAction openScriptAction = new LuvAction(
         "Open Script", "Open a script for execution.", VK_S, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  if (isExecuting)
                  {
                      try 
                      {
                          stopExecution();
                          JOptionPane.showMessageDialog(theLuv, "Stopping execution and opening script", "Stopping Execution", JOptionPane.INFORMATION_MESSAGE);
                      }
                      catch (IOException ex) 
                      {                       
                          JOptionPane.showMessageDialog(theLuv, "Error loading script. Please see Debug Window.", "Error", JOptionPane.ERROR_MESSAGE);
                          System.err.println("Error: " + ex.getMessage());
                      }
                  }
                   
                  fileHandler.chooseScript();
               }
         };

      /** Action to reload a plan. */

      LuvAction reloadAction = new LuvAction(
         "Reload Plan", "Reload current plan file.", VK_R, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                   if (isExecuting)
                   {
                       try 
                       {
                           stopExecution();
                           JOptionPane.showMessageDialog(theLuv, "Stopping execution and reloading plan", "Stopping Execution", JOptionPane.INFORMATION_MESSAGE);
                       }
                       catch (IOException ex) 
                       {
                           JOptionPane.showMessageDialog(theLuv, "Error reloading plan. Please see Debug Window.", "Error", JOptionPane.ERROR_MESSAGE);
                           System.err.println("Error: " + ex.getMessage());
                       }
                   }
                   
                   if (openedPlanViaLuvViewer)
                   {
                      clearLibraryNames();
                      setLuvViewerState(READY_STATE);     
                      conditionHandler = new ConditionHandler((Model) model.clone());
                      if(TreeTableView.getCurrent().isNodeInfoWindowOpen())
                            refreshPopUpNodeWindow();

                        try 
                        {
                            if (fileHandler.getCurrentFile(PLAN) == null)
                                fileHandler.loadRecentPlan(1);
                            else
                                fileHandler.loadPlan(fileHandler.getCurrentFile(PLAN));
                        } 
                        catch (IOException ex) 
                        {
                           JOptionPane.showMessageDialog(theLuv, "Error reloading plan. Please see Debug Window.", "Error", JOptionPane.ERROR_MESSAGE);
                               System.err.println("Error: " + ex.getMessage());
                        }

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
         "Show Luv Viewer Debug Window", "Show window with luv viewer debug text.", VK_L, META_MASK)
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
         
      /** Action to allow breakpoints. */
         
      LuvAction allowBreaksAction = new LuvAction(
         ENABLE_BREAKS, "Select this to allow breakpoints. Right-click on the node you want to set breakpoint on", VK_F2)
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

                    if (!isExecuting)
                    {
                        executedViaLuvViewer = true;
                        preExecutionState();
                        String command = getCommandLine();
                        if (!command.contains("Error"))
                            executionViaLuvViewerHandler.runExec(command);
                        else
                        {
                            JOptionPane.showMessageDialog(theLuv, command, "Error", JOptionPane.ERROR_MESSAGE);
                            showStatus("Stopped execution", Color.lightGray, 1000);
                            setLuvViewerState(READY_STATE);
                        }
                    }
                    else
                    {   
                        stopExecution();
                    }
                    
                } catch (IOException ex) 
                {
                    JOptionPane.showMessageDialog(theLuv, "Error executing plan. Please see Debug Window.", "Error", JOptionPane.ERROR_MESSAGE);
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
                   if (isExecuting)
                   {
                       planPaused = !planPaused;

                       statusMessageHandler.showStatus((planPaused ? "Pause" : "Resume") + " requested.", Color.BLACK, 1000);

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
         "Exit", "Terminate this program.", VK_ESCAPE)
         {
               public void actionPerformed(ActionEvent e)
               {
                   Object[] options = 
                     {
                        "Yes",                       
                        "No",
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
}
