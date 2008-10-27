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
import java.awt.GridLayout;
import java.io.IOException;
import javax.swing.*;

import static gov.nasa.luv.Constants.*;


public class AboutWindow extends JPanel
{
    private static JFrame      frame;
    
    public AboutWindow() 
    {       
        super(new GridLayout(1,0));
    }

    public static void openAboutWindow() throws IOException, InterruptedException 
    {       
        if (frame != null)
            frame.setVisible(false);
        frame = new JFrame("About Luv Viewer");

        AboutWindow aboutPane = new AboutWindow();
            
        aboutPane.setLayout(new BorderLayout());
        
        JPanel logoPanel = new JPanel();
        
        JLabel aboutLogo = new JLabel(getIcon(ABOUT_LOGO));
        logoPanel.add(aboutLogo);
        aboutPane.add(logoPanel, BorderLayout.NORTH);
        
        JTextArea infoArea = new JTextArea();
        
        String productVersion  = "Product:\tLuv Viewer version 1.0\n";
        String website         = "Website:\thttp://plexil.wiki.sourceforge.net/Luv\n";        
        String javaVersion     = "Java:\t" + System.getProperty("java.version") + "; " + System.getProperty("java.vm.name") + " " + System.getProperty("java.vm.version") + "\n";
        String systemVersion   = "System:\t" + System.getProperty("os.name") + " version " + System.getProperty("os.version") + " running on " + System.getProperty("os.arch") + "\n";
        String userDirectory   = "Userdir:\t" + System.getProperty("user.dir") + "\n";
        
        infoArea.setText(productVersion + website + javaVersion + systemVersion + userDirectory);
        infoArea.setEditable(false);
        aboutPane.add(infoArea, BorderLayout.SOUTH);
        
        aboutPane.setOpaque(true);
        
        frame.setContentPane(aboutPane);
        frame.setBounds(20, 20, 1200, 500);

        frame.pack();
        frame.setVisible(true);
    }
    
    public static boolean isAboutWindowOpen()
    {
        if (frame != null)
            return frame.isVisible();
        else 
            return false;
    }
    
    public static void closeAboutWindow()
    {
        frame.setVisible(false);
    }
}
