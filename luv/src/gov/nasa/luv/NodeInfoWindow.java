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
import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JComponent;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridLayout;
import static gov.nasa.luv.Constants.*;

public class NodeInfoWindow extends JPanel
{
    private JTabbedPane tabbedPane;
    private JFrame frame;
            
    public NodeInfoWindow(){}
    
    public NodeInfoWindow(Model node) 
    {
        super(new GridLayout(1, 1));
        
        tabbedPane = new JTabbedPane();

        addConditionsTab(node);
        addVariablesTab(node);
        addNodeTypeTab(node, node.getProperty(NODETYPE_ATTR));
        
        add(tabbedPane);
        
        // enables scrolling tabs
        tabbedPane.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
    }
    
    public JTabbedPane getCurrentNodeInfoWindow()
    {
        return tabbedPane;
    }
    
    public boolean isNodeInfoWindowOpen()
    {
        if (frame != null)
            return frame.isVisible();
        
        return false;
    }
    
    public void closeNodeInfoWindow()
    {
        if (frame != null)
            frame.setVisible(false);
    }
    
    private void addConditionsTab(Model node)
    {
        if (node.hasConditions())
        {
           JComponent panel = Luv.getLuv().getConditionsTab().getCurrentConditionsTab();
           panel.setPreferredSize(new Dimension(900, 300));
           tabbedPane.addTab("Conditions", null , panel, "Displays node conditions");
        }  
    }
    
    private void addVariablesTab(Model node)
    {
        if (node.hasVariables())
        {
           JComponent panel = Luv.getLuv().getVariablesTab().getCurrentVariablesTab();
           panel.setPreferredSize(new Dimension(900, 300));
           tabbedPane.addTab("Variables", null , panel, "Displays node local variables");
        }
    }
        
    private void addNodeTypeTab(Model node, String nodeType)
    {
        if (node.hasAction())
        {
           JComponent panel = Luv.getLuv().getActionTab().getCurrentActionTab();
           panel.setPreferredSize(new Dimension(900, 300));
           tabbedPane.addTab(nodeType, null , panel, "Displays action node expression");
        }
    }
    
    public void open(Model node) 
    {
        frame = new JFrame(node.getModelName() + " Information Window");
        
        frame.add(new NodeInfoWindow(node), BorderLayout.CENTER);
        frame.setSize(Luv.getLuv().getProperties().getDimension(PROP_NODEINFOWIN_SIZE));
        frame.setLocation(Luv.getLuv().getProperties().getPoint(PROP_NODEINFOWIN_LOC));
        
        frame.pack();
        frame.setVisible(true);
    }
}


