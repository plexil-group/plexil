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

import gov.nasa.luv.Constants.AppType;
import static gov.nasa.luv.Constants.AppType.*;
import static gov.nasa.luv.Constants.FILE_EXTENSIONS;

public class ExecSelectDialog extends JPanel {
	/**
	 * 
	 */	
    private Luv theLuv;
    private Settings settings;
	private JFrame frame;
	private JButton planBut, configBut, scriptBut, saveBut, cancelBut, defaultScriptBut, defaultConfigBut, libBut;
	private JRadioButton externalApp, plexilExec, plexilTest, plexilSim;
	private JLabel planLab, configLab, scriptLab;	
	private JPanel patternPanel;
	private PlexilFilter planFilter, configFilter, simScriptFilter, teScriptFilter;
	private JFileChooser dirChooser;

	public ExecSelectDialog(Luv luv) {
        theLuv = luv;
        settings = theLuv.getSettings();
		init();
		constructFrame();
		add(patternPanel);				
		setBorder(BorderFactory.createEmptyBorder(20, 40, 20, 40));		
        setOpaque(true);
		frame = new JFrame("Select Configuration");
        frame.setContentPane(this);
	}
	
	private void init() {
		ButtonGroup execGroup = new ButtonGroup(); 
		externalApp = new JRadioButton("External");
		externalApp.setToolTipText("Monitor plan execution on an external executive");
		execGroup.add(externalApp);
		externalApp.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    settings.setAppMode(EXTERNAL_APP);
                    theLuv.getStatusMessageHandler().showStatus("Monitor external exec", Color.GREEN.darker(), 1000);
                    refresh();
                }
            }
            );

		plexilExec = new JRadioButton("PlexilExec");
		plexilExec.setToolTipText("Execute a plan on the PlexilExec");
		execGroup.add(plexilExec);
		plexilExec.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    settings.setAppMode(PLEXIL_EXEC);
                    theLuv.getStatusMessageHandler().showStatus("Use UniversalExec", Color.GREEN.darker(), 1000);
                    refresh();
                }
            }
            );

		plexilTest = new JRadioButton("PlexilTest");
		plexilTest.setToolTipText("Simulate Plan Execution using a Test Executive Script");
		execGroup.add(plexilTest);
		plexilTest.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    settings.setAppMode(PLEXIL_TEST);
                    theLuv.getStatusMessageHandler().showStatus("Use TestExec", Color.GREEN.darker(), 1000);
                    refresh();
                }
            }
            );

		plexilSim = new JRadioButton("PlexilSim");
		plexilSim.setToolTipText("Execute a plan using PlexilExec and PlexilSimulator");
		execGroup.add(plexilSim);
		plexilSim.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    settings.setAppMode(PLEXIL_SIM);
                    theLuv.getStatusMessageHandler().showStatus("Use PlexilSim", Color.GREEN.darker(), 1000);
                    refresh();
                }
            }
            );

		planLab = new JLabel("");
		planBut =
            new JButton(new LuvAction("Plan",
                                      "Choose a plexil plan file.",
                                      VK_O,
                                      META_MASK) {
                    public void actionPerformed(ActionEvent e) {
                        File p = openFile(e,
                                          planFilter,
                                          settings.getPlanLocation());
                        if (p != null) {
                            settings.setPlanLocation(p);
                            updateLabel(planLab, p);
                        }
                    }
                }
                        );

		libBut = new JButton("Libraries");
		libBut.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    theLuv.getLibraryLoader().openDialog();
                }
            }
            );

		configLab = new JLabel("");
		configBut =
            new JButton(new LuvAction("Config",
                                      "Choose a Config file.",
                                      VK_E,
                                      META_MASK) {

                    public void actionPerformed(ActionEvent e) {
                        File c = openFile(e,
                                          configFilter,
                                          settings.getConfigLocation());
                        if (c != null) {
                            settings.setConfigLocation(c);
                            updateLabel(configLab, c);
                        }
                    }
                }
                        );
		configBut.setEnabled(false);
		configBut.setVisible(false);

		defaultConfigBut = new JButton("Use Default");
		defaultConfigBut.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    // FIXME - Only relevant to UE
                    settings.setConfigLocation(new File(Constants.DEFAULT_CONFIG_PATH, Constants.DEFAULT_CONFIG_NAME));
                    updateLabel(configLab, settings.getConfigLocation());
                    theLuv.getStatusMessageHandler().showStatus("Default Config");
                }
            }
            );
		defaultConfigBut.setVisible(false);

		scriptLab = new JLabel("");	
		scriptBut =
            new JButton(new LuvAction("Script",
                                      "Choose a script file.",
                                      VK_E,
                                      META_MASK) {
                    public void actionPerformed(ActionEvent e) {
                        File s = openFile(e,
                                          (settings.getAppMode() == PLEXIL_TEST)
                                          ? teScriptFilter
                                          : simScriptFilter,
                                          settings.getScriptLocation());
                        if (s != null) {
                            settings.setScriptLocation(s);
                            updateLabel(scriptLab, s);
                        }
                    }
                }
                        );

		defaultScriptBut = new JButton("Use Default");
		defaultScriptBut.addActionListener(new ActionListener() {
                // Dumbed down for simplicity's sake
                public void actionPerformed(ActionEvent e) {
                    File script = settings.defaultEmptyScriptFile();
                    settings.setScriptLocation(script);
                    updateLabel(scriptLab, script);
                    theLuv.getStatusMessageHandler().showStatus("Default Script");
                }
            }
            );
		
		saveBut = new JButton("OK");
		saveBut.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    saveSettings();
                    frame.setVisible(false);
                }
            }
            );
		cancelBut = new JButton("Cancel");
		cancelBut.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    settings.loadFromProperties(); // restore
                    frame.setVisible(false);
                    theLuv.getStatusMessageHandler().showStatus("Reverted Configuration");
                }
            }
            );
		
		patternPanel = new JPanel();
		
		dirChooser = new JFileChooser(Constants.PLEXIL_HOME);
        dirChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);

		planFilter = new PlexilFilter("PLX");
		configFilter = new PlexilFilter("XML");
        // FIXME
        simScriptFilter = new PlexilFilter("xml / plx / psx / txt / pst / pls");
		teScriptFilter = new PlexilFilter("PSX");
	}

	private void constructFrame() {
		GridBagLayout gridbag = new GridBagLayout();
		patternPanel.setLayout(gridbag);

        // Radio buttons
		int row = 0;
		gridbag.setConstraints(externalApp, makeConstraints(0, row, 1, 1, 1, 1));
		gridbag.setConstraints(plexilExec, makeConstraints(1, row, 1, 1, 1, 1));
		gridbag.setConstraints(plexilTest, makeConstraints(2, row, 1, 1, 1, 1));
		gridbag.setConstraints(plexilSim, makeConstraints(3, row, 1, 1, 1, 1));
        patternPanel.add(externalApp);
        patternPanel.add(plexilExec);
        patternPanel.add(plexilTest);
        patternPanel.add(plexilSim);
		
        // Plan button, label
		row++; //1
		gridbag.setConstraints(planBut, makeConstraints(0, row, 1, 1, 1, 1));
		gridbag.setConstraints(planLab, makeConstraints(1, row, 1, 5, 1, 1));
        patternPanel.add(planBut);
        patternPanel.add(planLab);

        // Library button
		row++; //2
		gridbag.setConstraints(libBut, makeConstraints(0, row, 1, 1, 1, 1));
        patternPanel.add(libBut);
			
        // Configuration
		row++; //3
		gridbag.setConstraints(configBut, makeConstraints(0, row, 1, 1, 1, 1));
		gridbag.setConstraints(configLab, makeConstraints(1, row, 1, 5, 1, 1));		
		gridbag.setConstraints(defaultConfigBut, makeConstraints(2, row, 1, 10, 1, 1));
        patternPanel.add(configBut);
        patternPanel.add(configLab);
        patternPanel.add(defaultConfigBut);
				
        // Sim script
		row++;//4
		gridbag.setConstraints(scriptBut, makeConstraints(0, row, 1, 1, 1, 1));
		gridbag.setConstraints(scriptLab, makeConstraints(1, row, 1, 5, 1, 1));
		gridbag.setConstraints(defaultScriptBut, makeConstraints(2, row, 1, 10, 1, 1));
        patternPanel.add(scriptBut);
        patternPanel.add(scriptLab);
        patternPanel.add(defaultScriptBut);
		
		row+=3;//6
		//gridbag.setConstraints(defaultBut, makeConstraints(0, row, 10, 10, 1, 10));
		gridbag.setConstraints(cancelBut, makeConstraints(1, row, 10, 10, 1, 10));
		gridbag.setConstraints(saveBut, makeConstraints(2, row, 10, 10, 1, 10));
        //patternPanel.add(defaultBut);
        patternPanel.add(cancelBut);
        patternPanel.add(saveBut);
	}
	
	/*
     * Extended Gridbag Configure component method
     * @param x and y are position in panel. top, left, bot, right refer to spacing around object
     */

	private static GridBagConstraints makeConstraints(int x, int y, int top, int left, int bot, int right) {
		GridBagConstraints c = new GridBagConstraints();		
        c.insets = new Insets(top , left , bot , right);
		c.weightx = 0.5;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridx = x;
		c.gridy = y;
        return c;
	}

    // Needed by LibraryLoader
    public FileFilter getPlanFileFilter() {
        return planFilter;
    }
	
	public JButton getSaveBut() {
		return saveBut;
	}

	public JButton getDefaultScriptBut() {
		return defaultScriptBut;
	}

	public void activate() {
        settings.saveToProperties(); // back up so we can restore if needed
		refresh();
        frame.setVisible(true);
	}
	
    // Refreshes the dialog display on update
	private void refresh() {
        Settings s = settings;
        setMode(s.getAppMode());
        updateButtonVisibility();

        // Update plan location
		File plan = s.getPlanLocation();
        if (plan != null) {
            if (!plan.exists()) {
                theLuv.getStatusMessageHandler().displayErrorMessage(null,
                                                                     "ERROR: unable to find plan file "
                                                                     + plan.toString());
                s.setPlanLocation(null);
            }
        }
        updateLabel(planLab, s.getPlanLocation());
        
        // Update script location if appropriate to mode
        AppType mode = settings.getAppMode();
        if (mode == PLEXIL_SIM || mode == PLEXIL_TEST) {
            File script = s.getScriptLocation();
            if (script != null) {
                if (!script.exists()) {
                    theLuv.getStatusMessageHandler().displayErrorMessage(null,
                                                                         "ERROR: simulation script file not found: "
                                                                         + script.toString());
                    s.setScriptLocation(null);
                }
                updateLabel(scriptLab, s.getScriptLocation());
            }
        } else {
            updateLabel(scriptLab, null);
        }
        
        // Update config location if appropriate to mode
        if (mode == PLEXIL_EXEC || mode == PLEXIL_SIM) {
            File config = s.getConfigLocation();
            if (config != null) {
                if (!config.exists()) {
                    theLuv.getStatusMessageHandler().displayErrorMessage(null,
                                                                         "ERROR: simulation config file not found: "
                                                                         + config.toString());
                    s.setConfigLocation(null);
                }
                updateLabel(configLab, s.getConfigLocation());
            }
        } else {
            updateLabel(configLab, null);
        }

        frame.pack();
	}

    private void updateButtonVisibility() {
        // Handle buttons and labels
        // Plan, library buttons always enabled and visible
        switch (settings.getAppMode()) {
        case EXTERNAL_APP:
            configBut.setVisible(false);
            configBut.setEnabled(false);
            configLab.setVisible(false);
            defaultConfigBut.setVisible(false);
            defaultConfigBut.setEnabled(false);

            scriptBut.setVisible(false);
            scriptBut.setEnabled(false);
            scriptLab.setVisible(false);
            defaultScriptBut.setVisible(false);
            defaultScriptBut.setEnabled(false);

            break;

        case PLEXIL_EXEC:
            configBut.setVisible(true);
            configBut.setEnabled(true);
            configLab.setVisible(true);
            defaultConfigBut.setEnabled(true);
            defaultConfigBut.setVisible(true);

            scriptBut.setVisible(false);
            scriptBut.setEnabled(false);
            scriptLab.setVisible(false);
            defaultScriptBut.setVisible(false);
            defaultScriptBut.setEnabled(false);
            break;

        case PLEXIL_TEST:
            configBut.setVisible(false);
            configBut.setEnabled(false);
            configLab.setVisible(false);
            defaultConfigBut.setVisible(false);
            defaultConfigBut.setEnabled(false);

            scriptBut.setVisible(true);
            scriptBut.setEnabled(true);
            scriptLab.setVisible(true);
            defaultScriptBut.setEnabled(true);
            defaultScriptBut.setVisible(true);
            break;

        case PLEXIL_SIM:
            configBut.setVisible(true);
            configBut.setEnabled(true);
            configLab.setVisible(true);
            defaultConfigBut.setEnabled(true);
            defaultConfigBut.setVisible(true);

            scriptBut.setVisible(true);
            scriptBut.setEnabled(true);
            scriptLab.setVisible(true);
            defaultScriptBut.setEnabled(true);
            defaultScriptBut.setVisible(true);
            break;

        case USER_SPECIFIED:
            // *** TODO ***
            break;
        }
    }

    private static void updateLabel(JLabel label, File filename)
    {
        if (filename == null) {
            label.setText("");
            label.setToolTipText("");
        } else {
            label.setText(filename.getName());
            label.setToolTipText(filename.toString());
        }
    }

	/*
	 * exposed method for reloading plan and supplement set from file menu
	 */
	public void reload() {		
        if (theLuv.getIsExecuting()) {
            // *** FIXME: delegate to Luv instance ***
            try {
                theLuv.stopExecutionState();
                theLuv.getStatusMessageHandler().displayInfoMessage("Stopping execution and reloading plan");
            } catch (IOException ex) {
                theLuv.getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while reloading plan");
            }
        }        
                
		refresh();

		File plan = settings.getPlanLocation();
        if (plan == null)
        	return;
        
        if (plan.exists()) {
            theLuv.loadPlan(plan);
        } else {
            theLuv.getStatusMessageHandler().displayErrorMessage(null,
                                                                 "ERROR: While reloading plan: unable to find plan file "
                                                                 + plan.toString());
            theLuv.reloadPlanState(); // *** FIXME ***
        }
        
        AppType mode = settings.getAppMode();
        if (mode == PLEXIL_SIM || mode == PLEXIL_TEST) {
            File script = settings.getScriptLocation();
            // *** FIXME ***
            if (script == null)
                return;
        
            if (script.exists()) {
                theLuv.getFileHandler().loadScript(script);
                theLuv.getStatusMessageHandler().showStatus("Script \""
                                                            + theLuv.getCurrentPlan().getAbsoluteScriptName()
                                                            + "\" loaded",
                                                            1000);
            }
        }

        if (mode == PLEXIL_SIM || mode == PLEXIL_EXEC) {
            File config = settings.getConfigLocation();
            // *** FIXME ***
            if (config == null)
                return;
        
            if (config.exists()) {
                theLuv.getFileHandler().loadConfig(config);
                theLuv.getStatusMessageHandler().showStatus("Config "
                                                            + theLuv.getCurrentPlan().getAbsoluteConfigName()
                                                            + " loaded",
                                                            1000);
            }
        }

        theLuv.setTitle();
		try {
			theLuv.getSourceWindow().refresh();
		} catch (IOException ex){
			theLuv.getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while refreshing source window");
		}
	}
	
	/*
	 * Changes configuration mode
	 * @param Plexil mode constant
	 */
	private void setMode(AppType newMode) {
		switch (newMode) {
        case EXTERNAL_APP:
            externalApp.setSelected(true);
            break;

        case PLEXIL_EXEC:
            plexilExec.setSelected(true);
            break;

        case PLEXIL_TEST:
            plexilTest.setSelected(true);
            break;

        case PLEXIL_SIM:
            plexilSim.setSelected(true);
            break;
		}
        settings.setAppMode(newMode);
	}

    // Called by save button listener
    private void saveSettings() {
        if (theLuv.getIsExecuting()) {
            // *** FIXME: delegate to Luv instance ***
            try {
                theLuv.stopExecutionState();
                theLuv.getStatusMessageHandler().displayInfoMessage("Stopping execution and loading plan");
            } catch (IOException ex) {
                theLuv.getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while reloading plan");
            }
        }

        // Commit settings
        settings.saveToProperties();
        settings.save(); // to prefs file

        if (!theLuv.getIsExecuting()) {
            if (settings.getPlanLocation() != null) {	
                theLuv.loadPlan(settings.getPlanLocation());
                theLuv.readyState();
            }
            AppType mode = settings.getAppMode();
            if (mode == PLEXIL_EXEC || mode == PLEXIL_SIM) {
                File config = settings.getConfigLocation();
                if (config != null) {
                    theLuv.getStatusMessageHandler().showStatus("Config \""
                                                                + theLuv.getCurrentPlan().getAbsoluteConfigName() // FIXME
                                                                + "\" loaded", 1000);
                }
            }
            if (mode == PLEXIL_SIM || mode == PLEXIL_TEST) {
                File script = settings.getScriptLocation();
                if (script != null) {
                    theLuv.getStatusMessageHandler().showStatus("Script \""
                                                                + theLuv.getCurrentPlan().getAbsoluteScriptName() // FIXME
                                                                + "\" loaded", 1000);
                    // FIXME: why is this done twice?
                    theLuv.setTitle();
                    try {
                        theLuv.getSourceWindow().refresh();
                    } catch (IOException ex) {
                        theLuv.getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while opening source window");
                    }					
                }
            }
            theLuv.setTitle();
            try {
                theLuv.getSourceWindow().refresh();
            } catch (IOException ex) {
                theLuv.getStatusMessageHandler().displayErrorMessage(ex, "ERROR: exception occurred while opening source window");
            }					
        }
    }
	
	/*
	 * universal open file method
	 */
	private File openFile(ActionEvent e, PlexilFilter pf, File dflt) {
		final JFileChooser fc;
        
        // If the default is a file rather than a directory, get its parent directory.
        if (dflt != null && !dflt.isDirectory())
            dflt = dflt.getParentFile();

		fc = new JFileChooser(dflt);
		if (pf != null)
            fc.addChoosableFileFilter(pf);

		int returnVal = fc.showDialog(dirChooser, "Choose File");
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            File file = fc.getSelectedFile();
            theLuv.getStatusMessageHandler().showStatus("Selected " + file.getAbsolutePath());
            return file;
        }
        return null;
	}

	private class PlexilFilter extends FileFilter {
		
		private String descr;
		private String[] filter;
		
		PlexilFilter(String description){
			descr = description;
			filter = descr.split(" / ");
			if (filter.length < 1 || filter[0].matches(""))
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
		    if (extension != null) {
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
}
