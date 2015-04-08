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

import java.io.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.text.*;
import static gov.nasa.luv.Constants.*;

/**
 * The DebugWindow class displays debug and status information in a 
 * separate window.
 */

public class DebugWindow extends JFrame
{
      private JTextArea     debugArea;
      private JScrollPane   debugScrollPane;
      private JButton       clearAll;
      private JPanel        outer;
      private JToolBar      toolBar;

      /**
       * Constructs a DebugWindow
       */
      public DebugWindow()
      {
          init();
          addWindowListener(new WindowAdapter() {
                  public void windowClosing(WindowEvent winEvt) {
                      // TODO: Perhaps ask user if they want to save any unsaved files first.
                  }
              });
         
          // output stream to direct all text into the the debug text area
          DualOutputStream dos = new DualOutputStream(null); 
         
          // print stream that uses the directed output stream
          PrintStream ps = new PrintStream(dos);
         
          // redirect all system output into the debug text area
          // Comment these out for debugging
          // System.setErr(ps);
          // System.setOut(ps);
      }
      
      private void init() 
      {
          debugArea = new javax.swing.JTextArea();
          debugArea.setEditable(false);
          debugArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
         
          Action clearAction = 
              new AbstractAction("Clear Output")
              {
                  public void actionPerformed(ActionEvent actionEvent)
                  {
                      clearDebugArea();
                  }
              };
            
          clearAll = new JButton(clearAction);
          toolBar = new javax.swing.JToolBar();
          toolBar.setFloatable(false);      
          toolBar.add(clearAll);

          debugScrollPane = new javax.swing.JScrollPane();
          debugScrollPane.setViewportView(debugArea);
         
          outer = new javax.swing.JPanel();
          outer.setLayout(new java.awt.BorderLayout());
          outer.add(debugScrollPane, java.awt.BorderLayout.CENTER);
         
          getContentPane().add(outer, java.awt.BorderLayout.CENTER);
          getContentPane().add(toolBar, java.awt.BorderLayout.NORTH);
         
          setPreferredSize(Luv.getLuv().getSettings().getDimension(PROP_DBWIN_SIZE));
          setTitle("Debug Window");                
          setLocation(Luv.getLuv().getSettings().getPoint(PROP_DBWIN_LOC));
          pack();
      }

      // append a string to the debug window
      private void append(String str)
      {
         String eol = "\n";
         
         str = replaceAll(str, "\n\r", eol);
         str = replaceAll(str, "\r\n", eol);
         str = replaceAll(str, "\r", eol);

         debugArea.append(str);
         debugArea.setCaretPosition(debugArea.getText().length());
      }

      // a simple search and replace
      private static String replaceAll(String str, String oldTxt, String newTxt)
      {
         int idx = 0;
         while ((idx = str.indexOf(oldTxt)) != -1)
            str = str.substring(0, idx) + newTxt + str.substring(idx + oldTxt.length());
         return str;
      }
      
      // clear the debugging window of all text
      private void clearDebugArea()
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
      
    /**
     * The DualOutputStream class creates an output stream to fork data out 
     * to a file (the Debug Log File) and to the debugging window.
     */
    
    private class DualOutputStream extends OutputStream {
        // send output to both debug window and log file
        FileOutputStream logStream;
            
        /**
         * Constructs a DualOutputStream with the specified debug log file name.
         * @param logFilename name of the debug log file that is output to
         */
        public DualOutputStream (String logFilename) {
            super();
            try {
                if (logFilename != null)
                    logStream = new FileOutputStream(logFilename);
            }
            catch (FileNotFoundException ex) {
            }
        }
            
        public void write(byte[] b) 
            throws IOException
        {
            if (this.logStream != null)
                this.logStream.write(b);
            append(new String(b));
        }

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
