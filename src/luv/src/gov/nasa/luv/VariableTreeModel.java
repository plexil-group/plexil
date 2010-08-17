package gov.nasa.luv;

import javax.swing.table.AbstractTableModel;
import treetable.AbstractTreeTableModel;
import treetable.TreeTableModel;

public class VariableTreeModel extends AbstractTreeTableModel<Object> {
	
	static protected String[] columnNames = {"In/InOut",
            "Name",
            "Type",
            "Value",
	};
	static protected Class[] cTypes = {String.class, String.class, String.class, String.class};	
	
	public VariableTreeModel(VariableNode model){
		super(model);		
	}    
	
	protected Variable getVar(Object node) {
		VariableNode varNode = ((VariableNode)node);
		return varNode.getVariable();
	}
	
	protected Object[] getChildren(Object node) {
		VariableNode varNode = ((VariableNode)node);
		return varNode.getChildren();
	}
	
	public boolean isLeaf(Object node)
    {
        return ((VariableNode)node).getChildCount() == 0;
    }
	
	public int getChildCount(Object node) {
		Object[] children = getChildren(node);
		return (children == null) ? 0 : children.length;
	}
	
	public Object getChild(Object node, int i) {
		return getChildren(node)[i];
	}
	
	public int getColumnCount() {
		return columnNames.length;
	}
	
	public String getColumnName(int column) {
		return columnNames[column];
	}
	
	public Class getColumnClass(int column) {
		return cTypes[column];
	}
	
	public Object getValueAt(Object node, int column) {
		Variable var = getVar(node);
		try {
    			switch(column) {
    			case 0:
    				return var.getInOut();
    			case 1:
    				return var.getName();
    			case 2:
    				return var.getType();
    			case 3:
    				return var.getValue();
    			}
			} catch (SecurityException se) {}
			
			return null;
	}	    	
}

class VariableNode {
	Variable m_var;
	Object[] children;
	static VariableTreeTable currentView;
	VariableTreeTable view;
	static VariableNode current;
	
	public VariableNode(Variable var) {		
		addNodes(var);
	}
	
	public VariableNode(Model model) {
		m_var = new Variable("--", "Root", "", "");
		children = new VariableNode[model.getVariableList().size()];
		int i = 0;
		for(Variable var : model.getVariableList())
		{
			children[i] = new VariableNode(var);
			i++;
		}		
		current = this;
		model.addChangeListener(new Model.ChangeAdapter()
        {
              public void variableAssigned(Model model, String property)
              {
            	  if(view == null)
            		  view = VariableNode.currentView;            	  
                  ((AbstractTableModel)view.getModel()).fireTableDataChanged();
              }
        });		
	}
	
	public static VariableNode getCurrentNode()
	{
		return current;
	}
	
	public static void setCurrentView(VariableTreeTable view)
    {
       VariableNode.currentView = view;
    }

	public void setView(VariableTreeTable view)
    {
       this.view = view;
    }
	
	private void addNodes(Variable var)
    {
		if(var instanceof ArrayVariable)
		{				    	
	    	children = new VariableNode[((ArrayVariable) var).size()];
	        for (int i = 0; i < ((ArrayVariable) var).size(); i++)
	        {
	        	if(((ArrayVariable) var).getVariable(i) != null)
	        	{	        				
					children[i] = new VariableNode(((ArrayVariable) var).getVariable(i));
	        	}
	        }
		}
		m_var = var;
			
    }	
	
	public Variable getVariable() {
		return m_var;
	}
	
	public String toString() {    		
			return m_var.getName();
	}
	
	protected Object[] getChildren() {
		if(children == null && m_var instanceof ArrayVariable)
		{
			
    		try {	    			
    			if(m_var != null) {
    				children = new VariableNode[((ArrayVariable) m_var).size()];
    				for(int i = 0; i < ((ArrayVariable) m_var).size(); i++) {
    					children[i] = new VariableNode(((ArrayVariable) m_var).getVariable(i));
    				}
    			}
    		} catch (SecurityException se) {}	    		
		}
		return children;
	}
	
	public int getChildCount()
    {
		if(children != null)
			return children.length;
		else 
			return 0;
    }
}