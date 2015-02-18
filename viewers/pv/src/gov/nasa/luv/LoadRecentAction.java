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

import java.awt.event.ActionEvent;
import static java.awt.event.KeyEvent.*;
import java.io.IOException;
import static gov.nasa.luv.Constants.*;

/**
 * The LoadRecentAction class provides methods for loading a recently run Plexil 
 * Plan amongst a saved list of plans. 
 */

public class LoadRecentAction extends LuvAction
{
    // file to switch to open when action is performed
    private int recentIndex;
    public static final int RECENT_BASE = 1, RECENT_DIR = 2;

    /**
     * Constructs a LoadRecentAction with the specified index of the plan
     * the user wishes to load.
     * 
     * @param recentIndex index of the plan to load
     * @param keyCode if user uses a keycode to load plan
     * @param modifiers if the user uses a modifier to load the plan
     */
    public LoadRecentAction(int recentIndex, int keyCode, int modifiers)
    {
       super(getRecentPlan(recentIndex) + " + " + getRecentScript(recentIndex),
             getRecentMenuDescription(recentIndex),
             keyCode, 
             modifiers);
       this.recentIndex = recentIndex;
    }
    
    /**
     * Gets the Plexil Plan base or directory.
     * 
     * @param parameter indicates base or directory
     * @return the Plexil Plan Constant
     */
    public static String defineRecentPlan(int parm)
    {
    	String str = "";
    	switch(Luv.getLuv().getExecSelect().getMode())
    	{
    	case PLEXIL_EXEC:
    		if(parm == RECENT_BASE)
    			str = PROP_FILE_EXEC_RECENT_PLAN_BASE;
    		else
    			str = PROP_FILE_EXEC_RECENT_PLAN_DIR;
    		break;
		case PLEXIL_TEST:
			if(parm == RECENT_BASE)
    			str = PROP_FILE_TEST_RECENT_PLAN_BASE;
    		else
    			str = PROP_FILE_TEST_RECENT_PLAN_DIR;
			break;
		case PLEXIL_SIM:
			if(parm == RECENT_BASE)
    			str = PROP_FILE_SIM_RECENT_PLAN_BASE;
    		else
    			str = PROP_FILE_SIM_RECENT_PLAN_DIR;
			break;	
    	}    	
    	return str;
    }
    
    /**
     * Gets the Plexil Supplement base or directory.
     * 
     * @param parameter indicates base or directory
     * @return the Plexil Supplement Constant
     */
    public static String defineRecentSupp(int parm)
    {
    	String str = "";
    	switch(Luv.getLuv().getExecSelect().getMode())
    	{
    	case PLEXIL_EXEC:
    		if(parm == RECENT_BASE)
    			str = PROP_FILE_EXEC_RECENT_CONFIG_BASE;
    		else
    			str = PROP_FILE_EXEC_RECENT_CONFIG_DIR;
    		break;
		case PLEXIL_TEST:
			if(parm == RECENT_BASE)
    			str = PROP_FILE_TEST_RECENT_SCRIPT_BASE;
    		else
    			str = PROP_FILE_TEST_RECENT_SCRIPT_DIR;
			break;
		case PLEXIL_SIM:
			if(parm == RECENT_BASE)
    			str = PROP_FILE_SIM_RECENT_SCRIPT_BASE;
    		else
    			str = PROP_FILE_SIM_RECENT_SCRIPT_DIR;
			break;	
    	}    	
    	return str;
    }
    
    /**
     * Gets the Plexil Lib base or directory.
     * 
     * @param parameter indicates base or directory
     * @return the Plexil Lib Constant
     */
    public static String defineRecentLib(int parm)
    {
    	String str = "";
    	switch(Luv.getLuv().getExecSelect().getMode())
    	{
    	case PLEXIL_EXEC:
    		if(parm == RECENT_BASE)
    			str = PROP_FILE_EXEC_RECENT_LIB_BASE;
    		else
    			str = PROP_FILE_EXEC_RECENT_LIB_DIR;
    		break;
		case PLEXIL_TEST:
			if(parm == RECENT_BASE)
    			str = PROP_FILE_TEST_RECENT_LIB_BASE;
    		else
    			str = PROP_FILE_TEST_RECENT_LIB_DIR;
			break;
		case PLEXIL_SIM:
			if(parm == RECENT_BASE)
    			str = PROP_FILE_SIM_RECENT_LIB_BASE;
    		else
    			str = PROP_FILE_SIM_RECENT_LIB_DIR;
			break;	
    	}    	
    	return str;
    }    
    
    /**
     * Gets the Plexil Plan with the specified index.
     * 
     * @param index indicates which Plexil Plan the user wants to load
     * @return the Plexil Plan to load
     */
    public static String getRecentPlan(int index)
    {
        String recentPlan = UNKNOWN;
        
        if (Luv.getLuv().getProperties().getProperty(defineRecentPlan(RECENT_BASE) + index) != null)
            recentPlan = Luv.getLuv().getProperties().getProperty(defineRecentPlan(RECENT_BASE) + index);  
        
        return recentPlan;
    }
    
    /**
     * Gets the Plexil Script with the specified index.
     * 
     * @param index indicates which Plexil Script the user wants to load
     * @return the Plexil Script to load
     */
    public static String getRecentScript(int index)
    {
        String recentScript = UNKNOWN;
        
        if (Luv.getLuv().getProperties().getProperty(defineRecentSupp(RECENT_BASE) + index) != null)
            recentScript = Luv.getLuv().getProperties().getProperty(defineRecentSupp(RECENT_BASE) + index);  
        
        return recentScript;
    }
    
    /**
     * Gets the Plexil Library with the specified index.
     * 
     * @param index indicates which Plexil Library the user wants to load
     * @return the Plexil Library to load
     */
    public static String getRecentLib(int index)
    {
        String recentLib = UNKNOWN;
        
        if (Luv.getLuv().getProperties().getProperty(defineRecentLib(RECENT_DIR) + index) != null)
        	recentLib = Luv.getLuv().getProperties().getProperty(defineRecentLib(RECENT_DIR) + index);  
        
        return recentLib;
    }
    
    // given a recent plan index, the description used for the recent menu item tooltip
    private static String getRecentMenuDescription(int index)
    {
        String plan = Luv.getLuv().getProperties().getProperty(defineRecentPlan(RECENT_BASE) + index);
        String script = Luv.getLuv().getProperties().getProperty(defineRecentSupp(RECENT_BASE) + index);
	
	String description = "Load " + plan + " + " + script;

	return description;
    }
      
    /**
     * Adds a file to the recent loaded Plexil Plan list. 
     */
    public static void addRunToRecentRunList()
    {
	// put newest file at the top of the list   
        String planName = Luv.getLuv().getCurrentPlan().getAbsolutePlanName();
        String scriptName = Luv.getLuv().getCurrentPlan().getAbsoluteScriptName();
        String libDirectory = Luv.getLuv().getProperties().getProperty(defineRecentLib(RECENT_DIR));      
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
                    planName = (String)Luv.getLuv().getProperties().setProperty(defineRecentPlan(RECENT_BASE) + i, planName);
                    scriptName = (String)Luv.getLuv().getProperties().setProperty(defineRecentSupp(RECENT_BASE) + i, scriptName);
                    if(libDirectory == null)
                    	libDirectory = "";
                    libDirectory = (String)Luv.getLuv().getProperties().setProperty(defineRecentLib(RECENT_DIR) + i, libDirectory);
                    
                    // if this run already existed in the list, we can stop
                    if (planName != null && planName.equals(currPlan) &&
                        scriptName != null && scriptName.equals(currScript))
                        break;
                }
            }

            updateRecentMenu();
        }
    }
      
    /**
     * Updates the recently loaded file menu.
     */
    public static void updateRecentMenu()
    {
	Luv.getLuv().getRecentRunMenu().removeAll();
	int count = Luv.getLuv().getProperties().getInteger(PROP_FILE_RECENT_COUNT);
        
	if (getRecentPlan(1) == null ||
            getRecentPlan(1).equals(UNKNOWN)) 
        {
	    Luv.getLuv().getRecentRunMenu().add("No recent runs");
	}
	else 
        {
	    for (int i = 0; i < count; ++i)
            {
		if ( getRecentPlan(i + 1) != null && !getRecentPlan(i + 1).equals(UNKNOWN))
                {
		    Luv.getLuv().getRecentRunMenu().add(new LoadRecentAction(i + 1, '1' + i, META_MASK));
                }
            }
	}

	// this menu is only enabled when there are items in it
	Luv.getLuv().getRecentRunMenu().setEnabled(Luv.getLuv().getRecentRunMenu().getMenuComponentCount() > 0);
    }

    /**
     * Loads the selected Plexil Plan from the recently loaded Plexil Plan list.
     * @param e
     */
    public void actionPerformed(ActionEvent e)
    {
        Luv.getLuv().setNewPlan(true);
        
        if (Luv.getLuv().getIsExecuting())
        {
           try 
           {
               Luv.getLuv().stopExecutionState();
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
           
            Luv.getLuv().loadRecentRunState();
        } 
        catch (IOException ex) 
        {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while loading recent plan");
        } 
        
        Luv.getLuv().setNewPlan(false);        
    }
}
