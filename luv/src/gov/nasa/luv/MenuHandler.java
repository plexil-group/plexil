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
import javax.swing.JMenu;
import javax.swing.JSeparator;
import java.util.Vector;

import java.io.File;

import static gov.nasa.luv.Constants.*;
import static java.awt.event.KeyEvent.*;

public class MenuHandler extends JFrame
{
    
      private JMenu fileMenu                = new JMenu("File");  
      private JMenu recentFileMenu          = new JMenu("Recent Files");
      private JMenu runMenu                 = new JMenu("Run");   
      private JMenu viewMenu                = new JMenu("View");
      private JMenu windowMenu              = new JMenu("Windows");
      
      public void createMenuBar(JMenuBar menuBar)
      {       
         // create file menu
         
         menuBar.add(fileMenu);
         fileMenu.add(Luv.getLuv().openAction);
         fileMenu.add(Luv.getLuv().openScriptAction);
         updateRecentMenu();
         fileMenu.add(recentFileMenu);
         fileMenu.add(Luv.getLuv().reloadAction);
         fileMenu.add(new JSeparator());
         fileMenu.add(Luv.getLuv().exitAction);

         // create and update exec menu
         
         menuBar.add(runMenu);
         updateRunMenu();

         // add view menu

         menuBar.add(viewMenu);

         // show window menu
 
         menuBar.add(windowMenu);
         windowMenu.add(Luv.getLuv().conditionsAction);
         windowMenu.add(Luv.getLuv().luvDebugWindowAction);
      }
      
      public JMenu getMenu(int menu)
      {
          switch (menu)
          {
              case FILE_MENU:
                  return fileMenu;
              case RECENT_FILE_MENU:
                  return recentFileMenu;
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
                  recentFileMenu.removeAll();
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
          
          if (windowMenu.getItemCount() > 0)
              windowMenu.getItem(SHOW_LUV_DEBUG_MENU_ITEM).setEnabled(false);
          windowMenu.setEnabled(false);
      }
      
      /** Update the state of the exec menu */

      public void updateRunMenu()
      {
         // clear any stuff from exec menu

         runMenu.removeAll();

         // add pause and step
         
         runMenu.add(Luv.getLuv().pauseAction);
         runMenu.add(Luv.getLuv().stepAction);
         
         if (!Luv.getLuv().getBoolean(ALLOW_BREAKS))
         {
             runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
             runMenu.getItem(STEP_MENU_ITEM).setEnabled(false);
         }
         
         if (Luv.getLuv().getBoolean(STOPPED_EXECUTION))
             runMenu.getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(false);
         
         if (Luv.getLuv().getBoolean(ALLOW_BREAKS))
             Luv.getLuv().allowBreaksAction.putValue(NAME, DISABLE_BREAKS);
         else
             Luv.getLuv().allowBreaksAction.putValue(NAME, ENABLE_BREAKS);

         runMenu.add(Luv.getLuv().allowBreaksAction);     
         runMenu.add(Luv.getLuv().execAction);
         
         if (!Luv.getLuv().getBoolean(IS_EXECUTING))
         {
             runMenu.getItem(BREAK_MENU_ITEM).setEnabled(true);
             runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(true);
         }
         else
         {
             runMenu.getItem(BREAK_MENU_ITEM).setEnabled(false);
             runMenu.getItem(EXECUTE_MENU_ITEM).setEnabled(false);
         }

          // add break point menu

         if (Luv.getLuv().getLuvBreakPointHandler().getBreakPointMap().size() > 0)
         {            
            Luv.getLuv().getLuvBreakPointHandler().addBreakMenu();
         }
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

         for (String libName: Luv.getLuv().getLibraryHandler().getRecentLibNames(recentIndex, false))
            description.append(" + " + new File(libName).getName());
         
         description.append(".");
         return description.toString();
      }
      
            // Add a file to the recently opened file list. 

      public void addRecent()
      {
         // put newest file at the top of the list

         String current = Luv.getLuv().getModel().getPlanName();
         Vector<String> libraries = Luv.getLuv().getModel().getLibraryNames();

         String filename = current;
         int count = Luv.getLuv().getProperties().getInteger(PROP_FILE_RECENT_COUNT);
         for (int i = 1; i <= count && filename != null; ++i)
         {
            if (filename != null)
            {
               // get (and remove) the old library names at this index

               Vector<String> tmpLibs = Luv.getLuv().getLibraryHandler().getRecentLibNames(i, true);

               // replace them with these library names

               int libIndex = 1;
               for (String library: libraries)
                  Luv.getLuv().getLibraryHandler().setRecentLibName(i, libIndex++, library);
               libraries = tmpLibs;

               filename = (String)Luv.getLuv().getProperties().setProperty(PROP_FILE_RECENT_PLAN_BASE + i, filename);

               // if this file already existed in the list, we can stop
               // it already appears at the top

               if (filename != null && filename.equals(current))
                  break;
            }
         }

         // update the recent menu

         updateRecentMenu();
      }
      
      /** Update the recently loaded files menu. */

      public void updateRecentMenu()
      {
         recentFileMenu.removeAll();
         int count = Luv.getLuv().getProperties().getInteger(PROP_FILE_RECENT_COUNT);
         for (int i = 0; i < count; ++i)
            if (Luv.getLuv().getFileHandler().getRecentPlanName(i + 1) != null)
               recentFileMenu.add(new LoadRecentAction(i + 1, '1' + i, META_MASK));

         // this menu is only enabled when there are items in it
         
         recentFileMenu.setEnabled(recentFileMenu.getMenuComponentCount() > 0);
      }  

}
