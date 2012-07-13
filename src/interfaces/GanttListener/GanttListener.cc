/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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
*
* By Madan, Isaac A.
*/

#include <iomanip> // for setprecision
#include "GanttListener.hh"
#include "Node.hh"
#include "Debug.hh"
#include "ExecListenerFactory.hh"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include "ExecDefs.hh"
#include <time.h>
#include <ctime>
#include <map>

#include "AdapterFactory.hh"
#include "CoreExpressions.hh"
#include "AdapterExecInterface.hh"
#include "Expression.hh"
#include "Debug.hh"

//necessary for getting working directories and environment variables
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

using std::cout;
using std::cin;
using std::ofstream;
using std::string;
using std::endl;
using std::vector;
using std::list;
using std::map;

namespace PLEXIL
{
  GanttListener::GanttListener () { }

  GanttListener::GanttListener (const pugi::xml_node& xml)
    : ExecListener(xml)
  { }

  GanttListener::~GanttListener () { }

  // For now, use the DebugMsg facilities (really intended for debugging the
  // *executive* and not plans) to display messages of interest.  Later, a more
  // structured approach including listener filters and a different user
  // interface may be in order.

  static string fullTemplate = "empty";
  static string myCloser = "];";

  //nodes
  struct nodeObj {
    string name;
    double start;
    double end;
    double duration;
    string type;
    string val;
    string parent;
    int id;
    string localvariables;
    string children;
    vector<string> localvarsvector;
  };

  //all the nodes
  static vector<nodeObj> nodes;

  //these values get reassigned for each node
  static string myId;
  static double myStartValdbl;
  static double myEndValdbl;
  static double myDurationValdbl;
  static string myType;
  static string myVal;
  static string myParent;
  static string myLocalVars;
  static string myChildren;

  static string myLocalVarsAfter;

  //these values are modified throughout plan execution
  static double nodeCounter = 0;
  static double actualId = -1;
  static double startTime = -1;

  static int index;

  static string myDirectory;
  static string uniqueFileName;
  static string ganttDirectory;
  static string plexilDirectory;
  static string plexilGanttDirectory;
  static string myHTMLFile;

  static map<NodeId, int> stateMap;
  static map<NodeId, int> counterMap;

  /** get the current time for the file name
   * example formatting Aug22_2011_01.28.42PM 
   * deprecated method due to verbose file naming; file name currently contains system time
   * uncomment line in createHTMLFile to use
   **/
  string getTime() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    strftime (buffer,80,"%b%d_%Y_%I.%M.%S%p",timeinfo);
    puts (buffer);
    string myTime (buffer);
    return myTime;
  }

  /** get working directory and environment variables **/
  void getCurrentWorkingDirectory() {
    char cCurrentPath[FILENAME_MAX];
    getcwd(cCurrentPath, FILENAME_MAX);
    myDirectory = cCurrentPath;

    /** get PLEXIL_HOME **/
    string pPath;
    try {
      pPath = getenv ("PLEXIL_HOME");
    }
    catch(int e) {
      debugMsg("GanttViewer:printErrors", "PLEXIL_HOME is not defined");
    }
    plexilDirectory = pPath;

    /** get Viewer directory under PLEXIL_HOME **/
    string pgPath = pPath + "/src/viewer";
    plexilGanttDirectory = pgPath + "/";
    debugMsg("GanttViewer:printProgress", "Current working directory set");
  }

  /** generate the HTML file at the end of a plan's execution that connects to necessary Javascript and produced JSON **/
  void createHTMLFile(const string& nodeName) {
    string tempName = uniqueFileName;
    //uncomment the following line to set filename to the format gantt_MMDD_YYYY_hour.min.sec_nodeName.html
    //uniqueFileName = getTime();
    string htmlFileName = myDirectory + "/" + "gantt_" + uniqueFileName + "_" + nodeName + ".html";
    string myTokenFileName = "json/" + uniqueFileName + "_" + nodeName + ".js";
    string lineBreak = "\n ";
    string htmlFile = 
      "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"+lineBreak+
      "<html lang=\"en\"> "+lineBreak+
      "<head> "+lineBreak+
      "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"> "+lineBreak+
      "<title>Gantt Temporal Plan Viewer</title> "+lineBreak+
      "<meta name=\"author\" content=\"By Madan, Isaac A. (ARC-TI); originally authored by Swanson, Keith J. (ARC-TI)\"> "+lineBreak+lineBreak+
      "<!-- jQuery is required --> "+lineBreak+
      "<script src=\""+plexilGanttDirectory+"jq/jquery-1.6.2.js\" type=\"text/javascript\"></script> "+lineBreak+
      "<link type=\"text/css\" href=\""+plexilGanttDirectory+"jq/jquery-ui-1.8.15.custom.css\" rel=\"Stylesheet\" /> "+lineBreak+
      "<script type=\"text/javascript\" src=\""+plexilGanttDirectory+"jq/jquery-ui-1.8.15.custom.min.js\"></script> "+lineBreak+lineBreak+
      "<!-- Load data locally --> "+lineBreak+
      "<script src=\""+plexilGanttDirectory+myTokenFileName+"\" type=\"text/javascript\"></script> "+lineBreak+lineBreak+
      "<!-- Application code --> "+lineBreak+      
      "<script src=\""+plexilGanttDirectory+"addons.js\" type=\"text/javascript\"></script> "+lineBreak+
      "<script src=\""+plexilGanttDirectory+"getAndConvertTokens.js\" type=\"text/javascript\"></script> "+lineBreak+
      "<script src=\""+plexilGanttDirectory+"showTokens.js\" type=\"text/javascript\"></script> "+lineBreak+
      "<script src=\""+plexilGanttDirectory+"detailsBox.js\" type=\"text/javascript\"></script> "+lineBreak+
      "<script src=\""+plexilGanttDirectory+"grid.js\" type=\"text/javascript\"></script> "+lineBreak+
      "<script src=\""+plexilGanttDirectory+"sizing.js\" type=\"text/javascript\"></script> "+lineBreak+
      "<script src=\""+plexilGanttDirectory+"main.js\" type=\"text/javascript\"></script> "+lineBreak+
      "<script src=\""+plexilGanttDirectory+"shortcuts.js\" type=\"text/javascript\"></script> "+lineBreak+lineBreak+
      "<!-- My styles --> "+lineBreak+
      "<link rel=\"stylesheet\" href=\""+plexilGanttDirectory+"styles.css\" type=\"text/css\"> "+lineBreak+
      "</head> \n <body> "+lineBreak+lineBreak+
      "<!-- Layout --> "+lineBreak+
      "<div id=\"footer\"></div> "+lineBreak+
      "<div id=\"mod\"></div> "+lineBreak+
      "<div id=\"gantt\"></div> "+lineBreak+
      "</body> "+lineBreak+
      "</html>";

    ofstream myfile;
    myfile.open(htmlFileName.c_str());
    myfile << htmlFile;
    myfile.close();

    myHTMLFile = "\n \n var myHTMLFilePathString =\"" + htmlFileName + "\";";
    debugMsg("GanttViewer:printProgress", "HTML file written to "+htmlFileName);
  }
     

  /** generate the JSON tokens file at the end of a plan's execution so that it can be parsed by Javascript in the Viewer **/
  void deliverAsFile(const string& fullTemplate, const string& myCloser, const string& nodeName) {
    ofstream myfile;
    uniqueFileName = plexilGanttDirectory + "/json/" + uniqueFileName + "_" + nodeName + ".js";
    myfile.open(uniqueFileName.c_str());
    myfile << fullTemplate << myCloser << myHTMLFile;
    myfile.close();
    debugMsg("GanttViewer:printProgress", "JSON tokens file written to "+uniqueFileName);
  }

  /** executed when the plan is added 
   *  gets the current directory and environment variables, 
   *  sets the header of the template,
   *  sets the start time of the plan's execution
   *  for use in file name
  **/
  void GanttListener::implementNotifyAddPlan(const PlexilNodeId& plan, 
                                            const LabelStr& parent) const 
  {
    getCurrentWorkingDirectory();
	// FIXME: Get time from someplace!
    startTime = 0;
    startTime = (int) startTime;
    std::ostringstream uFileName;
    uFileName.precision(10);
    uFileName << startTime;
    uniqueFileName = uFileName.str();
    fullTemplate = "var rawPlanTokensFromFile=\n[\n";
    //reset startTime; it will be set when first node executes
    startTime = -1;
    debugMsg("GanttViewer:printProgress", "GanttListener notified of plan; start time for filename set");
  }

  /** executed when nodes transition state
   *  resets the start time so it can be used in temporal calculations,
   *  grabs info from nodes in executing state,
   *  grabs info from nodes in finished state,
   *  nodes info is stored in each node's nodeObj struct
   **/
  void GanttListener::
  implementNotifyNodeTransition (NodeState prevState, const NodeId& nodeId) const
  {
    //startTime is when first node executes
    if(startTime == -1) startTime = nodeId->getCurrentStateStartTime();

    //make sure the temporary variables are cleaned out
    myId = " ";
    myStartValdbl = -1;
    myType = " ";
    myVal = " ";
    myParent = " ";
    
    //get state
    const NodeState& newState = nodeId->getState();
    if(newState == EXECUTING_STATE) {  
      myId = nodeId->getNodeId().toString();
      myStartValdbl= ((nodeId->getCurrentStateStartTime()) - startTime) * 100;
      myType = nodeId->getType().toString();
      myVal = nodeId->getStateName().toString();
      myParent = "none";
      if (nodeId->getParent().isId()) {
	myParent = nodeId->getParent()->getNodeId().toString();
      }
      if (myParent == " ") {
	myParent = nodeId->getNodeId().toString();
      }

      //increase nodeCounter for ID value
      nodeCounter += 1;
      actualId = nodeCounter; //actualId ensures that looping nodes have the same ID for each token

      //determine if a node looping; assign prior ID for loops and a new one for non loops
      stateMap[nodeId] += 1;
      if(stateMap[nodeId] > 1) {
	actualId = counterMap[nodeId];
      }
      else {
	counterMap[nodeId] = actualId;
      }

      //get local variables from map in state 'EXECUTING'
      vector<string> myLocalVariableMapValues;
      myLocalVars = " ";
      vector<string> myLocalVariableMap;
      VariableMap tempLocalVariablesMap = nodeId->getLocalVariablesByName();
      VariableMap::iterator it;
      int tempSize = tempLocalVariablesMap.size();
      if(tempSize == 0) myLocalVars = "none";
      for(it=tempLocalVariablesMap.begin(); it!=tempLocalVariablesMap.end(); it++) {
	double tempKey = (*it).first;
        ExpressionId temp = (*it).second;
	string tempValueString = temp->getId()->valueString();
	string tempKeyString = LabelStr(tempKey).toString();
	string tempString = "<br><i>" + tempKeyString + "</i>" + " = " + tempValueString;
	myLocalVariableMapValues.push_back(tempValueString);
	myLocalVariableMap.push_back(tempString);
	//filter out local variables that are 'state' key  or 'UNKNOWN' value
	if(tempKeyString != "state" && tempValueString != "UNKNOWN")
	  myLocalVars = myLocalVars + tempString + ", ";
      }
     
      //get child nodes
      myChildren = " ";
      vector<string> myChildNodes;
      const vector<NodeId>& tempChildList = nodeId->getChildren();
      if (tempChildList.size() == 0) myChildren = "none";
      else for (vector<NodeId>::const_iterator i =  
		tempChildList.begin(); i != tempChildList.end(); i++) {
          string tempString = ((NodeId) *i)->getNodeId().toString();
          myChildNodes.push_back(tempString);
          myChildren += tempString + ", ";
        }

      //convert actualId to string
      std::ostringstream ndcntr;
      ndcntr << actualId;
      string myNumber = ndcntr.str();

      //setup nodeObj and add to vector
      nodeObj temp;
      temp.name = myId;
      temp.start = myStartValdbl;
      temp.end = -1; //not yet known
      temp.duration = -1; //not yet known
      temp.type = myType;
      temp.val = myVal;
      temp.parent = myParent;
      temp.id = actualId;
      temp.children = myChildren;
      temp.localvariables = myLocalVars;
      temp.localvarsvector = myLocalVariableMapValues;
      nodes.push_back(temp);
    }

    if(newState == FINISHED_STATE) {
      //find the node it corresponds to in nodes vector
      string tempId = nodeId->getNodeId().toString();
      string tempType = nodeId->getType().toString();
      string tempParent = "invalid_parent_id";
      if(nodeId->getParent().isId()) tempParent = nodeId->getParent()->getNodeId().toString();
      for(size_t i=0; i<nodes.size(); i++) {
	if(tempParent != "invalid_parent_id") {
	  if(tempId==nodes[i].name 
	     && tempType==nodes[i].type
	     && tempParent==nodes[i].parent) {
	    index = i;
	  }
	}
	else {
	  if(tempId==nodes[i].name
	     && tempType==nodes[i].type) {
	    index = i;
	  }
	}
      }
      myEndValdbl = ((nodeId->getCurrentStateStartTime()) - startTime)*100;
      myDurationValdbl = myEndValdbl - nodes[index].start;
      //doesn't exist until node is finished     
      string myOutcome = nodeId->getOutcome().toString();
      if (nodeId->getParent().isId())
	myParent = nodeId->getParent()->getNodeId().toString();
      if(myParent == " ")
	myParent = nodes[index].name;
     
      string myTypeID;

      //get final values for local variables
      if(nodes[index].localvariables != "none" && nodes[index].localvarsvector.size() > 0) {
	vector<string> prevLocalVarsVector = nodes[index].localvarsvector;
	vector<string> thisLocalVarsVectorKeys;
	vector<string> thisLocalVarsVectorValues;

	VariableMap tempLocalVariableMapAfter = nodeId->getLocalVariablesByName();
	VariableMap::iterator it;
	int tempSize = tempLocalVariableMapAfter.size();
	if(tempSize == 0) myLocalVarsAfter = "none";
	for(it = tempLocalVariableMapAfter.begin(); it != tempLocalVariableMapAfter.end(); it++) {
	  double tempKey = (*it).first;
	  ExpressionId temp = (*it).second;
	  string tempValueString = temp->getId()->valueString();
	  string tempKeyString = LabelStr(tempKey).toString();
	  thisLocalVarsVectorKeys.push_back(tempKeyString);
	  thisLocalVarsVectorValues.push_back(tempValueString);
	}
	vector<string> fullStrings;

	//first local variable key should always be state; this makes sure it is
	int myCount = 0;
	while(thisLocalVarsVectorKeys.size() > prevLocalVarsVector.size() 
	      && thisLocalVarsVectorKeys[myCount] != "state") {
	  prevLocalVarsVector.insert(prevLocalVarsVector.begin(), "UNKNOWN");
	  myCount++;
	}

	//first local VALUE should always be 'EXECUTING' (or maybe 'FINISHED'); this makes sure it is
	myCount = 0;
	while(prevLocalVarsVector.size() > thisLocalVarsVectorKeys.size() 
	      && prevLocalVarsVector[myCount] != "EXECUTING" 
	      && prevLocalVarsVector[myCount] != "FINISHED") {
	  prevLocalVarsVector.erase(prevLocalVarsVector.begin());
	  myCount++;
	}

	//make sure all local variable vectors are filled
	if(prevLocalVarsVector.size() > 1 && thisLocalVarsVectorKeys.size() > 1 && thisLocalVarsVectorValues.size() > 1) {
	  int smallerSize;
	  if(prevLocalVarsVector.size() < thisLocalVarsVectorKeys.size())  smallerSize = prevLocalVarsVector.size();
	  else smallerSize = thisLocalVarsVectorKeys.size();
	  for(int i = 0; i < smallerSize; i++) {
	      //filter out local variables that are UNKNOWN at beginning of execution and at end of execution
	      if(prevLocalVarsVector[i] != "UNKNOWN" || thisLocalVarsVectorValues[i] != "UNKNOWN") {
		string tempFullString;
		//bolden final local variable values that changed during execution of node
		if(prevLocalVarsVector[i] != thisLocalVarsVectorValues[i]) {
		  tempFullString = "<i>" +  thisLocalVarsVectorKeys[i] + "</i>" + " = " + prevLocalVarsVector[i] + " --><strong><font color=\"blue\"> " + thisLocalVarsVectorValues[i] + "</strong></font>";
		}
		else {
		  tempFullString = "<i>" +  thisLocalVarsVectorKeys[i] + "</i>" + " = " + prevLocalVarsVector[i] + " --> " + thisLocalVarsVectorValues[i];
		} 
		fullStrings.push_back(tempFullString);
	      }
	    }
	    myLocalVarsAfter = " ";
	    for(size_t i = 0; i < fullStrings.size(); i++) {
	      myLocalVarsAfter += "<br>" + fullStrings[i] + ", ";
	    }
	}
	else {
	  myLocalVarsAfter = "none";
	}
      }
      else {
	myLocalVarsAfter = "none";
      }

      //add temp values to node
      nodes[index].end = myEndValdbl;
      nodes[index].duration = myDurationValdbl;
      nodes[index].parent = myParent;
      nodes[index].localvariables = myLocalVarsAfter;

      //add node info into variables for JSON string
      string myPredicate = nodes[index].name;
      string myEntity = nodes[index].type;
      string myNodeNameLower = nodes[index].parent;
      string myNodeNameReg = nodes[index].parent;
      string myNewVal = nodes[index].val;
      string myChildrenVal = nodes[index].children;
      string myLocalVarsVal = nodes[index].localvariables;
      
      //get rid of extra comma and space at end
      if(myChildrenVal != "none") {
	myChildrenVal.erase(myChildrenVal.end()-2);
      }
      if(myLocalVarsVal != "none") {
	myLocalVarsVal.erase(myLocalVarsVal.end()-2);
      }

      //convert node id number, start time, end time, and duration to strings
      std::ostringstream ndcntr;
      ndcntr << nodes[index].id;
      string myNumber = ndcntr.str();

      std::ostringstream strs;
      strs << nodes[index].start;
      string myStartVal = strs.str();

      std::ostringstream strs2;
      strs2 << nodes[index].end;
      string myEndVal = strs2.str();

      std::ostringstream strs3;
      strs3 << nodes[index].duration;
      string myDurationVal = strs3.str();

      /** Some notes
       * myPredicate is this node name (myId)
       * myEntity is this node type (myType)
       * myNodeNameLower and myNodeNameReg are parent node name (myParent)
       **/

      //add '[' and ']' before and after duration and start to add uncertainty to those values
      //setup JSON object to be added to array
      string newTemplate = "{\n'id': "
    +myNumber+
    ",\n'type':'"
    +myPredicate+
    "',\n'parameters': [\n{\n'name': 'entityName',\n'type': 'STRING',\n'value':'"
    +myEntity+
    "'\n},\n{\n'name': 'full type',\n'type': 'STRING',\n'value': '"
    +myNodeNameLower+
    "."
    +myPredicate+
    "'\n},\n{\n'name': 'state',\n'type': 'STRING',\n'value': 'ACTIVE'\n},\n{\n'name': 'object',\n'value': 'OBJECT:"
    +myNodeNameReg+
    "(6)'\n},\n{\n'name': 'duration',\n'type': 'INT',\n'value': '"
    +myDurationVal+
    "'\n},\n{\n'name': 'start',\n'type': 'INT',\n'value': '"
    +myStartVal+
    "'\n},\n{\n'name': 'end',\n'type': 'INT',\n'value': '"
    +myEndVal+
    "'\n},\n{\n'name': 'value',\n'type': 'INT',\n'value': '"
    +myNewVal+
    "'\n},\n{\n'name': 'children',\n'type': 'INT',\n'value': '"
    +myChildrenVal+
    "'\n},\n{\n'name': 'localvariables',\n'type': 'INT',\n'value': '"
    +myLocalVarsVal+
    "'\n}\n]\n},\n";

  //add JSON object to existing array
  fullTemplate = fullTemplate + newTemplate;
  debugMsg("GanttViewer:printProgress", "Token added for node "+myEntity+"."+myPredicate);
  
  // if it is the last token, create HTML and add the tokens to the js file
  if(myNumber == "1") { 
    createHTMLFile(myNodeNameLower);
    deliverAsFile(fullTemplate, myCloser, myNodeNameLower); 
    debugMsg("GanttViewer:printProgress","finished gathering data; JSON and HTML stored");
  }
  }
  }



  extern "C" {
    void initGanttListener() {
      REGISTER_EXEC_LISTENER(GanttListener, "GanttListener");
    }
  }

}
