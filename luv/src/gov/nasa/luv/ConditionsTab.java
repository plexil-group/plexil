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

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Color;
import java.util.ArrayList;
import java.util.HashMap;
import static gov.nasa.luv.Constants.*;

public class ConditionsTab extends JPanel 
{
    private ConditionsTab conditionsPane;
    private Model model;    
    private int rows;    
    private String info[][];    
    private JTable table;     
    
    public ConditionsTab() {}
    
    public ConditionsTab(Model model) 
    {       
        super(new GridLayout(1,0));
        
        this.model = model;
        rows = 1000;
        info = new String[rows][3];
        
        String[] columnNames = {"Conditions",
                                "Value",
                                "Expression"};             
                
        HashMap<Integer, ArrayList> nodeConditions = model.getConditionMap();
        int row = 0;
        int col = 0;
        if (nodeConditions != null)
        {
            for (final String condition: ALL_CONDITIONS)
            {
                ArrayList elements = nodeConditions.get(getConditionNum(condition));            

                if (elements != null && !elements.isEmpty())
                {
                    info[row][col] = condition; 
                    ++col;
                    info[row][col] = getConditionValue(condition);                    
                    ++col;

                    for (int i = 0; i < elements.size(); i++)
                    {
                        info[row][2] = (String) elements.get(i);
                        ++row;
                    }
                   
                    ++row;
                    col = 0;
                }

                // add model listener
                this.model.addChangeListener(new Model.ChangeAdapter()
                   {
                         @Override 
                         public void propertyChange(Model model, String property)
                         {
                            if (property.equals(condition))
                            {
                                for (int i = 0; i < rows; i++)
                                {
                                    if (condition.equals(info[i][0]))
                                    {
                                        info[i][1] = getConditionValue(condition);
                                        table.setValueAt(getConditionValue(condition), i, 1);
                                        table.repaint();
                                        break;
                                    }
                                }
                             }
                         }
                   });
            }
        }
        
        table = new JTable(info, columnNames);
        table.setAutoResizeMode(JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS);
        table.getColumnModel().getColumn(0).setPreferredWidth(200);
        table.getColumnModel().getColumn(1).setPreferredWidth(100);
        table.getColumnModel().getColumn(2).setPreferredWidth(600);        
        table.setPreferredScrollableViewportSize(new Dimension(900, 300));
        table.setShowGrid(false);
        table.setGridColor(Color.GRAY);
        JScrollPane scrollPane = new JScrollPane(table);

        add(scrollPane);
    }
    
    private String getConditionValue(String condition)
    {
        if (model.getProperty(condition) == null)
            return UNKNOWN;
        else if (model.getProperty(condition).equals("0"))
            return "FALSE";
        else if (model.getProperty(condition).equals("1"))
            return "TRUE";
        else if (model.getProperty(condition).equals("inf"))
            return "inf";
        else
            return model.getProperty(condition);
    }
    
    public static ArrayList<String> formatCondition(String condition)
    {
        String tempCondition = "";
        ArrayList<String> formattedCondition = new ArrayList<String>();
        
        if (condition != null)
        {
            if (condition.contains(SEPARATOR))
            {
                String array[] = condition.split(SEPARATOR); 

                for (int i = 0; i < array.length; i++)
                {
                    tempCondition += array[i] + " ";

                    if (array[i].equals("||") || array[i].equals("&&"))
                    {
                        formattedCondition.add(tempCondition);
                        tempCondition = "";
                    }
                }

                if (!tempCondition.equals(""))
                    formattedCondition.add(tempCondition);
            }
            else if (!condition.equals(""))
            {
                formattedCondition.add(condition);
            }
        }  
        else
        {
            formattedCondition.add("COULD NOT IDENTIFY CONDITION");
        }         
        
        return formattedCondition;
    }
    
    public ConditionsTab getCurrentConditionsTab()
    {
        return conditionsPane;
    }

    public void open(Model model) 
    {       
        conditionsPane = new ConditionsTab(model);
        conditionsPane.setOpaque(true);
    }
}
