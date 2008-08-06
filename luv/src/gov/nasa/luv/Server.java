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

import gov.nasa.luv.Luv.LinkCanceledException;
import java.io.ByteArrayInputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.InputStream;
import java.io.OutputStream;
import java.awt.Color;

import static gov.nasa.luv.Constants.*;

/** Functions as a server for plan event data clients (UEs). */

public class Server       
{
      /** Construct a server which listens on a given port.
       *
       * @param port port on witch this server listens. 
       */

      public Server(final int port)
      {
         // create a thread which listens for events

         new Thread()
         {
               public void run()
               {
                  accept(port);
               }
         }.start();
      }

      /** Wait for ue clients to connect on a given port.  This method
       * blocks indefinately and spins off threads for each connection.
       *
       * @param port port on witch this server listens.
       */

      public void accept(int port)
      {
         try
         {
            ServerSocket ss = new ServerSocket(port);

            while (true)
            {
               handleConnection(ss.accept());
            }
         }
         catch (Exception e)
         {
            e.printStackTrace();
         }
      }

      /** Handle a new connection being opened by a remote system.  This
       * method spins off a thread which dispatches incomming data.
       * Override this method to catch new connection events.
       *
       * @param s the socket of the new connection
       */

      public void handleConnection(final Socket s)
      {
         new Thread()
         {
               public void run()
               {
                  dispatchInput(s);
               }
         }.start();
      }

      /** Indentifies where messages begin and end and dispatches those
       * messages to the message handler.
       *
       * @param s socket from witch input issues forth
       */

      public void dispatchInput(Socket s)
      {
         try
         {
            // get the input stream for this socket and setup a message buffer
            
            InputStream is = s.getInputStream();
            OutputStream os = s.getOutputStream();
            StringBuilder message = new StringBuilder();

            Luv.pauseAtStart = true;
            Luv.isExecuting = true;

            // now just loop forever

            while (true)
            {
               // if there is input, grab it up

               while (is.available() > 0)
               {
                  // if we see the end of message char, dispatch message

                  int ch = is.read();
                  if (ch == END_OF_MESSAGE)
                  {                  
                     // handle the message                       

                     handleMessage(message.toString());
                     message = new StringBuilder();
                     
                     // if the viewer is should block, do so after message handled

                     if (doesViewerBlock())
                     {
                        os.write(END_OF_MESSAGE);
                     }
                  }

                  // otherwise append char to message

                  else
                     message.append((char)ch);
               } 
               
               // sleep for a bit while we wait for data to come in

               Thread.sleep(100);
            }            
         }
         catch (Exception e)
         {
            e.printStackTrace();
         }
      }
      
      public void handleMessage(final String message)
      {  

        /** Determine if the Luv Viewer should pause before executing. */

          if (Luv.pauseAtStart)
          {
              if (!Luv.executedViaLuvViewer)
              {
                  Boolean reset = false;
                  Luv.getLuv().planPaused = true;
                  if (Luv.getLuv().model.getProperty(VIEWER_BLOCKS) == null ||
                      Luv.getLuv().model.getProperty(VIEWER_BLOCKS).equals(FALSE) ||
                      Luv.getLuv().model.getProperty(VIEWER_BLOCKS).equals("false"))
                  {
                      Luv.getLuv().model.setProperty(VIEWER_BLOCKS, TRUE);
                      reset = true;
                  }

                  doesViewerBlock();                          

                  if (reset)
                      Luv.getLuv().model.setProperty(VIEWER_BLOCKS, FALSE);
              }
              Luv.pauseAtStart = false;
              Luv.executedViaLuvViewer = false;
          }

         // parse the message

         boolean isPlan = Luv.getLuv().parseXml(new ByteArrayInputStream(
                                      message.getBytes()), Luv.getLuv().model);

         // if this is a plan (or possibly a library)

         if (isPlan)
         {
            // if this is a library, store this in set of libraries

            if (Luv.getLuv().outstandingLibraryCount > 0)
            {
               Luv.getLuv().libraries.add(Luv.getLuv().model.removeChild(NODE));
               --Luv.getLuv().outstandingLibraryCount;
            }

            // otherwise it's a plan, link that plan and
            // libraries and show the new plan

            else
            {
               try
               {
                  Luv.getLuv().link(Luv.getLuv().model, Luv.getLuv().libraries);
               }
               catch (LinkCanceledException lce) {}
               Luv.getLuv().resetView();
               Luv.getLuv().libraries.clear();
            }
         }
      }

      public boolean doesViewerBlock()
      {
         String blocksStr = Luv.getLuv().model.getProperty(VIEWER_BLOCKS);
         boolean blocks = blocksStr != null 
            ? Boolean.valueOf(blocksStr)
            : false;

         if (blocks && Luv.getLuv().planPaused && !Luv.getLuv().planStep) 
         {
            Luv.statusBar.showStatus((Luv.breakPointHandler.breakPoint == null
                        ? "Plan execution paused."
                        : Luv.breakPointHandler.breakPoint.getReason()) +
               "  Hit " + 
               Luv.getLuv().pauseAction.getAcceleratorDescription() +
               " to resume, or " + 
               Luv.getLuv().stepAction.getAcceleratorDescription() +
               " to step.",
               Color.RED);
            Luv.breakPointHandler.breakPoint = null;

            while (Luv.getLuv().planPaused && !Luv.getLuv().planStep)
            {
               try
               {
                  Thread.sleep(50);
               }
               catch (Exception e)
               {
                  e.printStackTrace();
               }
            }
         }

         Luv.getLuv().planStep = false;

         return blocks;
      }
}
