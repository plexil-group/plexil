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

import java.io.IOException;
import javax.swing.JFrame;
import javax.swing.JMenuBar;
import javax.swing.JMenu;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JSeparator;
import javax.swing.JScrollPane;
import javax.swing.border.EmptyBorder;
import javax.swing.JPopupMenu;
import javax.swing.ToolTipManager;
import java.util.Map;
import java.util.Vector;
import java.awt.Container;
import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;
import java.io.File;
import static java.awt.BorderLayout.*;
import static java.awt.event.KeyEvent.*;
import static javax.swing.JFileChooser.*;
import javax.swing.ImageIcon;

public class LuvViewerHandler extends JFrame 
{
      Container frame;
      
      JMenuBar menuBar;
      
      JMenu fileMenu;
      JMenu debugWindowMenu;
      JMenu recentPlanMenu = new JMenu(RECENT_PLANS);
      JMenu runMenu;
      JMenu viewMenu;
      
      View currentView;
      
      JPanel viewPanel = new JPanel();
      
      Views views = new Views();
      
      DebugWindow debugWindow;
      
      public LuvViewerHandler() {}
      
      public void startConstructing() throws IOException
      {
          constructGUI();
          
          constructDebugWindow();
          
          // app exits when frame is closed
          
          setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    
          // things to do when app is closed
          
          saveGUIProperties();
          
          // when this frame get's focus, get it's menu bar back from the debug window
          
          addWindowFocusListener(new WindowAdapter()
            {
                @Override
                  public void windowGainedFocus(WindowEvent e)
                  {
                     JMenuBar mb = debugWindow.getJMenuBar();
                     if (mb != null) setJMenuBar(mb);
                  }
            });
         
         // make the frame visible
         
         pack();
         setVisible(true);
    }
    
      /**
       * Place all visible elements into the container in the main frame
       * of the application.
       *
       * @param frame the container in witch to place items.
       */

      public void constructGUI()
      {    
          frame = getContentPane();
          
          constructBase();
          constructMenus();  
          constructPanel();  
          constructStatusBar();
          
          setTitle();
          
          // set tooltip display time

          ToolTipManager.sharedInstance().setDismissDelay(Luv.getLuv().properties.getInteger(PROP_TOOLTIP_DISMISS));
                   
          // set size and location of frame

          setLocation(Luv.getLuv().properties.getPoint(PROP_WIN_LOC));
          setPreferredSize(Luv.getLuv().properties.getDimension(PROP_WIN_SIZE)); 
      }
      
      public void constructDebugWindow() throws IOException
      {
          // create the debug window
          
          debugWindow = new DebugWindow(this);
          
          // set size and location of frame
          
          debugWindow.setLocation(Luv.getLuv().properties.getPoint(PROP_DBWIN_LOC));
          debugWindow.setPreferredSize(Luv.getLuv().properties.getDimension(PROP_DBWIN_SIZE));
          debugWindow.pack();
          
          // make debug window visible
          
          debugWindow.setVisible(Luv.getLuv().properties.getBoolean(PROP_DBWIN_VISIBLE));
      }
      
      public void constructBase()
      {
         setLayout(new BorderLayout());
         setBackground(Luv.getLuv().properties.getColor(PROP_WIN_BCLR));
      }

      public void constructMenus()
      {
         menuBar = new JMenuBar();
         setJMenuBar(menuBar);
         
         constructFileMenu();
         constructRunMenu();
         constructViewMenu();
         constructDebugWindowMenu();
      }
      
      public void constructPanel()
      {
          frame.add(viewPanel, CENTER);
      }
      
      public void constructStatusBar()
      {
         final JLabel status = new JLabel(BLANK);
         status.setBorder(new EmptyBorder(2, 2, 2, 2));
         frame.add(status, SOUTH);
         Luv.statusBarHandler.startStatusBar(status);
      }
      
      public void constructFileMenu()
      {
         fileMenu = new JMenu(FILE);
         menuBar.add(fileMenu);
         fileMenu.add(Luv.actionHandler.openAction);
         fileMenu.add(Luv.actionHandler.openScriptAction);
         updateRecentMenu();
         
         fileMenu.add(recentPlanMenu);
         fileMenu.add(Luv.actionHandler.reloadAction);
         fileMenu.add(new JSeparator());
         fileMenu.add(Luv.actionHandler.exitAction);
      }
      
      public void constructRunMenu()
      {
         runMenu = new JMenu(RUN);
         menuBar.add(runMenu);
         updateRunMenu();          
      }
      
      public void constructViewMenu()
      {
         viewMenu = new JMenu(VIEW);
         menuBar.add(viewMenu);          
      }
      
      public void constructDebugWindowMenu()
      {
         debugWindowMenu = new JMenu(DEBUG_WINDOW);
         menuBar.add(debugWindowMenu);
         debugWindowMenu.add(Luv.actionHandler.debugWindowAction);          
      }
      
      public void saveGUIProperties() 
      {
          Runtime.getRuntime().addShutdownHook(new Thread()
            {
            @Override
                  public void run()
                  {
                     Luv.getLuv().properties.set(PROP_WIN_LOC, getLocation());
                     Luv.getLuv().properties.set(PROP_WIN_SIZE, getSize());
                     Luv.getLuv().properties.set(PROP_DBWIN_LOC, debugWindow.getLocation());
                     Luv.getLuv().properties.set(PROP_DBWIN_SIZE, debugWindow.getSize());
                     Luv.getLuv().properties.set(PROP_DBWIN_VISIBLE, debugWindow.isVisible());
                     currentView.setViewProperties(Luv.getLuv().properties);
                  }
            }); 
      }
       
      /** Update the state of the exec menu */

      public void updateRunMenu()
      {
         // clear any stuff from exec menu

         runMenu.removeAll();

         // add pause and step

         runMenu.add(Luv.actionHandler.pauseAction);
         runMenu.add(Luv.actionHandler.stepAction);
         runMenu.add(Luv.actionHandler.allowBreaksAction);
         runMenu.add(Luv.actionHandler.execAction);
         

          // add break point menu

         if (Luv.breakPointHandler.breakPoints.size() > 0)
         {
            runMenu.add(new JSeparator());
            
            JMenu breakMenu = new JMenu(BREAK_POINTS);
            runMenu.add(breakMenu);
            
            // add all the break points

            for (final BreakPoint breakPoint: Luv.breakPointHandler.breakPoints.keySet())
            {
               String actionStr = breakPoint.isEnabled() ? DISABLE : ENABLE;
               LuvAction action = new LuvAction(
                  actionStr + " " + breakPoint,
                  actionStr + " the breakpoint " + 
                  breakPoint + ".")
                  {
                        public void actionPerformed(ActionEvent e)
                        {
                           breakPoint.setEnabled(!breakPoint.isEnabled());
                           updateRunMenu();
                           refreshView();
                        }
                  };
               
               breakMenu.add(action);

               if (Luv.breakPointHandler.unfoundBreakPoints.contains(breakPoint))
                  action.setEnabled(false);
            }

            // add enabled disable all items

            breakMenu.add(new JSeparator());
            breakMenu.add(Luv.actionHandler.enableBreakpointsAction);
            breakMenu.add(Luv.actionHandler.disableBreakpointsAction);

            // add enabled remove all items

            breakMenu.add(new JSeparator());
            breakMenu.add(Luv.actionHandler.removeAllBreakpointsAction);
         }
      }

      /** Reset the currevnt view to refect the changes in the world. */

      public void resetView()
      {
         // create a new instance of the view

         setView(new TreeTableView("", Luv.getLuv(), Luv.getLuv().model));

         // map all the breakpoints into the new model

         Luv.breakPointHandler.unfoundBreakPoints.clear();
         for (Map.Entry<BreakPoint, ModelPath> pair: Luv.breakPointHandler.breakPoints.entrySet())
         {
            BreakPoint breakPoint = pair.getKey();
            ModelPath path = pair.getValue();

            Model target = path.find(Luv.getLuv().model);
            if (target != null)
            {
               breakPoint.setModel(target);
            }
            else
               Luv.breakPointHandler.unfoundBreakPoints.add(breakPoint);
         }

         // update the exec menu

         updateRunMenu();
      }

      /** Refresh the current view */

      public void refreshView()
      {
         ((Container)currentView).repaint();
         updateRunMenu();
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
            currentView.setViewProperties(Luv.getLuv().properties);
         currentView = (View)view;
         currentView.getViewProperties(Luv.getLuv().properties);

         // clear out the view panel and put the new view in there

         viewPanel.removeAll();
         viewPanel.setLayout(new BorderLayout());
         JScrollPane sp = new JScrollPane(view);
         sp.setBackground(Luv.getLuv().properties.getColor(PROP_WIN_BCLR));
         viewPanel.add(sp, CENTER);

         // insert the view menu items

         viewMenu.removeAll();
         for(LuvAction action: currentView.getViewActions())
            viewMenu.add(action);
         viewMenu.add(Luv.actionHandler.showHidePrlNodes);


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

      /** Set the title of the application. */

      public void setTitle()
      {
         /* if (Luv.getLuv().properties.getProperty(PLAN_FILENAME) != null)
          {
              String planFile = BLANK;
              String scriptFile = BLANK;
              
              planFile = Luv.getLuv().properties.getProperty(PLAN_FILENAME);
              
              if (Luv.script != null)
                  scriptFile = SCRIPT_TITLE + Luv.script.getName();
              if (planFile != null)
              {
                  String plan = Luv.fileHandler.stripDotXml(new File(planFile).getName());
                  StringBuffer title = new StringBuffer(plan);

                  for (String libName: Luv.getLuv().model.getLibraryNames())
                      title.append(" + " + Luv.fileHandler.stripDotXml(new File(libName).getName()));

                  setTitle(PLAN_TITLE + title.toString() + scriptFile);
             }
          }
          else*/
              setTitle(DEFAULT_TITLE);
      }
      
      /** Update the recently loaded files menu. */

      public void updateRecentMenu()
      {
         recentPlanMenu.removeAll();
         int count = Luv.getLuv().properties.getInteger(PROP_FILE_RECENT_COUNT);
         for (int i = 0; i < count; ++i)
            if (Luv.fileHandler.getRecentPlanName(i + 1) != null)
                recentPlanMenu.add(new LoadRecentAction(i + 1, '1' + i, META_MASK));

         // this menu is only enabled when there are items in it
         
         recentPlanMenu.setEnabled(recentPlanMenu.getMenuComponentCount() > 0);
      }  
      
      
      /** Add a file to the recently open files list. */

      public void addRecent()
      {
         // put newest file at the top of the list

         String current = Luv.getLuv().model.getPlanName();
         Vector<String> libraries = Luv.getLuv().model.getLibraryNames();

         String filename = current;
         int count = Luv.getLuv().properties.getInteger(PROP_FILE_RECENT_COUNT);
         for (int i = 1; i <= count && filename != null; ++i)
         {
            if (filename != null)
            {
               // get (and remove) the old library names at this index

               Vector<String> tmpLibs = Luv.fileHandler.getRecentLibNames(i, true);

               // replace them with these library names

               int libIndex = 1;
               for (String library: libraries)
                  Luv.fileHandler.setRecentLibName(i, libIndex++, library);
               libraries = tmpLibs;

               filename = (String)Luv.getLuv().properties
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
         File plan = new File(Luv.fileHandler.getRecentPlanName(recentIndex));
         StringBuffer description = new StringBuffer("Load " + plan.getName());

         for (String libName: Luv.fileHandler.getRecentLibNames(recentIndex, false))
            description.append(" + " + new File(libName).getName());
         
         description.append(".");
         return description.toString();
      } 
}
