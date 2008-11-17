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
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.InterruptedIOException;
import java.util.ArrayList;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;
import javax.swing.JOptionPane;
import org.xml.sax.InputSource;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;

import static gov.nasa.luv.Constants.*;
import static javax.swing.JFileChooser.*;

public class FileHandler 
{
    private static boolean doNotLoadScript             = false;        // is script already loaded? if so, do not waste time loading it again
    private static boolean stopSearchForMissingLibs    = false;        // is library found? if so, stop searching for missing libraries
    
    // directory chooser object 
      
    JFileChooser dirChooser = new JFileChooser()
	{
	    {
		setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
	    }
	};
      
    // file chooser object 
      
    JFileChooser fileChooser = new JFileChooser()
	{
	    {
		/** XML file filter */
                  
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
 
    private String getFileNameSansExtension(String name)
    {
        return name.substring(0, name.indexOf('.'));  
    }
    
    public boolean getStopSearchForMissingLibs()
    {
        return stopSearchForMissingLibs;
    }
    
    public void setStopSearchForMissingLibs(boolean value)
    {
        stopSearchForMissingLibs = value;
    }
    
    // find the libraries needed
    
    public File searchForLibrary(String libraryName) throws InterruptedIOException 
    {      
        String directory = Luv.getLuv().getProperty(PROP_FILE_RECENT_LIB_DIR);
        
        File library = new File(directory + System.getProperty(PROP_FILE_SEPARATOR) + libraryName + ".plx");
            
        if (!library.exists()) {  
            library = new File(directory + System.getProperty(PROP_FILE_SEPARATOR) + libraryName + ".xml");
            
            if (!library.exists()) {
                directory = Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR);
        
                library = new File(directory + System.getProperty(PROP_FILE_SEPARATOR) + libraryName + ".plx");
                
                if (!library.exists()) {  
                    library = new File(directory + System.getProperty(PROP_FILE_SEPARATOR) + libraryName + ".xml");

                    if (!library.exists()) {
			directory = unfoundLibrary(libraryName);
			if (directory == null)
			    library = null;
			else
			    library = new File(directory);
                    }
                }
            }
        }
   
        if (library != null && library.exists()) 
        {
            directory = library.getAbsolutePath();
            Luv.getLuv().setProperty(PROP_FILE_RECENT_LIB_DIR, library.getParent());
            Luv.getLuv().setProperty(PROP_FILE_RECENT_LIB_BASE, library.toString());
            Luv.getLuv().showStatus("Loading library "  + library, 1000);
	    loadPlan(library);
        }
        else
            library = null;
        
        return library;  
    }
    
    // find the appropriate script to be executed

    public File searchForScript() throws IOException 
    {
        File script = null;
        String directory = ""; 
        ArrayList<String> listOfDirectories = generateListOfDirectories();

        // if user did not specify script, look for it 
        
        for (int i = 0; i < listOfDirectories.size() && script == null; i++) 
        {
            directory = listOfDirectories.get(i);

            if (new File(directory).exists())
            {
                if (Luv.getLuv().getCurrentPlan() != null)
                    script = tryScriptNameVariations(Luv.getLuv().getCurrentPlan().getPlanNameSansPath(), directory);
                else
                    return null;
            }
        }
        
        // if cannot find script, create empty script and prompt user

        if (script == null)
        {
            directory = Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR) + System.getProperty(PROP_FILE_SEPARATOR);
            script = createEmptyScript(directory);
        }

        if (!doNotLoadScript)
        {
            Luv.getLuv().setProperty(PROP_FILE_RECENT_SCRIPT_DIR, script.getParent());
            Luv.getLuv().setProperty(PROP_FILE_RECENT_SCRIPT_BASE, script.toString()); 
            loadScript(script);   
        }
        
        return script;  
    }
         
    // Select and load a script from the disk.  This operates on the global model.
      
    public int chooseScript()
    {
	int option = -1;
	try 
        {
            fileChooser.setCurrentDirectory(new File(Luv.getLuv().getProperty(PROP_FILE_RECENT_SCRIPT_DIR)));
            
            if (fileChooser.showDialog(dirChooser, "Open Script") == APPROVE_OPTION)
            {
                    File script = fileChooser.getSelectedFile();
                    Luv.getLuv().setProperty(PROP_FILE_RECENT_SCRIPT_DIR, script.getParent());
                    Luv.getLuv().setProperty(PROP_FILE_RECENT_SCRIPT_BASE, script.toString()); 
                    loadScript(script);
                    return APPROVE_OPTION;
            }
	}
	catch(Exception e) 
        {
            Luv.getLuv().displayErrorMessage(e, "ERROR: exception occurred while choosing script");
	}
         
	return option;
    }
      
    // Select and load a plexil plan from the disk.  This operates on the global model.
      
    public int choosePlan()
    {
	try 
        {
            fileChooser.setCurrentDirectory(new File(Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR)));
            
            if (fileChooser.showDialog(dirChooser, "Open Plan") == APPROVE_OPTION)
            {
                    File plan = fileChooser.getSelectedFile();
                    Luv.getLuv().setProperty(PROP_FILE_RECENT_PLAN_DIR, plan.getParent());
                    Luv.getLuv().setProperty(PROP_FILE_RECENT_PLAN_BASE, plan.toString());
                    Luv.getLuv().setProperty(PROP_FILE_RECENT_LIB_DIR, plan.getParent());
                    Luv.getLuv().showStatus("Loading plan "  + plan, 1000);
                    loadPlan(plan);
                    return APPROVE_OPTION;
            }  
            else
                return -1;
	}
	catch(Exception e) 
        {
            Luv.getLuv().displayErrorMessage(e, "ERROR: exception occurred while choosing plan");
	}
         
	return -1;
    }  
          
    /**
     * Select and load a plexil library from the disk.  The library is
     * added to the set of global libraries.
     */
      
    public String chooseLibrary()
    {
	try 
        {
            String recent = Luv.getLuv().getProperty(PROP_FILE_RECENT_LIB_DIR);
            if (recent == null)
		recent = Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR);
            
            fileChooser.setCurrentDirectory(new File(recent));

            if (fileChooser.showDialog(dirChooser, "Open Library") == APPROVE_OPTION) 
            {
		File library = fileChooser.getSelectedFile();
		Luv.getLuv().setProperty(PROP_FILE_RECENT_LIB_DIR, library.getParent());
		return library.getAbsolutePath();
            }
	}
	catch(Exception e) 
        {
            Luv.getLuv().displayErrorMessage(e, "ERROR: exception occurred while choosing library");
	}
         
	return null;
    }
      
    /**
     * Load a plexil script from the disk.
     *
     * @param script file to load
     */
      
    public void loadScript(File script)
    {          
	if (script != null)
        {
            Luv.getLuv().showStatus("Loading script "  + script, 1000);
            if (Luv.getLuv().getCurrentPlan() != null)
                Luv.getLuv().getCurrentPlan().addScriptName(script.getAbsolutePath());
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
        if (plan != null)
        {
            readPlan(plan);
            Luv.getLuv().getCurrentPlan().addPlanName(plan.toString());
        }
    }
            
    // Load a recently loaded run

    public void loadRecentRun(int index) throws IOException
    {
        File script = null;
	String planName = Luv.getLuv().getRecentPlan(index);
	String scriptName = Luv.getLuv().getRecentScript(index);
         
	if (planName != null) 
        {
            File plan = new File(planName);
            loadPlan(plan);
            
            if (scriptName != null && !scriptName.equals(UNKNOWN)) 
            {
                script = new File(scriptName);
                
                if (script.exists())
                    loadScript(script);
                else
                    script = searchForScript();                
            }
            else 
            {
                script = searchForScript();
            }
	}
    }   
      
    /**
     * Read plexil plan from disk and create an internal model.
     *
     * @param file file containing plan
     *
     */

    private Model readPlan(File file)
    {
	Model result = null;
        
	try 
        {
            result = parseXml(new FileInputStream(file));
	}
	catch(Exception e) 
        {
            Luv.getLuv().displayErrorMessage(e, "ERROR: exception occurred while loading: " + file.getName());
	}
        
	return result;
    }

    /** Parse a plan from an XML stream
     * @param input source of xml to parse
     * @return returns top level node or null
     * @note Is reentrant to support loading of libraries during plan loading.
     */

    private Model parseXml(InputStream input)
    {
	PlexilPlanHandler ch = new PlexilPlanHandler();
	try 
        {
            InputSource is = new InputSource(input);
            XMLReader parser = XMLReaderFactory.createXMLReader();
            parser.setContentHandler(ch);
            parser.parse(is);          
	}
        catch (Exception e) 
        {
            Luv.getLuv().displayErrorMessage(e, "ERROR: exception occurred while parsing XML message");
	    return null;
	}
        
	return ch.getPlan();
    }
      
    private String unfoundLibrary(String callName) throws InterruptedIOException
    {
	boolean retry = true;
	String fullName = "";
          
	do {
		// if we didn't make the link, ask user for library

		if (retry) {

		    Object[] options = 
			{
			    "I will locate library",
			    "Cancel loading plan"
			};

		    // show the options

		    Luv.getLuv().showStatus("Unable to locate the \"" + callName + "\" library", 1000);
		    int result = JOptionPane.showOptionDialog(Luv.getLuv(),
							      "Unable to locate the \"" + callName + "\" library.\n\n" +
							      "What do you want to do?\n\n",
							      "Load the library?",
							      JOptionPane.YES_NO_CANCEL_OPTION,
							      JOptionPane.WARNING_MESSAGE,
							      null,
							      options,
							      options[0]);

		    // process the results

		    switch (result) {
			// try to load the library and retry the link

		    case 0:
                        fullName = chooseLibrary();
                        retry = false;
                        break;

			// if the user doesn't want to load any libraries,
			// halt the link operation now

		    case 1:
			Luv.getLuv().startState();
			retry = false;
			stopSearchForMissingLibs = true;
			break;
		    }
		}
            } 
	while (retry); 
          
	return fullName;
    } 
        
    private File tryScriptNameVariations(String planName, String path) throws IOException 
    {
	try 
        {
	    planName = getFileNameSansExtension(planName);  
            
            ArrayList<String> listOfScriptNames = generateListOfScriptNames(planName, path);
            
            for (String scriptName : listOfScriptNames)
            {
                if (new File(scriptName).exists())
                {
                    return new File(scriptName);
                }
            }
            
        }
        catch (Exception e)
        {
            Luv.getLuv().displayErrorMessage(e, "ERROR: exception occurred while locating script");
	} 
          
	return null;
    }
  
    // generate a list of possible script names if user didn't not specify one
    
    private ArrayList<String> generateListOfScriptNames(String planName, String path)
    {
        ArrayList<String> listOfScriptNames = new ArrayList<String>();
        
        listOfScriptNames.add(path + planName + "_script.plx");
        listOfScriptNames.add(path + planName + "-script.plx");
        listOfScriptNames.add(path + "script-" + planName + ".plx");
        listOfScriptNames.add(path + "script_" + planName + ".plx");
        listOfScriptNames.add(path + planName + "_script.xml");
        listOfScriptNames.add(path + planName + "-script.xml");
        listOfScriptNames.add(path + "script-" + planName + ".xml");
        listOfScriptNames.add(path + "script_" + planName + ".xml");
       
        return listOfScriptNames;
    }
    
    // generate a list of possible directories to search for script
    
    private ArrayList<String> generateListOfDirectories()
    {
        ArrayList<String> listOfDirectories = new ArrayList<String>();
  
        listOfDirectories.add(Luv.getLuv().getProperty(PROP_FILE_RECENT_SCRIPT_DIR) + System.getProperty(PROP_FILE_SEPARATOR));
        listOfDirectories.add(Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR) + System.getProperty(PROP_FILE_SEPARATOR));
        listOfDirectories.add(Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR) + System.getProperty(PROP_FILE_SEPARATOR) + "scripts" + System.getProperty(PROP_FILE_SEPARATOR));
        listOfDirectories.add(Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR) + System.getProperty(PROP_FILE_SEPARATOR) + "script" + System.getProperty(PROP_FILE_SEPARATOR));
        
        String path = Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR);
        path = path.substring(0, path.lastIndexOf('/') + 1);
        listOfDirectories.add(path);
        
        path = Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR);
        path = path.substring(0, path.lastIndexOf('/') + 1) + "script" + System.getProperty(PROP_FILE_SEPARATOR);
        listOfDirectories.add(path);
        
        path = Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR);
        path = path.substring(0, path.lastIndexOf('/') + 1) + "scripts" + System.getProperty(PROP_FILE_SEPARATOR);
        listOfDirectories.add(path);      
       
        return listOfDirectories;
    }
    
    private File createEmptyScript(String path) throws IOException 
    {
	Object[] options = 
	    {
		"Yes, use empty script",
		"No, I will locate Script",
		"Cancel plan execution"
	    };
         
	int option = 
	    JOptionPane.showOptionDialog(Luv.getLuv(),
					 "Unable to locate a script for this plan. \n\nDo you want to use the following default empty script?\n\n"
					 + Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR)
					 + System.getProperty(PROP_FILE_SEPARATOR)
					 + DEFAULT_SCRIPT_NAME
					 + "\n\n",
					 "Default Script Option",
					 JOptionPane.YES_NO_CANCEL_OPTION,
					 JOptionPane.WARNING_MESSAGE,
					 null,
					 options,
					 options[0]);
        
        switch (option)
        {
            case 0:
                String scriptName = path + DEFAULT_SCRIPT_NAME;
                FileWriter emptyScript = new FileWriter(scriptName);
                BufferedWriter out = new BufferedWriter(emptyScript);
                out.write(EMPTY_SCRIPT);
                out.close();                          
                Luv.getLuv().getCurrentPlan().addScriptName(scriptName);
                return new File(scriptName);
            case 1:
                doNotLoadScript = true;
                if (chooseScript() == APPROVE_OPTION)
                    return new File(Luv.getLuv().getProperty(PROP_FILE_RECENT_SCRIPT_BASE));
                else
                    return null;
            case 2:
                doNotLoadScript = true;
                Luv.getLuv().readyState();
                return null;                    
        }
        
        
        return null;
    }
}
