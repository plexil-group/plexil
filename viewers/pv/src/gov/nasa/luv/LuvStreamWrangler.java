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

import java.io.InputStream;
import java.io.OutputStream;
import java.io.EOFException;

import org.xml.sax.InputSource;
import org.xml.sax.XMLReader;
import org.xml.sax.helpers.XMLReaderFactory;

import static gov.nasa.luv.Constants.END_OF_MESSAGE;

/** 
 * The LuvSocketWrangler class represents one client connection to the 
 * Luv server. 
 */
public class LuvStreamWrangler implements StreamWrangler {

    /**
     * Wrangles the connection to the Luv server with the specified input and output streams.
     * @param in the InputStream
     * @param out the OutputStream
     */
    public void wrangle(InputStream in, OutputStream out) {
        // wrap the input stream
        InputStreamWrapper is = new InputStreamWrapper(in);

        // set up an XML reader
        XMLReader parser;
        try {
            parser = XMLReaderFactory.createXMLReader();
        } catch (Exception e) {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while initializing XML reader");
            return;
        }

        DispatchHandler dh = new DispatchHandler();
        parser.setContentHandler(dh);
        InputSource src = new InputSource(is);

        while (true) {
            // if there is input, grab it up
            try {
                parser.parse(src);
            } // The stream wrapper signals an EOFException when the wrapped stream hits EOF.
            // This would be a good place to notify viewer that execution is complete.
            catch (EOFException e) {
                try {
                    in.close();
                } catch (Exception f) {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(f, "ERROR: exception occurred while closing Universal Executive input stream");
                }

                Luv.getLuv().finishedExecutionState();

                break;
            } catch (Exception e) {
                Luv.getLuv().finishedExecutionState();
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
        }
    }
}
