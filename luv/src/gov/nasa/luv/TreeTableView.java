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
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseAdapter;

import java.util.ArrayList;
import java.util.Vector;
import java.util.Enumeration;

import treetable.TreeTableModel;
import treetable.AbstractTreeTableModel;

import static gov.nasa.luv.Constants.*;
import static java.awt.event.KeyEvent.*;

import treetable.JTreeTable;

public class TreeTableView extends JTreeTable implements View
{
      /** This is the most recent tree view created stored in case it's
       * needed to set expansion state of the items in the new tree */

      private static TreeTableView lastView;
      
      private static ArrayList<String> path = new ArrayList<String>();

      /** swing tree object */

      private JTree tree;

      /** root of tree */

      private Wrapper root;

      /** show or hid node types */

      private static boolean showTextTypes = false;

      /** Construct a tree view. 
       *
       * @param name the name of the view
       * @param model model this view is viewing
       */

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
                  }
            });
         
         root = (Wrapper)tree.getModel().getRoot();
         Wrapper.setView(this);

         // manage handles

         tree.setShowsRootHandles(true);

         // if there is an old version of this view

         if (lastView != null)
         {
            // regain the expand stat from the old view

            regainExpandedState(
               tree, new TreePath(root),
               lastView.tree, new TreePath(lastView.root));

            // set the column widths to the old view

            setPreferredColumnWidths(lastView.getColumnWidths());
         }
         lastView = this;

         // setup the column coloring logic

         setDefaultRenderer(
            TreeModel.cTypes[STATE_COL_NUM], 
            new DefaultTableCellRenderer()
            {
            @Override
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
                     
                     
                     setBackground(
                        isSelected
                        ? table.getSelectionBackground()
                        : getRowColor(row));
                     
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

                     setIcon(
                        !showTextTypes 
                        ? Constants.getIcon(model.getProperty(NODETYPE_ATTR))
                        : null);
                     
                     Vector<BreakPoint> breakPoints = Luv.getLuv().getLuvBreakPointHandler().getBreakPoints(model);
                     if (breakPoints.size() > 0)
                     {
                        setForeground(lookupColor(MODEL_DISABLED_BREAKPOINTS));
                        for (BreakPoint breakPoint: breakPoints)
                           if (breakPoint.isEnabled())
                              setForeground(lookupColor(MODEL_ENABLED_BREAKPOINTS));
                     }
                     
                     setBackgroundNonSelectionColor(
                        isSelected
                        ? table.getSelectionBackground()
                        : getRowColor(row));

                     return component;
                  }
            });
      }

      /** Display node information from under tool tip
       * 
       * @return tooltip text for element in tree
       */

      @Override public String getToolTipText(MouseEvent event)
      {
          StringBuffer   toolTip     = new StringBuffer();
          TreePath       nodePath    = tree.getPathForLocation(event.getX(), event.getY());          
          
          if (nodePath != null)
          {    
              Model          node        = ((Wrapper)nodePath.getLastPathComponent()).model;
              String         nodeName    = node.getModelName();
          
              toolTip.append("<html>");         
              toolTip.append("<b>NAME</b> " + nodeName);
              toolTip.append("<br><b>TYPE</b> " + node.getProperty(MODEL_TYPE));
          }
          
          return toolTip.length() > 0 ? toolTip.toString() : null;
      }

      /** Ensure that this views properties are propertly reflected in
       * the luv properties.
       *
       * @param properties the luv properties
       */

      public void getViewProperties(Properties properties)
      {
         // handle column widths 

         int i = 1;

         Vector<Integer> widths = new Vector<Integer>();
         while (properties.exists(PROP_TTV_COL_WIDTH_BASE + i))
            widths.add(properties.getInteger(PROP_TTV_COL_WIDTH_BASE + i++));
         
         setPreferredColumnWidths(widths);

         // handle text/icon node types

         showTextTypes = properties.getBoolean(PROP_TTV_TEXT_TYPES);
      }

      /** Allow this view to extract properties from the luv properties.
       *
       * @param properties the luv properties
       */

      public void setViewProperties(Properties properties)
      {
         int i = 1;
         for (int width: getColumnWidths())
            properties.set(PROP_TTV_COL_WIDTH_BASE + i++, width);

         properties.set(PROP_TTV_TEXT_TYPES, showTextTypes);
      }

      /** Handle popup menu event.
       *
       * @param mouseEvent event which triggered popup menu
       * @param luv current active luv
       */

      public void handlePopupEvent(MouseEvent mouseEvent)
      {
          if (Luv.getLuv().breaksAllowed())
          {          
             TreePath   nodePath  = tree.getClosestPathForLocation(mouseEvent.getX(), mouseEvent.getY());
             Model      node      = ((Wrapper)nodePath.getLastPathComponent()).model;
             JPopupMenu popup = new JPopupMenu();

             // construct the popup menu

             popup = Luv.getLuv().getLuvBreakPointHandler().constructNodePopupBreakPointMenu(node);

             // display the popup menu

             popup.show(mouseEvent.getComponent(),mouseEvent.getX(), mouseEvent.getY());
          }
      }
      
      public ArrayList<String> getPathToInfoWindowNode()
      {
          return path;
      }

      /** Handle popup menu event.
       *
       * @param mouseEvent event which triggered popup menu
       * @param luv current active luv
       */

      public void handleClickEvent(MouseEvent mouseEvent)
      {
         TreePath   nodePath    = tree.getClosestPathForLocation(mouseEvent.getX(), mouseEvent.getY());
         Model      node        = ((Wrapper)nodePath.getLastPathComponent()).model;
         String     nodeName    = node.getModelName();
         
         // save pop up node window information 
         
         savePopUpWindowNodeInfo(node, nodePath); 
         
         // create conditons window if possible
         
         if (!openConditionsWindow(node, nodeName))
             Luv.getLuv().showStatus("No conditions specified for " + nodeName, Color.BLACK, 1000);
             
      }
      
      public boolean openConditionsWindow(Model node, String nodeName)
      {
          boolean valid = true;         
          
          if (node.conditionMap != null && !node.conditionMap.isEmpty())
              ConditionsWindow.createAndShowGUI(node, nodeName + " Conditions");
          else
              valid = false;             
          
          return valid;
      }
      
      public void savePopUpWindowNodeInfo(Model node, TreePath tp)
      {
         path.clear();
         path.add(node.getModelName());

         while (!node.isRoot())
         {
             path.add(node.getParent().getModelName());
             node = node.getParent();
         } 
         
      }
      
      public boolean isNodeInfoWindowOpen()
      {
          return ConditionsWindow.isConditionsWindowOpen(); 
      }
      
      public void resetNodeInfoWindow(Model model, String name)
      {
          ConditionsWindow.resetGUI(model, name);
      }
      
      public void closeNodeInfoWindow()
      {
          ConditionsWindow.closeConditonsWindow();
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

      /** Get the current tree table view if it exists.
       *
       * @return current tree table view, or null if it does not exist
       */

      public static TreeTableView getCurrent()
      {
         return lastView;
      }

      /** Get the current column widths.
       *
       * @return a vector containing the column widths
       */

      public Vector<Integer> getColumnWidths()
      {
         Vector<Integer> widths = new Vector<Integer>();

         Enumeration<TableColumn> columns = getColumnModel().getColumns();
         while (columns.hasMoreElements())
            widths.add(columns.nextElement().getWidth());
         return widths;
      }

      /** Set Preferred the current column widths.
       *
       * @param widths a vector containing the column widths
       */

      public void setPreferredColumnWidths(Vector<Integer> widths)
      {
         Enumeration<TableColumn> columns = getColumnModel().getColumns();
         for (int width: widths)
            columns.nextElement().setPreferredWidth(width);
      }

      /** If the previous view was the same plan, then set the expanded
       * state of the new tree to that of the old tree.
       *
       * @param t1  original tree
       * @param tp1 current path in original tree
       * @param t2  new tree
       * @param tp2 current path in new tree
       * @return true if the two plans are the same
       */
      
      public boolean regainExpandedState(JTree t1, TreePath tp1, 
                                         JTree t2, TreePath tp2)
      {
         // get the tree nodes for the paths

         Wrapper w1 =
            (Wrapper)tp1.getLastPathComponent();
         Wrapper w2 =
            (Wrapper)tp2.getLastPathComponent();

         // if the child count differs, then we should stop right here,
         // the two trees are differnt

          if (w1.getChildCount() != w2.getChildCount())
             return false;

          // if the names differ, then we should stop right here, the two
          // trees are differnt

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

      /** This class wraps a model object and provides a naming and
       * equivelency testing services. 
       */

      public static class Wrapper
      {
            int row;
            int col;
            Model model;
            Vector<Wrapper> children = new Vector<Wrapper>();
            static TreeTableView view;

            public Wrapper(Model model)
            {
               this.model = model;

               model.addChangeListener(new Model.ChangeAdapter()
                  {
                        public void propertyChange(Model model, String property)
                        {
                            ((AbstractTableModel)view.getModel())
                               .fireTableDataChanged();
                        }
                  });

               for (Model child: model.getChildren()) {
                  if (!AbstractModelFilter.isModelFiltered(child))
                     children.add(new Wrapper(child));
	       }
            }


            public static void setView(TreeTableView view)
            {
               Wrapper.view = view;
            }

            public String toString()
            {
               return showTextTypes
                     ? model.getProperty(MODEL_TYPE) + " " + model.getModelName()
                     : model.getModelName();
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

      public static class TreeModel 
         extends AbstractTreeTableModel<Object> 
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

            //
            //  The TreeTableNode interface. 
            //
            
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

      /** get this view */

      public View getView()
      {
         return this;
      }
      /** Expand all nodes. */

      public void expandAllNodes()
      {
         for (int i = 0; i < tree.getRowCount(); i++)
            tree.expandRow(i);
      }

      /** Collapse all nodes. */

      public void collapseAllNodes()
      {
         for (int i = 0; i < tree.getRowCount(); i++)
            tree.collapseRow(i);
      }

      /** Return an array of specalized actions for this view.
       *
       * @return an array of specalized actions for this view.
       */

      public LuvAction[] getViewActions()
      {
         LuvAction[] actions =
            {
               expandAll,
               collapseAll,
               showNodeType,
            };
         return actions;
      }
      
      /** Action to fully expand tree. */

      LuvAction expandAll = new LuvAction(
         "Expand All", "Expand all tree nodes.", VK_EQUALS)
         {
               public void actionPerformed(ActionEvent e)
               {
                  expandAllNodes();
               }
         };

      /** Action to fully collapse tree. */

      LuvAction collapseAll = new LuvAction(
         "Collapse All", "Collapse all tree nodes.", VK_MINUS)
         {
               public void actionPerformed(ActionEvent e)
               {
                  collapseAllNodes();
               }
         };

      /** Action to fully collapse tree. */

      LuvAction showNodeType = new LuvAction(
         "Toggle Node Text Types", "Show node types as icons or text.",
         VK_N, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  showTextTypes = !showTextTypes;
                  tree.setLargeModel(true);
                  repaint();
               }
         };
}
