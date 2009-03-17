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

import java.awt.Color;
import java.io.IOException;
import java.util.Set;
import static gov.nasa.luv.Constants.*;

public class LuvStateHandler 
{
    public LuvStateHandler() {}
    
    public void startState()
    {
	Luv.getLuv().disableAllMenus();
          
	Luv.getLuv().setBreaksAllowed(false);
	Luv.getLuv().setIsExecuting(false);     
	Luv.getLuv().setIsPaused(false);
          
	Model.getRoot().clear();  
          
	Luv.getLuv().getViewHandler().clearCurrentView();
	Luv.getLuv().getStatusMessageHandler().clearStatusMessageQ();
        Luv.getLuv().getLuvBreakPointHandler().removeAllBreakPoints();
          
	// reset all menu items
                 
	Luv.getLuv().getFileMenu().getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);
        Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(EXIT_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().setEnabled(true);
          
	Luv.getLuv().updateBlockingMenuItems();
	LuvActionHandler.allowBreaksAction.putValue(NAME, "Enable Breaks");
        Luv.getLuv().getRunMenu().getItem(BREAK_MENU_ITEM).setEnabled(false);
        Luv.getLuv().getRunMenu().setEnabled(true);
        
        Luv.getLuv().getViewMenu().setEnabled(true);
        
        Luv.getLuv().getDebugMenu().setEnabled(true);
    }
      
    public void readyState()
    {
	// set only certain luv viewer variables
          
	Luv.getLuv().setIsPaused(false);
        Luv.getLuv().setIsStepped(false);
	Luv.getLuv().getFileHandler().setStopSearchForMissingLibs(false); 
        
        PlexilPlanHandler.resetRowNumber();
        
        Luv.getLuv().setTitle();
        
        Luv.getLuv().getLuvBreakPointHandler().clearBreakPoint();
        
	// set certain menu items
          
	LuvActionHandler.execAction.putValue(NAME, "Execute Plan");

	Luv.getLuv().getFileMenu().getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);          
	Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(EXIT_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().setEnabled(true);
 
	Luv.getLuv().updateBlockingMenuItems();
        
        if (Luv.getLuv().getIsExecuting())
            Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(false);
        else
            Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(true);
       
	Luv.getLuv().getRunMenu().setEnabled(true);

	if (Luv.getLuv().getViewMenu().getMenuComponentCount() > 0) {
	    Luv.getLuv().getViewMenu().getItem(EXPAND_MENU_ITEM).setEnabled(true);
	    Luv.getLuv().getViewMenu().getItem(COLLAPSE_MENU_ITEM).setEnabled(true);
	    Luv.getLuv().getViewMenu().getItem(HIDE_OR_SHOW_NODES_MENU_ITEM).setEnabled(true);
            Luv.getLuv().getViewMenu().getItem(FIND_MENU_ITEM).setEnabled(true);
	    Luv.getLuv().getViewMenu().setEnabled(true);
	}
	else
	    Luv.getLuv().getViewMenu().setEnabled(false);
        
        Luv.getLuv().getDebugMenu().setEnabled(true);
    }

    // Called when we receive EOF on the LuvListener stream. 
    public void finishedExecutionState()
    {  
        Luv.getLuv().setIsExecuting(false);
        Luv.getLuv().setIsPaused(false);
        Luv.getLuv().setIsStepped(false);	
        Luv.getLuv().getFileHandler().setStopSearchForMissingLibs(false);
  
	// set certain menu items
          
	LuvActionHandler.execAction.putValue(NAME, "Execute Plan");

	Luv.getLuv().getFileMenu().getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);          
	Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(EXIT_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().setEnabled(true);
 
	Luv.getLuv().updateBlockingMenuItems();  
        
        Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(true);      
        
	Luv.getLuv().getRunMenu().setEnabled(true);

	if (Luv.getLuv().getViewMenu().getMenuComponentCount() > 0)
        {
            Luv.getLuv().getViewMenu().getItem(EXPAND_MENU_ITEM).setEnabled(true);
            Luv.getLuv().getViewMenu().getItem(COLLAPSE_MENU_ITEM).setEnabled(true);
            Luv.getLuv().getViewMenu().getItem(HIDE_OR_SHOW_NODES_MENU_ITEM).setEnabled(true); 
            Luv.getLuv().getViewMenu().getItem(FIND_MENU_ITEM).setEnabled(true);
            Luv.getLuv().getViewMenu().setEnabled(true);
        }
	else
	    Luv.getLuv().getViewMenu().setEnabled(false);
        
        Luv.getLuv().getDebugMenu().setEnabled(true);

	Luv.getLuv().getStatusMessageHandler().showStatus("Execution stopped", Color.BLUE);
    }
      
    public void preExecutionState()
    {
        Luv.getLuv().setShouldHighlight(false);
        Luv.getLuv().getCurrentPlan().resetMainAttributesOfAllNodes();
        
	Luv.getLuv().getFileMenu().getItem(OPEN_PLAN_MENU_ITEM).setEnabled(false);
	Luv.getLuv().getFileMenu().getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(false);
	Luv.getLuv().getFileMenu().getItem(OPEN_RECENT_MENU_ITEM).setEnabled(false);
	Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(false);
	Luv.getLuv().getRunMenu().getItem(BREAK_MENU_ITEM).setEnabled(false);	
        Luv.getLuv().getRunMenu().getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(false);
        Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(false);
    }
      
    public void executionState()
    {
        Luv.getLuv().setIsExecuting(true);
          
	Luv.getLuv().getStatusMessageHandler().showStatus("Executing...", Color.GREEN.darker());
        
        LuvActionHandler.execAction.putValue(NAME, "Stop Execution");
        
        Luv.getLuv().getFileMenu().getItem(OPEN_PLAN_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(OPEN_SCRIPT_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(OPEN_RECENT_MENU_ITEM).setEnabled(true);
	Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(true);     
        
        Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(true);
        
        if (Luv.getLuv().breaksAllowed()) 
            enabledBreakingState();
        else
            disabledBreakingState();
          
	Luv.getLuv().updateBlockingMenuItems();
    }     
          
    public void stopExecutionState() throws IOException
    {
        Luv.getLuv().getExecutionHandler().killUEProcess();
        
	Luv.getLuv().setIsPaused(false);
        Luv.getLuv().setIsStepped(false);
	
    }
    
    public void openPlanState()
    {
        Luv.getLuv().getLuvBreakPointHandler().removeAllBreakPoints();
        
        Luv.getLuv().getCurrentPlan().resetMainAttributesOfAllNodes();
        
        Luv.getLuv().getCurrentPlan().addScriptName(UNKNOWN);

        Luv.getLuv().getNodeInfoWindow().closeNodeInfoWindow();                

        readyState();
    }
    
    public void loadRecentRunState()
    {
        Luv.getLuv().getLuvBreakPointHandler().removeAllBreakPoints();
        
        Luv.getLuv().getCurrentPlan().resetMainAttributesOfAllNodes();

        Luv.getLuv().getNodeInfoWindow().closeNodeInfoWindow();                

        readyState();
    }
    
    public void reloadPlanState()
    {
        Luv.getLuv().getCurrentPlan().resetMainAttributesOfAllNodes();

        readyState();
    }
    
    public void pausedState()
    {
        Luv.getLuv().setIsPaused(true);
        Luv.getLuv().setIsStepped(false);
          
	Luv.getLuv().updateBlockingMenuItems();
    }
      
    public void stepState()
    {
        Luv.getLuv().setIsPaused(false);
        Luv.getLuv().setIsStepped(true);
          
	Luv.getLuv().updateBlockingMenuItems();
    }

    //
    // Sub-states
    //
      
    public void disabledBreakingState()
    {
        Luv.getLuv().setBreaksAllowed(false);
        
	LuvActionHandler.allowBreaksAction.putValue(NAME, "Enable Breaks");
        
        Luv.getLuv().setForeground(lookupColor(MODEL_DISABLED_BREAKPOINTS));
        
        Set<LuvBreakPoint> breakPoints = Luv.getLuv().getLuvBreakPointHandler().getBreakPointMap().keySet();
        
        for (BreakPoint bp : breakPoints)
        {
            bp.setEnabled(Luv.getLuv().breaksAllowed());
        }
        
        Luv.getLuv().getViewHandler().refreshView();
        
	Luv.getLuv().updateBlockingMenuItems();
    }
      
    public void enabledBreakingState()
    {
	Luv.getLuv().setBreaksAllowed(true);
        
	LuvActionHandler.allowBreaksAction.putValue(NAME, "Disable Breaks");
        
        Luv.getLuv().setForeground(lookupColor(MODEL_ENABLED_BREAKPOINTS));
        
        Set<LuvBreakPoint> breakPoints = Luv.getLuv().getLuvBreakPointHandler().getBreakPointMap().keySet();
        
        for (LuvBreakPoint bp : breakPoints)
        {
            if (!bp.getReserveBreakStatus())
                bp.setEnabled(Luv.getLuv().breaksAllowed());
        }
        
        Luv.getLuv().getViewHandler().refreshView();
        
	Luv.getLuv().updateBlockingMenuItems();
    }
}
