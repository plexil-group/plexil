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
import javax.swing.AbstractAction;
import javax.swing.KeyStroke;
import javax.swing.JMenuBar;
import javax.swing.JMenu;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JTextArea;
import javax.swing.JButton;
import javax.swing.JSeparator;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;
import javax.swing.JScrollPane;
import javax.swing.JOptionPane;
import javax.swing.ImageIcon;
import javax.swing.border.EmptyBorder;
import javax.swing.JPopupMenu;
import javax.swing.JDesktopPane;
import javax.swing.ToolTipManager;

import java.util.Set;
import java.util.Map;
import java.util.Vector;
import java.util.HashMap;
import java.util.LinkedList;

import java.net.Socket;

import java.awt.Toolkit;
import java.awt.Container;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowStateListener;
import java.awt.event.WindowAdapter;
import java.awt.event.ItemListener;
import java.awt.event.ItemEvent;
import java.awt.event.KeyListener;
import java.awt.event.KeyEvent;

import java.io.File;
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.IOException;
import javax.swing.JCheckBox;
import javax.swing.JButton;
import javax.swing.Action;

import java.io.FileInputStream;
import java.io.InputStream;
import java.io.StringReader;
import java.io.ByteArrayInputStream;

import org.xml.sax.*;
import org.xml.sax.helpers.XMLReaderFactory;

import static gov.nasa.luv.Constants.*;

import static java.lang.System.*;
import static java.awt.BorderLayout.*;
import static java.awt.event.InputEvent.CTRL_MASK;
import static java.awt.event.InputEvent.META_MASK;
import static java.awt.event.InputEvent.SHIFT_MASK;
import static java.awt.event.KeyEvent.*;
import static javax.swing.JFileChooser.*;

/**
 * The containing frame for the Lightweight UE Viewer.
 */

public class Luv extends JFrame
{
      /** the current model being displayed */

      final Model model = new Model("dummy")
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



      /** persistant properties for this application */

      Properties properties = new Properties(PROPERTIES_FILE_LOCATION)
         {
               {                  
                  define(PROP_FILE_AUTO_LOAD,    PROP_FILE_AUTO_LOAD_DEF);
                  define(PROP_FILE_RECENT_COUNT, PROP_FILE_RECENT_COUNT_DEF);
                  
                  define(PROP_FILE_RECENT_PLAN_DIR,
                         System.getenv(PROP_PLEXIL_HOME) != null
                         ? System.getenv(PROP_RECENT_FILES) + "/plans"
                         : System.getProperty(PROP_RECENT_FILES));
                  
                  define(PROP_FILE_RECENT_SCRIPT_DIR,
                         System.getenv(PROP_PLEXIL_HOME) != null
                         ? System.getenv(PROP_RECENT_FILES) + "/scripts"
                         : System.getProperty(PROP_RECENT_FILES));

                  //define(PROP_FILE_UELIB,        PROP_FILE_UELIB_DEF);

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

                  define(PROP_VIEW_HIDE_PLEXILLISP, 
                         PROP_VIEW_HIDE_PLEXILLISP_DEF);
               }
         };
         
      public static boolean allowDebug, pauseAtStart, isExecuting, executedViaLuvViewer = false;
      public static boolean allowBreaks = true;
      public static File debug, plan, script;
      public static FileWriter emptyScript;
      public static String planName, scriptName = "";

      /** current view */

      View currentView;

      /** queue of status messages */

      LinkedList<StatusMessage> statusMessageQ = new LinkedList<StatusMessage>();

      /** count of specifed libraries not yet loaded */

      int outstandingLibraryCount = 0;

      /** set of active libraries */

      Vector<Model> libraries = new Vector<Model>();

      /** if break has occured, the causal break point object */

      IBreakPoint breakPoint = null;

      /** a collect of all breakpoints */

      HashMap<IBreakPoint, ModelPath> breakPoints = 
         new HashMap<IBreakPoint, ModelPath>();

      /** breakpoints not found in this plan */

      Vector<IBreakPoint> unfoundBreakPoints = new Vector<IBreakPoint>();

      /** is the plan current paused */

      boolean planPaused = false;

      /** should the plan take one step */

      boolean planStep = false;

      /** recent file menu */

      JMenu recentPlanMenu = new JMenu("Recent Plans");

      /** the execution control menu */

      JMenu execMenu;

      /** custom view acion menu */

      JMenu viewMenu = new JMenu("View");

      /** debugging window text */
      
      DebugWindow debugWindow;

      /** file chooser object */
      
      JFileChooser fileChooser = new JFileChooser()
         {
               {
                  /** XML file fliter */
                  
                  addChoosableFileFilter(new FileFilter()
                     {
                           // accept file?
                           
                           public boolean accept(File f) 
                           {
                              // allow browse directories
                              
                              if (f.isDirectory())
                                 return true;
                              
                              // allow files with correct extention
                              
                              String extension = getExtension(f);
                              Boolean correctExtension = false;
                              if (extension != null)
                                  if (extension.equals(XML_EXTENSION) || 
                                      extension.equals(PLX_EXTENSION))
                                      correctExtension = true;
                              
                              return correctExtension;
                           }

                           // get file extension
                           
                           public String getExtension(File f)
                           {
                              String ext = null;
                              String s = f.getName();
                              int i = s.lastIndexOf('.');
                              
                              if (i > 0 && i < s.length() - 1)
                                 ext = s.substring(i+1).toLowerCase();
                              
                              return ext;
                           }

                           // return descriton
                           
                           public String getDescription()
                           {
                              return "XML Files (.xml) / PLX Files (.plx)";
                           }
                     });
               }
         };

      /** directory chooser object */
      
      JFileChooser dirChooser = new JFileChooser()
         {
               {
                  setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
               }
         };

      /** Panel in which different views are placed. */

      JPanel viewPanel = new JPanel();

      /** The different views available. */

      Views views = new Views();

      /** the current working instance of luv */

      private static Luv theLuv;

      /**
       * Entry point for this program.
       */

      public static void main(String[] args)
      {
         //runExecTest();
         runApp();
         //runServerTest();
      }

      public static void runServerTest()
      {
         try
         {
            Socket ss = new Socket("127.0.0.1", 9787);
            ss.sendUrgentData(99);
            try
            {
               out.println("start sleep.");
               Thread.sleep(5000);
               out.println("stop sleep.");
            }
            catch (Exception e)
            {
               e.printStackTrace();
            }
            out.println("test");
         }
         catch (Exception e)
         {
            e.printStackTrace();
         }
      }
      
      /** Locates or Creates the appropriate script file.
       *
       * @param scriptName string that represents the script filename
       * @param planName string that represents the plan filename
       * @param path string that represents the absolute path to where scripts are located
       */

      public void findScriptOrCreateEmptyOne(String scriptName, String planName, String path) throws IOException 
      {
          try 
          {
              //PLAN_script.plx
              String name1 = planName + "_script";
              script = new File(path + name1);
              if (!script.canRead())
              {
                  //script-PLAN.plx
                  String name2 = "script-" + planName;
                  script = new File(path + name2);
                  if (!script.canRead())
                  {
                      //script_PLAN.plx
                      String name3 = "script_" + planName;
                      script = new File(path + name3);
                      if (!script.canRead())
                      {
                          script = new File(path + "default-empty-script.plx");
                          if (!script.canRead())
                          {
                              //must create an empty script
                              scriptName = path + "default-empty-script.plx";
                              emptyScript = new FileWriter(scriptName);
                              BufferedWriter out = new BufferedWriter(emptyScript);
                              out.write(EMPTY_SCRIPT);
                              out.close();                          
                              script = new File(scriptName);
                          }
                      }
                  }
              }
          }
          catch (Exception e)
          {
            out.println("Error: " + e.getMessage());
          }    
      }
      
      public void runExecTest() throws IOException
      {                    
         File exec = new File(PROP_UE_EXEC);
        // File lib = new File(PROP_FILE_UELIB_DEF);

         while (model.getPlanName() == null && plan == null)
             choosePlan();
    
         if (model.getPlanName() != null)  
             plan = new File(model.getPlanName());
            
         if (model.getScriptName() != null)
             script = new File(model.getScriptName());
         
         if (script == null || !script.canRead())
         {      
             String path = PROP_RECENT_FILES + "/scripts/";
             String name = plan.getName().replace(".plx", "-script.plx");
             script = new File(path + name);    
             
             if (!script.canRead())
                 findScriptOrCreateEmptyOne(script.getName(),plan.getName(),path);
         }
         
         model.addScriptName(script.getName());
         
         ExecExec ee = new ExecExec(exec, null, null, plan, script, allowBreaks, allowDebug);
         
         try
         {
            InputStream is = ee.start();
            InputStream es = ee.getErrorStream();
            while (ee.isRunning())
            {
              while (is.available() > 0)
                  out.write(is.read());
              while (es.available() > 0)
                  err.write(es.read());
            }
            while (is.available() > 0)
               out.write(is.read());
            while (es.available() > 0)
               err.write(es.read());
            out.flush();
         }
         catch (Exception e)
         {
            e.printStackTrace();
         }
      }

      public static void runApp()
      {
         // if we're on a mac, us mac style menus
         
         System.setProperty("apple.laf.useScreenMenuBar", "true");

         // give me some luv
         
         try
         {
            new Luv();
         }
         catch (Exception e)
         {
            e.printStackTrace();
         }
      }
      
      public Luv()
      {
         // record instance of self

         theLuv = this;

         // construct the frame
         
         constructFrame(getContentPane());

         // set tooltip display time

         ToolTipManager.sharedInstance().setDismissDelay(
            properties.getInteger(PROP_TOOLTIP_DISMISS));
         
         // expirentmenting with captureing key events

//          KeyListener kl = new KeyListener()
//             {
//                   public void keyPressed(KeyEvent e)
//                   {
//                      System.out.println("press: " + e);
//                   }
//                   public void keyReleased(KeyEvent e)
//                   {
//                      System.out.println("release: " + e);
//                   }
//                   public void keyTyped(KeyEvent e)
//                   {
//                      System.out.println("type: " + e);
//                   }
//             };


//          getRootPane().addKeyListener(kl);
//          getGlassPane().addKeyListener(kl);
//          getLayeredPane().addKeyListener(kl);
//          addKeyListener(kl);

         // app exits when frame is closed

         setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

         // things to do when app is closed

         Runtime.getRuntime().addShutdownHook(new Thread()
            {
                  public void run()
                  {
                     properties.set(PROP_WIN_LOC, getLocation());
                     properties.set(PROP_WIN_SIZE, getSize());
                     properties.set(PROP_DBWIN_LOC, debugWindow.getLocation());
                     properties.set(PROP_DBWIN_SIZE, debugWindow.getSize());
                     properties.set(PROP_DBWIN_VISIBLE, debugWindow.isVisible());
                     currentView.setViewProperties(properties);
                  }
            });
         
         // set size and location of frame

         setLocation(properties.getPoint(PROP_WIN_LOC));
         setPreferredSize(properties.getDimension(PROP_WIN_SIZE));

         // create the debug window
         
         debugWindow = new DebugWindow(this);

         // set size and location off frame

         debugWindow.setLocation(properties.getPoint(PROP_DBWIN_LOC));
         debugWindow.setPreferredSize(properties.getDimension(PROP_DBWIN_SIZE));
         debugWindow.pack();


         // when this frame get's focus, get it's menu bar back from the debug window

         addWindowFocusListener(new WindowAdapter()
            {
                  public void windowGainedFocus(WindowEvent e)
                  {
                     JMenuBar mb = debugWindow.getJMenuBar();
                     if (mb != null) setJMenuBar(mb);
                  }
            });

         // make debug window visible

         debugWindow.setVisible(properties.getBoolean(PROP_DBWIN_VISIBLE));
         
         // make the frame visable
         
         pack();
         setVisible(true);

         // add a listener to the model to catch new plan events

         model.addChangeListener(new Model.ChangeAdapter()
            {
                  @Override public void libraryNameAdded(
                     Model model, String libraryFilename)
                  {
                     ++outstandingLibraryCount;
                  }
            });

         // if expected to, autoload most recent plan and script

         if (properties.getBoolean(PROP_FILE_AUTO_LOAD))
            loadRecentPlan(1);
            
         allowBreaksAction.actionPerformed(null);

         // start the server listening for events

         Server s = new Server(properties.getInteger(PROP_NET_SERVER_PORT))
            {
                  @Override public void handleMessage(final String message)
                  {  
                     
                    /** Determine if the Luv Viewer should pause before executing. */
                      
                      if (pauseAtStart)
                      {
                          if (!executedViaLuvViewer)
                          {
                              Boolean reset = false;
                              planPaused = true;
                              if (model.getProperty(VIEWER_BLOCKS) == null ||
                                  model.getProperty(VIEWER_BLOCKS).equals(FALSE) ||
                                  model.getProperty(VIEWER_BLOCKS).equals("false"))
                              {
                                  model.setProperty(VIEWER_BLOCKS, TRUE);
                                  reset = true;
                              }

                              doesViewerBlock();                          

                              if (reset)
                                  model.setProperty(VIEWER_BLOCKS, FALSE);
                          }
                          pauseAtStart = false;
                          executedViaLuvViewer = false;
                      }
                      
                     // parse the message
                     
                     boolean isPlan = parseXml(new ByteArrayInputStream(
                                                  message.getBytes()), model);

                     // if this is a plan (or possibly a library)

                     if (isPlan)
                     {
                        // if this is a library, store this in set of libraries
               
                        if (outstandingLibraryCount > 0)
                        {
                           libraries.add(model.removeChild(NODE));
                           --outstandingLibraryCount;
                        }
                        
                        // otherwise it's a plan, link that plan and
                        // libraries and show the new plan
                        
                        else
                        {
                           try
                           {
                              link(model, libraries);
                           }
                           catch (LinkCanceledException lce) {}
                           resetView();
                           libraries.clear();
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
                        showStatus((breakPoint == null
                                    ? "Plan execution paused."
                                    : breakPoint.getReason()) +
                           "  Hit " + 
                           pauseAction.getAcceleratorDescription() +
                           " to resume, or " + 
                           stepAction.getAcceleratorDescription() +
                           " to step.",
                           Color.RED);
                        breakPoint = null;
                        
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

                     planStep = false;

                     return blocks;
                  }
            };
      }

      /**
       * Place all visible elements into the container in the main frame
       * of the application.
       *
       * @param frame the container in witch to place items.
       */

      public void constructFrame(Container frame)
      {
         // set layout and background color

         setLayout(new BorderLayout());
         setBackground(properties.getColor(PROP_WIN_BCLR));

         // create a menu bar

         JMenuBar menuBar = new JMenuBar();
         setJMenuBar(menuBar);
         
         // create file menu

         JMenu fileMenu = new JMenu("File");
         menuBar.add(fileMenu);
         fileMenu.add(openAction);
         fileMenu.add(openScriptAction);
         updateRecentMenu();
         fileMenu.add(recentPlanMenu);
         fileMenu.add(reloadAction);
         //fileMenu.add(testAction);
         fileMenu.add(new JSeparator());
         fileMenu.add(exitAction);

         // create and update Run menu

         execMenu = new JMenu("Run");
         menuBar.add(execMenu);
         updateExecMenu();
         
         

         // add veiw menue

         menuBar.add(viewMenu);

         // show window menu

         JMenu windowMenu = new JMenu("Window");
         menuBar.add(windowMenu);
         windowMenu.add(debugWindowAction);         

         // add view panel

         frame.add(viewPanel, CENTER);

         // create the status bar

         final JLabel statusBar = new JLabel(" ");
         statusBar.setBorder(new EmptyBorder(2, 2, 2, 2));
         frame.add(statusBar, SOUTH);
         startStatusBarThread(statusBar);
      }

      /** Creates and returns the status bar thread.
       *
       * @param statusBar the status bar to create the thread around
       */

      public void startStatusBarThread(final JLabel statusBar)
      {
         new Thread()
         {
               @Override public void run()
               {
                  try
                  {
                     StatusMessage lastMessage = null;
                     
                     while (true)
                     {
                        if (!statusMessageQ.isEmpty())
                        {
                           // kill any preceeding auto clear thread

                           if (lastMessage != null)
                              lastMessage.abortAutoClear = true;

                           // get the message

                           final StatusMessage message = 
                              statusMessageQ.removeFirst();
                           lastMessage = message.autoClearTime > 0 
                              ? message
                              : null;

                           // show the message

                           statusBar.setForeground(message.color);
                           statusBar.setText(message.message);
                           if (message.message != 
                               StatusMessage.BLANK_MESSAGE.message)
                              out.println("STATUS: " + message.message);

                           // if auto clear requestd start a thread for that

                           if (message.autoClearTime > 0)
                              new Thread()
                              {
                                    @Override public void run()
                                    {
                                       try
                                       {
                                          sleep(message.autoClearTime);
                                          if (!message.abortAutoClear)
                                             statusBar.setText(
                                                StatusMessage.BLANK_MESSAGE.message);
                                       }
                                       catch (Exception e)
                                       {
                                          e.printStackTrace();
                                       }
                                    }
                              }.start();
                        }

                        // wait a bit then check for the next message

                        sleep(50);

                     }
                  }
                  catch (Exception e)
                  {
                  }
               }
         }.start();
      }

      /** Update the state of the exec menu */

      public void updateExecMenu()
      {
         // clear any stuff from exec menu

         execMenu.removeAll();

         // add pause and step

         execMenu.add(pauseAction);
         execMenu.add(stepAction);
         execMenu.add(allowBreaksAction);
         execMenu.add(execAction);
         

          // add break point menu

         if (breakPoints.size() > 0)
         {
            execMenu.add(new JSeparator());
            
            JMenu breakMenu = new JMenu("Break Points");
            execMenu.add(breakMenu);
            
            // add all the break points

            for (final IBreakPoint breakPoint: breakPoints.keySet())
            {
               String actionStr = breakPoint.isEnabled() ? "Disable" : "Enable";
               LuvAction action = new LuvAction(
                  actionStr + " " + breakPoint,
                  actionStr + " the breakpoint " + 
                  breakPoint + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           breakPoint.setEnabled(!breakPoint.isEnabled());
                           updateExecMenu();
                           refreshView();
                        }
                  };
               
               breakMenu.add(action);

               if (unfoundBreakPoints.contains(breakPoint))
                  action.setEnabled(false);
            }

            // add enabled disable all items

            breakMenu.add(new JSeparator());
            breakMenu.add(enableBreakpointsAction);
            breakMenu.add(disableBreakpointsAction);

            // add enabled remove all items

            breakMenu.add(new JSeparator());
            breakMenu.add(removeAllBreakpointsAction);
         }
      }

      /** Reset the currevnt view to refect the changes in the world. */

      public void resetView()
      {
         // create a new instance of the view

         setView(new TreeTableView("", this, model));

         // map all the breakpoints into the new model

         unfoundBreakPoints.clear();
         for (Map.Entry<IBreakPoint, ModelPath> pair: breakPoints.entrySet())
         {
            IBreakPoint breakPoint = pair.getKey();
            ModelPath path = pair.getValue();

            Model target = path.find(model);
            if (target != null)
            {
               breakPoint.setModel(target);
            }
            else
               unfoundBreakPoints.add(breakPoint);
         }

         // update the exec menu

         updateExecMenu();
      }

      /** Refresh the current view */

      public void refreshView()
      {
         ((Container)currentView).repaint();
         updateExecMenu();
      }

      /**
       * Sets the current view. 
       *
       * @param view view to display
       */

      public void setView(Container view)
      {
         // handle view properties

         if (currentView != null)
            currentView.setViewProperties(properties);
         currentView = (View)view;
         currentView.getViewProperties(properties);

         // clear out the view panel and put the new view in there

         viewPanel.removeAll();
         viewPanel.setLayout(new BorderLayout());
         JScrollPane sp = new JScrollPane(view);
         sp.setBackground(properties.getColor(PROP_WIN_BCLR));
         viewPanel.add(sp, CENTER);

         // insert the view menu items

         viewMenu.removeAll();
         for(LuvAction action: currentView.getViewActions())
            viewMenu.add(action);
         viewMenu.add(showHidePrlNodes);


         // enable that menu if we actally have menu items
         
         viewMenu.setEnabled(viewMenu.getMenuComponentCount() > 0);
         setLocation(getLocation());

         // size everything

         setPreferredSize(getSize());

         // set the frame title
         
         setTitle();

         // show the new view

         pack();
         repaint();
      }

      /** Get the current active instance of luv.
       *
       * @return current luv instance.
       */

      public static Luv getLuv()
      {
         return theLuv;
      }

      /** Set the title of the application. */

      public void setTitle()
      {
         String planFile = model.getPlanName();
         String scriptFile = "";
         
         if (script != null)
             scriptFile = " SCRIPT: " + script.getName();
         
         if (planFile != null)
         {
            String plan = stripDotXml(new File(planFile).getName());
            StringBuffer title = new StringBuffer(plan);
            
            for (String libName: model.getLibraryNames())
               title.append(" + " + stripDotXml(new File(libName).getName()));
            
            setTitle("PLAN: " + title.toString() + scriptFile);
         }
      }

      /** Set a program wide property.
       *
       * @param key unique name of property
       * @param value value of property as a string
       */

      public void setProperty(String key, String value)
      {
         properties.setProperty(key, value);
      }

      /**
       * Set a program wide property.
       *
       * @param key unique name of property
       * @return The value of property as a string, null if it is not
       * present in the list of properties.
       */

      public String getProperty(String key)
      {
         return properties.getProperty(key);
      }

      /**
       * Read plexil plan from disk and create an internal model.
       *
       * @param model model to read plan into
       * @param file file containing plan
       *
       */

      public void readPlan(Model model, File file)
      {
         showStatus("Loading "  + file);
         try
         {
            parseXml(new FileInputStream(file), model);
         }
         catch(Exception e)
         {
            JOptionPane.showMessageDialog(
               this,
               "Error loading plan: " + file.getName() + 
               "  See debug window for details.",
               "Parse Error",
               JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
         }
         clearStatus();
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
         
         final Vector<IBreakPoint> breakPoints = getBreakPoints(model);

         // if we got any add enabele/disabel & remove item for each one

         if (breakPoints.size() > 0)
         {
            // add the breakpoints

            popup.add(new JSeparator());
            for (final IBreakPoint breakPoint: breakPoints)
            {
               String action = breakPoint.isEnabled() ? "Disable" : "Enable";
               popup.add(new LuvAction(
                            action + " " + breakPoint,
                            action + " the breakpoint " + 
                            breakPoint + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           breakPoint.setEnabled(!breakPoint.isEnabled());
                           refreshView();
                        }
                  }); 
            }

            // add the breakpoints

            popup.add(new JSeparator());
            for (final IBreakPoint breakPoint: breakPoints)
            {
               popup.add(new LuvAction(
                            "Remove " + breakPoint,
                            "Permanently remove the breakpoint " + 
                            breakPoint + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           removeBreakPoint(breakPoint);
                        }
                  }); 
            }
         }

         // if there is more then one break point add a remove all item

         if (breakPoints.size() > 1)
         {
            // add the remove all action
            
            popup.add(new JSeparator());
            popup.add(new LuvAction(
                         "Remove All Break Points From " + name,
                         "Permanently remove all breakpoints from " + name + ".")
               {
                     public void actionPerformed(ActionEvent e)
                     {
                        for (final IBreakPoint breakPoint: breakPoints)
                           removeBreakPoint(breakPoint);
                        
                        updateExecMenu();
                     }
               });
         }

         // return our freshly created popup menu

         return popup;
      }

      /** Return all the breakpoints for a given model. */

      public Vector<IBreakPoint> getBreakPoints(Model model)
      {
         Vector<IBreakPoint> bps = new Vector<IBreakPoint>();
         for (IBreakPoint breakPoint: breakPoints.keySet())
            if (model == breakPoint.getModel())
               bps.add(breakPoint);
         return bps;
      }
      
      /** An abstract breakpoint which supplies the breaking action in
       * the event that a breakpoint is signaled.  Derrived classes are
       * expected to provide the conditions underwich the break is
       * eligable to fire. */

      public abstract class LuvBreakPoint extends AbstractBreakPoint
      {
            /** Old value, use to test for changes */

            String oldValue = "";

            /** Targe model property to watch. */
            
            String targetProperty;

            /** storage for reason for current break point occurrance. */

            String reason = "NO break has occurred yet. This should NEVER be seen!";

            /** Construct a Luv specific break point. 
             *
             * @param model the model on which the break point operates
             */
            
            public LuvBreakPoint(Model model, String targetProperty)
            {
               super(model);
               this.targetProperty = targetProperty;
               oldValue = model.getProperty(targetProperty);
               addBreakPoint(this, model);
            }
            
            /** {@inheritDoc} */
            
            public void onBreak()
            {
               planPaused = true;
               breakPoint = this;
               oldValue = model.getProperty(targetProperty);
            }

            /** {@inheritDoc} */
            
            public String getReason()
            {
               return reason;
            }

            /** {@inheritDoc} */

            public void setModel(Model model)
            {
               super.setModel(model);
               if (targetProperty != null)
                  oldValue = model.getProperty(targetProperty);
            }
      };

      /** Add breakpoint to grand list of breakpoints.
       *
       * @param breakPoint breakpoint to add
       * @param model model breakpoint is associated with
       */

      public void addBreakPoint(IBreakPoint breakPoint, Model model)
      {
         ModelPath mp = new ModelPath(model);
         breakPoints.put(breakPoint, mp);

         showStatus("Added break on " + breakPoint, 5000l);
         refreshView();
      }

      /** Remove breakpoint from grand list of breakpoints.
       *
       * @param breakPoint breakpoint to remove
       */

      public void removeBreakPoint(IBreakPoint breakPoint)
      {
         breakPoint.unregister();
         breakPoints.remove(breakPoint);
         showStatus("Removed break on " + breakPoint, 5000l);
         refreshView();
      }

      /** Remove all breakpoints from grand list of breakpoints. */

      public void removeAllBreakPoints()
      {
         for (IBreakPoint breakPoint: breakPoints.keySet())
            breakPoint.unregister();
         breakPoints.clear();

         showStatus("Removed all breakponts.", 5000l);
         refreshView();
      }
      
      /** Create a breakpoint which fires when the model state
       * changes.
       *
       * @param model the model to watch for state changes
       */
      
      public IBreakPoint createChangeBreakpoint(Model model)
      {
         return new LuvBreakPoint(model, MODEL_STATE)
            {
                  public boolean isBreak()
                  {
                     return !model.getProperty(MODEL_STATE).equals(oldValue);
                  }
                  
                  public void onBreak()
                  {
                     reason = model.getProperty(MODEL_NAME) + 
                        " changed from " + oldValue +
                        " to " + model.getProperty(MODEL_STATE) + ".";
                     oldValue = model.getProperty(MODEL_STATE);
                     super.onBreak();
                  }

                  public String toString()
                  {
                     return model.getProperty(MODEL_NAME) +
                        " state changed";
                  }
         };
      }

      /** Create a breakpoint which fires when the model state
       * changes to a specifed state.
       *
       * @param model the model to watch for state changes
       * @param propertyTitle printed name of property
       * @param targetProperty property to watch for break
       * @param targetValue value to watch for break
       */
      
      public IBreakPoint createTargetPropertyValueBreakpoint(
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

      /** Parse a given XML message string.
       *
       * @param input source of xml to parse
       * @param model model to work on
       *
       * @return returns true if the xml was in the form of a plan
       */

      public boolean parseXml(InputStream input, Model model)
      {
         boolean isPlan = false;

         try
         {
            InputSource is = new InputSource(input);
            XMLReader parser = XMLReaderFactory.createXMLReader();
            DispatchHandler dh = new DispatchHandler(model);
            parser.setContentHandler(dh);
            parser.parse(is);

            // if the dispatch handler selected a plexil plan handler
            // then the a new plans was read in

            isPlan = dh.getHandler() instanceof PlexilPlanHandler;
         }
         catch (Exception e)
         {
            JOptionPane.showMessageDialog(
               this,
               "Error parsing XML message.  See debug window for details.",
               "Parse Error",
               JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
         }

         return isPlan;
      }

      /** Link a plan with it's provided libraries 
       *
       * @param node current node of model being linked
       * @param libraries the collected libraries which will linked into
       * the plan
       *
       * @return false if all linking cancled by user, this does NOT mean that the 
       */

      public boolean link(Model node, Vector<Model> libraries) 
         throws LinkCanceledException
      {
         // if this is a library node call, link that node
         
         String type = node.getProperty(NODETYPE_ATTR);
         if (type != null && type.equals(LIBRARYNODECALL))
         {
            boolean retry = true;
            String callName = node.getProperty(MODEL_LIBRARY_CALL_ID);

            do
            {
               // look through the libraries for match

               for (Model library: libraries)
               {
                  String libName = library.getProperty(NODE_ID);
                  
                  if (callName.equals(libName))
                  {
                     node.addChild((Model)library.clone());
                     retry = false;
                     break;
                  }
               }

               // if we didn't make the link, ask user for library

               if (retry)
               {
                  // option

                  Object[] options = 
                     {
                        "load this library",
                        "do not load this library",
                        "stop asking about libraries",
                     };

                  // show the options

                  int result = JOptionPane.showOptionDialog(
                     this,
                     "Missing library which contains \"" + callName +
                     "\".  Would you like to load this library?",
                     "Load the library?",
                     JOptionPane.YES_NO_CANCEL_OPTION,
                     JOptionPane.WARNING_MESSAGE,
                     null,
                     options,
                     options[0]);

                  // process the results

                  switch (result)
                  {
                     // try to load the library and retry the link

                     case 0:
                        chooseLibrary();
                        break;
                        
                        // if the user doesn't want to find this library
                        // go on with link but don't retry to like this
                        // one

                     case 1:
                        return false;
                        
                     // if the user doesn't want to load any libraries,
                     // halt the link operation now
                     
                     case 2:
                        throw new LinkCanceledException(callName);
                  }
               }
            }
            while (retry);
         }

         // if this is node, traverse into any children it might have,
         // note that this could be a node could have newly minted
         // children from the linking action above

         boolean success = true;
         for (Model child: node.getChildren())
            if (!link(child, libraries))
               success = false;

         return success;
      }
      
      /**
       * Select and load a script from the disk.  This operates on
       * the global model.
       */
      
      public void chooseScript()
      {
         try
         {
            fileChooser.setCurrentDirectory(
               new File(properties.getString(PROP_FILE_RECENT_SCRIPT_DIR)));
            if (fileChooser.showOpenDialog(this) == APPROVE_OPTION)
            {
               script = fileChooser.getSelectedFile();
               properties.set(PROP_FILE_RECENT_SCRIPT_DIR, script.getParent());
               loadScript(script);
            }
         }
         catch(Exception e)
         {
            e.printStackTrace();
         }
      }
      
       /**
       * Load a plexil script from the disk.  This operates on the global
       * model.
       *
       * @paramscript file to load
       */
      
      public void loadScript (File script)
      {
         model.addScriptName(script.toString());
         resetView();
      }


      /**
       * Select and load a plexil plan from the disk.  This operates on
       * the global model.
       */
      
      public void choosePlan()
      {
         try
         {
            fileChooser.setCurrentDirectory(
               new File(properties.getString(PROP_FILE_RECENT_PLAN_DIR)));
            if (fileChooser.showOpenDialog(this) == APPROVE_OPTION)
            {
               File plan = fileChooser.getSelectedFile();
               properties.set(PROP_FILE_RECENT_PLAN_DIR, plan.getParent());
               script = null;
               loadPlan(plan);
            }
         }
         catch(Exception e)
         {
            e.printStackTrace();
         }
      }

      /**
       * Load a plexil plan from the disk.  This operates on the global
       * model.
       *
       * @param plan the plan file to load
       */
      
      public void loadPlan(File plan)
      {
         loadPlan(plan, null);
      }

      /**
       * Load a plexil plan from the disk.  This operates on the global
       * model.
       *
       * @param plan the plan file to load
       * @param libraryNames names of library file the plan to load
       */
      
      public void loadPlan(File plan, Vector<String> libraryNames)
      {
         model.clear();
         model.addPlanName(plan.toString());
         model.setProperty(VIEWER_BLOCKS, FALSE);
         
         readPlan(model, plan);
         if (libraryNames != null)
            for (String libraryName: libraryNames)
               loadLibrary(new File(libraryName));
         try
         {
            link(model, libraries);
         }
         catch (LinkCanceledException lce) {}
         resetView();
         addRecent();
         outstandingLibraryCount = 0;
         libraries.clear();
      }

      /**
       * Select and load a plexil library from the disk.  The library is
       * added to the set of global libraries.
       */
      
      public void chooseLibrary()
      {
         try
         {
            String recent = properties.getString(PROP_FILE_RECENT_LIB_DIR);
            if (recent == null)
               recent = properties.getString(PROP_FILE_RECENT_PLAN_DIR);
            fileChooser.setCurrentDirectory(new File(recent));
            if (fileChooser.showOpenDialog(this) == APPROVE_OPTION)
            {
               File library = fileChooser.getSelectedFile();
               properties.set(PROP_FILE_RECENT_LIB_DIR, library.getParent());
               loadLibrary(library);
            }
         }
         catch(Exception e)
         {
            e.printStackTrace();
         }
      }

      /**
       * Load a plexil library from the disk.  The library is added to
       * the set of global libraries.
       */
      
      public void loadLibrary(File libraryFile)
      {
         model.addLibraryName(libraryFile.toString());
         Model library = new Model("a library");
         readPlan(library, libraryFile);
         libraries.add(library.findChild(NODE));
      }

      /**
       * Read all plans in a dirctory for testing purposes.
       */
      
      public void test()
      {
         try
         {
            String recent = properties.getString(PROP_FILE_RECENT_TEST_DIR);
            if (recent == null)
               recent = properties.getString(PROP_FILE_RECENT_PLAN_DIR);

            dirChooser.setCurrentDirectory(
               new File(new File(recent).getParent()));
            if (dirChooser.showOpenDialog(this) == APPROVE_OPTION)
            {
               File dir = dirChooser.getSelectedFile();
               properties.set(PROP_FILE_RECENT_TEST_DIR, dir.toString());
               File[] tests = dir.listFiles(new java.io.FileFilter()
                  {
                        public boolean accept(File file)
                        {
                           return file.getName().endsWith(XML_EXTENSION);
                        }
                  });

               for (File file: tests)
               {
                  readPlan(new Model("Test Model"), file);
               }
            }
         }
         catch(Exception e)
         {
            e.printStackTrace();
         }
      }

      /** Load a recently loaded plan.
       *
       * @param index index of recently loaded plan
       */

      public void loadRecentPlan(int index)
      {
         String planName = getRecentPlanName(index);
         String scriptName = getRecentScriptName(index);
         if (planName != null)
         {
            loadPlan(new File(planName), getRecentLibNames(index, false));
            if (scriptName != null)
                script = new File(scriptName);
            resetView();
         }
      }
      
      /** Add a file to the recently open files list. */

      public void addRecent()
      {
         // put newest file at the top of the list

         String current = model.getPlanName();
         Vector<String> libraries = model.getLibraryNames();

         String filename = current;
         int count = properties.getInteger(PROP_FILE_RECENT_COUNT);
         for (int i = 1; i <= count && filename != null; ++i)
         {
            if (filename != null)
            {
               // get (and remove) the old library names at this index

               Vector<String> tmpLibs = getRecentLibNames(i, true);

               // replace them with these library names

               int libIndex = 1;
               for (String library: libraries)
                  setRecentLibName(i, libIndex++, library);
               libraries = tmpLibs;

               
               //for (String lib: libraries

               filename = (String)properties
                  .setProperty(PROP_FILE_RECENT_PLAN_BASE + i, filename);

               // if this file already existed in the list, we can stop
               // it already appears at the top

               if (filename != null && filename.equals(current))
                  break;
            }
         }

         // update the recent menu

         updateRecentMenu();
      }

      /** Get a list of library names associated with a recently loaded plan.
       *
       * @param recentIndex the index of the recent plan these libraries
       * were loaded with
       * @param remove remove names as they are collected
       *
       * @return a vector of library names, which might be empty
       */

      public Vector<String> getRecentLibNames(int recentIndex, boolean remove)
      {
         Vector<String> libNames = new Vector<String>();
         int libIndex = 1;

         String libName;
         while ((libName = getRecentLibName(
                    recentIndex, libIndex++, remove)) != null)
            libNames.add(libName);

         return libNames;
      }

      /** Given a recent plan index and library index return the
       * associated recent library filename.
       *
       * @param recentIndex the index of the recent plan this library
       * was loaded with
       * @param libIndex the index of the particular library
       * @param remove remove name as it is collected
       *
       * @return the name of the library, which may be null
       */

      public String getRecentLibName(int recentIndex, int libIndex, 
                                     boolean remove)
      {
         String name = PROP_FILE_RECENT_LIB_BASE + recentIndex + "-" + libIndex;
         String prop = properties.getProperty(name);
         if (remove)
            properties.remove(name);
         return prop;
      }


      /** Given a recent plan index and library index, set that library name.
       *
       * @param recentIndex the index of the recent plan this library
       * was loaded with
       * @param libIndex the index of the particular library
       * @param libName name of this library
       *
       * @return the old value of this property
       */

      public String setRecentLibName(int recentIndex, int libIndex, String libName)
      {
         String name = PROP_FILE_RECENT_LIB_BASE + recentIndex + "-" + libIndex;
         return (String)properties.setProperty(name, libName);
      }

      /** Given a recent plan index, return the associated recent plan
       * filename.
       *
       * @param recentIndex the index of the recent plan
       *
       * @return the name of the recent plan, possibly null
       */

      public String getRecentPlanName(int recentIndex)
      {
         return properties.getProperty(PROP_FILE_RECENT_PLAN_BASE + recentIndex);
      }
      
       /** Given a recent script index, return the associated recent script
       * filename.
       *
       * @param recentIndex the index of the recent script
       *
       * @return the name of the recent script, possibly null
       */

      public String getRecentScriptName(int recentIndex)
      {
         return properties.getProperty(PROP_FILE_RECENT_SCRIPT_BASE + recentIndex);
      }


      /** Given a recent plan index, the description used for the recent
       * menu item tooltip. 
       *
       * @param recentIndex the index of the recent plan
       *
       * @return the description of what gets loaded
       */

      public String getRecentMenuDescription(int recentIndex)
      {
         File plan = new File(getRecentPlanName(recentIndex));
         StringBuffer description = new StringBuffer("Load " + plan.getName());

         for (String libName: getRecentLibNames(recentIndex, false))
            description.append(" + " + new File(libName).getName());
         
         description.append(".");
         return description.toString();
      }

      public String stripDotXml(String filename)
      {
         return filename.split(".xml")[0];
      }

      /** Update the recently loaded files menu. */

      public void updateRecentMenu()
      {
         recentPlanMenu.removeAll();
         int count = properties.getInteger(PROP_FILE_RECENT_COUNT);
         for (int i = 0; i < count; ++i)
            if (getRecentPlanName(i + 1) != null)
               recentPlanMenu.add(
                  new LoadRecentAction(i + 1, '1' + i, META_MASK));

         // this menu is only enabled when there are items in it
         
         recentPlanMenu.setEnabled(recentPlanMenu.getMenuComponentCount() > 0);
      }      

      /** A status message, with some text, a message color, and an
       * optional auto clear time. */

      public static class StatusMessage
      {
            public static final StatusMessage BLANK_MESSAGE = 
               new StatusMessage(" ", Color.BLACK, 0);

            boolean abortAutoClear = false;
            long    autoClearTime = 0;
            Color   color = Color.BLACK;
            String  message;

            public StatusMessage(String message, Color color, long autoClearTime)
            {
               this.message = message;
               this.color = color;
               this.autoClearTime = autoClearTime;
            }
      }
            
      /** Add message to status bar.
       *
       * @param message message to add to status bar
       */
        
      public void showStatus(String message)
      {
         showStatus(message, Color.BLACK, 0);
      }

      /** Add message to status bar.
       *
       * @param message message to add to status bar
       * @param autoClearTime milliseconds to wait before clearing the
       * message, a value <= 0 will be left until some other action
       * clears or overwrites this message
       */
        
      public void showStatus(String message, long autoClearTime)
      {
         showStatus(message, Color.BLACK, autoClearTime);
      }

      /** Add message to status bar.
       *
       * @param message message to add to status bar
       * @param color the color of the mesage
       */
        
      public void showStatus(String message, Color color)
      {
         showStatus(message, color, 0);
      }

      /** Add message to status bar.
       *
       * @param message message to add to status bar
       * @param color the color of the mesage
       * @param autoClearTime milliseconds to wait before clearing the
       * message, a value <= 0 will be left until some other action
       * clears or overwrites this message
       */
        
      public void showStatus(String message, Color color, final long autoClearTime)
      {
         statusMessageQ.add(new StatusMessage(message, color, autoClearTime));
      }

      /** Clear the status bar. */

      public void clearStatus()
      {
         statusMessageQ.add(StatusMessage.BLANK_MESSAGE);
      }

      static public ImageIcon loadImage(String name)
      {
         return new ImageIcon(
            Toolkit.getDefaultToolkit()
            .getImage(ClassLoader.getSystemResource(ICONS_DIR + name)));
      }

      /**
       * Exit this program.
       */

      public void exit()
      {
         System.exit(0);
      }

      /** Exception used to signal that the user cancled a link. */

      public class LinkCanceledException extends Exception
      {
            LinkCanceledException(String nodeId)
            {
               super("Link cancled at node: " + nodeId);
            }
      }

      /** Action to select different views. */

      public class SelectViewAction extends LuvAction
      {
            /** View to switch to when this action is performed. */

            Container view;

            /** Construct a view action.
             *
             * @param view    view selected when this action is peformed
             * @param keyCode identifies shortcut key for this action
             */

            public SelectViewAction(Container view, int keyCode)
            {
               super(view.toString(), "Select " + view + " view", keyCode);
               this.view = view;
            }
            
            /**
             * Called when user wishes to make visible this type of view.
             *
             * @param  e action event 
             */

            public void actionPerformed(ActionEvent e)
            {
               setView(view);
            }
      }

      /** Action to load a recent plan. */

      public class LoadRecentAction extends LuvAction
      {
            /** File to switch to open when action is performed. */

            int recentIndex;
            
            /** Construct a file action.
             *
             * @param recentIndex index of recent file to load
             * @param keyCode identifies shortcut key for this action
             * @param modifiers modifiers for shortcut (SHIFT, META, etc.)
             */

            public LoadRecentAction(int recentIndex, int keyCode, int modifiers)
            {
               super(getRecentPlanName(recentIndex),
                     getRecentMenuDescription(recentIndex),
                     keyCode, 
                     modifiers);
               this.recentIndex = recentIndex;
            }
            
            /**
             * Called when user wishes to make visible this type of file.
             *
             * @param  e action event 
             */

            public void actionPerformed(ActionEvent e)
            {
               loadRecentPlan(recentIndex);
            }
      }

      /** Action to open and view a plan. */

      LuvAction openAction = new LuvAction(
         "Open Plan", "Open a plan for viewing.", VK_O, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  choosePlan();
               }
         };
      
      /** Action to load a script for Execution. */
      LuvAction openScriptAction = new LuvAction(
         "Open Script", "Open a script for execution.", VK_O, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  chooseScript();
               }
         };

      /** Action to test all plans in a directory. */

      LuvAction testAction = new LuvAction(
         "Test", "Load all plans in a directory testing purposes.", VK_T, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  test();
               }
         };

      /** Action to reload a plan. */

      LuvAction reloadAction = new LuvAction(
         "Reload Plan", "Reload current plan file.", VK_R, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  loadRecentPlan(1);
               }
         };

      /** Action to show the debugging window. */

      LuvAction debugWindowAction = new LuvAction(
         "Show Debug", "Show window with debug text.", VK_D, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  debugWindow.setVisible(!debugWindow.isVisible());
               }
         };

      /** Action to allow breakpoints. */
         
      LuvAction allowBreaksAction = new LuvAction(
         "Enable Breakpoints", "Select this to allow breakpoints.", VK_F2)
	 {
             public void actionPerformed(ActionEvent actionEvent)
             {
                 allowBreaks = !allowBreaks;
                 if (allowBreaks)
                     showStatus("Breaking ENabled.", Color.GREEN.darker());
                 else
                     showStatus("Breaking DISabled (Pressing F2 toggles breaking ability)", Color.RED);
             }
	 };
         
      /** Action to execute a plexil plan. */

      LuvAction execAction = new LuvAction(
         "Execute Plan", 
         "Execute plan currently loaded.",
         VK_F1)
         {
             public void actionPerformed(ActionEvent actionEvent)
             {
                try {
                    showStatus("Executing...", Color.GREEN.darker(), 1000);
                    executedViaLuvViewer = true;
                    runExecTest();                    
                } catch (IOException ex) {
                    System.err.println("Error: " + ex.getMessage());
                }
             }
      };

      LuvAction pauseAction = new LuvAction(
         "Pause or Resume plan", 
         "Pause or resume an executing plan, if it is blocking.",
         VK_SPACE)
         {
               public void actionPerformed(ActionEvent e)
               {            
                  planPaused = !planPaused;
                  showStatus((planPaused ? "Pause" : "Resume") + " requested.",
                             Color.BLACK, 1000);
               }
         };

      /** Action to step a paused plexil plan. */

      LuvAction stepAction = new LuvAction(
         "Step", 
         "Step a plan, pausing it if is not paused.",
         VK_ENTER)
         {
               public void actionPerformed(ActionEvent e)
               {
                  if (!planPaused)
                  {
                     planPaused = true;
                     showStatus("Step requested.", Color.BLACK, 1000);
                  }
                  else
                  {
                     planStep = true;
                     showStatus("Step plan.", Color.BLACK, 1000);
                  }
               }
         };

      /** Action show node types in different ways. */
      
      LuvAction showHidePrlNodes = new LuvAction(
         "Toggle Plexil Lisp Nodes", 
         "Show or hide nodes that start with \"plexillisp_\".",
         VK_P, META_MASK)
         {
               RegexModelFilter filter = 
                  new RegexModelFilter(
                     properties.getBoolean(PROP_VIEW_HIDE_PLEXILLISP),
                     "^plexilisp_.*", 
                     MODEL_NAME);

               {
                  filter.addListener(
                     new AbstractModelFilter.Listener()
                     {
                           @Override public void filterChanged(
                              AbstractModelFilter filter) 
                           {
                              resetView();
                           }
                     });
               }

               public void actionPerformed(ActionEvent e)
               {
                  filter.setEnabled(!filter.isEnabled());
                  properties.set(PROP_VIEW_HIDE_PLEXILLISP,
                                 filter.isEnabled());
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
                  for (IBreakPoint bp: breakPoints.keySet())
                     bp.setEnabled(true);
                  refreshView();
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
                  for (IBreakPoint bp: breakPoints.keySet())
                     bp.setEnabled(false);
                  refreshView();
               }
         };

      /** Action to remove all breakpoints. */

      LuvAction removeAllBreakpointsAction = new LuvAction(
         "Remove All Break Points", 
         "Permanently remove all breakpoint from the system.")
         {
               public void actionPerformed(ActionEvent e)
               {
                  removeAllBreakPoints();
               }
         };

      /** Action to exit the program. */

      LuvAction exitAction = new LuvAction(
         "Exit", "Terminate this program.", VK_ESCAPE)
         {
               public void actionPerformed(ActionEvent e)
               {
                  exit();
               }
         };
}
