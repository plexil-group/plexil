//original author Europa group
//modified for Plexil

package gov.nasa.luv;

import static gov.nasa.luv.Constants.PROP_DBWIN_SIZE;

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JFrame;

public class DebugHistoryWindow extends JFrame 
{
    private DebugHistoryPanel dbhp;

    /**
     * Constructs a DebugWindow
     */
    public DebugHistoryWindow()
    {
    	dbhp = new DebugHistoryPanel();       
       
       addWindowListener(new WindowAdapter() {
		public void windowClosing(WindowEvent winEvt) {
		    // Perhaps ask user if they want to save any unsaved files first.
		    LuvActionHandler.luvDebugHistoryWindowAction.actionPerformed(null);   
		}
	    });
        setPreferredSize(Luv.getLuv().getProperties().getDimension(PROP_DBWIN_SIZE));
        setTitle("Debug History Window");                               
	    pack();      
	    getContentPane().add(dbhp);
    }
     
}