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

import gov.nasa.luv.Luv;

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

/**
 * 
 * The FileHandler class handles the searching, opening and loading of Plexil 
 * plans, scripts and libraries.
 */

public class FileHandler 
{
    // is script already loaded? if so, do not waste time loading it again
    private static boolean doNotLoadScript;        
    // is library found? if so, stop searching for missing libraries
    private static boolean stopSearchForMissingLibs;           
    // directory chooser object       
    private JFileChooser dirChooser;      
    // file chooser object       
    private JFileChooser fileChooser;
    
    /**
     * Constructs a FileHandler.
     */
    public FileHandler() 
    {
        init();
    }
    
    private void init()
    {
        doNotLoadScript = false;
        stopSearchForMissingLibs  = false; 
        
        dirChooser = new JFileChooser()
	{
	    {
		setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
	    }
	};
        
        fileChooser = new JFileChooser()
	{
	    {
		// XML file filter                  
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
                            {
                                for (String ext: FILE_EXTENSIONS)
                                    if (extension.equals(ext))
                                        correctExtension = true;
                            }
                              
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
			    return "XML / PLX / PLS";
			}
		    });
	    }
	};
    }
    
    /**
     * Returns whether or not Luv should stop searching for missing Plexil libraries.
     * @return whether or not Luv should stop searching for missing Plexil libraries
     */
 
    public boolean getStopSearchForMissingLibs()
    {
        return stopSearchForMissingLibs;
    }
    
    /**
     * Sets whether or not Luv should stop searching for missing Plexil libraries.
     * @param value sets whether or not Luv should stop searching for missing Plexil libraries
     */
    public void setStopSearchForMissingLibs(boolean value)
    {
        stopSearchForMissingLibs = value;
    }
    
    /** Finds the Plexil library needed.
     * 
     * @param libraryName the name of the Plexil library to search for
     * @return Plexil library or null if not found
     * @throws java.io.InterruptedIOException
     */
    public File searchForLibrary(String libraryName) throws InterruptedIOException 
    {      
    	String directory = Luv.getLuv().getProperty(LoadRecentAction.defineRecentLib(LoadRecentAction.RECENT_DIR));    	
        
        File library = new File(directory + System.getProperty("file.separator") + libraryName + ".plx");
            
        if (!library.exists()) {
        	//algorithm to use user defined libraries
        	for(File testName : Luv.getLuv().getLibLoad().getLibraryList())
        	{
        		String libraryTestName = testName.getName();
        		if(!library.exists())
        		{
	        		if(testName.isDirectory())
	        			library = new File(testName + System.getProperty("file.separator") + libraryName + ".plx");
	        		for(int i = 0; i<Constants.FILE_EXTENSIONS.length;i++)
	        			libraryTestName = libraryTestName.replaceAll("."+Constants.FILE_EXTENSIONS[i], "");
	        		if(testName.isFile() && libraryTestName.equals(libraryName))
	        			library = testName;
        		}
        	}
        	
        	if (!library.exists())
        		library = new File(directory + System.getProperty("file.separator") + libraryName + ".xml");
            
            if (!library.exists()) {
                directory = Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR));
        
                library = new File(directory + System.getProperty("file.separator") + libraryName + ".plx");
                
                if (!library.exists()) {  
                    library = new File(directory + System.getProperty("file.separator") + libraryName + ".xml");

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
            Luv.getLuv().setProperty(LoadRecentAction.defineRecentLib(LoadRecentAction.RECENT_DIR), library.getParent());
            Luv.getLuv().setProperty(LoadRecentAction.defineRecentLib(LoadRecentAction.RECENT_BASE), library.toString());         
            loadPlan(library);
            Luv.getLuv().getStatusMessageHandler().showStatus("Library \"" + library.toString() + "\" loaded", 1000);
        }
        else
            library = null;
        
        return library;  
    }
    
    /** Finds the Plexil config needed.
     * 
     * @return the Plexil config or null if not found
     * @throws java.io.IOException
     */
    public File searchForConfig() throws IOException
    {
    	File config = null;
    	String directory = "";
    	// create empty script and prompt user

            directory = Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR)) + System.getProperty("file.separator");
            config = createEmptyScript(directory);

        if (!doNotLoadScript)
        {
            Luv.getLuv().setProperty(LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_DIR), config.getParent());
            Luv.getLuv().setProperty(LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_BASE), config.toString()); 
            loadScript(config);  
            Luv.getLuv().getStatusMessageHandler().showStatus("Config \"" + config.toString() + "\" loaded", 1000);
        }
        
        return config;
    	
    }
    
    
    /** Finds the Plexil script needed.
     * 
     * @return the Plexil script or null if not found
     * @throws java.io.IOException
     */
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
                    script = tryScriptNameVariations(Luv.getLuv().getCurrentPlan().getPlanName(), directory);
                else
                    return null;
            }
        }
        
        // if cannot find script, create empty script and prompt user
        if (script == null)
        {
            directory = Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR)) + System.getProperty("file.separator");
				script = createEmptyScript(directory);
        }

        if (!doNotLoadScript)
        {
            Luv.getLuv().setProperty(LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_DIR), script.getParent());
            Luv.getLuv().setProperty(LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_BASE), script.toString()); 
            loadScript(script);  
            Luv.getLuv().getStatusMessageHandler().showStatus("Script \"" + script.toString() + "\" loaded", 1000);
        }
        
        return script;  
    }
    
    /** Finds the Plexil script path.
     * 
     * @return the Plexil script location 
     * @throws java.io.IOException
     */
    public File searchForScriptPath(String plan) throws IOException
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
                if (!plan.equals(""))
                    script = tryScriptNameVariations(plan, directory);
                else
                    return null;
            }
        }        
        
        return script;  
    }
         
    /** Selects and loads a Plexil script from the disk. This operates on the global model.    
     * 
     * @return the choice type (APPROVE_OPTION, etc.) the user made
     */ 
    public int chooseScript()
    {
	int option = -1;
	String supplement = "Script";
	if (Luv.getLuv().getAppMode() == PLEXIL_EXEC)
		supplement = "Config";
	try 
        {
            fileChooser.setCurrentDirectory(new File(Luv.getLuv().getProperty(LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_DIR))));
            
            if (fileChooser.showDialog(dirChooser, "Open " + supplement) == APPROVE_OPTION)
            {
                    File script = fileChooser.getSelectedFile();
                    Luv.getLuv().setProperty(LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_DIR), script.getParent());
                    Luv.getLuv().setProperty(LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_BASE), script.toString()); 
                    loadScript(script);
                    if(supplement.equals("Config"))
                    	//ExecSelect.getExecSel().setConfigName(script.toString());
                    	ExecSelect.getExecSel().getSettings().setSuppLocation(script.toString());
                    else	
                    	//ExecSelect.getExecSel().setScriptName(script.toString());
                    	ExecSelect.getExecSel().getSettings().setSuppLocation(script.toString());

                    return APPROVE_OPTION;
            }
	}
	catch(Exception e) 
        {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while choosing script");
	}
         
	return option;
    }
      
    /** Selects and loads a Plexil plan from the disk. This operates on the global model.  
     * 
     * @return the choice type (APPROVE_OPTION, etc.) the user made
     */    
    public int choosePlan()
    {
	try 
        {
            fileChooser.setCurrentDirectory(new File(Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR))));
            
            if (fileChooser.showDialog(dirChooser, "Open Plan") == APPROVE_OPTION)
            {
                    File plan = fileChooser.getSelectedFile();
                    Luv.getLuv().setProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR), plan.getParent());
                    Luv.getLuv().setProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_BASE), plan.toString());
                    Luv.getLuv().setProperty(LoadRecentAction.defineRecentLib(LoadRecentAction.RECENT_DIR), plan.getParent());
                    loadPlan(plan);
                    Luv.getLuv().getStatusMessageHandler().showStatus("Plan \"" + plan.toString() + "\" loaded", 1000);
                    //ExecSelect.getExecSel().setPlanName(plan.toString());
                    ExecSelect.getExecSel().getSettings().setPlanLocation(plan.toString());
                    return APPROVE_OPTION;
            }  
            else
                return -1;
	}
	catch(Exception e) 
        {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while choosing plan");
	}
         
	return -1;
    }  
          
    /** Selects and loads a Plexl library from the disk. This operates on the global model.
     * 
     * @return the Plexil library path or null if not found
     */       
    public String chooseLibrary()
    {
	try 
        {
            String recent = Luv.getLuv().getProperty(LoadRecentAction.defineRecentLib(LoadRecentAction.RECENT_DIR));
            if (recent == null)
		recent = Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR));
            
            fileChooser.setCurrentDirectory(new File(recent));

            if (fileChooser.showDialog(dirChooser, "Open Library") == APPROVE_OPTION) 
            {
		File library = fileChooser.getSelectedFile();
		Luv.getLuv().setProperty(LoadRecentAction.defineRecentLib(LoadRecentAction.RECENT_DIR), library.getParent());
		return library.getAbsolutePath();
            }
	}
	catch(Exception e) 
        {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while choosing library");
	}
         
	return null;
    }
      
    /** Loads a Plexil script from the disk.
     * 
     * @param script the Plexil script to be loaded
     */
    public void loadScript(File script)
    {          
	if (script != null)
        {
            if (Luv.getLuv().getCurrentPlan() != null)
                Luv.getLuv().getCurrentPlan().addScriptName(script.getAbsolutePath());
        }
    }

    /** Loads a Plexil plan from the disk. This operates on the global model.
     * 
     * @param plan the Plexil plan to be loaded
     */
    public void loadPlan(File plan)
    {
        if (plan != null)
        {      
            readPlan(plan); 
            Luv.getLuv().getCurrentPlan().addPlanName(plan.toString());
        }
    }
            
    /** Loads a recently loaded Plexil plan, script and (if any) supporting libraries.
     * 
     * @param index the item in the list of previous Plexil plans
     * @throws java.io.IOException
     */
    public void loadRecentRun(int index) throws IOException
    {
        File script = null;
	String planName = LoadRecentAction.getRecentPlan(index);
	String scriptName = LoadRecentAction.getRecentScript(index);
         
	if (planName != null) 
        {
            File plan = new File(planName);
            loadPlan(plan);
            Luv.getLuv().getStatusMessageHandler().showStatus("Plan \"" + plan.toString() + "\" loaded", 1000);
            
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
      
    // read plexil plan from disk and create an internal model.
    private Model readPlan(File file)
    {
	Model result = null;
        
	try 
        {
            result = parseXml(new FileInputStream(file));
	}
	catch(Exception e) 
        {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while loading: " + file.getName());
	}
        
	return result;
    }

    // parse a plan from an XML stream
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
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while parsing XML message");
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
			    "Skip library"
			};

		    // show the options
		    Luv.getLuv().getStatusMessageHandler().showStatus("Unable to locate the \"" + callName + "\" library", 1000);
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
	    planName = planName.substring(0, planName.indexOf('.')); 
            
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
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while locating script");
	} 
          
	return null;
    }
  
    // generate a list of possible script names if user didn't not specify one    
    private ArrayList<String> generateListOfScriptNames(String planName, String path)
    {
        ArrayList<String> listOfScriptNames = new ArrayList<String>();
        listOfScriptNames.add(path + planName + ".psx");
        return listOfScriptNames;
    }
    
    // generate a list of possible directories to search for script   
    private ArrayList<String> generateListOfDirectories()
    {
        ArrayList<String> listOfDirectories = new ArrayList<String>();
  
        listOfDirectories.add(Luv.getLuv().getProperty(LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_DIR)) + System.getProperty("file.separator"));
        listOfDirectories.add(Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR)) + System.getProperty("file.separator"));
        listOfDirectories.add(Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR)) + System.getProperty("file.separator") + "scripts" + System.getProperty("file.separator"));
        listOfDirectories.add(Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR)) + System.getProperty("file.separator") + "script" + System.getProperty("file.separator"));
        
        String path = Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR));
        path = path.substring(0, path.lastIndexOf('/') + 1);
        listOfDirectories.add(path);
        
        path = Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR));
        path = path.substring(0, path.lastIndexOf('/') + 1) + "script" + System.getProperty("file.separator");
        listOfDirectories.add(path);
        
        path = Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR));
        path = path.substring(0, path.lastIndexOf('/') + 1) + "scripts" + System.getProperty("file.separator");
        listOfDirectories.add(path);      
       
        return listOfDirectories;
    }
    
    /*
     * Determines appropriate default supplement's absolute path based upon mode
     * @return absolute filename of default supplement
     */
    private String defaultEmptySupp(){
    	String str = "";
    	switch(Luv.getLuv().getAppMode())
    	{
    	case PLEXIL_TEST:
    		str = Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR))
			 + System.getProperty("file.separator")
			 + DEFAULT_SCRIPT_NAME;
    		break;
    	case PLEXIL_EXEC:
    		str = DEFAULT_CONFIG_PATH+DEFAULT_CONFIG_NAME;
    		break;
    	case PLEXIL_SIM:
    		str = DEFAULT_SIM_SCRIPT_PATH+DEFAULT_SIM_SCRIPT_NAME;
    		break;
    	}
    	return str;
    }
    
    private File createEmptyScript(String path) throws IOException 
    {
    String supplement = Luv.getLuv().getExecSelect().getSettings().getSuppName();
    String defaultSup = "Yes, use default " + supplement;
    String message_supplement = defaultEmptySupp();
    
	Object[] options = 
	    {
		defaultSup,
		"No, I will locate " + supplement,
		"Cancel plan execution"
	    };
         
	int option = 
	    JOptionPane.showOptionDialog(Luv.getLuv(),
					 "Unable to locate a " + supplement + " for this plan. \n\nDo you want to use the following default "+ supplement +"?\n\n"
					 + message_supplement
					 + "\n\n",
					 "Default "+ supplement +" Option",
					 JOptionPane.YES_NO_CANCEL_OPTION,
					 JOptionPane.WARNING_MESSAGE,
					 null,
					 options,
					 options[0]);	
        
        switch (option)
        {
            case 0:
            	String scriptName = path + DEFAULT_SCRIPT_NAME;
            	if (Luv.getLuv().getAppMode() == PLEXIL_TEST){	                
	                FileWriter emptyScript = new FileWriter(scriptName);
	                BufferedWriter out = new BufferedWriter(emptyScript);
	                out.write(EMPTY_SCRIPT);
	                out.close();                          
	                Luv.getLuv().getCurrentPlan().addScriptName(scriptName);
	                return new File(scriptName);
            	} else if(Luv.getLuv().getAppMode() == PLEXIL_EXEC || Luv.getLuv().getAppMode() == PLEXIL_SIM){
            		return new File(message_supplement);            		
            	}
            	else {
            		doNotLoadScript = true;      
            		return null;
            	}
            		
            case 1:
                if (chooseScript() == APPROVE_OPTION)
                    return new File(Luv.getLuv().getProperty(LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_BASE)));
                else
                    return null;
            case 2:
                doNotLoadScript = true;
                Luv.getLuv().getLuvStateHandler().readyState();
                return null;                    
        }
        
        
        return null;
    }
        
}
