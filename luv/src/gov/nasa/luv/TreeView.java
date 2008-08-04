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

import javax.swing.Icon;
import javax.swing.JTree;
import javax.swing.JPanel;
import javax.swing.ImageIcon;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;

import java.util.HashMap;

import static gov.nasa.luv.Constants.*;
import static java.awt.event.KeyEvent.*;

public class TreeView extends JPanel implements View
{
      /** This is the most recent tree view created stored in case it's
       * needed to set expansion state of the items in the new tree */

      private static TreeView lastView;

      /** The name of this view. */

      String name = null;

      /** the luv application */
      
      Luv luv = null;

      /** show node types */

      static boolean showTypes = true;

      /** swing tree object */

      JTree tree;

      /** root of tree */

      MutableTreeNode root;

      /** Construct a tree view. 
       *
       * @param name the name of the view
       * @param luv active luv object
       * @param model model this view is viewing
       */

      public TreeView(String name, Luv luv, Model model)
      {
         // record name a luv

         this.name = name;
         this.luv = luv;
         
         // set layout

         setLayout(new BorderLayout());

         // create tree

         root = createNode(model.findChild(NODE));
         tree = new JTree(root);
         add(tree);

         // set show root handle

         tree.setShowsRootHandles(true);

         // if there was an older tree view see if we can regain the
         // expanded state fo the tree

         if (lastView != null)
            regainExpandedState(
               tree, new TreePath(root),
               lastView.tree, new TreePath(lastView.root));
         lastView = this;

         // set background color

         tree.setBackground(luv.properties.getColor(PROP_WIN_BCLR));

         // eliminate all icons
         
         tree.setCellRenderer(new DefaultTreeCellRenderer()
            {
                  {
                     setLeafIcon(null);
                     setOpenIcon(null);
                     setClosedIcon(null);
                  }
            });
      }

      /** Ensure that this views properties are propertly reflected in
       * the luv properties.
       *
       * @param properties the luv properties
       */

      public void getViewProperties(Properties properties)
      {
      }

      /** Allow this view to extract properties from the luv properties.
       *
       * @param properties the luv properties
       */

      public void setViewProperties(Properties properties)
      {
      }

      /** If the preveouse view was the same plan, then set the expanded
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

         DefaultMutableTreeNode dmtn1 =
            (DefaultMutableTreeNode)tp1.getLastPathComponent();
         DefaultMutableTreeNode dmtn2 =
            (DefaultMutableTreeNode)tp2.getLastPathComponent();

         // if the child count differs, then we should stop right here,
         // the two trees are differnt

         if (dmtn1.getChildCount() != dmtn2.getChildCount())
            return false;

         // get the model objects from the nodes

         Wrapper w1 = (Wrapper)dmtn1.getUserObject();
         Wrapper w2 = (Wrapper)dmtn2.getUserObject();

         // if the names differ, then we should stop right here, the two
         // trees are differnt

         if (!w1.equals(w2))
            return false;

         // traverse down the tree

         for (int i = 0; i < dmtn1.getChildCount(); ++i)
         {
            if (!regainExpandedState(
                   t1, tp1.pathByAddingChild(dmtn1.getChildAt(i)),
                   t2, tp2.pathByAddingChild(dmtn2.getChildAt(i))))
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

      /** Return the contained tree */

      public JTree getTree()
      {
         return tree;
      }

      /**
       * Create a tree from a given model.
       *
       * @param model model to create tree from
       * @return the new tree to be viewed
       */

      public MutableTreeNode createNode(final Model model)
      {
         // create wrapper and tree node which holds it

         final Wrapper wrapper = new Wrapper(model);
         final DefaultMutableTreeNode node 
            = new DefaultMutableTreeNode(wrapper);

         // add a change listener so events show update live

         model.addChangeListener(new Model.ChangeAdapter()
            {
                  DefaultTreeModel treeModel;

                  public void propertyChange(Model model, String property)
                  {
                     wrapper.establishName();
                     if (treeModel == null)
                        treeModel = (DefaultTreeModel)getTree().getModel();
                     treeModel.nodeChanged(node);
                  }
             });

         // add all the children nodes (think of the children)
         
         for (Model child: model.getChildren())
            node.add(createNode(child));

         // return our newly minted node

         return node;
      }

      /** This class wraps a model object and provides a naming and
       * equivelency testing services. 
       */

      public static class Wrapper
      {
            String name;
            Model model;

            public Wrapper(Model model)
            {
               this.model = model;
               establishName();
            }

            public Model getModel()
            {
               return model;
            }

            public boolean equals(Wrapper other)
            {
               String n1 = model.getProperty(MODEL_NAME);
               String n2 = other.model.getProperty(MODEL_NAME);

               return n1.equals(n2);
            }

            public void establishName()
            {
               String name = model.getProperty(MODEL_NAME);
               String type = model.getProperty(MODEL_TYPE);
               String state = model.getProperty(MODEL_STATE);
               String outcome = model.getProperty(MODEL_OUTCOME);
               
               if (name == null)
                  name = model.getType();
               else if (showTypes && type != null)
                  name = type + " " + name + " " + state + " " + outcome;
               
               this.name = name;
            }

            public String toString()
            {
               return name;
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

      /** Action show node types in different ways. */

      LuvAction showNodeType = new LuvAction(
         "Show/Hide Type", "Show or hide types of nodes in the tree view.", 
         VK_T, META_MASK)
         {
               public void actionPerformed(ActionEvent e)
               {
                  showTypes = !showTypes;
                  tree.setLargeModel(true);
                  tree.repaint();
               }
         };
}
