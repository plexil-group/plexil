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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

import static gov.nasa.luv.Constants.*;

public class HideOrShowWindow extends JPanel implements ListSelectionListener
{
    private JFrame frame;
    private JLabel instructions;    
    private JList list;
    private DefaultListModel listModel; 
    private JTextField textField;
    private JButton showButton;
    private JButton hideButton;        
    private JScrollPane listScrollPane;    
    private HideListener hideListener;   
    private JPanel instructionsPane;
    private Box buttonPane;
    private Box checkBoxList;
    
    public HideOrShowWindow() {}

    public HideOrShowWindow(String regexList) 
    {
        super(new BorderLayout());  
        
        createIntructionSection();
        createScrollListSection(regexList);
        createHideShowButtonSection();  
        createCheckBoxList(); 
             
        add(instructionsPane, BorderLayout.NORTH);
        add(listScrollPane, BorderLayout.WEST);
        add(checkBoxList, BorderLayout.EAST);
        add(buttonPane, BorderLayout.SOUTH);
    }
    
    private void createIntructionSection()
    {     
        String instructionText = "<html>Type the full or partial name of the nodes you want to hide." +
                "<br>Use (<b>*</b>) wildcard as a prefix and/or suffix to select multiple nodes." +
                "<br>" + 
                "<br>For example:" +
                "<br>Type <b>Child</b> to hide node <b>Child</b> only" +
                "<br>Type <b>Child*</b> to hide nodes Child<b>1</b>, Child<b>2</b>, and Child<b>3</b>" +
                "<br>Type <b>*Child</b> to hide nodes <b>a</b>Child, <b>b</b>Child, and <b>c</b>Child" +
                "<br>Type <b>*Child*</b> to hide nodes <b>a</b>Child<b>1</b>, <b>b</b>Child<b>2</b>, and <b>c</b>Child<b>3</b>";
        
        instructions = new JLabel(instructionText);
        instructions.setFont(new Font("Monospaced", Font.PLAIN, 12));
        instructionsPane = new JPanel();
        instructionsPane.add(instructions);
    }
    
    private void createScrollListSection(String regexList)
    {
        listModel = new DefaultListModel();
        if (!regexList.equals(UNKNOWN) && !regexList.equals(""))
        {
            String [] array = regexList.split(", ");
            for (int i = 0; i < array.length; i++)
            {
                listModel.insertElementAt(array[i], i);
            }
        }
        list = new JList(listModel);
        list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        list.setSelectedIndex(0);
        list.addListSelectionListener(this);
        list.setVisibleRowCount(5);   
        listScrollPane = new JScrollPane(list);  
        listScrollPane.setPreferredSize(new Dimension(300, 80));
    }
    
    private void createHideShowButtonSection()
    {
        hideButton = new JButton("Hide");
        hideListener = new HideListener(hideButton);
        hideButton.setActionCommand("Hide");
        hideButton.addActionListener(hideListener);
        hideButton.setEnabled(false);      

        showButton = new JButton("Show");
        showButton.setActionCommand("Show");
        showButton.addActionListener(new ShowListener());       
        if (listModel.isEmpty())
            showButton.setEnabled(false);      
        else
            showButton.setEnabled(true);

        textField = new JTextField("[Type node name here]", 10);
        textField.setForeground(Color.lightGray);
        textField.addActionListener(hideListener);         
        textField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusGained(java.awt.event.FocusEvent evt) {
                if (textField.getText().equals("[Type node name here]"))
                    textField.setText("");
            }
        });        
        textField.getDocument().addDocumentListener(hideListener);  
        
        buttonPane = Box.createHorizontalBox();
        buttonPane.add(Box.createHorizontalGlue());
        buttonPane.add(showButton);
        buttonPane.add(Box.createHorizontalStrut(5));
        buttonPane.add(new JSeparator(SwingConstants.VERTICAL));
        buttonPane.add(Box.createHorizontalStrut(5));
        buttonPane.add(textField);
        buttonPane.add(hideButton);
    }
    
    private void createCheckBoxList()
    {
        checkBoxList = Box.createHorizontalBox();
        
        JPanel iconSide = new JPanel();
        iconSide.setLayout(new GridLayout(7,1));
        
        JPanel checkBoxSide = new JPanel();
        checkBoxSide.setLayout(new GridLayout(7,1));
        
        final JCheckBox assnBox = new JCheckBox("Show " + ASSN + " Nodes", isBoxChecked(ASSN));
        final JCheckBox cmdBox = new JCheckBox("Show " + COMMAND + " Nodes", isBoxChecked(COMMAND)); 
        final JCheckBox updateBox = new JCheckBox("Show " + UPDATE + " Nodes", isBoxChecked(UPDATE)); 
        final JCheckBox emptyBox = new JCheckBox("Show " + EMPTY + " Nodes", isBoxChecked(EMPTY)); 
        final JCheckBox funcBox = new JCheckBox("Show " + FUNCCALL + " Nodes", isBoxChecked(FUNCCALL));
        final JCheckBox listBox = new JCheckBox("Show " + NODELIST + " Nodes", isBoxChecked(NODELIST)); 
        final JCheckBox libBox = new JCheckBox("Show " + LIBRARYNODECALL + " Nodes", isBoxChecked(LIBRARYNODECALL));
        
        assnBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        cmdBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        updateBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        emptyBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        funcBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        listBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        libBox.setFont(new Font("Monospaced", Font.PLAIN, 12)); 
   
        iconSide.add(new JLabel(getIcon(NODELIST), JLabel.LEFT));
        checkBoxSide.add(listBox);
        iconSide.add(new JLabel(getIcon(ASSN), JLabel.LEFT));
        checkBoxSide.add(assnBox); 
        iconSide.add(new JLabel(getIcon(COMMAND), JLabel.LEFT));
        checkBoxSide.add(cmdBox); 
        iconSide.add(new JLabel(getIcon(EMPTY), JLabel.LEFT));
        checkBoxSide.add(emptyBox);
        iconSide.add(new JLabel(getIcon(FUNCCALL), JLabel.LEFT));
        checkBoxSide.add(funcBox);
        iconSide.add(new JLabel(getIcon(LIBRARYNODECALL), JLabel.LEFT));
        checkBoxSide.add(libBox);
        iconSide.add(new JLabel(getIcon(UPDATE), JLabel.LEFT));
        checkBoxSide.add(updateBox); 
        
        checkBoxList.add(iconSide);
        checkBoxList.add(checkBoxSide);

        assnBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!assnBox.isSelected())
                    Luv.getLuv().setProperty(ASSN, "HIDE");
                else
                    Luv.getLuv().setProperty(ASSN, "SHOW");
                Luv.getLuv().refreshRegexView();
            }
        });
        cmdBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!cmdBox.isSelected())
                    Luv.getLuv().setProperty(COMMAND, "HIDE");
                else
                    Luv.getLuv().setProperty(COMMAND, "SHOW");
                Luv.getLuv().refreshRegexView();
            }
        });
        updateBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!updateBox.isSelected())
                    Luv.getLuv().setProperty(UPDATE, "HIDE");
                else
                    Luv.getLuv().setProperty(UPDATE, "SHOW");
                Luv.getLuv().refreshRegexView();
            }
        });
        emptyBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!emptyBox.isSelected())
                    Luv.getLuv().setProperty(EMPTY, "HIDE");
                else
                    Luv.getLuv().setProperty(EMPTY, "SHOW");
                Luv.getLuv().refreshRegexView();
            }
        });
        funcBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!funcBox.isSelected())
                    Luv.getLuv().setProperty(FUNCCALL, "HIDE");
                else
                    Luv.getLuv().setProperty(FUNCCALL, "SHOW");
                Luv.getLuv().refreshRegexView();
            }
        });
        listBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!listBox.isSelected())
                    Luv.getLuv().setProperty(NODELIST, "HIDE");
                else
                    Luv.getLuv().setProperty(NODELIST, "SHOW");
                Luv.getLuv().refreshRegexView();
            }
        });
        libBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!libBox.isSelected())
                    Luv.getLuv().setProperty(LIBRARYNODECALL, "HIDE");
                else
                    Luv.getLuv().setProperty(LIBRARYNODECALL, "SHOW");
                Luv.getLuv().refreshRegexView();
            }
        });
    }

    private boolean isBoxChecked(String type)
    {
        if (Luv.getLuv().getProperty(type).equals("SHOW"))
            return true;
        else if (Luv.getLuv().getProperty(type).equals(UNKNOWN))
            return true;
        else
            return false;
    }

    class ShowListener implements ActionListener 
    {
        public void actionPerformed(ActionEvent e) 
        {
            //This method can be called only if
            //there's a valid selection
            //so go ahead and remove whatever's selected.
            int index = list.getSelectedIndex();
            int size = listModel.getSize();

            if (size == 0) 
            { 
                //Nobody's left, disable firing.
                showButton.setEnabled(false);
            } 
            else 
            { 
                showButton.setEnabled(true);
                String regex = (String) listModel.remove(index);
                Luv.getLuv().removeRegex(regex);
            
                //Select an index.
                if (index == listModel.getSize()) 
                {
                    //removed item in last position
                    index--;
                }

                list.setSelectedIndex(index);
                list.ensureIndexIsVisible(index);
            }
        }
    }

    //This listener is shared by the text field and the hide button.
    class HideListener implements ActionListener, DocumentListener
    {
        private boolean alreadyEnabled = false;
        private JButton button;

        public HideListener(JButton button) 
        {
            this.button = button;
        }

        // Required by ActionListener.
        public void actionPerformed(ActionEvent e) 
        {
            String regex = textField.getText();            
            
            // User didn't type in a unique name...
            if (regex.equals("") || alreadyInList(regex)) 
            {
                JOptionPane.showMessageDialog(Luv.getLuv(), 
                                          regex + " has already been entered", 
                                          "Error", 
                                          JOptionPane.ERROR_MESSAGE);
                textField.requestFocusInWindow();
                textField.selectAll();
                return;
            }
            else
            {
                Luv.getLuv().addRegex(regex);
            }               

            int index = list.getSelectedIndex(); //get selected index
            if (index == -1) 
            { 
                //no selection, so insert at beginning
                index = 0;
            } 
            else 
            {           
                //add after the selected item
                index++;
            }

            listModel.insertElementAt(textField.getText(), index);

            //Reset the text field.
            textField.requestFocusInWindow();
            textField.setText("");

            //Select the new item and make it visible.
            list.setSelectedIndex(index);
            list.ensureIndexIsVisible(index);
        }         

        //This method tests for string equality. You could certainly
        //get more sophisticated about the algorithm.  For example,
        //you might want to ignore white space and capitalization.
        protected boolean alreadyInList(String name) 
        {
            return listModel.contains(name);
        }

        //Required by DocumentListener.
        public void insertUpdate(DocumentEvent e) 
        {
            textField.setForeground(Color.BLACK);            
            enableButton();
        }

        //Required by DocumentListener.
        public void removeUpdate(DocumentEvent e) 
        {
            handleEmptyTextField(e);
        }

        //Required by DocumentListener.
        public void changedUpdate(DocumentEvent e) 
        {
            if (!handleEmptyTextField(e)) 
            {
                enableButton();
            }
        }

        private void enableButton() 
        {
            if (!alreadyEnabled) 
            {
                button.setEnabled(true);
            }
        }

        private boolean handleEmptyTextField(DocumentEvent e) 
        {
            if (e.getDocument().getLength() <= 0) 
            {
                button.setEnabled(false);
                alreadyEnabled = false;
                return true;
            }
            return false;
        }
    }
    
    //This method is required by ListSelectionListener.
    public void valueChanged(ListSelectionEvent e) 
    {
        if (e.getValueIsAdjusting() == false) 
        {
            if (list.getSelectedIndex() == -1) 
            {           
                //No selection, disable show button.
                showButton.setEnabled(false);
            } 
            else 
            {
                //Selection, enable the show button.
                showButton.setEnabled(true);
            }
        }
    }
    
    public void makeVisible()
    {
        frame.setVisible(true);
    }

    public void createHideOrShowWindow(String regexList) 
    {
        frame = new JFrame("Hide/Show Nodes");

        frame.add(new HideOrShowWindow(regexList), BorderLayout.CENTER);

        frame.setSize(Luv.getLuv().getProperties().getDimension(PROP_HIDESHOWWIN_SIZE));
        frame.setLocation(Luv.getLuv().getProperties().getPoint(PROP_HIDESHOWWIN_LOC));
        frame.pack();  
    }
}
