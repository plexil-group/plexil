//original author Europa group
//modified for Plexil

package gov.nasa.luv;

import java.util.List;
import java.util.Vector;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.JScrollPane;


public class DebugHistoryPanel
    extends JPanel
    implements DebugHistory, ActionListener
{
	protected List<DebugHistoryEntry> history_;
	protected List<Debug> debugHistory_;
	protected JTable historyTable_;
	protected int currentIdx_;
    protected JTextField gotoIdx_;
    protected JLabel status_;
    
	public DebugHistoryPanel()
	{
		history_ = new Vector<DebugHistoryEntry>();
		debugHistory_ = new Vector<Debug>();
		
	    historyTable_ = new JTable();
	    currentIdx_ = -1;
	    
    	JPanel buttonPanel = new JPanel(new FlowLayout());
    	JButton b;
		b = new JButton("<<"); b.addActionListener(this);b.setActionCommand("prev"); buttonPanel.add(b);
		b = new JButton(">>"); b.addActionListener(this);b.setActionCommand("next"); buttonPanel.add(b);
		b = new JButton("Go To Step"); b.addActionListener(this);b.setActionCommand("goto"); buttonPanel.add(b);
		gotoIdx_ = new JTextField(6);
		buttonPanel.add(gotoIdx_);
		
        status_ = new JLabel("");
		JPanel statusPanel = new JPanel(new FlowLayout());
        statusPanel.add(status_);
        
		setLayout(new BorderLayout());
    	add(BorderLayout.NORTH,buttonPanel);
    	add(BorderLayout.CENTER,new JScrollPane(historyTable_));		
    	add(BorderLayout.SOUTH,statusPanel);
    	
    	updateStatus();
	}

    public List getHistory() { return history_; }
    public List getDebugHistory() { return debugHistory_; }
    
    protected void updateStatus() 
    {	
	    status_.setText("Currently showing step "+(currentIdx_+1)+" out of "+debugHistory_.size()+" available");    	
    }
    
    public void showStep(int step) 
    { 
    	if (step>=0 && step<debugHistory_.size()) {
        	currentIdx_ = step;
    	    historyTable_.setModel(new DebugTableModel(debugHistory_.get(currentIdx_)));
    	    updateStatus();
    	}
    }

	public void add(int stepCount,Debug bw,String operatorHistory)
	{
		DebugHistoryEntry step = new DebugHistoryEntry(history_.size()+1,stepCount,bw.toString(),operatorHistory);
		debugHistory_.add(bw);
		history_.add(step);
	    updateStatus();
		//System.out.println(step);
	}
	
    public void actionPerformed(ActionEvent e) 
    {
        if ("next".equals(e.getActionCommand())) {
        	showStep(currentIdx_+1);
        	return;
        } 

        if ("prev".equals(e.getActionCommand())) {
        	showStep(currentIdx_-1);
        	return;
        }         

        if ("goto".equals(e.getActionCommand())) {
    		int step = 0;    		
    		try {
    			step = new Integer(gotoIdx_.getText());
    		}
    		catch (Exception ex) {
    			ex.printStackTrace();
    		}
    		
		    showStep(step-1);
    		
        	return;
        }         
    }    	        
}

