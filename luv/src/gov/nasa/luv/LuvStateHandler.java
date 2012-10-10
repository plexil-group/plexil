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

import gov.nasa.luv.Luv;

import java.awt.Color;
import java.io.IOException;
import java.util.Set;
import static gov.nasa.luv.Constants.*;

/**
 * The LuvStateHandler class manages Luv states, basically, enabling or
 * disabling Luv menu items based on whether Luv just started, is executing, is
 * paused, etc.
 */

public class LuvStateHandler {
	public LuvStateHandler() {
	}

	/**
	 * Sets the Luv application to a Start State, this occurs as when the Luv
	 * application opens for the first time.
	 */
	public void startState() {
		disableAllMenus();
		Luv.getLuv().setBreaksAllowed(false);
		Luv.getLuv().setIsExecuting(false);
		Luv.getLuv().setIsPaused(false);

		Model.getRoot().clear();

		Luv.getLuv().getViewHandler().clearCurrentView();
		Luv.getLuv().getStatusMessageHandler().clearStatusMessageQ();
		Luv.getLuv().getLuvBreakPointHandler().removeAllBreakPoints();

		// reset all menu items

		Luv.getLuv().getExecSelect().getSaveBut().setEnabled(true);
		Luv.getLuv().getFileMenu().getItem(EXIT_MENU_ITEM).setEnabled(true);
		Luv.getLuv().getFileMenu().setEnabled(true);

		Luv.getLuv().updateBlockingMenuItems();
		LuvActionHandler.allowBreaksAction.putValue(NAME, "Enable Breaks");
		Luv.getLuv().getRunMenu().getItem(BREAK_MENU_ITEM).setEnabled(false);
		Luv.getLuv().getRunMenu().getItem(EXEC_SELECT_ITEM).setEnabled(true);
		Luv.getLuv().getRunMenu().setEnabled(true);

		Luv.getLuv().getViewMenu().setEnabled(true);

		Luv.getLuv().getDebugMenu().setEnabled(true);
	}

	/**
	 * Sets the Luv application to a Ready State.
	 */
	public void readyState() {
		// set only certain luv viewer variables

		Luv.getLuv().setIsPaused(false);
		Luv.getLuv().setIsStepped(false);
		Luv.getLuv().getFileHandler().setStopSearchForMissingLibs(false);

		PlexilPlanHandler.resetRowNumber();

		Luv.getLuv().setTitle();

		Luv.getLuv().getLuvBreakPointHandler().clearBreakPoint();

		// set certain menu items

		LuvActionHandler.execAction.putValue(NAME, "Execute Plan");

		Luv.getLuv().getExecSelect().getSaveBut().setEnabled(true);
		Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(true);
		Luv.getLuv().getFileMenu().getItem(EXIT_MENU_ITEM).setEnabled(true);
		Luv.getLuv().getFileMenu().setEnabled(true);

		Luv.getLuv().updateBlockingMenuItems();

		if (Luv.getLuv().getIsExecuting())
			Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(
					false);
		else
			Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(
					true);

		Luv.getLuv().getRunMenu().setEnabled(true);

		if (Luv.getLuv().getViewMenu().getMenuComponentCount() > 0) {		
			Luv.getLuv().getViewMenu().getItem(EXPAND_MENU_ITEM).setEnabled(
					true);
			Luv.getLuv().getViewMenu().getItem(COLLAPSE_MENU_ITEM).setEnabled(
					true);
			Luv.getLuv().getViewMenu().getItem(HIDE_OR_SHOW_NODES_MENU_ITEM)
					.setEnabled(true);
			Luv.getLuv().getViewMenu().getItem(FIND_MENU_ITEM).setEnabled(true);
			Luv.getLuv().getViewMenu().getItem(EPX_VIEW_MENU_ITEM).setEnabled(
					true);
			Luv.getLuv().getViewMenu().setEnabled(true);
		} else
			Luv.getLuv().getViewMenu().setEnabled(false);

		Luv.getLuv().getDebugMenu().setEnabled(true);
	}

	/**
	 * Sets the Luv application to a Finished Execution State and occurs when
	 * EOF on the LuvListener stream is received.
	 */
	public void finishedExecutionState() {
		Luv.getLuv().setIsExecuting(false);
		Luv.getLuv().setIsPaused(false);
		Luv.getLuv().setIsStepped(false);
		Luv.getLuv().getFileHandler().setStopSearchForMissingLibs(false);

		// set certain menu items

		LuvActionHandler.execAction.putValue(NAME, "Execute Plan");

		Luv.getLuv().getExecSelect().getSaveBut().setEnabled(true);
		Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(true);
		Luv.getLuv().getFileMenu().getItem(EXIT_MENU_ITEM).setEnabled(true);
		Luv.getLuv().getFileMenu().setEnabled(true);

		Luv.getLuv().updateBlockingMenuItems();

		Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(true);

		Luv.getLuv().getRunMenu().setEnabled(true);

		if (Luv.getLuv().getViewMenu().getMenuComponentCount() > 0) {			
			Luv.getLuv().getViewMenu().getItem(EXPAND_MENU_ITEM).setEnabled(
					true);
			Luv.getLuv().getViewMenu().getItem(COLLAPSE_MENU_ITEM).setEnabled(
					true);
			Luv.getLuv().getViewMenu().getItem(HIDE_OR_SHOW_NODES_MENU_ITEM)
					.setEnabled(true);
			Luv.getLuv().getViewMenu().getItem(FIND_MENU_ITEM).setEnabled(true);
			Luv.getLuv().getViewMenu().getItem(EPX_VIEW_MENU_ITEM).setEnabled(
					true);
			Luv.getLuv().getViewMenu().setEnabled(true);
		} else
			Luv.getLuv().getViewMenu().setEnabled(false);

		Luv.getLuv().getDebugMenu().setEnabled(true);

		Luv.getLuv().getStatusMessageHandler().showStatus("Execution stopped",
				Color.BLUE);
		Luv.getLuv().getStatusMessageHandler().showChangeOnPort("Listening on port " + Luv.getLuv().getPort());
	}

	/**
	 * Sets the Luv application to a Pre Execution State and occurs just before
	 * the loaded Plexil Plan is about to execute.
	 */
	public void preExecutionState() {
		Luv.getLuv().setShouldHighlight(false);
		Luv.getLuv().getCurrentPlan().resetMainAttributesOfAllNodes();

		Luv.getLuv().getExecSelect().getSaveBut().setEnabled(false);
		Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(false);
		Luv.getLuv().getRunMenu().getItem(BREAK_MENU_ITEM).setEnabled(false);
		Luv.getLuv().getRunMenu().getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(
				false);
		Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(false);
	}

	/**
	 * Sets the Luv application to an Execution State and occurs while the
	 * loaded Plexil Plan is executing.
	 */
	public void executionState() {
		Luv.getLuv().setIsExecuting(true);
		
		Luv.getLuv().getStatusMessageHandler().showIdlePortMessage();
		
		Luv.getLuv().getStatusMessageHandler().showStatus("Executing...",
				Color.GREEN.darker());

		LuvActionHandler.execAction.putValue(NAME, "Stop Execution");

		Luv.getLuv().getExecSelect().getSaveBut().setEnabled(true);
		Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(true);

		Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(true);

		if (Luv.getLuv().breaksAllowed())
			enabledBreakingState();
		else
			disabledBreakingState();

		Luv.getLuv().updateBlockingMenuItems();
	}

	/**
	 * Sets the Luv application to a Stopped Execution State and occurs when the
	 * user manually stops the execution of a Plexil Plan.
	 */
	public void stopExecutionState() throws IOException {
		Luv.getLuv().getExecutionHandler().killUEProcess();

		Luv.getLuv().setIsPaused(false);
		Luv.getLuv().setIsStepped(false);

	}

	/**
	 * Sets the Luv application to an Open Plan State and occurs when a new
	 * Plexil Plan is newly opened in the Luv application.
	 */
	public void openPlanState() {
		Luv.getLuv().getLuvBreakPointHandler().removeAllBreakPoints();

		Luv.getLuv().getCurrentPlan().resetMainAttributesOfAllNodes();

		Luv.getLuv().getCurrentPlan().addScriptName(UNKNOWN);

		NodeInfoWindow.closeNodeInfoWindow();

		readyState();
	}

	/**
	 * Sets the Luv application to an Load Recent Run State and occurs when a
	 * reccently loaded Plexil Plan is newly opened in the Luv application from
	 * the recently run menu.
	 */
	public void loadRecentRunState() {
		Luv.getLuv().getLuvBreakPointHandler().removeAllBreakPoints();

		Luv.getLuv().getCurrentPlan().resetMainAttributesOfAllNodes();

		NodeInfoWindow.closeNodeInfoWindow();

		readyState();
	}

	/**
	 * Sets the Luv application to an Reload Plan State and occurs when a
	 * currently loaded Plexil Plan is refreshed in the Luv application.
	 */
	public void reloadPlanState() {
		if (Luv.getLuv().getIsExecuting()
				&& Luv.getLuv().getExecutionHandler().isAlive())
			try {
				stopExecutionState();
			} catch (IOException e) {
				e.printStackTrace();
			}

		Luv.getLuv().getCurrentPlan().resetMainAttributesOfAllNodes();

		readyState();
	}

	/**
	 * Sets the Luv application to an Paused State and occurs when the Luv
	 * application has breaks enabled and is at the beginning of executing a
	 * Plexil Plan or the user manually pauses a currently running Plexil Plan.
	 */
	public void pausedState() {
		Luv.getLuv().setIsPaused(true);
		Luv.getLuv().setIsStepped(false);

		Luv.getLuv().updateBlockingMenuItems();
	}

	/**
	 * Sets the Luv application to an Step State and occurs when the Luv
	 * application has breaks enabled and the user manually steps through a
	 * currently running Plexil Plan.
	 */
	public void stepState() {
		Luv.getLuv().setIsPaused(false);
		Luv.getLuv().setIsStepped(true);

		Luv.getLuv().updateBlockingMenuItems();
	}

	//
	// Sub-states
	//

	/**
	 * Sets the Luv application to an Disabled Breaking State and occurs when
	 * the Luv application has breaks disabled.
	 */
	public void disabledBreakingState() {
		Luv.getLuv().setBreaksAllowed(false);

		LuvActionHandler.allowBreaksAction.putValue(NAME, "Enable Breaks");

		Luv.getLuv().setForeground(lookupColor(MODEL_DISABLED_BREAKPOINTS));

		Set<LuvBreakPoint> breakPoints = Luv.getLuv().getLuvBreakPointHandler()
				.getBreakPointMap().keySet();

		for (BreakPoint bp : breakPoints) {
			bp.setEnabled(Luv.getLuv().breaksAllowed());
		}

		Luv.getLuv().getViewHandler().refreshView();

		Luv.getLuv().updateBlockingMenuItems();
	}

	/**
	 * Sets the Luv application to an Enabled Breaking State and occurs when the
	 * Luv application has breaks enabled.
	 */
	public void enabledBreakingState() {
		Luv.getLuv().setBreaksAllowed(true);

		LuvActionHandler.allowBreaksAction.putValue(NAME, "Disable Breaks");

		Luv.getLuv().setForeground(lookupColor(MODEL_ENABLED_BREAKPOINTS));

		Set<LuvBreakPoint> breakPoints = Luv.getLuv().getLuvBreakPointHandler()
				.getBreakPointMap().keySet();

		for (LuvBreakPoint bp : breakPoints) {
			if (!bp.getReserveBreakStatus())
				bp.setEnabled(Luv.getLuv().breaksAllowed());
		}

		Luv.getLuv().getViewHandler().refreshView();

		Luv.getLuv().updateBlockingMenuItems();
	}

	private void disableAllMenus() {		
		Luv.getLuv().getExecSelect().getSaveBut().setEnabled(false);
		Luv.getLuv().getFileMenu().getItem(RELOAD_MENU_ITEM).setEnabled(false);
		Luv.getLuv().getFileMenu().getItem(EXIT_MENU_ITEM).setEnabled(false);
		Luv.getLuv().getFileMenu().setEnabled(false);

		Luv.getLuv().getRunMenu().getItem(PAUSE_RESUME_MENU_ITEM).setEnabled(
				false);
		Luv.getLuv().getRunMenu().getItem(STEP_MENU_ITEM).setEnabled(false);
		Luv.getLuv().getRunMenu().getItem(BREAK_MENU_ITEM).setEnabled(false);
		Luv.getLuv().getRunMenu().getItem(REMOVE_BREAKS_MENU_ITEM).setEnabled(
				false);		
		Luv.getLuv().getRunMenu().getItem(EXEC_SELECT_ITEM).setEnabled(false);
		Luv.getLuv().getRunMenu().getItem(EXECUTE_MENU_ITEM).setEnabled(false);
		Luv.getLuv().getRunMenu().setEnabled(false);

		if (Luv.getLuv().getViewMenu().getMenuComponentCount() > 0) {			
			Luv.getLuv().getViewMenu().getItem(EXPAND_MENU_ITEM).setEnabled(
					false);
			Luv.getLuv().getViewMenu().getItem(COLLAPSE_MENU_ITEM).setEnabled(
					false);
			Luv.getLuv().getViewMenu().getItem(HIDE_OR_SHOW_NODES_MENU_ITEM)
					.setEnabled(false);
			Luv.getLuv().getViewMenu().getItem(FIND_MENU_ITEM)
					.setEnabled(false);
			Luv.getLuv().getViewMenu().getItem(EPX_VIEW_MENU_ITEM).setEnabled(
					false);
		}
		Luv.getLuv().getViewMenu().setEnabled(false);

		Luv.getLuv().getDebugMenu().setEnabled(false);
	}
}
