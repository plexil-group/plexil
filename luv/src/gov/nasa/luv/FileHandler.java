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

import java.io.FileInputStream;
import java.io.InputStream;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;
import javax.swing.JOptionPane;
import org.xml.sax.InputSource;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;

import java.util.HashMap;
import static gov.nasa.luv.Constants.*;
import static java.lang.System.*;
import static javax.swing.JFileChooser.*;

public class FileHandler 
{
    
      // file variables
      
      private static File debug = null;                                  // debug file user wants to use when executing via command prompt
      private static File plan = null;                                   // current plexil plan  
      private static File script = null;                                 // current plexil script
      
      // file chooser object 
      
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

      // directory chooser object 
      
      JFileChooser dirChooser = new JFileChooser()
         {
               {
                  setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
               }
         };
         
      // return current instance of either the plan, script or debug file
         
      public File getCurrentFile(int file)
      {
          switch (file)
          {
              case PLAN:
                  return plan;
              case SCRIPT:
                  return script;
              case DEBUG:
                  return debug;
              default:
                  return null; //error
          }
      }
      
      // return current instance of either the plan, script or debug file
         
      public void setCurrentFile(int type) throws IOException
      {
          switch (type)
          {
              case PLAN:
                  plan = getPlan();
                  break;
              case SCRIPT:
                  script = getScript();
                  break;
              case DEBUG:
                  debug = null;
                  break;
              default:
                  ; //error
          }
      }
      
      public void clearCurrentFile(int type)
      {
          switch (type)
          {
              case PLAN:
                  plan = null;
                  break;
              case SCRIPT:
                  script = null;
                  break;
              case DEBUG:
                  debug = null;
                  break;
              default:
                  ; //error
          }
      }
    
      // find the appropriate plan to be executed
      
    public File getPlan() 
    {    
        if (Luv.getLuv().getBoolean(AT_START_SCREEN) || getRecentPlanName(1) == null)
        {
            JOptionPane.showMessageDialog(Luv.getLuv(), "Please select a plan.");
            choosePlan();
        }              
        
        plan = new File(getRecentPlanName(1));

        return plan;  
    }
    
    // find the libraries needed
    
    public String getLibrary(String library) 
    {      
        String path = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_LIB_DIR, UNKNOWN);
        
        File testPath = new File(path + System.getProperty("file.separator") + library + ".plx");
            
        if (!testPath.exists())
        {  
            testPath = new File(path + System.getProperty("file.separator") + library + ".xml");
            
            if (!testPath.exists())
            {
                path = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_DIR, UNKNOWN);
        
                testPath = new File(path + System.getProperty("file.separator") + library + ".plx");
                
                if (!testPath.exists())
                {  
                    testPath = new File(path + System.getProperty("file.separator") + library + ".xml");

                    if (!testPath.exists())
                    {
                        testPath = new File(unfoundLibrary(library));
                    }
                }
            }
        }
   
        if (testPath.exists())
        {
            path = testPath.getAbsolutePath();
            Luv.getLuv().showStatus("Loading " + path, 1000);
            String newHomePath = path.substring(0, path.lastIndexOf("/", path.length() - 1));
            Luv.getLuv().getProperties().setProperty(PROP_FILE_RECENT_LIB_DIR, newHomePath);
        }
        else
            path = null;
        
        return path;  
    }

    
    // find the appropriate script to be executed

    public File getScript() throws IOException 
    {      
        String path = "";
        String name = "";
                
        if (script == null)
        {
            // first check plan directory for script
            
            path = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_DIR, UNKNOWN);
            File testPath = new File(path);
            
            if (testPath.exists())
            {             
                path = path + System.getProperty(PROP_FILE_SEPARATOR);
                
                if (plan.getName().contains(".xml"))
                    name = plan.getName().replace(".xml", "-script.xml");
                else
                    name = plan.getName().replace(".plx", "-script.plx");

                script = new File(path + name);

                if (!script.canRead())
                {
                    script = searchForScript(name, plan.getName(), path);

                    if (script == null)
                    {
                        // if no script in plan directory, look for script directory
                        
                        path = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_DIR, UNKNOWN);
                        path = path + System.getProperty(PROP_FILE_SEPARATOR);
                        path = path.substring(0, path.lastIndexOf("/"));                       
                        path = path + System.getProperty(PROP_FILE_SEPARATOR) + "scripts" + System.getProperty(PROP_FILE_SEPARATOR);
                        
                        if (plan.getName().contains(".xml"))
                            name =  plan.getName().replace(".xml", "-script.xml");
                        else
                            name =  plan.getName().replace(".plx", "-script.plx");

                        script = new File(path + name);

                        if (!script.canRead())
                        {
                            if (name.contains(".xml"))
                                name = name.replaceAll(".xml", ".plx");
                            else
                                name = name.replaceAll(".plx", ".xml");
                            
                            script = new File(path + name);
                            
                            if (!script.canRead())
                            {
                                script = searchForScript(name, plan.getName(), path);

                                if (script == null)
                                {
                                    // if no script anywhere, create an empty script

                                    path = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_DIR, UNKNOWN);
                                    path = path + System.getProperty(PROP_FILE_SEPARATOR);
                                    createEmptyScript(path);
                                }
                            }
                        }
                    }
                }
            }
            if (!Luv.getLuv().getBoolean(DONT_LOAD_SCRIPT_AGAIN))
                loadScript(script);
        }

        if (script != null)
        {
            Luv.getLuv().getProperties().set(PROP_FILE_RECENT_SCRIPT_DIR, script.getParent());
            Luv.getLuv().getProperties().set(PROP_FILE_RECENT_SCRIPT_BASE + 1, script.toString());
        }
        
        return script;  
    }
    
    public File searchForScript(String scriptName, String planName, String path) throws IOException 
      {
          try 
          {
              planName = planName.substring(0, planName.indexOf('.'));
              
              //PLAN_script.plx
              String name1 = planName + "_script.plx";
              script = new File(path + name1);
              if (!script.canRead())
              {
                  //script-PLAN.plx
                  String name2 = "script-" + planName + ".plx";
                  script = new File(path + name2);
                  if (!script.canRead())
                  {
                      //script_PLAN.plx
                      String name3 = "script_" + planName + ".plx";
                      script = new File(path + name3);
                      if (!script.canRead())
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
          
          return script;
      }
    
      public void createEmptyScript(String path) throws IOException 
      {
         Object[] options = 
         {
            "Yes, use empty script",
            "No, I will locate Script",
            "Cancel plan execution"
         };
         
         Luv.getLuv().showStatus("Unable to locate a script for this plan", 1000);

         int findScript = JOptionPane.showOptionDialog(
               Luv.getLuv(),
         "Unable to locate a script for this plan. \n\nDo you want to use the following default empty script?\n\n"
                 + Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_DIR) + System.getProperty(PROP_FILE_SEPARATOR) + DEFAULT_SCRIPT_NAME + "\n\n",
         "Default Script Option",
         JOptionPane.YES_NO_CANCEL_OPTION,
         JOptionPane.WARNING_MESSAGE,
         null,
         options,
         options[0]);

         if (findScript == 0)
         {
              String scriptName = path + DEFAULT_SCRIPT_NAME;
              FileWriter emptyScript = new FileWriter(scriptName);
              BufferedWriter out = new BufferedWriter(emptyScript);
              out.write(EMPTY_SCRIPT);
              out.close();                          
              script = new File(scriptName);
              Luv.getLuv().getProperties().set(PROP_FILE_RECENT_SCRIPT_BASE + 1, script.getAbsolutePath());
         }
         else if (findScript == 1)
         {
             Luv.getLuv().setBoolean(DONT_LOAD_SCRIPT_AGAIN, true);
             chooseScript();
         }
         else if (findScript == 2)
         {
             Luv.getLuv().setLuvViewerState(READY_STATE);
         }
     }
         
       // Select and load a script from the disk.  This operates on the global model.
      
      public void chooseScript()
      {
         try
         {
            fileChooser.setCurrentDirectory(new File(Luv.getLuv().getProperties().getString(PROP_FILE_RECENT_PLAN_DIR)));
            int option = fileChooser.showOpenDialog(Luv.getLuv());
            
            switch (option)
            {
                case APPROVE_OPTION:
                {
                   script = fileChooser.getSelectedFile();
                   Luv.getLuv().getProperties().set(PROP_FILE_RECENT_SCRIPT_DIR, script.getParent());
                   Luv.getLuv().getProperties().set(PROP_FILE_RECENT_SCRIPT_BASE + 1, script.toString());               
                   loadScript(script);
                   break;
                }
                case CANCEL_OPTION:
                {
                    Luv.getLuv().setLuvViewerState(READY_STATE); 
                    break;
                }
                case ERROR_OPTION:
                    break;
            }
         }
         catch(Exception e)
         {
            e.printStackTrace();
         }
      }
      
      // Select and load a plexil plan from the disk.  This operates on the global model.
      
      public int choosePlan()
      {
         int option = -1;
         try
         {
            fileChooser.setCurrentDirectory(new File(Luv.getLuv().getProperties().getString(PROP_FILE_RECENT_PLAN_DIR)));
            option = fileChooser.showOpenDialog(Luv.getLuv());
            
            switch (option)
            {
                case APPROVE_OPTION:
                    plan = fileChooser.getSelectedFile();
                    Luv.getLuv().getProperties().set(PROP_FILE_RECENT_PLAN_DIR, plan.getParent());
                    Luv.getLuv().getProperties().set(PROP_FILE_RECENT_SCRIPT_DIR, plan.getParent());
                    Luv.getLuv().getProperties().set(PROP_FILE_RECENT_LIB_DIR, plan.getParent());
                    script = null;
                    Luv.getLuv().getProperties().set(PROP_FILE_RECENT_SCRIPT_BASE + 1, UNKNOWN);
                    Luv.getLuv().setLuvViewerState(READY_STATE);
                    Luv.getLuv().setBoolean(OPEN_PLN_VIA_LUV, true);
                    loadPlan(plan);
                    break;
                case CANCEL_OPTION:
                    if (Luv.getLuv().getBoolean(AT_START_SCREEN))
                        Luv.getLuv().setLuvViewerState(START_STATE);
                    Luv.getLuv().setBoolean(OPEN_PLN_VIA_LUV, false);
                    break;
                case ERROR_OPTION:
                    break;
            }           
         }
         catch(Exception e)
         {
            e.printStackTrace();
         }
         
         return option;
      }
      
      /**
       * Load a plexil script from the disk.  This operates on the global model.
       *
       * @paramscript file to load
       */
      
      public void loadScript (File script)
      {
          
         if (!Luv.getLuv().getBoolean(STOPPED_EXECUTION) && script != null)
             Luv.getLuv().showStatus("Loading "  + script, 50);
         
         if (script != null)
             Luv.getLuv().getModel().addScriptName(script.toString());
      }
      
      /**
       * Load a plexil plan from the disk.  This operates on the global
       * model.
       *
       * @param plan the plan file to load
       * @param libraryNames names of library file the plan to load
       */
      
      public void loadPlan(File plan) 
      {
         Luv.getLuv().getModel().clear();
         Luv.getLuv().getModel().addPlanName(plan.toString());
         Luv.getLuv().getModel().setProperty(VIEWER_BLOCKS, FALSE);
     
         readPlan(Luv.getLuv().getModel(), plan);
            
         if(!Luv.getLuv().getBoolean(STOP_SRCH_LIBS))
         {
             Luv.getLuv().getViewHandler().resetView();
             Luv.getLuv().addFileToRecentFileList();
         }
         
      }
            
      // Load a recently loaded plan

      public void loadRecentPlan(int index)
      {
         String planName = getRecentPlanName(index);
         String scriptName = getRecentScriptName(index);
         HashMap<String, String> libraryNames = new HashMap<String, String>(Luv.getLuv().getLibraryNames());
         
         if (planName != null)
         {
            loadPlan(new File(planName));
            if (scriptName != null && !scriptName.equals(UNKNOWN))
            {
                script = new File(scriptName);
                if (script.exists())
                    loadScript(script);
                else
                    script = null;
            }
            else
            {
                script = null;
            }
            
            Luv.getLuv().clearLibraryNames();
            
            for (String libName : libraryNames.keySet())
            {
                String path = getLibrary(libName);
                if (path == null)
                    break;
                Luv.getLuv().addLibraryName(libName, path);
            }
            
            Luv.getLuv().getViewHandler().resetView();
            Luv.getLuv().setLuvViewerState(READY_STATE);
         }
      }      
      
      // Given a recent plan index, return the associated recent plan filename.

      public String getRecentPlanName(int recentIndex)
      {
         return Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_BASE + recentIndex);
      }
      
      // Given a recent script index, return the associated recent script filename

      public String getRecentScriptName(int recentIndex)
      {
         return Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_SCRIPT_BASE + recentIndex);
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
         Luv.getLuv().showStatus("Loading "  + file);
         try
         {
            parseXml(new FileInputStream(file), model);
         }
         catch(Exception e)
         {
            JOptionPane.showMessageDialog(
               Luv.getLuv(),
               "Error loading plan: " + file.getName() + 
               "  See debug window for details.",
               "Parse Error",
               JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
         }
         Luv.getLuv().clearStatus();
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
               Luv.getLuv(),
               "Error parsing XML message.  See debug window for details.",
               "Parse Error",
               JOptionPane.ERROR_MESSAGE);
            e.printStackTrace();
         }

         return isPlan;
      }
      
      public String unfoundLibrary(String callName)
      {
          boolean retry = true;
          String fullName = "";
          
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
                        "Cancel loading plan"
                     };

                  // show the options

                  Luv.getLuv().showStatus("Unable to locate the \"" + callName + "\" library", 1000);
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
                        fullName = chooseLibrary();
                        retry = false;
                        break;

                        // if the user doesn't want to find this library
                        // go on with link but don't retry to like this
                        // one

                     case 1:
                         Luv.getLuv().getViewHandler().resetView();
                         Luv.getLuv().addFileToRecentFileList();
                         retry = false;
                         Luv.getLuv().setBoolean(STOP_SRCH_LIBS, true);
                         break;

                     // if the user doesn't want to load any libraries,
                     // halt the link operation now

                     case 2:
                         Luv.getLuv().setLuvViewerState(START_STATE);
                         retry = false;
                         Luv.getLuv().setBoolean(CANCEL_PLAN_LOADING, true);
                         Luv.getLuv().setBoolean(STOP_SRCH_LIBS, true);
                         throw(new Error("Canceled loading plan"));
                  }
               }
            } while (retry); 
          
          return fullName;
      }
      
      /**
       * Select and load a plexil library from the disk.  The library is
       * added to the set of global libraries.
       */
      
      public String chooseLibrary()
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
               return library.getAbsolutePath();
            }
         }
         catch(Exception e)
         {
            e.printStackTrace();
         }
         
         return null;
      }
}
