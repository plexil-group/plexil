/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

extern "C" 
{
   #include <stdio.h>  // extern "C"
   #include <stdlib.h>
   #include <time.h>
}

#include <iomanip> // for setprecision
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <map>

#include "GanttListener.hh"
#include "Node.hh"
#include "Debug.hh"
#include "ExecDefs.hh"
#include "ExecListenerFactory.hh"
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
   // For now, use the DebugMsg facilities (really intended for debugging the
   // *executive* and not plans) to display messages of interest.  Later, a more
   // structured approach including listener filters and a different user
   // interface may be in order.

   //nodes
   struct NodeObj {
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
      NodeObj (const string& id, double start_val, double end_val,
               double duration_val, const string& type_id, const string& val_str,
               const string& parent_str, int id_val, const string& loc_var,
               const string& child_str, vector<string>& loc_var_vec)
              : name(id),
               start(start_val),
               end(end_val),
               duration(duration_val),
               type(type_id),
               val(val_str),
               parent(parent_str),
               id(id_val),
               localvariables(loc_var),
               children(child_str),
               localvarsvector(loc_var_vec)
               { }
   };

   static string uniqueFileName;

   /** get the current time for the file name
   * example formatting Aug22_2011_01.28.42PM 
   * deprecated method due to verbose file naming; file name currently contains system time
   * uncomment line in createHTMLFile to use
   **/
   string getTime() 
   {
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
   void getCurrentWorkingDirectory(string& myDirectory, string& plexilGanttDirectory) 
   {
      char * buffer;
      if (!(buffer = getcwd(NULL, FILENAME_MAX)))
         cout << "getcwd error!" << endl;
      else
      {
         myDirectory = buffer;
         free(buffer);
      }

      /** get PLEXIL_HOME **/
      string pPath;
      try {
         pPath = getenv ("PLEXIL_HOME");
      }
      catch(int e) {
         debugMsg("GanttViewer:printErrors", "PLEXIL_HOME is not defined");
      }

      /** get Viewer directory under PLEXIL_HOME **/
      plexilGanttDirectory = pPath + "/viewers/gantt/";
      debugMsg("GanttViewer:printProgress", "Current working directory set to " 
      << plexilGanttDirectory);
   }

   /** generate the HTML file at the end of a plan's execution 
   that connects to necessary Javascript and produced JSON **/
   string createHTMLFile(const string& nodeName, const string& myDirectory, 
                         const string& plexilGanttDirectory) 
   {
      string myHTMLFilePath;
      //uncomment the following line to set filename to the 
      // format gantt_MMDD_YYYY_hour.min.sec_nodeName.html
      //uniqueFileName = getTime();
      static const string htmlFileName = myDirectory + "/" + 
         "gantt_" + uniqueFileName + "_" + nodeName + ".html";
      static const string myTokenFileName = "json/" + 
         uniqueFileName + "_" + nodeName + ".js";
      string lineBreak = "\n ";
      string htmlFile = 
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 "
         "Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">"+ lineBreak +
         "<html lang=\"en\"> " + lineBreak +
         "<head> " + lineBreak +
         "<meta http-equiv=\"Content-Type\" "
         "content=\"text/html; charset=utf-8\"> " + lineBreak +
         "<title>Gantt Temporal Plan Viewer</title> " + lineBreak +
         "<meta name=\"author\" content=\"By Madan, Isaac "
         "A. (ARC-TI); originally authored by "
         "Swanson, Keith J. (ARC-TI)\"> " + lineBreak + lineBreak +
         "<!-- jQuery is required --> "+ lineBreak +
         "<script src=\"" + plexilGanttDirectory + "jq/jquery-1.6.2.js\" "
         "type=\"text/javascript\"></script> " + lineBreak +
         "<link type=\"text/css\" href=\"" + plexilGanttDirectory + 
         "jq/jquery-ui-1.8.15.custom.css\" "
         "rel=\"Stylesheet\" /> " + lineBreak +
         "<script type=\"text/javascript\" src=\"" + plexilGanttDirectory +
         "jq/jquery-ui-1.8.15.custom.min.js\"></script> " + lineBreak+lineBreak +
         "<!-- Load data locally --> " + lineBreak +
         "<script src=\"" + plexilGanttDirectory+myTokenFileName + 
         "\" type=\"text/javascript\"></script> " + lineBreak + lineBreak +
         "<!-- Application code --> " + lineBreak +      
         "<script src=\"" + plexilGanttDirectory + 
         "addons.js\" type=\"text/javascript\"></script> " + lineBreak +
         "<script src=\"" + plexilGanttDirectory + 
         "getAndConvertTokens.js\" type=\"text/javascript\"></script> " + lineBreak +
         "<script src=\"" + plexilGanttDirectory +
         "showTokens.js\" type=\"text/javascript\"></script> " + lineBreak +
         "<script src=\"" + plexilGanttDirectory +
         "detailsBox.js\" type=\"text/javascript\"></script> " + lineBreak +
         "<script src=\"" + plexilGanttDirectory +
         "grid.js\" type=\"text/javascript\"></script> " + lineBreak +
         "<script src=\""+plexilGanttDirectory+
         "sizing.js\" type=\"text/javascript\"></script> " + lineBreak +
         "<script src=\"" + plexilGanttDirectory +
         "main.js\" type=\"text/javascript\"></script> " + lineBreak +
         "<script src=\"" + plexilGanttDirectory +
         "shortcuts.js\" type=\"text/javascript\"></script> " + lineBreak + lineBreak +
         "<!-- My styles --> "+lineBreak+
         "<link rel=\"stylesheet\" href=\"" + plexilGanttDirectory + 
         "styles.css\" type=\"text/css\"> " + lineBreak +
         "</head> \n <body> " + lineBreak + lineBreak +
         "<!-- Layout --> " + lineBreak +
         "<div id=\"footer\"></div> " + lineBreak +
         "<div id=\"mod\"></div> " + lineBreak +
         "<div id=\"gantt\"></div> " + lineBreak +
         "</body> " + lineBreak +
         "</html>";

      ofstream myfile;
      myfile.open(htmlFileName.c_str());
      myfile << htmlFile;
      myfile.close();
      
      myHTMLFilePath = "\n \n var myHTMLFilePathString =\"" + htmlFileName + "\";";
      debugMsg("GanttViewer:printProgress", "HTML file written to "+htmlFileName);
      
      return myHTMLFilePath;
   }
     
   /** generate the JSON tokens file at the end of a plan's 
   execution so that it can be parsed by Javascript in the Viewer **/
   void deliverJSONAsFile(const string& fullTemplate, const string& nodeName, 
                          const string& myHTMLFilePath, const string& plexilGanttDirectory) 
   {
      const string myCloser = "];";
      ofstream myfile;
      static const string outputFileName = plexilGanttDirectory + 
         "json/" + uniqueFileName + "_" + nodeName + ".js";
      myfile.open(outputFileName.c_str());
      myfile << fullTemplate << myCloser << myHTMLFilePath;
      myfile.close();
      debugMsg("GanttViewer:printProgress", 
         "JSON tokens file written to "+ outputFileName);
   }

   string getLocalVarInExecStateFromMap(const NodeId& nodeId, 
                                        vector<string>& myLocalVariableMapValues)
   {
      string myLocalVars;
      vector<string> myLocalVariableMap;
      VariableMap tempLocalVariablesMap = nodeId->getLocalVariablesByName();
      if (tempLocalVariablesMap.empty())
         myLocalVars = "none";
      for (VariableMap::iterator it = tempLocalVariablesMap.begin(); 
         it != tempLocalVariablesMap.end(); ++it) 
      {
         const std::string& tempNameString = it->first.toString();
         ExpressionId temp = it->second;
         string tempValueString = temp->valueString();
         string tempString = "<br><i>" + tempNameString + "</i>" 
            + " = " + tempValueString;
         myLocalVariableMapValues.push_back(tempValueString);
         myLocalVariableMap.push_back(tempString);
         //filter out local variables that are 'state' key  or 'UNKNOWN' value
         if (tempNameString != "state" && tempValueString != "UNKNOWN")
            myLocalVars += tempString + ", ";
      }
      return myLocalVars;
   }

   string getChildNode(const NodeId& nodeId)
   {
      string myChildren;
      //get child nodes
      //   vector<string> myChildNodes; // really needed?
      const vector<NodeId>& tempChildList = nodeId->getChildren();
      if (tempChildList.size() == 0) 
         myChildren = "none";
      else
      {
         for (vector<NodeId>::const_iterator i = tempChildList.begin(); 
            i != tempChildList.end(); i++) 
         {
            string tempString = ((NodeId) *i)->getNodeId().toString();
            //myChildNodes.push_back(tempString);
            myChildren += tempString + ", ";
         }
      }
      return myChildren;
   }

   NodeObj createNodeObj(const NodeId& nodeId, double& startTime, 
                         int& nodeCounter, int& actualId, 
                         map<NodeId, int>& stateMap, 
                         map<NodeId, int>& counterMap, 
                         string& myParent)
   {
      vector<string> myLocalVariableMapValues;

      //make sure the temporary variables are cleaned out
      string myId;
      double myStartValdbl = -1;
      string myType;
      string myVal = " ";
      string myLocalVars = " ";
      string myChildren = " ";

      myId = nodeId->getNodeId().toString();
      myStartValdbl = ((nodeId->getCurrentStateStartTime()) - startTime) * 100;
      myType = nodeId->getType().toString();
      myVal = nodeId->getStateName().getStringValue();

      if (nodeId->getParent().isId())
        myParent = nodeId->getParent()->getNodeId().toString();
      if (myParent == " ") {
         myParent = nodeId->getNodeId().toString();
      }

      //increase nodeCounter for ID value
      nodeCounter += 1;
      actualId = nodeCounter; //actualId ensures that looping nodes 
                              //have the same ID for each token

      //determine if a node looping; assign prior 
      // ID for loops and a new one for non loops
      stateMap[nodeId] += 1;
      if(stateMap[nodeId] > 1) 
         actualId = counterMap[nodeId];
      else
         counterMap[nodeId] = actualId;

      //get local variables from map in state 'EXECUTING'
      myLocalVars = getLocalVarInExecStateFromMap(nodeId, myLocalVariableMapValues);
      myChildren = getChildNode(nodeId); //get child nodes

      //convert actualId to string
      std::ostringstream ndcntr;
      ndcntr << actualId;
      string myNumber = ndcntr.str();

      NodeObj temp(myId, myStartValdbl, -1, -1, myType, myVal, 
         myParent, actualId, myChildren, myLocalVars, myLocalVariableMapValues);

      return temp;
   }

   //find the node it corresponds to in nodes vector
   int findNode(const NodeId& nodeId, vector<NodeObj>& nodes)
   {
      string tempId = nodeId->getNodeId().toString();
      string tempType = nodeId->getType().toString();
      string tempParent = "invalid_parent_id";
      int index;

      if(nodeId->getParent().isId()) 
         tempParent = nodeId->getParent()->getNodeId().toString();
      for(size_t i = 0; i < nodes.size(); i++) 
      {
         if(tempParent != "invalid_parent_id") 
            if(tempId==nodes[i].name && 
               tempType==nodes[i].type && 
               tempParent==nodes[i].parent) 
                 index = i;
         else
            if(tempId==nodes[i].name && tempType==nodes[i].type)
               index = i;
      }
      return index;
   }

   string boldenFinalString(vector<string>& prevLocalVarsVector, 
                            vector<string>& thisLocalVarsVectorValues,
                            vector<string>& thisLocalVarsVectorKeys,
                            int i)
   {
      string tempFullString;
      //bolden final local variable values that changed during execution of node
      if(prevLocalVarsVector[i] != thisLocalVarsVectorValues[i]) 
      {
         tempFullString = "<i>" +  
            thisLocalVarsVectorKeys[i] + "</i>" + " = " + 
            prevLocalVarsVector[i] + " --><strong><font color=\"blue\"> " + 
            thisLocalVarsVectorValues[i] + "</strong></font>";
      }
      else 
      {
         tempFullString = "<i>" +  thisLocalVarsVectorKeys[i] + 
            "</i>" + " = " + prevLocalVarsVector[i] + " --> " + 
            thisLocalVarsVectorValues[i];
      }
      return tempFullString; 
   }

   void processLocalVar(vector<string>& prevLocalVarsVector, 
                        vector<string>& thisLocalVarsVectorValues, 
                        vector<string>& thisLocalVarsVectorKeys,
                        string& myLocalVarsAfter)
   {
      //make sure all local variable vectors are filled
      int smallerSize;
      vector<string> fullStrings;

      if(prevLocalVarsVector.size() > 1 && 
         thisLocalVarsVectorKeys.size() > 1 && 
         thisLocalVarsVectorValues.size() > 1) 
      {
         if(prevLocalVarsVector.size() < thisLocalVarsVectorKeys.size())  
            smallerSize = prevLocalVarsVector.size();
         else 
            smallerSize = thisLocalVarsVectorKeys.size();
         for(int i = 0; i < smallerSize; i++) 
         {
            //filter out local variables that are UNKNOWN at 
            // beginning of execution and at end of execution
            if(prevLocalVarsVector[i] != "UNKNOWN" || 
               thisLocalVarsVectorValues[i] != "UNKNOWN") 
               fullStrings.push_back(boldenFinalString(prevLocalVarsVector, 
                  thisLocalVarsVectorValues, thisLocalVarsVectorKeys, i));
         }
         for(size_t i = 0; i < fullStrings.size(); i++)
            myLocalVarsAfter += "<br>" + fullStrings[i] + ", ";
      }
      else 
         myLocalVarsAfter = "none";
   }

   void getFinalLocalVar(vector<NodeObj>& nodes, const NodeId& nodeId, 
                         int index, string& myLocalVarsAfter)
   {
      VariableMap tempLocalVariableMapAfter = nodeId->getLocalVariablesByName();
      vector<string> prevLocalVarsVector = nodes[index].localvarsvector;
      vector<string> thisLocalVarsVectorKeys;
      vector<string> thisLocalVarsVectorValues;

      if(nodes[index].localvariables != "none" && 
         nodes[index].localvarsvector.size() > 0) 
      {
         if (tempLocalVariableMapAfter.empty())
            myLocalVarsAfter = "none";
         for (VariableMap::iterator it = tempLocalVariableMapAfter.begin(); 
            it != tempLocalVariableMapAfter.end(); it++) 
         {
            ExpressionId temp = it->second;
            thisLocalVarsVectorKeys.push_back(it->first.toString());
            thisLocalVarsVectorValues.push_back(temp->valueString());
         }
         processLocalVar(prevLocalVarsVector, thisLocalVarsVectorValues, 
            thisLocalVarsVectorKeys, myLocalVarsAfter);
      }
      else 
         myLocalVarsAfter = "none";
   }

   void processTempValsForNode(vector<NodeObj>& nodes, const NodeId& nodeId, 
                               int index, double startTime, double& myEndValdbl,
                               double& myDurationValdbl, string& myParent, 
                               string& myLocalVarsAfter)
   {
      myEndValdbl = ((nodeId->getCurrentStateStartTime()) - startTime)*100;
      myDurationValdbl = myEndValdbl - nodes[index].start;
      //doesn't exist until node is finished     
      string myOutcome = nodeId->getOutcome().getStringValue();
      if (nodeId->getParent().isId()) {
         myParent = nodeId->getParent()->getNodeId().toString();
      }
      if(myParent == " ") {
         myParent = nodes[index].name;
      }
      //get final values for local variables
      getFinalLocalVar(nodes, nodeId, index, myLocalVarsAfter);
   }

   void prepareDataForJSONObj(vector<NodeObj>& nodes, int index, double& myEndValdbl,
                              double& myDurationValdbl, const string& myParent,
                              const string& myLocalVarsAfter, string& myPredicate,
                              string& myEntity, string& myNodeNameLower,
                              string& myNodeNameReg, string& myNewVal, 
                              string& myChildrenVal, string& myLocalVarsVal, 
                              string& myNumber, string& myStartVal, string& myEndVal,
                              string& myDurationVal)
   {
      //add temp values to node
      nodes[index].end = myEndValdbl;
      nodes[index].duration = myDurationValdbl;
      nodes[index].parent = myParent;
      nodes[index].localvariables = myLocalVarsAfter;

      //add node info into variables for JSON string
      myPredicate = nodes[index].name;
      myEntity = nodes[index].type;
      myNodeNameLower = nodes[index].parent;
      myNodeNameReg = nodes[index].parent;
      myNewVal = nodes[index].val;
      myChildrenVal = nodes[index].children;
      myLocalVarsVal = nodes[index].localvariables;
   
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
      myNumber = ndcntr.str();

      std::ostringstream strs;
      strs << nodes[index].start;
      myStartVal = strs.str();

      std::ostringstream strs2;
      strs2 << nodes[index].end;
      myEndVal = strs2.str();

      std::ostringstream strs3;
      strs3 << nodes[index].duration;
      myDurationVal = strs3.str();
   }

   string produceSingleJSONObj(const string& myPredicate, const string& myEntity, 
                               const string& myNodeNameLower, const string& myNodeNameReg, 
                               const string& myNewVal, const string& myChildrenVal, 
                               const string& myLocalVarsVal, const string& myNumber, 
                               const string& myStartVal, const string& myEndVal,
                               const string& myDurationVal)
   {
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
         "'\n},\n{\n'name': 'state',\n'type': 'STRING',\n'value':"
         " 'ACTIVE'\n},\n{\n'name': 'object',\n'value': 'OBJECT:"
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
      return newTemplate;
   }

   void generateTempOutputFiles(const string& myNodeNameLower, const string& fullTemplate, 
                                const string& myDirectory, const string& plexilGanttDirectory)
   {
      string myHTMLFilePath;
      myHTMLFilePath = createHTMLFile(myNodeNameLower, myDirectory, 
         plexilGanttDirectory);
      deliverJSONAsFile(fullTemplate, myNodeNameLower, 
         myHTMLFilePath, plexilGanttDirectory); 
      debugMsg("GanttViewer:printProgress", 
         "finished gathering data; JSON and HTML stored");
   }

   void generateFinalOutputFiles(const string& myNodeNameLower, const string& fullTemplate, 
                                 const string& nodeIDNum, const string& myDirectory, 
                                 const string& plexilGanttDirectory)
   {
      string myHTMLFilePath;
   
      if(nodeIDNum == "1") 
      { 
         myHTMLFilePath = createHTMLFile(myNodeNameLower, myDirectory, 
            plexilGanttDirectory);
         deliverJSONAsFile(fullTemplate, myNodeNameLower, 
            myHTMLFilePath, plexilGanttDirectory); 
         debugMsg("GanttViewer:printProgress", 
            "finished gathering data; JSON and HTML stored");
      }
   }   
   /** executed when the plan is added 
   *  gets the current directory and environment variables, 
   *  sets the header of the template,
   *  sets the start time of the plan's execution
   *  for use in file name
   **/
   void GanttListener::implementNotifyAddPlan(const PlexilNodeId& /* plan */, 
                                              const LabelStr& /* parent */) const 
   {
      // FIXME: Get time from someplace!
      int start = 0;
      std::ostringstream uFileName;
      uFileName.precision(10);
      uFileName << start;
      uniqueFileName = uFileName.str();
      //reset startTime; it will be set when first node executes
      debugMsg("GanttViewer:printProgress", 
         "GanttListener notified of plan; start time for filename set");
   }

   /** executed when nodes transition state
   *  resets the start time so it can be used in temporal calculations,
   *  grabs info from nodes in executing state,
   *  grabs info from nodes in finished state,
   *  nodes info is stored in each node's NodeObj struct
   **/
   void GanttListener::implementNotifyNodeTransition(NodeState /* prevState */, 
                                                     const NodeId& nodeId) const
   {
      string myDirectory, plexilGanttDirectory;
      string myPredicate, myEntity, myNodeNameLower, myNodeNameReg, myNewVal;
      string myChildrenVal, myLocalVarsVal, myNumber, myStartVal, myEndVal;
      string myDurationVal;
      static int index;
      static double startTime = -1;
      static vector<NodeObj> nodes;
      static string fullTemplate = "var rawPlanTokensFromFile=\n[\n";        
      static map<NodeId, int> stateMap;
      static map<NodeId, int> counterMap;
      static int nodeCounter = 0;
      static string rootName;

      //make sure the temporary variables are cleaned out
      double myEndValdbl, myDurationValdbl;
      string myParent = " ";
      string myLocalVarsAfter;
      int actualId = -1;

      getCurrentWorkingDirectory(myDirectory, plexilGanttDirectory);
      
      //startTime is when first node executes
      if(startTime == -1) {
         startTime = nodeId->getCurrentStateStartTime();
      }

      //get state
      const NodeState& newState = nodeId->getState();
      if(newState == EXECUTING_STATE) {  
         //setup NodeObj and add to vector
         nodes.push_back(createNodeObj(nodeId, startTime, 
            nodeCounter, actualId, stateMap, counterMap, myParent));
      }

      if(newState == FINISHED_STATE) {
         // find the node it corresponds to in nodes vector
         index = findNode(nodeId, nodes);
         processTempValsForNode(nodes, nodeId, index, startTime, myEndValdbl,
            myDurationValdbl, myParent, myLocalVarsAfter); 
         // add temp values to node
         prepareDataForJSONObj(nodes, index, myEndValdbl, myDurationValdbl, myParent,
            myLocalVarsAfter, myPredicate, myEntity, myNodeNameLower, myNodeNameReg, myNewVal,
            myChildrenVal, myLocalVarsVal, myNumber, myStartVal, myEndVal, myDurationVal);

         // add JSON object to existing array
         fullTemplate += produceSingleJSONObj(myPredicate, myEntity, myNodeNameLower,
            myNodeNameReg, myNewVal, myChildrenVal, myLocalVarsVal, myNumber, myStartVal, 
            myEndVal, myDurationVal);   
         // generate temporary HTML and JSON files if the plan is loop or stuck
      //   cout << nodes[0].name << endl;
         rootName = nodes[0].name;
         generateTempOutputFiles(rootName, fullTemplate, myDirectory,
            plexilGanttDirectory);
         // if it is the last token, create final HTML and add the tokens to the js file
         generateFinalOutputFiles(myNodeNameLower, fullTemplate, 
            myNumber, myDirectory, plexilGanttDirectory);
         debugMsg("GanttViewer:printProgress", 
            "Token added for node "+myEntity+"."+myPredicate);
      }
   }

   extern "C" {
      void initGanttListener() {
         REGISTER_EXEC_LISTENER(GanttListener, "GanttListener");
      }
   }
}
