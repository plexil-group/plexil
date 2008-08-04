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

package src.gov.nasa.luv;

import java.util.Vector;

import javax.swing.JLabel;
import javax.swing.JWindow;
import javax.swing.JFrame;
import javax.swing.JButton;
import javax.swing.BoxLayout;

import java.awt.Font;
import java.awt.Frame;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.event.ActionEvent;

import static java.awt.event.KeyEvent.*;
import static java.awt.GridBagConstraints.*;

import static src.gov.nasa.luv.Constants.*;

/** A Window for displaying informaion about a given node. */

class NodeInfoWindow extends JFrame
{
      /** model which contains node it question */

      Model model;

      /** model listeners */

      Vector<Model.ChangeListener> modelListeners = 
         new Vector<Model.ChangeListener>();

      /** Constructs and displays a NodeInfoWindow object. */
      
      public NodeInfoWindow(Frame parent, Model model)
      {
         setModel(model);
         setVisible(true);
      }

      /** Set the model for this window. 
       * 
       * @param model the model that will be shown in this window.
       */

      public void setModel(Model model)
      {
         // remove old model listeners

         for (Model.ChangeListener listener: modelListeners)
            model.remove(listener);
         modelListeners.clear();

         // record new model

         this.model = model;
         
         // get the current size of the window

         Dimension size = getContentPane().getSize();

         // reconstruct window with new model

         getContentPane().removeAll();
         constructWindow();

         // prep window for re-display

         if (isVisible())
            getContentPane().setPreferredSize(size);
         else
            repaint();
         pack();
      }
      
      /** Construct element of this window. */

      public void constructWindow()
      {
         GridBagLayout gridBag = new GridBagLayout();
         GridBagConstraints c = new GridBagConstraints();
         setLayout(gridBag);
         //setLayout(new BoxLayout(getContentPane(), BoxLayout.Y_AXIS));

         // add node type icon

         JLabel icon = new JLabel(getIcon(model.getProperty(NODETYPE_ATTR)));
         c.anchor = FIRST_LINE_START;
         c.weighty = 1;
         c.weightx = 0;
         c.gridx = 0;
         c.gridy = 0;
         add(icon, c);
         
         JLabel name = new JLabel(model.getProperty(MODEL_NAME));
         name.setFont(name.getFont().deriveFont(30f).deriveFont(Font.BOLD));
         c.anchor = FIRST_LINE_START;
         c.weightx = 1;
         c.gridx = 1;
         c.gridheight = 2;
         c.gridwidth = 2;
         add(name, c);
         
         // add colums for each of condtions

         c.gridheight = 1;
         c.gridy++;
         for (final String condition: ALL_CONDITIONS)
         {
            // updae to next line

            c.gridy++;

            // add condition label

            c.gridx = 1;
            c.anchor = LINE_END;
            final JLabel label = new JLabel(condition + ":  ");
            label.setFont(label.getFont().deriveFont(20f).deriveFont(Font.BOLD));
            add(label, c);

            // add condition value

            c.gridx = 3;
            c.anchor = LINE_START;
            final JLabel value = new JLabel("value");
            add(value, c);

            // add model listener

            model.addChangeListener(new Model.ChangeAdapter()
               {
                     @Override 
                     public void propertyChange(Model model, String property)
                     {
                        if (property.equals(condition))
                        {
                           value.setText(model.getProperty(condition));
                           value.repaint();
                        }
                     }
               });
         }
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
