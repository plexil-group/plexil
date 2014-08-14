/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

import java.awt.Point;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Toolkit;
import javax.swing.ImageIcon;
import java.util.HashMap;
import java.util.Vector;

/**
 * The Constants class provides a repository for all constants in the 
 * Luv application.
 */

public class Constants
{     
	  public static final String PLEXIL_VERSION = "Version 2.5\n";
	  public static final String PLEXIL_COPYRIGHT = "Copyright 2006-2012 Universities Space Research Association\n";
	  public static final String PLEXIL_WEBSITE = "plexil.sourceforge.net";
      /** Represents the location of the Luv application properties file. */      
      public static final String    PROPERTIES_FILE_LOCATION = System.getProperty("user.home") + System.getProperty("file.separator") + ".luv";
      
      ////////// Info stored in the Luv application properties file  //////////
      
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilExec plan. */      
      public static final String    PROP_FILE_EXEC_RECENT_PLAN_BASE = "file.exec.recent-plan-";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilExec script. */
      public static final String    PROP_FILE_EXEC_RECENT_CONFIG_BASE = "file.exec.recent-config-";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilExec plan directory. */
      public static final String    PROP_FILE_EXEC_RECENT_PLAN_DIR = "file.exec.recent-plan-directory";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilExec script directory. */
      public static final String    PROP_FILE_EXEC_RECENT_CONFIG_DIR = "file.exec.recent-config-directory";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilExec library. */  
      public static final String    PROP_FILE_EXEC_RECENT_LIB_BASE  = "file.exec.recent-lib-";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilExec library directory. */  
      public static final String    PROP_FILE_EXEC_RECENT_LIB_DIR = "file.exec.recent-library-directory";
      
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilTest plan. */      
      public static final String    PROP_FILE_TEST_RECENT_PLAN_BASE = "file.test.recent-plan-";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilTest script. */
      public static final String    PROP_FILE_TEST_RECENT_SCRIPT_BASE = "file.test.recent-script-";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilTest plan directory. */
      public static final String    PROP_FILE_TEST_RECENT_PLAN_DIR = "file.test.recent-plan-directory";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilTest script directory. */
      public static final String    PROP_FILE_TEST_RECENT_SCRIPT_DIR = "file.test.recent-script-directory";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilTest library. */  
      public static final String    PROP_FILE_TEST_RECENT_LIB_BASE  = "file.test.recent-lib-";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilTest library directory. */  
      public static final String    PROP_FILE_TEST_RECENT_LIB_DIR = "file.test.recent-library-directory";
      
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilSim plan. */      
      public static final String    PROP_FILE_SIM_RECENT_PLAN_BASE = "file.sim.recent-plan-";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilSim script. */
      public static final String    PROP_FILE_SIM_RECENT_SCRIPT_BASE = "file.sim.recent-script-";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilSim plan directory. */
      public static final String    PROP_FILE_SIM_RECENT_PLAN_DIR = "file.sim.recent-plan-directory";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilSim script directory. */
      public static final String    PROP_FILE_SIM_RECENT_SCRIPT_DIR = "file.sim.recent-script-directory";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilSim library. */  
      public static final String    PROP_FILE_SIM_RECENT_LIB_BASE  = "file.sim.recent-lib-";
      /** Stored in the Luv application properties file, this represents the location of the most recent PlexilSim library directory. */  
      public static final String    PROP_FILE_SIM_RECENT_LIB_DIR = "file.sim.recent-library-directory";            
      
      /** Stored in the Luv application properties file, this represents the number of Plexil plan executions Luv will save. */ 
      public static final String    PROP_FILE_RECENT_COUNT = "file.recent-count";
      /** Stored in the Luv application properties file, this is the number of Plexil plan executions Luv will save. */ 
      public static final int       PROP_FILE_RECENT_COUNT_DEF = 9;
      /** Stored in the Luv application properties file, this represents the number of characters the viewer will allow arrays to display for arrays. */ 
      public static final String    PROP_ARRAY_MAX_CHARS = "array.max.chars";
      /** Stored in the Luv application properties file, this is the number of characters the viewer will allow arrays to display for arrays. */ 
      public static final int       PROP_ARRAY_MAX_CHARS_DEF = 40;
      /** Stored in the Luv application properties file, this represents the server port number. */ 
      public static final String    PROP_NET_SERVER_PORT      = "net.server.port";
      /** Stored in the Luv application properties file, this is the server port number. */ 
      public static final int       PROP_NET_SERVER_PORT_DEF  = 9787;
      /** Stored in the Luv application properties file, this represents the list of Plexil nodes that hidden by the HideOrShowWindow. */ 
      public static final String    PROP_HIDE_SHOW_LIST = "hide.show.list";  
      /** Stored in the Luv application properties file, this represents the list of Plexil nodes previously searched for in the FindWindow. */ 
      public static final String    PROP_SEARCH_LIST = "search.list";   
      /** Stored in the Luv application properties file, this represents the location of the Luv application window. */ 
      public static final String    PROP_WIN_LOC        = "window.location";
      /** Stored in the Luv application properties file, this is the location of the Luv application window. */ 
      public static final Point     PROP_WIN_LOC_DEF    = new Point(100, 100);
      /** Stored in the Luv application properties file, this represents the size of the Luv application window. */ 
      public static final String    PROP_WIN_SIZE       = "window.size";
      /** Stored in the Luv application properties file, this is the size of the Luv application window. */ 
      public static final Dimension PROP_WIN_SIZE_DEF   = new Dimension(1200, 650);
      /** Stored in the Luv application properties file, this represents the background color of the Luv application window. */
      public static final String    PROP_WIN_BCLR       = "window.background";
      /** Stored in the Luv application properties file, this is the background color of the Luv application window. */ 
      public static final Color     PROP_WIN_BCLR_DEF   = Color.WHITE;
      /** Stored in the Luv application properties file, this represents the location of the DebugWindow. */
      public static final String    PROP_DBWIN_LOC            = "dbwindow.location";
      /** Stored in the Luv application properties file, this is the location of the DebugWindow. */
      public static final Point     PROP_DBWIN_LOC_DEF        = new Point(100, 750);
      /** Stored in the Luv application properties file, this represents the size of the DebugWindow. */ 
      public static final String    PROP_DBWIN_SIZE           = "dbwindow.size";
      /** Stored in the Luv application properties file, this is the size of the DebugWindow. */ 
      public static final Dimension PROP_DBWIN_SIZE_DEF       = new Dimension(1200, 300);     
      /** Stored in the Luv application properties file, this represents the location of the NodeInfoWindow. */
      public static final String    PROP_NODEINFOWIN_LOC      = "niwindow.location";
      /** Stored in the Luv application properties file, this is the location of the NodeInfoWindow. */
      public static final Point     PROP_NODEINFOWIN_LOC_DEF  = new Point(300, 300);
      /** Stored in the Luv application properties file, this represents the size of the NodeInfoWindow. */ 
      public static final String    PROP_NODEINFOWIN_SIZE     = "niwindow.size";
      /** Stored in the Luv application properties file, this is the size of the NodeInfoWindow. */
      public static final Dimension PROP_NODEINFOWIN_SIZE_DEF = new Dimension(900, 300);     
      /** Stored in the Luv application properties file, this represents the location of the FindWindow. */
      public static final String    PROP_FINDWIN_LOC          = "fwindow.location";
      /** Stored in the Luv application properties file, this is the location of the FindWindow. */
      public static final Point     PROP_FINDWIN_LOC_DEF      = new Point(500, 300);
      /** Stored in the Luv application properties file, this represents the size of the FindWindow. */ 
      public static final String    PROP_FINDWIN_SIZE         = "fwindow.size";
      /** Stored in the Luv application properties file, this is the size of the FindWindow. */
      public static final Dimension PROP_FINDWIN_SIZE_DEF     = new Dimension(400, 100);     
      /** Stored in the Luv application properties file, this represents the location of the HideOrShowWindow. */
      public static final String    PROP_HIDESHOWWIN_LOC      = "hswindow.location";
      /** Stored in the Luv application properties file, this is the location of the HideOrShowWindow. */
      public static final Point     PROP_HIDESHOWWIN_LOC_DEF  = new Point(300, 300);
      /** Stored in the Luv application properties file, this represents the size of the HideOrShowWindow. */ 
      public static final String    PROP_HIDESHOWWIN_SIZE     = "hswindow.size";
      /** Stored in the Luv application properties file, this is the size of the HideOrShowWindow. */
      public static final Dimension PROP_HIDESHOWWIN_SIZE_DEF = new Dimension(900, 700); 
      /** Stored in the Luv application properties file, this represents the location of the DebugCFGWindow. */
      public static final String    PROP_CFGWIN_LOC           = "cfgwindow.location";
      /** Stored in the Luv application properties file, this is the location of the DebugCFGWindow. */
      public static final Point     PROP_CFGWIN_LOC_DEF       = new Point(300, 300);
      /** Stored in the Luv application properties file, this represents the size of the DebugCFGWindow. */ 
      public static final String    PROP_CFGWIN_SIZE          = "cfgwindow.size";
      /** Stored in the Luv application properties file, this is the size of the DebugCFGWindow. */
      public static final Dimension PROP_CFGWIN_SIZE_DEF      = new Dimension(900, 700);
      
      //////////// Port configuration information /////////////
	  /** Represents the location of the Luv port temp file. */      
      public static final String    PORT_TEMP_FILE_LOCATION = System.getProperty("file.separator") + "tmp" + System.getProperty("file.separator");    

	  /** Represents the smallest port value. */      
      public static final int    PORT_MIN = 65400;    

      /** Represents the maximum port value. */      
      public static final int    PORT_MAX = 65420;
      
      ////////////Executive configuration information /////////////
      
      public static final int	PLEXIL_EXEC = 1;
      
      public static final int	PLEXIL_TEST = 2;
      
      public static final int	PLEXIL_SIM = 3;
      
      //////////// Various constant info used within this program /////////////

      /** Represents the "PLEXIL_HOME" environment variable. */
      public static final String    PLEXIL_HOME = System.getenv("PLEXIL_HOME");
      
      /** Represents the "test executive" for PLEXIL. */ 
      public static final String    TEST_EXEC = "test-exec_g_rt";
      
      /** Represents the "universal executive" for PLEXIL. */ 
      public static final String    UE_EXEC = "universalExec";

      /** Represents the "test executive" for PLEXIL. */ 
      public static final String    UE_TEST_EXEC = "TestExec";
      
      /** Represents the script for running the "Universal executive" for PLEXIL. */ 
      public static final String    UE_SCRIPT = "plexilexec";
      
      /** Represents the script for running the "Test executive" for PLEXIL. */ 
      public static final String    TE_SCRIPT = "plexiltest";
      
      /** Represents the script for running the "Plexil Simulator (includes Universal Exec)" for PLEXIL. */ 
      public static final String    SIM_SCRIPT = "plexilsim";
      
      /** Represents the script path for running the "Universal executive" for PLEXIL. */ 
      public static final String    RUN_UE_EXEC = PLEXIL_HOME +
                                              System.getProperty("file.separator") + 
                                              "scripts" + 
                                              System.getProperty("file.separator") + 
                                              UE_SCRIPT;
      
      /** Represents the script path for running the "Test executive" for PLEXIL. */ 
      public static final String    RUN_TEST_EXEC = PLEXIL_HOME +
                                              System.getProperty("file.separator") + 
                                              "scripts" + 
                                              System.getProperty("file.separator") + 
                                              TE_SCRIPT;
      
      /** Represents the script path for running the "Plexil Simulator" for PLEXIL. */ 
      public static final String    RUN_SIMULATOR = PLEXIL_HOME +
                                              System.getProperty("file.separator") + 
                                              "scripts" + 
                                              System.getProperty("file.separator") + 
                                              SIM_SCRIPT;
      
      /** Represents the Luv application debug log file. Note this file is not referenced yet in the DebugWindow class. */ 
      public static final String    DEBUG_LOG_FILE = System.getProperty("user.home") + 
                                                     System.getProperty("file.separator") + 
                                                     "luv.log";      
      
      /** Represents the python script that is used to create a complete list of debug flag list. */ 
      public static final String    PYTHON_SCRIPT = PLEXIL_HOME +
                                                    System.getProperty("file.separator") +
                                                    "scripts" + 
                                                    System.getProperty("file.separator") +
                                                    "debugFlagCollector.py";
      
      public static final String    PORT_CHECK_UTIL_RHEL5 = System.getProperty("file.separator") + "usr"
      											+ System.getProperty("file.separator") + "sbin"
      											+ System.getProperty("file.separator") + "lsof -i";
      
      public static final String    PORT_CHECK_UTIL_UNIX = "lsof -i";
      
      /** Represents the complete list of debug flags. */ 
      public static final String    COMPLETE_FLAG_LIST = PLEXIL_HOME +
                                                         System.getProperty("file.separator") +
                                                         "doc" + 
                                                         System.getProperty("file.separator") +
                                                         "CompleteDebugFlags.cfg";
      
      /** Represents the user defined list of debug flags. */
      public static final String    DEBUG_CFG_FILE = PLEXIL_HOME +
      												 System.getProperty("file.separator") +
      												 "viewers" +
                                                     System.getProperty("file.separator") +
                                                     "pv" + 
                                                     System.getProperty("file.separator") +
                                                     "Debug.cfg";

      /** Represents the location of the Plexil node icon files. */
      public static final String    ICONS_DIR = "resources" + 
                                                System.getProperty("file.separator") + 
                                                "icons" + 
                                                System.getProperty("file.separator");
      
      /** Plexil node icon file names */
      public static final String    IF_ICO  = "ACTION_IF.gif";
      public static final String    THEN_ICO  = "ACTION_THEN.gif";
      public static final String    ELSE_ICO  = "ACTION_ELSE.gif";
      public static final String    WHILE_ICO  = "ACTION_WHILE.gif";
      public static final String    TRY_ICO  = "ACTION_TRY.gif";
      public static final String    FOR_ICO  = "ACTION_FOR.gif";
      public static final String    SEQ_ICO  = "ACTION_SEQ.gif";
      public static final String    UNCHKD_SEQ_ICO  = "ACTION_UNCHECKED_SEQ.gif";
      public static final String    CONCURRENCE_ICO  = "ACTION_CONCURRENCE.gif";

      public static final String    THEN_LIST_ICO  = "THEN_NODE_LIST.gif";
      public static final String    THEN_ASSN_ICO  = "THEN_NODE_A.gif";
      public static final String    THEN_CMD_ICO  = "THEN_NODE_C.gif";
      public static final String    THEN_EMPTY_ICO  = "THEN_NODE_E.gif";
      public static final String    THEN_LIB_ICO  = "THEN_NODE_L.gif";
      public static final String    THEN_UPDATE_ICO  = "THEN_NODE_U.gif";

      public static final String    THEN_IF_ICO  = "THEN_ACTION_IF.gif";
      public static final String    THEN_WHILE_ICO  = "THEN_ACTION_W.gif";
      public static final String    THEN_FOR_ICO  = "THEN_ACTION_FOR.gif";
      public static final String    THEN_TRY_ICO  = "THEN_ACTION_TRY.gif";
      public static final String    THEN_SEQ_ICO  = "THEN_ACTION_SEQ.gif";
      public static final String    THEN_UNCHKD_SEQ_ICO  = "THEN_ACTION_UNCHKD_SEQ.gif";
      public static final String    THEN_CONCURRENCE_ICO  = "THEN_ACTION_CONC.gif";

      public static final String    ELSE_LIST_ICO  = "ELSE_NODE_LIST.gif";
      public static final String    ELSE_ASSN_ICO  = "ELSE_NODE_A.gif";
      public static final String    ELSE_CMD_ICO  = "ELSE_NODE_C.gif";
      public static final String    ELSE_EMPTY_ICO  = "ELSE_NODE_E.gif";
      public static final String    ELSE_LIB_ICO  = "ELSE_NODE_L.gif";
      public static final String    ELSE_UPDATE_ICO  = "ELSE_NODE_U.gif";

      public static final String    ELSE_IF_ICO  = "ELSE_ACTION_IF.gif";
      public static final String    ELSE_WHILE_ICO  = "ELSE_ACTION_W.gif";
      public static final String    ELSE_FOR_ICO  = "ELSE_ACTION_FOR.gif";
      public static final String    ELSE_TRY_ICO  = "ELSE_ACTION_TRY.gif";
      public static final String    ELSE_SEQ_ICO  = "ELSE_ACTION_SEQ.gif";
      public static final String    ELSE_UNCHKD_SEQ_ICO  = "ELSE_ACTION_UNCHKD_SEQ.gif";
      public static final String    ELSE_CONCURRENCE_ICO  = "ELSE_ACTION_CONC.gif";
      
      public static final String    ABOUT_SCREEN_ICO  = "LUV_plexil-logo.jpg";//LUV_ICON.gif
      public static final String    START_SCREEN_ICO  = "LUV_SPLASH_SCREEN.jpg";//LUV_SPLASH_SCREEN.gif
      public static final String    NODELIST_ICO_NAME = "NODE_LIST.gif";
      public static final String    COMMAND_ICO_NAME  = "NODE_COMMAND.gif";
      public static final String    ASSN_ICO_NAME     = "NODE_ASSN.gif";
      public static final String    EMPTY_ICO_NAME    = "NODE_EMPTY.gif";
      public static final String    FUNCALL_ICO_NAME  = "NODE_FUNCTION.gif";
      public static final String    UPDATE_ICO_NAME   = "NODE_UPDATE.gif";
      public static final String    LIBCALL_ICO_NAME  = "NODE_LIB_CALL.gif";    
  
      private static HashMap<String, ImageIcon> iconLut = new HashMap<String, ImageIcon>()
      {
         {
            add(NODELIST,        NODELIST_ICO_NAME);
            add(COMMAND,         COMMAND_ICO_NAME);
            add(ASSN,            ASSN_ICO_NAME);
            add(EMPTY,           EMPTY_ICO_NAME);
            add(FUNCCALL,        FUNCALL_ICO_NAME);
            add(UPDATE,          UPDATE_ICO_NAME);
            add(LIBRARYNODECALL, LIBCALL_ICO_NAME);
            add(START_LOGO,      START_SCREEN_ICO);
            add(ABOUT_LOGO,      ABOUT_SCREEN_ICO);

            add(IF,      IF_ICO);
            add(THEN, THEN_ICO);
            add(ELSE, ELSE_ICO);
            add(WHILE, WHILE_ICO);
            add(FOR, FOR_ICO);
            add(TRY, TRY_ICO);
            add(SEQ, SEQ_ICO);
            add(UNCHKD_SEQ, UNCHKD_SEQ_ICO);
            add(CONCURRENCE, CONCURRENCE_ICO);

            add(THEN_EMPTY, THEN_EMPTY_ICO);
            add(THEN_CMD, THEN_CMD_ICO);
            add(THEN_ASSN, THEN_ASSN_ICO);
            add(THEN_LIST, THEN_LIST_ICO);
            add(THEN_LIB, THEN_LIB_ICO);
            add(THEN_UPDATE, THEN_UPDATE_ICO);

            add(THEN_IF, THEN_IF_ICO);
            add(THEN_WHILE, THEN_WHILE_ICO);
            add(THEN_FOR, THEN_FOR_ICO);
            add(THEN_TRY, THEN_TRY_ICO);
            add(THEN_SEQ, THEN_SEQ_ICO);
            add(THEN_UNCHKD_SEQ, THEN_UNCHKD_SEQ_ICO);
            add(THEN_CONCURRENCE, THEN_CONCURRENCE_ICO);

            add(ELSE_EMPTY, ELSE_EMPTY_ICO);
            add(ELSE_CMD, ELSE_CMD_ICO);
            add(ELSE_ASSN, ELSE_ASSN_ICO);
            add(ELSE_LIST, ELSE_LIST_ICO);
            add(ELSE_LIB, ELSE_LIB_ICO);
            add(ELSE_UPDATE, ELSE_UPDATE_ICO);

            add(ELSE_IF, ELSE_IF_ICO);
            add(ELSE_WHILE, ELSE_WHILE_ICO);
            add(ELSE_FOR, ELSE_FOR_ICO);
            add(ELSE_TRY, ELSE_TRY_ICO);
            add(ELSE_SEQ, ELSE_SEQ_ICO);
            add(ELSE_UNCHKD_SEQ, ELSE_UNCHKD_SEQ_ICO);
            add(ELSE_CONCURRENCE, ELSE_CONCURRENCE_ICO);
         }
         
         public void add(String tag, String iconName)
         {
            put(tag, loadImage(iconName));
         }
      };

      /**
       * Returns the matching ImageIcon of the specified icon name.
       * @param icon the icon name on which to find a matching ImageIcon
       * @return the ImageIcon of the specified icon name
       */
      public static ImageIcon getIcon(String icon)
      {
         return iconLut.get(icon);
      }
      
      private static ImageIcon loadImage(String name)
      {         
         return new ImageIcon(Toolkit.getDefaultToolkit().getImage(ClassLoader.getSystemResource(ICONS_DIR + name)));
      }

      /** Represents index of Name column in the Luv application. */     
      public static final int NAME_COL_NUM         = 0;
      /** Represents index of State column in the Luv application. */   
      public static final int STATE_COL_NUM        = 1;
      /** Represents index of Outcome column in the Luv application. */   
      public static final int OUTCOME_COL_NUM      = 2;
      /** Represents index of Failure Type column in the Luv application. */   
      public static final int FAILURE_TYPE_COL_NUM = 3;
      
      /** Represents column name for the Name column in the Luv application. */   
      public static final String NAME_COL_NAME         = "Name";
      /** Represents column name for the State column in the Luv application. */
      public static final String STATE_COL_NAME        = "State";
      /** Represents column name for the Outcome column in the Luv application. */
      public static final String OUTCOME_COL_NAME      = "Outcome";
      /** Represents column name for the Failure Type column in the Luv application. */
      public static final String FAILURE_TYPE_COL_NAME = "Failure Type";      
      
      /** While displayed in the State column, it indicates that the Plexil node is in an INACTIVE state. */
      public static final String INACTIVE         = "INACTIVE";
      /** While displayed in the State column, it indicates that the Plexil node is in an WAITING state. */
      public static final String WAITING          = "WAITING";
      /** While displayed in the State column, it indicates that the Plexil node is in an EXECUTING state. */
      public static final String EXECUTING        = "EXECUTING";
      /** While displayed in the State column, it indicates that the Plexil node is in an FINISHING state. */
      public static final String FINISHING        = "FINISHING";
      /** While displayed in the State column, it indicates that the Plexil node is in an FINISHED state. */
      public static final String FINISHED         = "FINISHED";
      /** While displayed in the State column, it indicates that the Plexil node is in an FAILING state. */
      public static final String FAILING          = "FAILING";
      /** While displayed in the State column, it indicates that the Plexil node is in an ITERATION_ENDED state. */
      public static final String ITERATION_ENDED  = "ITERATION_ENDED";

      /** A collection of all the possible Plexil node states. */
      public static final String[] NODE_STATES = 
      {
         INACTIVE,
         WAITING,
         EXECUTING,
         FINISHING,
         FINISHED,
         FAILING,
         ITERATION_ENDED,
      };

      /** While displayed in the Outcome column, it indicates that the Plexil node outcome was UNKNOWN. */
      public static final String UNKNOWN = "UNKNOWN";
       /** While displayed in the Outcome column, it indicates that the Plexil node outcome was a SUCCESS. */
      public static final String SUCCESS = "SUCCESS";
       /** While displayed in the Outcome column, it indicates that the Plexil node outcome was a FAILURE. */
      public static final String FAILURE = "FAILURE";
       /** While displayed in the Outcome column, it indicates that the Plexil node outcome was SKIPPED. */
      public static final String SKIPPED = "SKIPPED";

      /** A collection of all the possible Plexil node outcomes. */
      public static final String[] NODE_OUTCOMES = 
      {
         UNKNOWN,
         SUCCESS,
         FAILURE,
         SKIPPED,
      };

      /** While displayed in the Failure Type column, it indicates that the Plexil node failed due to an INFINITE_LOOP. */
      public static final String INFINITE_LOOP = "INFINITE_LOOP";
      /** While displayed in the Failure Type column, it indicates that the Plexil node failed due to an PRE_CONDITION_FALSE. */
      public static final String PRE_CONDITION_FALSE = "PRE_CONDITION_FALSE";
      /** While displayed in the Failure Type column, it indicates that the Plexil node failed due to an POST_CONDITION_FALSE. */
      public static final String POST_CONDITION_FALSE = "POST_CONDITION_FALSE";
      /** While displayed in the Failure Type column, it indicates that the Plexil node failed due to an INVARIANT_CONDITION_FALSE. */
      public static final String INVARIANT_CONDITION_FALSE = "INVARIANT_CONDITION_FALSE";
      /** While displayed in the Failure Type column, it indicates that the Plexil node failed due to an ANCESTOR_INVARIANT_CONDITION_FALSE. */
      public static final String ANCESTOR_INVARIANT_CONDITION_FALSE = "ANCESTOR_INVARIANT_CONDITION_FALSE";
      /** While displayed in the Failure Type column, it indicates that the Plexil node failed due to an PARENT_FAILED. */
      public static final String PARENT_FAILED = "PARENT_FAILED";

      /** A collection of all the possible Plexil node failure types. */
      public static final String[] NODE_FAILURE_TYPES = 
      {
         INFINITE_LOOP,
         PRE_CONDITION_FALSE,
         POST_CONDITION_FALSE,
         INVARIANT_CONDITION_FALSE,
         ANCESTOR_INVARIANT_CONDITION_FALSE,
         PARENT_FAILED,
      };
      
      /** Indicates that a Plexil node has a breakpoint set on it. */
      public static final String MODEL_ENABLED_BREAKPOINTS = "model.breakpoint.enabled";
      /** Indicates that a Plexil node does not have a breakpoint set on it. */
      public static final String MODEL_DISABLED_BREAKPOINTS = "model.breakpoint.disabled";

      /** Holds the Luv application row color pattern. */
      public static final Vector<Color> TREE_TABLE_ROW_COLORS = new Vector<Color>()
      {
         {
            add(Color.WHITE);
            add(new Color(245, 245, 255));
            add(Color.WHITE);
            add(new Color(255, 246, 246));
         }
      };

      // colors
      private static HashMap<String, Color> colorLut = new HashMap<String, Color>()
      {
         {
            // node state
            put(INACTIVE,        Color.LIGHT_GRAY);
            put(WAITING,         Color.RED); 
            put(EXECUTING,       Color.GREEN.darker());
            put(FINISHING,       new Color(128, 128, 255));
            put(FINISHED,        Color.GRAY);
            put(FAILING,         new Color(255, 128, 128));
            put(ITERATION_ENDED, Color.BLUE.darker());

            // node outcome
            put(UNKNOWN,         new Color(255, 255, 255, 0));
            put(SUCCESS,         Color.GREEN.darker());
            put(FAILURE,         Color.RED);
            put(SKIPPED,         Color.BLUE.darker());

            // node failure types
            put(INFINITE_LOOP,                      Color.RED.darker());
            put(PRE_CONDITION_FALSE,                Color.RED.darker());
            put(POST_CONDITION_FALSE,               Color.RED.darker());
            put(INVARIANT_CONDITION_FALSE,          Color.RED.darker());
            put(ANCESTOR_INVARIANT_CONDITION_FALSE, Color.RED.darker());
            put(PARENT_FAILED,                      Color.RED.darker());

            // model colors
            put(MODEL_ENABLED_BREAKPOINTS,          Color.RED);
            put(MODEL_DISABLED_BREAKPOINTS,         Color.ORANGE);
         }
      };

      /**
       * Returns the matching color of the specified Plexil node State, Outcome, 
       * Failure Type or if a breakpoint was set on it.
       * 
       * @param value the type of Plexil node State, Outcome, Failure Type or Breakpoint status
       * @return the matching color
       */
      public static Color lookupColor(String value)
      {
         return colorLut.get(value);
      }

      /** Indicates that no accelerator key is used. */
      public static final int       NO_ACCELERATOR = Integer.MIN_VALUE;

      /** Represents an end of message in an input stream. */      
      public static final int       END_OF_MESSAGE = 4;

      /** Used as a marker for formatting Plexil node condition, local variable and action information. */
      public static final String SEPARATOR = "_Separator_";

      /** A collection of all the default file extensions visible to the Luv application. */
      public static final String[] FILE_EXTENSIONS = 
      {
         "xml",
         "plx",
         "psx",
         "txt",
         "pst",
         "pls",
      };
      
      /** Represents an empty Plexil script. */ 
      public static final String DEFAULT_SCRIPT_NAME = "empty.psx";
      /** Represents an default UE config. */      
      public static final String DEFAULT_CONFIG_NAME = "dummy-config.xml";
      /** Represents an default UE path. */
      public static final String DEFAULT_CONFIG_PATH = PLEXIL_HOME +
		 												System.getProperty("file.separator") +
		 												"examples" + System.getProperty("file.separator");      
      /** Represents an default SIM Script. */      
      public static final String DEFAULT_SIM_SCRIPT_NAME = "unified-script.txt";
      /** Represents an default SIM Script path. */
      public static final String DEFAULT_SIM_SCRIPT_PATH = PLEXIL_HOME +
		 System.getProperty("file.separator") +	"src" + System.getProperty("file.separator") + "apps"
		 + System.getProperty("file.separator") + "StandAloneSimulator" + System.getProperty("file.separator")
		 + "PlexilSimulator" + System.getProperty("file.separator") + "test" + System.getProperty("file.separator");
      /** Contains the contents of an empty Plexil script. */
      public static final String EMPTY_SCRIPT = "<PLEXILScript><Script></Script></PLEXILScript>";          
      
      /** Represents index of the Executive Select under the File menu in the Luv application. */
      public static final int EXEC_SELECT_ITEM      		= 0;
      /** Represents index of the Reload item under the File menu in the Luv application. */
      public static final int RELOAD_MENU_ITEM              = 1;
      /** Represents index of the Exit item under the File menu in the Luv application. */ 
      public static final int EXIT_MENU_ITEM                = 3;    
      
      /** Represents index of the Pause/Resume item under the Run menu in the Luv application. */ 
      public static final int PAUSE_RESUME_MENU_ITEM        = 0;
      /** Represents index of the Step item under the Run menu in the Luv application. */
      public static final int STEP_MENU_ITEM                = 1;
      /** Represents index of the Break item under the Run menu in the Luv application. */
      public static final int BREAK_MENU_ITEM               = 2;
      /** Represents index of the Remove Breaks item under the Run menu in the Luv application. */
      public static final int REMOVE_BREAKS_MENU_ITEM       = 3;      
      /** Represents index of the Execute item under the Run menu in the Luv application. */
      public static final int EXECUTE_MENU_ITEM             = 5;
      
      /** Represents index of the Expand All item under the View menu in the Luv application. */
      public static final int EXPAND_MENU_ITEM              = 0;
      /** Represents index of the Collapse All item under the View menu in the Luv application. */
      public static final int COLLAPSE_MENU_ITEM            = 1;
      /** Represents index of the Hide Or Show... item under the View menu in the Luv application. */
      public static final int HIDE_OR_SHOW_NODES_MENU_ITEM  = 2;
      /** Represents index of the Find... item under the View menu in the Luv application. */
      public static final int FIND_MENU_ITEM                = 3;
      
      public static final int EPX_VIEW_MENU_ITEM            = 5;

      public static final int VIEW_SOURCE_MENU_ITEM         = 7;

      ////////////////////////// Plexil plan XML tags ////////////////////////
      ///////////////// Scanned for in the PlexilPlanHandler //////////////////
      
      /** Represents the XML tag in the Plexil Plan marking the Model Name information. */
      public static final String MODEL_NAME         = "ModelName";
      /** Represents the XML tag in the Plexil Plan marking the Model Type information. */
      public static final String MODEL_TYPE         = "ModelType";
      /** Represents the XML tag in the Plexil Plan marking the Model State information. */
      public static final String MODEL_STATE        = "ModelState";
      /** Represents the XML tag in the Plexil Plan marking the Model Outcome information. */
      public static final String MODEL_OUTCOME      = "ModelOutcome";
      /** Represents the XML tag in the Plexil Plan marking the Model Failure Type information. */
      public static final String MODEL_FAILURE_TYPE = "ModelFailureType";
      /** Represents the XML tag in the Plexil Plan marking the Model Library Call ID information. */
      public static final String MODEL_LIBRARY_CALL_ID = "ModelLibraryCallId";
      /** Represents the XML tag indicating the loaded file is a Plexil Plan. */
      public static final String PLEXIL_PLAN       = "PlexilPlan";
      /** Represents the XML tag indicating the loaded file is a Plexil Library. */
      public static final String PLEXIL_LIBRARY    = "PlexilLibrary";
      /** Represents the XML tag indicating the loaded file is a Plexil Script. */
      public static final String PLEXIL_SCRIPT     = "PLEXILScript";
      /** Represents the XML tag indicating the loaded file is a Plexil Library. */
      public static final String LIBRARY           = "Library";   
      /** Represents the XML tag in the Plexil Plan indicating that a Node State Update is occurring. */
      public static final String NODE_STATE_UPDATE = "NodeStateUpdate";
      /** Represents the XML tag in the Plexil Plan indicating that a variable assignment is occurring. */
      public static final String ASSIGNMENT = "Assignment";
      /** Represents the XML tag in the Plexil Plan marking the Plan Info section. */
      public static final String PLAN_INFO     = "PlanInfo";
      /** Represents the XML tag in the Plexil Plan marking the Plexil Plan file name information. */
      public static final String PLAN_FILENAME = "PlanFilename";
      /** Represents the XML tag in the Plexil Plan marking the Plexil Script file name information. */
      public static final String SCRIPT_FILENAME = "ScriptFilename";
      /** Represents the XML tag in the Plexil Plan marking the User Blocking or No Blocking information. */
      public static final String VIEWER_BLOCKS = "ViewerBlocks";
      
      // condition tags
      /** Represents the XML tag in the Plexil Plan marking the SkipCondition information. */
      public static final String SKIP_CONDITION = "SkipCondition";
      /** Represents the XML tag in the Plexil Plan marking the StartCondition information. */
      public static final String START_CONDITION = "StartCondition";
      /** Represents the XML tag in the Plexil Plan marking the EndCondition information. */
      public static final String END_CONDITION = "EndCondition";
      /** Represents the XML tag in the Plexil Plan marking the InvariantCondition information. */
      public static final String INVARIANT_CONDITION = "InvariantCondition";
      /** Represents the XML tag in the Plexil Plan marking the PreCondition information. */
      public static final String PRE_CONDITION = "PreCondition";
      /** Represents the XML tag in the Plexil Plan marking the PostCondition information. */
      public static final String POST_CONDITION = "PostCondition";
      /** Represents the XML tag in the Plexil Plan marking the RepeatCondition information. */
      public static final String REPEAT_CONDITION = "RepeatCondition";
      /** Represents the XML tag in the Plexil Plan marking the AncestorInvariantCondition information. */
      public static final String ANCESTOR_INVARIANT_CONDITION = "AncestorInvariantCondition";
      /** Represents the XML tag in the Plexil Plan marking the AncestorEndCondition information. */
      public static final String ANCESTOR_END_CONDITION = "AncestorEndCondition";
      /** Represents the XML tag in the Plexil Plan marking the ParentExecutingCondition information. */
      public static final String PARENT_EXECUTING_CONDITION = "ParentExecutingCondition";
      /** Represents the XML tag in the Plexil Plan marking the ParentFinishedCondition information. */
      public static final String PARENT_FINISHED_CONDITION = "ParentFinishedCondition";
      /** Represents the XML tag in the Plexil Plan marking the AllChildrenWaitingOrFinishedCondition information. */
      public static final String CHILDREN_WAITING_OR_FINISHED = "AllChildrenWaitingOrFinishedCondition";
      /** Represents the XML tag in the Plexil Plan marking the AbortCompleteCondition information. */
      public static final String ABORT_COMPLETE = "AbortCompleteCondition";
      /** Represents the XML tag in the Plexil Plan marking the ParentWaitingCondition information. */
      public static final String PARENT_WAITING_CONDITION = "ParentWaitingCondition";
      /** Represents the XML tag in the Plexil Plan marking the CommandHandleReceivedCondition information. */
      public static final String COMMAND_HANDLE_RECEIVED_CONDITION = "CommandHandleReceivedCondition";

      /**
       * Returns the matching integer of the specified condition name.
       * @param condition the condition name on which to find a matching integer
       * @return the integer of the specified condition name
       */
      public static int getConditionNum(String condition)
      {
          if (condition.equals(SKIP_CONDITION))                         return 0;    
          else if (condition.equals(START_CONDITION))                   return 1;                
          else if (condition.equals(END_CONDITION))                     return 2;   
          else if (condition.equals(INVARIANT_CONDITION))               return 3; 
          else if (condition.equals(PRE_CONDITION))                     return 4; 
          else if (condition.equals(POST_CONDITION))                    return 5;
          else if (condition.equals(REPEAT_CONDITION))                  return 6;
          else if (condition.equals(ANCESTOR_INVARIANT_CONDITION))      return 7;
          else if (condition.equals(ANCESTOR_END_CONDITION))            return 8;           
          else if (condition.equals(PARENT_EXECUTING_CONDITION))        return 9;   
          else if (condition.equals(PARENT_FINISHED_CONDITION))         return 10;       
          else if (condition.equals(CHILDREN_WAITING_OR_FINISHED))      return 11;
          else if (condition.equals(ABORT_COMPLETE))                    return 12;                  
          else if (condition.equals(PARENT_WAITING_CONDITION))          return 13;        
          else if (condition.equals(COMMAND_HANDLE_RECEIVED_CONDITION)) return 14;
          else                                                          return -1; //error
      }

      /** A collection of all the possible Plexil Plan conditions. */
      public static final String[] ALL_CONDITIONS = 
      {
         SKIP_CONDITION,
         START_CONDITION,
         END_CONDITION,
         INVARIANT_CONDITION,
         PRE_CONDITION,
         POST_CONDITION,
         REPEAT_CONDITION,
         ANCESTOR_INVARIANT_CONDITION,
         ANCESTOR_END_CONDITION,
         PARENT_EXECUTING_CONDITION,
         PARENT_FINISHED_CONDITION,
         CHILDREN_WAITING_OR_FINISHED,
         ABORT_COMPLETE,
         PARENT_WAITING_CONDITION,
         COMMAND_HANDLE_RECEIVED_CONDITION,
      };

      /** Represents the Node XML tag in the Plexil Plan. */
      public static final String NODE = "Node";
      /** Represents the NodeId XML tag in the Plexil Plan. */
      public static final String NODE_ID = "NodeId";
      /** Represents the Priority XML tag in the Plexil Plan. */
      public static final String PRIORITY = "Priority";
      /** Represents the Permissions XML tag in the Plexil Plan. */
      public static final String PERMISSIONS = "Permissions";
      /** Represents the Interface XML tag in the Plexil Plan. */
      public static final String INTERFACE = "Interface";
      /** Represents the VariableDeclarations XML tag in the Plexil Plan. */
      public static final String VAR_DECLS = "VariableDeclarations";
      /** Represents the DeclareVariable XML tag in the Plexil Plan. */
      public static final String DECL_VAR = "DeclareVariable";
      /** Represents the Variable XML tag in the Assignment xml. */
      public static final String VARIABLE_NAME = "VariableName";
      /** Represents the Variable Value XML tag in the Assignment xml. */
      public static final String VALUE = "Value";
      /** Represents the In XML tag in the Plexil Plan. */
      public static final String IN = "In";
      /** Represents the InOut XML tag in the Plexil Plan. */
      public static final String INOUT = "InOut";
      /** Represents the Type XML tag in the Plexil Plan. */
      public static final String TYPE = "Type";
      /** Represents the MaxSize XML tag in the Plexil Plan. */
      public static final String MAXSIZE = "MaxSize";
      /** Represents the InitialValue XML tag in the Plexil Plan. */
      public static final String INITIALVAL = "InitialValue";
      /** Represents the Assignment XML tag in the Plexil Plan. */
      public static final String ASSN = "Assignment";
      /** Represents the Empty XML tag in the Plexil Plan. */
      public static final String EMPTY = "Empty";
      /** Represents the NodeBody XML tag in the Plexil Plan. */
      public static final String NODE_BODY = "NodeBody";
      /** Represents the RHS XML tag in the Plexil Plan. */
      public static final String RHS = "RHS";
      /** Represents the NumericRHS XML tag in the Plexil Plan. */
      public static final String NUMERIC_RHS = "NumericRHS";
      /** Represents the StringRHS XML tag in the Plexil Plan. */
      public static final String STRING_RHS = "StringRHS";
      /** Represents the BooleanRHS XML tag in the Plexil Plan. */
      public static final String BOOLEAN_RHS = "BooleanRHS";
      /** Represents the LookupRHS XML tag in the Plexil Plan. */
      public static final String LOOKUP_RHS = "LookupRHS";
      /** Represents the ArrayRHS XML tag in the Plexil Plan. */
      public static final String ARRAY_RHS = "ArrayRHS";
      /** Represents the TimeRHS XML tag in the Plexil Plan. */
      public static final String TIME_RHS = "TimeRHS";
      /** Represents the NodeList XML tag in the Plexil Plan. */
      public static final String NODELIST = "NodeList";
      /** Represents the LibraryNodeCall XML tag in the Plexil Plan. */
      public static final String LIBRARYNODECALL = "LibraryNodeCall";
      /** Represents the Alias XML tag in the Plexil Plan. */
      public static final String ALIAS = "Alias";
      /** Represents the NodeParameter XML tag in the Plexil Plan. */
      public static final String NODE_PARAMETER = "NodeParameter";
      /** Represents the Command XML tag in the Plexil Plan. */
      public static final String COMMAND = "Command";
      /** Represents the FunctionCall XML tag in the Plexil Plan. */
      public static final String FUNCCALL = "FunctionCall";
      /** Represents the Name XML tag in the Plexil Plan. */
      public static final String NAME = "Name";
      /** Represents the Index XML tag in the Plexil Plan. */
      public static final String INDEX = "Index";
      /** Represents the Arguments XML tag in the Plexil Plan. */
      public static final String ARGS = "Arguments";
      /** Represents the Lookup XML tag in the Plexil Plan. */
      public static final String LOOKUP = "Lookup";
      /** Represents the LookupNow XML tag in the Plexil Plan. */
      public static final String LOOKUPNOW = "LookupNow";
      /** Represents the LookupOnChange XML tag in the Plexil Plan. */
      public static final String LOOKUPCHANGE = "LookupOnChange";
      /** Represents the LookupWithFrequency XML tag in the Plexil Plan. */
      public static final String LOOKUPFREQ = "LookupWithFrequency";
      /** Represents the Frequency XML tag in the Plexil Plan. */
      public static final String FREQ = "Frequency";
      /** Represents the High XML tag in the Plexil Plan. */
      public static final String HIGH = "High";
      /** Represents the Low XML tag in the Plexil Plan. */
      public static final String LOW = "Low";
      /** Represents the Tolerance XML tag in the Plexil Plan. */
      public static final String TOLERANCE = "Tolerance";
      /** Represents the NodeRef XML tag in the Plexil Plan. */
      public static final String NODEREF = "NodeRef";
      /** Represents the StateName XML tag in the Plexil Plan. */
      public static final String STATE_NAME = "StateName";
      /** Represents the Timepoint XML tag in the Plexil Plan. */
      public static final String TIMEPOINT = "Timepoint";
      /** Represents the Update XML tag in the Plexil Plan. */
      public static final String UPDATE = "Update";
      /** Represents the Request XML tag in the Plexil Plan. */
      public static final String REQ = "Request";
      /** Represents the Pair XML tag in the Plexil Plan. */
      public static final String PAIR = "Pair";
      /** Represents the ArrayValue XML tag in the Plexil Plan. */
      public static final String ARRAY_VAL = "ArrayValue";
      /** Represents the ArrayVariable XML tag in the Plexil Plan. */
      public static final String ARRAY_VAR = "ArrayVariable";      
      /** Represents the ResourceList XML tag in the Plexil Plan. */
      public static final String RESOURCE_LIST = "ResourceList";
      /** Represents the Resource XML tag in the Plexil Plan. */
      public static final String RESOURCE = "Resource";
      /** Represents the ResourceName XML tag in the Plexil Plan. */
      public static final String RESOURCE_NAME = "ResourceName";
      /** Represents the ResourcePriority XML tag in the Plexil Plan. */
      public static final String RESOURCE_PRIORITY = "ResourcePriority";
      /** Represents the ResourceLowerBound XML tag in the Plexil Plan. */
      public static final String RESOURCE_LOWER_BOUND = "ResourceLowerBound";
      /** Represents the ResourceUpperBound XML tag in the Plexil Plan. */
      public static final String RESOURCE_UPPER_BOUND = "ResourceUpperBound";
      /** Represents the ResourceReleaseAtTermination XML tag in the Plexil Plan. */
      public static final String RESOURCE_RELEASE_AT_TERM = "ResourceReleaseAtTermination";
      /** Represents the AND operator XML tag in the Plexil Plan. */
      public static final String AND = "AND";
      /** Represents the OR operator XML tag in the Plexil Plan. */
      public static final String OR = "OR";
      /** Represents the XOR operator XML tag in the Plexil Plan. */
      public static final String XOR = "XOR";
      /** Represents the NOT operator XML tag in the Plexil Plan. */
      public static final String NOT = "NOT";
      /** Represents the Concat (concatenation) XML tag in the Plexil Plan. */
      public static final String CONCAT = "Concat";
      /** Represents the IsKnown XML tag in the Plexil Plan. */
      public static final String IS_KNOWN = "IsKnown";
      /** Represents the EQ (equals) XML tag in the Plexil Plan. */
      public static final String EQ = "EQ";
      /** Represents the EQNumeric (numeric equals) XML tag in the Plexil Plan. */
      public static final String EQ_NUMERIC = "EQNumeric";
      /** Represents the EQString (string equals) XML tag in the Plexil Plan. */
      public static final String EQ_STRING = "EQString";
      /** Represents the EQBoolean (boolean equals) XML tag in the Plexil Plan. */
      public static final String EQ_BOOLEAN = "EQBoolean";
      /** Represents the EQBoolean (boolean equals) XML tag in the Plexil Plan. */
      public static final String EQ_TIME ="EQTime";
      /** Represents the EQInternal (internal equals) XML tag in the Plexil Plan. */
      public static final String EQ_INTERNAL = "EQInternal";
      /** Represents the NE (not equals) XML tag in the Plexil Plan. */
      public static final String NE = "NE";
      /** Represents the NENumeric (numeric not equals) XML tag in the Plexil Plan. */
      public static final String NE_NUMERIC = "NENumeric";
      /** Represents the NEString (string not equals) XML tag in the Plexil Plan. */
      public static final String NE_STRING = "NEString";
      /** Represents the NEBoolean (boolean not equals) XML tag in the Plexil Plan. */
      public static final String NE_BOOLEAN = "NEBoolean";
      /** Represents the NEInternal (internal not equals) XML tag in the Plexil Plan. */
      public static final String NE_INTERNAL = "NEInternal";
      /** Represents the LT (less than) operator XML tag in the Plexil Plan. */
      public static final String LT = "LT";
      /** Represents the LE (less than or equal) operator XML tag in the Plexil Plan. */
      public static final String LE = "LE";
      /** Represents the GT (greater than) operator XML tag in the Plexil Plan. */
      public static final String GT = "GT";
      /** Represents the GE (greater than or equal) operator XML tag in the Plexil Plan. */
      public static final String GE = "GE";
      /** Represents the ADD (addition) operator XML tag in the Plexil Plan. */
      public static final String ADD = "ADD";
      /** Represents the SUB (subtraction) operator XML tag in the Plexil Plan. */
      public static final String SUB = "SUB";
      /** Represents the MUL (multiplication) operator XML tag in the Plexil Plan. */
      public static final String MUL = "MUL";
      /** Represents the DIV (division) operator XML tag in the Plexil Plan. */
      public static final String DIV = "DIV";
      /** Represents the SQRT (square root) operator XML tag in the Plexil Plan. */
      public static final String SQRT = "SQRT";
      /** Represents the ABS (absolute) operator XML tag in the Plexil Plan. */
      public static final String ABS = "ABS";
      /** Represents the XML tag marking an array declaration indicator in the Plexil Plan. */
      public static final String DECL_ARRAY = "DeclareArray";
      /** Represents the XML tag marking an array element value in the Plexil Plan. */
      public static final String ARRAYELEMENT = "ArrayElement";

      /** Represents the Node Type in a Plexil Plan and XML tag marking a NodeType in the Plexil Plan. */
      public static final String NODETYPE_ATTR = "NodeType";
      public static final String NODETYPE_ATTR_PLX = "NodeTypePLX";

      /** Represents the XML tag in the Plexil Plan marking the Node Outcome information. */
      public static final String NODE_OUTCOME      = "NodeOutcome";
      /** Represents the XML tag in the Plexil Plan marking the Node Failure information. */
      public static final String NODE_FAILURE      = "NodeFailure";
      /** Represents the XML tag in the Plexil Plan marking the Node Failure Type information. */
      public static final String NODE_FAILURE_TYPE = "NodeFailureType";
      /** Represents the XML tag in the Plexil Plan marking the Node State information. */
      public static final String NODE_STATE        = "NodeState";
      /** Represents the XML tag in the Plexil Plan marking the Node Timepoint information. */
      public static final String NODE_TIMEPOINT    = "NodeTimepoint";
      /** Represents the XML tag in the Plexil Plan marking the Node Command Handle information. */
      public static final String NODE_CMD_HANDLE   = "NodeCommandHandle";

      public static final String    START_LOGO        = "Clear Screen";
      public static final String    ABOUT_LOGO        = "About Logo";


      /* Extended Plexil elements */
      public static final String EPX = "epx";
      public static final String AUX = "aux";
      public static final String AUX_THEN = "aux_then";
      public static final String AUX_ELSE = "aux_else";

      public static final String IF = "If";
      public static final String THEN  = "Then";
      public static final String ELSE  = "Else";
      public static final String WHILE  = "While";
      public static final String TRY  = "Try";
      public static final String FOR  = "For";
      public static final String SEQ  = "Sequence";
      public static final String UNCHKD_SEQ = "UncheckedSequence";
      public static final String CONCURRENCE = "Concurrence";

      public static final String THEN_LIST  = "ThenNodeList";
      public static final String THEN_ASSN  = "ThenAssignment";
      public static final String THEN_CMD  = "ThenCommand";
      public static final String THEN_EMPTY  = "ThenEmpty";
      public static final String THEN_LIB  = "ThenLibraryNodeCall";
      public static final String THEN_UPDATE  = "ThenUpdate";

      public static final String THEN_IF  = "ThenIf";
      public static final String THEN_WHILE  = "ThenWhile";
      public static final String THEN_FOR  = "ThenFor";
      public static final String THEN_TRY  = "ThenTry";
      public static final String THEN_SEQ  = "ThenSequence";
      public static final String THEN_UNCHKD_SEQ  = "ThenUncheckedSequence";
      public static final String THEN_CONCURRENCE  = "ThenConcurrence";

      public static final String ELSE_LIST  = "ElseNodeList";
      public static final String ELSE_ASSN  = "ElseAssignment";
      public static final String ELSE_CMD  = "ElseCommand";
      public static final String ELSE_EMPTY  = "ElseEmpty";
      public static final String ELSE_LIB  = "ElseLibraryNodeCall";
      public static final String ELSE_UPDATE  = "ElseUpdate";

      public static final String ELSE_IF  = "ElseIf";
      public static final String ELSE_WHILE  = "ElseWhile";
      public static final String ELSE_FOR  = "ElseFor";
      public static final String ELSE_TRY  = "ElseTry";
      public static final String ELSE_SEQ  = "ElseSequence";
      public static final String ELSE_UNCHKD_SEQ  = "ElseUncheckedSequence";
      public static final String ELSE_CONCURRENCE = "ElseConcurrence";

      /** A collection of all the possible Plexil Plan node types. */
      public static HashMap<String, String> NODE_TYPES = new HashMap<String, String>()
      {
	  {
	      put(NODELIST,        "List Node");
	      put(COMMAND,         "Command Node");
	      put(ASSN,            "Assignment Node");
	      put(EMPTY,           "Empty Node");
	      put(FUNCCALL,        "Function Call Node");
	      put(UPDATE,          "Update Node");
	      put(LIBRARYNODECALL, "Library Node");
	  }
      };

      /** Represents the Integer Value XML tag in the Plexil Plan. */
      public static final String INT_VAL = "IntegerValue";
      /** Represents the Real Value XML tag in the Plexil Plan. */
      public static final String REAL_VAL = "RealValue";
      /** Represents the String Value XML tag in the Plexil Plan. */
      public static final String STRING_VAL = "StringValue";
      /** Represents the Boolean Value XML tag in the Plexil Plan. */
      public static final String BOOL_VAL = "BooleanValue";
      /** Represents the Time Value XML tag in the Plexil Plan. */
      public static final String TIME_VAL = "TimeValue";
      /** Represents the Node Outcome Value XML tag in the Plexil Plan. */
      public static final String NODE_OUTCOME_VAL = NODE_OUTCOME + "Value";
      /** Represents the Node Failure Value XML tag in the Plexil Plan. */
      public static final String NODE_FAILURE_VAL = NODE_FAILURE + "Value";
      /** Represents the Node State Value XML tag in the Plexil Plan. */
      public static final String NODE_STATE_VAL = NODE_STATE + "Value";
      /** Represents the Node Timepoint Value XML tag in the Plexil Plan. */
      public static final String NODE_TIMEPOINT_VAL = NODE_TIMEPOINT + "Value";
      /** Represents the Node Command Handle Value XML tag in the Plexil Plan. */
      public static final String NODE_CMD_HANDLE_VAL = NODE_CMD_HANDLE + "Value";
      /** Represents the Assignment Value XML tag in the Plexil Plan. */
      public static final String ASSN_VAL = ASSN + "Value";          

      /** A collection of all the possible Plexil Plan node types which expand to children. */
      public static final String[] CHILD_TAGS =
      {
         NODE,
         LIBRARYNODECALL
      };
      
      /** A collection of all the possible XML tags that represent Plexil Plan properties. */
      public static final String[] PROPERTY_TAGS = 
      {
         MODEL_NAME,
         MODEL_TYPE,
         MODEL_STATE,
         MODEL_OUTCOME,
         MODEL_FAILURE_TYPE,
         MODEL_LIBRARY_CALL_ID,

         VIEWER_BLOCKS,
         PLAN_FILENAME,
         SCRIPT_FILENAME,

         PLEXIL_PLAN,
         PLEXIL_SCRIPT,
         LIBRARY,

         NODETYPE_ATTR,
         NODE_ID,
         NODE_BODY,
         NODELIST,
         NAME,
         TYPE,
         MAXSIZE,
         INDEX,
         INT_VAL,
         REAL_VAL,
         STRING_VAL,
         BOOL_VAL,
         DECL_VAR,
         DECL_ARRAY,
         ARRAYELEMENT,
      };

      /** A collection of all the possible XML tags that represent Node State values. */   
      public static final String[] NODE_STATE_TAGS =
      {
          NODE_OUTCOME_VAL, 
          NODE_FAILURE_VAL, 
          NODE_STATE_VAL,  
          NODE_CMD_HANDLE_VAL,
      };
    
      /** A collection of all the possible XML tags that represent Returns. */  
      public static final String[] RETURN_TAGS =
      {
          RHS,
          NUMERIC_RHS, 
          STRING_RHS, 
          TIME_RHS,
          BOOLEAN_RHS,
          LOOKUP_RHS,
          ARRAY_RHS,
      };
    
      /** A collection of all the possible XML tags that represent Resource information. */  
      public static final String[] RESOURCE_TAGS =
      {
          RESOURCE_NAME,
          RESOURCE_PRIORITY, 
          RESOURCE_LOWER_BOUND, 
          RESOURCE_UPPER_BOUND,
          RESOURCE_RELEASE_AT_TERM,
      };
    
      /** A collection of all the possible XML tags that represent mathematic operators. */  
      public static final String[] MATH_TAGS =
      {
          ADD,
          SUB, 
          MUL, 
          DIV,
          SQRT,
          ABS
      };
    
      /** A collection of all the possible XML tags that represent Plexil Plan Lookups. */  
      public static final String[] LOOKUP_TAGS =
      {
          LOOKUPNOW,
          LOOKUPCHANGE, 
          LOOKUPFREQ,
      };
    
      /** A collection of all the possible XML tags that represent comparison operators. */  
      public static final String[] COMPARISON_TAGS =
      {
          LT,
          GT,
          LE,
          GE, 
          EQ, 
          EQ_NUMERIC,
          EQ_STRING, 
          EQ_BOOLEAN,
          EQ_TIME,
          EQ_INTERNAL,
          NE,
          NE_NUMERIC,
          NE_STRING,
          NE_BOOLEAN, 
          NE_INTERNAL,
      };
      
      /** A collection of all the possible XML tags that represent Node Actions (Assignement, Command, etc). */  
      public static final String[] ACTION_TAGS = 
      {
         ASSN,
         COMMAND,
         FUNCCALL,
         UPDATE,
      };
      
      /** A collection of all the possible XML tags that represent variable declarationsZo. */  
      public static final String[] VARIABLE_DECL_TAGS =
      {
	 DECL_VAR,
         DECL_ARRAY,
      };     
}
