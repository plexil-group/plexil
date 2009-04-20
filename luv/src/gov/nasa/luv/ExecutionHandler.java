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
import java.io.*;
import static gov.nasa.luv.Constants.*;

/** The ExecutionHandler class runs an instance of the Universal Executive. */

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
          System.out.println(command);
          
          if (!command.contains("ERROR")) 
          {   
              ee = new ExecutionHandler(command);
              ee.start();
              return true;
          } 
          
          Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, command);
          
          return false;
      }
      
    /** Creates the command to execute the Universal Executive.
     * 
     *  @return the command to execute the Universal Executive or an error message if the command could not be created.
     */
      
      private String createCommandLine() throws IOException
      {
          Model currentPlan = Luv.getLuv().getCurrentPlan();
          String command = UE_EXEC + " -v";

	  if (Luv.getLuv().breaksAllowed())
	      command += " -b"; 
          
          command += " -d " + DEBUG_CFG_FILE;
  
          // get plan
        
	  if (currentPlan != null && 
              currentPlan.getAbsolutePlanName() != null &&
              !currentPlan.getAbsolutePlanName().equals(UNKNOWN))
          {
              if (new File(currentPlan.getAbsolutePlanName()).exists())
              {
                  command += " " + currentPlan.getAbsolutePlanName(); 
              }
              else
                  return "ERROR: unable to identify plan.";
          }
          else
              return "ERROR: unable to identify plan.";
        
          // get script
        
          if (currentPlan != null &&
              currentPlan.getAbsoluteScriptName() != null &&
              !currentPlan.getAbsoluteScriptName().equals(UNKNOWN))
          {
              if (new File(currentPlan.getAbsoluteScriptName()).exists())
              {
                  command += " " + currentPlan.getAbsoluteScriptName(); 
              }
              else if (Luv.getLuv().getFileHandler().searchForScript() != null)
              {
                  command += " " + currentPlan.getAbsoluteScriptName();
              }
              else
                  return "ERROR: unable to identify script.";
          }
          else if (Luv.getLuv().getFileHandler().searchForScript() != null)
          {
              command += " " + currentPlan.getAbsoluteScriptName();
          }
          else
              return "ERROR: unable to identify script.";
             
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
              for (String libFile : currentPlan.getLibraryNames()) 
              {
                  // double check that library still exists
                  if (new File(libFile).exists()) 
                  {
                      command += " -l ";
                      command += libFile;
                  }
                  else 
                  {
                      return "ERROR: library file " + libFile + " does not exist.";
                  }
              }
          }       

	  return command;
      }
      
    /** Kills the currently running instance of the Universal Executive. */
      
      public void killUEProcess() throws IOException
      {
          String kill_ue = "killall " + TEST_EXEC;
            
          try 
          {
              Runtime.getRuntime().exec(kill_ue);
          }
          catch (IOException e) 
          {
              Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: unable to kill " + TEST_EXEC + " process");
          }
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
                  Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: error reported by the Universal Executive");
              }

              System.out.println(line);             
          }

          // display standard error message from process if any
          while ((line = err.readLine()) != null)
          {    
              Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: error reported by the Universal Executive");
              System.out.println(line);
          }
      }
}
