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

import javax.swing.table.AbstractTableModel;

public class VariableNode {
	Variable m_var;
	Object[] children;
	static VariableTreeTable currentView;
	VariableTreeTable view;
	static VariableNode current;
	
	public VariableNode(Variable var) {		
		addNodes(var);
	}
	
	public VariableNode(Node node) {
		m_var = new Variable("--", "Root", "", "");
		children = new VariableNode[node.getVariableList().size()];
		int i = 0;
		for(Variable var : node.getVariableList())
            {
                children[i] = new VariableNode(var);
                i++;
            }		
		current = this;
		node.addChangeListener(new Node.ChangeAdapter()
            {
                public void variableAssigned(Node node, String property)
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
		if (var instanceof ArrayVariable) {				    	
            children = new VariableNode[((ArrayVariable) var).size()];
            for (int i = 0; i < ((ArrayVariable) var).size(); i++) {
                    if(((ArrayVariable) var).getVariable(i) != null) {
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
		if (children == null && m_var instanceof ArrayVariable) {
            try {
                if (m_var != null) {
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
		if (children != null)
			return children.length;
		else 
			return 0;
    }
}

