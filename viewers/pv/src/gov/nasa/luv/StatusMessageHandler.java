/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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
import java.awt.Component;
import java.awt.Font;
import java.util.LinkedList;
import javax.swing.JLabel;
import javax.swing.JOptionPane;

import static gov.nasa.luv.Constants.UNKNOWN;
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
    private static JLabel portBar;
    private static final long DEFAULT_WAIT = 0;
    
    // queue of status messages
    private LinkedList<StatusMessageHandler> StatusMessageHandlerQ;       
    private LinkedList<StatusMessageHandler> PortStatusMessageHandlerQ;
    private boolean abortAutoClear;
    private long autoClearTime;
    private Color color;
    private String message;
    private boolean idleMessage = false;

    /**
     * Constructs a StatusMessageHandler.
     */
    public StatusMessageHandler() 
    {
        StatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
        PortStatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
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
        PortStatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
        this.message = message;
        this.color = color;
        this.autoClearTime = autoClearTime;
        abortAutoClear = false;
    }   
    
    /**
     * Constructs a StatusMessageHandler with the specified message, color, idleStatus, and 
     * amount of time it will be displayed.
     * 
     * @param message the message displayed
     * @param color the color the message will be displayed in
     * @param autoClearTime the amount of time message will be displayed
     */
    public StatusMessageHandler(String message, Color color, Boolean idle, long autoClearTime)
    {
        StatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
        PortStatusMessageHandlerQ = new LinkedList<StatusMessageHandler>();
        this.message = message;
        this.color = color;
        this.autoClearTime = autoClearTime;
        this.idleMessage = idle;
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
    public LinkedList<StatusMessageHandler> getStatusMessageHandlerQ()
    {
        return StatusMessageHandlerQ;
    }

    /**
     * Clears the Queue of messages.
     */
    public void clearPortStatusMessageQ()
    {
    	PortStatusMessageHandlerQ.clear();
    }
    
    /**
     * Returns the queue of status messages.
     * 
     * @return the queue of status messages
     */
    public LinkedList<StatusMessageHandler> getPortStatusMessageHandlerQ()
    {
        return PortStatusMessageHandlerQ;
    }
    
    /**
     * Creates and returns the message status bar thread.
     * 
     * @param statusBar the message status bar thread
     */    
    public void startMsgStatusBarThread(final JLabel statusBar)
    {
    	this.statusBar = statusBar;    	
    	startStatusBarThread(this.statusBar, StatusMessageHandlerQ);
    }
    
    /**
     * Creates and returns the port status bar thread.
     * 
     * @param statusBar the port status bar thread
     */    
    public void startPortStatusBarThread(final JLabel portBar)
    {
    	this.portBar = portBar;
    	startStatusBarThread(this.portBar, PortStatusMessageHandlerQ);
    }
    
    /**
     * Creates and returns the status bar thread.
     * 
     * @param statusBar the status bar thread
     */
    public void startStatusBarThread(final JLabel statusBar, final LinkedList<StatusMessageHandler> messageHandlerQ)
    {         
         statusBar.setFont(statusBar.getFont().deriveFont(Font.PLAIN, 12.0f));
          
         new Thread()
         {
               @Override public void run()
               {
                  try
                  {
                     StatusMessageHandler lastMessage = null;
                     long idleTime = 0;
                     while (true)
                     {                                           	  
                        if (!messageHandlerQ.isEmpty())
                        {
                           // kill any preceding auto clear thread
                           if(lastMessage != null)              
                           {
                              lastMessage.abortAutoClear = true;                           
                              idleTime = lastMessage.autoClearTime;                             
                           }

                           // get the message
                           final StatusMessageHandler message = 
                        	   messageHandlerQ.removeFirst();
                           lastMessage = message.autoClearTime > 0 
                              ? message
                              : null;                           
                           
                           // print to debug window only pertinent messages  
                           if(message.idleMessage)
                           {
	                        	new Thread()
	                           {
	                                 @Override public void run()
	                                 {
	                                    try
	                                    {                 
	                                       statusBar.setForeground(message.color);
	                                       statusBar.setText(message.message);
	                                    }
	                                    catch (Exception e)
	                                    {
	                                       e.printStackTrace();
	                                    }
	                                 }
	                           }.start();
	                           
                           }
                           else
                           {
                        	    statusBar.setForeground(message.color);
                           		statusBar.setText(message.message);
                           }

                           if (!message.message.equals(BLANK_MESSAGE.message))
                           {                  
                               out.println("STATUS: " + message.message);
                           }

                           // if auto clear requested start a thread for that
                           if (message.autoClearTime > 0 && !message.idleMessage)
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
    
    // Exactly one caller, in class Luv.
    public void showIdlePortMessage() {    	
    	if (Luv.getLuv().getIsExecuting()
            && (Luv.getLuv().getCurrentPlan() == null
                || Luv.getLuv().getCurrentPlan().getPlanName() == null)) {
    		idleMessage = true;
            if (message != null && !message.isEmpty())
                PortStatusMessageHandlerQ.add(new StatusMessageHandler(message, Color.GREEN, idleMessage, DEFAULT_WAIT));
    	}
    }
    
    /**
     * Displays the specified message to the port status bar and not the Debug Window.
     * 
     * @param message the message to be displayed
     */
    public void showChangeOnPort(String message) {
    	showChangeOnPort(message, Color.BLACK);
    }

    /**
     * Adds the specified message and color to port status bar
     * (Color only apply to port status bar, not Debug Window)
     * 
     * @param message the message to display
     * @param color the color the message will display in the port status bar
     */
    public void showChangeOnPort(String message, Color color) {
    	idleMessage = false;
        if (!message.isEmpty())
        	PortStatusMessageHandlerQ.add(new StatusMessageHandler(message, color, DEFAULT_WAIT));
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
     * Displays a consistently formatted error message in a Dialog Box and 
     * again to the Debug Window.
     * 
     * @param e the exception that triggered the error message, can be null
     * @param errorMessage the message to be displayed with the error
     */
    public void displayErrorMessage(Exception e, String errorMessage) {
        displayErrorMessage(Luv.getLuv(), e, errorMessage);
    }
      
    /**
     * Displays a consistently formatted error message in a Dialog Box and 
     * again to the Debug Window.
     * 
     * @param c The display component reporting the error.
     * @param e the exception that triggered the error message, can be null
     * @param errorMessage the message to be displayed with the error
     */
    public void displayErrorMessage(Component c, Exception e, String errorMessage) {
        if (e != null) {
            JOptionPane.showMessageDialog(c, 
                                          errorMessage + ".\nPlease see Debug Window.", 
                                          "Error", 
                                          JOptionPane.ERROR_MESSAGE);
            out.println(errorMessage + "\n" + e.getMessage());
            e.printStackTrace(out);
            // make debug window visible
            Luv.getLuv().getDebugWindow().setVisible(true);
            Luv.getLuv().getDebugWindow().toFront();
        }
        else {
            JOptionPane.showMessageDialog(c,
                                          errorMessage, 
                                          "Error", 
                                          JOptionPane.ERROR_MESSAGE);

            out.println(errorMessage);
        }
    }
    
    /**
     * Displays a consistently formatted error information or message in a Dialog Box.
     * 
     * @param msg the message to be displayed 
     */
    public void displayWarningMessage(String msg, String title)
    {
        JOptionPane.showMessageDialog(Luv.getLuv(),
                                      msg,
                                      title,
                                      JOptionPane.WARNING_MESSAGE);
        out.println("WARN: " + msg);
    }
    
    /**
     * Displays a consistently formatted error information or message in a Dialog Box.
     * 
     * @param component the component to return focus to when the dialog is dismissed
     * @param msg the message to be displayed 
     * @param title Title to display on the warning dialog.
     */
    public void displayWarningMessage(Component component, String msg, String title)
    {
        JOptionPane.showMessageDialog(component,
                                      msg,
                                      title,
                                      JOptionPane.WARNING_MESSAGE);
        out.println("WARN: " + msg);
    }
    
    /**
     * Displays a consistently formatted error information or message in a Dialog Box.
     * 
     * @param c The UI component displaying the message.
     * @param infoMessage The message to be displayed.
     * @param title The title to use on the info dialog.
     */
    public void displayInfoMessage(Component c, String infoMessage, String title)
    {
        JOptionPane.showMessageDialog(c,
                                      infoMessage,
                                      title,
                                      JOptionPane.INFORMATION_MESSAGE);
        out.println("INFO: " + infoMessage);
    }

    /**
     * Displays a consistently formatted error information or message in a Dialog Box.
     * 
     * @param infoMessage The message to be displayed.
     * @param title The title to use on the info dialog.
     */
    public void displayInfoMessage(String infoMessage, String title) {
        displayInfoMessage(Luv.getLuv(), infoMessage, title);
    }

    /**
     * Clears the status bar.
     */
    public void clearStatus()
    {
        StatusMessageHandlerQ.add(BLANK_MESSAGE);
    }
}
