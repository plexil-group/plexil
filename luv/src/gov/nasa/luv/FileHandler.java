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

import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;
import javax.swing.JOptionPane;

import java.util.Vector;

import java.io.File;
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.InputStream;

import org.xml.sax.*;
import org.xml.sax.helpers.XMLReaderFactory;

import static java.lang.System.*;
import static javax.swing.JFileChooser.*;

public class FileHandler
{       
      public FileHandler () {}

      /** count of specifed libraries not yet loaded */

      int outstandingLibraryCount = 0;

      /** set of active libraries */

      Vector<Model> libraries = new Vector<Model>();

      /** file chooser object */
      
      public JFileChooser fileChooser = new JFileChooser()
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
      
      /** Locates or Creates the appropriate script file.
       *
       * @param scriptName string that represents the script filename
       * @param planName string that represents the plan filename
       * @param path string that represents the absolute path to where scripts are located
       */

      public File searchScript(String scriptName, String planName, String path) throws IOException 
      {
          try 
          {
              planName = planName.substring(0, planName.indexOf('.'));
              
              //PLAN_script.plx
              String name1 = planName + "_script.plx";
              Luv.script = new File(path + name1);
              if (!Luv.script.canRead())
              {
                  //script-PLAN.plx
                  String name2 = "script-" + planName + ".plx";
                  Luv.script = new File(path + name2);
                  if (!Luv.script.canRead())
                  {
                      //script_PLAN.plx
                      String name3 = "script_" + planName + ".plx";
                      Luv.script = new File(path + name3);
                      if (!Luv.script.canRead())
                      {
                          return null;
                      }
                  }
              }
          }
          catch (Exception e)
          {
            out.println("Error: " + e.getMessage());
          } 
          
          return Luv.script;
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
         Luv.statusBarHandler.showStatus("Loading "  + file);
         try
         {
            parseXml(new FileInputStream(file), model);
         }
         catch(Exception e)
         {
            JOptionPane.showMessageDialog(
               Luv.luvViewerHandler,
               "Error loading plan: " + file.getName() + 
               "  See debug window for details.",
               "Parse Error",
               JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
         }
         Luv.statusBarHandler.clearStatus();
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
               Luv.luvViewerHandler,
               "Error parsing XML message.  See debug window for details.",
               "Parse Error",
               JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
         }

         return isPlan;
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
               new File(Luv.getLuv().properties.getString(PROP_FILE_RECENT_SCRIPT_DIR)));
            if (fileChooser.showOpenDialog(Luv.luvViewerHandler) == APPROVE_OPTION)
            {
               File script = fileChooser.getSelectedFile();
               Luv.getLuv().properties.set(PROP_FILE_RECENT_SCRIPT_DIR, script.getParent());
               Luv.getLuv().properties.set(PROP_FILE_RECENT_SCRIPT_BASE + PROP_RECENT_FILE, script.toString());
               Luv.getLuv().model.addScriptName(script.toString());              
               Luv.script = script;
               Luv.luvViewerHandler.resetView();
            }
         }
         catch(Exception e)
         {
            e.printStackTrace();
         }
      }
         
      /**
       * Select and load a plexil plan from the disk.  This operates on the global model.
       */
      
      public void choosePlan()
      {
         try
         {
             Luv.isExecuting = false;
            fileChooser.setCurrentDirectory(
               new File(Luv.getLuv().properties.getString(PROP_FILE_RECENT_PLAN_DIR)));
            if (fileChooser.showOpenDialog(Luv.luvViewerHandler) == APPROVE_OPTION)
            {
               File plan = fileChooser.getSelectedFile();
               Luv.getLuv().properties.set(PROP_FILE_RECENT_PLAN_DIR, plan.getParent());
               Luv.script = null;
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
         Luv.getLuv().model.clear();
         Luv.getLuv().model.addPlanName(plan.toString());
         Luv.getLuv().model.setProperty(VIEWER_BLOCKS, FALSE);
         
         readPlan(Luv.getLuv().model, plan);
         if (libraryNames != null)
            for (String libraryName: libraryNames)
               loadLibrary(new File(libraryName));
         try
         {
            Luv.libraryHandler.link(Luv.getLuv().model, libraries);
         }
         catch (LinkCanceledException lce) {}
         Luv.luvViewerHandler.resetView();
         Luv.luvViewerHandler.addRecent();
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
            String recent = Luv.getLuv().properties.getString(PROP_FILE_RECENT_LIB_DIR);
            if (recent == null)
               recent = Luv.getLuv().properties.getString(PROP_FILE_RECENT_PLAN_DIR);
            fileChooser.setCurrentDirectory(new File(recent));
            if (fileChooser.showOpenDialog(Luv.luvViewerHandler) == APPROVE_OPTION)
            {
               File library = fileChooser.getSelectedFile();
               Luv.getLuv().properties.set(PROP_FILE_RECENT_LIB_DIR, library.getParent());
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
         Luv.getLuv().model.addLibraryName(libraryFile.toString());
         Model library = new Model("a library");
         readPlan(library, libraryFile);
         libraries.add(library.findChild(NODE));
      }

      /** Load a recently loaded plan.
       *
       * @param index index of recently loaded plan
       */

      public void loadRecentPlan(int index)
      {
         Luv.isExecuting = false;
         String planName = getRecentPlanName(index);
         String scriptName = getRecentScriptName(index);
         if (planName != null)
         {
            loadPlan(new File(planName), getRecentLibNames(index, false));
            if (scriptName != null)
                Luv.script = new File(scriptName);
            Luv.luvViewerHandler.resetView();
         }
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
         String prop = Luv.getLuv().properties.getProperty(name);
         if (remove)
            Luv.getLuv().properties.remove(name);
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
         return (String)Luv.getLuv().properties.setProperty(name, libName);
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
         return Luv.getLuv().properties.getProperty(PROP_FILE_RECENT_PLAN_BASE + recentIndex);
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
         return Luv.getLuv().properties.getProperty(PROP_FILE_RECENT_SCRIPT_BASE + recentIndex);
      } 
      

      public String stripDotXml(String filename)
      {
         return filename.split(".xml")[0];
      }
      
      public void createEmptyScript(String path) throws IOException 
      {
          JOptionPane.showMessageDialog(Luv.luvViewerHandler, "Unable to locate a script for this plan. \n\nAn empty script has been created for you:\n\n"
                  + Luv.getLuv().properties.getProperty(PROP_FILE_RECENT_PLAN_DIR) + 
                  System.getProperty(PROP_FILE_SEPARATOR) + DEFAULT_SCRIPT_NAME);
          String scriptName = path + DEFAULT_SCRIPT_NAME;
          FileWriter emptyScript = new FileWriter(scriptName);
          BufferedWriter out = new BufferedWriter(emptyScript);
          out.write(EMPTY_SCRIPT);
          out.close();                          
          Luv.script = new File(scriptName);
          Luv.getLuv().properties.set(PROP_FILE_RECENT_SCRIPT_BASE + PROP_RECENT_FILE, Luv.script.getAbsolutePath());
      }
}