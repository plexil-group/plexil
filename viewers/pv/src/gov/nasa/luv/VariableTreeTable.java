package gov.nasa.luv;

import gov.nasa.luv.TreeTableView.TreeModel;
import gov.nasa.luv.TreeTableView.Wrapper;

import java.awt.Color;
import java.awt.Component;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JTable;
import javax.swing.JTree;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreePath;

import treetable.JTreeTable;

public class VariableTreeTable extends JTreeTable {
	
	// most recent tree view created stored in case it's needed to set 
    // expansion state of the items in the new tree
    private static VariableTreeTable currentView;
	private JTree tree;
	private VariableNode root;	

	public void setTree(JTree tree) {
		this.tree = tree;
	}

	public VariableTreeTable(Model model) {
		super(new VariableTreeModel(new VariableNode(model)));
		//new TreeModel(new Wrapper(model))
		tree = getTree();   

		final JTable table = this;
		
		// add mouse listener which puts up pop-up menus
        addMouseListener(new MouseAdapter()
           {
           @Override
                 public void mousePressed(MouseEvent e)
                 {   
                    if (e.getClickCount() == 2)
                       handleClickEvent(e);
                    
                 }
           }); 
		
		root = (VariableNode)tree.getModel().getRoot();
		//passes TreeTable instance to VariableNode used in super
		VariableNode.getCurrentNode().setView(this);
		setDefaultRenderer(
	            VariableTreeModel.cTypes[1], 
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
	                     	                                          
	                     setBackgroundNonSelectionColor(
	                        isSelected
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
    public static VariableTreeTable getCurrent()
    {
        return currentView;
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
    
    /**
     * Handles a double click and triggers a specified MouseEvent.
     * 
     * @param mouseEvent the double click event
     */
    public void handleClickEvent(MouseEvent mouseEvent)
    {
       TreePath nodePath = tree.getClosestPathForLocation(mouseEvent.getX(), mouseEvent.getY());
       int visible_row = tree.getRowForPath(nodePath);
       if (tree.isExpanded(nodePath))
    	   tree.collapseRow(visible_row);  
       else
    	   tree.expandRow(visible_row); 
              
    }
    
    /**
     * Expands all the nodes of this TreeTableView.
     */
    public void expandAllNodes()
    {
        for (int i = 0; i < tree.getRowCount(); i++)
            tree.expandRow(i);
    }
    
    /**
     * Collapses all the node of this TreeTableView.
     */
    public void collapseAllNodes()
    {
        for (int i = tree.getRowCount() - 1; i > 0; i--)
            tree.collapseRow(i);
    }
}
