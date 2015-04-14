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

import java.io.BufferedReader;
import java.io.EOFException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.Reader;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Set;
import java.util.TreeSet;
import java.util.Vector;

import org.xml.sax.InputSource;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;

import static gov.nasa.luv.Constants.END_OF_MESSAGE;

public class LuvSocketServer {

    private boolean exitRequested;
    private static Thread serverThread;

    /**
     * Constructs a server which listens on the specified port and StreamWranglerFactory.
     *
     * @param port port on which this server listens.
     */
    public LuvSocketServer() {
        exitRequested = false;
        serverThread = null;
    }

    public void startServer(int port) {
        if (serverThread != null) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: attempt to start server when it is already running");
            return;
        }

        if (!portFree(port)) {
            if (Luv.getLuv().getSettings().getPortSupplied()) {
                // User chose this port
                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null,
                                                                           "ERROR: port " + port + " is in use, please try another.");
                Luv.getLuv().getStatusMessageHandler().showChangeOnPort("Unable to listen on port " + port);
                return;
            }
            // Used default, choose another
            else {
                Vector<Integer> ports = getPortList();
                Luv.getLuv().getStatusMessageHandler().displayWarningMessage("Port " + port
                                                                             + " is unavailable, using port " + ports.firstElement()
                                                                             + " instead.",
                                                                             "Port unavailable");
                port = ports.firstElement();
                Luv.getLuv().getSettings().setPort(port);
            }
        }

        final int thePort = port; // work around compiler error
        // create a thread which listens for events
        serverThread = new Thread() {
                public void run() {
                    acceptConnections(thePort);
                }
            };
        serverThread.start();
        Luv.getLuv().getStatusMessageHandler().showChangeOnPort("Listening on port " + port);
    }
    
    public void stopServer() {
        if (serverThread == null)
            return; // nothing to do

        Luv.getLuv().getStatusMessageHandler().showChangeOnPort("Stopping service on port "
                                                                + Luv.getLuv().getSettings().getPort());
        Thread.State s = serverThread.getState();
        if (s != Thread.State.TERMINATED) {
            exitRequested = true;
            try {
                serverThread.join(2000);
            } catch (InterruptedException e) {
            }
        }
        serverThread = null;
    }

    public boolean isGood() {
        if (serverThread == null)
            return false;
        Thread.State s = serverThread.getState();
        if (s == Thread.State.TERMINATED)
            return false;
        return true;
    }

    /**
     * Waits for client to connect on a given port.  This method
     * blocks indefinitely and spins off threads for each connection.
     *
     * @param port port on which this server listens.
     */
    public void acceptConnections(int port) {
        try {
            ServerSocket luvSocket = new ServerSocket(port);
            try {
                luvSocket.setSoTimeout(1000); // check for request to quit once a second
            } catch (java.net.SocketException e) {
                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: while configuring server socket on port " + port);
            }
            while (!exitRequested) {
                try {
                    new ServerThread(luvSocket.accept()).start();
                } catch (java.net.SocketTimeoutException ie) {
                    // do nothing and repeat
                } catch (java.net.SocketException e) {
                    if (e.getMessage().equals("Socket closed")) {
                        Luv.getLuv().getStatusMessageHandler().showStatus("Previous " + e.getMessage(), 100);
                        return;
                    }
                    else if (e.getMessage().equals("Permission denied")) {
                        Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: " + e.getMessage()
                                                                                   + " Port " + port + " is in use");
                        Luv.getLuv().getStatusMessageHandler().showChangeOnPort("Port " + port + " unavailable, please change server port");
                        return;
                    }
                    else {        			
                        Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: socket exception occurred while starting server on port " + port);
                        return;
                    }
                }
            }
            luvSocket.close();
        } catch (Exception e) {
                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while connecting to server using port " + port);
                e.printStackTrace();
        }
        exitRequested = false;
    }

    private class ServerThread
        extends Thread {
        Socket socket;

        public ServerThread(Socket s) {
            socket = s;
        }

        public void run() {
            try {
                wrangle(socket.getInputStream(), socket.getOutputStream());
            } catch (IOException e) {
                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception in server on port "
                                                                           + socket.getLocalPort());
            }
        }

        /**
         * Wrangles the connection to the Luv server with the specified input and output streams.
         * @param in the InputStream
         * @param out the OutputStream
         */
        private void wrangle(InputStream in, OutputStream out) {
            // set up an XML reader
            XMLReader parser;
            try {
                parser = XMLReaderFactory.createXMLReader();
            } catch (Exception e) {
                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while initializing XML reader");
                return;
            }
            parser.setContentHandler(new DispatchHandler());

            InputSource is = new InputSource(new InputStreamWrapper(in));
            boolean quit = false;
            do {
                // if there is input, grab it up
                try {
                    parser.parse(is);
                } // The stream wrapper signals an EOFException when the wrapped stream hits EOF.
                // This would be a good place to notify viewer that execution is complete.
                catch (EOFException e) {
                    quit = true;
                    try {
                        in.close();
                    } catch (Exception f) {
                        Luv.getLuv().getStatusMessageHandler().displayErrorMessage(f, "ERROR: exception occurred while closing Universal Executive input stream");
                    }

                    Luv.getLuv().finishedExecutionState(); // FIXME: tell Luv what (EOF), not how
                    break;
                } catch (Exception e) {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: while parsing input stream");
                    e.printStackTrace();
                    Luv.getLuv().finishedExecutionState(); // FIXME: tell Luv what (comm error), not how
                    quit = true;
                    break;
                }

                if (Luv.getLuv().breaksAllowed()) {
                    if (Luv.getLuv().shouldBlock()) {
                        Luv.getLuv().blockViewer();
                    }

                    // tell Exec it's OK to proceed
                    try {
                        out.write(END_OF_MESSAGE);
                    } catch (Exception e) {
                        Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while acknowledging message from the Universal Executive");
                        break;
                    }
                }
            } while (!quit);
        }
    }


    // Utilities

    public static Set<Integer> getPortsInUse() {
        Set<Integer> result = new TreeSet<Integer>();
        String[] cmd = {"list_ports_in_use"};
        try {
            Process p = Runtime.getRuntime().exec(cmd);
            BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream()));
            // Port numbers are returned one at a time
            String line = null;
            while ((line = r.readLine()) != null) {
                try {
                    result.add(Integer.parseUnsignedInt(line));
                } catch (NumberFormatException e) {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "Error reading ports in use");
                    return new TreeSet<Integer>();
                }
            }
        } catch (IOException e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "Error reading ports in use");
            return new TreeSet<Integer>();
        }
        return result;
    }

    public static Vector<Integer> getPortList() {
        Set<Integer> inUse = getPortsInUse();
        Vector<Integer> portList = new Vector<Integer>(Constants.PORT_MAX - Constants.PORT_MIN + 1);
        int deflt = Luv.getLuv().getSettings().getPort(); // current setting
		for (Integer i = Constants.PORT_MIN; i <= Constants.PORT_MAX; i++)
			if (!inUse.contains(i)
                || (serverThread != null && i == deflt))
				portList.add(Integer.valueOf(i));
        return portList;
    }

    /** Return true if port free, false otherwise */
    public static boolean portFree(int port) {
        String[] cmd = {"port_in_use", "-q", ""};
        cmd[2] = Integer.toString(port);
        try {
            return (1 == Runtime.getRuntime().exec(cmd).waitFor());
        } catch (Exception e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while checking port");
        }
        return false; // caution on side of error
    }


}
