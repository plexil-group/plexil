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

import static gov.nasa.luv.Constants.PLEXIL_EXEC;
import static gov.nasa.luv.Constants.PLEXIL_SIM;
import static gov.nasa.luv.Constants.PLEXIL_TEST;
import static gov.nasa.luv.Constants.PROP_CFGWIN_LOC;
import static gov.nasa.luv.Constants.PROP_CFGWIN_SIZE;

import gov.nasa.luv.ExecSelect.AppSettings;
import gov.nasa.luv.ExecSelect.PlexilFilter;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;

/**
 * This class was modeled after the CheckNodeTreeExample, CheckNode, CheckRenerer
 * classes from Tame Swing examplescreated by Nobuo Tamemasa.
 *
 * http://devdaily.com/java/swing/tame/
 * 
@author Jason Ho
@version 1.0 12/23/10
*/

public class LibraryLoader extends JFrame implements ItemListener {
    private static boolean error;
    private JPanel topSection;
    private JScrollPane checkBoxList;
    private JScrollPane previewArea;
    private JPanel buttonPane;
    private ArrayList<CheckNode> nodes;
    private JTextArea preview;
    private JTree main_tree;
    private DynamicTree dyn_tree;
    private Vector<File> libraryPath;
    private Vector<File> libraryFiles;
    private Vector<String> libraryNames;

    /** Construct a LibraryLoader. 
     *
     * @param title the title of this LibraryLoader
     * @param cmdLineArgs the parameters given at the command line
     */
    public LibraryLoader(String title, String[] cmdLineArgs)
    {
        super(title);
        error = false;
        parseCommandLineOptions(cmdLineArgs);
        createCheckList();

        if (!error) {
            createPreviewArea();
            createTopSection();
            createButtons();

            getContentPane().add(topSection, BorderLayout.NORTH);
            getContentPane().add(checkBoxList, BorderLayout.WEST);
            getContentPane().add(dyn_tree, BorderLayout.WEST);
            getContentPane().add(previewArea, BorderLayout.EAST);
            getContentPane().add(buttonPane, BorderLayout.SOUTH);
        }
    }

    /*
     *
     */
    private void parseCommandLineOptions(String[] cmdLineArgs)
    {
        libraryPath = new Vector<File>();
        libraryNames = new Vector<String>();
        libraryFiles = new Vector<File>();
        for (int i = 0; i < cmdLineArgs.length; ++i) {
            if (cmdLineArgs[i].equals("-L")) {
                // Process library path entry
                if ((++i) >= cmdLineArgs.length) {
                    // throw command parse error fatal exception
                }
                libraryPath.add(new File(cmdLineArgs[i]));
            }
            else if (cmdLineArgs[i].equals("-l")) {
                // Process library name
                if ((++i) >= cmdLineArgs.length) {
                    // throw command parse error fatal exception
                }
                libraryNames.add(cmdLineArgs[i]);
            }
        }
        // TODO: verify that library file exists
    }

    /*
     * Initializes checklist and tree
     */
    private void createCheckList()
    {
        nodes = new ArrayList<CheckNode>();
        dyn_tree = new DynamicTree();

        // TODO: Bypass setting default if user specs library dirs on cmd line
        File defaultPath = new File(Constants.PLEXIL_HOME
                                    + System.getProperty("file.separator")
                                    + "examples"
                                    + System.getProperty("file.separator"));
        addLibrary(defaultPath);
                
        // place check boxes into check box tree        
        checkBoxList = new JScrollPane(dyn_tree);
        checkBoxList.setPreferredSize(new Dimension(450, 50));
    }

    /*
     * Builds display window for loaded libraries
     */
    private void createPreviewArea() {
        preview = new JTextArea();
        preview.setPreferredSize(new Dimension(435, 50));
        setPreviewOfLibraries();
        preview.setEditable(false);
        previewArea = new JScrollPane(preview);
    }

    /*
     * Message to User Section
     */
    private void createTopSection() {
        JLabel topMessage = new JLabel();
        topMessage.setText(getTopMessage());
        topMessage.setFont(topMessage.getFont().deriveFont(Font.PLAIN, 12.0f));
        
        topSection = new JPanel();
        topSection.setLayout(new BoxLayout(topSection, BoxLayout.PAGE_AXIS));
        topSection.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        topSection.add(topMessage);
    }

    /*
     * Button Initialization
     */
    private void createButtons() {
        JButton cancelButton = new JButton("Cancel");
        cancelButton.addActionListener(new ButtonActionListener());
        JButton libButton = new JButton("Add Library");
        libButton.addActionListener(new ButtonActionListener());
        JButton clearButton = new JButton("Clear Libraries");
        clearButton.addActionListener(new ButtonActionListener());
        JButton createCFGButton = new JButton("OK");
        createCFGButton.addActionListener(new ButtonActionListener());

        // Panel to hold buttons and file location message
        buttonPane = new JPanel();
        buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
        buttonPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        buttonPane.add(Box.createHorizontalGlue());
        buttonPane.add(cancelButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(libButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(clearButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(createCFGButton);
        buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    }
    
    /*
     * Adds library file
     */
    private void addLibrary(File lib)
    {
    	if (lib != null) {
            CheckNode node = new CheckNode(lib); 
            if (nodes.size() > 0)
                nodes.get(0).add(node);
            nodes.add(node);
            dyn_tree.addObject(node);
        }
    }
    
    /*
     * Removes all check nodes
     */
    public void removeAllNodes()
    {
    	dyn_tree.clear();
        getPreview().setText(null);
        nodes.removeAll(nodes);
        nodes.add(new CheckNode(new File(Constants.PLEXIL_HOME+System.getProperty("file.separator")+"examples"+System.getProperty("file.separator"))));
    }

    private class SelectLibraryListener implements ActionListener
    {
        SelectLibraryListener()
        {
        }
        
        public void actionPerformed(ActionEvent e)
        {
            try {
                removeAllNodes();
                selectLibraries();
                open();
            }
            catch(IOException ex) {
                ex.printStackTrace();
            }
        }
    }

    public ActionListener getSelectLibraryListener()
    {
        return new SelectLibraryListener();
    }

    /*
     * Acquires all library directories from persistent state and loads them
     */
    private void selectLibraries() throws FileNotFoundException
    {
        ArrayList<String> libArr = Luv.getLuv().getExecSelect().getSettings().getLibArray();
        ArrayList<String> uniqArr = new ArrayList<String>();
        boolean dup = false;
        
        for (int i = 0; i<libArr.size();i++)
        	if (!libArr.get(i).equals("")) {
                for (int j=0; j<uniqArr.size(); j++) {
                    if (uniqArr.get(j).equals(libArr.get(i)))
                        dup = true;
                }        		
                if (!dup) {
                    addLibrary(new File(libArr.get(i)));
                    uniqArr.add(libArr.get(i));
                }
            }
    }

    /*
     * Display text for libraries
     */
    private void setPreviewOfLibraries() {
        ArrayList<File> lines = new ArrayList<File>();
        //TODO: Initialize Libraries

        if (!lines.isEmpty()) {
            preview.setText("");
            for (File line : lines) {
                preview.append(line.getAbsolutePath() + "\n");
            }
        } else {
            if (preview != null) {
                preview.setText("");
            }
        }
    }

    /*
     * Allows for future message handling to user
     */
    private String getTopMessage() {
        StringBuffer sb = new StringBuffer();

        sb.append("<html><p align=left>");
        sb.append("<br></br>");
        sb.append("<br></br>");       
        sb.append("</p></html>");

        return sb.toString();
    }
    
    /*
     * Expose preview window
     */
    public JTextArea getPreview(){
    	return preview;
    }
    
    /*
     * Expose library list of directories
     */
    public ArrayList<File> getLibraryList() {
    	File selected = null;    	
    	ArrayList<File> list = new ArrayList<File>();
    	for(CheckNode node : nodes)
    	{
    		selected = (File)node.getUserObject();
    		//if(node.isSelected)
    			list.add(selected);
    	}
    	return list;
    }

    /*
     * Button class for add, cancel, select, clear
     */
    class ButtonActionListener implements ActionListener {    	

        ButtonActionListener() {
        }

        public void actionPerformed(ActionEvent ev) {
            if (ev.getActionCommand().equals("Cancel")) {
                setVisible(false);
            } else if (ev.getActionCommand().equals("Add Library")) {            	
            	JFileChooser fc = new JFileChooser(Constants.PLEXIL_HOME);//new File(Luv.getLuv().getProperty(LoadRecentAction.defineRecentLib(LoadRecentAction.RECENT_DIR)))
            	ExecSelect.PlexilFilter pf = Luv.getLuv().getExecSelect().new PlexilFilter("PLX");
            	if(pf != null)
        		{				
        			fc.addChoosableFileFilter(pf);
        		}	            	
            	int returnVal = fc.showDialog(null, "Choose File");
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    File file = fc.getSelectedFile();
                    addLibrary(file);
                    Luv.getLuv().getStatusMessageHandler().showStatus("Added Library " + file.getAbsolutePath());                   
                }            	            	            	
            } else if (ev.getActionCommand().equals("Clear Libraries")) {
                // verify that the user wants to clear Libraries
                Object[] options = {"Yes", "No"};

                int clear =
                        JOptionPane.showOptionDialog(Luv.getLuv(),
                        "Are you sure you want to clear all Libraries?",
                        "Clear Libraries",
                        JOptionPane.YES_NO_CANCEL_OPTION,
                        JOptionPane.WARNING_MESSAGE,
                        null,
                        options,
                        options[0]);

                if (clear == 0) {
                    // clear text preview area
                	Luv.getLuv().getLibraryLoader().removeAllNodes();                    
                    
                }
            } else if (ev.getActionCommand().equals("OK")) {
            	Luv.getLuv().getLibraryLoader().getPreview().setText("");
                ArrayList<File> parentSelected = new ArrayList<File>();
                
                Iterator<CheckNode> it = Luv.getLuv().getLibraryLoader().nodes.iterator();

                while (it.hasNext()) {
                    CheckNode node = it.next();

                    //if (node.isSelected()) {
                                                
                        if(node.getUserObject() instanceof File)
                        {
                        	File selected = (File)node.getUserObject();
                        	parentSelected.add(selected.getAbsoluteFile());
                        	Luv.getLuv().getLibraryLoader().getPreview().append(selected.getAbsolutePath() + "\n");                        	
                        }                        
                    //}                
                }//end while
                setVisible(false);
            }
        }

    }

    /** Displays the LibraryLoader.  */
    public void open() throws FileNotFoundException
    {
        setVisible(false);
        if (!error) {
            setPreferredSize(Luv.getLuv().getProperties().getDimension(PROP_CFGWIN_SIZE));
            setLocation(Luv.getLuv().getProperties().getPoint(PROP_CFGWIN_LOC));
            pack();
            setVisible(true);
        }
    }

    /** {@inheritDoc} */
    public void itemStateChanged(ItemEvent e) {
        setPreviewOfLibraries();
    }
    
    /*
     * Custom node selection handler
     */
    public class NodeSelectionListener extends MouseAdapter {

        JTree tree;

        NodeSelectionListener(JTree tree) {
            this.tree = tree;
        }

        public void mouseClicked(MouseEvent e) {
            int x = e.getX();
            int y = e.getY();
            int row = tree.getRowForLocation(x, y);
            TreePath path = tree.getPathForRow(row);

            if (path != null) {
                CheckNode node = (CheckNode) path.getLastPathComponent();
                boolean isSelected = !(node.isSelected());
                node.setSelected(isSelected);

                ((DefaultTreeModel) tree.getModel()).nodeChanged(node);

                if (row == 0) {
                    tree.revalidate();
                    tree.repaint();
                }
            }
        }
    }
}
