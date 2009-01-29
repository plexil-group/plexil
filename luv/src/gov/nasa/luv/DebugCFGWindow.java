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
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTree;
import javax.swing.UIManager;
import javax.swing.plaf.ColorUIResource;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;

import java.io.PrintStream;
import java.io.FileOutputStream;

import static gov.nasa.luv.Constants.*;


public class DebugCFGWindow extends JFrame 
{
    private static DebugCFGWindow frame;
    private CheckNode[] nodes;
            
    public DebugCFGWindow() {}
    
    public DebugCFGWindow(String title) 
    {
        super(title);
        
        assignLinesToCheckNodes(); 
        JTree tree = new JTree(nodes[0]);
        tree.setCellRenderer(new CheckRenderer());
        tree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        tree.putClientProperty("JTree.lineStyle", "Angled");
        tree.addMouseListener(new NodeSelectionListener(tree));       
        JScrollPane checkBoxList = new JScrollPane(tree);

        JTextArea textArea = new JTextArea(7,10);       
        JScrollPane textPanel = new JScrollPane(textArea);
        
        JButton cancelButton = new JButton("Close");        
        cancelButton.addActionListener(new ButtonActionListener(nodes[0], textArea));
        JButton createCFGButton = new JButton("Create CFG file");        
        createCFGButton.addActionListener(new ButtonActionListener(nodes[0], textArea));        
        JPanel buttonPane = new JPanel();
        buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.LINE_AXIS));
        buttonPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
        buttonPane.add(Box.createHorizontalGlue());
        buttonPane.add(cancelButton);
        buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
        buttonPane.add(createCFGButton);
        
        getContentPane().add(checkBoxList,    BorderLayout.NORTH);
        getContentPane().add(textPanel,    BorderLayout.CENTER);
        getContentPane().add(buttonPane, BorderLayout.SOUTH);
    }
    
    private void assignLinesToCheckNodes()
    {
        HashMap<String, String> lines = Luv.getLuv().getDebugDataFileProcessor().getLines();
        nodes = new CheckNode[lines.size() + 1];

        // root node
        nodes[0] = new CheckNode("Check All");       
        
        // first tier == A
        for (int a = 0; a < lines.size(); a++)
        {
            if (lines.containsKey("A" + a))
            {
                nodes[a + 1] = new CheckNode(lines.get("A" + a));
                nodes[0].add(nodes[a + 1]);

                // second tier == AB
                for (int b = 0; b < lines.size(); b++)
                {
                    if (lines.containsKey("A" + a + "B" + b))  
                    {
                        nodes[a + b + 2] = new CheckNode(lines.get("A" + a + "B" + b));
                        nodes[a + 1].add(nodes[a + b + 2]);

                        // third tier == ABC  
                        for (int c = 0; c < lines.size(); c++)
                        {
                            if (lines.containsKey("A" + a + "B" + b + "C" + c))  
                            {
                                nodes[a + b + c + 3] = new CheckNode(lines.get("A" + a + "B" + b + "C" + c));
                                nodes[a + b + 2].add(nodes[a + b + c + 3]);     
                            }
                        }
                    }
                }
            }
        }
    } 

    class NodeSelectionListener extends MouseAdapter 
    {
        JTree tree;

        NodeSelectionListener(JTree tree) 
        {
            this.tree = tree;
        }

        public void mouseClicked(MouseEvent e) 
        {
            int x = e.getX();
            int y = e.getY();
            int row = tree.getRowForLocation(x, y);
            TreePath  path = tree.getPathForRow(row);
            
            if (path != null) 
            {
                CheckNode node = (CheckNode)path.getLastPathComponent();
                boolean isSelected = ! (node.isSelected());
                node.setSelected(isSelected);
                
                if (isSelected)
                    tree.expandPath(path);
                else
                    tree.collapsePath(path);                 
                
                
                ((DefaultTreeModel) tree.getModel()).nodeChanged(node);

                if (row == 0) 
                {
                    tree.revalidate();
                    tree.repaint();
                }
            }
        }
    }

    class ButtonActionListener implements ActionListener 
    {
        CheckNode root;
        
        JTextArea textArea;

        ButtonActionListener(final CheckNode root, final JTextArea textArea) 
        {
            this.root = root;
            this.textArea = textArea;
        }

        public void actionPerformed(ActionEvent ev) 
        {
            if (ev.getActionCommand().equals("Close"))
            {
                frame.setVisible(false);
            }
            else if (ev.getActionCommand().equals("Create CFG file"))
            {
                textArea.setText(null);
                ArrayList<String> parentSelected = new ArrayList<String>();
                Enumeration e = root.breadthFirstEnumeration();

                while (e.hasMoreElements()) 
                {
                    CheckNode node = (CheckNode) e.nextElement();
                    TreeNode[] nodePath = node.getPath();

                    // if the Check all box is selected
                    if (node.isSelected && node.toString().equals("Check All"))
                    {
                        appendAllFirstTierNodes(e);
                        break;
                    }

                    if (node.isSelected() && !parentSelected(nodePath, parentSelected)) 
                    {
                        textArea.append(node.toString() + "\n");
                        parentSelected.add(node.toString());
                    }
                }
                
                writeToDebugCFGFile(textArea);
            }
        }
        
        private void appendAllFirstTierNodes(Enumeration e)
        {
            int numOfFirstTiers = root.getChildCount();
        
            for (int i = 0; i < numOfFirstTiers; i++)
            {
                CheckNode node = (CheckNode) e.nextElement();
                textArea.append(node.toString() + "\n");
            }
        }
        
        private boolean parentSelected(TreeNode[] nodePath, ArrayList<String> parentSelected)
        {
            if (parentSelected == null || parentSelected.size() == 0)
                return false;

            for (int i = 0; i < parentSelected.size(); i++)
            {
                for (int j = 0; j < nodePath.length; j++)
                {
                    if (nodePath[j].toString().endsWith(parentSelected.get(i)))
                        return true;
                }               
            }

            return false;
        }
        
        private void writeToDebugCFGFile(JTextArea textArea)
        {
            FileOutputStream out; 
            PrintStream p; 

            try
            {
                out = new FileOutputStream(DEBUG_CFG_FILE);
                p = new PrintStream( out );
                p.println (textArea.getText());
                p.close();
            }
            catch (Exception e)
            {
                System.err.println ("Error writing to file");
            }
        }   
    }   

    public void openWindow() 
    {
        try 
        {
            UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
        } 
        catch (Exception evt) {}
        
        if (frame != null && frame.isVisible())
            frame.setVisible(false);

        frame = new DebugCFGWindow("Create Debug.cfg File");
        
        frame.addWindowListener(new WindowAdapter() 
        {
            public void windowClosing(WindowEvent e) 
            {
                // clear holder of check nodes when close window
                ;
            }
        });
        
        frame.setPreferredSize(Luv.getLuv().getProperties().getDimension(PROP_CFGWIN_SIZE));
        frame.setLocation(Luv.getLuv().getProperties().getPoint(PROP_CFGWIN_LOC));
        frame.pack();
        frame.setVisible(true);
    }
}
class CheckRenderer extends JPanel implements TreeCellRenderer 
{
    protected JCheckBox check;

    protected TreeLabel label;

    public CheckRenderer() 
    {
        setLayout(null);
        add(check = new JCheckBox());
        add(label = new TreeLabel());
        check.setBackground(UIManager.getColor("Tree.textBackground"));
        label.setForeground(UIManager.getColor("Tree.textForeground"));
    }

    public Component getTreeCellRendererComponent(JTree tree, 
                                                  Object value,
                                                  boolean isSelected, 
                                                  boolean expanded, 
                                                  boolean leaf, 
                                                  int row,
                                                  boolean hasFocus) 
    {
        String stringValue = tree.convertValueToText(value, 
                                                     isSelected,
                                                     expanded, 
                                                     leaf, 
                                                     row, 
                                                     hasFocus);
        setEnabled(tree.isEnabled());
        check.setSelected(((CheckNode) value).isSelected());
        label.setFont(tree.getFont());
        label.setText(stringValue);
        label.setSelected(isSelected);
        label.setFocus(hasFocus);

        if (leaf) 
            label.setIcon(UIManager.getIcon("Tree.leafIcon"));
        else if (expanded) 
            label.setIcon(UIManager.getIcon("Tree.openIcon"));
        else 
            label.setIcon(UIManager.getIcon("Tree.closedIcon"));

        return this;
    }

    public Dimension getPreferredSize() 
    {
        Dimension d_check = check.getPreferredSize();
        Dimension d_label = label.getPreferredSize();

        return new Dimension(d_check.width + d_label.width,
        (d_check.height < d_label.height ? d_label.height
        : d_check.height));
    }

    public void doLayout() 
    {
        Dimension d_check = check.getPreferredSize();
        Dimension d_label = label.getPreferredSize();
        int y_check = 0;
        int y_label = 0;

        if (d_check.height < d_label.height) 
            y_check = (d_label.height - d_check.height) / 2;
        else
            y_label = (d_check.height - d_label.height) / 2;

        check.setLocation(0, y_check);
        check.setBounds(0, y_check, d_check.width, d_check.height);
        label.setLocation(d_check.width, y_label);
        label.setBounds(d_check.width, y_label, d_label.width, d_label.height);
    }

    public void setBackground(Color color) 
    {
        if (color instanceof ColorUIResource)
            color = null;

        super.setBackground(color);
    }

    public class TreeLabel extends JLabel 
    {
        boolean isSelected;

        boolean hasFocus;

        public TreeLabel() {}

        public void setBackground(Color color) 
        {
            if (color instanceof ColorUIResource)
                color = null;

            super.setBackground(color);
        }

        public void paint(Graphics g) 
        {
            String str;

            if ((str = getText()) != null) 
            {
                if (0 < str.length()) 
                {
                    if (isSelected)                        
                        g.setColor(UIManager.getColor("Tree.selectionBackground"));                        
                    else 
                        g.setColor(UIManager.getColor("Tree.textBackground"));

                    Dimension d = getPreferredSize();
                    int imageOffset = 0;
                    Icon currentI = getIcon();

                    if (currentI != null)
                        imageOffset = currentI.getIconWidth() + Math.max(0, getIconTextGap() - 1);

                    g.fillRect(imageOffset, 0, d.width - 1 - imageOffset, d.height);

                    if (hasFocus) 
                    {
                        g.setColor(UIManager.getColor("Tree.selectionBorderColor"));
                        g.drawRect(imageOffset, 0, d.width - 1 - imageOffset, d.height - 1);
                    }
                }
            }

            super.paint(g);
        }

        public Dimension getPreferredSize() 
        {
            Dimension retDimension = super.getPreferredSize();

            if (retDimension != null)
                retDimension = new Dimension(retDimension.width + 3, retDimension.height);

            return retDimension;
        }

        public void setSelected(boolean isSelected) 
        {
            this.isSelected = isSelected;
        }

        public void setFocus(boolean hasFocus) 
        {
            this.hasFocus = hasFocus;
        }
    }
}

class CheckNode extends DefaultMutableTreeNode 
{
    public final static int EXPANDED_SELECTION = 4;

    protected int selectionMode;

    protected boolean isSelected;

    public CheckNode() 
    {
        this(null);
    }

    public CheckNode(Object userObject) 
    {
        this(userObject, true, false);
    }

    public CheckNode(Object userObject, 
                     boolean allowsChildren,
                     boolean isSelected) 
    {
        super(userObject, allowsChildren);
        this.isSelected = isSelected;
        selectionMode = EXPANDED_SELECTION;
    }

    public int getSelectionMode() 
    {
        return selectionMode;
    }

    public void setSelected(boolean isSelected) 
    {
        this.isSelected = isSelected;

        if (children != null) 
        {
            Enumeration e = children.elements();

            while (e.hasMoreElements()) 
            {
                CheckNode node = (CheckNode) e.nextElement();
                node.setSelected(isSelected);
            }
        }
    }

    public boolean isSelected() 
    {
        return isSelected;
    }
}

