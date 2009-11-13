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

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Color;
import java.util.ArrayList;
import static gov.nasa.luv.Constants.*;

/** 
 * The ActionTab class provides methods for displaying Plexil Model Actions, 
 * such as, Assignment, Command, FunctionCall, LibraryCall or Update. 
 */

public class ActionTab extends JPanel 
{    
    private static ActionTab actionPane;
    
    /** 
     * Constructs an ActionTab with the specified Plexil Model.
     *
     * @param model Plexil Model on which the ActionTab represents
     */
       
    public ActionTab(Model model) 
    {       
        super(new GridLayout(1,0));
        
        // for now there is only one column since the UE does not transmit a
        // resulting value to LUV yet.
        String[] columnNames = {"Expression",};       
        String[][] info = new String[1000][1];  
        
        ArrayList<String> actionList = model.getActionList();     
        int row = 0;
        for (String action : actionList)
        {
            if (action != null)
            {
                info[row][0] = action; 
                ++row;
            }
        }
        
        JTable table = new JTable(info, columnNames);
        table.setAutoResizeMode(JTable.AUTO_RESIZE_SUBSEQUENT_COLUMNS);
        table.getColumnModel().getColumn(0).setPreferredWidth(900);
        table.setPreferredScrollableViewportSize(new Dimension(900, 300));
        table.setShowGrid(false);
        table.setGridColor(Color.GRAY);
        JScrollPane scrollPane = new JScrollPane(table);

        add(scrollPane);
        setOpaque(true);
    }
    
    /** Rewrites the action information into standard Plexil syntax for
     *  better user readability.   
     *
     * @param expression action information before it has been rewritten
     */
     
    public static String formatAction(String expression)
    {
        String formattedExpression = "COULD NOT IDENTIFY ACTION";
  
        if (expression != null && expression.contains(SEPARATOR))
        {
            String array[] = expression.split(SEPARATOR);      

            if (array.length > 0)
            {
                formattedExpression = "";
                    
                for (int i = 0; i < array.length; i++)
                {
                    formattedExpression += array[i] + " ";
                }                    
            }
        }
        
        return formattedExpression;
    }
    
    /** 
     * Returns the current instance of the ActionTab. 
     *
     * @return the current instance of the ActionTab
     */
    
    public static ActionTab getCurrentActionTab()
    {
        return actionPane;
    }
    
    /** 
     * Creates an instance of an ActionTab with the specified Plexil Model. 
     *
     * @param model the model on which to create an ActionTab
     */

    public static void open(Model model) 
    {       
        actionPane = new ActionTab(model);
    }
}
