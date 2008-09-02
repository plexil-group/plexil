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

import java.util.Vector;

import static java.lang.System.*;
import static gov.nasa.luv.Constants.*;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

/** Used to run an instance of the Universal Executive. */

public class ExecutionViaLuvViewerHandler
{
      /** Vector holds command line argument */
          
      final Vector<String> args = new Vector<String>();
          
      /** ue process */

      Process ueProcess;
      
      /** thred in which ue process is started */

      Thread runThread;

      /** indicates if ue is running */

      boolean running = false;
      
      ExecutionViaLuvViewerHandler() {}

      /** Construct a UE executor instance.
       *
       * @param ue the path the universal executive plan runner
       * @param plan the path to the plan to be executed
       * @param script the script to run against the plan
       * @param libraries libraries to load, may be null
       */

      public ExecutionViaLuvViewerHandler(File ue, File plan, File script)
      {
          // populate Vector with command line arguments
          
          setArgs(ue, plan, script);
          
          // create a new thread for the ue process
          
          runThread = new Thread()
          {
            @Override
              public void run()
              {
                  try
                  {
                      Runtime runtime = Runtime.getRuntime();
                      running = true;
                      ueProcess = runtime.exec(args.toArray(new String[args.size()]));
                      running = false;
                  }
                  catch (Exception e)
                  {
                      out.println(e.getMessage());
                  }
              }
          };
      }

      /** Start running the UE. */

      public InputStream start()
      {
         try
         {
            // start the ue
            
            runThread.start();

            // wait until we have an active process object 
            
            while (ueProcess == null)
               Thread.sleep(10);
            
         }
         catch (Exception e)
         {
            e.printStackTrace();
         }
         
         // then return the input stream for the process

         return ueProcess.getInputStream();
      }

      /** Get the input stream for the ue process.
       *
       * @return the stream to witch the ue process send its output
       */

      public InputStream getInputStream()
      {
         return ueProcess != null 
            ? ueProcess.getInputStream()
            : null;
      }

      /** Get the error stream for the ue process.
       *
       * @return the stream to witch the ue process send its errors
       */

      public InputStream getErrorStream()
      {
         return ueProcess != null 
            ? ueProcess.getErrorStream()
            : null;
      }

      /** Check to see if the UE is still running.
       *
       * @return true if the ue is still running, false if not
       */

      public boolean isRunning()
      {
         return running;
      }
      
      public void setArgs (File ue, File plan, File script)
      {
          args.add(ue.toString());
          
          args.add("-v");
          
          if (Luv.getLuv().getBoolean(ALLOW_BREAKS))
              args.add("-b");
          
          args.add("-p");
          args.add(plan.toString());
          
          args.add("-s");
          args.add(script.toString());
          
          Vector<String> libNames = Luv.getLuv().getModel().getLibraryNames();
          
          if (libNames.size() > 0)
          {
              for (String libName : libNames)
              {
                  args.add("-l");
                  args.add(libName.toString());
              }
          }
      }
      
      public void runExec() throws IOException
      {                    
        File ue = new File(PROP_UE_EXEC);         
        Luv.getLuv().getFileHandler().setCurrentFile(PLAN);
        Luv.getLuv().setBoolean(DONT_LOAD_SCRIPT_AGAIN, false);
        Luv.getLuv().getFileHandler().setCurrentFile(SCRIPT);

        if (Luv.getLuv().getFileHandler().getCurrentFile(PLAN) != null && 
            Luv.getLuv().getFileHandler().getCurrentFile(SCRIPT) != null)
        {
            ExecutionViaLuvViewerHandler ee = new ExecutionViaLuvViewerHandler(ue, 
                                                                               Luv.getLuv().getFileHandler().getCurrentFile(PLAN), 
                                                                               Luv.getLuv().getFileHandler().getCurrentFile(SCRIPT));

             try
             {
                InputStream is = ee.start();
                InputStream es = ee.getErrorStream();
                while (ee.isRunning())
                {
                  while (is.available() > 0)
                      out.write(is.read());
                  while (es.available() > 0)
                      err.write(es.read());
                }
                while (is.available() > 0)
                   out.write(is.read());
                while (es.available() > 0)
                   err.write(es.read());
                out.flush();
             }
             catch (Exception e)
             {
                e.printStackTrace();
             }
        }
      }
}
