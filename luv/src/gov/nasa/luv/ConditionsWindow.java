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

public class ConditionsWindow extends JPanel 
{
    
    private Model model;
    private String status = UNKNOWN;
    private ConditionsWindow conditionsPane;
    private int rows = 1000;
    private int columns = 3;
    private String info[][];
    private JTable table;     
    
    private HashMap nodeConditions;
    private ArrayList elements;
    
    public ConditionsWindow() {}
    
    public ConditionsWindow(Model model) 
    {       
        super(new GridLayout(1,0));
        
        this.model = model;

        String[] columnNames = {"Conditions",
                                "Value",
                                "Expression"};
        
        int row = 0;
        int col = 0;
        info = new String[rows][columns];
        
        nodeConditions = model.getConditionMap();
        
        if (nodeConditions != null)
        {
            for (final String condition: ALL_CONDITIONS)
            {
                elements = (ArrayList) nodeConditions.get(getConditionNum(condition));            

                if (elements != null)
                {
                    info[row][col] = condition; 
                    ++col;
                    info[row][col] = getConditionValue(condition);                    
                    ++col;

                    if (elements.size() > 1)
                    {
                        int count = 0;

                        for (int i = 0; i < elements.size(); i++)
                        {
                            // place 2 elements and then jump to next line

                            if (count == 0)
                                info[row][2] = elements.get(i) + " ";
                            else
                                info[row][2] += elements.get(i) + " ";

                            count++;

                            if (count == 2)
                            {
                                row++;
                                count = 0;
                            }
                        }
                    }
                    else if (elements.size() == 1)
                        info[row][2] = (String)elements.get(0);

                    col = 0;
                    ++row;
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
        
        // Disable auto resizing
        
        table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
    
        // Set the first visible column to 100 pixels wide

        table.getColumnModel().getColumn(0).setPreferredWidth(200);
        table.getColumnModel().getColumn(1).setPreferredWidth(100);
        table.getColumnModel().getColumn(2).setPreferredWidth(600);
        
        table.setPreferredScrollableViewportSize(new Dimension(900, 300));

        table.setShowGrid(false);

        table.setGridColor(Color.GRAY);

        //Create the scroll pane and add the table to it.
        JScrollPane scrollPane = new JScrollPane(table);

        //Add the scroll pane to this panel.
        add(scrollPane);
    }
    
    private String getConditionValue(String condition)
    {
        if (model.getProperty(condition) == null)
            status = UNKNOWN;
        else if (model.getProperty(condition).equals("0"))
            status = FALSE;
        else if (model.getProperty(condition).equals("1"))
            status = TRUE;
        else if (model.getProperty(condition).equals("inf"))
            status = "inf";
        else
            status = model.getProperty(condition);
        
        return status;
    }
    
    public ConditionsWindow getCurrentConditionsTab()
    {
        return conditionsPane;
    }

    public void createConditionTab(Model model) 
    {       
        conditionsPane = new ConditionsWindow(model);
        conditionsPane.setOpaque(true);
    }
}
