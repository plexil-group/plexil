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

/** 
 * Teh NodeInfoWindow class holds the ConditionsTab, VariablesTab and ActionTab 
 * Plexil Model might contain. 
 */

public class NodeInfoWindow extends JPanel
{
    private static  JTabbedPane tabbedPane;
    private static JFrame frame;
            
    public NodeInfoWindow(){}
    
    /** 
     * Constructs a NodeInfoWindow with the specified Plexil Model. 
     *
     * @param model the Plexil Model on which the NodeInfoWindow displays information
     */
    
    public NodeInfoWindow(Model model) 
    {
        super(new GridLayout(1, 1));
        
        tabbedPane = new JTabbedPane();

        addConditionsTab(model);
        addVariablesTab(model);
        addActionTab(model, model.getProperty(NODETYPE_ATTR));
        
        add(tabbedPane);

        tabbedPane.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
    }
    
    /** 
     * Returns whether the current instance of the NodeInfoWindow is open.
     *
     * @return whether the current instance of the NodeInfoWindow is open
     */
    
    public static boolean isNodeInfoWindowOpen()
    {
        if (frame != null)
            return frame.isVisible();
        
        return false;
    }
    
    /** 
     * Closes the current instance of the NodeInfoWindow.
     */
    
    public static void closeNodeInfoWindow()
    {
        if (frame != null)
            frame.setVisible(false);
    }
    
    /** 
     * Adds a ConditionsTab to this NodeInfoWindow for the specified Plexil Model. 
     *
     * @param model the Plexil Model on which the NodeInfoWindow gathers condition information
     */
    
    private void addConditionsTab(Model model)
    {
        if (model.hasConditions())
        {
           JComponent panel = ConditionsTab.getCurrentConditionsTab();
           panel.setPreferredSize(new Dimension(900, 300));
           tabbedPane.addTab("Conditions", null , panel, "Displays node conditions");
        }  
    }
    
    /** 
     * Adds a VariablesTab to this NodeInfoWindow for the specified Plexil Model. 
     *
     * @param model the Plexil Model on which the NodeInfoWindow gathers local variable information
     */
    
    private void addVariablesTab(Model model)
    {
        if (model.hasVariables())
        {
           JComponent panel = VariablesTab.getCurrentVariablesTab();
           panel.setPreferredSize(new Dimension(900, 300));
           tabbedPane.addTab("Variables", null , panel, "Displays node local variables");
        }
    }
    
    /** 
     * Adds a ActionTab to this NodeInfoWindow for the specified Plexil Model and action type. 
     *
     * @param model the Plexil Model on which the NodeInfoWindow gathers action information
     * @param actioType the type of action this Plexil Model represents
     */
        
    private void addActionTab(Model model, String actionType)
    {
        if (model.hasAction())
        {
           JComponent panel = ActionTab.getCurrentActionTab();
           panel.setPreferredSize(new Dimension(900, 300));
           tabbedPane.addTab(actionType, null , panel, "Displays action node expression");
        }
    }
    
    /** 
     * Creates an instance of an NodeInfoWindow for the specified Plexil Model. 
     *
     * @param model the model on which to create a NodeInfoWindow
     */
    
    public static void open(Model model) 
    {
        frame = new JFrame(model.getModelName() + " Information Window");
        
        frame.add(new NodeInfoWindow(model), BorderLayout.CENTER);
        frame.setSize(Luv.getLuv().getProperties().getDimension(PROP_NODEINFOWIN_SIZE));
        frame.setLocation(Luv.getLuv().getProperties().getPoint(PROP_NODEINFOWIN_LOC));
        
        frame.pack();
        frame.setVisible(true);
    }
}


