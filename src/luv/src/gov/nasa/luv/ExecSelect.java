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
	private JButton planBut, configBut, scriptBut, saveBut, cancelBut, clearBut;
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
	private ExecAction action;	
	private AppSettings execSet, testSet, simSet, backExec, backTest, backSim;
	public static final int RECENT_LIB = 1, RECENT_PLAN = 2, RECENT_SUPP = 3;
		
	public JButton getSaveBut() {
		return saveBut;
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
		action = new ExecAction();
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
		
		saveBut = new JButton("Ok");
		saveBut.addActionListener(new SaveButtonListener());
		cancelBut = new JButton("Cancel");
		cancelBut.addActionListener(new ButtonListener());
		clearBut = new JButton("Clear Script");
		clearBut.addActionListener(new ButtonListener());
		
		patternPanel = new JPanel();
		
		dirChooser = new JFileChooser()
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
		String planName = "", suppName = "";
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
		
		setGridbag(plexilExec, 0, 0, 1, 1, 1, 1);
		setGridbag(plexilTest, 1, 0, 1, 1, 1, 1);
		setGridbag(plexilSim, 2, 0, 1, 1, 1, 1);
		
		setGridbag(planBut, 0, 1, 1, 1, 1, 1);
		setGridbag(planLab, 1, 1, 1, 5, 1, 1);
		setupButton(planBut, planLab, planFilter, RECENT_PLAN);
		
		setGridbag(configBut, 0, 2, 1, 1, 1, 1);
		setGridbag(configLab, 1, 2, 1, 5, 1, 1);
		setupButton(configBut, configLab, configFilter, RECENT_SUPP);
		
		setGridbag(scriptBut, 0, 3, 1, 1, 1, 1);
		setGridbag(scriptLab, 1, 3, 1, 5, 1, 1);		
		setupButton(scriptBut, scriptLab, scriptFilter, RECENT_SUPP);		
				
		setGridbag(saveBut, 0, 5, 10, 10, 1, 10);
		setGridbag(cancelBut, 1, 5, 10, 10, 1, 10);
		setGridbag(clearBut, 2, 5, 10, 10, 1, 10);
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
		 case PLEXIL_EXEC: getSettings().initializeSuppLabel(configLab); break;
		 case PLEXIL_TEST: getSettings().initializeSuppLabel(scriptLab); break;
		 case PLEXIL_SIM: getSettings().initializeSuppLabel(scriptLab); break;			 
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
                clearBut.setText("Clear Config");
            }
        } else
        {            
           	script = new File(suppnm);
            if (script.exists()) {
                getSettings().initializeSuppLabel(scriptLab);
                clearBut.setText("Clear Script");
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
                Luv.getLuv().getLuvStateHandler().stopExecutionState();
                Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Stopping execution and reloading plan");
            } catch (IOException ex) {
                Luv.getLuv().getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while reloading plan");
            }
        }        
                
        if(plannm == null || plannm.equals(""))
        	return;
        
        plan = new File(plannm);
        if (plan.exists()) {
            Luv.getLuv().getFileHandler().loadPlan(plan);
            Luv.getLuv().getStatusMessageHandler().showStatus("Plan \"" + Luv.getLuv().getCurrentPlan().getAbsolutePlanName() + "\" loaded", 1000);
            Luv.getLuv().getLuvStateHandler().openPlanState();
        } else {
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, "ERROR: trying to reload an UNKNOWN plan");
            Luv.getLuv().getLuvStateHandler().reloadPlanState();
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
	}
	
	class ButtonListener implements ActionListener {
		ButtonListener(){			
		}
		
		public void actionPerformed(ActionEvent e){						
			if(e.getActionCommand().equals("Cancel"))
			{
				Luv.getLuv().getExecSelect().frame.setVisible(false);
				Luv.getLuv().getStatusMessageHandler().showStatus("Reverted Configuration");
				setInputs(execSet, backExec.getPlanLocation(), backExec.getSuppLocation());
				setInputs(testSet, backTest.getPlanLocation(), backTest.getSuppLocation());
				setInputs(simSet, backSim.getPlanLocation(), backSim.getSuppLocation());				
				setRadioMode(modeTemp);
			}
			if(e.getActionCommand().equals("Clear Script") || e.getActionCommand().equals("Clear Config"))
			{
				//getSettings().setPlanLocation("");
				getSettings().setSuppLocation("");
				refresh();
				Luv.getLuv().getStatusMessageHandler().showStatus("Cleared");
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
			File plan = null, supp = null;
			if (Luv.getLuv().getIsExecuting()) {
	            try {
	                Luv.getLuv().getLuvStateHandler().stopExecutionState();
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
						Luv.getLuv().getFileHandler().loadPlan(plan);									            
						Luv.getLuv().getLuvStateHandler().openPlanState();						
						Luv.getLuv().getStatusMessageHandler().showStatus("Plan \"" + Luv.getLuv().getCurrentPlan().getAbsolutePlanName() + "\" loaded", 1000);
						Luv.getLuv().getLuvStateHandler().readyState();
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
					JButton[] off={scriptBut};
					handleButtons(off, false);
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
					JButton[] off={configBut};
					handleButtons(off, false);
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
					JButton[] off={configBut};
					handleButtons(off, false);
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
	}
	
	class PlexilFilter extends FileFilter {
		
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
	/** Action to load a plan for Execution. */
	class ExecAction {
	    public LuvAction choosePlanAction =
            new LuvAction("Plan",
            "Choose a plexil plan file.",
            VK_O,
            META_MASK) {

                public void actionPerformed(ActionEvent e) {
                	openFile(e);
                }
            };
    /** Action to load a script for Execution. */
    public LuvAction chooseScriptAction =    	
            new LuvAction("Script",
            "Choose a script file.",
            VK_E,
            META_MASK) {

                public void actionPerformed(ActionEvent e) {
                	openFile(e);                    
                }
            };
    
    /** Action to load a config for Execution. */
    public LuvAction chooseConfigAction =    	
        new LuvAction("Config",
                "Choose a Config file.",
                VK_E,
                META_MASK) {

                    public void actionPerformed(ActionEvent e) {
                    	openFile(e);
                    }
                };   
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
		AppSettings(String name, String supp){
			this.name = name;
			this.suppName = supp;
		}
		public String getName() {
			return name;
		}		
		public String getSuppName() {
			return suppName;
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
		public PlexilFilter getFilter() {
			return filter;
		}
		public void setFilter(PlexilFilter filter) {
			this.filter = filter;
		}
	}
	
}