package gov.nasa.luv;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;
import static gov.nasa.luv.Constants.*;

public class LuvPortGUI extends JPanel implements ActionListener {
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private JFrame frame;
	private static LuvPortGUI thePortGui;
	private Integer pick = 0;
	private Vector<Integer> portList;
	
	public LuvPortGUI() {
		thePortGui = this;
		portList = new Vector<Integer>();
		refresh();
		JLabel userPrompt = new JLabel("Select a port:");
		JButton cancelButton = new JButton("Cancel");
		JComboBox portComboBox = new JComboBox(portList);
		portComboBox.addActionListener(this);
		cancelButton.addActionListener(new ButtonListener());
		
		JPanel patternPanel = new JPanel();
		patternPanel.setLayout(new BoxLayout(patternPanel, BoxLayout.PAGE_AXIS));
		patternPanel.add(userPrompt);
		patternPanel.add(Box.createRigidArea(new Dimension(0,10)));
		portComboBox.setAlignmentX(Component.LEFT_ALIGNMENT);
		patternPanel.add(portComboBox);
		patternPanel.add(Box.createRigidArea(new Dimension(0,10)));
		patternPanel.add(cancelButton);
		patternPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
		add(patternPanel);		
		setBorder(BorderFactory.createEmptyBorder(20, 40, 20, 40));
		frame = new JFrame("Server Port");
		
	}
	
	public static LuvPortGUI getLuvPortGUI()
	{
		return thePortGui;
	}
	
	class ButtonListener implements ActionListener {
		ButtonListener(){			
		}
		
		public void actionPerformed(ActionEvent e){
			if(e.getActionCommand().equals("Cancel"))
				Luv.getLuv().getPortGUI().frame.setVisible(false);
		}
	}
	
	public void actionPerformed(ActionEvent e) {
        JComboBox cb = (JComboBox)e.getSource();
        Integer newSelection = (Integer)cb.getSelectedItem();
        if(!LuvTempFile.checkPort(newSelection))
        {
	        Luv.getLuv().getPortGUI().setPick(newSelection);        
	        Luv.getLuv().getPortGUI().frame.setVisible(false);
	        Luv.getLuv().changePort(Luv.getLuv().getPortGUI().getPick()+"");	        
        } else
        {
        	Luv.getLuv().getPortGUI().frame.setVisible(false);
        	Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Port: " + newSelection + " in Use, try again");
        	Luv.getLuv().getStatusMessageHandler().showChangeOnPort("Still on port " + Luv.getLuv().getPort(), Color.BLUE);
        	Luv.getLuv().getPortGUI().refresh();
        }
    }	
	
	public void setPick(Integer pick_in)
	{
		this.pick = pick_in;
	}
	
	public Integer getPick()
	{
		return this.pick;
	}
	
	public JFrame getFrame()
	{
		return frame;
	}

	public void refresh()
	{
		portList.clear();
		for(int i = PORT_MIN; i < PORT_MAX + 1; i++)
			if(!LuvTempFile.checkPort(i))
				portList.add(i);
		if(!portList.isEmpty())
			pick = portList.firstElement();
	}
	
	public boolean isEmpty()
	{
		return portList.isEmpty();
	}
}
