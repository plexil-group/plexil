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

public class LuvSocketServer {

    private StreamWranglerFactory wranglerFactory;
    private ServerSocket luvSocket;
    private boolean threadDone;

    /**
     * Constructs a server which listens on the specified port and StreamWranglerFactory.
     *
     * @param port port on which this server listens.
     */
    public LuvSocketServer(final int port) {
        wranglerFactory = new LuvStreamWranglerFactory();
        threadDone = true;

        // create a thread which listens for events
        new Thread() {

            public void run() {
                accept(port);
            }
        }.start();
    }

    /**
     * Waits for client to connect on a given port.  This method
     * blocks indefinitely and spins off threads for each connection.
     *
     * @param port port on which this server listens.
     */
    public void accept(int port) {
        try {
        	luvSocket = new ServerSocket(port);
            while (threadDone) {
                handleConnection(luvSocket.accept());
            }
        }catch (java.net.SocketException e){
        	if(e.getMessage().equals("Socket closed"))
        		Luv.getLuv().getStatusMessageHandler().showStatus("Previous " + e.getMessage(), 100);
        	else if(e.getMessage().equals("Permission denied")) {
        		if(!LuvTempFile.checkPort(Luv.getLuv().getPrevPort()) && Luv.getLuv().getPrevPort() != 0)
        		{
        			Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: " + e.getMessage() + " using port " + port + ". Reverting to " + Luv.getLuv().getPrevPort());
        			Luv.getLuv().changePort(Luv.getLuv().getPrevPort()+"");
        		}
        		else if(Luv.getLuv().getPrevPort() == 0)
        		{
        			Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "Switching to next avaliable");
        			Luv.getLuv().getPortGUI().refresh();
        			Luv.getLuv().changePort(Luv.getLuv().getPortGUI().getPick()+"");
        		}
        		else
        		{        			
        			Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while connecting to server using ports " + port
        					+ " and " + Luv.getLuv().getPrevPort());
        			Luv.getLuv().getStatusMessageHandler().showChangeOnPort("Port Unassigned, change server port");
        		}
        	}
        	else {
        		Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while connecting to server using port " + port);
        		e.printStackTrace();
        	}
        }        
        catch (Exception e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while connecting to server using port " + port);            
        }
    }

    public void handleConnection(final Socket s) {
        final StreamWrangler w = wranglerFactory.createStreamWrangler();
        new Thread() {

            public void run() {
                try {
                    w.wrangle(s.getInputStream(), s.getOutputStream());
                } catch (IOException e) {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while connecting...");
                }
            }
        }.start();
    }
    
    public void stopServer() throws IOException
    {
	    	threadDone = false;
	    	if(luvSocket != null)
	    		luvSocket.close();	    	    	    
    }
}
