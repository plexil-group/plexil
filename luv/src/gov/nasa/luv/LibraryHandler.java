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

import javax.swing.JOptionPane;

import java.util.Vector;

import java.io.File;

import static gov.nasa.luv.Constants.*;
import static java.lang.System.*;
import static javax.swing.JFileChooser.*;

public class LibraryHandler 
{   
      private Vector<Model> libraries = new Vector<Model>();        
      private Vector<String> callNameList = new Vector<String>(); 
      
      public String getLibraryNames()
      {
          String libraryNames = "";
              
          for (int i = 0; i < callNameList.size(); i++)
          {
              libraryNames += " -l " + callNameList.elementAt(i) + ".plx";
          }
          
          return libraryNames;          
      }
      
      public void clearCallNameList()
      {
          callNameList.clear();
      }
      
      public void clearLibraries()
      {
          libraries.clear();
      }
      
      public void resetLibraryInfo()
      {
          libraries.clear();
          libraries = new Vector<Model>();
          callNameList.clear();
          callNameList = new Vector<String>();
      }
      
      /** Link a plan with it's provided libraries 
       *
       * @param node current node of model being linked
       * @param libraries the collected libraries which will linked into
       * the plan
       *
       * @return false if all linking canceled by user, this does NOT mean that the 
       */

      public boolean linkLibraries(Model node) 
      {
         // if this is a library node call, link that node
         
         String type = node.getProperty(NODETYPE_ATTR);
         if (type != null && type.equals(LIBRARYNODECALL))
         {
            boolean foundLibrary = false;
            boolean retry = true;
            String callName = node.getProperty(MODEL_LIBRARY_CALL_ID); 
            
            if (!callNameList.contains(callName))
            {
                callNameList.add(callName);
                do
                {
                   // look through the libraries for match
                    
                    for (Model libCheck: libraries)
                    {
                        if(libCheck.getProperty(MODEL_NAME).equals(callName))
                        {
                            retry = false;
                            foundLibrary = true;
                            break;
                        }
                    }

                    if (foundLibrary)
                        break;

                    File library;
                    if (Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_LIB_DIR) != null)
                        library = new File(Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_LIB_DIR) + System.getProperty(PROP_FILE_SEPARATOR) + callName + ".plx");
                    else
                        library = new File(Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_DIR) + System.getProperty(PROP_FILE_SEPARATOR) + callName + ".plx");

                    if (!library.exists())
                    {
                        String newPath = library.getAbsolutePath();
                        newPath = newPath.replace(".plx", ".xml");
                        library = new File(newPath);
                        if(library.exists())
                        {
                            Luv.getLuv().getProperties().set(PROP_FILE_RECENT_LIB_DIR, library.getParent());
                            loadLibrary(library);
                            retry = false;
                            break;
                        }
                    }
                    else
                    {
                        Luv.getLuv().getProperties().set(PROP_FILE_RECENT_LIB_DIR, library.getParent());
                        loadLibrary(library);
                        retry = false;
                        break;                    
                    }

                   // if we didn't make the link, ask user for library

                   if (retry)
                   {
                      // option

                      Object[] options = 
                         {
                            "I will locate library",
                            "Do not load this library",
                            "Cancel plan loading",
                         };

                      // show the options

                      Luv.getLuv().getStatusMessageHandler().showStatus("Unable to locate the \"" + callName + "\" library\n\n", 1000);
                      int result = JOptionPane.showOptionDialog(
                         Luv.getLuv(),
                         "Unable to locate the \"" + callName + "\" library.\n\n" +
                         "What do you want to do?\n\n",
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
                             Luv.getLuv().getViewHandler().resetView();
                             Luv.getLuv().getMenuHandler().addRecent();
                             libraries.clear();
                             retry = false;
                             Luv.getLuv().setBoolean(STOP_SRCH_LIBS, true);
                             break;

                         // if the user doesn't want to load any libraries,
                         // halt the link operation now

                         case 2:
                             Luv.getLuv().setLuvViewerState(START_STATE);
                             retry = false;
                             Luv.getLuv().setBoolean(STOP_SRCH_LIBS, true);
                             break;
                      }
                   }
                }
                while (retry);           
            }
         }

         // if this is node, traverse into any children it might have,
         // note that this could be a node could have newly minted
         // children from the linking action above

         if (Luv.getLuv().getBoolean(STOP_SRCH_LIBS))
             return false;
         
         boolean success = true;

         for (Model child: node.getChildren())
         {
            if (!linkLibraries(child))
               success = false;
            if (Luv.getLuv().getBoolean(STOP_SRCH_LIBS))
                break;
         }


         return success;
         
      }

      /**
       * Select and load a plexil library from the disk.  The library is
       * added to the set of global libraries.
       */
      
      public void chooseLibrary()
      {
         try
         {
            String recent = Luv.getLuv().getProperties().getString(PROP_FILE_RECENT_LIB_DIR);
            if (recent == null)
               recent = Luv.getLuv().getProperties().getString(PROP_FILE_RECENT_PLAN_DIR);
            Luv.getLuv().getFileHandler().fileChooser.setCurrentDirectory(new File(recent));
            if (Luv.getLuv().getFileHandler().fileChooser.showOpenDialog(Luv.getLuv()) == APPROVE_OPTION)
            {
               File library = Luv.getLuv().getFileHandler().fileChooser.getSelectedFile();
               Luv.getLuv().getProperties().set(PROP_FILE_RECENT_LIB_DIR, library.getParent());
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
         Luv.getLuv().getModel().addLibraryName(libraryFile.toString());
         Model library = new Model("a library");
         Luv.getLuv().getFileHandler().readPlan(library, libraryFile);
         libraries.add(library.findChild(NODE));
      }
      
      public void unfoundLibrary(String callName)
      {
          boolean retry = true;
          do
          {

               // if we didn't make the link, ask user for library

               if (retry)
               {
                  // option

                  Object[] options = 
                     {
                        "I will locate library",
                        "Do not load this library",
                        "Cancel plan loading",
                     };

                  // show the options

                  Luv.getLuv().getStatusMessageHandler().showStatus("Unable to locate the \"" + callName + "\" library\n\n", 1000);
                  int result = JOptionPane.showOptionDialog(
                     Luv.getLuv(),
                     "Unable to locate the \"" + callName + "\" library.\n\n" +
                     "What do you want to do?\n\n",
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
                         Luv.getLuv().getViewHandler().resetView();
                         Luv.getLuv().getMenuHandler().addRecent();
                         libraries.clear();
                         retry = false;
                         Luv.getLuv().setBoolean(STOP_SRCH_LIBS, true);
                         break;

                     // if the user doesn't want to load any libraries,
                     // halt the link operation now

                     case 2:
                         Luv.getLuv().setLuvViewerState(START_STATE);
                         retry = false;
                         Luv.getLuv().setBoolean(STOP_SRCH_LIBS, true);
                         break;
                  }
               }
            } while (retry); 
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
         String prop = Luv.getLuv().getProperties().getProperty(name);
         if (remove)
            Luv.getLuv().getProperties().remove(name);
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
         return (String)Luv.getLuv().getProperties().setProperty(name, libName);
      }
}
