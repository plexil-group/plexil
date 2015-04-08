/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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
import java.awt.Container;
import java.awt.BorderLayout;
import static gov.nasa.luv.Constants.*;
import static java.awt.BorderLayout.*;

/**
 * The ViewHandler class provides methods to set and refresh the TreeTableView of the 
 * Plexil Model in the Luv application.
 */

public class ViewHandler
{   
    private TreeTableView currentView;   // current view
    private Model currentModel; // plan for current view
    private JPanel viewPanel;   // Panel in which different views are placed
    
    /**
     * Constructs a ViewHandler.
     */
    public ViewHandler()
    {
        currentView = null;
        currentModel = null;
        viewPanel = new JPanel();
    }

    /**
     * Returns the current TreeTableView of the Plexil Model.
     * 
     * @return the current TreeTableView of the Plexil Model
     */
    public TreeTableView getCurrentView()
    {
	return currentView;
    }

    /**
     * Returns the current Plexil Model.
     * 
     * @return the current Plexil Model
     */
    public Model getCurrentModel()
    {
	return currentModel;
    }

    /**
     * Returns the JPanel holding the TreeTableView of the Plexil Model.
     * 
     * @return the JPanel holding the TreeTableView of the Plexil Model
     */
    public JPanel getViewPanel()
    {
	return viewPanel;
    }

    /**
     * Clears the current view of the Plexil Model.
     */
    public void clearCurrentView()
    {
        currentModel = null;
        currentView = null;
    }

    /**
     * Sets the current view to the specified Container.
     * 
     * @param view the specified Container
     */
    private void setView(Container view) {
        currentView = ((TreeTableView)view);

        // clear out the view panel and put the new view in there
        viewPanel.removeAll();
        viewPanel.setLayout(new BorderLayout());
        JScrollPane sp = new JScrollPane(view);
        sp.setBackground(Luv.getLuv().getSettings().getColor(PROP_WIN_BCLR));
        viewPanel.add(sp, CENTER);

        Luv.getLuv().setLocation(Luv.getLuv().getLocation());
        Luv.getLuv().setPreferredSize(Luv.getLuv().getSize());

        Luv.getLuv().pack();
        Luv.getLuv().repaint();
    }
    
    /**
     * Refreshes the current view of the Plexil Model.
     */
    public void refreshView()
    {
        if (currentView != null)
            ((Container)currentView).repaint();
    }
    
    /**
     * Displays the specified Plexil Model in the viewer.
     * 
     * @param model the Plexil Model
     */
    public void showModelInViewer(Model model) {
        // create a new instance of the view only if necessary
        if (model == null || model == currentModel) 
            return; 
        currentModel = model;
        setView(new TreeTableView(model));
        Luv.getLuv().getLuvBreakPointHandler().mapBreakPointsToNewModel(currentModel);
    }
    
    /**
     * Refreshes the view of the Plexil Model when new filters are added.
     */
    public void refreshRegexView()
    {
        currentModel = Luv.getLuv().getCurrentPlan();
        setView(new TreeTableView(currentModel));
        Luv.getLuv().getLuvBreakPointHandler().mapBreakPointsToNewModel(currentModel);
    }
}
