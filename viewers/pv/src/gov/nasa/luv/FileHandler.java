/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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
import java.util.Collections;
import java.util.Date;
import java.util.List;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;
import javax.swing.JOptionPane;
import org.xml.sax.InputSource;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;

import static gov.nasa.luv.Constants.*;
import static gov.nasa.luv.Constants.AppType.*;
import static javax.swing.JFileChooser.*;

/**
 * 
 * The FileHandler class handles the searching, opening and loading of Plexil 
 * plans, scripts and libraries.
 */

public class FileHandler 
{
    // is library found? if so, stop searching for missing libraries
    private static boolean stopSearchForMissingLibs;           
    // directory chooser object       
    private JFileChooser dirChooser;      
    // file chooser object       
    private JFileChooser fileChooser;
    // list of loaded libraries
    private List<PlanInfo> loadedLibraries;
    
    /**
     * Constructs a FileHandler.
     */
    public FileHandler() {
        stopSearchForMissingLibs  = false; 
        // Must use a synchronized list because can be accessed from multiple threads.
        loadedLibraries = Collections.synchronizedList(new ArrayList<PlanInfo>());
        
        dirChooser = new JFileChooser();
        dirChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        
        fileChooser = new JFileChooser();
        // XML file filter                  
        fileChooser.addChoosableFileFilter(new FileFilter() {
                // accept file?                          
                public boolean accept(File f) {
                    // allow browse directories                             
                    if (f.isDirectory())
                        return true;
                              
                    // allow files with correct extention                              
                    String extension = getExtension(f);
                    Boolean correctExtension = false;
                    if (extension != null) {
                        for (String ext: FILE_EXTENSIONS)
                            if (extension.equals(ext))
                                correctExtension = true;
                    }
                    return correctExtension;
                }

                // get file extension                        
                public String getExtension(File f) {
                    String ext = null;
                    String s = f.getName();
                    int i = s.lastIndexOf('.');
                              
                    if (i > 0 && i < s.length() - 1)
                        ext = s.substring(i+1).toLowerCase();
                              
                    return ext;
                }

                // return descriton                          
                public String getDescription() {
                    return "XML / PLX / PLS";
                }
            });
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

    public Model searchForLibrary(String libraryName, boolean askUser)
        throws InterruptedIOException {
        // Try known libraries first
        synchronized(loadedLibraries) {
            PlanInfo dummy = new PlanInfo();
            dummy.name = libraryName;
            int idx = loadedLibraries.indexOf(dummy);
            if (idx >= 0) {
                // Check that it hasn't been updated
                PlanInfo p = loadedLibraries.get(idx);
                if (p.file != null && p.file.isFile()) {
                    Date lastMod = new Date(p.file.lastModified());
                    if (lastMod.after(p.lastModified)) {
                        // stale, reload it
                        p.model = readPlan(p.file);
                        p.lastModified = lastMod;
                    }
                    return p.model;
                } else {
                    // file no longer exists in that location
                    loadedLibraries.remove(p);
                }
            }
        }

        // Try plan directory next
        String candidateName = libraryName + ".plx";
    	File libDir = Luv.getLuv().getSettings().getPlanLocation().getParentFile();
        File candidate = new File(libDir, candidateName);
        if (candidate.isFile()) {
            Model result = loadLibraryFile(candidate);
            if (result != null)
                return result;
        }

        // Check user specified library path
        for (File entry : Luv.getLuv().getSettings().getLibDirs()) {
            if (entry.isDirectory()) {
                candidate = new File(entry, candidateName);
                if (candidate.isFile()) {
                    Model m = loadLibraryFile(candidate);
                    if (m != null)
                        return m;
                }
            }
        }
            
        // Search failed, ask user
        if (askUser) {
            candidate = unfoundLibrary(libraryName);
            if (candidate != null && candidate.isFile()) {
                return loadLibraryFile(candidate);
            }
        }
        return null;
    }

    private Model loadLibraryFile(File f) {
        File location = f.getAbsoluteFile();
        Model result = readPlan(location);
        if (result != null) {
            Luv.getLuv().getSettings().addLib(location.toString());
            Luv.getLuv().getStatusMessageHandler().showStatus("Library "
                                                              + location.toString()
                                                              + " loaded",
                                                              1000);
            PlanInfo p = new PlanInfo();
            p.name = result.getNodeName();
            p.model = result;
            p.file = location;
            p.lastModified = new Date(location.lastModified());
            registerLibrary(p);
        }
        return result;
    }

    /** Called when a new library notification is received from the Exec. */
    public void handleNewLibrary(Model m) {
        if (m == null)
            return;
        PlanInfo p = new PlanInfo();
        p.model = m;
        p.name = m.getNodeName();
        p.file = null; // don't have file info from exec, which may be on another machine
        p.lastModified = null;
        registerLibrary(p);
    }

    private void registerLibrary(PlanInfo p) {
        synchronized(loadedLibraries) {
            int idx = loadedLibraries.indexOf(p);
            if (idx >= 0) {
                PlanInfo existing = loadedLibraries.get(idx);
                // If we have file dates for both, and new is <= old, assume no change
                if (p.file != null) {
                    if (existing.file != null) {
                        if (!p.lastModified.after(existing.lastModified))
                            return; // no change
                    }
                }
                // Dates missing or inconclusive, have to compare, sigh
                if (p.model.equivalent(existing.model))
                    return; // no change
                // Replace old with new
                loadedLibraries.remove(idx);
                // fall thru to insert at front
            }

            // Simply insert at front
            loadedLibraries.add(0, p);
        }
    }

    private int chooseConfig()
    {
        int option = -1;
        // *** TODO ***
        return option;
    }
          
    /** Selects and loads a Plexl library from the disk. This operates on the global model.
     * @return the Plexil library path or null if not found
     */       
    public File chooseLibrary() {
        try {
            File defaultDir = Luv.getLuv().getSettings().getPlanLocation().getParentFile();
            fileChooser.setCurrentDirectory(defaultDir);
            if (fileChooser.showDialog(dirChooser, "Open Library") == APPROVE_OPTION)
                return fileChooser.getSelectedFile().getAbsoluteFile();
        } catch(Exception e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while choosing library");
        }
        return null;
    }

    public void loadConfig(File cfg) {
        if (cfg != null) {
            if (Luv.getLuv().getCurrentPlan() != null)
                Luv.getLuv().getCurrentPlan().setConfigFile(cfg.getAbsoluteFile());
        }
    }
      
    /** Loads a Plexil script from the disk.
     * 
     * @param script the Plexil script to be loaded
     */
    public void loadScript(File script) {          
        if (script != null) {
            if (Luv.getLuv().getCurrentPlan() != null)
                Luv.getLuv().getCurrentPlan().setScriptFile(script.getAbsoluteFile());
        }
    }
      
    // read plexil plan from disk and create an internal model.
    public Model readPlan(File file) {
        try {
            Model result = parseXml(new FileInputStream(file));
            result.setPlanFile(file.getAbsoluteFile());
            return result;
        } catch(Exception e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: while loading: " + file.getName());
        }
        return null;
    }

    // Helper class for parseXml.
    private class PlanCatcher
        implements PlexilPlanHandler.PlanReceiver {

        public Model plan;

        PlanCatcher() {
            plan = null;
        }
        public void newPlan(Model m) {
            plan = m;
        }
        public void newLibrary(Model m) {
            plan = m;
        }
    }

    // parse a plan from an XML stream
    private Model parseXml(InputStream input) {
        PlanCatcher c = new PlanCatcher();
        PlexilPlanHandler ch =
            new PlexilPlanHandler(c);
        try {
            InputSource is = new InputSource(input);
            XMLReader parser = XMLReaderFactory.createXMLReader();
            parser.setContentHandler(ch);
            parser.parse(is);          
        }
        catch (Exception e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while parsing XML message");
            return null;
        }
        return c.plan;
    }
      
    private File unfoundLibrary(String callName)
        throws InterruptedIOException {
        boolean retry = true;
        File libFile = null;
        do {
            // if we didn't make the link, ask user for library
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
                libFile = chooseLibrary();
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
        while (retry); 
          
        return libFile;
    }

    // *** FIXME: Only relevant for the TestExec ***
    // *** FIXME: where does this behavior belong?
    private File createEmptyScript(File path)
        throws IOException {
        File script = Luv.getLuv().getSettings().defaultEmptyScriptFile();
        Object[] options = 
            {
                "Yes, use default script",
                "No, I will locate script",
                "Cancel plan execution"
            };
         
        int option = 
            JOptionPane.showOptionDialog(Luv.getLuv(),
                                         "Unable to locate a script for this plan. \n\nDo you want to use the default script\n\n"
                                         + script.toString()
                                         + "instead? \n\n",
                                         "Default Script Option",
                                         JOptionPane.YES_NO_CANCEL_OPTION,
                                         JOptionPane.WARNING_MESSAGE,
                                         null,
                                         options,
                                         options[0]);

        switch (option) {
        case 0:
            File scriptFile = new File(path, DEFAULT_SCRIPT_NAME);
            if (Luv.getLuv().getAppMode() == PLEXIL_TEST) {	                
                writeEmptyPlexilTestScript(scriptFile);
                Luv.getLuv().getSettings().setScriptLocation(scriptFile);
                return scriptFile;
            } else if (Luv.getLuv().getAppMode() == PLEXIL_SIM) {
                return script;
            }
            else {
                return null;
            }
            		
            // TEMPORARY -- comment out so we can compile
        // case 1:
        //     return chooseScript();

        case 2:
            Luv.getLuv().readyState();
            return null;                    
        }
        
        return null;
    }

    private void writeEmptyPlexilTestScript(File scriptFile)
        throws IOException {
        FileWriter emptyScript = new FileWriter(scriptFile);
        BufferedWriter out = new BufferedWriter(emptyScript);
        out.write(EMPTY_SCRIPT);
        out.close();                          
    }        

    private final class PlanInfo {
        public String name;
        public File file;
        public Model model;
        public Date lastModified;

        @Override
        public boolean equals(Object o) {
            if (o instanceof PlanInfo)
                return this.equals((PlanInfo) o);
            return super.equals(o);
        }

        // Not full equality; used for search by name
        public boolean equals(PlanInfo other) {
            return this == other
                || (name == null && other.name == null)
                || name.equals(other.name);
        }
    }
    
}
