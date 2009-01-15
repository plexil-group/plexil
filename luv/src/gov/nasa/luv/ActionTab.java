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

public class ActionTab extends JPanel 
{
    
    private Model model;
    private ActionTab actionPane;
    private int rows = 1000;
    private int columns = 1;
    private String info[][];
    private JTable table;   
    
    private ArrayList<String> actionList;
    
    public ActionTab() {}
    
    public ActionTab(Model model) 
    {       
        super(new GridLayout(1,0));
        
        this.model = model;

        String[] columnNames = {"Expression",
        };
        
        int row = 0;
        int col = 0;
        info = new String[rows][columns];
        
        actionList = model.getActionList();
        
        for (String action : actionList)
        {
            if (action != null)
            {
                info[row][col] = action; 
                ++row;
            }
        }
        
        table = new JTable(info, columnNames);
        
        // Disable auto resizing
        
        table.setAutoResizeMode(JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS);
    
        table.getColumnModel().getColumn(0).setPreferredWidth(900);
        
        table.setPreferredScrollableViewportSize(new Dimension(900, 300));

        table.setShowGrid(false);

        table.setGridColor(Color.GRAY);

        //Create the scroll pane and add the table to it.
        JScrollPane scrollPane = new JScrollPane(table);

        //Add the scroll pane to this panel.
        add(scrollPane);
    }
    
    public ActionTab getCurrentActionTab()
    {
        return actionPane;
    }

    public void createActionTab(Model model) 
    {       
        actionPane = new ActionTab(model);
        actionPane.setOpaque(true);
    }
}
