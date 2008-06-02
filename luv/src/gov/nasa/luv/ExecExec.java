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

import java.lang.Runtime;

import java.io.File;
import java.io.InputStream;

/** Used to run an instance of the Universal Executive. */

public class ExecExec
{
      /** ue process */

      Process ueProcess;
      
      /** thred in which ue process is started */

      Thread runThread;

      /** indicates if ue is running */

      boolean running = false;

      /** Construct a UE executor instance.
       *
       * @param ue the path the universal executive plan runner
       * @param plan the path to the plan to be executed
       * @param script the script to run against the plan
       */

      public ExecExec(File ue, File ueLib, File plan, File script)
      {
         this(ue, ueLib, plan, script, null);
      }

      /** Construct a UE executor instance.
       *
       * @param ue the path the universal executive plan runner
       * @param plan the path to the plan to be executed
       * @param script the script to run against the plan
       * @param libraries libraries to load, may be null
       */

      public ExecExec(File ue, final File ueLib, 
                      File plan, File script, File[] libraries)
      {
         // construct set of arguments which form the exec call

         final Vector<String> args = new Vector<String>();
         args.add(ue.toString());
         args.add("-p"); args.add(plan.toString());
         args.add("-s"); args.add(script.toString());
         if (libraries != null) 
            for (File library: libraries)
            {
               args.add("-l"); 
               args.add(library.toString());
            }
         
         // create a new thread for the ue process

         runThread = new Thread()
            {
                  public void run()
                  {
                     try
                     {
                        Runtime runtime = Runtime.getRuntime();
                        running = true;
                        runtime.loadLibrary(ueLib.toString());
                        ueProcess = runtime
                           .exec(args.toArray(new String[args.size()]));
                        ueProcess.waitFor();
                        running = false;
                     }
                     catch (Exception e)
                     {
                        e.printStackTrace();
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

            // wait until we have an active proces object 
            
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
}
