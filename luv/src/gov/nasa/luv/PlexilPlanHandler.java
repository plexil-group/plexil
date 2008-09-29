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

/** SAX PlexilPlan XML handler */

public class PlexilPlanHandler extends AbstractDispatchableHandler
{
      private static boolean recordDeclareVariables, valueExists, recordArrayInitValues = false;  
      private static String arrayValues = "";
      
      private static boolean recordAssignments, recordRHS, recordArrayElement = false;
      private static ArrayList<String> operatorHolder = new ArrayList<String>();
      private static String owner = "";
      private static String arrayName = "";
      
      private static boolean recordCondition, recordEQ, recordNE, recordArray, lookupChange, 
              lonelyValue, recordTime, tolerance, lookupNow, lonelyVariable, recordNodeTimepoint = false;
      private static String save = "";  
      private static String conditionEquation = "";
      private static ArrayList<String> equationHolder = new ArrayList<String>();
      private static ArrayList<String> lookupArguments = new ArrayList<String>(); 
      
      private static boolean recordLibraryNames = false;

      private Stack<Model> stack = new Stack<Model>();
      
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

      public void startElement(String uri, String tagName, 
                               String qName, Attributes attributes)
      {
          
         // get the current node in the stack
         
         Model node = stack.peek();

         // if this SHOULD be a child node, make that happen

         if (Model.isNode(tagName))
         {
            Model child = new Model(tagName);

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
            }
            
         }

         // else if its not a property the we can ignore it
         
         else if (!Model.isProperty(tagName))
            node = null;
         
         // check if tagName indicates that we will need to store variable 
         // and conditon info before executing
         
         catchStartTag(tagName);
        
         // push new node onto the stack

         stack.push(node);
      }

      /** Handle end of an XML element. */

    @Override
      public void endElement(String uri, String tagName, String qName)
      {
         Model topNode = stack.peek();
         Model nodeToUpdate = getNodeToUpdate();         
        
         // check if tagName indicates that we are finishing storage of info
         // about a certain variable or condition
         
         catchEndTag(nodeToUpdate, tagName);
         
         // get tweener text and put it in it's place

         String text = getTweenerText();
         
         // assign info (text) to the appropriate variable or condition
         
         assignTweenerText(nodeToUpdate, tagName, text);
 
         // assign model name to the appropriate model
         
         if (topNode != null && text != null)
             topNode.setProperty(tagName, text);

         // if node, assign the main attributes for display in luv viewer
         // name, type, state and outcome

         if (topNode != null && tagName.equals(topNode.getType()))
            topNode.setMainAttributesOfNode();

         // pop the node off the stack

         stack.pop();        
      }

      /** Handel end of document event. */

      public void endDocument()
      {
         model.planChanged();
      }
      
      public void catchStartTag(String tagName)
      {
         if (tagName.equals(VAR_DECLS))
         {
             recordDeclareVariables = true;
             valueExists = false;
         }
         else if (tagName.equals(LIBRARYNODECALL))
         {
             recordLibraryNames = true;
         }
         else if (tagName.equals(ARRAYELEMENT) && recordAssignments)
         {
             recordArrayElement = true;
         }
         else if (tagName.equals(DECL_ARRAY))
         {
             recordArrayInitValues = true;
             arrayValues = "{";
         }
         else if (tagName.equals(ASSN))
         {
             recordAssignments = true;
             operatorHolder = new ArrayList<String>();
         }
         else if (tagName.contains(RHS))
             recordRHS = true;
         else if (tagName.contains(CONDITION))
         {
             recordCondition = true;     
             recordEQ = recordNE = recordArray = lookupChange = lonelyValue = lonelyVariable = recordTime = tolerance = lookupNow = false;
             save = conditionEquation = "";
             equationHolder.clear();
             lookupArguments.clear();
         }  
         else if (tagName.equals(NODE_TIMEPOINT_VAL))
             recordNodeTimepoint = true;


         if (recordCondition)
             recordStartConditionInfo(tagName);
      }
      
      public void assignTweenerText(Model nodeToUpdate, String tagName, String text)
      {
         if (text != null)
         {           
             if (recordDeclareVariables)
             {
                 if (tagName.contains(VAL) && !recordArrayInitValues)
                 {
                     valueExists = true;
                     nodeToUpdate.recordVariableDeclarations(tagName, text);
                 }
                 else if (tagName.contains(VAL) && recordArrayInitValues)
                 {
                     arrayValues += "," + text;
                 }
                 else
                     nodeToUpdate.recordVariableDeclarations(tagName, text);
             }             

             if (recordAssignments)
             {
                 if (tagName.contains(VAR) && !recordRHS)
                 {
                     nodeToUpdate.setUpdateVariableMap(NAME, text);
                     findVariableOwner(nodeToUpdate, text);
                     nodeToUpdate.setUpdateVariableMap("Locale", owner);
                 }
                 else if (recordArrayElement)
                 {
                     if (tagName.equals(NAME))
                         arrayName = text;                    
                     else if (!recordRHS)
                     {
                         arrayName += "[" + text + "]";

                         nodeToUpdate.setUpdateVariableMap(NAME, arrayName);
                         findVariableOwner(nodeToUpdate, text);
                         nodeToUpdate.setUpdateVariableMap("Locale", owner);
                     }
                     else
                     {
                         Integer index = Integer.getInteger(text);
                         
                         if (index == null)
                             index = Integer.parseInt(text);
                         
                         if (index == null)
                         {
                             String textValue = nodeToUpdate.getVariableValue(nodeToUpdate, text);
                             if (textValue.equals("error"))
                                 ;  // error resolving what index of the array is
                             else
                                 operatorHolder.add(nodeToUpdate.getArrayElementValue(nodeToUpdate, arrayName, Integer.parseInt(textValue)));
                         }
                         else
                             operatorHolder.add(nodeToUpdate.getArrayElementValue(nodeToUpdate, arrayName, index));
                     }
                 }
                 else
                     operatorHolder.add(text);
             }
             
             if (recordLibraryNames)
             {
                 if (!Luv.getLuv().getLibraryNames().containsKey(text))
                 {
                     String fullPath = Luv.getLuv().getFileHandler().getLibrary(text);
                     
                     if (fullPath == null)
                     {
                         Luv.getLuv().addLibraryName(text, text);
                     }
                     else
                     {
                         Luv.getLuv().addLibraryName(text, fullPath);
                     }
                 }

                 recordLibraryNames = false;
             }

             if (recordCondition)
                recordMiddleConditionInfo(tagName, text);
             
         }
         else if (tagName.equals(NODEREF))
             conditionEquation += nodeToUpdate.getProperty(NODE_ID);
      }
      
      public void catchEndTag(Model nodeToUpdate, String tagName)
      {        
         if (tagName.equals(VAR_DECLS))
         {
             recordDeclareVariables = false;
         }
         else if (tagName.equals(DECL_ARRAY))
         {
             valueExists = false;
             recordArrayInitValues = false;
             arrayValues += "}";
             arrayValues = arrayValues.replace("{,", "{");
             
             if (arrayValues.equals("{}"))
                 nodeToUpdate.recordVariableDeclarations(VAL, UNKNOWN);                 
             else
                 nodeToUpdate.recordVariableDeclarations(VAL, arrayValues);
         }   
         else if (tagName.equals(ARRAYELEMENT))
         {
             recordArrayElement = false;
         }
         else if (tagName.equals(DECL_VAR))
         {
             if (!valueExists)
                 nodeToUpdate.recordVariableDeclarations(VAL, UNKNOWN);
             valueExists = false;
         }
         else if (tagName.equals(ASSN))
         {
             recordAssignments = false;
             operatorHolder.clear();
         }
         else if (tagName.contains(RHS))
         {
             recordRHS = false;
             String update = operatorHolder.get(0);
             nodeToUpdate.setUpdateVariableMap(VAL, update);
         }
         else if (tagName.equals(ADD) ||
                  tagName.equals(SUB) ||
                  tagName.equals(MUL) ||
                  tagName.equals(DIV))

         {
             if (recordDeclareVariables || recordAssignments)
             {                     
                 addOperatorToEquation(tagName);
             }
             else if (recordCondition)
             {
                 if (tagName.equals(ADD))
                     tagName = "+";
                 if (tagName.equals(SUB))
                     tagName = "-";
                 if (tagName.equals(MUL))
                     tagName = "*";
                 if (tagName.equals(DIV))
                     tagName = "/";
                 conditionEquation = conditionEquation.replace("PlaceHolder", tagName);
             }
         }

         if (recordCondition)
             recordEndConditionInfo(tagName);
      }
      
      public void findVariableOwner(Model node, String text)
      {
            if (node != null && node.declNameVarList.contains(text))                        
                owner = node.getProperty(NODE_ID);
            else if (node != null)
                findVariableOwner(node.getParent(), text);
      }
      
      public Model getNodeToUpdate()
      {
            int i = stack.size() - 1;
            while (stack.elementAt(i) == null)
               i--; 
            return stack.elementAt(i);
      } 
      
      public void recordStartConditionInfo(String tagName)
      { 
         int name = getTagName(tagName);
         
         switch (name)
         {
             case TOLERANCE_NUM: tolerance = true; break;
             case EQ_NUM: 
                 equationHolder.add("PlaceHolder");
                 recordEQ = true;
                 break;
             case NE_NUM:
                 equationHolder.add("PlaceHolder");
                 recordNE = true;
                 break;
             case TIME_NUM:
                 if (recordEQ)
                     conditionEquation += " == [";  
                 else
                     conditionEquation += " != [";
                 recordTime = true;
                 break;
             case ARRAYELEMENT_NUM:
                 recordArray = true;             
                 if (!recordEQ)
                     equationHolder.add("PlaceHolder");
                 break;
             case NOT_NUM: conditionEquation = "!("; break;
             case LOOKUP_NUM:
                 if (conditionEquation.length() > 0 && !conditionEquation.equals("!("))
                 {
                     if (recordEQ)
                         conditionEquation += " == ";
                     else
                         conditionEquation += " != ";
                 }

                 if (tagName.contains(LOOKUPNOW))
                 {
                    conditionEquation += LOOKUPNOW;
                    lookupNow = true;
                 }
                 else if (tagName.equals(LOOKUPCHANGE)) 
                 {
                    conditionEquation += LOOKUPCHANGE;
                    lookupChange = true;
                 }            

                 lookupArguments = new ArrayList<String>();
                 conditionEquation += "(";
                 break;                
         }        
      }
      
      public void recordMiddleConditionInfo(String tagName, String text)
      {
             if (tagName.contains(VAR) || tagName.equals(NAME) || tagName.equals(NODEREF) || tagName.equals(NODE_ID) || tagName.equals(STATE_NAME))
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
             else if (tagName.equals(STRING_VAL))
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
             else if (tagName.contains(VAL))
             {
                 lonelyVariable = false;
                 
                 if (tolerance)
                 {
                     lookupArguments.add(text);
                 }
                 else if (recordNodeTimepoint)
                 {
                     lonelyVariable = true;
                     conditionEquation += "." + text;
                 }
                 else if (recordArray)
                 {
                     conditionEquation += "[" + text + "]";
                     recordArray = false;
                 }
                 else if (conditionEquation.length() > 0 && !recordTime)
                 {
                     if (recordNE)
                         conditionEquation += " != " + text;
                     else if (recordEQ)
                         conditionEquation += " == " + text;
                     else
                         conditionEquation += " PlaceHolder " + text;
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
             else if (tagName.equals(TIMEPOINT))
             {
                 lonelyVariable = true;
                 conditionEquation += "." + text;
             }                
      }
      
      public void recordEndConditionInfo(String tagName)
      {
          
         int name = getTagName(tagName);
         
         switch (name)
         {
             case NODE_OUTCOME_NUM:     conditionEquation += ".outcome"; break;
             case NODE_FAILURE_NUM:     conditionEquation += ".failure"; break;               
             case NODE_STATE_NUM:       conditionEquation += ".state"; break;   
             case NODE_TIMEPOINT_NUM:   conditionEquation += ".timepoint"; break; 
             case NODE_CMD_HANDLE_NUM:  conditionEquation += ".command_handle"; break; 
             case TIME_NUM: 
                 conditionEquation += "]";  
                 conditionEquation = conditionEquation.replace("[, ", "[");
                 equationHolder.add(conditionEquation);
                 conditionEquation = "";          
                 recordTime = false;
                 break;
             case LT_NUM:
             case GT_NUM:
             case LE_NUM:        
             case GE_NUM:  
                 if (!equationHolder.isEmpty())
                 {
                     String update = "";
                     switch (name)
                     {
                         case LT_NUM: update = " < "; break;
                         case GT_NUM: update = " > "; break;
                         case LE_NUM: update = " <= "; break;
                         case GE_NUM: update = " >= "; break;
                     }
                     
                     update += conditionEquation;                     
                     updateEquation(update);
                 }
                 break;   
             case IS_KNOWN_NUM: 
                 if (!equationHolder.isEmpty())
                 {
                     updateEquation(" is known");
                 }
                 else
                 {
                     conditionEquation += " is known";
                     equationHolder.add(conditionEquation);
                     conditionEquation = "";
                 }
                 break;       
             case LOOKUP_NUM:
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
                 break;
             case EQ_NUM:                  
             case NE_NUM:
                 recordEquation(tagName);
                 break;
             case ARRAYELEMENT_NUM:
                 if (!recordEQ)
                     recordEquation(tagName);
                 break;
             case NOT_NUM: 
                 if (!equationHolder.isEmpty())
                 {
                     updateEquation(")");
                 }
                 else
                 {
                     conditionEquation += ")";
                     equationHolder.add(conditionEquation);
                     conditionEquation = "";
                 }
                 break;
             case AND_NUM:
             case OR_NUM: 
                 for (int i = equationHolder.size() - 1; i >= 0; i--)
                 {
                     if (equationHolder.get(i).equals("PlaceHolder"))
                     {
                         equationHolder.set(i, tagName);
                         break;
                     }
                 }

                 save = tagName;
                 break;
             case CONDITION_NUM: 
                 recordCondition = false;
                 int condition = -1;

                 for (int i = 0; i < ALL_CONDITIONS.length; i++)
                 {
                     if (tagName.equals(ALL_CONDITIONS[i]))
                     {
                             condition = i;
                             break;
                     }
                 } 
                 
                 if (conditionEquation.length() > 0)
                 {
                     equationHolder.add(conditionEquation);
                     conditionEquation = "";
                 }

                 if (equationHolder.size() > 1)
                 {
                     if (equationHolder.get(0).equals(OR) || equationHolder.get(0).equals(AND) || equationHolder.get(0).equals("PlaceHolder"))
                         equationHolder.remove(0);
                 }

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

                 getNodeToUpdate().addConditionInfo(condition, equationHolder);

                 equationHolder = new ArrayList<String>();
                 break;
             case NODE_TIMEPOINT_VAL_NUM:
                 recordNodeTimepoint = false;
                 break;
         }        
      }  
      
      public void recordEquation(String tagName)
      {
          if (conditionEquation.length() > 0)
             {
                 if (lonelyValue || lonelyVariable)
                 {
                     if (!equationHolder.isEmpty())
                     {
                         int lastIndex = equationHolder.size() - 1;

                         if (tagName.contains(EQ))
                             conditionEquation = " == " + conditionEquation;
                         else if (tagName.contains(NE))
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
                              
             if (tagName.contains(EQ))
                 recordEQ = false;
             else if (tagName.contains(NE))
                 recordNE = false;
      }
      
      public void updateEquation(String update)
      {
          int lastIndex = equationHolder.size() - 1;
          String replace = equationHolder.get(lastIndex) + update;
          equationHolder.set(lastIndex,replace);
          conditionEquation = "";
      }
      
      public void addOperatorToEquation(String operator)
      {
          String update = "";
          
          if (operator.equals(MUL))
          {
              update = operatorHolder.get(operatorHolder.size() - 2) + " * " + operatorHolder.get(operatorHolder.size() - 1);
              operatorHolder.remove(operatorHolder.size() - 1);
              operatorHolder.remove(operatorHolder.size() - 1);
          }
          else if (operator.equals(DIV))
          {
              update = operatorHolder.get(operatorHolder.size() - 2) + " / " + operatorHolder.get(operatorHolder.size() - 1);
              operatorHolder.remove(operatorHolder.size() - 1);
              operatorHolder.remove(operatorHolder.size() - 1);
          }
          else if (operator.equals(ADD))
          {
              update = operatorHolder.get(operatorHolder.size() - 2) + " + " + operatorHolder.get(operatorHolder.size() - 1);
              operatorHolder.remove(operatorHolder.size() - 1);
              operatorHolder.remove(operatorHolder.size() - 1);
          }
          else if (operator.equals(SUB))
          {
              update = operatorHolder.get(operatorHolder.size() - 2) + " - " + operatorHolder.get(operatorHolder.size() - 1);
              operatorHolder.remove(operatorHolder.size() - 1);
              operatorHolder.remove(operatorHolder.size() - 1);
          }
       
          operatorHolder.add(update);
      }
}
