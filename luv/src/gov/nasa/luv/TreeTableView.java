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

public class TreeTableView extends JTreeTable 
{
    // most recent tree view created stored in case it's needed to set 
    // expansion state of the items in the new tree
    private static TreeTableView currentView;
    private JTree tree;
    // root of tree
    private Wrapper root;
    private static int currentBreakingRow = -1;
    private static int row = -1;
    private static int lastRow = -1;
    
    public TreeTableView(String name, Model model)
    {
         super(new TreeModel(new Wrapper(model)));
         setRowColors(TREE_TABLE_ROW_COLORS);

         tree = getTree();   

         final JTable table = this;

         // add mouse listener which puts up pop-up menus
         addMouseListener(new MouseAdapter()
            {
            @Override
                  public void mousePressed(MouseEvent e)
                  {   
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
         if (currentView != null)
         {
            // regain the expanded state from the old view
            regainExpandedState(
               tree, new TreePath(root),
               currentView.tree, new TreePath(currentView.root));

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
                     Component component = super.getTreeCellRendererComponent(
                        newTree, value, isSelected, expanded, leaf, row, hasFocus);
                     
                     Model model = ((Wrapper)value).model;

                     setIcon(Constants.getIcon(model.getProperty(NODETYPE_ATTR)));
                     
                     Vector<LuvBreakPoint> breakPoints = Luv.getLuv().getLuvBreakPointHandler().getBreakPoints(model);
                     if (breakPoints.size() > 0)
                     {
                        setForeground(lookupColor(MODEL_DISABLED_BREAKPOINTS));
                        for (BreakPoint breakPoint: breakPoints)
                           if (breakPoint.isEnabled())
                           {
                              setForeground(lookupColor(MODEL_ENABLED_BREAKPOINTS));
                           }
                     }
                     
                     setBackgroundNonSelectionColor(
                        isSelected
                        ? table.getSelectionBackground()
                        : getRowColor(row));
                     
                     return component;
                  }
            }); 
    }
    
    // get the current tree table view if it exists.
    public static TreeTableView getCurrent()
    {
        return currentView;
    }
    
    // display node information from under tool tip
    @Override public String getToolTipText(MouseEvent event)
    {
        StringBuffer toolTip  = new StringBuffer();
        TreePath nodePath = tree.getPathForLocation(event.getX(), event.getY());    
          
        if (nodePath != null)
        {    
            Model node = ((Wrapper)nodePath.getLastPathComponent()).model;
            String nodeName = node.getModelName();
          
            toolTip.append("<html>");         
            toolTip.append("<b>NAME</b> " + nodeName);
            toolTip.append("<br><b>TYPE</b>  " + node.getProperty(MODEL_TYPE));
            toolTip.append("<br><hr>");
            toolTip.append("<b>Double-Click</b> on node to view condition information");    
            toolTip.append("<br><b>Right-Click</b> on node to set breakpoints");
        }
          
        return toolTip.length() > 0 ? toolTip.toString() : null;
    }

    // handle a user right click or control click. triggers breakpoint options
    public void handlePopupEvent(MouseEvent mouseEvent)
    {
        if (Luv.getLuv().breaksAllowed())
        {          
           TreePath nodePath = tree.getClosestPathForLocation(mouseEvent.getX(), mouseEvent.getY());
           Model node = ((Wrapper)nodePath.getLastPathComponent()).model;
           JPopupMenu popup = new JPopupMenu();
           
           popup = Luv.getLuv().getLuvBreakPointHandler().constructNodePopupBreakPointMenu(node);
           popup.show(mouseEvent.getComponent(),mouseEvent.getX(), mouseEvent.getY());
        }
    }

    // handle a user double click on a node. triggers node info window
    public void handleClickEvent(MouseEvent mouseEvent)
    {
       TreePath nodePath = tree.getClosestPathForLocation(mouseEvent.getX(), mouseEvent.getY());
       Model node = ((Wrapper)nodePath.getLastPathComponent()).model;
         
       if (node.hasConditions() || node.hasVariables() || node.hasAction())
       {
           Luv.getLuv().getConditionsTab().open(node);
           Luv.getLuv().getVariablesTab().open(node);  
           Luv.getLuv().getActionTab().open(node);
           Luv.getLuv().getNodeInfoWindow().open(node);
       }
       else
       {
           Luv.getLuv().getStatusMessageHandler().showStatus("No additional information is available for " + node.getModelName(), 5000);
       }          
    }

    /** Focus has been disabled for this view.  This way the view
     * doesn't grab up all the key events that it shouldn't.  This is
     * not the best way to acheive this goal.
     *
     * @return good old false, nothing beats false
     */
      
    @Override public boolean isFocusable()
    {
       return false;
    }
    
    public void setPreferredColumnWidths()
    {
        if (getColumnModel().getColumnCount() == 4)
        {
            int state_col = 130;
            int outcome_col = 70;
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

    // if the previous view was the same plan, then set the expanded state 
    // of the new tree to that of the old tree.
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
    
    public void highlightRow(Model node)
    {
        restartSearch();
        int highlight = findNode(node.pathToNode(node));    

        if (highlight > -1)
        {
            currentBreakingRow = highlight;
            tree.setSelectionRow(currentBreakingRow);      
            currentView.setSelectionBackground(Color.PINK);
            scrollToRow(currentBreakingRow);
        }
    }
      
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
        Model node = ((Wrapper)nodePath.getLastPathComponent()).model;
          
        int visible_row = tree.getRowForPath(nodePath);
        int node_number = node.getRowNumber();
          
        tree.setSelectionRow(visible_row);
          
        node_number++;
        visible_row++;
           
        Luv.getLuv().getStatusMessageHandler().showStatusOnBarOnly("Row: " + visible_row + " Node: " + node_number);  
    }
    
    public void restartSearch()
    {
        row = lastRow = -1;
        tree.clearSelection();
    }
      
    public int showNode(Stack<String> node_path, int next)
    {
        findNode(node_path);
        selectRow(row);
        scrollToRow(row);
        return row;
    }
      
    private int findNode(Stack<String> node_path)
    {
        TreePath test = null;
        while (!node_path.empty())
        {
            if (tree.getRowCount() > row + 1)
            {
                String validate = node_path.peek();
                test = tree.getNextMatch(node_path.pop(), row + 1, Position.Bias.Forward); 

                if (node_path.empty() && !test.toString().endsWith(validate + "]"))
                {
                    node_path.push(validate);
                }
                  
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
    
    public boolean selectRow(int row)
    {
        if (row > -1)
        {
            tree.setSelectionRow(row);
            return true;
        }
        
        return false;
    }
      
    public void scrollToRow(int row)
    {
        tree.getAutoscrolls();
        int width = Luv.getLuv().getRootPane().getWidth();
        int height = Luv.getLuv().getRootPane().getHeight();
        int start = this.getRowHeight() * (row - 3);
          
        scrollRectToVisible(new Rectangle(0, start, width, height));
    }
    
    public void expandAllNodes()
    {
        for (int i = 0; i < tree.getRowCount(); i++)
            tree.expandRow(i);
    }
    
    public void collapseAllNodes()
    {
        for (int i = tree.getRowCount() - 1; i >= 0; i--)
            tree.collapseRow(i);
    }

    /** This class wraps a model object and provides a naming and
     * equivelency testing services. 
    */

    public static class Wrapper
      {            
            Model model;
            Vector<Wrapper> children = new Vector<Wrapper>();
            static TreeTableView view;
            static int changed_row;                

            public Wrapper(Model model)
            {
               this.model = model;

               model.addChangeListener(new Model.ChangeAdapter()
                  {
                        public void propertyChange(Model model, String property)
                        {     
                            ((AbstractTableModel)view.getModel()).fireTableDataChanged();
                        }
                  });
                
               addNodesToTree(model); 
            }

            private void addNodesToTree(Model model)
            {
                for (Model child: model.getChildren()) 
                {
                    if (!AbstractModelFilter.isModelFiltered(child))
                    {
                        children.add(new Wrapper(child));
                    }
                    else
                    {
                        addNodesToTree(child);
                    }
                }
            }

            public static void setView(TreeTableView view)
            {
               Wrapper.view = view;
            }

            public String toString()
            {
               return model.getModelName();
            }

            public Model getModel()
            {
               return model;
            }

            public boolean equals(Wrapper other)
            {
               String n1 = model.getModelName();
               String n2 = other.model.getModelName();
               
               if (n1 == null)
                   return false;
               
               if (n2 == null)
                   return false;

               return n1.equals(n2);
            }

            public Vector<Wrapper> getChildren()
            {
               return children;
            }

            public int getChildCount()
            {
               return children.size();
            }

            public Wrapper getChild(int i)
            {
               return children.get(i);
            }
      }

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

            public Object getValueAt(Object node, int column) 
            {
               Model model = ((Wrapper)node).getModel();
               
               if (column == NAME_COL_NUM)
                  return null;

               if (column == STATE_COL_NUM)
                  return model.getProperty(MODEL_STATE);

               if (column == OUTCOME_COL_NUM)
                  return model.getProperty(MODEL_OUTCOME);

               if (column == FAILURE_TYPE_COL_NUM)
                  return model.getProperty(MODEL_FAILURE_TYPE);

               return "<blank>";
            }
      }
}
