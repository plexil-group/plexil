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

public class LuvSplashScreen extends JWindow 
{
     // splash screen variables
    
     private BorderLayout borderLayout1 = new BorderLayout();
     private JLabel imageLabel = new JLabel();
     private ImageIcon imageIcon;
     private static ImageIcon luvLogo = new ImageIcon(gov.nasa.luv.Luv.class.getResource("/resources/icons/LuvIcon.gif"));
     private static LuvSplashScreen splashScreen = new LuvSplashScreen(luvLogo);
     
     public LuvSplashScreen(ImageIcon imageIcon) 
     {
         this.imageIcon = imageIcon;
         try {
             jbInit();
         }
         catch(Exception ex) 
         {
             ex.printStackTrace();
         }
     }
     
     void jbInit() throws Exception 
     {
         imageLabel.setIcon(imageIcon);
         this.getContentPane().setLayout(borderLayout1);
         this.getContentPane().add(imageLabel, BorderLayout.CENTER);
         this.pack();
     }

     public void setScreenVisible(boolean b)
     {
         final boolean boo = b;
         SwingUtilities.invokeLater(new Runnable() 
         {
             public void run() 
             {
                 setVisible(boo);
             }
         });
     }     
           
      public static void enableSplashScreen(boolean splash)
      {
          if (splash)
          {
              splashScreen.setLocationRelativeTo(null);
              splashScreen.setScreenVisible(true);
          }
          else
              splashScreen.setScreenVisible(false);
      }
}
