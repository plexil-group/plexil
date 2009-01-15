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

import java.util.Stack;

import static gov.nasa.luv.Constants.*;

public class VariablesTab extends JPanel 
{
    
    private Model model;
    private VariablesTab variablesPane;
    private int rows = 1000;
    private int columns = 4;
    private String info[][];
    private JTable table;   
    
    private ArrayList<Stack<String>> variableList;
    
    public VariablesTab() {}
    
    public VariablesTab(Model model) 
    {       
        super(new GridLayout(1,0));
        
        this.model = model;

        String[] columnNames = {"In/InOut",
                                "Name",
                                "Type",
                                "Initial Value ONLY - (UE does not currently provide updated values to LUV)",
        };
        
        int row = 0;
        int col = 0;
        info = new String[rows][columns];
        
        variableList = model.getVariableList();
        
        for (Stack<String> original : variableList)
        {
            Stack<String> copy = new Stack<String>();
        
            Object[] obj = original.toArray();

            for (int i = 0; i < obj.length; i++)
            {
                copy.push((String) obj[i]);
            }

            if (copy != null)
            {
                String value;
                String type;
                String name;
                String in_inout;
                    
                if (copy.size() == 4)
                {
                    value = (String) copy.pop();
                    type = (String) copy.pop();
                    name = (String) copy.pop();
                    in_inout = (String) copy.pop(); 
                }
                else
                {
                    value = UNKNOWN;
                    type = UNKNOWN;
                    name = UNKNOWN;
                    in_inout = UNKNOWN;
                }
                
                info[row][col] = in_inout; 
                col++;
                info[row][col] = name; 
                col++;
                info[row][col] = type; 
                col++;
                info[row][col] = value; 
                
                col = 0;
                ++row;
            }
        }
        
        table = new JTable(info, columnNames);
        
        // Disable auto resizing
        
        table.setAutoResizeMode(JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS);
    
        // Set the first visible column to 100 pixels wide

        table.getColumnModel().getColumn(0).setPreferredWidth(50);
        table.getColumnModel().getColumn(1).setPreferredWidth(200);
        table.getColumnModel().getColumn(2).setPreferredWidth(100);
        table.getColumnModel().getColumn(3).setPreferredWidth(550);
        
        table.setPreferredScrollableViewportSize(new Dimension(900, 300));

        table.setShowGrid(false);

        table.setGridColor(Color.GRAY);

        //Create the scroll pane and add the table to it.
        JScrollPane scrollPane = new JScrollPane(table);

        //Add the scroll pane to this panel.
        add(scrollPane);
    }
    
    public VariablesTab getCurrentVariablesTab()
    {
        return variablesPane;
    }

    public void createVariableTab(Model model) 
    {       
        variablesPane = new VariablesTab(model);
        variablesPane.setOpaque(true);
    }
}
