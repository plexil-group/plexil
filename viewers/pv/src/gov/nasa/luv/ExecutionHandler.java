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

import static gov.nasa.luv.Constants.AppType.*;
import static gov.nasa.luv.Constants.DEBUG_CFG_FILE;
import static gov.nasa.luv.Constants.RUN_SIMULATOR;
import static gov.nasa.luv.Constants.RUN_TEST_EXEC;
import static gov.nasa.luv.Constants.RUN_UE_EXEC;
import static gov.nasa.luv.Constants.SIM_SCRIPT;
import static gov.nasa.luv.Constants.TE_SCRIPT;
import static gov.nasa.luv.Constants.UE_EXEC;
import static gov.nasa.luv.Constants.UE_SCRIPT;
import static gov.nasa.luv.Constants.UE_TEST_EXEC;
import static gov.nasa.luv.Constants.UNKNOWN;

import gov.nasa.luv.Luv;
import gov.nasa.luv.CommandGenerator;
import gov.nasa.luv.CommandGenerationException;

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
     * This methods returns a concrete CommandGenerator for the selected mode.
     * If the environment variable ALT_EXECUTIVE is set, returns an instance of the
     * class named to generate the command.
     * @return a plexil command generator.
     */
    // *** FIXME: don't construct a new instance for each call! ***
    @SuppressWarnings("unchecked")
    private CommandGenerator getCommandGenerator() throws CommandGenerationException
    {
        String alternativeExecutive = System.getenv("ALT_EXECUTIVE");
        if (alternativeExecutive != null) {
            Class ecgClass;
            try {    			
				ecgClass = Class.forName(alternativeExecutive);
            } catch (ClassNotFoundException e) {
				throw new CommandGenerationException("The class named by the ALT_EXECUTIVE system variable, "
                                                     + alternativeExecutive + ", doesn't exist.");
			}
            Constructor ecgClCons;
            try {
				ecgClCons = ecgClass.getConstructor(new Class[]{});
			} catch (SecurityException e) {
				throw new CommandGenerationException("Error trying to get constructor for user-defined command generator "
                                                     + alternativeExecutive + ":",
                                                     e);
			} catch (NoSuchMethodException e) {
				throw new CommandGenerationException("Error trying to get constructor for user-defined command generator "
                                                     + alternativeExecutive + ":",
                                                     e);
            }
            Object o;
            try {
                // N.B. IllegalArgumentException shouldn't happen here.
                o = ecgClCons.newInstance(new Object[]{});
			} catch (IllegalArgumentException e) {
				throw new CommandGenerationException("Error trying to create instance of user-defined command generator "
                                                     + alternativeExecutive + ":",
                                                     e);
			} catch (InstantiationException e) {
				throw new CommandGenerationException("Error trying to create instance of user-defined command generator "
                                                     + alternativeExecutive + ":",
                                                     e);
			} catch (IllegalAccessException e) {
				throw new CommandGenerationException("Error trying to create instance of user-defined command generator "
                                                     + alternativeExecutive + ":",
                                                     e);
			} catch (InvocationTargetException e) {
				throw new CommandGenerationException("Error trying to create instance of user-defined command generator "
                                                     + alternativeExecutive + ":",
                                                     e);
			}
            if (o instanceof CommandGenerator)
                return (CommandGenerator) o;
            else
                throw new CommandGenerationException("The class named by the ALT_EXECUTIVE system variable, "
                                                     + alternativeExecutive
                                                     + ", is not a subclass of gov.nasa.luv.CommandGenerator.");
        } else {
            switch (Luv.getLuv().getAppMode()) {
            case PLEXIL_EXEC:
                return new PlexilUniversalExecutiveCommandGenerator();

            case PLEXIL_TEST:
                return new PlexilTestExecutiveCommandGenerator();

            case PLEXIL_SIM:
                return new PlexilSimulatorCommandGenerator();

            case EXTERNAL_APP:
                return null;
            }
        }
        return null; // make f'n compiler happy
    }
      
      
      
    /** Creates the command to execute the Universal Executive.
     * 
     *  @return the command to execute or an error message if the command could not be created.
     */      
    private List<String> createCommand()
        throws IOException, CommandGenerationException
    {
        if (Luv.getLuv().getAppMode() == EXTERNAL_APP)
            return null;

        CommandGenerator pe;
        try {
            pe = getCommandGenerator();
        }
        catch (CommandGenerationException e) {
            throw e;
        }

        try {
            return pe.generateCommand(Luv.getLuv().getSettings());
        }
        catch (CommandGenerationException e) {
            throw e; // *** where is this handled? ***
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

    private class PlexilUniversalExecutiveCommandGenerator
        extends CommandGeneratorBase
        implements CommandGenerator {

        @Override
        public List<String> generateCommand(Settings s) {
            // TODO: Check that required files exist: plan, config, 

            Vector<String> command = new Vector<String>();
  
            System.out.println("Using Universal Executive...");

            //viewer
            command.add(RUN_UE_EXEC);
            command.add("-v");
            //port
            command.add("-n");
            command.add(Integer.toString(s.getPort()));
            //blocking
            if (s.blocksExec())
                command.add("-b");
            //automation to allow PID capture	  
            command.add("-a");

            //debug file		   
            File debug = s.getDebugLocation();
            if (debug != null && debug.exists()) {
                command.add("-d");
                command.add(debug.toString());
            }

            //Check Plan file	  
            if (s.checkPlan())
                command.add("-check");
	  
            // get plan
            command.add("-p");
            command.add(s.getPlanLocation().toString());
	  	  
            if (s.getConfigLocation() != null) {
                command.add("-c");
                command.add(s.getConfigLocation().toString());
            }

            if (s.getLibDirs() != null) {
                for (File ld : s.getLibDirs()) {
                    command.add("-L");
                    command.add(ld.toString());
                }
            }
		  
            if (s.getLibs() != null) {
                for (String lf: s.getLibs()) {
                    command.add("-l");
                    command.add(lf);
                }
            }	  

            return command;
        }

    }

    class PlexilTestExecutiveCommandGenerator
        extends CommandGeneratorBase
        implements CommandGenerator {

        @Override
        public List<String> generateCommand(Settings s) 
        {
            // TODO: Check that required files exist
            Vector<String> command = new Vector<String>();
		  
            System.out.println("Using Test Executive...");
            command.add(RUN_TEST_EXEC);

            //viewer
            command.add("-v");

            //port
            command.add("-n");
            command.add(Integer.toString(s.getPort()));

            //breaks
            if (Luv.getLuv().breaksAllowed())
                command.add("-b");

            //automation to allow PID capture
            command.add("-a");

            //debug file		   
            File debug = s.getDebugLocation();
            if (debug != null && debug.exists()) {
                command.add("-d");
                command.add(debug.toString());
            }

            //Check Plan file	  
            if (Luv.getLuv().checkPlan())
                command.add("-check");
		  
            // get plan
            command.add("-p");
            command.add(s.getPlanLocation().toString());
            command.add("-s");
            command.add(s.getScriptLocation().toString());

            if (s.getLibDirs() != null) {
                for (File ld : s.getLibDirs()) {
                    command.add("-L");
                    command.add(ld.toString());
                }
            }

            if (s.getLibs() != null) {
                for (String lf: s.getLibs()) {
                    command.add("-l");
                    command.add(lf);
                }
            }	  

            return command;
        }	

    }

    class PlexilSimulatorCommandGenerator
        extends CommandGeneratorBase
        implements CommandGenerator {

        @Override
        public List<String> generateCommand(Settings s) {
            // TODO: Check that required files exist

            Vector<String> command = new Vector<String>();	 
  
            System.out.println("Using PlexilSim...");

            //viewer
            command.add(RUN_SIMULATOR);
            command.add("-v");
            //port
            command.add("-n");
            command.add(Integer.toString(s.getPort()));
            //breaks
            if (Luv.getLuv().breaksAllowed())
                command.add("-b");

            //debug file		   
            // TODO: sim can have debug file too!
            File debug = s.getDebugLocation();
            if (debug != null && debug.exists()) {
                command.add("-d");
                command.add(debug.toString());
            }

            //Check Plan file	  
            if (Luv.getLuv().checkPlan())
                command.add("-check");	  
	  
            // get plan
            command.add("-p");
            command.add(s.getPlanLocation().toString());

            File c = s.getConfigLocation();
            if (c != null && c.isFile() && c.canRead()) {
                command.add("-c");
                command.add(c.toString());
            }
	  	  
            File scr = s.getScriptLocation();
            if (scr != null && scr.isFile() && scr.canRead()) {
                command.add("-s");
                command.add(scr.toString());
            }

            if (s.getLibDirs() != null) {
                for (File ld : s.getLibDirs()) {
                    command.add("-L");
                    command.add(ld.toString());
                }
            }

            if (s.getLibs() != null) {
                for (String lf : s.getLibs()) {
                    command.add("-l");
                    command.add(lf);
                }
            }

            return command;
        }

    }
}
