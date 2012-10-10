/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

/** 
 * The HideOrShowWindow class is an interface for the user to identify the nodes 
 * to hide, by name or node type.
 */

public class HideOrShowWindow extends JPanel implements ListSelectionListener
{
    private JFrame frame;
    private JPanel instructionsPane;
    private JScrollPane listScrollPane;
    private Box checkBoxList;
    private Box buttonPane;
    private JList list;
    private DefaultListModel listModel; 
    private JTextField textField;
    private JButton showButton;          
    
    public HideOrShowWindow() {}
    
    /**
     * Constructs a HideOrShowWindow with the specified list of saved elements
     * that are currently hidden.
     * 
     * @param regexList the list of currently hidden elements and clan be empty
     */
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
        
        JLabel instructions = new JLabel(instructionText);
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
        JButton hideButton = new JButton("Hide");
        HideListener hideListener = new HideListener(hideButton);
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
        iconSide.setLayout(new GridLayout(16,1));
        
        JPanel checkBoxSide = new JPanel();
        checkBoxSide.setLayout(new GridLayout(16,1));
        
        final JCheckBox assnBox = new JCheckBox("Show " + ASSN + " Nodes", isBoxChecked(ASSN));
        final JCheckBox cmdBox = new JCheckBox("Show " + COMMAND + " Nodes", isBoxChecked(COMMAND)); 
        final JCheckBox updateBox = new JCheckBox("Show " + UPDATE + " Nodes", isBoxChecked(UPDATE)); 
        final JCheckBox emptyBox = new JCheckBox("Show " + EMPTY + " Nodes", isBoxChecked(EMPTY)); 
        final JCheckBox listBox = new JCheckBox("Show " + NODELIST + " Nodes", isBoxChecked(NODELIST)); 
        final JCheckBox libBox = new JCheckBox("Show " + LIBRARYNODECALL + " Nodes", isBoxChecked(LIBRARYNODECALL));
        final JCheckBox ifBox = new JCheckBox("Show " + IF + " Actions", isBoxChecked(IF));
        final JCheckBox thenBox = new JCheckBox("Show " + THEN + " Actions", isBoxChecked(THEN));
        final JCheckBox elseBox = new JCheckBox("Show " + ELSE + " Actions", isBoxChecked(ELSE));
        final JCheckBox whileBox = new JCheckBox("Show " + WHILE + " Loop Actions", isBoxChecked(WHILE));
        final JCheckBox forBox = new JCheckBox("Show " + FOR + " Loop Actions", isBoxChecked(FOR));
        final JCheckBox tryBox = new JCheckBox("Show " + TRY + " Actions", isBoxChecked(TRY));
        final JCheckBox seqBox = new JCheckBox("Show " + SEQ + " Actions", isBoxChecked(SEQ));
        final JCheckBox unchkdSeqBox = new JCheckBox("Show " + UNCHKD_SEQ + " Actions", isBoxChecked(UNCHKD_SEQ));
        final JCheckBox concBox = new JCheckBox("Show " + CONCURRENCE + " Actions", isBoxChecked(CONCURRENCE));
        
        assnBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        cmdBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        updateBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        emptyBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        listBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        libBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        ifBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        thenBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        elseBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        whileBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        forBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        tryBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        seqBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        unchkdSeqBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
        concBox.setFont(new Font("Monospaced", Font.PLAIN, 12));
   
        iconSide.add(new JLabel(getIcon(NODELIST), JLabel.LEFT));
        checkBoxSide.add(listBox);
        iconSide.add(new JLabel(getIcon(ASSN), JLabel.LEFT));
        checkBoxSide.add(assnBox); 
        iconSide.add(new JLabel(getIcon(COMMAND), JLabel.LEFT));
        checkBoxSide.add(cmdBox); 
        iconSide.add(new JLabel(getIcon(EMPTY), JLabel.LEFT));
        checkBoxSide.add(emptyBox);
        iconSide.add(new JLabel(getIcon(LIBRARYNODECALL), JLabel.LEFT));
        checkBoxSide.add(libBox);
        iconSide.add(new JLabel(getIcon(UPDATE), JLabel.LEFT));
        checkBoxSide.add(updateBox);
        iconSide.add(new JLabel(getIcon(IF), JLabel.LEFT));
        checkBoxSide.add(ifBox);
        iconSide.add(new JLabel(getIcon(THEN), JLabel.LEFT));
        checkBoxSide.add(thenBox);
        iconSide.add(new JLabel(getIcon(ELSE), JLabel.LEFT));
        checkBoxSide.add(elseBox);
        iconSide.add(new JLabel(getIcon(WHILE), JLabel.LEFT));
        checkBoxSide.add(whileBox);
        iconSide.add(new JLabel(getIcon(FOR), JLabel.LEFT));
        checkBoxSide.add(forBox);
        iconSide.add(new JLabel(getIcon(TRY), JLabel.LEFT));
        checkBoxSide.add(tryBox);
        iconSide.add(new JLabel(getIcon(SEQ), JLabel.LEFT));
        checkBoxSide.add(seqBox);
        iconSide.add(new JLabel(getIcon(UNCHKD_SEQ), JLabel.LEFT));
        checkBoxSide.add(unchkdSeqBox);
        iconSide.add(new JLabel(getIcon(CONCURRENCE), JLabel.LEFT));
        checkBoxSide.add(concBox);
        
        checkBoxList.add(iconSide);
        checkBoxList.add(checkBoxSide);

        assnBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!assnBox.isSelected()) {
                    Luv.getLuv().setProperty(ASSN, "HIDE");
                    Luv.getLuv().setProperty(THEN_ASSN, "HIDE");
                    Luv.getLuv().setProperty(ELSE_ASSN, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(ASSN, "SHOW");
                    Luv.getLuv().setProperty(THEN_ASSN, "SHOW");
                    Luv.getLuv().setProperty(ELSE_ASSN, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        cmdBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!cmdBox.isSelected()) {
                    Luv.getLuv().setProperty(COMMAND, "HIDE");
                    Luv.getLuv().setProperty(THEN_CMD, "HIDE");
                    Luv.getLuv().setProperty(ELSE_CMD, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(COMMAND, "SHOW");
                    Luv.getLuv().setProperty(THEN_CMD, "SHOW");
                    Luv.getLuv().setProperty(ELSE_CMD, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        updateBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!updateBox.isSelected()) {
                    Luv.getLuv().setProperty(UPDATE, "HIDE");
                    Luv.getLuv().setProperty(THEN_UPDATE, "HIDE");
                    Luv.getLuv().setProperty(ELSE_UPDATE, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(UPDATE, "SHOW");
                    Luv.getLuv().setProperty(THEN_UPDATE, "SHOW");
                    Luv.getLuv().setProperty(ELSE_UPDATE, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        emptyBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!emptyBox.isSelected()) {
                    Luv.getLuv().setProperty(EMPTY, "HIDE");
                    Luv.getLuv().setProperty(THEN_EMPTY, "HIDE");
                    Luv.getLuv().setProperty(ELSE_EMPTY, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(EMPTY, "SHOW");
                    Luv.getLuv().setProperty(THEN_EMPTY, "SHOW");
                    Luv.getLuv().setProperty(ELSE_EMPTY, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        listBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!listBox.isSelected()) {
                    Luv.getLuv().setProperty(NODELIST, "HIDE");
                    Luv.getLuv().setProperty(THEN_LIST, "HIDE");
                    Luv.getLuv().setProperty(ELSE_LIST, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(NODELIST, "SHOW");
                    Luv.getLuv().setProperty(THEN_LIST, "SHOW");
                    Luv.getLuv().setProperty(ELSE_LIST, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        libBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!libBox.isSelected()) {
                    Luv.getLuv().setProperty(LIBRARYNODECALL, "HIDE");
                    Luv.getLuv().setProperty(THEN_LIB, "HIDE");
                    Luv.getLuv().setProperty(ELSE_LIB, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(LIBRARYNODECALL, "SHOW");
                    Luv.getLuv().setProperty(THEN_LIB, "SHOW");
                    Luv.getLuv().setProperty(ELSE_LIB, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        ifBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!ifBox.isSelected()) {
                    Luv.getLuv().setProperty(IF, "HIDE");
                    Luv.getLuv().setProperty(THEN_IF, "HIDE");
                    Luv.getLuv().setProperty(ELSE_IF, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(IF, "SHOW");
                    Luv.getLuv().setProperty(THEN_IF, "SHOW");
                    Luv.getLuv().setProperty(ELSE_IF, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        thenBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!thenBox.isSelected()) {
                    Luv.getLuv().setProperty(THEN, "HIDE");
                    Luv.getLuv().setProperty(THEN_LIST, "HIDE");
                    Luv.getLuv().setProperty(THEN_ASSN, "HIDE");
                    Luv.getLuv().setProperty(THEN_CMD, "HIDE");
                    Luv.getLuv().setProperty(THEN_EMPTY, "HIDE");
                    Luv.getLuv().setProperty(THEN_LIB, "HIDE");
                    Luv.getLuv().setProperty(THEN_UPDATE, "HIDE");
                    Luv.getLuv().setProperty(THEN_IF, "HIDE");
                    Luv.getLuv().setProperty(THEN_WHILE, "HIDE");
                    Luv.getLuv().setProperty(THEN_FOR, "HIDE");
                    Luv.getLuv().setProperty(THEN_TRY, "HIDE");
                    Luv.getLuv().setProperty(THEN_SEQ, "HIDE");
                    Luv.getLuv().setProperty(THEN_UNCHKD_SEQ, "HIDE");
                    Luv.getLuv().setProperty(THEN_CONCURRENCE, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(THEN, "SHOW");
                    Luv.getLuv().setProperty(THEN_LIST, "SHOW");
                    Luv.getLuv().setProperty(THEN_ASSN, "SHOW");
                    Luv.getLuv().setProperty(THEN_CMD, "SHOW");
                    Luv.getLuv().setProperty(THEN_EMPTY, "SHOW");
                    Luv.getLuv().setProperty(THEN_LIB, "SHOW");
                    Luv.getLuv().setProperty(THEN_UPDATE, "SHOW");
                    Luv.getLuv().setProperty(THEN_IF, "SHOW");
                    Luv.getLuv().setProperty(THEN_WHILE, "SHOW");
                    Luv.getLuv().setProperty(THEN_FOR, "SHOW");
                    Luv.getLuv().setProperty(THEN_TRY, "SHOW");
                    Luv.getLuv().setProperty(THEN_SEQ, "SHOW");
                    Luv.getLuv().setProperty(THEN_UNCHKD_SEQ, "SHOW");
                    Luv.getLuv().setProperty(THEN_CONCURRENCE, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        elseBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!elseBox.isSelected()) {
                    Luv.getLuv().setProperty(ELSE, "HIDE");
                    Luv.getLuv().setProperty(ELSE_LIST, "HIDE");
                    Luv.getLuv().setProperty(ELSE_ASSN, "HIDE");
                    Luv.getLuv().setProperty(ELSE_CMD, "HIDE");
                    Luv.getLuv().setProperty(ELSE_EMPTY, "HIDE");
                    Luv.getLuv().setProperty(ELSE_LIB, "HIDE");
                    Luv.getLuv().setProperty(ELSE_UPDATE, "HIDE");
                    Luv.getLuv().setProperty(ELSE_IF, "HIDE");
                    Luv.getLuv().setProperty(ELSE_WHILE, "HIDE");
                    Luv.getLuv().setProperty(ELSE_FOR, "HIDE");
                    Luv.getLuv().setProperty(ELSE_TRY, "HIDE");
                    Luv.getLuv().setProperty(ELSE_SEQ, "HIDE");
                    Luv.getLuv().setProperty(ELSE_UNCHKD_SEQ, "HIDE");
                    Luv.getLuv().setProperty(ELSE_CONCURRENCE, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(ELSE, "SHOW");
                    Luv.getLuv().setProperty(ELSE_LIST, "SHOW");
                    Luv.getLuv().setProperty(ELSE_ASSN, "SHOW");
                    Luv.getLuv().setProperty(ELSE_CMD, "SHOW");
                    Luv.getLuv().setProperty(ELSE_EMPTY, "SHOW");
                    Luv.getLuv().setProperty(ELSE_LIB, "SHOW");
                    Luv.getLuv().setProperty(ELSE_UPDATE, "SHOW");
                    Luv.getLuv().setProperty(ELSE_IF, "SHOW");
                    Luv.getLuv().setProperty(ELSE_WHILE, "SHOW");
                    Luv.getLuv().setProperty(ELSE_FOR, "SHOW");
                    Luv.getLuv().setProperty(ELSE_TRY, "SHOW");
                    Luv.getLuv().setProperty(ELSE_SEQ, "SHOW");
                    Luv.getLuv().setProperty(ELSE_UNCHKD_SEQ, "SHOW");
                    Luv.getLuv().setProperty(ELSE_CONCURRENCE, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        whileBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!whileBox.isSelected()) {
                    Luv.getLuv().setProperty(WHILE, "HIDE");
                    Luv.getLuv().setProperty(THEN_WHILE, "HIDE");
                    Luv.getLuv().setProperty(ELSE_WHILE, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(WHILE, "SHOW");
                    Luv.getLuv().setProperty(THEN_WHILE, "SHOW");
                    Luv.getLuv().setProperty(ELSE_WHILE, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        forBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!forBox.isSelected()) {
                    Luv.getLuv().setProperty(FOR, "HIDE");
                    Luv.getLuv().setProperty(THEN_FOR, "HIDE");
                    Luv.getLuv().setProperty(ELSE_FOR, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(FOR, "SHOW");
                    Luv.getLuv().setProperty(THEN_FOR, "SHOW");
                    Luv.getLuv().setProperty(ELSE_FOR, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        tryBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!tryBox.isSelected()) {
                    Luv.getLuv().setProperty(TRY, "HIDE");
                    Luv.getLuv().setProperty(THEN_TRY, "HIDE");
                    Luv.getLuv().setProperty(ELSE_TRY, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(TRY, "SHOW");
                    Luv.getLuv().setProperty(THEN_TRY, "SHOW");
                    Luv.getLuv().setProperty(ELSE_TRY, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        seqBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!seqBox.isSelected()) {
                    Luv.getLuv().setProperty(SEQ, "HIDE");
                    Luv.getLuv().setProperty(THEN_SEQ, "HIDE");
                    Luv.getLuv().setProperty(ELSE_SEQ, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(SEQ, "SHOW");
                    Luv.getLuv().setProperty(THEN_SEQ, "SHOW");
                    Luv.getLuv().setProperty(ELSE_SEQ, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        unchkdSeqBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!unchkdSeqBox.isSelected()) {
                    Luv.getLuv().setProperty(UNCHKD_SEQ, "HIDE");
                    Luv.getLuv().setProperty(THEN_UNCHKD_SEQ, "HIDE");
                    Luv.getLuv().setProperty(ELSE_UNCHKD_SEQ, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(UNCHKD_SEQ, "SHOW");
                    Luv.getLuv().setProperty(THEN_UNCHKD_SEQ, "SHOW");
                    Luv.getLuv().setProperty(ELSE_UNCHKD_SEQ, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
            }
        });
        concBox.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e){
                if (!concBox.isSelected()) {
                    Luv.getLuv().setProperty(CONCURRENCE, "HIDE");
                    Luv.getLuv().setProperty(THEN_CONCURRENCE, "HIDE");
                    Luv.getLuv().setProperty(ELSE_CONCURRENCE, "HIDE");
                }
                else {
                    Luv.getLuv().setProperty(CONCURRENCE, "SHOW");
                    Luv.getLuv().setProperty(THEN_CONCURRENCE, "SHOW");
                    Luv.getLuv().setProperty(ELSE_CONCURRENCE, "SHOW");
                }
                Luv.getLuv().getViewHandler().refreshRegexView();
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

    /**
     * The ShowListener class provides a listener for when the user presses the 
     * "Show" button which activates the actionPerformed() method that will 
     * un-hide the element the user had selected from the list of hidden elements.
     */
    class ShowListener implements ActionListener 
    {
        /**
         * Reveals the elements in the list of hidden elements the user selected.
         * 
         * @param e the ActionEvent that represents when the user pressed the "Show" button
         */
        public void actionPerformed(ActionEvent e) 
        {
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
                Luv.getLuv().getRegexModelFilter().removeRegex(regex);
            
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

    /**
     * The HideListener class is a listener shared by the text field and the 
     * hide button and listens for when the user pressed the "Hide" button.
     */
    class HideListener implements ActionListener, DocumentListener
    {
        private boolean alreadyEnabled = false;
        private JButton button;

        /**
         * Constructs a HideListener with the specified button.
         * 
         * @param button the "Hide" button
         */
        public HideListener(JButton button) 
        {
            this.button = button;
        }

        /**
         * Hides the element the user typed into the text field.
         * 
         * @param e the ActionEvent that represents when the user pressed the 
         * "Hide" button
         */
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
                Luv.getLuv().getRegexModelFilter().addRegex(regex);
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

        /**
         * Tests for whether the specified string is already in the hidden list.
         * @param name the string the user entered to be hidden
         * @return whether or not the string is already present
         */
        protected boolean alreadyInList(String name) 
        {
            return listModel.contains(name);
        }

        /** Required by DocumentListener. */
        public void insertUpdate(DocumentEvent e) 
        {
            textField.setForeground(Color.BLACK);            
            enableButton();
        }

        /** Required by DocumentListener. */
        public void removeUpdate(DocumentEvent e) 
        {
            handleEmptyTextField(e);
        }

        /** Required by DocumentListener. */
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
    
    /** Required by ListSelectionListener. */ 
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
    
    /**
     * Creates an instance of a HideOrShowWindow.  
     */
    public void open()
    {
        if (frame == null)
        {
            frame = new JFrame("Hide/Show Nodes");

            frame.add(new HideOrShowWindow(Luv.getLuv().getProperties().getProperty(PROP_HIDE_SHOW_LIST, UNKNOWN)), BorderLayout.CENTER);
            frame.setSize(Luv.getLuv().getProperties().getDimension(PROP_HIDESHOWWIN_SIZE));
            frame.setLocation(Luv.getLuv().getProperties().getPoint(PROP_HIDESHOWWIN_LOC));

            frame.pack();
            frame.setVisible(true);
        }
        else if (!frame.isVisible())
            frame.setVisible(true);
    }
}
