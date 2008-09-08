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

import java.awt.Point;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Toolkit;
import javax.swing.ImageIcon;
import java.util.HashMap;
import java.util.Vector;

/**
 * This class is the repository for all constants for the Luv
 * application.
 */

public class Constants
{
      /** indicates that no accelerator key is used */

      public static final int    NO_ACCELERATOR = Integer.MIN_VALUE;

      /** end of message marker */
      
      public static final int    END_OF_MESSAGE = 4;
      public static final String MESSAGE_ACKNOWLEDGE = "<acknowledge/>";

      /** properties file */      
      public static final String PROPERTIES_FILE_LOCATION =
         System.getProperty("user.home") + 
         System.getProperty("file.separator") + ".luv";

      /** icon constants */

      public static final String    ICONS_DIR = 
         "resources" + System.getProperty("file.separator") +
         "icons" +  System.getProperty("file.separator");
      
      

      // node icon file names

      public static final String    SPLASH_SCREEN_ICO = "LuvIcon.gif";
      public static final String    START_SCREEN_ICO  = "LuvIcon6.gif";
      public static final String    NODELIST_ICO_NAME = "nodelist_node.gif";
      public static final String    COMMAND_ICO_NAME  = "command_node.gif";
      public static final String    ASSN_ICO_NAME     = "assignment_node.gif";
      public static final String    EMPTY_ICO_NAME    = "empty_node.gif";
      public static final String    FUNCALL_ICO_NAME  = "functioncall_node.gif";
      public static final String    UPDATE_ICO_NAME   = "update_node.gif";
      public static final String    REQUEST_ICO_NAME  = "request_node.gif";
      public static final String    LIBCALL_ICO_NAME  = "library_node_call.gif";

      // icon table

      static HashMap<String, ImageIcon> iconLut = new HashMap<String, ImageIcon>()
      {
         {
            add(NODELIST,        NODELIST_ICO_NAME);
            add(COMMAND,         COMMAND_ICO_NAME);
            add(ASSN,            ASSN_ICO_NAME);
            add(EMPTY,           EMPTY_ICO_NAME);
            add(FUNCCALL,        FUNCALL_ICO_NAME);
            add(UPDATE,          UPDATE_ICO_NAME);
            add(REQ,             REQUEST_ICO_NAME);
            add(LIBRARYNODECALL, LIBCALL_ICO_NAME);
            add(SPLASH_LOGO,      SPLASH_SCREEN_ICO);
            add(START_LOGO,      START_SCREEN_ICO);
         }
         
         public void add(String tag, String iconName)
         {
            put(tag, loadImage(iconName));
         }
      };

      public static ImageIcon getIcon(String tag)
      {
         return iconLut.get(tag);
      }

      // table view constants

      public static final int NAME_COL_NUM         = 0;
      public static final int STATE_COL_NUM        = 1;
      public static final int OUTCOME_COL_NUM      = 2;
      public static final int FAILURE_TYPE_COL_NUM = 3;
      
      // Empty script default contents
      
      public static final String EMPTY_SCRIPT = "<PLEXILScript><Script></Script></PLEXILScript>";
       
      public static final String EMPTY_PLAN = "<PlexilPlan xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"plexil.xsd\"><Node NodeType=\"Empty\"><NodeId>EMPTY</NodeId></Node></PlexilPlan>";
       
      public static final String NAME_COL_NAME         = "Name";
      public static final String STATE_COL_NAME        = "State";
      public static final String OUTCOME_COL_NAME      = "Outcome";
      public static final String FAILURE_TYPE_COL_NAME = "Failure Type";

      // node states

      public static final String INACTIVE         = "INACTIVE";
      public static final String WAITING          = "WAITING";
      public static final String EXECUTING        = "EXECUTING";
      public static final String FINISHING        = "FINISHING";
      public static final String FINISHED         = "FINISHED";
      public static final String FAILING          = "FAILING";
      public static final String ITERATION_ENDED  = "ITERATION_ENDED";

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

      // node outcomes

      public static final String UNKNOWN = "UNKNOWN";
      public static final String SUCCESS = "SUCCESS";
      public static final String FAILURE = "FAILURE";
      public static final String SKIPPED = "SKIPPED";

      public static final String[] NODE_OUTCOMES = 
      {
         UNKNOWN,
         SUCCESS,
         FAILURE,
         SKIPPED,
      };

      // node failure types
      
      public static final String INFINITE_LOOP = "INFINITE_LOOP";
      public static final String PRE_CONDITION_FALSE = "PRE_CONDITION_FALSE";
      public static final String POST_CONDITION_FALSE = "POST_CONDITION_FALSE";
      public static final String INVARIANT_CONDITION_FALSE = "INVARIANT_CONDITION_FALSE";
      public static final String ANCESTOR_INVARIANT_CONDITION_FALSE = "ANCESTOR_INVARIANT_CONDITION_FALSE";
      public static final String PARENT_FAILED = "PARENT_FAILED";


      public static final String[] NODE_FAILURE_TYPES = 
      {
         INFINITE_LOOP,
         PRE_CONDITION_FALSE,
         POST_CONDITION_FALSE,
         INVARIANT_CONDITION_FALSE,
         ANCESTOR_INVARIANT_CONDITION_FALSE,
         PARENT_FAILED,
      };

      // table tree row colors

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

      static HashMap<String, Color> colorLut = new HashMap<String, Color>()
      {
         {
            // node state

            put(INACTIVE,        Color.LIGHT_GRAY);
            put(WAITING,         Color.YELLOW.darker()); 
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

      // model color tags

      public static final String MODEL_ENABLED_BREAKPOINTS = "model.breakpoint.enabled";
      public static final String MODEL_DISABLED_BREAKPOINTS = "model.breakpoint.disabled";

      public static Color lookupColor(String value)
      {
         return colorLut.get(value);
      }

      // properties

      public static final String    PROP_FILE_AUTO_LOAD = "file.autoload";
      public static final boolean   PROP_FILE_AUTO_LOAD_DEF = true;
      public static final String    PROP_JAVA_LIB_PATH = "java.library.path";
      public static final String    PROP_FILE_RECENT_PLAN_BASE = "file.recent-plan-";
      public static final String    PROP_FILE_RECENT_SCRIPT_BASE = "file.recent-script-";
      public static final String    PROP_FILE_RECENT_PLAN_DIR = "file.recent-plan-directory";
      public static final String    PROP_FILE_RECENT_SCRIPT_DIR = "file.recent-script-directory";
      public static final String    PROP_FILE_RECENT_LIB_BASE  = "file.recent-lib-";
      public static final String    PROP_FILE_RECENT_LIB_DIR = "file.recent-library-directory";
      public static final String    PROP_FILE_RECENT_TEST_DIR = "file.recent-test-directory";
      public static final String    PROP_FILE_RECENT_COUNT = "file.recent-count";
      public static final int       PROP_FILE_RECENT_COUNT_DEF = 10;
      
      public static final String    PROP_MAC_MENU_STYLE = "apple.laf.useScreenMenuBar";

      public static final String    PROP_WIN_LOC        = "window.location";
      public static final Point     PROP_WIN_LOC_DEF    = new Point(50, 50);
      public static final String    PROP_WIN_SIZE       = "window.size";
      public static final Dimension PROP_WIN_SIZE_DEF   = new Dimension(1200, 650);
      public static final String    PROP_WIN_BCLR       = "window.background";
      public static final Color     PROP_WIN_BCLR_DEF   = Color.WHITE;

      public static final String    PROP_TOOLTIP_DISMISS      = "tootip.dismiss"; 
      public static final int       PROP_TOOLTIP_DISMISS_DEF  = 60000;

      public static final String    PROP_DBWIN_LOC        = "dbwindow.location";
      public static final Point     PROP_DBWIN_LOC_DEF    = new Point(50, 50);
      public static final String    PROP_DBWIN_SIZE       = "dbwindow.size";
      public static final Dimension PROP_DBWIN_SIZE_DEF   = new Dimension(1200, 300);
      public static final String    PROP_DBWIN_VISIBLE    = "dbwindow.visible";
      public static final boolean   PROP_DBWIN_VISIBLE_DEF= false;

      public static final String    PROP_NET_AUTO_CONNECT     = "net.autoconnect";
      public static final boolean   PROP_NET_AUTO_CONNECT_DEF = true;
      public static final String    PROP_NET_RECENT_HOST      = "net.host";
      public static final String    PROP_NET_RECENT_HOST_DEF  = "localhost";
      public static final String    PROP_NET_SERVER_PORT      = "net.server.port";
      public static final int       PROP_NET_SERVER_PORT_DEF  = 9787;

      public static final String    PROP_TTV_COL_WIDTH_BASE   = "treetable.colwidth-";
      public static final String    PROP_TTV_TEXT_TYPES    = "treetable.text-types";

      public static final String    PROP_VIEW_HIDE_PLEXILLISP  = "view.hide-plexilisp";
      public static final boolean   PROP_VIEW_HIDE_PLEXILLISP_DEF  = false;

      public static final String    PROP_PLEXIL_HOME = "PLEXIL_HOME";
      public static final String    PROP_USER_HOME = "user.home";
      public static final String    PROP_FILE_SEPARATOR = "file.separator";
      
      public static final String    PROP_UE_EXEC =        
              System.getenv(PROP_PLEXIL_HOME) + 
              System.getProperty(PROP_FILE_SEPARATOR) + "apps" + 
              System.getProperty(PROP_FILE_SEPARATOR) + "TestExec" + 
              System.getProperty(PROP_FILE_SEPARATOR) + "test-exec_g_rt";
      
      public static final String    PROP_RECENT_FILES =   
              System.getenv(PROP_PLEXIL_HOME) + 
              System.getProperty(PROP_FILE_SEPARATOR) + "apps" + 
              System.getProperty(PROP_FILE_SEPARATOR) + "TestExec";
      
      public static final String    PROP_LOG_FILE =
      	    System.getProperty(PROP_USER_HOME) + 
            System.getProperty(PROP_FILE_SEPARATOR) + "luv.log";
      
      public static final int PROP_RECENT_FILE = 1;

      // file

      public static final String XML_EXTENSION = "xml";
      public static final String PLX_EXTENSION = "plx";
      
      public static final String DEBUG_LOG_FILE = "debug.log";
      
      // miscellaneous constants
      
      public static final String ROOT = "root";
      public static final String DUMMY = "dummy";
      public static final String RECENT_PLANS = "Recent Plans";
      public static final String VIEW = "View";
      public static final String DEBUG_WINDOW = "Debug Window";
      public static final String FILE = "File";
      public static final String RUN = "Run";
      public static final String BLANK = " ";
      public static final String ENABLE = "Enable";
      public static final String DISABLE = "Disable";
      
      public static final String FONT = "Monospaced";
      
      public static final String NODE_POPUP_MENU = "Node Popup Menu";
      
      public static final String CLEAR_SCREEN = "Clear Screen";
      
      public static final String START_LOGO = "Clear Screen";
      public static final String SPLASH_LOGO = "Splash Screen";
      
      public static final String DEFAULT_TITLE = "Welcome to Luv Viewer";
      public static final String SCRIPT_TITLE = " SCRIPT: ";
      public static final String PLAN_TITLE = "PLAN: ";
      
      public static final String DEFAULT_SCRIPT_NAME = "default-empty-script.plx";
      
      public static final String PAUSE_OR_RESUME_PLAN = "Pause or Resume plan";
      public static final String STEP = "Step";
      public static final String ENABLE_BREAKS = "Enable Breaks";
      public static final String DISABLE_BREAKS = "Disable Breaks";
      public static final String BREAK_POINTS = "Break Points";
      public static final String EXECUTE_PLAN = "Execute Plan";
      public static final String STOP_EXECUTION = "Stop Execution";
 
      //File menu
      public static final int OPEN_PLAN_MENU_ITEM           = 0;
      public static final int OPEN_SCRIPT_MENU_ITEM         = 1;
      public static final int OPEN_RECENT_MENU_ITEM         = 2;
      public static final int RELOAD_MENU_ITEM              = 3;
      public static final int EXIT_MENU_ITEM                = 5;    // 5 is used because there is a separator element that is the 4th indexed element
      
      // run menu
      public static final int PAUSE_RESUME_MENU_ITEM        = 0;
      public static final int STEP_MENU_ITEM                = 1;
      public static final int BREAK_MENU_ITEM               = 2;
      public static final int EXECUTE_MENU_ITEM             = 3;
      
      // view menu
              
      public static final int EXPAND_MENU_ITEM              = 0;
      public static final int COLLAPSE_MENU_ITEM            = 1;
      public static final int TOGGLE_TEXT_TYPES_MENU_ITEM   = 2;
      public static final int TOGGLE_LISP_NODES_MENU_ITEM   = 3;
      
      // debug menu
     
      public static final int SHOW_CONDITIONS_MENU_ITEM     = 0;
      public static final int SHOW_LUV_DEBUG_MENU_ITEM      = 1;
      public static final int SHOW_CMD_DEBUG_MENU_ITEM      = 2;
      
      // file types 
      
      public static final int PLAN                          = 0;
      public static final int SCRIPT                        = 1;
      public static final int DEBUG                         = 2;
      
      // menus
      
      public static final int FILE_MENU                     = 0;
      public static final int RECENT_FILE_MENU              = 1;
      public static final int RUN_MENU                      = 2;
      public static final int VIEW_MENU                     = 3;
      public static final int WINDOW_MENU                   = 4;
      
      // luv boolean variables
      
      public static final int ALLOW_BREAKS                  = 0;
      public static final int EXEC_VIA_LUV                  = 1;
      public static final int EXEC_VIA_CMD_PRMPT            = 2;
      public static final int IS_EXECUTING                  = 3;
      public static final int AT_START_SCREEN               = 4;
      public static final int DONT_LOAD_SCRIPT_AGAIN        = 5;
      public static final int STOP_SRCH_LIBS                = 6;
      public static final int OPEN_PLN_VIA_LUV              = 7;
      public static final int PLAN_PAUSED                   = 8;
      public static final int PLAN_STEP                     = 9;
      public static final int SHOW_CONDITIONS               = 10;
      public static final int STOPPED_EXECUTION             = 11;
      
      // Luv Viewer States
      
      public static final int NULL_STATE                    = 0;
      public static final int START_STATE                   = 1;
      public static final int READY_STATE                   = 2;
      public static final int EXECUTION_STATE               = 3;
      public static final int LUV_VIEWER_EXECUTION_STATE    = 4;
      public static final int CMD_PROMPT_EXECUTION_STATE    = 5;
      public static final int PAUSED_STATE                  = 6;
      public static final int DISABLED_BREAKING_STATE       = 7;
      public static final int ENABLED_BREAKING_STATE        = 8;
      public static final int END_STATE                     = 9;
      
      public static final String YES = "Yes";
      public static final String NO = "No";
      public static final String EXIT = "Exit";
      public static final String PAUSE = "Pause";
      public static final String RESUME = "Resume";
      
      public static final String OPEN_PLAN = "Open Plan";
      public static final String OPEN_SCRIPT = "Open Script";
      public static final String RELOAD_PLAN = "Reload Plan";
      public static final String SHOW_LUV_VIEWER_DEBUG_WINDOW = "Show Luv Viewer Debug Window";
      public static final String HIDE_LUV_VIEWER_DEBUG_WINDOW = "Hide Luv Viewer Debug Window";
      public static final String SHOW_CMD_PROMPT_DEBUG_WINDOW = "Show Command Prompt Debug Window";
      public static final String HIDE_CMD_PROMPT_DEBUG_WINDOW = "Hide Command Prompt Debug Window";

      // boolean constant values

      public static final String TRUE = "TRUE";
      public static final String FALSE = "FALSE";

      // model property tags

      public static final String MODEL_NAME         = "ModelName";
      public static final String MODEL_TYPE         = "ModelType";
      public static final String MODEL_STATE        = "ModelState";
      public static final String MODEL_OUTCOME      = "ModelOutcome";
      public static final String MODEL_FAILURE_TYPE = "ModelFailureType";
      public static final String MODEL_LIBRARY_CALL_ID = "ModelLibraryCallId";
      
      public static final String MODEL_VAR_NAME = "ModelVariableName";

      /////////////////////////// XML tags /////////////////////////

      // plan tags

      public static final String PLEXIL_PLAN       = "PlexilPlan";
      public static final String PLEXIL_SCRIPT     = "PlexilScript";
      public static final String LIBRARY           = "Library";
      public static final String NODE_STATE_UPDATE = "NodeStateUpdate";
      public static final String NODE_PATH_ELEMENT = "NodePathElement";
      
      // plan info tags

      public static final String PLAN_INFO     = "PlanInfo";
      public static final String PLAN_FILENAME = "PlanFilename";
      public static final String SCRIPT_FILENAME = "ScriptFilename";
      public static final String LIBRARY_FILENAME = "LibraryFilename";
      public static final String LIBRARY_COUNT = "LibraryCount";
      public static final String VIEWER_BLOCKS = "ViewerBlocks";
      
      // condition tags

      public static final String CONDITION = "Condition";
      public static final String SKIP_CONDITION = "SkipCondition";
      public static final String START_CONDITION = "StartCondition";
      public static final String END_CONDITION = "EndCondition";
      public static final String INVARIANT_CONDITION = "InvariantCondition";
      public static final String PRE_CONDITION = "PreCondition";
      public static final String POST_CONDITION = "PostCondition";
      public static final String REPEAT_CONDITION = "RepeatCondition";
      public static final String ANCESTOR_INVARIANT_CONDITION = "AncestorInvariantCondition";
      public static final String ANCESTOR_END_CONDITION = "AncestorEndCondition";
      public static final String PARENT_EXECUTING_CONDITION = "ParentExecutingCondition";
      public static final String PARENT_FINISHED_CONDITION = "ParentFinishedCondition";
      public static final String CHILDREN_WAITING_OR_FINISHED = "AllChildrenWaitingOrFinishedCondition";
      public static final String ABORT_COMPLETE = "AbortCompleteCondition";
      public static final String PARENT_WAITING_CONDITION = "ParentWaitingCondition";
      public static final String COMMAND_HANDLE_RECEIVED_CONDITION = "CommandHandleReceivedCondition";

      public static final int SKIP_CONDITION_NUM                    = 0;
      public static final int START_CONDITION_NUM                   = 1;
      public static final int END_CONDITION_NUM                     = 2;
      public static final int INVARIANT_CONDITION_NUM               = 3;
      public static final int PRE_CONDITION_NUM                     = 4;
      public static final int POST_CONDITION_NUM                    = 5;
      public static final int REPEAT_CONDITION_NUM                  = 6;
      public static final int ANCESTOR_INVARIANT_CONDITION_NUM      = 7;
      public static final int ANCESTOR_END_CONDITION_NUM            = 8;
      public static final int PARENT_EXECUTING_CONDITION_NUM        = 9;
      public static final int PARENT_FINISHED_CONDITION_NUM         = 10;
      public static final int CHILDREN_WAITING_OR_FINISHED_NUM      = 11;
      public static final int ABORT_COMPLETE_NUM                    = 12;
      public static final int PARENT_WAITING_CONDITION_NUM          = 13;
      public static final int COMMAND_HANDLE_RECEIVED_CONDITION_NUM = 14;
      
      public static int getConditionNum(String condition)
      {
          if (condition.equals(SKIP_CONDITION))                         return SKIP_CONDITION_NUM;    
          else if (condition.equals(START_CONDITION))                   return START_CONDITION_NUM;                
          else if (condition.equals(END_CONDITION))                     return END_CONDITION_NUM;   
          else if (condition.equals(INVARIANT_CONDITION))               return INVARIANT_CONDITION_NUM; 
          else if (condition.equals(PRE_CONDITION))                     return PRE_CONDITION_NUM; 
          else if (condition.equals(POST_CONDITION))                    return POST_CONDITION_NUM;
          else if (condition.equals(REPEAT_CONDITION))                  return REPEAT_CONDITION_NUM;
          else if (condition.equals(ANCESTOR_INVARIANT_CONDITION))      return ANCESTOR_INVARIANT_CONDITION_NUM;
          else if (condition.equals(ANCESTOR_END_CONDITION))            return ANCESTOR_END_CONDITION_NUM;           
          else if (condition.equals(PARENT_EXECUTING_CONDITION))        return PARENT_EXECUTING_CONDITION_NUM;   
          else if (condition.equals(PARENT_FINISHED_CONDITION))         return PARENT_FINISHED_CONDITION_NUM;       
          else if (condition.equals(CHILDREN_WAITING_OR_FINISHED))      return CHILDREN_WAITING_OR_FINISHED_NUM;
          else if (condition.equals(ABORT_COMPLETE))                    return ABORT_COMPLETE_NUM;                  
          else if (condition.equals(PARENT_WAITING_CONDITION))          return PARENT_WAITING_CONDITION_NUM;        
          else if (condition.equals(COMMAND_HANDLE_RECEIVED_CONDITION)) return COMMAND_HANDLE_RECEIVED_CONDITION_NUM;
          else                                                          return -1; //error
      }
      
      public static String getConditionName(String condition)
      {
          if (condition.equals(SKIP_CONDITION))                         return "Skip";    
          else if (condition.equals(START_CONDITION))                   return "Start";                
          else if (condition.equals(END_CONDITION))                     return "End";   
          else if (condition.equals(INVARIANT_CONDITION))               return "Invariant"; 
          else if (condition.equals(PRE_CONDITION))                     return "Pre"; 
          else if (condition.equals(POST_CONDITION))                    return "Post";
          else if (condition.equals(REPEAT_CONDITION))                  return "Repeat";
          else if (condition.equals(ANCESTOR_INVARIANT_CONDITION))      return "Ancestor Invariant";
          else if (condition.equals(ANCESTOR_END_CONDITION))            return "Ancestor End";           
          else if (condition.equals(PARENT_EXECUTING_CONDITION))        return "Parent Executing";   
          else if (condition.equals(PARENT_FINISHED_CONDITION))         return "Parent Finished";       
          else if (condition.equals(CHILDREN_WAITING_OR_FINISHED))      return "Children Waiting or Finished";
          else if (condition.equals(ABORT_COMPLETE))                    return "Abort Complete";                  
          else if (condition.equals(PARENT_WAITING_CONDITION))          return "Parent Waiting";        
          else if (condition.equals(COMMAND_HANDLE_RECEIVED_CONDITION)) return "Command Handle Received";
          else                                                          return ""; //error
      }

      // the set of all condtions

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

      // displayed node types

      public static final String DISPLAY_LIST_NODE_TYPE   = "";
      public static final String DISPLAY_COMMAND_NODE_TYPE   = "";
      public static final String DISPLAY_ASSIGNMENT_NODE_TYPE   = "";
      public static final String DISPLAY_EMPTY_NODE_TYPE   = "";
      public static final String DISPLAY_FUNCTIONCALL_NODE_TYPE   = "";
      public static final String DISPLAY_UPDATE_NODE_TYPE   = "";

      // tags adapted from UE code

      public static final String SPLASH_PAGE ="SplashPage";
      public static final String NODE = "Node";
      public static final String NODE_ID = "NodeId";
      public static final String PRIORITY = "Priority";
      public static final String PERMISSIONS = "Permissions";
      public static final String INTERFACE = "Interface";
      public static final String VAR_DECLS = "VariableDeclarations";
      public static final String DECL_VAR = "DeclareVariable";
      public static final String IN = "In";
      public static final String INOUT = "InOut";
      public static final String VAR = "Variable";
      public static final String TYPE = "Type";
      public static final String MAXSIZE = "MaxSize";
      public static final String DECL = "Declare";
      public static final String VAL = "Value";
      public static final String INITIALVAL = "InitialValue";
      public static final String ASSN = "Assignment";
      public static final String EMPTY = "Empty";
      public static final String BODY = "NodeBody";
      public static final String RHS = "RHS";
      public static final String NUMERIC_RHS = "NumericRHS";
      public static final String NODELIST = "NodeList";
      public static final String LIBRARYNODECALL = "LibraryNodeCall";
      public static final String ALIAS = "Alias";
      public static final String NODE_PARAMETER = "NodeParameter";
      public static final String COMMAND = "Command";
      public static final String COMMAND_NAME = "CommandName";
      public static final String FUNCCALL = "FunctionCall";
      public static final String FUNCCALL_NAME = "FunctionName";
      public static final String NAME = "Name";
      public static final String INDEX = "Index";
      public static final String ARGS = "Arguments";
      public static final String LOOKUPNOW = "LookupNow";
      public static final String LOOKUPCHANGE = "LookupOnChange";
      public static final String LOOKUPFREQ = "LookupWithFrequency";
      public static final String FREQ = "Frequency";
      public static final String HIGH = "High";
      public static final String LOW = "Low";
      public static final String TOLERANCE = "Tolerance";
      public static final String NODEREF = "NodeRef";
      public static final String STATE_VAL = "NodeStateValue";
      public static final String STATE_NAME = "StateName";
      public static final String TIMEPOINT = "Timepoint";
      public static final String UPDATE = "Update";
      public static final String REQ = "Request";
      public static final String PAIR = "Pair";
      public static final String COND = "Condition";
      public static final String ARRAY_VAL = "ArrayValue";

      // operators (adapted from UE code)

      public static final String AND = "AND";
      public static final String OR = "OR";
      public static final String XOR = "XOR";
      public static final String NOT = "NOT";
      public static final String CONCAT = "Concat";
      public static final String IS_KNOWN = "IsKnown";
      public static final String EQ = "EQ";
      public static final String EQ_NUMERIC = "EQNumeric";
      public static final String EQ_STRING = "EQString";
      public static final String EQ_BOOLEAN = "EQBoolean";
      public static final String EQ_INTERNAL = "EQInternal";
      public static final String NE = "NE";
      public static final String NE_NUMERIC = "NENumeric";
      public static final String NE_STRING = "NEString";
      public static final String NE_BOOLEAN = "NEBoolean";
      public static final String NE_INTERNAL = "NEInternal";
      public static final String LT = "LT";
      public static final String LE = "LE";
      public static final String GT = "GT";
      public static final String GE = "GE";
      public static final String ADD = "ADD";
      public static final String SUB = "SUB";
      public static final String MUL = "MUL";
      public static final String DIV = "DIV";
      public static final String SQRT = "SQRT";
      public static final String ABS = "ABS";

      // types

      public static final String INT = "Integer";
      public static final String REAL = "Real";
      public static final String BOOL = "Boolean";
      public static final String BLOB = "String";
      public static final String ARRAY = "Array";
      public static final String DECL_ARRAY = "DeclareArray";
      public static final String ARRAYELEMENT = "ArrayElement";
      public static final String STRING = "String";
      public static final String TIME = "Time";

      // attributes

      public static final String NODETYPE_ATTR = "NodeType";
      public static final String DIR_ATTR = "dir";

      // values
      
      public static final String PARENT_VAL = "parent";
      public static final String CHILD_VAL = "child";
      public static final String SIBLING_VAL = "sibling";
      public static final String SELF_VAL = "self";

      // node properties

      public static final String NODE_OUTCOME      = "NodeOutcome";
      public static final String NODE_FAILURE      = "NodeFailure";
      public static final String NODE_FAILURE_TYPE = "NodeFailureType";
      public static final String NODE_STATE        = "NodeState";
      public static final String NODE_TIMEPOINT    = "NodeTimepoint";

      // variable references

      public static final String INT_VAR = INT + VAR;
      public static final String REAL_VAR = REAL + VAR;
      public static final String ARRAY_VAR = ARRAY + VAR;
      public static final String STRING_VAR = STRING + VAR;
      public static final String BOOL_VAR = BOOL + VAR;
      public static final String TIME_VAR = TIME + VAR;
      public static final String BLOB_VAR = BLOB + VAR;
      public static final String NODE_OUTCOME_VAR = NODE_OUTCOME + VAR;
      public static final String NODE_FAILURE_VAR = NODE_FAILURE + VAR;
      public static final String NODE_STATE_VAR = NODE_STATE + VAR;
      public static final String NODE_TIMEPOINT_VAR = NODE_TIMEPOINT + VAR;

      // values

      public static final String INT_VAL = INT + VAL;
      public static final String REAL_VAL = REAL + VAL;
      public static final String STRING_VAL = STRING + VAL;
      public static final String BOOL_VAL = BOOL + VAL;
      public static final String TIME_VAL = TIME + VAL;
      public static final String BLOB_VAL = BLOB + VAL;
      public static final String NODE_OUTCOME_VAL = NODE_OUTCOME + VAL;
      public static final String NODE_FAILURE_VAL = NODE_FAILURE + VAL;
      public static final String NODE_STATE_VAL = NODE_STATE + VAL;
      public static final String NODE_TIMEPOINT_VAL = NODE_TIMEPOINT + VAL;
      public static final String ASSN_VAL = ASSN + VAL;
      
      public static final String ASSN_NAME = "AssignmentName";

      /** tags for XML nodes which expand to children */

      public static final String[] CHILD_TAGS =
      {
         NODE,
         LIBRARYNODECALL,
//          COMMAND,

         // conditions
         
//          PRE_COND,
//          START_COND,
//          INVARIANT_COND,
//          END_COND,
//          POST_COND,
//          REPEAT_COND,
      };

      /** tags for XML nodes which become properties */

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
         LIBRARY_FILENAME,
         LIBRARY_COUNT,

         PLEXIL_PLAN,
         PLEXIL_SCRIPT,
         LIBRARY,

         NODETYPE_ATTR,
         NODE_ID,
         BODY,
         NODELIST,
//          COMMAND_NAME,
//          FUNCCALL_NAME,
          NAME,
          TYPE,
          MAXSIZE,
          INDEX,
//         ASSN_NAME,

//          // states

//          STATE_VAL,
//          STATE_NAME,
         
//          // values

//            ASSN_VAL,
          INT_VAL,
          REAL_VAL,
          STRING_VAL,
          BOOL_VAL,
//          TIME_VAL,
//          BLOB_VAL,
//          NODE_OUTCOME_VAL,
//          NODE_FAILURE_VAL,
//          NODE_STATE_VAL,
//          NODE_TIMEPOINT_VAL,
//          PRIORITY,
//          PERMISSIONS,
//          INTERFACE,
//          VAR_DECLS,
          DECL_VAR,
          DECL_ARRAY,
//          IN,
//          INOUT,
//          VAR,
//          DECL,
//          VAL,
//          INITIALVAL,
//          ASSN,
//          RHS,
//          LIBRARYNODECALL,
//          ALIAS,
//          NODE_PARAMETER,
//          COMMAND,
//          FUNCCALL,
//          ARGS,
//          LOOKUPNOW,
//          LOOKUPCHANGE,
//          LOOKUPFREQ,
//          FREQ,
//          HIGH,
//          LOW,
//          TOLERANCE,
//          NODEREF,
//          STATE_VAL,
//          STATE_NAME,
//          TIMEPOINT,
//          UPDATE,
//          REQ,
//          PAIR,
//          COND,

//          // operators

//          AND,
//          OR,
//          XOR,
//          NOT,
//          CONCAT,
//          IS_KNOWN,
//          EQ,
//          EQ_NUMERIC,
//          EQ_STRING,
//          EQ_BOOLEAN,
//          EQ_INTERNAL,
//          NE,
//          NE_NUMERIC,
//          NE_STRING,
//          NE_BOOLEAN,
//          NE_INTERNAL,
//          LT,
//          LE,
//          GT,
//          GE,
//          ADD,
//          SUB,
//          MUL,
//          DIV,
//          SQRT,
//          ABS,

//          // types

//          INT,
//          REAL,
//          BOOL,
//          BLOB,
//          ARRAY,
//          DECL_ARRAY,
          ARRAYELEMENT,
//          STRING,
//          TIME,

//          // variable references

//          INT_VAR,
//          REAL_VAR,
//          ARRAY_VAR,
//          STRING_VAR,
//          BOOL_VAR,
//          TIME_VAR,
//          BLOB_VAR,
//          NODE_OUTCOME_VAR,
//          NODE_FAILURE_VAR,
//          NODE_STATE_VAR,
//          NODE_TIMEPOINT_VAR,

//          // values

//          INT_VAL,
//          REAL_VAL,
//          STRING_VAL,
//          BOOL_VAL,
//          TIME_VAL,
//          BLOB_VAL,
//          NODE_OUTCOME_VAL,
//          NODE_FAILURE_VAL,
//          NODE_STATE_VAL,
//          NODE_TIMEPOINT_VAL,
      };
      
      static public ImageIcon loadImage(String name)
      {         
         return new ImageIcon(
            Toolkit.getDefaultToolkit()
            .getImage(ClassLoader.getSystemResource(ICONS_DIR + name)));
      }
}
