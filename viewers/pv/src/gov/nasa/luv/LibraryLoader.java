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

import static gov.nasa.luv.Constants.AppType.*;
import static gov.nasa.luv.Constants.PROP_CFGWIN_LOC;
import static gov.nasa.luv.Constants.PROP_CFGWIN_SIZE;

import java.awt.BorderLayout;
import java.awt.Container;
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
import java.util.Collection;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.filechooser.FileFilter;
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

public class LibraryLoader
    extends JFrame
    implements ItemListener {

    private static String topMessage = null; // *** FIXME ***
    private static boolean error; // *** FIXME ***
    private JPanel topSection;
    private FileListPanel pathList;
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
     */
    public LibraryLoader() {
        super("Libraries");
        error = false;
        createPathList();
        createCheckList();

        if (!error) {
            createPreviewArea();
            createTopSection();
            createButtons();

            Container pane = getContentPane();
            pane.add(topSection, BorderLayout.NORTH);
            pane.add(pathList, BorderLayout.WEST);
            pane.add(checkBoxList, BorderLayout.CENTER);
            pane.add(dyn_tree, BorderLayout.CENTER);
            pane.add(previewArea, BorderLayout.EAST);
            pane.add(buttonPane, BorderLayout.SOUTH);
        }
    }

    private void createPathList() {
        pathList = new FileListPanel();
        pathList.setPreferredSize(new Dimension(450, 50)); // *** FIXME ***

        Settings s = Luv.getLuv().getSettings();
        Collection<File> dirs = s.getLibDirs();
        if (!dirs.isEmpty()) {
            pathList.setFiles(dirs);
        }
        else if (s.getPlanLocation() != null) {
            pathList.addFile(s.getPlanLocation().getParentFile());
        }
        // else { // choose a sane default
        // }
    }

    /*
     * Initializes checklist and tree
     */
    private void createCheckList() {
        nodes = new ArrayList<CheckNode>();
        dyn_tree = new DynamicTree();
                
        // place check boxes into check box tree        
        checkBoxList = new JScrollPane(dyn_tree);
        checkBoxList.setPreferredSize(new Dimension(450, 50));

        // TODO: Bypass setting default if user specs library dirs on cmd line
        // File defaultPath = Constants.PLEXIL_EXAMPLES_DIR;
        // addLibrary(defaultPath);
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
        cancelButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    setVisible(false);
                }
            }
            );
        JButton dirButton = new JButton("Add Directory");
        dirButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) { 
                    File dflt = pathList.getLast();
                    if (dflt == null) {
                        File planLoc = Luv.getLuv().getSettings().getPlanLocation();
                        if (planLoc != null) {
                            dflt = planLoc.getParentFile();
                        }
                        else {
                            dflt = new File(System.getenv("PWD")); // *** use homedir instead? ***
                        }
                    }
                    
                    JFileChooser dc = new JFileChooser(dflt);
                    dc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
                    FileFilter df =
                        new FileFilter() {
                            public boolean accept(File f) {
                                return f.isDirectory();
                            }
                            public String getDescription() {
                                return "Select a directory";
                            }
                        };
                    if (dc.showDialog(null, "Choose This Directory") ==
                        JFileChooser.APPROVE_OPTION) {
                        File dir = dc.getSelectedFile();
                        pathList.addFile(dir);
                        Luv.getLuv().getStatusMessageHandler().showStatus("Added Library Directory" + dir.getAbsolutePath());
                    }
                }
            }
            );

        JButton libButton = new JButton("Add Library");
        libButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) { 
                    JFileChooser fc = new JFileChooser(Constants.PLEXIL_HOME); // *** FIXME ***
                    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
                    FileFilter pf = Luv.getLuv().getExecSelectDialog().getPlanFileFilter();
                    if (pf != null)
                        fc.addChoosableFileFilter(pf);
                    int returnVal = fc.showDialog(null, "Choose File");
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        File file = fc.getSelectedFile();
                        addLibrary(file);
                        Luv.getLuv().getStatusMessageHandler().showStatus("Added Library " + file.getAbsolutePath());
                    }            	            	            	
                }
            }
            );
        
        JButton clearDirsButton = new JButton("Clear Directories");
        clearDirsButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    // verify that the user wants to clear Libraries
                    Object[] options = {"Yes", "No"};
                    int clear =
                        JOptionPane.showOptionDialog(Luv.getLuv(),
                                                     "Are you sure you want to clear all Library directories?",
                                                     "Clear Path",
                                                     JOptionPane.YES_NO_CANCEL_OPTION,
                                                     JOptionPane.WARNING_MESSAGE,
                                                     null,
                                                     options,
                                                     options[0]);

                    if (clear == 0) {
                        pathList.clearFiles();
                    }
                }
            }
            );

        JButton clearButton = new JButton("Clear Libraries");
        clearButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                }
            }
            );
        
        JButton createCFGButton = new JButton("OK");
        createCFGButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    getPreview().setText("");
                    ArrayList<File> parentSelected = new ArrayList<File>();
                    Iterator<CheckNode> it = nodes.iterator();
                    while (it.hasNext()) {
                        CheckNode node = it.next();
                        //if (node.isSelected()) {
                        if (node.getUserObject() instanceof File) {
                            File selected = (File)node.getUserObject();
                            parentSelected.add(selected.getAbsoluteFile());
                            getPreview().append(selected.getAbsolutePath() + "\n");
                        }
                        //}
                    }//end while
                    setVisible(false);
                }
            }
            );

        // Panel to hold buttons and file location message
        buttonPane = new JPanel();
        buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
        buttonPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        buttonPane.add(Box.createHorizontalGlue());
        buttonPane.add(cancelButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(dirButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(libButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(clearDirsButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(clearButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(createCFGButton);
        buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    }
    
    /*
     * Adds library file
     */
    private void addLibrary(File lib) {
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
    public void removeAllNodes() {
    	dyn_tree.clear();
        getPreview().setText(null);
        nodes.removeAll(nodes);
        nodes.add(new CheckNode(Constants.PLEXIL_EXAMPLES_DIR));
    }

    // Called by ExecSelectDialog
    // *** FIXME ***
    public void openDialog() {
        try {
            removeAllNodes();
            selectLibraries();
            open();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    // *** FIXME ***
    private void selectLibraries() throws FileNotFoundException {
        ArrayList<String> uniqArr = new ArrayList<String>();
        for (String lib: Luv.getLuv().getSettings().getLibs())
        	if (!lib.isEmpty()) { // FIXME: why isn't this an error?
                boolean dup = false;
                for (String loaded : uniqArr)
                    if (loaded.equals(lib)) {
                        dup = true;
                        break;
                    }
                if (!dup) {
                    // *** FIXME ***
                    addLibrary(new File(lib));
                    uniqArr.add(lib);
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
    private static String getTopMessage() {
        if (topMessage == null) {
            StringBuilder sb = new StringBuilder();
            sb.append("<html><p align=left>");
            sb.append("<br></br>");
            sb.append("<br></br>");       
            sb.append("</p></html>");
            topMessage = sb.toString();
        }
        return topMessage;
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
    	for(CheckNode node : nodes) {
            selected = (File)node.getUserObject();
            //if(node.isSelected)
            list.add(selected);
        }
    	return list;
    }

    /** Displays the LibraryLoader.  */
    public void open() throws FileNotFoundException {
        setVisible(false);
        if (!error) {
            setPreferredSize(Luv.getLuv().getSettings().getDimension(PROP_CFGWIN_SIZE));
            setLocation(Luv.getLuv().getSettings().getPoint(PROP_CFGWIN_LOC));
            pack();
            setVisible(true);
        }
    }

    /** {@inheritDoc} */
    public void itemStateChanged(ItemEvent e) {
        setPreviewOfLibraries();
    }

}
