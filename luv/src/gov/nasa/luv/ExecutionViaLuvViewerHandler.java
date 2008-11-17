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

/** Used to run an instance of the Universal Executive. */

public class ExecutionViaLuvViewerHandler
{
      private ExecutionViaLuvViewerHandler ee;   
      private Runtime runtime;
      private Thread runThread;
      
      public ExecutionViaLuvViewerHandler() {}

      public ExecutionViaLuvViewerHandler(final String command)
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
                      Luv.getLuv().displayErrorMessage(e, "ERROR: exception occurred while executing plan");
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
             Luv.getLuv().displayErrorMessage(e, "ERROR: exception occurred while starting the Universal Executive");
         }
      }
      
      public void runExec(String command) throws IOException
      {
          ee = new ExecutionViaLuvViewerHandler(command);
          ee.start();
      }
      
      public void killUEProcess() throws IOException
      {
          String kill_ue = "killall " + TEST_EXEC;
            
          try 
          {
              Runtime.getRuntime().exec(kill_ue);
          }
          catch (IOException e) 
          {
              Luv.getLuv().displayErrorMessage(e, "ERROR: unable to kill " + TEST_EXEC + " process");
          }
      }
      
      private void displayProcessMessagesToDebugWindow(Process ue_process) throws IOException
      {
          BufferedReader is = new BufferedReader(new InputStreamReader(ue_process.getInputStream()));
          BufferedReader err = new BufferedReader(new InputStreamReader(ue_process.getErrorStream()));                    
          String line;
          String isMessage = "";
          String errMessage = "";

           // display standard output from process (may contain an error message from UE)
          while ((line = is.readLine()) != null)
          {
              if (line.contains("Error"))
              {
                  Luv.getLuv().displayErrorMessage(null, "ERROR: error reported by the Universal Executive");
              }

              isMessage += "\n" + line;
          }

          System.out.println(isMessage);

          // display standard error message from process if any
          while ((line = err.readLine()) != null)
          {                                     
              errMessage += line + "\n";
          }

          if (!errMessage.equals("") &&
              !errMessage.contains(UE_TERMINATE_EXEC_MESSAGE))
          {
              Luv.getLuv().displayErrorMessage(null, "ERROR: error reported by the Universal Executive");

              System.out.println("\n" + errMessage);
          }
      }
}
