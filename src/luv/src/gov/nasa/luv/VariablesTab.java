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

import static java.awt.BorderLayout.SOUTH;
import gov.nasa.luv.Model.ChangeAdapter;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTree;
import javax.swing.border.EmptyBorder;

import treetable.JTreeTable;
import treetable.TreeTableModel;

/** 
 * The VariablesTab class provides methods for displaying a Plexil Model's local 
 * variable information. 
 */

public class VariablesTab extends JPanel 
{ 
    private static VariablesTab variablesPane;
    private Model model;   
    private int rows;
    private String info[][];
    private JTable table;   
    private VariableTreeTable treeTable;    
    private JButton flex;
    private static boolean flexState;
    
    public VariablesTab() {}
    
    /** 
     * Constructs a VariablesTab with the specified Plexil Model.
     *
     * @param model model on which the VariablesTab represents
     */
    
    public VariablesTab(Model model) 
    {       
        super(new BorderLayout());
        this.model = model;
        treeTable = new VariableTreeTable(model);///

        String[] columnNames = {"In/InOut",
                                "Name",
                                "Type",
                                "Value",
        };
        VariablesTab.flexState = false;
        
        JPanel buttonBar = new JPanel();
        add(buttonBar, BorderLayout.NORTH);
        GridBagLayout gridbag = new GridBagLayout();
        buttonBar.setLayout(gridbag);
        GridBagConstraints c = new GridBagConstraints();
                
        /*
        rows = 1000;        
        info = new String[rows][4];
        table = new JTable(info, columnNames);
        table.setAutoResizeMode(JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS);
        table.getColumnModel().getColumn(0).setPreferredWidth(50);
        table.getColumnModel().getColumn(1).setPreferredWidth(200);
        table.getColumnModel().getColumn(2).setPreferredWidth(100);
        table.getColumnModel().getColumn(3).setPreferredWidth(550);
        table.setPreferredScrollableViewportSize(new Dimension(900, 300));
        table.setShowGrid(false);
        table.setGridColor(Color.GRAY);
        */       
        //treeTable.setAutoResizeMode(JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS);
        treeTable.getColumnModel().getColumn(0).setPreferredWidth(50);
        treeTable.getColumnModel().getColumn(1).setPreferredWidth(200);
        treeTable.getColumnModel().getColumn(2).setPreferredWidth(100);
        treeTable.getColumnModel().getColumn(3).setPreferredWidth(550);
        //treeTable.setPreferredScrollableViewportSize(new Dimension(900, 300));
        //treeTable.setShowGrid(false);
        //treeTable.setGridColor(Color.GRAY);        
        ///
        /*
        refreshTable();
        
        JScrollPane scrollPane = new JScrollPane(table);
        add(scrollPane);
        model.addChangeListener(new ChangeAdapter() {
        	public void variableAssigned(Model model, String variableName) {
        		refreshTable();
        	}
        });
        setOpaque(true);
        */
        
        JScrollPane scrollPane = new JScrollPane(treeTable);
        add(scrollPane);
        treeTable.addNotify();
        flex = new JButton("Expand All");        
        flex.addActionListener(new ActionListener()
        {
        	public void actionPerformed(ActionEvent e) {
        		
        		if(VariablesTab.flexState)
        		{
        			treeTable.collapseAllNodes();
        			flex.setText("Expand All");	        	    
	        	    VariablesTab.flexState = false;
        		} else
        		{
        			treeTable.expandAllNodes();        	    
	        	    flex.setText("Collapse All");
	        	    VariablesTab.flexState = true;
        		}
        	  }
        });
                               
        c.fill = GridBagConstraints.HORIZONTAL;
        c.weightx = 0.02;
        gridbag.setConstraints(flex, c);
        buttonBar.add(flex);
        JLabel blank = new JLabel("");
        c.weightx = 1.0;
        gridbag.setConstraints(blank, c);
        buttonBar.add(blank);
        
    }  
    
    private void refreshTable() {
        int row = 0;
        int col = 0;
        for (Variable original : model.getVariableList())
        {
            if (original != null)
            {
                info[row][col] = original.getInOut(); 
                col++;
                info[row][col] = original.getName(); 
                col++;
                info[row][col] = original.getType(); 
                col++;
                info[row][col] = original.getValue(); 
                
                col = 0;
                ++row;
            }
        }
        table.addNotify();
    }
    
    /** 
     * Returns the current instance of the VariablesTab. 
     *
     * @return the current instance of the VariablesTab
     */
    
    public static VariablesTab getCurrentVariablesTab()
    {
        return variablesPane;
    }
    
    /** 
     * Creates an instance of a VariablesTab with the specified model. 
     *
     * @param model the model on which to create an VariablesTab
     */

    public static void open(Model model) 
    {       
        variablesPane = new VariablesTab(model);
    }        
    	    
}    