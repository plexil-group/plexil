/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package gov.nasa.luv;

import java.awt.Container;
import java.awt.event.ActionEvent;


/** Action to select different views. */

public class SelectViewAction extends LuvAction
{
    /** View to switch to when this action is performed. */

    Container view;

    /** Construct a view action.
     *
     * @param view    view selected when this action is peformed
     * @param keyCode identifies shortcut key for this action
     */

    public SelectViewAction(Container view, int keyCode)
    {
       super(view.toString(), "Select " + view + " view", keyCode);
       this.view = view;
    }

    /**
     * Called when user wishes to make visible this type of view.
     *
     * @param  e action event 
     */

    public void actionPerformed(ActionEvent e)
    {
       Luv.luvViewerHandler.setView(view);
    }
}
