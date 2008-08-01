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

import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import static gov.nasa.luv.Constants.*;

import javax.swing.JOptionPane;

import java.util.HashMap;

/** Dispatches XML messages to the correct handler from the set of
 * provided handlers in the handler map.
 */

public class DispatchHandler extends DefaultHandler
{
      /** table of messages handlers */
      
      static HashMap<String, AbstractDispatchableHandler> handlerMap = 
         new HashMap<String, AbstractDispatchableHandler>();
      
      /** The currently selected handler */
      
      AbstractDispatchableHandler currentHandler;
            
      /** Construct a dispatch handler witch operats on the provided
       * model.
       *
       * @param model model which will be operated on
       */
      
      public DispatchHandler(Model model)
      {
         // add each type of handler the the table of possible handlers

          registerHandler(new PlanInfoHandler(model));
          registerHandler(new PlexilPlanHandler(model));
          registerHandler(new StateUpdateHandler(model));
      }

      /** Add a new handler to the set of available handlers to dispatch.
       *
       * @param handler hander to add to the set available handers to dispatch.
       */

      public static void registerHandler(AbstractDispatchableHandler handler)
      {
         handlerMap.put(handler.getKey(), handler);
      }

      /** Handel start of document event. */

      public void startDocument()
      {
         // at the start of the document ensuer that handler is nulled out

         currentHandler = null;
      }

      /** Establish the current handler then dispatch events to that
       * hander. */

      public void startElement(String uri, String localName, 
                               String qName, Attributes attributes) 
         throws SAXException
      {
         // if no handler has been identified, find one now!

         if (currentHandler == null)
         {
            currentHandler = handlerMap.get(localName);
            
            if (currentHandler == null)
            {
               JOptionPane.showMessageDialog(
                  null,
                  "Unhandled XML tag: <" + localName + ">." +
                  "  See debug window for details.",
                  "Parse Error",
                  JOptionPane.ERROR_MESSAGE);
               throw(new Error("Unhandled XML tag: <" + localName + ">."));
            }
            
            // signal start of document (because we didn't know who to
            // send the event to before)

            currentHandler.startDocument();
         }
         
         // using the current handler dispatch the startElement action

         currentHandler.startElement(uri, localName, qName, attributes);
      }

      /** Dispatch the characters action. */
      
      public void characters(char[] ch, int start, int length)
         throws SAXException
      {
         currentHandler.characters(ch, start, length);
      }
      
      /** Dispatch the endElement action. */
      
      public void endElement(String uri, String localName, String qName)
         throws SAXException
      {
         currentHandler.endElement(uri, localName, qName);
      }

      /** Dispatch end of document event. */

      public void endDocument()
         throws SAXException
      {
         currentHandler.endDocument();
      }

      /** Model accesor which returns the model from selected handler.
       *
       * @return the model from the selected hander
       */

      public Model getModel()
      {
         return currentHandler.getModel();
      }

      /** Get the selected handler.
       *
       * @return the handler selected to parse last message
       */

      public AbstractDispatchableHandler getHandler()
      {
         return currentHandler;
      }
}
