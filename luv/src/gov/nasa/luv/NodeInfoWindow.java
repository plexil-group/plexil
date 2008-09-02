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

import java.awt.Color;
import java.util.Vector;

import javax.swing.JLabel;
import javax.swing.JFrame;

import java.awt.Font;
import java.awt.Frame;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.event.ActionEvent;

import static java.awt.event.KeyEvent.*;
import static java.awt.GridBagConstraints.*;

import static gov.nasa.luv.Constants.*;

/** A Window for displaying informaion about a given node. */

class NodeInfoWindow extends JFrame
{
      /** model which contains node it question */

      Model model;

      /** model listeners */

      Vector<Model.ChangeListener> modelListeners = new Vector<Model.ChangeListener>();

      /** Constructs and displays a NodeInfoWindow object. */
      
      public NodeInfoWindow(Frame parent, Model model, String name)
      {
         setModel(model, name);
      }
      
      public Model getModel()
      {
          return model;
      }

      /** Set the model for this window. 
       * 
       * @param model the model that will be shown in this window.
       */

      public void setModel(Model model, String name)
      {
          // set title
          
          this.setTitle(name);
          
         // remove old model listeners

         for (Model.ChangeListener listener: modelListeners)
            model.remove(listener);
         modelListeners.clear();

         // record new model

         this.model = model;

         // reconstruct window with new model

         getContentPane().removeAll();
         constructWindow();

         // re-display
         
         repaint();
         pack();
         setVisible(true);
      }
      
      /** Construct element of this window. */

      public void constructWindow()
      {
         GridBagLayout gridBag = new GridBagLayout();
         final GridBagConstraints c = new GridBagConstraints();
         setLayout(gridBag);
         
         // add columns for each of conditions

         c.gridheight = 1;
         c.gridy++;
         
         if (model.conditionMap.isEmpty())
         {
             c.gridy++;
             c.gridy++;
             add(new JLabel("No conditions for this node"), c);
         }
         else
         {               
             for (final String condition: ALL_CONDITIONS)
             {                           
                // update to next line

                c.gridy++;

                // add condition value

                c.gridx = 3;
                c.anchor = LINE_START;
                final JLabel value = new JLabel(UNKNOWN);
                value.setText(getConditionElements(condition));
                if (!value.getText().equals(UNKNOWN))
                {
                    value.setForeground(Color.GRAY);
                    add(value, c);
                }

                // add condition label

                c.gridx = 1;
                c.anchor = LINE_START;
                final JLabel label = new JLabel(getConditionName(condition) + " Condition:  ");               
                if (!value.getText().equals(UNKNOWN))
                {
                    label.setFont(label.getFont().deriveFont(15f).deriveFont(Font.BOLD));
                    label.setForeground(Color.GRAY);
                    add(label, c);   
                }
                
                // add result label
                
                c.gridx = 2;
                c.anchor = LINE_START;
                final JLabel status = new JLabel("< Condition Status >         ");
                if (!value.getText().equals(UNKNOWN))
                {
                    status.setForeground(Color.GRAY);
                    add(status, c);
                }

                // add model listener

                model.addChangeListener(new Model.ChangeAdapter()
                   {
                         @Override 
                         public void propertyChange(Model model, String property)
                         {
                            if (property.equals(condition))
                            {                               
                               if (!value.getText().equals(UNKNOWN))
                               {
                                   c.gridx = 4;
                                   c.anchor = LINE_START;
                                   status.setText("              " + model.getProperty(condition) + "              ");
                                   status.setForeground(Color.BLUE.darker());
                                   status.repaint();
                               }
                            }
                         }
                   });
             }
         }
      }
      
      public String getConditionElements(String condition)
      {
            String conditionValue = "";
            int conditionNum = getConditionNum(condition);
          
            if (model.conditionMap.get(conditionNum) != null)
            {
                for (int i = 0; i < model.conditionMap.get(conditionNum).size(); i++)
                {
                    String var = model.conditionMap.get(conditionNum).get(i).toString();  
                    conditionValue += " " + var;
                    if (i + 1 >= model.conditionMap.get(conditionNum).size())
                        if (conditionValue.substring(conditionValue.length() - 2, conditionValue.length()).equals("&&") ||
                            conditionValue.substring(conditionValue.length() - 2, conditionValue.length()).equals("||") ||
                            conditionValue.substring(conditionValue.length() - 2, conditionValue.length()).equals("^ ") ||
                            conditionValue.substring(conditionValue.length() - 2, conditionValue.length()).equals("! "))
                            conditionValue = conditionValue.substring(0, (conditionValue.length() - 2));
                }
                return conditionValue;
            }  
            return UNKNOWN;
      }

      /** Action to close this window. */

      LuvAction closeWindowAction = new LuvAction(
         "Close", "Close this Window.", VK_ESCAPE)
         {
               public void actionPerformed(ActionEvent e)
               {
                  setVisible(false);
               }
         };
}
