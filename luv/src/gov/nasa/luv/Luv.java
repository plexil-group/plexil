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

import static gov.nasa.luv.Constants.*;
import gov.nasa.luv.Exec;
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
import static java.lang.System.*;
import static java.awt.BorderLayout.*;
import static java.awt.event.KeyEvent.*;
import static javax.swing.JFileChooser.*;

/**
 * The containing frame for the Lightweight UE Viewer.
 */

public class Luv extends JFrame
{           
      public static boolean allowBreaks, allowDebug, pauseAtStart, executedViaLuvViewer, isExecuting = false;
      public static File debug, plan, script;
      public FileHandler fh = new FileHandler();
      public Exec ex = new Exec();
      public static StatusBar statusBar = new StatusBar();
      public static BreakPointHandler breakPointHandler = new BreakPointHandler();
      
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

      Properties properties = new Properties(PROPERTIES_FILE_LOCATION);

      /** current view */

      View currentView;

      /** count of specifed libraries not yet loaded */

      int outstandingLibraryCount = 0;

      /** set of active libraries */

      Vector<Model> libraries = new Vector<Model>();

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
         runLuvViewer();
      }

      public static void runLuvViewer()
      {
         // if we're on a mac, us mac style menus
         
         System.setProperty("apple.laf.useScreenMenuBar", "true");

         // create instance of Luv
         
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
            fh.loadRecentPlan(1);
            
         allowBreaksAction.actionPerformed(null);

         // start the server listening for events
         
         Server s = new Server(properties.getInteger(PROP_NET_SERVER_PORT));
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
         fileMenu.add(new JSeparator());
         fileMenu.add(exitAction);

         // create and update run menu

         execMenu = new JMenu("Run");
         menuBar.add(execMenu);
         updateExecMenu();
         
         // add view menu

         menuBar.add(viewMenu);

         // show window menu

         JMenu windowMenu = new JMenu("Window");
         menuBar.add(windowMenu);
         windowMenu.add(debugWindowAction);         

         // add view panel

         frame.add(viewPanel, CENTER);

         // create the status bar

         final JLabel status = new JLabel(" ");
         status.setBorder(new EmptyBorder(2, 2, 2, 2));
         frame.add(status, SOUTH);
         statusBar.startStatusBar(status);
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

         if (breakPointHandler.breakPoints.size() > 0)
         {
            execMenu.add(new JSeparator());
            
            JMenu breakMenu = new JMenu("Break Points");
            execMenu.add(breakMenu);
            
            // add all the break points

            for (final IBreakPoint breakPoint: breakPointHandler.breakPoints.keySet())
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

               if (breakPointHandler.unfoundBreakPoints.contains(breakPoint))
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

         breakPointHandler.unfoundBreakPoints.clear();
         for (Map.Entry<IBreakPoint, ModelPath> pair: breakPointHandler.breakPoints.entrySet())
         {
            IBreakPoint breakPoint = pair.getKey();
            ModelPath path = pair.getValue();

            Model target = path.find(model);
            if (target != null)
            {
               breakPoint.setModel(target);
            }
            else
               breakPointHandler.unfoundBreakPoints.add(breakPoint);
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
          String planFile = "";
          String scriptFile = "";
          
          if (plan == null)
              planFile = model.getPlanName();
          else
              planFile = plan.getName();
         
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
                     breakPointHandler.createChangeBreakpoint(model);
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
                        breakPointHandler.createTargetPropertyValueBreakpoint(
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
                        breakPointHandler.createTargetPropertyValueBreakpoint(
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
                        breakPointHandler.createTargetPropertyValueBreakpoint(
                           model, name + " failure type", 
                           MODEL_FAILURE_TYPE, failureType);
                     }
               }); 

         // get the break points for this model
         
         final Vector<IBreakPoint> IbreakPoints = breakPointHandler.getBreakPoints(model);

         // if we got any add enabele/disabel & remove item for each one

         if (IbreakPoints.size() > 0)
         {
            // add the breakpoints

            popup.add(new JSeparator());
            for (final IBreakPoint IbreakPoint: IbreakPoints)
            {
               String action = IbreakPoint.isEnabled() ? "Disable" : "Enable";
               popup.add(new LuvAction(
                            action + " " + IbreakPoint,
                            action + " the breakpoint " + 
                            IbreakPoint + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           IbreakPoint.setEnabled(!IbreakPoint.isEnabled());
                           refreshView();
                        }
                  }); 
            }

            // add the breakpoints

            popup.add(new JSeparator());
            for (final IBreakPoint IbreakPoint: IbreakPoints)
            {
               popup.add(new LuvAction(
                            "Remove " + IbreakPoint,
                            "Permanently remove the breakpoint " + 
                            IbreakPoint + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           breakPointHandler.removeBreakPoint(IbreakPoint);
                        }
                  }); 
            }
         }

         // if there is more then one break point add a remove all item

         if (IbreakPoints.size() > 1)
         {
            // add the remove all action
            
            popup.add(new JSeparator());
            popup.add(new LuvAction(
                         "Remove All Break Points From " + name,
                         "Permanently remove all breakpoints from " + name + ".")
               {
                     public void actionPerformed(ActionEvent e)
                     {
                        for (final IBreakPoint IbreakPoint: IbreakPoints)
                           breakPointHandler.removeBreakPoint(IbreakPoint);
                        
                        updateExecMenu();
                     }
               });
         }

         // return our freshly created popup menu

         return popup;
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
                        fh.chooseLibrary();
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

               Vector<String> tmpLibs = fh.getRecentLibNames(i, true);

               // replace them with these library names

               int libIndex = 1;
               for (String library: libraries)
                  fh.setRecentLibName(i, libIndex++, library);
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
      
      /** Given a recent plan index, the description used for the recent
       * menu item tooltip. 
       *
       * @param recentIndex the index of the recent plan
       *
       * @return the description of what gets loaded
       */

      public String getRecentMenuDescription(int recentIndex)
      {
         File plan = new File(fh.getRecentPlanName(recentIndex));
         StringBuffer description = new StringBuffer("Load " + plan.getName());

         for (String libName: fh.getRecentLibNames(recentIndex, false))
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
            if (fh.getRecentPlanName(i + 1) != null)
               recentPlanMenu.add(
                  new LoadRecentAction(i + 1, '1' + i, META_MASK));

         // this menu is only enabled when there are items in it
         
         recentPlanMenu.setEnabled(recentPlanMenu.getMenuComponentCount() > 0);
      }      

      /**
       * Exit this program.
       */

      public void exit()
      {
         System.exit(0);
      }

      /** Exception used to signal that the user canceled a link. */

      public class LinkCanceledException extends Exception
      {
            LinkCanceledException(String nodeId)
            {
               super("Link canceled at node: " + nodeId);
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
               super(fh.getRecentPlanName(recentIndex),
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
               fh.loadRecentPlan(recentIndex);
            }
      }

      /** Action to open and view a plan. */

      LuvAction openAction = new LuvAction(
         "Open Plan", "Open a plan for viewing.", VK_O, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  fh.choosePlan();
               }
         };
      
      /** Action to load a script for Execution. */
      LuvAction openScriptAction = new LuvAction(
         "Open Script", "Open a script for execution.", VK_O, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  fh.chooseScript();
               }
         };

      /** Action to reload a plan. */

      LuvAction reloadAction = new LuvAction(
         "Reload Plan", "Reload current plan file.", VK_R, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  fh.loadRecentPlan(1);
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
                     statusBar.showStatus("Breaking ENabled.", Color.GREEN.darker());
                 else
                     statusBar.showStatus("Breaking DISabled (Pressing F2 toggles breaking ability)", Color.RED);
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
                    if (!planPaused && !isExecuting)
                    {
                        executedViaLuvViewer = true;
                        statusBar.showStatus("Executing...", Color.GREEN.darker(), 1000);
                        ex.runExec();
                    }
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
                   if (allowBreaks)
                   {
                       planPaused = !planPaused;
                       statusBar.showStatus((planPaused ? "Pause" : "Resume") + " requested.", Color.BLACK, 1000);
                   }
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
                     statusBar.showStatus("Step requested.", Color.BLACK, 1000);
                  }
                  else
                  {
                     planStep = true;
                     statusBar.showStatus("Step plan.", Color.BLACK, 1000);
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
                  for (IBreakPoint bp: breakPointHandler.breakPoints.keySet())
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
                  for (IBreakPoint bp: breakPointHandler.breakPoints.keySet())
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
                  breakPointHandler.removeAllBreakPoints();
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
                       exit();
               }
         };
}
