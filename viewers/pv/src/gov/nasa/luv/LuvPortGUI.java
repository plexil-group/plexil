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
		JComboBox<Integer> portComboBox = new JComboBox<Integer>(portList);
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
			if (e.getActionCommand().equals("Cancel"))
				frame.setVisible(false);
		}
	}
	
	public void actionPerformed(ActionEvent e) {
        JComboBox cb = (JComboBox)e.getSource();
        Integer newSelection = (Integer)cb.getSelectedItem();
        if(!LuvTempFile.checkPort(newSelection))
            {
                setPick(newSelection);
                frame.setVisible(false);
                Luv.getLuv().changePort(getPick()+"");	        
            } else {
        	frame.setVisible(false);
        	Luv.getLuv().getStatusMessageHandler().displayInfoMessage("Port: " + newSelection + " in Use, try again");
        	Luv.getLuv().getStatusMessageHandler().showChangeOnPort("Still on port " + Luv.getLuv().getPort(), Color.BLUE);
        	refresh();
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
		for (int i = PORT_MIN; i < PORT_MAX + 1; i++)
			if (!LuvTempFile.checkPort(i))
				portList.add(i);
		if (!portList.isEmpty())
			pick = portList.firstElement();
	}
	
	public boolean isEmpty()
	{
		return portList.isEmpty();
	}
}
