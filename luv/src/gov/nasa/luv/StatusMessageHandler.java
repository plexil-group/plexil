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

/**
 * The StatusMessageHandler class provides methods to display status messages 
 * to the status bar at the bottom of the Luv application and to the Debug Window.
 */

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

    /**
     * Constructs a StatusMessageHandler.
     */
    public StatusMessageHandler() 
    {
        StatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
        autoClearTime = 0;
        color = Color.BLACK;
        abortAutoClear = false;
    }
    
    /**
     * Constructs a StatusMessageHandler with the specified message, color and 
     * amount of time it will be displayed.
     * 
     * @param message the message displayed
     * @param color the color the message will be displayed in
     * @param autoClearTime the amount of time message will be displayed
     */
    public StatusMessageHandler(String message, Color color, long autoClearTime)
    {
        StatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
        this.message = message;
        this.color = color;
        this.autoClearTime = autoClearTime;
        abortAutoClear = false;
    }   
    
    /**
     * Clears the Queue of messages.
     */
    public void clearStatusMessageQ()
    {
        StatusMessageHandlerQ.clear();
    }
    
    /**
     * Returns the queue of status messages.
     * 
     * @return the queue of status messages
     */
    public LinkedList getStatusMessageHandlerQ()
    {
        return StatusMessageHandlerQ;
    }
    
    /**
     * Creates and returns the status bar thread.
     * 
     * @param statusBar the status bar thread
     */
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
    
    /**
     * Displays the specified message to the status bar and not the Debug Window.
     * 
     * @param message the message to be displayed
     */
    public void showStatusOnBarOnly(String message)
    {
        statusBar.setForeground((Color.BLACK));
        statusBar.setText(message);
    }
    
    /**
     * Adds the specified message to status bar and debug Window with default
     * color and time. (Color and time only apply to status bar, not Debug Window)
     * 
     * @param message the message to display
     */
    public void showStatus(String message)
    {
         showStatus(message, Color.BLACK, 0);
    }

    /**
     * Adds the specified message and time to status bar and debug Window with default
     * color. (Color and time only apply to status bar, not Debug Window)
     * 
     * @param message the message to display
     * @param autoClearTime the amount of time the message will display
     */
    public void showStatus(String message, long autoClearTime)
    {
        showStatus(message, Color.BLACK, autoClearTime);
    }

    /**
     * Adds the specified message and color to status bar and debug Window with default
     * time. (Color and time only apply to status bar, not Debug Window)
     * 
     * @param message the message to display
     * @param color the color the message will display in the status bar
     */
    public void showStatus(String message, Color color)
    {
       showStatus(message, color, 0);
    }

    /**
     * Adds the specified message, color and time to status bar and debug Window.
     * (Color and time only apply to status bar, not Debug Window)
     * 
     * @param message the message to display
     * @param color the color the message will display in the status bar
     * @param autoClearTime the amount of time the message will display
     */
    public void showStatus(String message, Color color, final long autoClearTime)
    {
        if (message.length() > 0)
            StatusMessageHandlerQ.add(new StatusMessageHandler(message, color, autoClearTime));
    }
      
    /**
     * Displays a consistantly formatted error message in a Dialog Box and 
     * again to the Debug Window.
     * 
     * @param e the exception that triggered the error message, can be null
     * @param errorMessage the message to be displayed with the error
     */
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
    
    /**
     * Displays a consistantly formatted errinformationor message in a Dialog Box.
     * 
     * @param infoMessage the message to be displayed 
     */
    public void displayInfoMessage(String infoMessage)
    {
        JOptionPane.showMessageDialog(Luv.getLuv(),
                                      infoMessage,
                                      "Stopping Execution",
                                      JOptionPane.INFORMATION_MESSAGE);
    }

    /**
     * Clears the status bar.
     */
    public void clearStatus()
    {
        StatusMessageHandlerQ.add(BLANK_MESSAGE);
    }
}
