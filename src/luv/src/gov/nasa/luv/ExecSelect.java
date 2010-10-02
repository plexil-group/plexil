package gov.nasa.luv;

import static gov.nasa.luv.Constants.FILE_EXTENSIONS;
import static gov.nasa.luv.Constants.PROP_FILE_RECENT_COUNT;
import static gov.nasa.luv.Constants.PROP_FILE_RECENT_PLAN_DIR;
import static java.awt.event.InputEvent.META_MASK;
import static java.awt.event.KeyEvent.VK_E;
import static java.awt.event.KeyEvent.VK_O;
import static javax.swing.JFileChooser.APPROVE_OPTION;

import java.awt.Color;
import java.awt.Component;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.io.File;
import java.io.IOException;

import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
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
	private JButton planBut, configBut, scriptBut, simScriptBut, saveBut, cancelBut;
	private JRadioButton plexilExec, plexilTest, plexilSim;
	//private JCheckBox plexilExec, plexilTest, plexilSim;
	private JLabel planLab, configLab, scriptLab, simScriptLab;
	private ArrayList<JButton> buttonList;
	private HashMap<JButton, String> recentDirMap;
	private HashMap<JButton, JLabel> objMap;
	private HashMap<JButton, PlexilFilter> extMap;
	private JPanel patternPanel;
	GridBagLayout gridbag;
	private GridBagConstraints c;
	private PlexilFilter planFilter, configFilter, scriptFilter, simScriptFilt;
	private JFileChooser dirChooser;
	private int mode, modeTemp;
	private ExecAction action;
	public static final int PLEXIL_EXEC = 1, PLEXIL_TEST = 2, PLEXIL_SIM = 3;
	
	private String planName = "", configName = "", scriptName = "", simScriptName = "";
	private String planTemp = "", configTemp = "", scriptTemp = "", simScriptTemp = "";
	
	public String getPlanName() {
		return planName;
	}

	public void setPlanName(String planName) {
		this.planName = planName;
		if(planBut!=null)
			planLab.setText(planName.replaceAll(".*/",""));
	}

	public String getConfigName() {
		return configName;
	}

	public void setConfigName(String configName) {
		this.configName = configName;
		if(configBut!=null)
			configLab.setText(configName.replaceAll(".*/",""));
	}

	public String getScriptName() {
		return scriptName;
	}

	public void setScriptName(String scriptName) {
		this.scriptName = scriptName;
		if(scriptBut!=null)
			scriptLab.setText(scriptName.replaceAll(".*/",""));
	}

	public void setSimScriptName(String simScriptName) {
		this.simScriptName = simScriptName;
		if(simScriptBut!=null)
			simScriptLab.setText(simScriptName.replaceAll(".*/",""));
	}
	
	public String getSimScriptName() {
		return simScriptName;
	}
	
	public JButton getSaveBut() {
		return saveBut;
	}
	
	public int getMode() {
		return mode;
	}

	public ExecSelect() {
		init();	
		loadFromPersistence();
		constructFrame();
		add(patternPanel);				
		setBorder(BorderFactory.createEmptyBorder(20, 40, 20, 40));		
		frame = new JFrame("Executive Select");
		
	}
	
	private void init(){
		exec = this;
		objMap = new HashMap<JButton, JLabel>();		
		buttonList = new ArrayList<JButton>();
		recentDirMap = new HashMap<JButton, String>();
		extMap = new HashMap<JButton, PlexilFilter>();		
		action = new ExecAction();
		
		ButtonGroup execGroup = new ButtonGroup(); 
		plexilExec = new JRadioButton("PlexilExec");		
		execGroup.add(plexilExec);
		plexilExec.addActionListener(new ExecChooserListener());		
		plexilTest = new JRadioButton("PlexilTest");
		execGroup.add(plexilTest);
		setRadioMode(PLEXIL_TEST);
		plexilTest.addActionListener(new ExecChooserListener());		
		plexilSim = new JRadioButton("PlexilSim");
		execGroup.add(plexilSim);
		plexilSim.addActionListener(new ExecChooserListener());		
		
		planBut = new JButton(action.choosePlanAction);
		planBut.addActionListener(new ButtonFileListener());
		planLab = new JLabel("");
		configBut = new JButton(action.chooseConfigAction);
		configBut.addActionListener(new ButtonFileListener());
		configBut.setEnabled(false);
		configLab = new JLabel("");
		scriptBut = new JButton(action.chooseScriptAction);
		scriptBut.addActionListener(new ButtonFileListener());
		scriptLab = new JLabel("");
		simScriptBut = new JButton("Choose Additional Script...");
		simScriptBut.addActionListener(new ExtensionButtonFileListener());
		simScriptBut.setEnabled(false);
		simScriptLab = new JLabel("");		
		
		saveBut = new JButton("Save");
		saveBut.addActionListener(new SaveButtonListener());
		cancelBut = new JButton("Cancel");
		cancelBut.addActionListener(new ButtonListener());
		
		patternPanel = new JPanel();
		
		dirChooser = new JFileChooser()
		{
		    {
			setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
		    }
		};		
		planFilter = new PlexilFilter("XML / PLX");
		configFilter = new PlexilFilter("XML");
		scriptFilter = new PlexilFilter("XML / PLS");		
		simScriptFilt = new PlexilFilter("TXT");
	}
	
	private void loadFromPersistence() {
		String planName = LoadRecentAction.getRecentPlan(1) == Constants.UNKNOWN ? "" : LoadRecentAction.getRecentPlan(1);
		String scriptName = LoadRecentAction.getRecentScript(1) == Constants.UNKNOWN ? "" : LoadRecentAction.getRecentScript(1);		
		setPlanName(planName);
		setScriptName(scriptName);
	}
	
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
		
		setGridbag(plexilExec, 0, 0, 1, 1, 1, 1);
		setGridbag(plexilTest, 0, 1, 1, 1, 1, 1);
		setGridbag(plexilSim, 0, 2, 1, 1, 1, 1);
		
		setGridbag(planBut, 0, 3, 1, 1, 1, 1);
		setGridbag(planLab, 1, 3, 1, 5, 1, 1);
		setupButton(planBut, planLab, planFilter);
		
		setGridbag(configBut, 0, 4, 1, 1, 1, 1);
		setGridbag(configLab, 1, 4, 1, 5, 1, 1);
		setupButton(configBut, configLab, configFilter);
		
		setGridbag(scriptBut, 0, 5, 1, 1, 1, 1);
		setGridbag(scriptLab, 1, 5, 1, 5, 1, 1);
		setupButton(scriptBut, scriptLab, scriptFilter);			
		
		setGridbag(simScriptBut, 0, 6, 1, 1, 1, 1);
		setGridbag(simScriptLab, 1, 6, 1, 5, 1, 1);
		setupButton(simScriptBut, simScriptLab, simScriptFilt);				
				
		setGridbag(saveBut, 0, 7, 1, 1, 1, 1);
		setGridbag(cancelBut, 1, 7, 1, 1, 1, 1);	
	}
	
	private void setGridbag(Component comp, int x, int y, int top, int left, int bot, int right)
	{
		c.insets = new Insets(top , left , bot , right);
		setGridbag(comp, x, y);
	}
	
	private void setGridbag(Component comp, int x, int y)
	{
		c.weightx = 0.5;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = x;
		c.gridy = y;
		gridbag.setConstraints(comp, c);
		patternPanel.add(comp);		
	}		
	
	private void setupButton(JButton button, JLabel lab, PlexilFilter filt , String constant){
		setupButton(button, lab, filt);		
		recentDirMap.put(button, constant);
	}
	
	private void setupButton(JButton button, JLabel lab, PlexilFilter filt){		
		setupButton(button, lab);		
		extMap.put(button, filt);
	}
	
	private void setupButton(JButton button, JLabel lab){
		buttonList.add(button);
		objMap.put(button, lab);		
	}		
	
	public JFrame getFrame()
	{
		return frame;
	}
	
	class ButtonListener implements ActionListener {
		ButtonListener(){			
		}
		
		public void actionPerformed(ActionEvent e){						
			if(e.getActionCommand().equals("Cancel"))
			{
				Luv.getLuv().getExecSelect().frame.setVisible(false);
				Luv.getLuv().getStatusMessageHandler().showStatus("Reverted Configuration");
				setPlanName(planTemp);
				setConfigName(configTemp);
				setScriptName(scriptTemp);
				setSimScriptName(simScriptTemp);
				setRadioMode(modeTemp);
			}
		}
	}
	
	private void setRadioMode(int mode){
		this.mode = mode;
		switch (mode){
			case PLEXIL_EXEC: plexilExec.setSelected(true); break;
			case PLEXIL_TEST: plexilTest.setSelected(true); break;
			case PLEXIL_SIM: plexilSim.setSelected(true); break;			
		}
	}
	
	private void matchButtonFunction(File file, JButton but){				
		if(file != null)
			objMap.get(but).setText(file.getName());		
		if(but == planBut)
			planName = file.getAbsolutePath();
		else if(but == configBut)
			configName = file.getAbsolutePath();
		else if(but == scriptBut)
			scriptName = file.getAbsolutePath();
	}
	
	private void openFile(ActionEvent e){
		final JFileChooser fc;			
		String nameProp = "";
		if(e.getSource() != null)
			nameProp = recentDirMap.get(e.getSource());
		if(nameProp == null)
			nameProp = Luv.getLuv().getProperty(PROP_FILE_RECENT_PLAN_DIR);
		fc = new JFileChooser(new File(nameProp));
		PlexilFilter pf = extMap.get(e.getSource());
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
	
	public void backupNames(){
		planTemp = planName;
		configTemp = configName;
		scriptTemp = scriptName;
		simScriptTemp = simScriptName;
		modeTemp = mode;
	}

	class ButtonFileListener implements ActionListener {
		ButtonFileListener(){			
		}
		public void actionPerformed(ActionEvent e){			
	    		if(e.getSource()==ExecSelect.exec.planBut && Luv.getLuv().getCurrentPlan().getPlanName()!=Constants.UNKNOWN)
	    			objMap.get(e.getSource()).setText(Luv.getLuv().getCurrentPlan().getPlanName());
	    		else if(Luv.getLuv().getCurrentPlan().getScriptName()!=Constants.UNKNOWN)
	    			objMap.get(e.getSource()).setText(Luv.getLuv().getCurrentPlan().getScriptName());    		                		            
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
			if(!Luv.getLuv().getIsExecuting())
			{
				if(planName != "")
				{								
						Luv.getLuv().setNewPlan(true);
						PlexilPlanHandler.resetRowNumber();
						Luv.getLuv().getFileHandler().loadPlan(new File(ExecSelect.getExecSel().getPlanName()));
						//Luv.getLuv().setNewPlan(false);
						Luv.getLuv().getLuvStateHandler().openPlanState();
						Luv.getLuv().getLuvStateHandler().readyState();
						Luv.getLuv().setNewPlan(false);
				}
				if(mode == PLEXIL_EXEC)		
					Luv.getLuv().getFileHandler().loadScript(new File(getConfigName()));
				if(mode == PLEXIL_TEST)
					Luv.getLuv().getFileHandler().loadScript(new File(getScriptName()));
				if(mode == PLEXIL_SIM)
					Luv.getLuv().getFileHandler().loadScript(new File(getSimScriptName()));
			}
			/*else	{
                try {
                    Luv.getLuv().getLuvStateHandler().stopExecutionState();
                    Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Stopping execution and opening a new Config");
                } catch (IOException ex) {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while stopping execution");
                }
            }

            Luv.getLuv().getLuvStateHandler().readyState();
            */
			Luv.getLuv().getExecSelect().frame.setVisible(false);
		}
	}
	
	class ExecChooserListener implements ActionListener {
		ExecChooserListener(){			
		}
            // KMD: this doesn't work with javac 1.5
            //		@Override		
		public void actionPerformed(ActionEvent e){
			if(e.getActionCommand() == plexilExec.getText())
			{				
				if (!Luv.getLuv().getIsExecuting() && plexilExec.isSelected()) {
					mode = PLEXIL_EXEC;
					Luv.getLuv().setTestExecAllowed(false);
					Luv.getLuv().getStatusMessageHandler().showStatus(
							"Use UniversalExec", Color.GREEN.darker(), 1000);						
					JButton[] on={planBut, configBut};
					handleButtons(on, true);
					JButton[] off={scriptBut, simScriptBut};
					handleButtons(off, false);
					reload(plexilExec);
				}
			}
			else if(e.getActionCommand() == plexilTest.getText())
			{
				if (!Luv.getLuv().getIsExecuting() && plexilTest.isSelected()) {
					mode = PLEXIL_TEST;
					Luv.getLuv().setTestExecAllowed(true);
					Luv.getLuv().getStatusMessageHandler().showStatus(
							"Use TestExec", Color.GREEN.darker(), 1000);				
					JButton[] on={planBut, scriptBut};
					handleButtons(on, true);
					JButton[] off={configBut, simScriptBut};
					handleButtons(off, false);
					reload(plexilTest);
				}
			}
			else if(e.getActionCommand() == plexilSim.getText())
			{
				if (!Luv.getLuv().getIsExecuting() && plexilSim.isSelected()) {
					mode = PLEXIL_SIM;
					Luv.getLuv().setTestExecAllowed(false);
					Luv.getLuv().getStatusMessageHandler().showStatus(
							"Use PlexilSim", Color.GREEN.darker(), 1000);								
					JButton[] on={planBut, configBut, simScriptBut};
					handleButtons(on, true);
					JButton[] off={scriptBut};
					handleButtons(off, false);
					reload(plexilSim);
				}
			}
		}
		private void handleButtons(JButton[] buttons, boolean toggle){
			for(int i=0;i<buttons.length;i++)
				buttons[i].setEnabled(toggle);
		}
		private void reload(JRadioButton supplement){			
			File plan = null, script = null, config = null;
			Luv.getLuv().setNewPlan(true);
            PlexilPlanHandler.resetRowNumber();

            if (Luv.getLuv().getIsExecuting()) {
                try {
                    Luv.getLuv().getLuvStateHandler().stopExecutionState();
                    Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Stopping execution and reloading plan");
                } catch (IOException ex) {
                    Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while reloading plan");
                }
            }

            if(planName == null || planName == "")
            	return;
            plan = new File(planName);
            if (plan.exists()) {
                Luv.getLuv().getFileHandler().loadPlan(plan);
                Luv.getLuv().getStatusMessageHandler().showStatus("Plan \"" + Luv.getLuv().getCurrentPlan().getAbsolutePlanName() + "\" loaded", 1000);
                Luv.getLuv().getLuvStateHandler().openPlanState();
            } else {
                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: trying to reload an UNKNOWN plan");
                Luv.getLuv().getLuvStateHandler().reloadPlanState();
            }
            Luv.getLuv().setNewPlan(false);
			
            if(supplement == plexilTest)
            {
	            if(scriptName == null || scriptName == "")
	            	return;
	            script = new File(scriptName);
	            if (script.exists()) {
	                Luv.getLuv().getFileHandler().loadScript(script);
	                Luv.getLuv().getStatusMessageHandler().showStatus("Script \"" + Luv.getLuv().getCurrentPlan().getAbsoluteScriptName() + "\" loaded", 1000);                
	            }
            } else
            {
            	if(configName != null || configName == "")
	            	config = new File(configName);
	            if (config.exists()) {
	                Luv.getLuv().getFileHandler().loadScript(config);
	                Luv.getLuv().getStatusMessageHandler().showStatus("Config \"" + Luv.getLuv().getCurrentPlan().getAbsoluteScriptName() + "\" loaded", 1000);                
	            }
            }
		}		
	}
	
	class PlexilFilter extends FileFilter {
		
		private String descr;
		
		PlexilFilter(String description){
			descr = description;
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
                            for (String ext: FILE_EXTENSIONS)
                                if (extension.equals(ext))
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
	
	class ExecAction {
	    public LuvAction choosePlanAction =
            new LuvAction("Choose Plan",
            "Choose a plexil plan file.",
            VK_O,
            META_MASK) {

                public void actionPerformed(ActionEvent e) {
                	openFile(e);
                }
            };
    /** Action to load a script for Execution. */
    public LuvAction chooseScriptAction =    	
            new LuvAction("Choose Script",
            "Choose a script file.",
            VK_E,
            META_MASK) {

                public void actionPerformed(ActionEvent e) {
                	openFile(e);                    
                }
            };
    
    /** Action to load a config for Execution. */
    public LuvAction chooseConfigAction =    	
        new LuvAction("Choose Config",
                "Choose a Config file.",
                VK_E,
                META_MASK) {

                    public void actionPerformed(ActionEvent e) {
                    	openFile(e);
                    }
                };   
	}
	
}