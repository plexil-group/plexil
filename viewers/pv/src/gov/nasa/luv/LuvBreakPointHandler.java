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

import java.awt.Color;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import static gov.nasa.luv.Constants.*;

/**
 * The LuvBreakPointHandler class provides methods to handle breakpoints set
 * on a Plexil Plan loaded in the Luv application.
 */

public class LuvBreakPointHandler 
{    

    // if break has occured, the causal break point object     
    private LuvBreakPoint activeBreakPoint;
      
    // collection of all breakpoints
    private HashMap<LuvBreakPoint, Integer> breakPointMap;  
      
    /**
     * Constructs a LuvBreakPointHandler.
     */
    public LuvBreakPointHandler()
    {
        activeBreakPoint = null;
        breakPointMap = new HashMap<LuvBreakPoint, Integer>();
    }
      
    /**
     * Returns whether or not LuvBreakPoints exist in the current Plexil Plan.
     * @return whether or not LuvBreakPoints exist in the current Plexil Plan
     */
    public boolean breakpointsExist()
    {
        return !breakPointMap.isEmpty();
    }
      
    /**
     * Returns the LuvBreakPoint.
     * @return the LuvBreakPoint
     */
    public LuvBreakPoint getActiveBreakPoint()
    {
        return activeBreakPoint;
    }

    /**
     * Sets the specified LuvBreakPoint in the current Plexil Plan.
     * @param bp the LuvBreakPoint to be set
     */
    public void setActiveBreakPoint(LuvBreakPoint bp)
    {
        activeBreakPoint = bp;
    }

    /**
     * Clears the LuvBreakPoint.
     */
    public void clearActiveBreakPoint()
    {
        activeBreakPoint = null;
    }
      
    /**
     * Returns the set of all LuvBreakPoints.
     */
    public Set<LuvBreakPoint> getBreakPointSet()
    {
        return breakPointMap.keySet();
    }

    /**
     * Adds the specified LuvBreakPoint to the full list of LuvBreakPoints.
     * @param breakPoint the LuvBreakPoint to be added
     */
    public void addBreakPoint(LuvBreakPoint breakPoint) {
        breakPointMap.put(breakPoint,
                          breakPoint.getNode().getRowNumber());
        Luv.getLuv().enableRemoveBreaksMenuItem(true);
        Luv.getLuv().getStatusMessageHandler().showStatus("Added break on " 
                                                          + breakPoint, 5000l);
        Luv.getLuv().getViewHandler().refreshView();       
    }

    /**
     * Removes the specified LuvBreakPoint from the full list of LuvBreakPoints.
     * @param breakPoint
     */
    public void removeBreakPoint(LuvBreakPoint breakPoint)
    {
        breakPointMap.remove(breakPoint);
        Luv.getLuv().getStatusMessageHandler().showStatus("Removed break on " 
                                                          + breakPoint, 5000l);
        Luv.getLuv().getViewHandler().refreshView();
    }

    /**
     * Removes all the LuvBreakPoints.
     */
    public void removeAllBreakPoints()
    {
        Luv.getLuv().enableRemoveBreaksMenuItem(false);
        activeBreakPoint = null;
        for (LuvBreakPoint bp: breakPointMap.keySet())
            bp.getNode().deleteBreakPoint(bp);
        breakPointMap.clear();
        Luv.getLuv().getViewHandler().refreshView();
    }
      
    /**
     * Maps all the current LuvBreakPoints to the new specified Plexil node.
     * @param node the new Plexil node the current LuvBreakPoints will be mapped to
     */
    public void mapBreakPointsToNewModel(Model model) {
        // *** TEMP DEBUG ***
        System.out.println("mapBreakPointsToNewModel called");

	    for (Map.Entry<LuvBreakPoint, Integer> pair: breakPointMap.entrySet()) {
            LuvBreakPoint bp = pair.getKey();
            int row_number = pair.getValue();

            Node target = model.findChildByRowNumber(row_number);
            if (target != null)
                bp.setNode(target);
        }
    }
}
