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

import static gov.nasa.luv.Constants.FILE_EXTENSIONS;
import static gov.nasa.luv.Constants.PLEXIL_EXEC;
import static gov.nasa.luv.Constants.PLEXIL_TEST;
import static gov.nasa.luv.Constants.PLEXIL_SIM;
import static java.awt.event.InputEvent.META_MASK;
import static java.awt.event.KeyEvent.VK_E;
import static java.awt.event.KeyEvent.VK_O;

import java.awt.Color;
import java.awt.Component;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.filechooser.FileFilter;

import java.util.ArrayList;
import java.util.HashMap;

public class ExecSelect extends JPanel {
	/**
	 * 
	 */	
	private JFrame frame;
	private static ExecSelect exec;	
	private JButton planBut, configBut, scriptBut, saveBut, cancelBut, defaultScriptBut, defaultConfigBut, libBut;
	private JRadioButton plexilExec, plexilTest, plexilSim;
	private JLabel planLab, configLab, scriptLab;	
	private ArrayList<JButton> buttonList;
	private HashMap<JButton, Integer> recentDirMap;
	private HashMap<JButton, JLabel> objMap;
	private HashMap<JButton, PlexilFilter> extMap;
	private JPanel patternPanel;
	GridBagLayout gridbag;
	private GridBagConstraints c;
	private PlexilFilter planFilter, configFilter, scriptFilter;
	private JFileChooser dirChooser;
	private int mode, modeTemp;
	private AppSettings execSet, testSet, simSet, backExec, backTest, backSim;
	private static final int RECENT_LIB = 1, RECENT_PLAN = 2, RECENT_SUPP = 3;
		
	public JButton getSaveBut() {
		return saveBut;
	}
	
	public JButton getDefaultScriptBut() {
		return defaultScriptBut;
	}
	
	public int getMode() {
		return mode;
	}

	public ExecSelect() {
		init();			
		constructFrame();
		add(patternPanel);				
		setBorder(BorderFactory.createEmptyBorder(20, 40, 20, 40));		
		frame = new JFrame("Select Configuration");
		
	}
	
	private void init(){
		exec = this;
		objMap = new HashMap<JButton, JLabel>();		
		buttonList = new ArrayList<JButton>();
		recentDirMap = new HashMap<JButton, Integer>();
		extMap = new HashMap<JButton, PlexilFilter>();		
		execSet = new AppSettings("plexilExec", "config");
		execSet.setFilter(new PlexilFilter(""));
		testSet = new AppSettings("testExec", "script");
		testSet.setFilter(new PlexilFilter("PSX"));
		simSet = new AppSettings("simExec", "script");
		simSet.setFilter(new PlexilFilter("xml / plx / psx / txt / pst / pls"));
		backExec = new AppSettings("plexilExec", "config");
		backTest = new AppSettings("testExec", "script");
		backSim = new AppSettings("simExec","script");
		
		ButtonGroup execGroup = new ButtonGroup(); 
		plexilExec = new JRadioButton("PlexilExec");
		plexilExec.setToolTipText("Execute a plan on one or more external systems or executives");
		execGroup.add(plexilExec);
		plexilExec.addActionListener(new ExecChooserListener());		
		plexilTest = new JRadioButton("PlexilTest");
		plexilTest.setToolTipText("Simulate Plan Execution using a Test Executive Script");
		execGroup.add(plexilTest);
		setRadioMode(PLEXIL_TEST);
		plexilTest.addActionListener(new ExecChooserListener());		
		plexilSim = new JRadioButton("PlexilSim");
		plexilSim.setToolTipText("Execute a plan using Plexil Simulator");
		execGroup.add(plexilSim);
		plexilSim.addActionListener(new ExecChooserListener());		
		
		planBut =
            new JButton(new LuvAction("Plan",
                                      "Choose a plexil plan file.",
                                      VK_O,
                                      META_MASK) {
                    public void actionPerformed(ActionEvent e) {
                        openFile(e);
                    }
                }
                        );
		planBut.addActionListener(new ButtonFileListener());
		planLab = new JLabel("");
		libBut = new JButton("Libraries");
		libBut.addActionListener(Luv.getLuv().getLibraryLoader().getSelectLibraryListener());

		configBut =
            new JButton(new LuvAction("Config",
                                      "Choose a Config file.",
                                      VK_E,
                                      META_MASK) {

                    public void actionPerformed(ActionEvent e) {
                        openFile(e);
                    }
                }
                        );
		configBut.addActionListener(new ButtonFileListener());
		configBut.setEnabled(false);
		configLab = new JLabel("");
		configBut.setVisible(false);

		scriptBut =
            new JButton(new LuvAction("Script",
                                      "Choose a script file.",
                                      VK_E,
                                      META_MASK) {

                    public void actionPerformed(ActionEvent e) {
                        openFile(e);                    
                    }
                }
                        );
		scriptBut.addActionListener(new ButtonFileListener());
		scriptLab = new JLabel("");	
		
		saveBut = new JButton("OK");
		saveBut.addActionListener(new SaveButtonListener());
		cancelBut = new JButton("Cancel");
		cancelBut.addActionListener(new ButtonListener());
		defaultScriptBut = new JButton("Use Default");
		defaultScriptBut.addActionListener(new ButtonListener(defaultScriptBut));
		defaultConfigBut = new JButton("Use Default");
		defaultConfigBut.addActionListener(new ButtonListener(defaultConfigBut));
		defaultConfigBut.setVisible(false);
		
		patternPanel = new JPanel();
		
		dirChooser = new JFileChooser(Constants.PLEXIL_HOME)
		{
		    {
			setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		    }
		};		
		planFilter = new PlexilFilter("PLX");
		configFilter = new PlexilFilter("XML");
		scriptFilter = testSet.getFilter();						
	}
	
	/*
     * Defines recent plan and supplements from viewer preference hidden file
     */
	public void loadFromPersistence() {
		String planName = "", suppName = "", libName = "";
		AppSettings m_app = null;
		int currMode = mode;		 		
		for(int i=1;i<4;i++)
		{
			mode = i;
			planName = LoadRecentAction.getRecentPlan(1) == Constants.UNKNOWN ? "" : LoadRecentAction.getRecentPlan(1);
			 suppName = LoadRecentAction.getRecentScript(1) == Constants.UNKNOWN ? "" : LoadRecentAction.getRecentScript(1);			 
			 switch (i) {
			 case PLEXIL_EXEC: m_app = execSet; break;
			 case PLEXIL_TEST: m_app = testSet; break;
			 case PLEXIL_SIM: m_app = simSet; break;			 
			 }
			 for(int j=0;j<Constants.PROP_FILE_RECENT_COUNT_DEF;j++)
			 {
				 libName = LoadRecentAction.getRecentLib(j) == Constants.UNKNOWN ? "" : LoadRecentAction.getRecentLib(j);
				 m_app.getLibArray().add(libName);
			 }
			setInputs(m_app, planName, suppName);	
		}		 		
		mode = currMode;
	}
	
	/*
	 * Helper function for persistence loading
	 */
	private void setInputs(AppSettings app, String plan, String supp)
	{
		app.setPlanLocation(plan);
		app.setSuppLocation(supp);					
	}
	
	/*
	 * Exposes current configuration for viewer
	 */
	public static ExecSelect getExecSel() {
		return exec;
	}

	public static void setExecSel(ExecSelect exec) {
		ExecSelect.exec = exec;
	}

	private void constructFrame(){
		gridbag = new GridBagLayout();
		patternPanel.setLayout(gridbag);
		c = new GridBagConstraints();		
		c.fill = GridBagConstraints.HORIZONTAL;
		int row = 0;
		
		setGridbag(plexilExec, 0, row, 1, 1, 1, 1);
		setGridbag(plexilTest, 1, row, 1, 1, 1, 1);
		setGridbag(plexilSim, 2, row, 1, 1, 1, 1);
		
		row++;//1
		setGridbag(planBut, 0, row, 1, 1, 1, 1);
		setGridbag(planLab, 1, row, 1, 5, 1, 1);
		setupButton(planBut, planLab, planFilter, RECENT_PLAN);
		
		row++;//2
		setGridbag(libBut, 0, row, 1, 1, 1, 1);
		setupButton(libBut, planLab, planFilter, RECENT_LIB);		
			
		row++;//3
		setGridbag(configBut, 0, row, 1, 1, 1, 1);
		setGridbag(configLab, 1, row, 1, 5, 1, 1);		
		setGridbag(defaultConfigBut, 2, row, 1, 10, 1, 1);		
		setupButton(configBut, configLab, configFilter, RECENT_SUPP);
				
		row++;//4
		setGridbag(scriptBut, 0, row, 1, 1, 1, 1);
		setGridbag(scriptLab, 1, row, 1, 5, 1, 1);
		setGridbag(defaultScriptBut, 2, row, 1, 10, 1, 1);
		setupButton(scriptBut, scriptLab, scriptFilter, RECENT_SUPP);						
		
		row+=3;//6
		//setGridbag(defaultBut, 0, row, 10, 10, 1, 10);
		setGridbag(cancelBut, 1, row, 10, 10, 1, 10);		
		setGridbag(saveBut, 2, row, 10, 10, 1, 10);
	}
	
	/*
     * Extended Gridbag Configure component method
     * @param x and y are position in panel. top, left, bot, right refer to spacing around object
     */
	private void setGridbag(Component comp, int x, int y, int top, int left, int bot, int right)
	{
		c.insets = new Insets(top , left , bot , right);
		setGridbag(comp, x, y);
	}
	
	/*
     * Gridbag Configure component method adjusting weight and attaching to main component
     * @param x and y are position in panel
     */
	private void setGridbag(Component comp, int x, int y)
	{
		c.weightx = 0.5;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = x;
		c.gridy = y;
		gridbag.setConstraints(comp, c);
		patternPanel.add(comp);		
	}		
	
	/*
     * Extended Button Configure method
     * @param lab refers to text field next button, filt is file filter associated with button,
     * recent refers to plan or supplement constants
     */
	private void setupButton(JButton button, JLabel lab, PlexilFilter filt , int recent){
		setupButton(button, lab, filt);		
		recentDirMap.put(button, recent);
	}
	
	/*
     * Extended Button Configure method
     * @param lab refers to text field next button, filt is file filter associated with button
     */
	private void setupButton(JButton button, JLabel lab, PlexilFilter filt){		
		setupButton(button, lab);		
		extMap.put(button, filt);
	}
	
	/*
     * Button Configure method
     * @param lab refers to text field next button
     */
	private void setupButton(JButton button, JLabel lab){
		buttonList.add(button);
		objMap.put(button, lab);		
	}		
	
	public JFrame getFrame()
	{
		return frame;
	}
	
	/*
	 * Used to clear text fields for applicable mode
	 */
	private void rememberBlank(){
		switch (mode) {
		 case PLEXIL_EXEC: getSettings().initializeSuppLabel(configLab);
		 getSettings().setSuppToolTip(configLab);
		 break;
		 case PLEXIL_TEST: getSettings().initializeSuppLabel(scriptLab);
		 getSettings().setSuppToolTip(scriptLab);
		 break;
		 case PLEXIL_SIM: getSettings().initializeSuppLabel(scriptLab);
		 getSettings().setSuppToolTip(scriptLab);
		 break;			 
		 }
	}
	
	public void loadExecSelect() {
		refresh();
	}
	
	/*
	 * Used to refresh runtime state while configuring plans and supplements
	 * refreshes labels and buttons
	 */
	private void refresh() {
		String plannm = getSettings().getPlanLocation();
		String suppnm = getSettings().getSuppLocation();
		File plan = null, script = null, config = null;             
        
        if(suppnm.equals(""))
        	rememberBlank();
        
        if(plannm == null || plannm.equals(""))
        	return;
        
        plan = new File(plannm);
        if (plan.exists()) {        	
            getSettings().initializePlanLabel(planLab);
            getSettings().setPlanToolTip(planLab);
        } else {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: trying to reload an UNKNOWN plan");
        }
        
        if(suppnm == null || suppnm.equals(""))
        	return;
        
        if(mode == PLEXIL_EXEC)
        {	            
            config = new File(suppnm);
            if (config.exists()) {                     
                getSettings().initializeSuppLabel(configLab);
                getSettings().setSuppToolTip(configLab);                
            }
        } else
        {            
           	script = new File(suppnm);
            if (script.exists()) {
                getSettings().initializeSuppLabel(scriptLab);
                getSettings().setSuppToolTip(scriptLab);                
            }
        }
	}
	
	/*
	 * exposed method for reloading plan and supplement set from file menu
	 */
	public void reload(){		
		refresh();
		String plannm = getSettings().getPlanLocation();
		String suppnm = getSettings().getSuppLocation();
		File plan = null, script = null, config = null;
		Luv.getLuv().setNewPlan(true);
        PlexilPlanHandler.resetRowNumber();

        if (Luv.getLuv().getIsExecuting()) {
            try {
                Luv.getLuv().stopExecutionState();
                Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Stopping execution and reloading plan");
            } catch (IOException ex) {
                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while reloading plan");
            }
        }        
                
        if(plannm == null || plannm.equals(""))
        	return;
        
        plan = new File(plannm);
        if (plan.exists()) {
            Luv.getLuv().loadPlan(plan);
        } else {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: trying to reload an UNKNOWN plan");
            Luv.getLuv().reloadPlanState();
        }
        Luv.getLuv().setNewPlan(false);                
        
        if(suppnm == null || suppnm.equals(""))
        	return;
        
        if(mode == PLEXIL_EXEC)
        {	            
            config = new File(suppnm);
            if (config.exists()) {
                Luv.getLuv().getFileHandler().loadScript(config);
                Luv.getLuv().getStatusMessageHandler().showStatus("Config \"" + Luv.getLuv().getCurrentPlan().getAbsoluteScriptName() + "\" loaded", 1000);                
            }
        } else
        {            
           	script = new File(suppnm);
            if (script.exists()) {
                Luv.getLuv().getFileHandler().loadScript(script);
                Luv.getLuv().getStatusMessageHandler().showStatus("Script \"" + Luv.getLuv().getCurrentPlan().getAbsoluteScriptName() + "\" loaded", 1000);                
            }
        }
        Luv.getLuv().setTitle();
		try{
			Luv.getLuv().getSourceWindow().refresh();
			
		} catch (IOException ex){
			Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while opening source window");
		}
        
	}
	
	class ButtonListener implements ActionListener {
		private JButton but;
		ButtonListener(){			
		}
		ButtonListener(JButton but){	
			this.but = but;
		}
		
		public void actionPerformed(ActionEvent e){		
			File script = null;
			if(e.getActionCommand().equals("Cancel"))
			{
				Luv.getLuv().getExecSelect().frame.setVisible(false);
				Luv.getLuv().getStatusMessageHandler().showStatus("Reverted Configuration");
				setInputs(execSet, backExec.getPlanLocation(), backExec.getSuppLocation());
				setInputs(testSet, backTest.getPlanLocation(), backTest.getSuppLocation());
				setInputs(simSet, backSim.getPlanLocation(), backSim.getSuppLocation());				
				setRadioMode(modeTemp);
			}
			if(e.getActionCommand().equals("Use Default") && but.equals(Luv.getLuv().getExecSelect().getDefaultScriptBut()))
			{								
					try{
					script = Luv.getLuv().getFileHandler().searchForScriptPath(getSettings().getPlanLocation().replaceAll(".*/",""));
					if(script != null)
					{
						getSettings().setSuppLocation(script.getAbsolutePath());						
					}
					else
					{
						getSettings().setSuppLocation(Constants.DEFAULT_CONFIG_PATH+Constants.DEFAULT_SCRIPT_NAME);
					}
					}catch (IOException ex)
					{ex.printStackTrace();}					
				refresh();
				Luv.getLuv().getStatusMessageHandler().showStatus("Default Script");
			}
			if(e.getActionCommand().equals("Use Default") && !but.equals(Luv.getLuv().getExecSelect().getDefaultScriptBut()))
			{				
				getSettings().setSuppLocation(Constants.DEFAULT_CONFIG_PATH+Constants.DEFAULT_CONFIG_NAME);
				refresh();
				Luv.getLuv().getStatusMessageHandler().showStatus("Default Config");
			}
		}
	}	
	
	/*
	 * Changes configuration mode
	 * @param Plexil mode constant
	 */
	private void setRadioMode(int mode){
		this.mode = mode;
		switch (mode){
			case PLEXIL_EXEC: plexilExec.setSelected(true);	break;
			case PLEXIL_TEST: plexilTest.setSelected(true);	break;
			case PLEXIL_SIM:  plexilSim.setSelected(true);	break;			
		}
	}
	
	/*
	 * application settings getter
	 * @return returns appropriate application settings based upon mode
	 */
	public AppSettings getSettings(){
		AppSettings app = null;
		switch (mode){
		case PLEXIL_EXEC: app = execSet; break;
		case PLEXIL_TEST: app = testSet; break;
		case PLEXIL_SIM: app = simSet; break;			
		}
		return app;		
	}
	
	/*
	 * Configures runtime absolute path names
	 * @param user selected file and relevant button
	 */
	private void matchButtonFunction(File file, JButton but){		
		if(file != null)
			objMap.get(but).setText(file.getName());		
		if(but == planBut)
		{
			getSettings().setPlanLocation(file.getAbsolutePath());
		}
		else if(but == configBut)
		{
			getSettings().setSuppLocation(file.getAbsolutePath());
		}
		else if(but == scriptBut)
		{
			getSettings().setSuppLocation(file.getAbsolutePath());
		}
	}
	
	/*
	 * function for recent file directories based upon file type
	 * @param Configuration constant type of recent file
	 * @return recent file directory based upon configuration constant
	 */
	private String lookupRecent(int parm){
		String str = "";
		switch(parm){
			case RECENT_PLAN: str = LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR); break;
			case RECENT_LIB: str = LoadRecentAction.defineRecentLib(LoadRecentAction.RECENT_DIR); break;
			case RECENT_SUPP: str = LoadRecentAction.defineRecentSupp(LoadRecentAction.RECENT_DIR); break;
		}
		return str;
	}
	
	/*
	 * universal open file method
	 * References hash maps to determine appropriate recent directory for user searches,
	 * file filters based upon application, and setting chosen files to text fields 
	 */
	private void openFile(ActionEvent e){
		final JFileChooser fc;
		PlexilFilter pf = null;
		String nameProp = "", recent = "";
		if(e.getSource() != null)
		{
			recent = lookupRecent(recentDirMap.get(e.getSource()));
			nameProp = Luv.getLuv().getProperties().getProperty(recent);
		}
		if(nameProp == null)
			nameProp = Luv.getLuv().getProperty(LoadRecentAction.defineRecentPlan(LoadRecentAction.RECENT_DIR));
		fc = new JFileChooser(new File(nameProp));
		if(e.getSource() == scriptBut)		
			pf = getSettings().getFilter();
		else
			pf = extMap.get(e.getSource());
		
		if(pf != null)
		{				
			fc.addChoosableFileFilter(pf);
		}				
		int returnVal = fc.showDialog(dirChooser, "Choose File");
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            File file = fc.getSelectedFile();
            Luv.getLuv().getStatusMessageHandler().showStatus("Selected " + file.getAbsolutePath());
            for(JButton but : buttonList)
            	if(but != null && e.getSource() == but)
            	{
            		matchButtonFunction(file, but);
            		//Luv.getLuv().getStatusMessageHandler().showStatus(file.getName());
            	}
        }
	}
	
	/*
	 * temporarily stores names into a buffer in case user decides to cancel settings
	 */
	public void backupNames(){
		setInputs(backExec, execSet.getPlanLocation(), execSet.getSuppLocation());
		setInputs(backTest, testSet.getPlanLocation(), testSet.getSuppLocation());
		setInputs(backSim, simSet.getPlanLocation(), simSet.getSuppLocation());
		modeTemp = mode;
	}

	class ButtonFileListener implements ActionListener {
		ButtonFileListener(){			
		}
		public void actionPerformed(ActionEvent e){  		                		            
		}
	}	
	
	class ExtensionButtonFileListener implements ActionListener {
		ExtensionButtonFileListener(){			
		}
		public void actionPerformed(ActionEvent e){
			openFile(e);						
		}
	}
	
	class SaveButtonListener implements ActionListener {
		SaveButtonListener(){			
		}
		public void actionPerformed(ActionEvent e){
			File plan = null, supp = null;
			if (Luv.getLuv().getIsExecuting()) {
	            try {
	                Luv.getLuv().stopExecutionState();
	                Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Stopping execution and loading plan");
	            } catch (IOException ex) {
	                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while reloading plan");
	            }
	        }
			if(!Luv.getLuv().getIsExecuting())
			{
				if(ExecSelect.getExecSel().getSettings().getPlanLocation() != "")
				{	
						Luv.getLuv().setNewPlan(true);
						PlexilPlanHandler.resetRowNumber();
						plan = new File(ExecSelect.getExecSel().getSettings().getPlanLocation());
						Luv.getLuv().loadPlan(plan);
						Luv.getLuv().readyState();
						Luv.getLuv().setNewPlan(false);
						Luv.getLuv().setProperty(lookupRecent(RECENT_PLAN), plan.getParent());
				}
				if(getSettings().getSuppLocation() != "")
				{
					supp = new File(getSettings().getSuppLocation());				
					if(mode == PLEXIL_EXEC)							
					{
						Luv.getLuv().getFileHandler().loadScript(supp);					
					}
					if(mode == PLEXIL_TEST)
					{
						Luv.getLuv().getFileHandler().loadScript(supp);
					}
					if(mode == PLEXIL_SIM)
					{
						Luv.getLuv().getFileHandler().loadScript(supp);
					}		
					Luv.getLuv().getStatusMessageHandler().showStatus(ExecSelect.getExecSel().getSettings().getSuppName() + " \"" + Luv.getLuv().getCurrentPlan().getAbsoluteScriptName() + "\" loaded", 1000);
					Luv.getLuv().setTitle();
					try{
						Luv.getLuv().getSourceWindow().refresh();
						
					} catch (IOException ex){
						Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while opening source window");
					}					
					Luv.getLuv().setProperty(lookupRecent(RECENT_SUPP), supp.getParent());
				}
			}		
			Luv.getLuv().getExecSelect().frame.setVisible(false);
		}
	}
	
	class ExecChooserListener implements ActionListener {
		ExecChooserListener(){			
		}            	
		public void actionPerformed(ActionEvent e){
			if(e.getActionCommand() == plexilExec.getText())
			{				
				if (plexilExec.isSelected()) {
					mode = PLEXIL_EXEC;
					Luv.getLuv().setAppMode(PLEXIL_EXEC);
					Luv.getLuv().getStatusMessageHandler().showStatus(
							"Use UniversalExec", Color.GREEN.darker(), 1000);						
					JButton[] on={planBut, configBut};
					handleButtons(on, true);
					handleVisibleButtons(on, true);
					defaultScriptBut.setVisible(false);
					defaultConfigBut.setVisible(true);
					JButton[] off={scriptBut};
					handleButtons(off, false);		
					handleVisibleButtons(off, false);
					refresh();
				}
			}
			else if(e.getActionCommand() == plexilTest.getText())
			{
				if (plexilTest.isSelected()) {
					mode = PLEXIL_TEST;
					Luv.getLuv().setAppMode(PLEXIL_TEST);
					Luv.getLuv().getStatusMessageHandler().showStatus(
							"Use TestExec", Color.GREEN.darker(), 1000);				
					JButton[] on={planBut, scriptBut};
					handleButtons(on, true);
					handleVisibleButtons(on, true);
					defaultScriptBut.setVisible(true);
					defaultConfigBut.setVisible(false);
					JButton[] off={configBut};
					handleButtons(off, false);
					handleVisibleButtons(off, false);
					refresh();
				}
			}
			else if(e.getActionCommand() == plexilSim.getText())
			{
				if (plexilSim.isSelected()) {
					mode = PLEXIL_SIM;
					Luv.getLuv().setAppMode(PLEXIL_SIM);
					Luv.getLuv().getStatusMessageHandler().showStatus(
							"Use PlexilSim", Color.GREEN.darker(), 1000);								
					JButton[] on={planBut, scriptBut};
					handleButtons(on, true);
					handleVisibleButtons(on, true);
					JButton[] off={configBut};
					handleButtons(off, false);
					defaultScriptBut.setVisible(false);
					defaultConfigBut.setVisible(false);
					handleVisibleButtons(off, false);
					refresh();
				}
			}
		}
		private void handleButtons(JButton[] buttons, boolean toggle){
			for(int i=0;i<buttons.length;i++)
			{
				buttons[i].setEnabled(toggle);
				if(toggle == false)
					objMap.get(buttons[i]).setText("");
			}
		}
		private void handleVisibleButtons(JButton[] buttons, boolean toggle){
			for(int i=0;i<buttons.length;i++)
			{
				buttons[i].setVisible(toggle);				
			}
		}
	}
	
	public class PlexilFilter extends FileFilter {
		
		private String descr;
		private String[] filter;
		
		PlexilFilter(String description){
			descr = description;
			filter = descr.split(" / ");
			if(filter.length<1 || filter[0].matches(""))
				filter = FILE_EXTENSIONS;
		}
		// accept file?                          
		public boolean accept(File f) 
		{
		    // allow browse directories                             
		    if (f.isDirectory())
			return true;
                          
		    // allow files with correct Extension                              
		    String extension = getExtension(f);
		    Boolean correctExtension = false;
		    if (extension != null)
                        {
                            for (String ext: filter)
                                if (extension.toLowerCase().equals(ext.toLowerCase()))
                                    correctExtension = true;
                        }
                          
		    return correctExtension;
		}

		// get file extension                        
		public String getExtension(File f)
		{
		    String ext = null;
		    String s = f.getName();
		    int i = s.lastIndexOf('.');
                          
		    if (i > 0 && i < s.length() - 1)
			ext = s.substring(i+1).toLowerCase();
                          
		    return ext;
		}

		// return Description                          
		public String getDescription()
		{
		    return descr;
		}
	}
	
	/*
	 * Class for configuration for each application 
	*/
	class AppSettings {
		private String name = "";
		private String suppName = "";		
		private String planLocation = "";
		private String suppLocation = "";	
		private PlexilFilter filter = null;
		private ArrayList<String> libArray = null; 
		AppSettings(String name, String supp){
			this.name = name;
			this.suppName = supp;
			libArray = new ArrayList<String>();
		}
		public String getName() {
			return name;
		}		
		public String getSuppName() {
			return suppName;
		}
		public ArrayList<String> getLibArray() {
			return libArray;
		}
		public void setSuppName(String suppName) {
			this.suppName = suppName;
		}
		public String getPlanLocation() {
			return planLocation;
		}
		public void setPlanLocation(String planLocation) {
			this.planLocation = planLocation;
		}
		public String getSuppLocation() {
			return suppLocation;
		}
		public void setSuppLocation(String suppLocation) {
			this.suppLocation = suppLocation;
		}
		public void initializePlanLabel(JLabel lab){
			lab.setText(planLocation.replaceAll(".*/",""));
		}
		public void initializeSuppLabel(JLabel lab){
			lab.setText(suppLocation.replaceAll(".*/",""));
		}
		public void setPlanToolTip(JComponent comp){
			comp.setToolTipText(planLocation);
		}
		public void setSuppToolTip(JComponent comp){
			comp.setToolTipText(suppLocation);
		}
		public PlexilFilter getFilter() {
			return filter;
		}
		public void setFilter(PlexilFilter filter) {
			this.filter = filter;
		}
	}
	
}
