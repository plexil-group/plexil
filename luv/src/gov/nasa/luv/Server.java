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

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.io.InputStream;
import java.io.OutputStream;

import static gov.nasa.luv.Constants.*;
import java.util.logging.Level;
import java.util.logging.Logger;

/** Functions as a server for plan event data clients (UEs). */

public abstract class Server
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
                    try 
                    {
                        dispatchInput(s);
                    } 
                    catch (InterruptedException ex) 
                    {
                        System.out.println(ex);
                    }
                    catch (IOException ex) 
                    {
                        System.out.println(ex);
                    }
               }
         }.start();
      }

      /** Indentifies where messages begin and end and dispatches those
       * messages to the message handler.
       *
       * @param s socket from witch input issues forth
       */
      
      public void dispatchInput(Socket s) throws IOException, InterruptedException
      {            
          // get the input stream for this socket and setup a message buffer
          InputStream is = s.getInputStream();
          OutputStream os = s.getOutputStream();
          
          StringBuilder fullMessage = null;
          int numOfBytesRead = 0;
          int numOfBytesThatCouldbeRead = 0;
          
          while (true) 
          {
              while (is.available() > 0)
              {  
                 numOfBytesThatCouldbeRead = is.available();
                 byte[] partialMessage = new byte[numOfBytesThatCouldbeRead];
                 numOfBytesRead = is.read(partialMessage);
                 
                 if (fullMessage == null)
                    fullMessage = new StringBuilder(numOfBytesRead);
                 for (int i = 0; i < numOfBytesRead; i++)
                 {
                     if (partialMessage[i] == END_OF_MESSAGE)
                     {
                         handleMessage(fullMessage.toString());    
                         if (i == (numOfBytesRead - 1))
                             fullMessage = null;
                         else
                             fullMessage = new StringBuilder(numOfBytesRead - i - 1);
                         
                         if (doesViewerBlock())
                         {
                            os.write(END_OF_MESSAGE);
                         }
                     }
                     else
                         fullMessage.append((char)partialMessage[i]);
                 }
              } 
              
              Thread.sleep(100);
          } 
      }
      
      public abstract void handleMessage(final String message);

      public abstract boolean doesViewerBlock();
}
