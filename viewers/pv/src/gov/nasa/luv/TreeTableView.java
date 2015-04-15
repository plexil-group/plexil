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

import javax.swing.JMenu;
import javax.swing.JSeparator;
import javax.swing.JTree;
import javax.swing.JTable;
import javax.swing.tree.TreePath;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.AbstractTableModel;
import javax.swing.JPopupMenu;
import java.awt.Color;
import java.awt.Component;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseAdapter;
import java.util.Vector;
import java.util.Enumeration;
import java.util.Stack;
import javax.swing.text.Position;
import treetable.TreeTableModel;
import treetable.AbstractTreeTableModel;
import static gov.nasa.luv.Constants.*;
import treetable.JTreeTable;

/**
 * The TreeTableView class provides methods and sub classes to create a tree
 * representation of the Plexil Node.
 */

public class TreeTableView extends JTreeTable 
{
    //
    // Local constants
    //

    /** Represents index of Name column in the Luv application. */     
    private static final int NAME_COL_NUM         = 0;
    /** Represents index of State column in the Luv application. */   
    private static final int STATE_COL_NUM        = 1;
    /** Represents index of Outcome column in the Luv application. */   
    private static final int OUTCOME_COL_NUM      = 2;
    /** Represents index of Failure Type column in the Luv application. */   
    private static final int FAILURE_TYPE_COL_NUM = 3;

    /** Represents column name for the Name column in the Luv application. */   
    private static final String NAME_COL_NAME         = "Name";
    /** Represents column name for the State column in the Luv application. */
    private static final String STATE_COL_NAME        = "State";
    /** Represents column name for the Outcome column in the Luv application. */
    private static final String OUTCOME_COL_NAME      = "Outcome";
    /** Represents column name for the Failure Type column in the Luv application. */
    private static final String FAILURE_TYPE_COL_NAME = "Failure Type";      

    // most recent tree view created stored in case it's needed to set 
    // expansion state of the items in the new tree
    private static TreeTableView currentView;
    private JTree tree;
    // root of tree
    private Wrapper root;
    // *** FIXME: variables used in several different routines ***
    private int currentBreakingRow = -1;
    private int row = -1;
    private int lastRow = -1;
    
    /**
     * Constructs a TreeTableView representation for the specified Plexil Node.
     * 
     * @param node the Plexil Node
     */
    public TreeTableView(Node node) {
        super(new TreeModel(new Wrapper(node)));
        setRowColors(TREE_TABLE_ROW_COLORS);
        tree = getTree();   
        final JTable table = this;

        // add mouse listener which puts up pop-up menus
        addMouseListener(new MouseAdapter() {
                @Override
                public void mousePressed(MouseEvent e) {
                    if (e.isPopupTrigger())
                        handlePopupEvent(e);
                    else if (e.getClickCount() == 2)
                        handleClickEvent(e);
                     
                    showRowInStatusBar(e);
                }
            });         
           
        root = (Wrapper)tree.getModel().getRoot();
        Wrapper.setView(this);

        // manage handles
        tree.setShowsRootHandles(true);

        // if there is an old version of this view
        if (currentView != null) {
            // regain the expanded state from the old view
            regainExpandedState(tree,
                                new TreePath(root),
                                currentView.tree,
                                new TreePath(currentView.root));

            // set the column widths to the old view
            setPreferredColumnWidths();
        }
         
        currentView = this;

        // setup the column coloring logic
        setDefaultRenderer(
                           TreeModel.cTypes[STATE_COL_NUM], 
                           new DefaultTableCellRenderer()
                           {
                               public Component getTableCellRendererComponent(
                                                                              JTable table, 
                                                                              Object value, 
                                                                              boolean isSelected, 
                                                                              boolean hasFocus, 
                                                                              int row, 
                                                                              int column)
                               {
                                   Component component = super.getTableCellRendererComponent(
                                                                                             table, value, isSelected, hasFocus, row, column);
                     
                                   String label = (String)value;
                                   Color color = Constants.lookupColor(label);
                                   setForeground(color != null
                                                 ? color
                                                 : Color.BLACK);
                     
                                   setBackground(isSelected ? table.getSelectionBackground() : getRowColor(row));  

                                   return component;
                               }
                           });

        // set cell renderer to customize icons
        tree.setCellRenderer(new DefaultTreeCellRenderer()
            {
                public Component getTreeCellRendererComponent(
                                                              JTree newTree,
                                                              Object value,
                                                              boolean isSelected,
                                                              boolean expanded,
                                                              boolean leaf,
                                                              int row,
                                                              boolean hasFocus) 
                {
                    Component component =
                        super.getTreeCellRendererComponent(newTree, value, isSelected, expanded, leaf, row, hasFocus);
                     
                    Node node = ((Wrapper)value).node;

                    // select icon depending on if in extended or core view
                    if (Luv.getLuv().getIsExtendedViewOn())
                        setIcon(Constants.getIcon(node.getProperty(NODETYPE_ATTR)));
                    else
                        setIcon(Constants.getIcon(node.getProperty(NODETYPE_ATTR_PLX)));
                     
                    Vector<LuvBreakPoint> breakPoints = node.getBreakPoints();
                    if (!breakPoints.isEmpty()) {
                        setForeground(lookupColor(NODE_DISABLED_BREAKPOINTS));
                        for (LuvBreakPoint breakPoint: breakPoints)
                            if (breakPoint.isEnabled())
                                setForeground(lookupColor(NODE_ENABLED_BREAKPOINTS));
                    }
                     
                    setBackgroundNonSelectionColor(isSelected
                                                   ? table.getSelectionBackground()
                                                   : getRowColor(row));
                     
                    return component;
                }
            }); 
    }
    
    /**
     * Returns the current TreeTableView.
     * 
     * @return the current TreeTableView
     */
    public static TreeTableView getCurrent()
    {
        return currentView;
    }
    
    /**
     * Display node information from under tool tip for the specified event.
     * 
     * @param event represents the Node under the tool tip
     * @return string representation of the tool tip or null
     */
    @Override
    public String getToolTipText(MouseEvent event) {

        TreePath nodePath = tree.getPathForLocation(event.getX(), event.getY());    
        if (nodePath == null)
            return null;

        Node node = ((Wrapper)nodePath.getLastPathComponent()).node;
        StringBuilder toolTip  = new StringBuilder();
        toolTip.append("<html>");         
        toolTip.append("<b>NAME</b> ");
        toolTip.append(node.getNodeName());
        toolTip.append("<br><b>TYPE</b>  ");
        toolTip.append(node.getProperty(NODE_TYPE));
        toolTip.append("<br><hr>");
        toolTip.append("<b>Double-Click</b> on node to view condition information");    
        toolTip.append("<br><b>Right-Click</b> on node to set breakpoints");
          
        return toolTip.toString();
    }

    /**
     * Handles a right or control click and triggers breakpoint options
     * for the specified MouseEvent.
     * 
     * @param mouseEvent the right or control click event
     */
    public void handlePopupEvent(MouseEvent mouseEvent)
    {
        if (Luv.getLuv().breaksAllowed())
        {          
           TreePath nodePath = tree.getClosestPathForLocation(mouseEvent.getX(), mouseEvent.getY());
           Node node = ((Wrapper)nodePath.getLastPathComponent()).node;
           JPopupMenu popup = new JPopupMenu();
           
           popup = constructNodePopupBreakPointMenu(node);
           popup.show(mouseEvent.getComponent(),mouseEvent.getX(), mouseEvent.getY());
        }
    }

    /**
     * Handles a double click and triggers a NodeInfoWindow to open
     * for the specified MouseEvent.
     * 
     * @param mouseEvent the double click event
     */
    public void handleClickEvent(MouseEvent mouseEvent)
    {
       TreePath nodePath = tree.getClosestPathForLocation(mouseEvent.getX(), mouseEvent.getY());
       Node node = ((Wrapper)nodePath.getLastPathComponent()).node;
         
       if (node.hasConditions() || node.hasVariables() || node.hasAction()) {
           ConditionsTab.open(node);
           VariablesTab.open(node);
           ActionTab.open(node);
           NodeInfoWindow.open(node);
       }
       else {
           Luv.getLuv().getStatusMessageHandler().showStatus("No additional information is available for " + node.getNodeName(), 5000);
       }          
    }

    /** 
     * Focus has been disabled for this view.  This way the view
     * doesn't grab up all the key events that it shouldn't.  This is
     * not the best way to acheive this goal.
     *
     * @return false
     */
      
    @Override public boolean isFocusable()
    {
       return false;
    }
    
    private void setPreferredColumnWidths()
    {
        if (getColumnModel().getColumnCount() == 4)
        {
            int state_col = 130;
            int outcome_col = 130;
            int failure_col = 230;
            int name_col = Luv.getLuv().getRootPane().getWidth() - 
                    (state_col + outcome_col + failure_col);

            Enumeration<TableColumn> columns = getColumnModel().getColumns();
            columns.nextElement().setPreferredWidth(name_col);   
            columns.nextElement().setPreferredWidth(state_col);
            columns.nextElement().setPreferredWidth(outcome_col);
            columns.nextElement().setPreferredWidth(failure_col);
        }
        else
            Luv.getLuv().getStatusMessageHandler().displayErrorMessage(null, 
                                                                       "Error: Invalid number of columns in viewer." );
    }

    /**
     * Handles situations where if the previous view was the same plan, it sets 
     * the expanded state of the new tree to that of the old tree.
     * 
     * @param t1 first tree
     * @param tp1 first tree path
     * @param t2 second tree
     * @param tp2 second tree path
     * @return false if different views
     */
    public boolean regainExpandedState(JTree t1, TreePath tp1, JTree t2, TreePath tp2)
    {
        // get the tree nodes for the paths
        Wrapper w1 = (Wrapper)tp1.getLastPathComponent();
        Wrapper w2 = (Wrapper)tp2.getLastPathComponent();

        // if the child count differs, then we should stop right here,
        // the two trees are different
        if (w1.getChildCount() != w2.getChildCount())
           return false;

        // if the names differ, then we should stop right here, the two
        // trees are different
        if (!w1.equals(w2))
           return false;

        // traverse down the tree
        for (int i = 0; i < w1.getChildCount(); ++i)
        {
           if (!regainExpandedState(
                  t1, tp1.pathByAddingChild(w1.getChild(i)),
                  t2, tp2.pathByAddingChild(w2.getChild(i))))
              return false;
        }
         
        // set the expanded state on the new tree to mach the old tree
        if (t2.isExpanded(tp2))
            t1.expandPath(tp1);
        else
            t1.collapsePath(tp1);

        // return true, both trees are the same thus far
        return true;
    }
    
    /**
     * Highlights the row that a breakpoint is stopped at. Not the best way to do
     * this. It selects the row and then sets the selection background to pink.
     * FIXME:
     * This causes problems if the user manually selects a row in the tree view
     * while the plan is executing because then that selected row will turn pink
     * even though its not a break point.
     * 
     * @param node the node at which the breakpoint is stopped at
     */
    public void highlightRow(Node node)
    {
        restartSearch();
        int highlight = findNode(node.pathToNode(node));    

        if (highlight > -1)
        {
            currentBreakingRow = highlight;
            tree.setSelectionRow(currentBreakingRow);      
            currentView.setSelectionBackground(Color.PINK); // *** FIXME ***
        }
    }
      
    /**
     * Unhighlights the current breaking row.
     */
    public void unHighlightRow()
    {
        if (currentBreakingRow >= 0)
        {
            currentView.setSelectionBackground(getRowColor(currentBreakingRow));
            tree.setSelectionRow(-1);
        }
    }
    
    private void showRowInStatusBar(MouseEvent mouseEvent)
    {
        TreePath nodePath = tree.getClosestPathForLocation(mouseEvent.getX(), mouseEvent.getY());
        Node node = ((Wrapper)nodePath.getLastPathComponent()).node;
          
        int visible_row = tree.getRowForPath(nodePath);
        int node_number = node.getRowNumber();
          
        tree.setSelectionRow(visible_row);
          
        node_number++;
        visible_row++;
           
        Luv.getLuv().getStatusMessageHandler().showStatusOnBarOnly("Row: " + visible_row + " Node: " + node_number);  
    }
    
    /**
     * Restarts the search through the tree to the beginning of the tree.
     */
    // *** FIXME: sets global variables row, lastRow ***
    public void restartSearch()
    {
        row = lastRow = -1;
        tree.clearSelection();
    }
    
    /**
     * Scrolls to and selects the Node from the specified Node path.
     * 
     * @param node_path the path to the node starting from the root node
     * @return the row that is currently being shown
     */
    public int showNode(Stack<String> node_path)
    {
        findNode(node_path);
        selectRow(row);
        scrollToRow(row);
        return row;
    }
      
    // *** FIXME: sets global variables row, lastRow ***
    private int findNode(Stack<String> node_path) {
        TreePath test = null;
        while (!node_path.empty()) {
            if (tree.getRowCount() > row + 1) {
                String validate = node_path.peek();
                test = tree.getNextMatch(node_path.pop(), row + 1, Position.Bias.Forward); 

                if (node_path.empty() && !test.toString().endsWith(validate + "]"))
                    node_path.push(validate);
                  
                row = tree.getRowForPath(test);    
                      
                if (!node_path.empty())
                    tree.expandPath(test); 
                else if (row == lastRow)
                    node_path.push(validate);
            }
            else
                test = tree.getNextMatch(node_path.pop(), 0, Position.Bias.Forward);                          
        } 
          
        lastRow = tree.getRowForPath(test);
        return row;
    }
    
    /**
     * Selects the specified row in the TreeTableView of the Plexil Node.
     * 
     * @param row the row to select
     * @return whether or not the row was found
     */
    public boolean selectRow(int row)
    {
        if (row > -1)
        {
            tree.setSelectionRow(row);
            return true;
        }
        
        return false;
    }
      
    /**
     * Scrolls to the specified row in the TreeTableView of the Plexil Node.
     * 
     * @param row the row to scroll to
     */
    public void scrollToRow(int row)
    {
        tree.getAutoscrolls();
        int width = Luv.getLuv().getRootPane().getWidth();
        int height = Luv.getLuv().getRootPane().getHeight();
        int start = this.getRowHeight() * (row - 3);
          
        scrollRectToVisible(new Rectangle(0, start, width, height));
    }
    
    /**
     * Expands all the nodes of this TreeTableView.
     */
    public void expandAllNodes() {
        for (int i = 0; i < tree.getRowCount(); i++)
            tree.expandRow(i);
    }
    
    /**
     * Collapses all the node of this TreeTableView.
     */
    public void collapseAllNodes()
    {
        for (int i = tree.getRowCount() - 1; i >= 0; i--)
            tree.collapseRow(i);
    }
      
    /**
     * Construct a node popup menu when the user right-clicks on the specified 
     * Plexil node/node.
     * 
     * @param node the Plexil node the user wants to set a LuvBreakPoint on
     * @return the pop up menu
     */
    public JPopupMenu constructNodePopupBreakPointMenu(final Node node)
    {
        // get the node name
        final String name = node.getNodeName();

        // construct the node popup menu         
        JPopupMenu popup = new JPopupMenu("Node Popup Menu");
         
        // get the break points for this node       
        final Vector<LuvBreakPoint> bps = node.getBreakPoints();

        // add node state change breakpoint
        popup.add(new LuvAction("Add Break Point for " + name + " State Change",
                                "Add a break point any time " + name + " changes state.")
            {
                public void actionPerformed(ActionEvent e) {
                    node.createChangeBreakpoint();
                }
            });
         
        // add target state break points menu
        JMenu stateMenu = new JMenu(
                                    "Add Break Point for " + name + " State");
        stateMenu.setToolTipText(
                                 "Add a break point which pauses execution when " + name + 
                                 " reaches a specified state.");
        popup.add(stateMenu);
        for (final String state: NODE_STATES)
            stateMenu.add(new LuvAction(state,
                                        "Add a break point when " + name + 
                                        " reaches the " + state + " state.")
                {
                    public void actionPerformed(ActionEvent e) {
                        node.createNodeStateValueBreakpoint(state);
                    }
                });

        // add target outcome break points menu
        JMenu outcomeMenu = new JMenu(
                                      "Add Break Point for " + name + " Outcome");
        outcomeMenu.setToolTipText(
                                   "Add a break point which pauses execution when " + name + 
                                   " reaches a specified outcome.");
        popup.add(outcomeMenu);
        for (final String outcome: NODE_OUTCOMES)
            outcomeMenu.add(new LuvAction(
                                          outcome,
                                          "Add a break point when " + name + 
                                          " reaches the " + outcome + " outcome.")
                {
                    public void actionPerformed(ActionEvent e) {
                        node.createNodeOutcomeValueBreakpoint(outcome);
                    }
                }); 

        // add target failure type break points menu
        JMenu failureTypeMenu = new JMenu(
                                          "Add Break Point for " + name + " Failure Type");
        failureTypeMenu.setToolTipText(
                                       "Add a break point which pauses execution when " + name + 
                                       " reaches a specified failure type.");
        popup.add(failureTypeMenu);
        for (final String failureType: NODE_FAILURE_TYPES)
            failureTypeMenu.add(new LuvAction(
                                              failureType,
                                              "Add a break point when " + name + 
                                              " reaches the " + failureType +
                                              " failure type.")
                {
                    public void actionPerformed(ActionEvent e)
                    {
                        node.createNodeFailureValueBreakpoint(failureType);
                    }
                }); 

        // if we got any add enable/disable & remove item for each one
        //if (bps.size() > 0 && !Luv.getLuv().getIsExecuting())
        if (!bps.isEmpty()) {
            // add the breakpoints
            popup.add(new JSeparator());
            for (final LuvBreakPoint bp: bps) {
                String action = bp.isEnabled() ? "Disable" : "Enable";
                popup.add(new LuvAction(action + " " + bp,
                                        action + " the breakpoint " + 
                                        bp + ".") {
                        public void actionPerformed(ActionEvent e) {
                            bp.setEnabled(!bp.isEnabled());
                            if (!bp.isEnabled())
                                bp.reserveBreakStatus(true);
                            Luv.getLuv().getViewHandler().refreshView();
                        }
                    }); 
            }

            // add the breakpoints
            popup.add(new JSeparator());
            for (final LuvBreakPoint bp2: bps) {
                popup.add(new LuvAction("Remove " + bp2,
                                        "Permanently remove the breakpoint " + 
                                        bp2 + ".") {
                        public void actionPerformed(ActionEvent e) {
                            bp2.unregister();
                        }
                    }); 
            }
        }

        // if there is more then one break point add a remove all item
        if (bps.size() > 1) {
            // add the remove all action           
            popup.add(new JSeparator());
            popup.add(new LuvAction(
                                    "Remove All Break Points From " + name,
                                    "Permanently remove all breakpoints from " + name + ".")
                {
                    public void actionPerformed(ActionEvent e) {
                        for (final LuvBreakPoint bp3: bps)
                            bp3.unregister();
                    }
                });
        }
               
        Luv.getLuv().updateBlockingMenuItems();

        // return our freshly created popup menu
        return popup;
    }

    /**
     * The Wrapper class wraps a Plexil Node object and provides naming and
     * equivelency testing services. 
     */
    public static class Wrapper {            
        Node node;
        Vector<Wrapper> children;
        static TreeTableView view;
        static int changed_row;                

        /**
         * Constructs a Wrapper with the specified Node.
         * 
         * @param node the node to wrap
         */
        public Wrapper(Node node)
        {
            this.node = node;
            children = new Vector<Wrapper>();
            node.addChangeListener(new Node.ChangeAdapter() {
                    public void propertyChange(Node node, String property) {
                        ((AbstractTableModel)view.getModel()).fireTableDataChanged();
                    }
                });
            addChildrenToTree(node);
        }

        private void addChildrenToTree(Node node) {
            if (!node.hasChildren())
                return;
            for (Node child: node.getChildren()) {
                if (!AbstractNodeFilter.isNodeFiltered(child))
                    children.add(new Wrapper(child));
                else
                    addChildrenToTree(child);
            }
        }

        /**
         * Sets this Wrapper's view to the specified TreeTableView.
         * 
         * @param view the TreeTableView
         */
        public static void setView(TreeTableView view) {
            Wrapper.view = view;
        }

        /**
         * Returns the name of the Node.
         * 
         * @return the name of the Node
         */
        public String toString() {
            return node.getNodeName();
        }

        /**
         * Returns the Node.
         *
         * @return the Node
         */
        public Node getNode() {
            return node;
        }

        /**
         * Tests whether the specified Wrapper is equivalent to this Wrapper.
         * 
         * @param other the other Wrapper to test with
         * @return whether the specified Wrapper is equivalent to this Wrapper
         */
        public boolean equals(Wrapper other) {
            String n1 = node.getNodeName();
            String n2 = other.node.getNodeName();
               
            if (n1 == null)
                return false;
               
            if (n2 == null)
                return false;

            return n1.equals(n2);
        }

        /**
         * Returns the Vector of Wrapper children for this Wrapper.
         * 
         * @return the Vector of Wrapper children
         */
        public Vector<Wrapper> getChildren() {
            return children;
        }

        /**
         * Returns the number of Wrapper children for this Wrapper.
         * 
         * @return the number of Wrapper children
         */
        public int getChildCount() {
            return children.size();
        }

        /**
         * Returns the specified Wrapper child for this Wrapper.
         * 
         * @param i the index of a Wrapper child
         * @return the Wrapper child
         */
        public Wrapper getChild(int i) {
            return children.get(i);
        }
    }

    /**
     * The TreeModel class provides methods for what to title the columns
     * and what types are displayed in each column of the TreeTableView and
     * returns values stored in the columns.
     */
    public static class TreeModel extends AbstractTreeTableModel<Object> 
    {
        // column names          
        static String[]  cNames = 
        {
            NAME_COL_NAME,
            STATE_COL_NAME,
            OUTCOME_COL_NAME,
            FAILURE_TYPE_COL_NAME,            
        };
            
        // column types
        static Class[]  cTypes = 
        {
            TreeTableModel.class,
            String.class, 
            String.class, 
            String.class,                
        };

        public TreeModel(Wrapper model)
        {
            super(model);
        }

        public int getChildCount(Object node) 
        { 
            return ((Wrapper)node).getChildCount();
        }
            
        public Object getChild(Object node, int i) 
        {
            return ((Wrapper)node).getChild(i);
        }

        public boolean isLeaf(Object node)
        {
            return ((Wrapper)node).getChildCount() == 0;
        }
            
        public int getColumnCount() 
        {
            return cNames.length;
        }
            
        public String getColumnName(int column) 
        {
            return cNames[column];
        }
            
        public Class getColumnClass(int column) 
        {
            return cTypes[column];
        }

        public Object getValueAt(Object node, int column) {
            Node theNode = ((Wrapper)node).getNode();
            String result = null;
            switch (column) {
            case NAME_COL_NUM:
                return null;

            case STATE_COL_NUM:
                // State should never be null
                return theNode.getState();

            case OUTCOME_COL_NUM:
                if ((result = theNode.getOutcome()) == null)
                    return UNKNOWN;
                else
                    return result;

            case FAILURE_TYPE_COL_NUM:
                if ((result = theNode.getFailureType()) == null)
                    return UNKNOWN;
                else
                    return result;

            default:
                return "<blank>";
            }
        }
    }
}
