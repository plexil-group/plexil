/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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
import static gov.nasa.luv.Constants.DEBUG_CFG_FILE;
import static gov.nasa.luv.Constants.PLEXIL_EXEC;
import static gov.nasa.luv.Constants.UE_TEST_EXEC;
import static gov.nasa.luv.Constants.UE_EXEC;
import static gov.nasa.luv.Constants.RUN_TEST_EXEC;
import static gov.nasa.luv.Constants.RUN_UE_EXEC;
import static gov.nasa.luv.Constants.UE_SCRIPT;
import static gov.nasa.luv.Constants.TE_SCRIPT;
import static gov.nasa.luv.Constants.RUN_SIMULATOR;
import static gov.nasa.luv.Constants.SIM_SCRIPT;
import static gov.nasa.luv.Constants.UNKNOWN;
import gov.nasa.luv.Luv;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.ProcessBuilder;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import java.util.Vector;

/** The ExecutionHandler class runs an instance of the Universal Executive. */

/**
 * Adapted by Hector Fabio Cadavid Rengifo. hector.cadavid@escuelaing.edu.co
 */
public class ExecutionHandler
{
    private Process execProcess;
    private Thread execMonitorThread;
      
    public ExecutionHandler()
    {
    }

	private void cleanup(Process ue_process) throws IOException
	{
		ue_process.getInputStream().close();
		ue_process.getOutputStream().close();
		ue_process.getErrorStream().close();		
	}
      
    /**
     * @brief Starts the selected PLEXIL Exec and a thread to monitor its output.
     * @return True if the process was launched, false if an error occurred.
     */
 
    public boolean runExec()
    {
        List<String> cmd;
        try {
            cmd = createCommand();
        }
        catch (Exception e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "Error constructing PLEXIL Exec command line");
            return false;
        }

        try {
            execMonitorThread = new Thread(new ExecRunner(cmd));
            execMonitorThread.start();
        }
        catch (Exception e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "Error launching PLEXIL Exec process");
            return false;
        }

        return true;
    }

    /** Stop running the UE. */
    public void stop()
    {
        if (execProcess != null)
            try {
                cleanup(execProcess);
                execProcess.destroy();
                execProcess = null;
            }
            catch (Exception e) {
                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while stopping the Universal Executive");
            }
    	  
    }
      
    /**
     * This methods returns a concrete CommandGenerator. By default, this method returns a standard UE command
     * generator. If a value for "ALT_EXECUTIVE" system variable is given when LUV is started, a different executive command
     * generator will be used (will be used the class name given on such variable).
     * @return a plexil command generator.
     */
    @SuppressWarnings("unchecked")
        private CommandGenerator getPlexilExecutive() throws CommandGenerationException
    {
        String alternativeExecutive=System.getenv("ALT_EXECUTIVE");
        CommandGenerator exec = null;
        if (alternativeExecutive == null) {
            switch (Luv.getLuv().getAppMode()) {
            case	Constants.PLEXIL_EXEC:
                exec = new PlexilUniversalExecutiveCommandGenerator();
                break;
            case	Constants.PLEXIL_TEST:
                exec = new PlexilTestExecutiveCommandGenerator();
                break;
            case	Constants.PLEXIL_SIM:
                exec = new PlexilSimulatorCommandGenerator();
                break;    					
            }    		  
            return exec;
        }
        else {
            try {    			
				Class ecgClass = Class.forName(alternativeExecutive);
				Constructor ecgClCons = ecgClass.getConstructor(new Class[]{});
				Object o = ecgClCons.newInstance(new Object[]{});
				if (o instanceof CommandGenerator) {
					return (CommandGenerator)o;
				}
				else {
					throw new CommandGenerationException("The class given with the ALT_EXECUTIVE system variable:"+alternativeExecutive+", must be an CommandGenerator subclass.");
				}
			} catch (ClassNotFoundException e) {
				throw new CommandGenerationException("The class given with the ALT_EXECUTIVE system variable:"+alternativeExecutive+", doesn't exist.");
			} catch (SecurityException e) {
				throw new CommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (NoSuchMethodException e) {
				throw new CommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (IllegalArgumentException e) {
				throw new CommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (InstantiationException e) {
				throw new CommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (IllegalAccessException e) {
				throw new CommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (InvocationTargetException e) {
				throw new CommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			}
        }    	  
    }
      
      
      
    /** Creates the command to execute the Universal Executive.
     * 
     *  @return the command to execute the Universal Executive or an error message if the command could not be created.
     */      
    private List<String> createCommand()
        throws IOException, CommandGenerationException
    {
        CommandGenerator pe;
        try {
            pe = getPlexilExecutive();
        }
        catch (CommandGenerationException e) {
            throw e;
        }

        Model currentPlan = Luv.getLuv().getCurrentPlan();

        pe.setCurrentPlan(currentPlan);

        // get plan
        if (currentPlan != null && 
            currentPlan.getAbsolutePlanName() != null &&
            !currentPlan.getAbsolutePlanName().equals(UNKNOWN)) {
            if (!new File(currentPlan.getAbsolutePlanName()).exists()) {
                throw new CommandGenerationException("ERROR: unable to identify plan.");
            }
        }
        else
            throw new CommandGenerationException("ERROR: unable to identify plan.");
    	  
        // get supp
        String supp = Luv.getLuv().getExecSelect().getSettings().getSuppName();
        if (currentPlan != null)
            switch(Luv.getLuv().getExecSelect().getMode()) {
            case Constants.PLEXIL_TEST: 
                if (currentPlan.getAbsoluteScriptName() != null &&
                    !currentPlan.getAbsoluteScriptName().equals(UNKNOWN)) {
                    if (new File(currentPlan.getAbsoluteScriptName()).exists()) {
                        pe.setScriptPath(currentPlan.getAbsoluteScriptName()); 
                    }
                    else if (Luv.getLuv().getFileHandler().searchForScript() != null) {
                        pe.setScriptPath(currentPlan.getAbsoluteScriptName());
                    }				  
                    else
                        throw new CommandGenerationException("ERROR: unable to identify " + supp);
                }
                else if (Luv.getLuv().getFileHandler().searchForScript() != null) {
                    pe.setScriptPath(currentPlan.getAbsoluteScriptName());
                }			  	  
                else
                    throw new CommandGenerationException("ERROR: unable to identify " + supp);
	    		break;

            case Constants.PLEXIL_SIM:
            case Constants.PLEXIL_EXEC:
                if (currentPlan.getAbsoluteScriptName() != null &&
                    !currentPlan.getAbsoluteScriptName().equals(UNKNOWN)) {
                    if (new File(currentPlan.getAbsoluteScriptName()).exists()) {
                        pe.setScriptPath(currentPlan.getAbsoluteScriptName()); 
                    }
                } 
                else if (Luv.getLuv().getFileHandler().searchForConfig() != null) {
                    pe.setScriptPath(currentPlan.getAbsoluteScriptName());
                }
                else
                    throw new CommandGenerationException("ERROR: unable to identify " + supp);
                break;
            }

        // get libraries
        if (!currentPlan.getMissingLibraries().isEmpty()) {
            // try to find libraries
            for (String libName : currentPlan.getMissingLibraries()) {
                Model lib = Luv.getLuv().getCurrentPlan().findLibraryNode(libName, true);
                if (lib == null) {
                    throw new CommandGenerationException("ERROR: library \"" + libName + "\" not found.");
                }
                else {
                    currentPlan.linkLibrary(lib);
                }
            }
        }

        if (!currentPlan.getLibraryNames().isEmpty()) {
            Set<String> libFiles = new LinkedHashSet<String>();
            for (String libFile : currentPlan.getLibraryNames()) {
                // double check that library still exists
                if (new File(libFile).exists())
                    libFiles.add(libFile);
                else 
                    throw new CommandGenerationException("ERROR: library file " + libFile + " does not exist.");
            }
            pe.setLibFiles(libFiles);
        }       

        try {
            return pe.generateCommand();
        }
        catch (CommandGenerationException e) {
            throw e;
        }
    }
      
    /** Kills the currently running instance of the Universal Executive. */
      
    public void killUEProcess() throws IOException
    {   
        String killa = "killall ";    	        	      	 
        String kill_ue = "kill " + Luv.getLuv().getPid();    	  
    	  
        try {
            if (Luv.getLuv().getPid() == 0) {
                Runtime.getRuntime().exec(killa + UE_SCRIPT);
                Runtime.getRuntime().exec(killa + UE_EXEC);        		  
                Runtime.getRuntime().exec(killa + TE_SCRIPT);        		  
                Runtime.getRuntime().exec(killa + UE_TEST_EXEC);
                Runtime.getRuntime().exec(killa + SIM_SCRIPT);
            }
            else {
                System.out.println("Killing PID: " + Luv.getLuv().getPid());
                Runtime.getRuntime().exec(kill_ue);
            }
        }
        catch (IOException e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: unable to execute " + kill_ue);
        }
    }

  	/**
  	 * Determine status of Execution Thread.
  	 * 
  	 * @return whether an internal instance is still runnning
  	 */

  	public boolean isAlive() 
    {
  		return execMonitorThread != null
            && execMonitorThread.isAlive();
  	}

    //
    // Helper classes
    //

    private class ExecRunner implements Runnable
    {
        private List<String> commandList;

        public ExecRunner(List<String> cmd)
        {
            commandList = cmd;
        }

        public void run()
        {
            ProcessBuilder builder = new ProcessBuilder(commandList);
            builder.redirectErrorStream(true); // funnel stderr to stdout
            // other environment setup goes here

            StatusMessageHandler msgHandler = Luv.getLuv().getStatusMessageHandler();

            // Launch the process
            try {
                for (String token : commandList) {
                    System.out.print(token);
                    System.out.print(' ');
                }
                System.out.println();

                execProcess = builder.start();
            }
            catch (Exception e) {
                msgHandler.displayErrorMessage(e, 
                                               "ERROR: unable to start PLEXIL Exec process: " + e.toString());
            }

            // Monitor process output (may contain error messages from exec)
            BufferedReader is = new BufferedReader(new InputStreamReader(execProcess.getInputStream()));
            String line;
            boolean pid_seen = false;
            try {
                while ((line = is.readLine()) != null) {
                    System.out.println(line);
                    if (!pid_seen && line.contains("RUN_UE_PID") || line.contains("RUN_TE_PID")) {
                        int pid = Integer.parseInt(line.replaceAll("[^0-9]", ""));
                        System.out.println("THE PID is: " + pid);
                        Luv.getLuv().setPid(pid);
                        pid_seen = true;
                    }
                    else if (line.contains("null interface adapter") && line.contains("command")) {
                        msgHandler.displayErrorMessage(null,
                                                       "an interface configuration xml file is required for handling "
                                                       + line.substring(line.indexOf("command"), line.length()));
                    }
                    else if (line.contains("PINGED") || line.contains("IPC Connected on port 1381")) {
                        // ignore
                    }
                    else if (line.contains("Error")) {            	  
                        msgHandler.displayErrorMessage(null, "ERROR: error reported by the Executive: " + line);
                    }
                }
            }
            catch (Exception e) {
                msgHandler.displayErrorMessage(e,
                                               "ERROR: error in monitoring PLEXIL Exec process: " + e.toString());
            }
            // If we get here, process has ended (or been killed)
            execProcess = null;
        }
    }

    private class CommandGenerationException extends Exception {

        public CommandGenerationException(String message, Throwable cause) {
            super(message, cause);
        }

        public CommandGenerationException(String message) {
            super(message);
        }

        public CommandGenerationException(Throwable cause) {
            super(cause);
        }

    }

    /**
     * 
     * @author Hector Fabio Cadavid Rengifo. hector.cadavid@escuelaing.edu.co
     *
     */
    private abstract class CommandGenerator {
	
        private String scriptPath;

        private boolean breaksAllowed;
	
        private Model currentPlan;
	
        private Set<String> libFiles;
	
        public Set<String> getLibFiles() {
            return libFiles;
        }

        public void setLibFiles(Set<String> libFiles) {
            this.libFiles = libFiles;
        }

        public Model getCurrentPlan() {
            return currentPlan;
        }

        public void setCurrentPlan(Model currentPlan) {
            this.currentPlan = currentPlan;
        }

        public boolean isBreaksAllowed() {
            return breaksAllowed;
        }

        public void setBreaksAllowed(boolean breaksAllowed) {
            this.breaksAllowed = breaksAllowed;
        }

        public String getScriptPath() {
            return scriptPath;
        }

        public void setScriptPath(String scriptPath) {
            this.scriptPath = scriptPath;
        }

        public abstract List<String> generateCommand()
            throws CommandGenerationException;
	
    }

    private class PlexilUniversalExecutiveCommandGenerator extends CommandGenerator
    {

        @Override
            public List<String> generateCommand() 
        {
            Vector<String> command = new Vector<String>();
  
            System.out.println("Using Universal Executive...");

            //viewer
            command.add(RUN_UE_EXEC);
            command.add("-v");  	  
            //port
            command.add("-n");
            command.add(Integer.toString(Luv.getLuv().getPort()));
            //breaks
            if (Luv.getLuv().breaksAllowed())
                command.add("-b");
            //automation to allow PID capture	  
            command.add("-a");
            //debug file		   
            command.add("-d");
            command.add(DEBUG_CFG_FILE);
            //Check Plan file	  
            if (Luv.getLuv().checkPlan())
                command.add("-check");
	  
            // get plan

            Model currentPlan = this.getCurrentPlan();
            command.add("-p");
            command.add(currentPlan.getAbsolutePlanName());
	  	  
            if (this.getScriptPath() != null) {
                command.add("-c");
                command.add(this.getScriptPath());
            }
	  
            if (this.getLibFiles() != null){
                for (String lf : this.getLibFiles()) {
                    command.add("-l");
                    command.add(lf);
                }
            }	  

            return command;
        }

    }

    class PlexilTestExecutiveCommandGenerator extends CommandGenerator
    {

        @Override
            public List<String> generateCommand() 
        {
            Vector<String> command = new Vector<String>();
		  
            System.out.println("Using Test Executive...");
            command.add(RUN_TEST_EXEC);

            //viewer
            command.add("-v");

            //port
            command.add("-n");
            command.add(Integer.toString(Luv.getLuv().getPort()));

            //breaks
            if (Luv.getLuv().breaksAllowed())
                command.add("-b");

            //automation to allow PID capture
            command.add("-a");

            //debug file		   
            command.add("-d");
            command.add(DEBUG_CFG_FILE);	  	  

            //Check Plan file	  
            if (Luv.getLuv().checkPlan())
                command.add("-check");
		  
            // get plan
            Model currentPlan=this.getCurrentPlan();
		  
            command.add("-p");
            command.add(currentPlan.getAbsolutePlanName());
            command.add("-s");
            command.add(this.getScriptPath());
		  
            if (this.getLibFiles() != null) {
                for (String lf:this.getLibFiles()) {
                    command.add("-l");
                    command.add(lf);
                }
            }	  

            return command;
        }	

    }

    class PlexilSimulatorCommandGenerator extends CommandGenerator 
    {
        @Override
            public List<String> generateCommand() 
        {
            Vector<String> command = new Vector<String>();	 
  
            System.out.println("Using PlexilSim...");

            //viewer
            command.add(RUN_SIMULATOR);
            command.add("-v");
            //port
            command.add("-n");
            command.add(Integer.toString(Luv.getLuv().getPort()));
            //breaks
            if (Luv.getLuv().breaksAllowed())
                command.add("-b");
            //debug file		   
            command.add("-d");
            command.add(DEBUG_CFG_FILE);
            //Check Plan file	  
            if (Luv.getLuv().checkPlan())
                command.add("-check");	  
	  
            // get plan
            Model currentPlan = this.getCurrentPlan();
            command.add("-p");
            command.add(currentPlan.getAbsolutePlanName());
	  	  
            if (this.getScriptPath() != null) {
                command.add("-s");
                command.add(this.getScriptPath());
            }
	  
            if (this.getLibFiles() != null) {
                for (String lf : this.getLibFiles()) {
                    command.add("-l");
                    command.add(lf);
                }
            }

            return command;
        }

    }
}