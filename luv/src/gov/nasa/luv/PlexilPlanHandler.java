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

import java.util.Vector;
import static gov.nasa.luv.Constants.*;

import static java.lang.System.out;

/** SAX PlexilPlan XML handler */

public class PlexilPlanHandler extends AbstractDispatchableHandler
{
      String indent = "";
      String indentIncrement = "  ";
      boolean showXmlTags = false;
      public static boolean recordLocalVariables = false;
      public static int count = 0;

      Stack<Model> stack = new Stack<Model>();
      
      public PlexilPlanHandler(Model model)
      {
         super(PLEXIL_PLAN, model);
         stack.push(model);
      }

      /** Handle start of an XML document */

      public void startDocument()
      {
      }

      /** Handle start of an XML element. */

      public void startElement(String uri, String localName, 
                               String qName, Attributes attributes)
      {
          
         if (showXmlTags)
         {
            out.println(indent + localName);
            indent = indent + indentIncrement;
         }

         // get the current node in the stack
         
         Model node = stack.peek();

         // if this SHOULD be a child node, make that happen

         if (Model.isChildType(localName))
         {
            Model child = new Model(localName);

            // if there is a parent, add child to it

            if (node != null)
               node.addChild(child);

            // this child is now the current node
            
            node = child;

            // add attributs for this child

            for (int i = 0; i < attributes.getLength(); ++i)
            {
               child.setProperty(attributes.getQName(i),
                                attributes.getValue(i));
               if (showXmlTags)
                  out.println(indent + attributes.getQName(i) +
                              " = " + attributes.getValue(i));
            }
            
         }

         // else if its not a property the we can ignore it
         
         else if (!Model.isProperty(localName))
            node = null;

         // push new node onto the stack

         stack.push(node);
      }

      /** Handle end of an XML element. */

    @Override
      public void endElement(String uri, String localName, String qName)
      {
         // get the current node, which may be null indicating that it
         // is to be ignored

         Model node = stack.peek();

         // get tweener text and put it in it's place

         String text = getTweenerText();
         
         if (localName.equals(DECL_VAR))
         {
             if (count == 2)
                 findFirstNonNullNode().addLocalVariableName(VAL, "nvl");
                 
             recordLocalVariables = false;
             count = 0;
         }
         
         if (text != null)
         {
            if (node == null)
               findFirstNonNullNode().addLocalVariableName(localName, text);
            
             if (recordLocalVariables)
             {
                 if (node == null)
                 {
                     if (localName.equals(NAME))
                         ++count;
                     if (localName.equals(TYPE))
                         ++count;
                     if (localName.equals(INT_VAL) || localName.equals(REAL_VAL) || localName.equals(BOOL_VAL) || localName.equals(STRING_VAL))
                         ++count;
                         
                     findFirstNonNullNode().addLocalVariableName(localName, text);
                 }
             }
            
            if (node != null)
               node.setProperty(localName, text);
            if (showXmlTags)
               out.println(indent + text);
         }

         // if showing tag data, print that

         if (showXmlTags)
         {
            indent = indent.substring(0, indent.length() - indentIncrement.length());
            out.println(indent + localName);
         }
         
         // if this node shold be tailor, do that

         if (node != null && localName.equals(node.getType()))
            node.tailor();

         // pop the node off the stack

         stack.pop();
      }

      /** Handel end of document event. */

      public void endDocument()
      {
         model.planChanged();
      }
      
      public Model findFirstNonNullNode()
      {
            int i = stack.size() - 1;
            while (stack.elementAt(i) == null)
               i--; 
            return stack.elementAt(i);
      } 
}
