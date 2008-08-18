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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

// Optional pop up window showing debug information that is normally printed in shell 
import javax.swing.JMenuBar;
import javax.swing.text.BadLocationException;
import javax.swing.text.Document;

public class ShellDebugWindow extends JFrame 
{
      // swing elements Variables declaration

      private javax.swing.JTextArea debugArea;
      private javax.swing.JScrollPane debugScrollPane;
      private javax.swing.JButton clearAll;
      private javax.swing.JPanel outer;
      private javax.swing.JToolBar toolBar;

      /** Construct a DebugWindow object. */

      public ShellDebugWindow(final JFrame owner)
      {
         initComponents();

         // when this frame get's focus, barrow the menu bar from parent
         
         addWindowFocusListener(new WindowAdapter()
            {
                  public void windowGainedFocus(WindowEvent e)
                  {
                     JMenuBar mb = owner.getJMenuBar();
                     if (mb != null) setJMenuBar(mb);
                  }
            });
         
         // set font
         
         debugArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
         
      }      
            
      // clear the debugging window of all text
      
      private void clearDebugArea ()
      {
         try 
         {
	    Document doc = debugArea.getDocument();
	    doc.remove(0, doc.getLength());
         }
         catch (BadLocationException ex) 
         {
         }
      }
            /** This method is called from within the constructor to
       * initialize the form.
       */

      private void initComponents() 
      {
         outer = new javax.swing.JPanel();
         debugScrollPane = new javax.swing.JScrollPane();
         debugArea = new javax.swing.JTextArea();
         toolBar = new javax.swing.JToolBar();
         
         Action clearAction = 
	    new AbstractAction("Clear Output")
	    {
                  public void actionPerformed(ActionEvent actionEvent)
                  {
                     clearDebugArea();
                  }
	    };
            
         clearAll = new JButton(clearAction);

         outer.setLayout(new java.awt.BorderLayout());
         
         debugArea.setEditable(false);
         
         debugScrollPane.setViewportView(debugArea);
         
         outer.add(debugScrollPane, java.awt.BorderLayout.CENTER);
         
         getContentPane().add(outer, java.awt.BorderLayout.CENTER);
         
         toolBar.setFloatable(false);
         
         toolBar.add(clearAll);
         
         getContentPane().add(toolBar, java.awt.BorderLayout.NORTH);
         
         java.awt.Dimension screenSize = java.awt.Toolkit.getDefaultToolkit().getScreenSize();
         setBounds((screenSize.width-500)/2, (screenSize.height-300)/2, 500, 300);
      }
}
