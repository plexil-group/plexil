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

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStreamReader;
import javax.swing.*;
import static gov.nasa.luv.Constants.*;


/**
 * The SourceWindow class is an interface for the user to view source file content.
 */
public class SourceWindow extends JFrame 
{
    private static SourceWindow frame;
    private JPanel planPanel;
    private JPanel scriptPanel;
    private int rows;
    private String planlines[][];
    private String scriptlines[][];
    private boolean loaded = false;

    public SourceWindow() {
    }

    /** Construct an SourceWindow.
     *
     * @param model the model source for this SourceWindow
     */
    public SourceWindow(Model model) throws FileNotFoundException {
        super("Source Window");
        
        if(model != null)
        {
        	if(loadPlanSource(model.getAbsolutePlanName()))
        	{
        		getContentPane().add(planPanel, BorderLayout.WEST);
        		loaded = true;
        	}
        	if(loadScriptSource(model.getAbsoluteScriptName()))
		        {   
			        getContentPane().add(new JSeparator(SwingConstants.VERTICAL));
			        getContentPane().add(scriptPanel, BorderLayout.EAST);
			        loaded = true;
        		}
        }
    }

    private boolean loadPlanSource(String filename) {
    	boolean success = false;
        planPanel = new JPanel(new GridLayout(1,0));
        rows = 10000;
        planlines = new String[rows][1];
        String[] columnNames = null;
        if(filename != null && !filename.equals("UNKNOWN"))
        {
	        columnNames = filename.split("  ");
	
	        try  {
	            FileInputStream fstream = new FileInputStream(filename);
	            DataInputStream in = new DataInputStream(fstream);
	            BufferedReader br = new BufferedReader(new InputStreamReader(in));
	            String line;
	            int row = 0;
	
	            while ((line = br.readLine()) != null && row < rows)
	            {
	                planlines[row][0] = line;
	                row++;
	            }
	
	            in.close();
	        } catch (Exception e) {
	            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while opening plan");
	        }        

        JTable source = new JTable(planlines, columnNames);        
        source.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);        
        source.getColumnModel().getColumn(0).setPreferredWidth(1000);
        source.setShowGrid(false);
        source.setGridColor(Color.GRAY);
        JScrollPane scrollPane = new JScrollPane(source);        
        scrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);        
        
        planPanel.add(scrollPane);
        planPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));        
        source.setOpaque(true);
        success = true;
        }
        return success;
    }
    
    private boolean loadScriptSource(String filename) {
    	boolean success = false;
        scriptPanel = new JPanel(new GridLayout(1,0));
        rows = 10000;
        scriptlines = new String[rows][1];
        String[] columnNames = null;
        if(filename != null && !filename.equals("UNKNOWN"))
        {
	        columnNames = filename.split("  ");
	
	        try  {
	            FileInputStream fstream = new FileInputStream(filename);
	            DataInputStream in = new DataInputStream(fstream);
	            BufferedReader br = new BufferedReader(new InputStreamReader(in));
	            String line;
	            int row = 0;
	
	            while ((line = br.readLine()) != null && row < rows)
	            {
	                scriptlines[row][0] = line;
	                row++;
	            }
	
	            in.close();
	        } catch (Exception e) {
	            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(e, "ERROR: exception occurred while opening script");
	        }        

        JTable source = new JTable(scriptlines, columnNames);
        source.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
        source.getColumnModel().getColumn(0).setPreferredWidth(1000);
        source.setShowGrid(false);
        source.setGridColor(Color.GRAY);
        JScrollPane scrollPane = new JScrollPane(source);
        scrollPane.setAutoscrolls(true);
        scrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);

        scriptPanel.add(scrollPane);
        scriptPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        source.setOpaque(true);
        success = true;
        }
        return success;
    }

    /**
     * Creates an instance of an SourceWindow for the specified Plexil Model.
     */
    public void open(Model model) throws FileNotFoundException {
        if (frame != null && frame.isVisible()) {
            frame.setVisible(false);
        }

        frame = new SourceWindow(model);   
        frame.setLocation(Luv.getLuv().getProperties().getPoint(PROP_CFGWIN_LOC));
        frame.pack();
        
        if(frame.loaded)
        	frame.setVisible(true);
        else
        	Luv.getLuv().getStatusMessageHandler().showStatus("No Data avaliable", Color.RED, 10000);
    }
    
    /**
     * Creates an instance of an SourceWindow for the current Plexil Model.
     */
    public void refresh() throws FileNotFoundException {
    	if(frame != null && frame.isVisible())
    	{
    	frame.setVisible(false);    	
    	frame = new SourceWindow(Luv.getLuv().getCurrentPlan());
    	frame.setLocation(Luv.getLuv().getProperties().getPoint(PROP_CFGWIN_LOC));
    	frame.pack();
    	if(frame.loaded)
        	frame.setVisible(true);
        else
        	Luv.getLuv().getStatusMessageHandler().showStatus("No Data avaliable", Color.RED, 10000);
    	}
    }
}
