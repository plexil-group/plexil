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

