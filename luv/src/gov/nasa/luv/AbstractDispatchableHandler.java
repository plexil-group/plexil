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

/** Dispatchable XML parsers will be derrived from this class. */

public abstract class AbstractDispatchableHandler extends DefaultHandler
{
      /** buffer which holds text between xml tags */

      StringBuffer tweenerBuffer;
      
      /** Construct an AbstractDispatchableHandler.
       */

      public AbstractDispatchableHandler()
      {
	  super();
      }

      /** Handle data between element tags.
       *
       * @param ch character buffer
       * @param start index of start of data characters in buffer
       * @param length number of data characters in buffer
       */

      public void characters(char[] ch, int start, int length)
      {
         if (length >= 0)
         {
            if (tweenerBuffer == null)
               tweenerBuffer = new StringBuffer();
            tweenerBuffer.append(ch, start, length);
         }
      }
      
      /** Get collected text that appears in between XML tags.  The text
       * has had any leading and trailing white space trimmmed off.
       * This is destructive action and clears the buffer which holds
       * this text, calling it again immediatly will always return null.
       *
       * @return the collected text or null of no such text existed
       */

      public String getTweenerText()
      {
         String text = null;

         // if there is some text between tags (after trimming leading
         // and trailing white space), record that

         if (tweenerBuffer != null)
         {
            text = tweenerBuffer.toString().trim();
            if (text.length() == 0)
               text = null;
            tweenerBuffer = null;
         }

         // return trimmed text or null

         return text;
      }

}
