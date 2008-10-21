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

import java.awt.Color;
import java.util.LinkedList;
import javax.swing.JLabel;

import static java.lang.System.*;

public class StatusMessageHandler
{    
    private LinkedList<StatusMessageHandler> StatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();         // queue of status messages
    
    public static final StatusMessageHandler BLANK_MESSAGE = new StatusMessageHandler(" ", Color.BLACK, 0);

    boolean abortAutoClear = false;
    long    autoClearTime = 0;
    Color   color = Color.BLACK;
    String  message;

    public StatusMessageHandler() {}
    
    public StatusMessageHandler(String message, Color color, long autoClearTime)
    {
       this.message = message;
       this.color = color;
       this.autoClearTime = autoClearTime;
    }   
    
    public void clearStatusMessageQ()
    {
        StatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
    }
    
    public LinkedList getStatusMessageHandlerQ()
    {
        return StatusMessageHandlerQ;
    }
    
    /** Creates and returns the status bar thread.
       *
       * @param statusBar the status bar to create the thread around
       */

      public void startStatusBarThread(final JLabel statusBar)
      {
         new Thread()
         {
               @Override public void run()
               {
                  try
                  {
                     StatusMessageHandler lastMessage = null;
                     
                     while (true)
                     {                       
                        
                        if (!StatusMessageHandlerQ.isEmpty())
                        {
                           // kill any preceeding auto clear thread

                           if (lastMessage != null)
                              lastMessage.abortAutoClear = true;

                           // get the message

                           final StatusMessageHandler message = 
                              StatusMessageHandlerQ.removeFirst();
                           lastMessage = message.autoClearTime > 0 
                              ? message
                              : null;

                           // print to debug window only pertinent messages

                           statusBar.setForeground(message.color);
                           statusBar.setText(message.message);

                           if (!message.message.equals(StatusMessageHandler.BLANK_MESSAGE.message))
                           {                  
                               out.println("STATUS: " + message.message);
                           }

                           // if auto clear requestd start a thread for that

                           if (message.autoClearTime > 0)
                              new Thread()
                              {
                                    @Override public void run()
                                    {
                                       try
                                       {
                                          sleep(message.autoClearTime);
                                          if (!message.abortAutoClear)
                                             statusBar.setText(
                                                StatusMessageHandler.BLANK_MESSAGE.message);
                                       }
                                       catch (Exception e)
                                       {
                                          e.printStackTrace();
                                       }
                                    }
                              }.start();
                        }

                        // wait a bit then check for the next message

                        sleep(50);

                     }
                  }
                  catch (Exception e)
                  {
                  }
               }
         }.start();
      }
    
     /** Add message to status bar.
       *
       * @param message message to add to status bar
       */
        
      public void showStatus(String message)
      {
         showStatus(message, Color.BLACK, 0);
      }

      /** Add message to status bar.
       *
       * @param message message to add to status bar
       * @param autoClearTime milliseconds to wait before clearing the
       * message, a value <= 0 will be left until some other action
       * clears or overwrites this message
       */
        
      public void showStatus(String message, long autoClearTime)
      {
         showStatus(message, Color.BLACK, autoClearTime);
      }

      /** Add message to status bar.
       *
       * @param message message to add to status bar
       * @param color the color of the mesage
       */
        
      public void showStatus(String message, Color color)
      {
         showStatus(message, color, 0);
      }

      /** Add message to status bar.
       *
       * @param message message to add to status bar
       * @param color the color of the mesage
       * @param autoClearTime milliseconds to wait before clearing the
       * message, a value <= 0 will be left until some other action
       * clears or overwrites this message
       */
        
      public void showStatus(String message, Color color, final long autoClearTime)
      {
          if (message.length() > 0)
              StatusMessageHandlerQ.add(new StatusMessageHandler(message, color, autoClearTime));
      }

      /** Clear the status bar. */

      public void clearStatus()
      {
         StatusMessageHandlerQ.add(StatusMessageHandler.BLANK_MESSAGE);
      }
}
