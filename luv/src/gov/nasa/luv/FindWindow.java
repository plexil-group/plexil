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

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.ArrayList;
import java.util.Stack;

import javax.swing.*;

import static gov.nasa.luv.Constants.*;

public class FindWindow extends JPanel implements KeyListener
{
    private static JFrame       frame;  
    private static JComboBox    searchListHolder;
    private static String []    searchList; 
    
    private JPanel              leftHalf;
    private Box                 entryPanel;    
    private JLabel              message_to_user; 
    private Font                regularFont, italicFont;  
    private JTextField          searchListEditor;    
       
    private boolean             searchSet = false;
    private boolean             foundMatch = false;              
    private ArrayList<Stack>    foundNodes = new ArrayList<Stack>();
    private String              previousSearch = "";
    private int                 next = 0;
    
    public FindWindow() {}

    public FindWindow(String list) 
    {
        setLayout(new BoxLayout(this, BoxLayout.LINE_AXIS));

        add(createEntryField(list));
        add(createMessageDisplay());     
    }
    
    private Component createEntryField(String list) 
    {
        setSearchList(list);
        
        leftHalf = new JPanel() {
            public Dimension getMaximumSize() {
                Dimension pref = getPreferredSize();
                return new Dimension(Integer.MAX_VALUE, pref.height);
            }
        };
        
        leftHalf.setLayout(new BoxLayout(leftHalf, BoxLayout.Y_AXIS));
     
        if (searchList != null)
            searchListHolder = new JComboBox(searchList);
        else
            searchListHolder = new JComboBox();
        searchListHolder.setEditable(true);  
        searchListHolder.setFont(searchListHolder.getFont().deriveFont(Font.PLAIN, 12.0f));
        
        searchListEditor = (JTextField) searchListHolder.getEditor().getEditorComponent();
        searchListEditor.addKeyListener(this);        
        searchListEditor.setFont(searchListEditor.getFont().deriveFont(Font.PLAIN, 12.0f));
        
        entryPanel = Box.createVerticalBox(); 
        entryPanel.add(searchListHolder);                
        entryPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 0, 10));
        
        leftHalf.add(entryPanel);
        
        return leftHalf;
    }
    
    private JComponent createMessageDisplay() 
    {
        JPanel panel = new JPanel(new BorderLayout());
        
        message_to_user = new JLabel();
        message_to_user.setHorizontalAlignment(JLabel.CENTER);
        
        regularFont = message_to_user.getFont().deriveFont(Font.PLAIN, 12.0f);
        italicFont = regularFont.deriveFont(Font.ITALIC);
        
        message_to_user.setText(getMessage(-1));
        message_to_user.setFont(italicFont);

        panel.setBorder(BorderFactory.createEmptyBorder(5, 0, 5, 0));
        panel.add(new JSeparator(JSeparator.VERTICAL),  BorderLayout.LINE_START);
        panel.add(message_to_user, BorderLayout.CENTER);
        panel.setPreferredSize(new Dimension(300, 50));

        return panel;
    }
    
    public void keyTyped(KeyEvent e) 
    {
    }

    public void keyPressed(KeyEvent e) 
    {
        if (e.getKeyCode() == (KeyEvent.VK_ENTER) && !searchListEditor.getText().equals(""))
        {
            searchSet = true; 
            lookForNode();
            searchListHolder.requestFocusInWindow();
            searchListEditor.selectAll();
        }
    }

    public void keyReleased(KeyEvent e) 
    {
    }

    private void lookForNode() 
    {
        String text = searchListEditor.getText(); 

        newSearch(text);
        
        if (foundMatch)
        {
            showUserNextNode();
            message_to_user.setFont(regularFont);
        }
        else
        {
            message_to_user.setText(getMessage(-1));
            message_to_user.setFont(italicFont);
        }  
    }
    
    private void showUserNextNode()
    {
        if (next >= foundNodes.size())
        {   
            next = 0;
            TreeTableView.getCurrent().restartSearch();
        }

        Stack<String> node_path = new Stack<String>();
        
        Object[] obj = foundNodes.get(next).toArray();

        for (int i = 0; i < obj.length; i++)
        {
            node_path.push((String) obj[i]);
        }
            
        int currentRow = TreeTableView.getCurrent().showNode(node_path);        
        message_to_user.setText(getMessage(currentRow));
        next++;
    }
    
    private void newSearch(String text)
    {
        if (!text.equals(previousSearch))
        {
            Luv.getLuv().addSearchWord(text);
            leftHalf.remove(entryPanel);
            leftHalf.add(createEntryField(Luv.getLuv().getSearchList()));
            foundNodes.clear();
            next = 0;
            TreeTableView.getCurrent().restartSearch();
            previousSearch = text;  
            foundMatch = false;
            searchPlan(Luv.getLuv().getCurrentPlan(), text);
        }
    }
    
    private void searchPlan(Model model, String text)
    {
       if (model.isRoot() || model.getParent().isRoot())
       {
           if (model.getModelName().equals(text))
            {
                Stack<String> node_path = model.getPath(model);    
                foundMatch = true;
                foundNodes.add(node_path);                    
            }
       }
            
       for (Model child: model.getChildren())
       {
            if (!AbstractModelFilter.isModelFiltered(child))
            {
                if (child.getModelName().equals(text))
                {
                    Stack<String> node_path = child.getPath(child);    
                    foundMatch = true;
                    foundNodes.add(node_path);                    
                }
            }
            searchPlan(child, text);       
       }
    }

    private String getMessage(int row) 
    {
        StringBuffer sb = new StringBuffer();
        if (!searchSet) return "No search set";
        
        String name = (String) searchListHolder.getSelectedItem(); 
        
        if (!foundMatch)
        {
            sb.append("<html><p align=center>");
            sb.append("No matching nodes were found for <b>" + name + "</b>");
            sb.append("</p></html>");
        }   
        else
        {
            sb.append("<html><p align=center>");
            sb.append(foundNodes.size());
            if (foundNodes.size() > 1)
            {
                sb.append(" matches found for <b>" + name + "</b>");
            }
            else 
            {
                sb.append(" match found for <b>" + name + "</b>");
            }
            
            if (row != -1)
            {
                row++;
                int match = next + 1;
                sb.append("</p>Match: " + match + " Row: " + row);
            }
            sb.append("</p></html>");
        }

        return sb.toString();
    }
    
    private void setSearchList(String list)
    {
        String [] getList;
        
        if (!list.equals(UNKNOWN) && !list.equals(""))
        {
            getList = list.split(", ");
            searchList = new String[getList.length + 1];
            
            for (int i = 0; i < getList.length; i++)
            {
                searchList[i] = getList[i];
            }
        }      
    }
   
    public static void makeVisible(String list)
    {
        if (frame != null && frame.isVisible())
            frame.setVisible(false);
        frame = new JFrame("Find Node");
        if (!Luv.getLuv().getCurrentPlan().getPlanNameSansPath().equals(UNKNOWN))
            frame.setTitle("Find Node in " + Luv.getLuv().getCurrentPlan().getPlanNameSansPath());
     
        frame.add(new FindWindow(list));
        
        frame.setPreferredSize(Luv.getLuv().getProperties().getDimension(PROP_FINDWIN_SIZE));
        frame.setLocation(Luv.getLuv().getProperties().getPoint(PROP_FINDWIN_LOC));
        
        frame.pack();
        
        if (searchList != null)
            searchListHolder.setSelectedIndex(searchList.length - 1);
        
        frame.setVisible(true);  
    }
}
