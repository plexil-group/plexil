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

public class HideOrShowWindow extends JPanel implements ListSelectionListener
{
    private JFrame frame;
    private JList list;
    private DefaultListModel listModel;

    private static final String hideString = "Hide";
    private static final String showString = "Show";
    private JButton showButton;
    private JButton hideButton;
    private JTextField textField;
    private JLabel instructions;
    private JPanel buttonPane;
    private HideListener hideListener;

    public HideOrShowWindow() 
    {
        super(new BorderLayout());

        listModel = new DefaultListModel();
        
        String instructionText = "<html>\n" +
                "Type the full or partial name of the nodes you want to hide." +
                "<br>Use (<b>*</b>) wildcard as a prefix and/or suffix to select multiple nodes." +
                "<br>" + 
                "<br>For example:" +
                "<br>Type <b>Child*</b> to hide nodes Child<b>1</b>, Child<b>2</b>, and Child<b>3</b>" +
                "<br>Type <b>*Child</b> to hide nodes <b>a</b>Child, <b>b</b>Child, and <b>c</b>Child" +
                "<br>Type <b>*Child*</b> to hide nodes <b>a</b>Child<b>1</b>, <b>b</b>Child<b>2</b>, and <b>c</b>Child<b>3</b>";
        
        instructions = new JLabel(instructionText);
        instructions.setFont(new Font("Monospaced", Font.PLAIN, 12));
        JPanel instructionsPane = new JPanel();
        instructionsPane.add(instructions);

        list = new JList(listModel);
        list.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        list.setSelectedIndex(0);
        list.addListSelectionListener(this);
        list.setVisibleRowCount(5);
        JScrollPane listScrollPane = new JScrollPane(list);

        hideButton = new JButton(hideString);
        hideListener = new HideListener(hideButton);
        hideButton.setActionCommand(hideString);
        hideButton.addActionListener(hideListener);
        hideButton.setEnabled(false);      

        showButton = new JButton(showString);
        showButton.setActionCommand(showString);
        showButton.addActionListener(new ShowListener());

        textField = new JTextField("[Type node name here]", 10);
        textField.addActionListener(hideListener);
        textField.getDocument().addDocumentListener(hideListener);

        //Create a panel that uses BoxLayout.
        buttonPane = new JPanel();
        buttonPane.setLayout(new BoxLayout(buttonPane,
                                           BoxLayout.LINE_AXIS));
        buttonPane.add(showButton);
        buttonPane.add(Box.createHorizontalStrut(5));
        buttonPane.add(new JSeparator(SwingConstants.VERTICAL));
        buttonPane.add(Box.createHorizontalStrut(5));
        buttonPane.add(textField);
        buttonPane.add(hideButton);
        buttonPane.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));

        add(instructionsPane, BorderLayout.NORTH);
        add(listScrollPane, BorderLayout.CENTER);
        add(buttonPane, BorderLayout.PAGE_END);       
    }

    class ShowListener implements ActionListener 
    {
        public void actionPerformed(ActionEvent e) 
        {
            //This method can be called only if
            //there's a valid selection
            //so go ahead and remove whatever's selected.
            int index = list.getSelectedIndex();
            String regex = (String) listModel.remove(index);
            String formattedRegex = formatRegex(regex);
            Luv.getLuv().removeRegex(formattedRegex);

            int size = listModel.getSize();

            if (size == 0) 
            { 
                //Nobody's left, disable firing.
                showButton.setEnabled(false);
            } 
            else 
            { 
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
                Toolkit.getDefaultToolkit().beep();
                textField.requestFocusInWindow();
                textField.selectAll();
                return;
            }
            else
            {
                String formattedRegex = formatRegex(regex);
                Luv.getLuv().addRegex(formattedRegex);
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
            //If we just wanted to add to the end, we'd do this:
            //listModel.addElement(employeeName.getText());

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
    
    private String formatRegex(String regex)
    {  
        String formattedRegex = "";

        if (regex.endsWith("*"))
        {
            if (regex.startsWith("*"))
            {
                formattedRegex = regex.substring(1, regex.length() - 1);
                formattedRegex = ".*" + formattedRegex + ".*";
            }
            else
            {
                formattedRegex = regex.substring(0, regex.length() - 1);
                formattedRegex = "^" + formattedRegex + ".*";
            }
        }
        else if (regex.startsWith("*"))
        {
            formattedRegex = regex.substring(1, regex.length());
            formattedRegex = ".*" + formattedRegex;
        }
        else
            formattedRegex = regex;

        return formattedRegex;
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

    public void createHideOrShowWindow() 
    {
        frame = new JFrame("Hide/Show Nodes");

        frame.add(new HideOrShowWindow(), BorderLayout.CENTER);

        frame.setLocationRelativeTo(Luv.getLuv());
        frame.pack();  
    }
}
