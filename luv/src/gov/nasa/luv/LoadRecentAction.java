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

import java.awt.event.ActionEvent;

import java.io.IOException;
import javax.swing.JOptionPane;

/** Action to load a recent plan. */

public class LoadRecentAction extends LuvAction
{
    /** File to switch to open when action is performed. */

    int recentIndex;

    /** Construct a file action.
     *
     * @param recentIndex index of recent file to load
     * @param keyCode identifies shortcut key for this action
     * @param modifiers modifiers for shortcut (SHIFT, META, etc.)
     */

    public LoadRecentAction(int recentIndex, int keyCode, int modifiers)
    {
       super(Luv.getLuv().getRecentPlanName(recentIndex),
             Luv.getLuv().getRecentMenuDescription(recentIndex),
             keyCode, 
             modifiers);
       this.recentIndex = recentIndex;
    }

    /**
     * Called when user wishes to make visible this type of file.
     *
     * @param  e action event 
     */

    public void actionPerformed(ActionEvent e)
    {
       if (Luv.getLuv().getIsExecuting())
       {
           try 
           {
               Luv.getLuv().stopExecution();
               JOptionPane.showMessageDialog(Luv.getLuv(), "Stopping execution and loading a recent plan", "Stopping Execution", JOptionPane.INFORMATION_MESSAGE);
           }
           catch (IOException ex) 
           {
               JOptionPane.showMessageDialog(Luv.getLuv(), "Error loading recent plan. Please see Debug Window.", "Error", JOptionPane.ERROR_MESSAGE);
               System.err.println("Error: " + ex.getMessage());
           }
       }
     
       try 
       {
           if(TreeTableView.getCurrent() != null &&
              TreeTableView.getCurrent().isConditionWindowOpen())
                TreeTableView.getCurrent().closeConditionWindow();
           
           Luv.getLuv().getFileHandler().loadRecentPlan(recentIndex);
       } 
       catch (IOException ex) 
       {
          JOptionPane.showMessageDialog(Luv.getLuv(), "Error loading recent plan. Please see Debug Window.", "Error", JOptionPane.ERROR_MESSAGE);
          System.err.println("Error: " + ex.getMessage());
       }

       Luv.getLuv().readyState();        
    }
}
