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
import static java.awt.event.KeyEvent.*;
import java.io.IOException;

import static gov.nasa.luv.Constants.*;

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
       super(getRecentPlan(recentIndex) + " + " + getRecentScript(recentIndex),
             getRecentMenuDescription(recentIndex),
             keyCode, 
             modifiers);
       this.recentIndex = recentIndex;
    }
    
    public static String getRecentPlan(int index)
    {
        String recentPlan = UNKNOWN;
        
        if (Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_BASE + index) != null)
            recentPlan = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_BASE + index);  
        
        return recentPlan;
    }
    
    public static String getRecentScript(int index)
    {
        String recentScript = UNKNOWN;
        
        if (Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_SCRIPT_BASE + index) != null)
            recentScript = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_SCRIPT_BASE + index);  
        
        return recentScript;
    }
    
    /** Given a recent plan index, the description used for the recent menu item tooltip. 
     *
     * @param recentIndex the index of the recent plan
     *
     * @return the description of what gets loaded
     */

    private static String getRecentMenuDescription(int index)
    {
        String plan = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_PLAN_BASE + index);
        String script = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_SCRIPT_BASE + index);
	
	String description = "Load " + plan + " + " + script;

	return description;
    }
      
    // Add a file to the recently opened file list. 

    public static void addRunToRecentRunList()
    {
	// put newest file at the top of the list
        
        String planName = Luv.getLuv().getCurrentPlan().getPlanName();
        String scriptName = Luv.getLuv().getCurrentPlan().getScriptName();
        String libDirectory = Luv.getLuv().getProperties().getProperty(PROP_FILE_RECENT_LIB_DIR);
        
        String currPlan = planName;
        String currScript = scriptName;
        
        if (planName != null && scriptName != null &&
            !planName.equals(UNKNOWN) && !scriptName.equals(UNKNOWN))
        {
            int count = Luv.getLuv().getProperties().getInteger(PROP_FILE_RECENT_COUNT);

            for (int i = 1; i <= count && planName != null; ++i) 
            {
                if (planName != null) 
                {
                    planName = (String)Luv.getLuv().getProperties().setProperty(PROP_FILE_RECENT_PLAN_BASE + i, planName);
                    scriptName = (String)Luv.getLuv().getProperties().setProperty(PROP_FILE_RECENT_SCRIPT_BASE + i, scriptName);
                    libDirectory = (String)Luv.getLuv().getProperties().setProperty(PROP_FILE_RECENT_LIB_DIR + i, libDirectory);
                    
                    // if this run already existed in the list, we can stop

                    if (planName != null && planName.equals(currPlan) &&
                        scriptName != null && scriptName.equals(currScript))
                        break;
                }
            }

            // update the recent menu

            updateRecentMenu();
        }
    }
      
    /** Update the recently loaded files menu. */

    public static void updateRecentMenu()
    {
	Luv.getLuv().getRecentRunMenu().removeAll();
	int count = Luv.getLuv().getProperties().getInteger(PROP_FILE_RECENT_COUNT);
        
	if (getRecentPlan(1) == null ||
            getRecentPlan(1).equals(UNKNOWN)) {
	    Luv.getLuv().getRecentRunMenu().add("No recent runs");
	}
	else {
	    for (int i = 0; i < count; ++i)
		if ( getRecentPlan(i + 1) != null && !getRecentPlan(i + 1).equals(UNKNOWN))
                {
		    Luv.getLuv().getRecentRunMenu().add(new LoadRecentAction(i + 1, '1' + i, META_MASK));
                }
	}

	// this menu is only enabled when there are items in it
         
	Luv.getLuv().getRecentRunMenu().setEnabled(Luv.getLuv().getRecentRunMenu().getMenuComponentCount() > 0);
    }

    /**
     * Called when user wishes to make visible this type of file.
     *
     * @param  e action event 
     */

    public void actionPerformed(ActionEvent e)
    {
        Luv.getLuv().setNewPlan(true);
        
        if (Luv.getLuv().getIsExecuting())
        {
           try 
           {
               Luv.getLuv().getLuvStateHandler().stopExecutionState();
               Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Stopping execution and loading a recent plan");
           }
           catch (IOException ex) 
           {
               Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while loading recent plan");              
           }
        }
     
        try 
        {
            Luv.getLuv().getFileHandler().loadRecentRun(recentIndex);
           
            Luv.getLuv().getLuvStateHandler().loadRecentRunState();
        } 
        catch (IOException ex) 
        {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while loading recent plan");
        } 
        
        Luv.getLuv().setNewPlan(false);        
    }
}
