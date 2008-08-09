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
import java.io.File;
import java.io.IOException;
import static java.lang.System.*;

/**
 * The containing frame for the Lightweight UE Viewer.
 */

public class Luv
{           
      public static boolean resetAllowBreaks = false; 
      public static boolean allowBreaks = false;
      public static boolean allowDebug = false;
      public static boolean pauseAtStart = false;
      public static boolean executedViaLuvViewer = false;
      public static boolean isExecuting = false;    
      
      /** is the plan current paused */

      public static boolean planPaused = false;

      /** should the plan take one step */

      public static boolean planStep = false;
      
      public static File debug; 
      public static File plan; 
      public static File script;
      
      public static FileHandler         fileHandler = new FileHandler();
      public static ExecutionHandler    executionHandler = new ExecutionHandler();
      public static StatusBarHandler    statusBarHandler = new StatusBarHandler();
      public static BreakPointHandler   breakPointHandler = new BreakPointHandler();
      public static LuvViewerHandler    luvViewerHandler = new LuvViewerHandler();
      public static LibraryHandler      libraryHandler = new LibraryHandler();
      public static NodePopupMenuHandler nodePopupMenuHandler = new NodePopupMenuHandler();
      public static ActionHandler       actionHandler;
      
      
      
      /** the current model being displayed */

      final Model model = new Model(DUMMY)
         {
               {
                  setProperty(MODEL_NAME, ROOT);
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

      /** count of specifed libraries not yet loaded */

      int outstandingLibraryCount = 0;

      /** the current working instance of luv */

      private static Luv theLuv;

      /**
       * Entry point for this program.
       */

      public static void main(String[] args)
      {
         runLuv();
      }

      public static void runLuv()
      {
         // if we're on a mac, us mac style menus
         
         System.setProperty(PROP_MAC_MENU_STYLE, TRUE);

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
      
      public Luv() throws IOException
      {
         // record instance of self

         theLuv = this;
         
         actionHandler = new ActionHandler();

         // construct the GUI
         
         luvViewerHandler.startConstructing();

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
            fileHandler.loadRecentPlan(1);
            
         actionHandler.allowBreaksAction.actionPerformed(null);

         // start the server listening for events
         
         Server s = new Server(properties.getInteger(PROP_NET_SERVER_PORT));
      }

      /** Get the current active instance of luv.
       *
       * @return current luv instance.
       */

      public static Luv getLuv()
      {
         return theLuv;
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
       * Exit this program.
       */

      public void exit()
      {
         System.exit(0);
      }
}
