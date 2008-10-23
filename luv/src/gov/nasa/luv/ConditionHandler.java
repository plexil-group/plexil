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

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;

import static gov.nasa.luv.Constants.*;

public class ConditionHandler 
{
    Model model;
    
    FileOutputStream out; // declare a file output object
    PrintStream p;
    
    public HashMap<String, HashMap<Integer, ArrayList>> nodeConditions = new HashMap<String, HashMap<Integer, ArrayList>>();
    
    public ConditionHandler(Model model)
    {
        this.model = model;
        saveConditionInfo(this.model);
    }
    
    public void saveConditionInfo(Model node)
    {
        if(!node.conditionMap.isEmpty())
            nodeConditions.put(node.getPath(), node.conditionMap);
         
        for (int i = 0; i < node.getChildCount(); i++)
        {
            if (!node.getChild(i).conditionMap.isEmpty())
                nodeConditions.put(node.getChild(i).getPath(), node.getChild(i).conditionMap);
            saveConditionInfo(node.getChild(i));
        }
    }
    
    public void openConditionsFile(Model node) throws FileNotFoundException
    {
        out = new FileOutputStream(node.getPlanName() + "-conditions.txt");

        // Connect print stream to the output stream
        p = new PrintStream(out);
    }
    
    public void outputAllConditions(Model node)
    {
        String nodePath = node.getPath();         
    
        if (nodePath != null)
        {
            p.println();
            p.println(nodePath + ":");

            if (!node.conditionMap.isEmpty())
            {
                
                Object[] array = nodeConditions.get(nodePath).keySet().toArray();
                
                for (int j = 0; j < array.length; j++)
                {
                    p.println("\t" + getConditionNameFromNumber((Integer) array[j]) + "Condition:");
                    ArrayList array2 = nodeConditions.get(nodePath).get(array[j]);
                    
                    for (int g = 0; g < array2.size(); g++)
                        p.println("\t\t" + array2.get(g));
                }
            }
            else
                p.println("\tNo conditions for this node.");   
            
            for (int i = 0; i < node.getChildCount(); i++)
            {
                outputAllConditions(node.getChild(i));
            }
        }
        else
        {
            for (int i = 0; i < node.getChildCount(); i++)
            {
                outputAllConditions(node.getChild(i));
            }
        }
        
        
    }
    
    public void closeConditionFile()
    {
        p.close();
    }
}
