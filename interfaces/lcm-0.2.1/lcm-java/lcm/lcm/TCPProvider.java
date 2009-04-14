package lcm.lcm;

import java.net.*;
import java.io.*;
import java.util.*;
import java.util.regex.*;
import java.nio.*;

/** LCM provider for the tcp: URL. All messages are sent to a central
 * "hub" process (that must be started separately), which will relay
 * the messages to all other processes. TCPService is an
 * implementation of the hub process.
 **/
public class TCPProvider implements Provider
{
    LCM lcm;

    static final String DEFAULT_NETWORK = "127.0.0.1:7700";
    InetAddress inetAddr;
    int         inetPort;
    
    ReaderThread reader;

    public static final int MAGIC_SERVER = 0x287617fa; // first word sent by server
    public static final int MAGIC_CLIENT = 0x287617fb; // first word sent by client
    public static final int VERSION = 0x0100;    // what version do we implement?
    public static final int MESSAGE_TYPE_PUBLISH = 1;

    public TCPProvider(LCM lcm, URLParser up) throws IOException
    {
	this.lcm = lcm;

	String addrport[] = up.get("network", DEFAULT_NETWORK).split(":");
	inetAddr = InetAddress.getByName(addrport[0]);
	inetPort = Integer.valueOf(addrport[1]);

	reader = new ReaderThread();
	reader.start();
    }

    /** Publish a message synchronously. However, if the server is not
     * available, it will return immediately. 
     **/
    public synchronized void publish(String channel, byte data[], int offset, int length)
    {
	try {
	    publishEx(channel, data, offset, length);
	} catch (Exception ex) {
	    System.err.println("ex: "+ex);
	}
    }

    public synchronized void subscribe(String channel)
    {
	// to-do.
    }

    public synchronized void close()
    {
	if (null != reader) {
	    reader.close();
	    try {
		reader.join();
	    } catch (InterruptedException ex) {
	    }
	}

	reader = null;
    }

    static final void safeSleep(int ms)
    {
	try {
	    Thread.sleep(ms);
	} catch (InterruptedException ex) {
	}
    }

    void publishEx(String channel, byte data[], int offset, int length) throws Exception
    {
	byte[] channel_bytes = channel.getBytes("US-ASCII");

	int payload_size = channel_bytes.length + length;

	ByteArrayOutputStream bouts = new ByteArrayOutputStream(length + channel.length() + 32);
	DataOutputStream outs = new DataOutputStream(bouts);
	
	outs.writeInt(MESSAGE_TYPE_PUBLISH);

	outs.writeInt(channel_bytes.length);
	outs.write(channel_bytes, 0, channel_bytes.length);
	
	outs.writeInt(length);
	outs.write(data, offset, length);
	
	byte[] b = bouts.toByteArray();

	// try to send message on socket. If the socket is not
	// connected, we'll simply fail. The readerthread is
	// responsible for maintaining a connection to the hub.
	OutputStream sockOuts = reader.getOutputStream();
	if (sockOuts != null) {
	    try {
		sockOuts.write(b);
		sockOuts.flush();
	    } catch (IOException ex) {
	    }
	}
    }

    class ReaderThread extends Thread
    {
	Socket sock;
	DataInputStream ins;
	OutputStream outs;
	boolean exit = false;
	int serverVersion;

	public void run()
	{
	    while (!exit) {

		//////////////////////////////////
		// reconnect
		try {
		    sock = new Socket(inetAddr, inetPort);
		    OutputStream _outs = sock.getOutputStream();
		    DataOutputStream _douts = new DataOutputStream(_outs);
		    _douts.writeInt(MAGIC_CLIENT);
		    _douts.writeInt(VERSION);
		    _douts.flush();
		    outs = _outs;
		    ins = new DataInputStream(new BufferedInputStream(sock.getInputStream()));

		    int magic = ins.readInt();
		    if (magic != MAGIC_SERVER) {
			sock.close();
			continue;
		    }

		    serverVersion = ins.readInt();
		    
		} catch (IOException ex) {
		    System.out.println("lcm.TCPProvider: Unable to connect to "+inetAddr+":"+inetPort);
		    safeSleep(500);

		    // try connecting again.
		    continue;
		}

		//////////////////////////////////
		// read loop
		try {
		    while (!exit) {
			int type = ins.readInt();
			int channellen = ins.readInt();
			byte channel[] = new byte[channellen];
			ins.readFully(channel);
			int datalen = ins.readInt();
			byte data[] = new byte[datalen];
			ins.readFully(data);

			lcm.receiveMessage(new String(channel), data, 0, data.length);
		    }
		    
		} catch (IOException ex) {
		    // exit read loop so we'll create a new connection.
		}
	    }
	}

	void close()
	{
	    try {
		sock.close();
	    } catch (IOException ex) {
	    }

	    exit = true;
	}

	OutputStream getOutputStream()
	{
	    return outs;
	}
    }
}
