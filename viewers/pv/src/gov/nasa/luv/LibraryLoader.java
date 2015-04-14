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
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
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
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTree;
import javax.swing.SwingConstants;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;

import static java.awt.GridBagConstraints.*;

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

    private FileListPanel pathList;
    private ArrayList<CheckNode> nodes;
    private JTextArea preview;
    private JTree main_tree;
    private DynamicTree dyn_tree;
    private Vector<File> libraryPath;
    private Vector<File> libraryFiles;
    private Vector<String> libraryNames;
    private GridBagLayout layout;

    /** Construct a LibraryLoader. 
     */
    public LibraryLoader() {
        super("Libraries");

        layout = new GridBagLayout();
        Container pane = getContentPane();
        pane.setLayout(layout);

        createPathListSection();
        createCheckListSection();
        createPreviewSection();
    }

    private void createPathListSection() {
        JLabel pathHeading = new JLabel("Library Search Path", SwingConstants.CENTER);
        GridBagConstraints headingConstraints = new GridBagConstraints();
        headingConstraints.gridx = 0;
        headingConstraints.gridy = 0;
        headingConstraints.fill = HORIZONTAL;
        layout.setConstraints(pathHeading, headingConstraints);
        add(pathHeading);

        pathList = new FileListPanel();
        GridBagConstraints listConstraints = new GridBagConstraints();
        listConstraints.gridx = 0;
        listConstraints.gridy = 1;
        listConstraints.weightx = 1.0;
        listConstraints.weighty = 1.0;
        listConstraints.fill = BOTH;
        layout.setConstraints(pathList, listConstraints);
        add(pathList);

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

        JButton dirButton = new JButton("Add Directory");
        dirButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) { 
                    File dflt = pathList.getLast();
                    if (dflt == null) {
                        File planLoc = Luv.getLuv().getSettings().getPlanLocation();
                        if (planLoc != null)
                            dflt = planLoc.getParentFile();
                        else
                            dflt = new File(System.getenv("PWD")); // *** use homedir instead? ***
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

                    if (clear == 0)
                        pathList.clearFiles();
                }
            }
            );

        JPanel buttonPane = new JPanel();
        buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
        buttonPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        buttonPane.add(Box.createHorizontalGlue());
        buttonPane.add(dirButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(clearDirsButton);
        buttonPane.add(Box.createHorizontalGlue());
        GridBagConstraints buttonConstraints = new GridBagConstraints();
        buttonConstraints.gridx = 0;
        buttonConstraints.gridy = 2;
        buttonConstraints.fill = HORIZONTAL;
        layout.setConstraints(buttonPane, buttonConstraints);
        add(buttonPane);
    }

    /*
     * Initializes checklist and tree
     */
    private void createCheckListSection() {
        JLabel heading = new JLabel("Libraries", SwingConstants.CENTER);
        GridBagConstraints headingConstraints = new GridBagConstraints();
        headingConstraints.gridx = 1;
        headingConstraints.gridy = 0;
        headingConstraints.fill = HORIZONTAL;
        layout.setConstraints(heading, headingConstraints);
        add(heading);

        nodes = new ArrayList<CheckNode>();
        dyn_tree = new DynamicTree();
        GridBagConstraints treeConstraints = new GridBagConstraints();
        treeConstraints.gridx = 1;
        treeConstraints.gridy = 1;
        treeConstraints.weightx = 1.0;
        treeConstraints.weighty = 1.0;
        treeConstraints.fill = BOTH;
        layout.setConstraints(dyn_tree, treeConstraints);
        add(dyn_tree);

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
        
        JButton clearButton = new JButton("Clear Libraries");
        clearButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    // *** TODO ***
                }
            }
            );

        JPanel buttonPane = new JPanel();
        buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
        buttonPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        buttonPane.add(Box.createHorizontalGlue());
        buttonPane.add(libButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(clearButton);
        buttonPane.add(Box.createHorizontalGlue());

        GridBagConstraints buttonConstraints = new GridBagConstraints();
        buttonConstraints.gridx = 1;
        buttonConstraints.gridy = 2;
        buttonConstraints.fill = HORIZONTAL;
        layout.setConstraints(buttonPane, buttonConstraints);
        add(buttonPane);

        // TODO: Bypass setting default if user specs library dirs on cmd line
        // File defaultPath = Constants.PLEXIL_EXAMPLES_DIR;
        // addLibrary(defaultPath);
    }

    /*
     * Builds display window for loaded libraries
     */
    private void createPreviewSection() {
        JLabel heading = new JLabel("Preview", SwingConstants.CENTER);
        GridBagConstraints headingConstraints = new GridBagConstraints();
        headingConstraints.gridx = 2;
        headingConstraints.gridy = 0;
        headingConstraints.fill = HORIZONTAL;
        layout.setConstraints(heading, headingConstraints);
        add(heading);

        preview = new JTextArea();
        setPreviewOfLibraries();
        preview.setEditable(false);
        JScrollPane previewArea = new JScrollPane(preview);
        GridBagConstraints previewConstraints = new GridBagConstraints();
        previewConstraints.gridx = 2;
        previewConstraints.gridy = 1;
        previewConstraints.weightx = 1.0;
        previewConstraints.weighty = 1.0;
        previewConstraints.fill = BOTH;
        layout.setConstraints(previewArea, previewConstraints);
        add(previewArea);

        JButton cancelButton = new JButton("Cancel");
        cancelButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    setVisible(false);
                }
            }
            );

        JButton createCFGButton = new JButton("OK");
        createCFGButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    preview.setText("");
                    ArrayList<File> parentSelected = new ArrayList<File>();
                    Iterator<CheckNode> it = nodes.iterator();
                    while (it.hasNext()) {
                        CheckNode node = it.next();
                        //if (node.isSelected()) {
                        if (node.getUserObject() instanceof File) {
                            File selected = (File)node.getUserObject();
                            parentSelected.add(selected.getAbsoluteFile());
                            preview.append(selected.getAbsolutePath() + "\n");
                        }
                        //}
                    }//end while
                    setVisible(false);
                }
            }
            );

        // Panel to hold buttons and file location message
        JPanel buttonPane = new JPanel();
        buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
        buttonPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        buttonPane.add(Box.createHorizontalGlue());
        buttonPane.add(cancelButton);
        buttonPane.add(Box.createHorizontalStrut(3));
        buttonPane.add(createCFGButton);
        buttonPane.add(Box.createHorizontalGlue());

        GridBagConstraints buttonConstraints = new GridBagConstraints();
        buttonConstraints.gridx = 2;
        buttonConstraints.gridy = 2;
        buttonConstraints.fill = HORIZONTAL;
        layout.setConstraints(buttonPane, buttonConstraints);
        add(buttonPane);
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
        preview.setText(null);
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
     * Expose preview window
     */
    public JTextArea getPreview() {
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
        setPreferredSize(Luv.getLuv().getSettings().getDimension(PROP_CFGWIN_SIZE));
        setLocation(Luv.getLuv().getSettings().getPoint(PROP_CFGWIN_LOC));
        pack();
        setVisible(true);
    }

    /** {@inheritDoc} */
    public void itemStateChanged(ItemEvent e) {
        setPreviewOfLibraries();
    }

}
