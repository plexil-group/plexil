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
import static gov.nasa.luv.Constants.DEBUG_CFG_FILE;
import static gov.nasa.luv.Constants.TEST_EXEC;
import static gov.nasa.luv.Constants.UE_TEST_EXEC;
import static gov.nasa.luv.Constants.UE_EXEC;
import static gov.nasa.luv.Constants.RUN_TEST_EXEC;
import static gov.nasa.luv.Constants.RUN_UE_EXEC;

import static gov.nasa.luv.Constants.UNKNOWN;
import gov.nasa.luv.Luv;
import gov.nasa.luv.runtime.AbstractPlexilExecutiveCommandGenerator;
import gov.nasa.luv.runtime.ExecutiveCommandGenerationException;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.LinkedHashSet;
import java.util.Set;

/** The ExecutionHandler class runs an instance of the Universal Executive. */

/**
 * Adapted by Hector Fabio Cadavid Rengifo. hector.cadavid@escuelaing.edu.co
 */
public class ExecutionHandler
{
      private ExecutionHandler ee;   
      private Runtime runtime;
      private Thread runThread;
      
      public ExecutionHandler() {}
      
    /** Construct an ExecutionHandler. 
     *
     * @param command the command that executes the Universal Exective
     */

      public ExecutionHandler(final String command)
      {        
          // create a new thread for the ue process
          
          runThread = new Thread()
          {
            @Override
              public void run()
              {
                  try
                  {
                      runtime = Runtime.getRuntime();
                      Process ue_process = runtime.exec(command);
                      
                      displayProcessMessagesToDebugWindow(ue_process);                                          
                  }
                  catch(Exception e)
                  {
                      Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while executing plan");
                  }
              }
          };
      }

      /** Start running the UE. */

      private void start()
      {
         try
         {    
            runThread.start();          
         }
         catch (Exception e)
         {
             Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while starting the Universal Executive");
         }
      }
      
    /** Creates an instance of an ExecutionHandler.
     * 
     *  @return whether an instance of an ExecutionHandler was created
     */
 
      public boolean runExec() throws IOException
      {
          String command = createCommandLine();          
          
          if (!command.contains("ERROR")) 
          {   
              ee = new ExecutionHandler(command);
              ee.start();
              return true;
          } 
          
          Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, command);
          
          return false;
      }
      
      /**
       * This methods returns a concrete PlexilExecutiveCommandGenerator. By default, this method returns a standard UE command
       * generator. If a value for "ALT_EXECUTIVE" system variable is given when LUV is started, a different executive command
       * generator will be used (will be used the class name given on such variable).
       * @return a plexil command generator.
       */
      private AbstractPlexilExecutiveCommandGenerator getPlexilExecutive() throws ExecutiveCommandGenerationException{
    	  String alternativeExecutive=System.getProperty("ALT_EXECUTIVE");    	  
    	  if (alternativeExecutive==null){
    		  return new PlexilUniversalExecutive();
    	  }
    	  else{
    		  try {    			
				Class ecgClass=Class.forName(alternativeExecutive);
				Constructor ecgClCons=ecgClass.getConstructor(new Class[]{});
				Object o=ecgClCons.newInstance(new Object[]{});
				if (o instanceof AbstractPlexilExecutiveCommandGenerator){
					return (AbstractPlexilExecutiveCommandGenerator)o;
				}
				else{
					throw new ExecutiveCommandGenerationException("The class given with the ALT_EXECUTIVE system variable:"+alternativeExecutive+", must be an AbstractPlexilExecutiveCommandGenerator subclass.");
				}
			} catch (ClassNotFoundException e) {
				throw new ExecutiveCommandGenerationException("The class given with the ALT_EXECUTIVE system variable:"+alternativeExecutive+", doesn't exist.");
			} catch (SecurityException e) {
				throw new ExecutiveCommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (NoSuchMethodException e) {
				throw new ExecutiveCommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (IllegalArgumentException e) {
				throw new ExecutiveCommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (InstantiationException e) {
				throw new ExecutiveCommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (IllegalAccessException e) {
				throw new ExecutiveCommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			} catch (InvocationTargetException e) {
				throw new ExecutiveCommandGenerationException("Error when trying to create an instance of the given ALT_EXECUTIVE system variable:"+alternativeExecutive+".");
			}
    	  }    	  
      }
      
      
      
      /** Creates the command to execute the Universal Executive.
       * 
       *  @return the command to execute the Universal Executive or an error message if the command could not be created.
       */      
      private String createCommandLine() throws IOException
      {
    	  AbstractPlexilExecutiveCommandGenerator pe;
    	  try {
    		  pe = getPlexilExecutive();
    	  } catch (ExecutiveCommandGenerationException e) {
    		  return "ERROR: unable to load alternative plexil executive:"+e.getMessage();	
    	  }

    	  Model currentPlan = Luv.getLuv().getCurrentPlan();

    	  pe.setCurrentPlan(currentPlan);


    	  if (currentPlan != null && 
    			  currentPlan.getAbsolutePlanName() != null &&
    			  !currentPlan.getAbsolutePlanName().equals(UNKNOWN))
    	  {
    		  if (!new File(currentPlan.getAbsolutePlanName()).exists())
    		  {
    			  return "ERROR: unable to identify plan."; 
    		  }

    	  }
    	  else
    		  return "ERROR: unable to identify plan.";    	      	 
    	  
    	  // get script
    	  if (Luv.getLuv().allowTest())
    	  {
			  if (currentPlan != null &&
					  currentPlan.getAbsoluteScriptName() != null &&
					  !currentPlan.getAbsoluteScriptName().equals(UNKNOWN))
			  {
				  if (new File(currentPlan.getAbsoluteScriptName()).exists())
				  {
					  pe.setScriptPath(currentPlan.getAbsoluteScriptName()); 
				  }
				  else if (Luv.getLuv().getFileHandler().searchForScript() != null)
				  {
					  pe.setScriptPath(currentPlan.getAbsoluteScriptName());
				  }				  
				  else
					  return "ERROR: unable to identify script.";
			  }			  
			  else if (Luv.getLuv().getFileHandler().searchForScript() != null)
			  {
				  pe.setScriptPath(currentPlan.getAbsoluteScriptName());
			  }			  	  
			  else
				  return "ERROR: unable to identify script.";
			  
    	  }else if (currentPlan != null &&
				  currentPlan.getAbsoluteScriptName() != null &&
				  !currentPlan.getAbsoluteScriptName().equals(UNKNOWN))
				  {
		    		  if (!new File(currentPlan.getAbsoluteScriptName()).exists())
					  {
						  return "ERROR: unable to identify config."; 
					  }
				  }else if (Luv.getLuv().getFileHandler().searchForConfig() != null)
				  {
					  pe.setScriptPath(currentPlan.getAbsoluteScriptName());
				  }
	    	  else
	    		  return "ERROR: unable to identify config.";
    		  
    	  // get config

    	  // get libraries

    	  if (!currentPlan.getMissingLibraries().isEmpty()) {
    		  // try to find libraries
    		  for (String libName : currentPlan.getMissingLibraries()) {
    			  Model lib = Luv.getLuv().getCurrentPlan().findLibraryNode(libName, true);
    			  if (lib == null) 
    			  {
    				  return "ERROR: library \"" + libName + "\" not found.";
    			  }
    			  else 
    			  {
    				  currentPlan.linkLibrary(lib);
    			  }
    		  }
    	  }

    	  if (!currentPlan.getLibraryNames().isEmpty()) 
    	  {
    		  Set<String> libFiles=new LinkedHashSet<String>();
    		  for (String libFile : currentPlan.getLibraryNames()) 
    		  {
    			  // double check that library still exists
    			  if (new File(libFile).exists()) 
    			  {
    				  libFiles.add(libFile);
    			  }
    			  else 
    			  {
    				  return "ERROR: library file " + libFile + " does not exist.";
    			  }
    		  }
    		  pe.setLibFiles(libFiles);
    	  }       

    	  try {
    		  return pe.generateCommandLine();
    	  } catch (ExecutiveCommandGenerationException e) {
    		  return "ERROR: unable to create executive command line:"+e.getMessage();
    	  }
      }
      
    /** Kills the currently running instance of the Universal Executive. */
      
      public void killUEProcess() throws IOException
      {    	      	  
    	  String kill_ue = "killall ";
    	  kill_ue += Luv.getLuv().allowTest() ? UE_TEST_EXEC : UE_EXEC;      	  
    	  
          //String kill_ue = "killall " + TEST_EXEC;
            
          try 
          {
              Runtime.getRuntime().exec(kill_ue);
          }
          catch (IOException e) 
          {
              Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: unable to execute " + kill_ue);
          }
      }

  	/**
  	 * Determine status of Execution Thread.
  	 * 
  	 * @return whether an internal instance is still runnning
  	 */

  	public boolean isAlive() {
  		return runThread.isAlive();
  	}      
      
      private void displayProcessMessagesToDebugWindow(Process ue_process) throws IOException
      {
          BufferedReader is = new BufferedReader(new InputStreamReader(ue_process.getInputStream()));
          BufferedReader err = new BufferedReader(new InputStreamReader(ue_process.getErrorStream()));                    
          String line;

          // display standard output from process (may contain an error message from UE)
          while ((line = is.readLine()) != null)
          {        	
              if (line.contains("Error"))
              {
                  Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: error reported by the Universal Executive, see debug window");                  
              }              
              
              System.out.println(line);             
          }

          // display standard error message from process if any
          while ((line = err.readLine()) != null)
          {    
              Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: error reported by the Universal Executive, see debug window");
              System.out.println(line);
        	  if (line.contains("null interface adapter") && line.contains("command"))
        		  Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "an interface configuration xml file is required for handling " + line.substring(line.indexOf("command"), line.length()));        	  
          }
      }
}



class PlexilUniversalExecutive extends AbstractPlexilExecutiveCommandGenerator{

	@Override
	public String generateCommandLine() {
	  String command = "";	  
	  System.out.println(Luv.getLuv().allowTest() ? "Using Test Executive..." : "Using Universal Executive...");
	  command = Luv.getLuv().allowTest() ? RUN_TEST_EXEC + " -v" : RUN_UE_EXEC + " -v";  	  

	  if (Luv.getLuv().breaksAllowed())
		  command += " -b";
	  		   
	  command += " -d " + DEBUG_CFG_FILE;

	  // get plan

	  Model currentPlan=this.getCurrentPlan();
	  
	  command += Luv.getLuv().allowTest() ? " " + currentPlan.getAbsolutePlanName() : 
		  " -p " + currentPlan.getAbsolutePlanName();
	  
	  if (Luv.getLuv().allowTest())
		  command += " " + this.getScriptPath();
	  else if(!Luv.getLuv().allowTest() && this.getScriptPath() != null)
		  command += " -c " + this.getScriptPath();	  

	  if (this.getLibFiles()!=null){
		  for (String lf:this.getLibFiles()){
			  command += " -l ";
			  command += lf;		  			  
		  }
	  }	  
	  ///command status
	  System.out.println(command);
	  return command;

	}

}
