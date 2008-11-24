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


import javax.swing.JTabbedPane;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JComponent;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridLayout;

import static java.awt.BorderLayout.*;

public class NodeInfoTabbedWindow extends JPanel
{
    private JTabbedPane tabbedPane;
    private JFrame frame;
            
    public NodeInfoTabbedWindow(){}
    
    public NodeInfoTabbedWindow(Model node, String nodeName) 
    {
        super(new GridLayout(1, 1));
        
        tabbedPane = new JTabbedPane();
        
        JComponent panel1;
        
        if (node.getConditionMap() != null && !node.getConditionMap().isEmpty())
           panel1 = Luv.getLuv().getConditionsWindow().getCurrentConditionsTab(); 
        else
           panel1 = makeTextPanel("No Conditions for this Node");
        
        panel1.setPreferredSize(new Dimension(900, 300));
        tabbedPane.addTab("Conditions", null , panel1, "Displays Node Conditions");

        JComponent panel2;
        
        if (node.getVariableList() != null && !node.getVariableList().isEmpty())
           panel2 = Luv.getLuv().getVariablesWindow().getCurrentVariablesTab();
        else
           panel2 = makeTextPanel("No Variables for this Node");
        
        panel2.setPreferredSize(new Dimension(900, 300));
        tabbedPane.addTab("Variables", null, panel2, "Displays Node Variables");
        
        //Add the tabbed pane to this panel.
        add(tabbedPane);
        
        //The following line enables to use scrolling tabs.
        tabbedPane.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
    }
    
    protected JComponent makeTextPanel(String text) 
    {
        JPanel panel = new JPanel(false);
        JLabel filler = new JLabel(text);
        filler.setHorizontalAlignment(JLabel.CENTER);
        panel.setLayout(new GridLayout(1, 1));
        panel.add(filler);
        return panel;
    }
    
    public JTabbedPane getCurrentNodeInfoTabbedWindow()
    {
        return tabbedPane;
    }
    
    public boolean isNodeInfoTabbedWindowOpen()
    {
        if (frame != null)
            return frame.isVisible();
        else 
            return false;
    }
    
    public void closeNodeInfoTabbedWindow()
    {
        frame.setVisible(false);
    }
    
    /**
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from
     * the event dispatch thread.
     */
    public void createAndShowGUI(Model node, String nodeName) 
    {
        //Create and set up the window.
        frame = new JFrame(nodeName + " Information Window");
        
        //Add content to the window.
        frame.add(new NodeInfoTabbedWindow(node, nodeName), BorderLayout.CENTER);
        
        //Display the window.
        frame.pack();
        frame.setVisible(true);
    }
}


