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
import javax.swing.JFrame;

import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Color;

import java.util.Vector;

import static gov.nasa.luv.Constants.*;

public class ConditionsWindow extends JPanel 
{
    
    Model model;
    static String status;
    static JFrame frame;
    static ConditionsWindow conditionsPane;
    int rows = 0;
    int columns = 3;
    String info[][];
    JTable table;
    
    Vector<Model.ChangeListener> modelListeners = new Vector<Model.ChangeListener>();
    
    public ConditionsWindow(Model model) 
    {       
        super(new GridLayout(1,0));
        
        this.model = model;

        String[] columnNames = {"Conditions",
                                "Value",
                                "Expression"};
        
        rows = model.conditionMap.size();
        int row = 0;
        int col = 0;
        info = new String[rows][columns];
        
        for (final String condition: ALL_CONDITIONS)
        {
            final String element = getConditionElements(condition);
            status = UNKNOWN;
            
            if (!element.equals(UNKNOWN))
            {
                info[row][col] = condition; 
                ++col;
                info[row][col] = status;
                ++col;
                info[row][col] = element;
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
                               if (!element.equals(UNKNOWN))
                               {
                                   status = model.getProperty(condition);
                                   if (status.equals("0"))
                                       status = FALSE;
                                   else if (status.equals("1"))
                                       status = TRUE;
                                   else if (status.equals("inf"))
                                       status = FALSE;
                                   
                                   for (int i = 0; i < rows; i++)
                                   {
                                       if (condition.equals(info[i][0]))
                                       {
                                           info[i][1] = status;
                                           table.setValueAt(status, i, 1);
                                           table.repaint();
                                           break;
                                       }
                                   }
                               }
                            }
                         }
                   });
        }
        
        table = new JTable(info, columnNames);
        
        // Disable auto resizing
        
        table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
    
        // Set the first visible column to 100 pixels wide

        table.getColumnModel().getColumn(0).setPreferredWidth(200);
        table.getColumnModel().getColumn(1).setPreferredWidth(100);
        table.getColumnModel().getColumn(2).setPreferredWidth(1000);
        
        table.setPreferredScrollableViewportSize(new Dimension(1300, 300));

        table.setShowGrid(false);

        table.setGridColor(Color.GRAY);

        //Create the scroll pane and add the table to it.
        JScrollPane scrollPane = new JScrollPane(table);

        //Add the scroll pane to this panel.
        add(scrollPane);
    }
    
    public static ConditionsWindow getCurrentWindow()
    {
        return conditionsPane;
    }
    
    public String getConditionElements(String condition)
    {
        int conditionNum = getConditionNum(condition);
        
        if (model.conditionMap.get(conditionNum) != null)
            return model.conditionMap.get(conditionNum);
        
        return UNKNOWN;
    }
    
    public static boolean isConditionsWindowOpen()
    {
        if (frame != null)
            return frame.isVisible();
        else 
            return false;
    }
    
    public static void closeConditonsWindow()
    {
        frame.setVisible(false);
    }

    /**
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from the
     * event-dispatching thread.
     */
    public static void createAndShowGUI(Model model, String nodeName) 
    {       
        //Create and set up the window.
        if (frame != null)
            frame.setVisible(false);
        frame = new JFrame(nodeName);

        //Create and set up the content pane.
        conditionsPane = new ConditionsWindow(model);
        conditionsPane.setOpaque(true); //content panes must be opaque
        frame.setContentPane(conditionsPane);
        frame.setBounds(20, 20, 1200, 500);

        //Display the window.
        frame.pack();
        frame.setVisible(true);
    }
    
    public static void resetGUI(Model model, String nodeName)
    {
        frame.setTitle(nodeName);
        
        status = INACTIVE;
        
        conditionsPane = new ConditionsWindow(model);
        conditionsPane.setOpaque(true); //content panes must be opaque
        frame.setContentPane(conditionsPane);

        //Display the window.
        frame.pack();
        frame.setVisible(true);
    }
}
