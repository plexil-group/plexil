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
import javax.swing.SwingConstants;

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
    extends JFrame {

    private FileListPanel pathList;
    private FileListPanel libList;
    private Vector<File> libraryPath;
    private Vector<File> libraryFiles;
    private GridBagLayout layout;

    /** Construct a LibraryLoader. 
     */
    public LibraryLoader() {
        super("Plan Libraries");

        layout = new GridBagLayout();
        Container pane = getContentPane();
        pane.setLayout(layout);

        createPathListSection();
        createLibListSection();
        createButtonSection();

        setPreferredSize(Luv.getLuv().getSettings().getDimension(PROP_CFGWIN_SIZE));
        setLocation(Luv.getLuv().getSettings().getPoint(PROP_CFGWIN_LOC));
        pack();
    }

    private void createPathListSection() {
        Box heading = Box.createHorizontalBox();
        heading.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

        JLabel label = new JLabel("Library Search Path", SwingConstants.LEFT);
        heading.add(label);
        heading.add(Box.createHorizontalGlue());
        JButton dirButton = new JButton("Add");
        dirButton.setToolTipText("Insert directory after selection, or at top if none selected");
        dirButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) { 
                    File dflt = pathList.getSelection();
                    if (dflt == null) {
                        File planLoc = Luv.getLuv().getSettings().getPlanLocation();
                        if (planLoc != null)
                            dflt = planLoc.getParentFile();
                        else
                            dflt = new File(System.getenv("HOME"));
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
                        pathList.insertAfterSelection(dir);
                        pathList.setSelection(dir);
                        Luv.getLuv().getStatusMessageHandler().showStatus("Added Library Directory " + dir.getAbsolutePath());
                    }
                }
            }
            );
        heading.add(dirButton);
        heading.add(Box.createHorizontalStrut(3));

        JButton removeButton = new JButton("Remove");
        removeButton.setToolTipText("Remove selection from search path");
        removeButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    if (pathList.getSelectionIndex() >= 0) {
                        // Confirm that the user wants to clear Libraries
                        Object[] options = {"OK", "Cancel"};
                        int del =
                            JOptionPane.showOptionDialog(LibraryLoader.this,
                                                         "Remove " + pathList.getSelection().toString() + " from path?",
                                                         "Confirm Remove From Path",
                                                         JOptionPane.OK_CANCEL_OPTION,
                                                         JOptionPane.WARNING_MESSAGE,
                                                         null,
                                                         options,
                                                         options[1]);

                        if (del == 0)
                            pathList.removeSelection();
                    }
                }
            }
            );
        heading.add(removeButton);
        heading.add(Box.createHorizontalStrut(3));

        JButton clearDirsButton = new JButton("Clear");
        clearDirsButton.setToolTipText("Clear library search path");
        clearDirsButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    Object[] options = {"OK", "Cancel"};
                    int clear =
                        JOptionPane.showOptionDialog(LibraryLoader.this,
                                                     "Are you sure you want to clear all Library directories?",
                                                     "Clear Path",
                                                     JOptionPane.OK_CANCEL_OPTION,
                                                     JOptionPane.WARNING_MESSAGE,
                                                     null,
                                                     options,
                                                     options[1]);

                    if (clear == 0)
                        pathList.clearFiles();
                }
            }
            );
        heading.add(clearDirsButton);

        GridBagConstraints headingConstraints = new GridBagConstraints();
        headingConstraints.gridx = 0;
        headingConstraints.gridy = 0;
        headingConstraints.fill = HORIZONTAL;
        layout.setConstraints(heading, headingConstraints);
        add(heading);

        pathList = new FileListPanel();
        GridBagConstraints listConstraints = new GridBagConstraints();
        listConstraints.gridx = 0;
        listConstraints.gridy = 1;
        listConstraints.weightx = 1.0;
        listConstraints.weighty = 1.0;
        listConstraints.fill = BOTH;
        layout.setConstraints(pathList, listConstraints);
        add(pathList);
    }

    /*
     * Initializes checklist and tree
     */
    private void createLibListSection() {
        Box heading = Box.createHorizontalBox();
        heading.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

        JLabel label = new JLabel("Libraries", SwingConstants.LEFT);
        heading.add(label);
        heading.add(Box.createHorizontalGlue());

        JButton libButton = new JButton("Add");
        libButton.setToolTipText("Insert library after selection, or at top if none selected");
        libButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) { 
                    File dflt = libList.getSelection();
                    if (dflt == null) {
                        File planLoc = Luv.getLuv().getSettings().getPlanLocation();
                        if (planLoc != null)
                            dflt = planLoc.getParentFile();
                        else
                            dflt = new File(System.getenv("HOME"));
                    }
                    JFileChooser fc = new JFileChooser(dflt);
                    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
                    FileFilter pf = Luv.getLuv().getExecSelectDialog().getPlanFileFilter();
                    if (pf != null)
                        fc.addChoosableFileFilter(pf);
                    int returnVal = fc.showDialog(null, "Choose Library File");
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        File lib = fc.getSelectedFile();
                        libList.insertAfterSelection(lib);
                        libList.setSelection(lib);
                        Luv.getLuv().getStatusMessageHandler().showStatus("Added Library File " + lib.getAbsolutePath());
                    }            	            	            	
                }
            }
            );
        heading.add(libButton);
        heading.add(Box.createHorizontalStrut(3));

        JButton removeButton = new JButton("Remove");
        removeButton.setToolTipText("Remove selected library");
        removeButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    if (libList.getSelectionIndex() >= 0) {
                        // Confirm that the user wants to clear Libraries
                        Object[] options = {"OK", "Cancel"};
                        int del =
                            JOptionPane.showOptionDialog(LibraryLoader.this,
                                                         "Remove library" + libList.getSelection().toString() + "?",
                                                         "Confirm Remove Library",
                                                         JOptionPane.OK_CANCEL_OPTION,
                                                         JOptionPane.WARNING_MESSAGE,
                                                         null,
                                                         options,
                                                         options[1]);

                        if (del == 0)
                            libList.removeSelection();
                    }
                }
            }
            );
        heading.add(removeButton);
        heading.add(Box.createHorizontalStrut(3));
        
        JButton clearButton = new JButton("Clear");
        clearButton.setToolTipText("Clear libraries");
        clearButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ev) {
                    Object[] options = {"OK", "Cancel"};
                    int clear =
                        JOptionPane.showOptionDialog(LibraryLoader.this,
                                                     "Are you sure you want to clear all libraries?",
                                                     "Confirm Clear Libraries",
                                                     JOptionPane.OK_CANCEL_OPTION,
                                                     JOptionPane.WARNING_MESSAGE,
                                                     null,
                                                     options,
                                                     options[1]);

                    if (clear == 0)
                        libList.clearFiles();
                }
            }
            );
        heading.add(clearButton);

        GridBagConstraints headingConstraints = new GridBagConstraints();
        headingConstraints.gridx = 1;
        headingConstraints.gridy = 0;
        headingConstraints.fill = HORIZONTAL;
        layout.setConstraints(heading, headingConstraints);
        add(heading);

        libList = new FileListPanel();
        GridBagConstraints libConstraints = new GridBagConstraints();
        libConstraints.gridx = 1;
        libConstraints.gridy = 1;
        libConstraints.weightx = 1.0;
        libConstraints.weighty = 1.0;
        libConstraints.fill = BOTH;
        layout.setConstraints(libList, libConstraints);
        add(libList);
    }

    private void createButtonSection() {
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
                    Settings s = Luv.getLuv().getSettings();
                    s.setLibDirs(pathList.getFiles());
                    // A little more involved with libraries
                    ArrayList<File> libs = libList.getFiles();
                    s.setLibs(new ArrayList<String>(libs.size()));
                    for (File f : libs)
                        s.addLib(f.getAbsolutePath());
                    // s.save(); // *** NO, DON'T! *** Let ExecSelectDialog do it
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
        buttonConstraints.gridx = 0;
        buttonConstraints.gridy = 2;
        buttonConstraints.gridwidth = REMAINDER;
        buttonConstraints.fill = HORIZONTAL;
        layout.setConstraints(buttonPane, buttonConstraints);
        add(buttonPane);
    }

    // Update contents from current settings
    private void refresh() {
        Settings s = Luv.getLuv().getSettings();
        File planDir = null;
        if (s.getPlanLocation() != null)
            planDir = s.getPlanLocation().getParentFile();
        else
            planDir = new File(System.getenv("HOME"));

        Collection<File> dirs = s.getLibDirs();
        if (!dirs.isEmpty())
            pathList.setFiles(dirs);
        else if (s.getPlanLocation() != null)
            // Try to set sane default for path
            pathList.addFile(planDir);
        else
            // no default
            pathList.clearFiles();

        // Use whatever libraries we have
        Collection<String> libNames = s.getLibs();
        ArrayList<File> libFiles = new ArrayList<File>(libNames.size());
        for (String name : libNames)
            libFiles.add(new File(planDir, name)); // parse relative to plan/homedir
        libList.setFiles(libFiles);
    }


    // Called by ExecSelectDialog
    // *** FIXME ***
    public void openDialog() {
        try {
            open();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    /** Displays the LibraryLoader.  */
    public void open() throws FileNotFoundException {
        setVisible(false);
        refresh();
        setVisible(true);
    }

}
