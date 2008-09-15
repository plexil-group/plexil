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

import java.util.ArrayList;
import org.xml.sax.Attributes;
import java.util.Stack;

import static gov.nasa.luv.Constants.*;

import static java.lang.System.out;

/** SAX PlexilPlan XML handler */

public class PlexilPlanHandler extends AbstractDispatchableHandler
{
      private String indent = "";
      private String indentIncrement = "  ";
      private boolean showXmlTags = false;
      private static boolean recordDeclareVariables = false;
      private static boolean recordCondition = false;      
      private static boolean recordEQ, recordNE, recordArray, lookupChange, lonelyValue, recordTime, tolerance, lookupNow, lonelyVariable = false;
      private static String save = "";
  
      private static String conditionEquation = "";
      private static ArrayList<String> equationHolder = new ArrayList<String>();
      private static ArrayList<String> lookupArguments = new ArrayList<String>();

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
         
         // store variable and conditon info before executing
         
         if (localName.equals(VAR_DECLS))
             recordDeclareVariables = true;
         
         if (localName.equals(TOLERANCE))
             tolerance = true;             
         
         if (localName.contains(CONDITION))
         {
             recordCondition = true;     
             recordEQ = recordNE = recordArray = lookupChange = lonelyValue = lonelyVariable = recordTime = tolerance = lookupNow = false;
             save = conditionEquation = "";
             equationHolder.clear();
             lookupArguments.clear();
         }
         
         if (localName.contains(EQ) || localName.contains(NE))
         {
             equationHolder.add("PlaceHolder");
             
             if (localName.contains(EQ))
                 recordEQ = true;
             else
                 recordNE = true;
         }     
         
         if (localName.equals(TIME_VAL))
         {
             if (recordEQ)
                 conditionEquation += " == [";  
             else
                 conditionEquation += " != [";
             
             recordTime = true;
         }
         
         if (localName.equals(ARRAYELEMENT))
         {            
             recordArray = true;
             
             if (!recordEQ)
                 equationHolder.add("PlaceHolder");
         }         
         
         if (localName.equals(NOT))
             conditionEquation = "!(";
         
         if (localName.contains(LOOKUP) && recordCondition)
         {
             if (conditionEquation.length() > 0 && !conditionEquation.equals("!("))
             {
                 if (recordEQ)
                     conditionEquation += " == ";
                 else
                     conditionEquation += " != ";
             }
             
             if (localName.contains(LOOKUPNOW))
             {
                conditionEquation += LOOKUPNOW;
                lookupNow = true;
             }
             else if (localName.equals(LOOKUPCHANGE)) 
             {
                conditionEquation += LOOKUPCHANGE;
                lookupChange = true;
             }            
   
             lookupArguments = new ArrayList<String>();
             conditionEquation += "(";
         }
         
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
         
         if (localName.equals(VAR_DECLS))
             recordDeclareVariables = false;
         
         if (localName.equals(NODE_OUTCOME_VAR)) 
             conditionEquation += ".outcome";
         if (localName.equals(NODE_FAILURE_VAR))
             conditionEquation += ".failure";
         if (localName.equals(NODE_STATE_VAR))
             conditionEquation += ".state";
         if (localName.equals(NODE_TIMEPOINT_VAR))
             conditionEquation += ".timepoint";
         if (localName.equals(NODE_CMD_HANDLE_VAR))
             conditionEquation += ".command_handle";
         
         if (localName.equals(TIME_VAL))
         {
             conditionEquation += "]";  
             conditionEquation = conditionEquation.replace("[, ", "[");
             equationHolder.add(conditionEquation);
             conditionEquation = "";          
             recordTime = false;
         }
         
         if (localName.equals(LT))
         {
             if (!equationHolder.isEmpty())
             {
                 int lastIndex = equationHolder.size() - 1;
                 String replace = equationHolder.get(lastIndex) + " < " + conditionEquation;
                 equationHolder.set(lastIndex,replace);
                 conditionEquation = "";
             }
         }
         
         if (localName.equals(GT))
         {
             if (!equationHolder.isEmpty())
             {
                 int lastIndex = equationHolder.size() - 1;
                 String replace = equationHolder.get(lastIndex) + " >= " + conditionEquation;
                 equationHolder.set(lastIndex,replace);
                 conditionEquation = "";
             }
         }
         
         if (localName.equals(LE))
         {
             if (!equationHolder.isEmpty())
             {
                 int lastIndex = equationHolder.size() - 1;
                 String replace = equationHolder.get(lastIndex) + " <= " + conditionEquation;
                 equationHolder.set(lastIndex,replace);
                 conditionEquation = "";
             }
         }
         
         if (localName.equals(GE))
         {
             if (!equationHolder.isEmpty())
             {
                 int lastIndex = equationHolder.size() - 1;
                 String replace = equationHolder.get(lastIndex) + " >= " + conditionEquation;
                 equationHolder.set(lastIndex,replace);
                 conditionEquation = "";
             }
         }
         
         if (localName.equals(IS_KNOWN))
         {
             if (!equationHolder.isEmpty())
             {
                 int lastIndex = equationHolder.size() - 1;
                 String replace = equationHolder.get(lastIndex) + " is known";
                 equationHolder.set(lastIndex,replace);
             }
             else
             {
                 conditionEquation += " is known";
                 equationHolder.add(conditionEquation);
                 conditionEquation = "";
             }
         }
         
         if (localName.contains(LOOKUP) && recordCondition)
         {
             String args = "";
             if (lookupArguments.size() > 1)
             {
                 if (lookupNow)
                 {
                     args = lookupArguments.get(0);
                     args += "(" + lookupArguments.get(1) + ")";
                 }
                 else if (lookupChange)
                 {
                     if (!tolerance)
                     {
                         args = lookupArguments.get(1);
                         args += "," + lookupArguments.get(0);
                     }
                     else
                     {
                         args = lookupArguments.get(0);
                         args += "," + lookupArguments.get(1);
                         tolerance = false;
                     }
                 }
             }
             else if (lookupArguments.size() == 1)
                 args = lookupArguments.get(0);
             
             conditionEquation += args + ")";
             
             equationHolder.add(conditionEquation);
             conditionEquation = "";
             lookupArguments.clear();
             
             lookupChange = false;
             lookupNow = false;
         }
         
         if (localName.contains(EQ) || localName.contains(NE) || (localName.equals(ARRAYELEMENT) && !recordEQ))
         {
             if (conditionEquation.length() > 0)
             {
                 if (lonelyValue || lonelyVariable)
                 {
                     if (!equationHolder.isEmpty())
                     {
                         int lastIndex = equationHolder.size() - 1;

                         if (localName.contains(EQ))
                             conditionEquation = " == " + conditionEquation;
                         else if (localName.contains(NE))
                             conditionEquation = " != " + conditionEquation;

                         String replace = equationHolder.get(lastIndex) + conditionEquation;
                         equationHolder.set(lastIndex,replace);
                     }
                     lonelyValue = lonelyVariable = false;
                 }
                 else           
                     equationHolder.add(conditionEquation);

                 conditionEquation = "";
             }
                              
             if (localName.contains(EQ))
                 recordEQ = false;
             else if (localName.contains(NE))
                 recordNE = false;
         }
         
          if (localName.equals(NOT))
          {
             if (!equationHolder.isEmpty())
             {
                 int lastIndex = equationHolder.size() - 1;
                 String replace = equationHolder.get(lastIndex) + ")";
                 equationHolder.set(lastIndex,replace);
             }
             else
             {
                 conditionEquation += ")";
                 equationHolder.add(conditionEquation);
                 conditionEquation = "";
             }
          }
         
         if (localName.equals(AND) || localName.equals(OR))
         {
             for (int i = equationHolder.size() - 1; i >= 0; i--)
             {
                 if (equationHolder.get(i).equals("PlaceHolder"))
                 {
                     equationHolder.set(i, localName);
                     break;
                 }
             }
             
             save = localName;
         }
         
         if (localName.contains(CONDITION))
         {
             recordCondition = false;
             int condition = -1;
             
             for (int i = 0; i < ALL_CONDITIONS.length; i++)
             {
                 if (localName.equals(ALL_CONDITIONS[i]))
                 {
                         condition = i;
                         break;
                 }
             } 
             
             if (equationHolder.size() > 1)
                 equationHolder.remove(0);
             
             if (equationHolder.contains("PlaceHolder"))
             {
                 for (int i = equationHolder.size() - 1; i >= 0; i--)
                 {
                     if (equationHolder.get(i).equals("PlaceHolder"))
                     {
                         equationHolder.set(i, save);
                     }
                 }
             }
             
             findFirstNonNullNode().addConditionInfo(condition, equationHolder);
             
             equationHolder = new ArrayList<String>();
         }

         if (text != null)
         {
             if (recordDeclareVariables)
                findFirstNonNullNode().addLocalVariableName(localName, text);
             
             if (recordCondition)
             {
                 if (localName.contains(VAR) || localName.equals(NAME) || localName.equals(NODEREF) || localName.equals(NODE_ID))
                 {
                     lonelyValue = false;
                     
                     if (lookupChange || lookupNow)
                         lookupArguments.add(text);
                     else
                     {
                         if (conditionEquation.length() == 0)
                         {
                             lonelyVariable = true;
                             conditionEquation += text;
                         }
                         else if (conditionEquation.equals("!("))
                             conditionEquation += text;
                         else
                         {
                             if (recordEQ)
                                 conditionEquation += " == " + text;
                             else
                                 conditionEquation += " != " + text;
                             
                             lonelyVariable = false;
                         }
                     }
                 }
                 else if (localName.equals(STRING_VAL))
                 {
                     lonelyVariable = false;
                     if (lookupChange || lookupNow)
                     {
                         text = "\"" + text + "\"";
                         lookupArguments.add(text);
                     }
                     else
                         conditionEquation += " == \"" + text + "\"";
                 }
                 else if (localName.contains(VAL))
                 {
                     lonelyVariable = false;
                     if (recordArray)
                     {
                         conditionEquation += "[" + text + "]";
                         recordArray = false;
                     }
                     else if (conditionEquation.length() > 0 && !recordTime)
                     {
                         if (recordNE)
                             conditionEquation += " != " + text;
                         else
                             conditionEquation += " == " + text;
                     }
                     else if (recordTime)
                     {
                         conditionEquation += ", " + text;
                     }
                     else
                     {
                         lonelyValue = true;
                         conditionEquation = text;
                     }
                 }                     
             }

             if (node != null)
                node.setProperty(localName, text);
             if (showXmlTags)
                out.println(indent + text);
         }
         else if (localName.equals(NODEREF))
             conditionEquation += findFirstNonNullNode().getProperty(NODE_ID);

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
