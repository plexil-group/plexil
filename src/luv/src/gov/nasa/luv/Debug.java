//original author Europa group
//modified for Plexil

package gov.nasa.luv;

import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.HashMap;

public class Debug
{	
	protected List<List<String>> msgList_;
	protected Map<String,List<String>> msgMap_;
	
	public Debug()
	{
		msgList_ = new Vector<List<String>>();
		msgMap_ = new HashMap<String,List<String>>();
	}

	public List<List<String>> getMsgList()
	{
		return msgList_;
	}
	
	public void addNode(String name,String state,String msg)
	{
		//System.out.println("Debug.add("+name+","+state+","+msg+")");

		List<String> node = msgMap_.get(name);
		if (node == null) {
			node = new Vector<String>();	
        	msgList_.add(node);
        	msgMap_.put(name, node);
		}

		int idx = node.indexOf(name);
		StringBuffer buf = new StringBuffer();
		buf.append(name).append("=").append(state).append(msg != null ? "-"+msg : "");
		String nodeValue = buf.toString();
		
		if (state.equals("OnTable")) {
			if (idx == -1) 
    			node.add(0,nodeValue);
			else
				node.set(idx,nodeValue);
		}
		
		if (state.equals("On")) {
			
			// Add Bottom if we need to
            int bottomIdx = -1;
            for (int i=0;i<node.size();i++) {
            	if (node.get(i).toString().startsWith(msg)) {
            		bottomIdx = i;
            		break;
            	}
            }

            if (bottomIdx == -1) {
            	bottomIdx = (idx == -1 ? 0 : idx++);
            	node.add(bottomIdx,msg);
            	msgMap_.put(msg, node);
            }

			if (idx == -1) 
                node.add(bottomIdx+1,nodeValue);
			else 
    			node.set(idx,nodeValue);
		}
    }
	
	public String toString()
	{
		StringBuffer buf = new StringBuffer();
		
		for (List<String> node : msgList_) {
			buf.append("{");
			for (int i=0;i<node.size();i++) {
				if (i > 0)
					buf.append(",");
		        buf.append(node.get(i));				
			}
			buf.append("}  ");
		}
		
		return buf.toString();
	}
}
