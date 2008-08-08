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
import java.util.Stack;

import static gov.nasa.luv.Constants.*;

import static java.lang.System.out;

public class PlanInfoHandler extends AbstractDispatchableHandler
{
      /** Make me a handler. */

      public PlanInfoHandler(Model model)
      {
         super(PLAN_INFO, model);
      }

      /** Handle start of an XML document */

      @Override public void startDocument()
      {
         // the model should be empty

         model.clear();
      }

      /** Handle end of an XML element. */

      @Override public void endElement(String uri, String localName, String qName)
      {
         // get text between tags

         String text = getTweenerText();

         // if this is a the plan filename set that property in model

         if (localName.equals(PLAN_FILENAME))
            model.addPlanName(text);

         // if this is a the plan filename set that property in model

         if (localName.equals(LIBRARY_FILENAME))
            model.addLibraryName(text);

         // if this is a the plan filename set that property in model

         else if (localName.equals(VIEWER_BLOCKS))
            model.setProperty(VIEWER_BLOCKS, text);
      }
}
