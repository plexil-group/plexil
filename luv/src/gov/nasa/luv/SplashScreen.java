package gov.nasa.luv;

import javax.swing.*;
import java.awt.*;

public class SplashScreen extends JWindow 
{
  BorderLayout borderLayout1 = new BorderLayout();
  JLabel imageLabel = new JLabel();
  ImageIcon imageIcon;

  public SplashScreen(ImageIcon imageIcon) {
    this.imageIcon = imageIcon;
    try {
      jbInit();
    }
    catch(Exception ex) {
      ex.printStackTrace();
    }
  }

  // note - this class created with JBuilder
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
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        setVisible(boo);
      }
    });
  }
}
