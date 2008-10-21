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

import java.net.Socket;
import java.io.InputStream;
import java.io.OutputStream;

import javax.swing.JOptionPane;

import org.xml.sax.InputSource;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;

import gov.nasa.luv.SocketWrangler;
import gov.nasa.luv.DispatchHandler;

/** Represents one client connection to the LUV server. */

public class LuvSocketWrangler
    implements SocketWrangler
{
    LuvSocketWrangler()
    {
    }

    public void wrangle(Socket s)
    {
	InputStream is;
	// get the input stream for this socket
	try {
	    is = s.getInputStream();
	}
	catch (Exception e) {
	    JOptionPane.showMessageDialog(Luv.getLuv(),
					  "Error initializing socket.  See debug window for details.",
					  "Internal Error",
					  JOptionPane.ERROR_MESSAGE);
	    e.printStackTrace();
	    return;
	}
	
	// set up an XML reader
	XMLReader parser;
	try {
	    parser = XMLReaderFactory.createXMLReader();
	}
	catch (Exception e) {
	    JOptionPane.showMessageDialog(Luv.getLuv(),
					  "Error initializing XML reader.  See debug window for details.",
					  "Internal Error",
					  JOptionPane.ERROR_MESSAGE);
	    e.printStackTrace();
	    return;
	}

	DispatchHandler dh = new DispatchHandler();
	parser.setContentHandler(dh);
	InputSource src = new InputSource(is);

	while (true) {
	    // if there is input, grab it up
	    try {
		parser.parse(src);
	    }
	    catch (Exception e) {
		JOptionPane.showMessageDialog(Luv.getLuv(),
					      "Error parsing XML message.  See debug window for details.",
					      "Parse Error",
					      JOptionPane.ERROR_MESSAGE);
		e.printStackTrace();
	    }
	    
	    // *** may need to do something special here after receiving plans?
	}
    }
}
