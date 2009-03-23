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
import java.awt.Font;
import java.util.LinkedList;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import static java.lang.System.*;

public class StatusMessageHandler
{   
    
    private static final StatusMessageHandler BLANK_MESSAGE = 
            new StatusMessageHandler(" ", Color.BLACK, 0);
    private static JLabel statusBar;
    
    // queue of status messages
    private LinkedList<StatusMessageHandler> StatusMessageHandlerQ;                     
    private boolean abortAutoClear;
    private long autoClearTime;
    private Color color;
    private String message;

    public StatusMessageHandler() 
    {
        StatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
        autoClearTime = 0;
        color = Color.BLACK;
        abortAutoClear = false;
    }
    
    public StatusMessageHandler(String message, Color color, long autoClearTime)
    {
        StatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
        this.message = message;
        this.color = color;
        this.autoClearTime = autoClearTime;
        abortAutoClear = false;
    }   
    
    public void clearStatusMessageQ()
    {
        StatusMessageHandlerQ.clear();
    }
    
    public LinkedList getStatusMessageHandlerQ()
    {
        return StatusMessageHandlerQ;
    }
    
    // creates and returns the status bar thread.
    public void startStatusBarThread(final JLabel statusBar)
    {
         this.statusBar = statusBar;
         statusBar.setFont(statusBar.getFont().deriveFont(Font.PLAIN, 12.0f));
          
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

                           if (!message.message.equals(BLANK_MESSAGE.message))
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
                                             statusBar.setText(BLANK_MESSAGE.message);
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
    
    // message to status bar only, not debug window
    public void showStatusOnBarOnly(String message)
    {
        statusBar.setForeground((Color.BLACK));
        statusBar.setText(message);
    }
    
    // add message to status bar (with message only)
    public void showStatus(String message)
    {
         showStatus(message, Color.BLACK, 0);
    }

    // add message to status bar (with message and time)
    public void showStatus(String message, long autoClearTime)
    {
        showStatus(message, Color.BLACK, autoClearTime);
    }

    // add message to status bar (with message and color)
    public void showStatus(String message, Color color)
    {
       showStatus(message, color, 0);
    }

    // add message to status bar (with message, color and time)
    public void showStatus(String message, Color color, final long autoClearTime)
    {
        if (message.length() > 0)
            StatusMessageHandlerQ.add(new StatusMessageHandler(message, color, autoClearTime));
    }
      
    public void displayErrorMessage(Exception e, String errorMessage)
    {
        if (e != null)
        {
            JOptionPane.showMessageDialog(Luv.getLuv(), 
                                          errorMessage + ". Please see Debug Window.", 
                                          "Error", 
                                          JOptionPane.ERROR_MESSAGE);
  
            System.err.println("ERROR: " + e.getMessage());
        }
        else
        {
            JOptionPane.showMessageDialog(Luv.getLuv(), 
                                          errorMessage, 
                                          "Error", 
                                          JOptionPane.ERROR_MESSAGE);

            System.err.println(errorMessage);
        }
    }
    
    public void displayInfoMessage(String infoMessage)
    {
        JOptionPane.showMessageDialog(Luv.getLuv(),
                                      infoMessage,
                                      "Stopping Execution",
                                      JOptionPane.INFORMATION_MESSAGE);
    }

    // clear the status bar
    public void clearStatus()
    {
        StatusMessageHandlerQ.add(BLANK_MESSAGE);
    }
}
