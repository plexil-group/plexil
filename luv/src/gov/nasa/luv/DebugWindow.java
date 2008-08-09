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

import static gov.nasa.luv.Constants.*;

import java.io.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.text.*;

public class DebugWindow extends JFrame
{
      // swing elements variable declaration

      private JTextArea debugArea;
      private JScrollPane debugScrollPane;
      private JButton clearAll;
      private JPanel debugPanel;
      private JToolBar toolBar;
     // private FileWriter debugLogFile = new FileWriter(DEBUG_LOG_FILE);
     // private BufferedWriter out = new BufferedWriter(debugLogFile);
       

      /** Construct a DebugWindow object. */

      public DebugWindow(final JFrame owner) throws IOException
      {
         initComponents();
         
         // construct an output stream that directs all text into the the debug text area      
         // construct a print stream that uses the directed output stream

         DualOutputStream dos = new DualOutputStream(null);
         PrintStream ps = new PrintStream(dos);
         
         // redirect all system output into the debug text area
         
         System.setErr(ps);
         System.setOut(ps);
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
         debugPanel = new JPanel();
         debugScrollPane = new JScrollPane();
         debugArea = new JTextArea();
         toolBar = new JToolBar();                    
         clearAll = new JButton(clearAction);

         debugPanel.setLayout(new java.awt.BorderLayout());
         debugPanel.add(debugScrollPane, java.awt.BorderLayout.CENTER);
         getContentPane().add(debugPanel, java.awt.BorderLayout.CENTER);
         
         debugArea.setEditable(false);
         debugArea.setFont(new Font(FONT, Font.PLAIN, 12));
         debugScrollPane.setViewportView(debugArea);
          
         toolBar.setFloatable(false);        
         toolBar.add(clearAll);         
         getContentPane().add(toolBar, java.awt.BorderLayout.NORTH);
         
         java.awt.Dimension screenSize = java.awt.Toolkit.getDefaultToolkit().getScreenSize();
         setBounds((screenSize.width-500)/2, (screenSize.height-300)/2, 500, 300);
      }
      
      Action clearAction = new AbstractAction(CLEAR_SCREEN)
      {
          public void actionPerformed(ActionEvent actionEvent)
          {
              clearDebugArea();
          }
      };
      
            // append a string to the debug window
      
      public void append(String str)
      {
         String eol = "\n";
         
         str = replaceAll(str, "\n\r", eol);
         str = replaceAll(str, "\r\n", eol);
         str = replaceAll(str, "\r", eol);
         
         debugArea.append(str);
         debugArea.setCaretPosition(debugArea.getText().length());
         
        /* try{

            out.write(str);     
            //Close the output stream
            out.close();
            
         }catch (Exception e){ //Catch exception if any
              System.err.println("Error: " + e.getMessage());
         }*/
      }

      // a simple search and replace

      public static String replaceAll(String str, String oldTxt, String newTxt)
      {
         int idx = 0;
         while ((idx = str.indexOf(oldTxt)) != -1)
            str = str.substring(0, idx) + newTxt + str.substring(idx + oldTxt.length());
         return str;
      }
      // an output streem which forks data out to a file and to the debugging window

      public class DualOutputStream extends OutputStream
      {
            //send output to both debug window and log file

            FileOutputStream logStream;
            
            public DualOutputStream (String logFilename)
            {
               super();
               try 
               {
                  if (logFilename != null)
                     logStream = new FileOutputStream(logFilename);
               }
               catch (FileNotFoundException ex) 
               {
               }
            }
            
        @Override
            public void write(byte[] b) 
               throws IOException
            {
               if (this.logStream != null)
                  this.logStream.write(b);
               append(new String(b));
            }
        @Override
            public void write(byte[] b, int off, int len) 
               throws IOException
            {
               if (this.logStream != null) 
                  this.logStream.write(b, off, len);
               append(new String(b, off, len));
            }
            public void write(int b) 
               throws IOException
            {
               if (this.logStream != null) 
                  this.logStream.write(b);
               byte[] ba = {(byte)b}; 
               append(new String(ba));
            }
      }
}
