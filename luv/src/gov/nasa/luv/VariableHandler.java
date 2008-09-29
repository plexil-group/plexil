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
import java.util.HashMap;
import java.util.Stack;
import java.util.StringTokenizer;

import static gov.nasa.luv.Constants.*;

public class VariableHandler 
{
    private Model model;
    
    private HashMap<String, HashMap<String, ArrayList>> nodeDeclVariables = new HashMap<String, HashMap<String, ArrayList>>(); 
    private HashMap<String, HashMap<String, ArrayList>> nodeUpdates = new HashMap<String, HashMap<String, ArrayList>>();
    private ArrayList<String> nameList = new ArrayList<String>();
    
    private HashMap<String, HashMap<String, ArrayList>> nodeDeclVariablesCopy = new HashMap<String, HashMap<String, ArrayList>>(); 
    private HashMap<String, HashMap<String, ArrayList>> nodeUpdatesCopy = new HashMap<String, HashMap<String, ArrayList>>();
    
    public VariableHandler(Model model)
    {
        this.model = model;
        saveVariableInfo(this.model);
        makeCopyOfVariableInfo(this.model);
    }
    
    public void saveVariableInfo(Model node)
    {
        nodeDeclVariables.put(node.getProperty(NODE_ID), node.declVarMap);
        nodeUpdates.put(node.getProperty(NODE_ID), node.updateVarMap);
        if (node.getProperty(NODE_ID) != null && !nameList.contains(node.getProperty(NODE_ID)))
            nameList.add(node.getProperty(NODE_ID));
         
        for (int i = 0; i < node.getChildCount(); i++)
        {
            nodeDeclVariables.put(node.getChild(i).getProperty(NODE_ID), node.getChild(i).declVarMap);
            nodeUpdates.put(node.getChild(i).getProperty(NODE_ID), node.getChild(i).updateVarMap);
            if (node.getProperty(NODE_ID) != null && !nameList.contains(node.getProperty(NODE_ID)))
                nameList.add(node.getProperty(NODE_ID));
            saveVariableInfo(node.getChild(i));
        }
    }
    
  @SuppressWarnings("unchecked")  
    public void applyUpdates(String nodeName)
    {
        if (nodeUpdates.get(nodeName) != null && !nodeUpdates.get(nodeName).isEmpty())
        {
            String nodeToUpdate = nodeUpdates.get(nodeName).get("Locale").toString();
            String variableToUpdate = nodeUpdates.get(nodeName).get(NAME).toString();
            nodeToUpdate = nodeToUpdate.substring(1, nodeToUpdate.length() - 1);
            variableToUpdate = variableToUpdate.substring(1, variableToUpdate.length() - 1);
            if (variableToUpdate.contains("["))
            {
                int start = variableToUpdate.indexOf('[') + 1;
                int end = variableToUpdate.indexOf(']');
                int element = Integer.parseInt(variableToUpdate.substring(start, end));                
                variableToUpdate = variableToUpdate.substring(0, start - 1);
                int length = variableToUpdate.length();
                int index = -1;
                
                for (int i = 0; i < nodeDeclVariables.get(nodeToUpdate).get(NAME).size(); i++)
                {
                    if (nodeDeclVariables.get(nodeToUpdate).get(NAME).get(i).toString().length() >= length)
                    {
                        if (nodeDeclVariables.get(nodeToUpdate).get(NAME).get(i).toString().substring(0, length).equals(variableToUpdate))
                        {
                            index = i;
                            break;
                        }
                    }
                }
                
                if (index != -1)
                {
                    String newValue = processNewValue(nodeDeclVariables.get(nodeToUpdate), nodeUpdates.get(nodeName).get(VAL).toString(), index);
                    String temp = nodeDeclVariables.get(nodeToUpdate).get(VAL).get(index).toString();
                    temp = temp.substring(1, temp.length() - 1);
                    int start2 = nodeDeclVariables.get(nodeToUpdate).get(NAME).get(index).toString().indexOf('[') + 1;
                    int end2 = nodeDeclVariables.get(nodeToUpdate).get(NAME).get(index).toString().indexOf(']');
                    if (start2 != -1 && end2 != -1)
                    {
                        int oldArraySize = Integer.parseInt(nodeDeclVariables.get(nodeToUpdate).get(NAME).get(index).toString().substring(start2, end2));
                        String[] array = temp.split(",");
                        int newArraySize = array.length;

                        if (element < oldArraySize)
                        {
                            if (element < newArraySize)
                            {
                                array[element] = newValue;
                            }
                            else
                            {
                                String newArray = "";
                                int extra = 0;
                                for (int i = 0; i < newArraySize; i++)
                                {
                                    newArray += array[i] + ",";
                                    extra = i;
                                }
                                if (extra + 1 == element)
                                    newArray += newValue;
                                else
                                {
                                    do
                                    {
                                        extra++;
                                        newArray += ",";

                                    } while (extra != element);

                                    newArray += newValue;                                
                                }
                                newArray = "{" + newArray + "}";
                                newValue = newArray;
                            }
                        }                   
                        nodeDeclVariables.get(nodeToUpdate).get(VAL).set(index, newValue);
                    }
                }
            }
            else
            {
                int index = nodeDeclVariables.get(nodeToUpdate).get(NAME).indexOf(variableToUpdate);
                if (index != -1)
                {
                    String newValue = processNewValue(nodeDeclVariables.get(nodeToUpdate), nodeUpdates.get(nodeName).get(VAL).toString(), index);
                    newValue = newValue.replace("[", "");
                    newValue = newValue.replace("]", "");
                    nodeDeclVariables.get(nodeToUpdate).get(VAL).set(index, newValue);             
                }
            }
        }      
    }
  
  public String processNewValue(HashMap<String, ArrayList> original, String update, int index)
  {
      if (update.contains(" "))
      {
          String type = (String) original.get(TYPE).get(index);
          type = type.replace("[", "");
          type = type.replace("]", "");
          update = update.replace("[", "");
          update = update.replace("]", "");
          String update2 = update;
          String type2 = "";
          
          String name = "";
          String value = "";

          StringTokenizer st = new StringTokenizer(update2); 

          for (int i = 0; i < original.get(NAME).size(); i++)
          {
              name = original.get(NAME).get(i).toString().replace("[", "");
              name = name.replace("]", "");
              value = original.get(VAL).get(i).toString().replace("[", "");
              value = value.replace("]", "");
              type2 = original.get(TYPE).get(i).toString().replace("[", "");
              type2 = type.replace("]", "");

              st = new StringTokenizer(update2);
              update2 = "";

              while (st.hasMoreTokens()) 
              {
                  String token = st.nextToken();

                  if (token.equals(name))
                      update2 += value + " ";
                  else
                      update2 += token + " ";              
              }
          }

          update2 = update2.substring(0, update2.length() - 1);
          
          if (!update2.equals(update))
              return calculateEquation(update2, type2);
          else
              return calculateEquation(update, type);
      }
      else
          return update;
  }
  
  public String calculateEquation(String update, String type)
  {
      return infixToPostfix(new StringTokenizer(update), type);
  }
 
  @SuppressWarnings("unchecked")  
  public String infixToPostfix(StringTokenizer tokenizer, String type)
  {      
      Stack operators = new Stack();
      String operands = "";
      
      while (tokenizer.hasMoreTokens())
      {
          String token = tokenizer.nextToken();
          
          if (token.equals("+") || token.equals("-"))
          {
              if (operators.empty())
                  operators.push(token);
              else
              {
                  while (!operators.isEmpty())
                  {
                      operands += operators.pop() + " ";                    
                  }
                  operators.push(token); 
              }
          }
          else if (token.equals("*") || token.equals("/"))
          {
              operators.push(token);
          }
          else
          {
              operands += token + " ";
          }
      }   
      
      while (!operators.empty())
          operands += operators.pop() + " ";
      
      return calcluatePostfix(operands, type);
  }
  
  @SuppressWarnings("unchecked")
  public String calcluatePostfix(String postfix, String type)
  {
      StringTokenizer tokens = new StringTokenizer(postfix);
      Stack result = new Stack();
      String temp = "";
      float num1 = (float)0.0;
      float num2 = (float)0.0;
      float num3 = (float)0.0;
      int num4 = 0;
      
      while (tokens.hasMoreTokens())
      {
          String element = tokens.nextToken();
          
          if (element.equals("+") || element.equals("-") || element.equals("*") || element.equals("/"))
          {
              if (result.peek().equals(UNKNOWN))
                 return UNKNOWN;
              num1 = Float.parseFloat((String)result.pop());
              
              if (result.peek().equals(UNKNOWN))
                 return UNKNOWN;
              num2 = Float.parseFloat((String)result.pop());                           
              
              if (element.equals("+"))
                  num3 = num2 + num1;
              else if (element.equals("-"))
                  num3 = num2 - num1;
              else if (element.equals("*"))
                  num3 = num2 * num1;
              else if (element.equals("/"))
                  num3 = num2 / num1;
              
              if (type.equals(INT))
              {
                  num4 = (int)num3;
                  result.push(Integer.toString(num4));
              }
              else
                  result.push(Float.toString(num3));
          }
          else
              result.push(element);
      }
      
      if (result.isEmpty())
          return null;
      else
          return (String)result.peek();
  }
  
  @SuppressWarnings("unchecked")
    public HashMap<String, HashMap<String, ArrayList>> getVariableMap()
    {
        return nodeDeclVariables;
    }
    
    @SuppressWarnings("unchecked")
    public void makeCopyOfVariableInfo(Model node)
    {
        nodeDeclVariablesCopy.put(node.getProperty(NODE_ID),(HashMap<String, ArrayList>) node.declVarMap.clone());
        nodeUpdatesCopy.put(node.getProperty(NODE_ID),(HashMap<String, ArrayList>) node.updateVarMap.clone());
         
        for (int i = 0; i < node.getChildCount(); i++)
        {
            nodeDeclVariablesCopy.put(node.getChild(i).getProperty(NODE_ID),(HashMap<String, ArrayList>) node.getChild(i).declVarMap.clone());
            nodeUpdatesCopy.put(node.getChild(i).getProperty(NODE_ID),(HashMap<String, ArrayList>) node.getChild(i).updateVarMap.clone());
            makeCopyOfVariableInfo(node.getChild(i));
        }
    }
    
@SuppressWarnings("unchecked")    
    public void resetVariableMap()
    {
        for (String name: nameList)
        {
            nodeDeclVariables.put(name,(HashMap<String, ArrayList>) nodeDeclVariablesCopy.get(name).clone());
            nodeUpdates.put(name,(HashMap<String, ArrayList>) nodeUpdatesCopy.get(name).clone());
        }
    }
}
