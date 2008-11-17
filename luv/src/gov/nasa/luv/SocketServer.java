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

import java.net.ServerSocket;
import java.net.Socket;

/** Functions as a server for data streams over a TCP socket.
 * For each incoming connection, accepts the socket,
 * and hands it off to a new instance of a SocketWrangler
 * as specified by a SocketWranglerFactory.
 */

public class SocketServer
{
    public SocketWranglerFactory wranglerFactory;

    /** Construct a server which listens on a given port.
     *
     * @param port port on which this server listens. 
     * @param factory An instance of SocketWranglerFactory to create socket wranglers.
     */

    public SocketServer(final int port, SocketWranglerFactory factory)
    {
	wranglerFactory = factory;

	// create a thread which listens for events
	new Thread()
	{
	    public void run()
	    {
		accept(port);
	    }
	}.start();
    }

    /** Wait for client to connect on a given port.  This method
     * blocks indefinitely and spins off threads for each connection.
     *
     * @param port port on which this server listens.
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
            Luv.getLuv().displayErrorMessage(e, "ERROR: exception occurred while handling connection to server");
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
	final SocketWrangler w = wranglerFactory.createSocketWrangler();
	new Thread()
	{
	    public void run()
	    {
		w.wrangle(s);
	    }
	}.start();
    }
}
