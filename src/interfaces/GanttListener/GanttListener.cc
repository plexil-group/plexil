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
* Updated by Cao, Yichuan // file ID (concurrency), auto call browser
*/

#include "GanttListener.hh"
#include "Node.hh"
#include "ExecListenerFactory.hh"
#include "AdapterFactory.hh"
#include "CoreExpressions.hh"
#include "AdapterExecInterface.hh"
#include "Expression.hh"
#include "Debug.hh"

#include <fstream>
#include <cmath>
#include <ctime>
#include <map>
#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
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
using std::endl;
using std::list;

namespace PLEXIL
{
   // For now, use the DebugMsg facilities (really intended for debugging the
   // *executive* and not plans) to display messages of interest.  Later, a more
   // structured approach including listener filters and a different user
   // interface may be in order.
   GanttListener::GanttListener() 
   { 
      getCurrDir();
      getGanttDir();
      setUniqueFileName();
      m_outputFinalJSON = true;
      m_outputHTML = true;
      m_planFailureState = false;
      m_startTime = -1;
      m_nodeCounter = 0;
      m_actualId = -1;
      m_first_time = true;
      m_fullTemplate = "var rawPlanTokensFromFile=\n[\n";
   }
   GanttListener::GanttListener(const pugi::xml_node& xml) : ExecListener(xml)
   { }
   GanttListener::~GanttListener() { }

   void GanttListener::getGanttDir()
   {
      /** get PLEXIL_HOME **/
      string pPath;
      try {
         pPath = getenv ("PLEXIL_HOME");
      }
      catch(int e) {
         debugMsg("GanttViewer:printErrors", "PLEXIL_HOME is not defined");
      }
      /** get Viewer directory under PLEXIL_HOME **/
      m_plexilGanttDirectory = pPath + "/viewers/gantt/";
   }

   void GanttListener::getCurrDir()
   {
      char * buffer;
      if (!(buffer = getcwd(NULL, FILENAME_MAX)))
      {
         cout << "getcwd error!" << endl;
      }
      else
      {
         m_currentWorkingDir = buffer;
         free(buffer);
      }
   }
   
   void GanttListener::setUniqueFileName()
   {
      m_uniqueFileName = "0";
   }

   static void launch(const string &url)
   {
      cout << "here!" << endl;
      string browser = "/usr/bin/firefox";//getenv("BROWSER");
      if (browser == "")
         return;
      char *args[3];
      args[0] = (char*)browser.c_str();
      args[1] = (char*)url.c_str();
      args[2] = 0;

      pid_t pid = fork();
      if (!pid)
         execvp(browser.c_str(), args);
   }

   /** generate the HTML file once a plan's execution started and 
   that connects to necessary Javascript and produced JSON **/
   void GanttListener::createHTMLFile(const string& rootName,  
                                      const string& currDir, 
                                      const string& ganttDir)
   {
      std::stringstream htmlFile;

      m_HTMLFilePath = currDir + "/" + 
         "gantt_" + m_uniqueFileName + "_" + rootName + ".html";
      const string myTokenFileName = "json/" + 
         m_uniqueFileName + "_" + rootName + ".js";

      string br = "\n "; // br, no need for the space after \n
      htmlFile << 
         "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 " <<
         "Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">" << br <<
         "<html lang=\"en\"> " << br <<
         "<head> " << br <<
         "<meta http-equiv=\"Content-Type\" " <<
         "content=\"text/html; charset=utf-8\"> " << br <<
         "<title>" << rootName << " - " << "Gantt Temporal Plan Viewer</title> " << 
         br << "<meta name=\"author\" content=\"By Madan, Isaac " <<
         "A. (ARC-TI); originally authored by " <<
         "Swanson, Keith J. (ARC-TI)\"> " << br << br <<
         "<!-- jQuery is required --> " << br <<
         "<script src=\"" << ganttDir << "jq/jquery-1.6.2.js\" " <<
         "type=\"text/javascript\"></script> " << br <<
         "<link type=\"text/css\" href=\"" << ganttDir << 
         "jq/jquery-ui-1.8.15.custom.css\" " <<
         "rel=\"Stylesheet\" /> " << br <<
         "<script type=\"text/javascript\" src=\"" << ganttDir <<
         "jq/jquery-ui-1.8.15.custom.min.js\"></script> " << br << br <<
         "<!-- Load data locally --> " << br <<
         "<script src=\"" << currDir << "/" << myTokenFileName << 
         "\" type=\"text/javascript\"></script> " << br << br <<
         "<!-- Application code --> " << br <<      
         "<script src=\"" << ganttDir <<
         "addons.js\" type=\"text/javascript\"></script> " << br <<
         "<script src=\"" << ganttDir <<
         "getAndConvertTokens.js\" type=\"text/javascript\"></script> " << br <<
         "<script src=\"" << ganttDir <<
         "showTokens.js\" type=\"text/javascript\"></script> " << br <<
         "<script src=\"" << ganttDir <<
         "detailsBox.js\" type=\"text/javascript\"></script> " << br <<
         "<script src=\"" << ganttDir <<
         "grid.js\" type=\"text/javascript\"></script> " << br <<
         "<script src=\"" << ganttDir <<
         "sizing.js\" type=\"text/javascript\"></script> " << br <<
         "<script src=\"" << ganttDir <<
         "main.js\" type=\"text/javascript\"></script> " << br <<
         "<script src=\"" << ganttDir <<
         "shortcuts.js\" type=\"text/javascript\"></script> " << br << br <<
         "<!-- My styles --> " << br <<
         "<link rel=\"stylesheet\" href=\"" << ganttDir << 
         "styles.css\" type=\"text/css\"> " << br <<
         "</head> \n <body> " << br << br <<
         "<!-- Layout --> " << br <<
         "<div id=\"footer\"></div> " << br <<
         "<div id=\"mod\"></div> " << br <<
         "<div id=\"gantt\"></div> " << br <<
         "</body> " << br <<
         "</html>"; 
      ofstream myfile;
      myfile.open(m_HTMLFilePath.c_str());
      if (myfile.fail())
      {
         cout << "Failed to create HTML file!" << endl;
         exit(1);
      }
      else
      {
         myfile << htmlFile.rdbuf() << std::flush;
         myfile.close();
      }
      
      m_HTMLFilePathForJSON = "\n \n var myHTMLFilePathString =\"" + m_HTMLFilePath + "\";";
      debugMsg("GanttViewer:printProgress", "HTML file written to "+ m_HTMLFilePath);
   }

   /** generate the JSON tokens file at the end of a plan's execution
   so that it can be parsed by Javascript in the Viewer **/
   void GanttListener::deliverJSONAsFile(const string& rootName, const string& JSONStream, 
                                         const string& curr_dir)
   {
      const string myCloser = "];";
      const string json_folder_path = curr_dir + "/" + "json";
      string outputFileName;
      if (m_outputFinalJSON)
      {
         if (access(json_folder_path.c_str(), 0) != 0)
         {
            mkdir(json_folder_path.c_str(), S_IRWXG | S_IRGRP | 
               S_IROTH | S_IRUSR | S_IRWXU);
         }
         ofstream myfile;
         outputFileName = curr_dir + "/" +
            "json/" + m_uniqueFileName + "_" + rootName + ".js";
         myfile.open(outputFileName.c_str());
         if (myfile.fail())
         {
            cout << "Failed to create JSON file!" << endl;
            exit(2);
         }
         else
         {
            myfile << JSONStream << myCloser << m_HTMLFilePathForJSON;
            myfile.close();
            m_outputFinalJSON = false;
         }
      }
      debugMsg("GanttViewer:printProgress", 
         "JSON tokens file written to "+ outputFileName);
   }

   /** generate the JSON tokens file during a plan's execution
   so that it can be parsed by Javascript in the Viewer **/
   void GanttListener::deliverPartialJSON(const string& rootName, const string& JSONStream, 
                                          const string& curr_dir) 
   {
      const string myCloser = "];";
      const string json_folder_path = curr_dir + "/" + "json";
      
      if (access(json_folder_path.c_str(), 0) != 0)
      {
         mkdir(json_folder_path.c_str(), S_IRWXG | S_IRGRP | 
            S_IROTH | S_IRUSR | S_IRWXU);
      }
      ofstream myfile;
      string outputFileName = curr_dir + "/" +
         "json/" + m_uniqueFileName + "_" + rootName + ".js";
      myfile.open(outputFileName.c_str());
      if (myfile.fail())
      {
         cout << "Failed to create partial JSON file!" << endl;
         exit(1);
      }
      else
      {
         myfile << JSONStream << myCloser << m_HTMLFilePathForJSON;
         myfile.close();
      }
      debugMsg("GanttViewer:printProgress", 
         "JSON tokens file written to "+ outputFileName);
   }

   static string getLocalVarInExecStateFromMap(const NodeId& nodeId, 
                                               vector<string>& myLocalVariableMapValues)
   {
      std::stringstream myLocalVars;
      const VariableMap tempLocalVariablesMap = nodeId->getLocalVariablesByName();
      if (tempLocalVariablesMap.empty())
      {
         return "none";
      }
      for (VariableMap::const_iterator it = tempLocalVariablesMap.begin();
         it != tempLocalVariablesMap.end(); ++it) 
      {
         const string& tempNameString = it->first.toString();
         ExpressionId temp = it->second;
         string tempValueString = temp->valueString();
         std::stringstream tempString;
         tempString << "<br><i>" << tempNameString << "</i>" 
            << " = " << tempValueString;
         myLocalVariableMapValues.push_back(tempValueString);
         //filter out local variables that are 'state' key  or 'UNKNOWN' value
         if (tempNameString != "state" && tempValueString != "UNKNOWN")
            myLocalVars << tempString << ", ";
      }
      return myLocalVars.str();
   }

   static string getChildNode(const NodeId& nodeId)
   {
      std::stringstream myChildNode;
      //get child nodes
      const vector<NodeId>& tempChildList = nodeId->getChildren();
      if (tempChildList.size() == 0) 
      {
         return "none";
      }
      else
      {
         for (vector<NodeId>::const_iterator i = tempChildList.begin(); 
            i != tempChildList.end(); i++) 
         {
            string tempString = ((NodeId) *i)->getNodeId().toString();
            myChildNode << tempString << ", ";
         }
      }
      return myChildNode.str();
   }

   GanttListener::NodeObj GanttListener::createNodeObj(const NodeId& nodeId, double& time, 
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
      myStartValdbl = ((nodeId->getCurrentStateStartTime()) - time) * 100;
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

      return NodeObj(myStartValdbl, -1, -1, myId, myType, myVal, 
         myParent, actualId, myChildren, myLocalVars, myLocalVariableMapValues);
   }

   static string boldenFinalString(vector<string>& prevLocalVarsVector, 
                                   vector<string>& thisLocalVarsVectorValues,
                                   vector<string>& thisLocalVarsVectorKeys,
                                   int i)
   {
      std::stringstream tempFullString;
      //bolden final local variable values that changed during execution of node
      if(prevLocalVarsVector[i] != thisLocalVarsVectorValues[i]) 
      {
         tempFullString << "<i>" <<  
            thisLocalVarsVectorKeys[i] << "</i>" << " = " << 
            prevLocalVarsVector[i] << " --><strong><font color=\"blue\"> " << 
            thisLocalVarsVectorValues[i] << "</strong></font>";
      }
      else 
      {
         tempFullString << "<i>" <<  thisLocalVarsVectorKeys[i] << 
            "</i>" << " = " << prevLocalVarsVector[i] << " --> " <<
            thisLocalVarsVectorValues[i];
      }
      return tempFullString.str(); 
   }

   static void processLocalVar(vector<string>& prevLocalVarsVector, 
                               vector<string>& thisLocalVarsVectorValues, 
                               vector<string>& thisLocalVarsVectorKeys,
                               string& myLocalVarsAfter)
   {
      //make sure all local variable vectors are filled
      int smallerSize;
      vector<string> fullStrings;
      std::stringstream ss;

      if(prevLocalVarsVector.size() > 1 && 
         thisLocalVarsVectorKeys.size() > 1 && 
         thisLocalVarsVectorValues.size() > 1) 
      {
         if(prevLocalVarsVector.size() < thisLocalVarsVectorKeys.size())
         {  
            smallerSize = prevLocalVarsVector.size();
         }
         else
         { 
            smallerSize = thisLocalVarsVectorKeys.size();
         }
         for(int i = 0; i < smallerSize; i++) 
         {
            //filter out local variables that are UNKNOWN at 
            // beginning of execution and at end of execution
            if(prevLocalVarsVector[i] != "UNKNOWN" || 
               thisLocalVarsVectorValues[i] != "UNKNOWN")
            { 
               fullStrings.push_back(boldenFinalString(prevLocalVarsVector, 
                  thisLocalVarsVectorValues, thisLocalVarsVectorKeys, i)); // performance issue
            }
         }
         for(size_t i = 0; i < fullStrings.size(); i++)
         {
            ss << "<br>" << fullStrings[i] << ", ";
            myLocalVarsAfter = ss.str();
         }
      }
      else 
         myLocalVarsAfter = "none";
   }

   void GanttListener::getFinalLocalVar(const vector<GanttListener::NodeObj>& nodes, 
                                        const NodeId& nodeId, 
                                        int index, string& myLocalVarsAfter)
   {
      const VariableMap tempLocalVariableMapAfter = nodeId->getLocalVariablesByName();
      vector<string> prevLocalVarsVector = nodes[index].localvarsvector;
      vector<string> thisLocalVarsVectorKeys;
      vector<string> thisLocalVarsVectorValues;

      if(nodes[index].localvariables != "none" && 
         nodes[index].localvarsvector.size() > 0) 
      {
         if (tempLocalVariableMapAfter.empty())
            myLocalVarsAfter = "none";
         for (VariableMap::const_iterator it = tempLocalVariableMapAfter.begin(); 
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

   void GanttListener::processTempValsForNode(const vector<GanttListener::NodeObj>& nodes, 
                                              const NodeId& nodeId, int index, double time, 
                                              double& myEndValdbl,double& myDurationValdbl, 
                                              string& myParent, string& myLocalVarsAfter)
   {
      myEndValdbl = ((nodeId->getCurrentStateStartTime()) - time)*100;
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

   void GanttListener::prepareDataForJSONObj(vector<GanttListener::NodeObj>& nodes, 
                                             int index, 
                                             double& myEndValdbl, 
                                             double& myDurationValdbl, 
                                             const string& myParent, 
                                             const string& myLocalVarsAfter, 
                                             string& predicate, 
                                             string& entity, 
                                             string& nodeNameLower,
                                             string& nodeNameReg, 
                                             string& newVal, 
                                             string& childrenVal, 
                                             string& localVarsVal, 
                                             string& nodeIDString, 
                                             string& startVal, 
                                             string& endVal,
                                             string& durationVal)
   {
      //add temp values to node
      nodes[index].end = myEndValdbl;
      nodes[index].duration = myDurationValdbl;
      nodes[index].parent = myParent;
      nodes[index].localvariables = myLocalVarsAfter;

      //add node info into variables for JSON string
      predicate = nodes[index].name;
      entity = nodes[index].type;
      nodeNameLower = nodes[index].parent;
      nodeNameReg = nodes[index].parent;
      newVal = nodes[index].val;
      childrenVal = nodes[index].children;
      localVarsVal = nodes[index].localvariables;
   
      //get rid of extra comma and space at end
      if(childrenVal != "none") {
         childrenVal.erase(childrenVal.end()-2);
      }
      if(localVarsVal != "none") {
         localVarsVal.erase(localVarsVal.end()-2);
      }

      //convert node id number, start time, end time, and duration to strings
      std::ostringstream ndcntr;
      ndcntr << nodes[index].id;
      nodeIDString = ndcntr.str();

      std::ostringstream strs;
      strs << nodes[index].start;
      startVal = strs.str();

      std::ostringstream strs2;
      strs2 << nodes[index].end;
      endVal = strs2.str();

      std::ostringstream strs3;
      strs3 << nodes[index].duration;
      durationVal = strs3.str();
   }

   static string produceSingleJSONObj(const string& predicate, const string& entity, 
                                      const string& nodeNameLower, const string& nodeNameReg, 
                                      const string& newVal, const string& childrenVal, 
                                      const string& localVarsVal, const string& nodeIDString, 
                                      const string& startVal, const string& endVal,
                                      const string& durationVal)
   {
      /** Some notes
      * predicate is this node name (myId)
      * entity is this node type (myType)
      * nodeNameLower and nodeNameReg are parent node name (myParent)
      **/

      //add '[' and ']' before and after duration and start to add uncertainty to those values
      //setup JSON object to be added to array
      std::stringstream newTemplate;
      newTemplate << "{\n'id': " << nodeIDString << ",\n'type':'" 
         << predicate 
         << "',\n'parameters': [\n{\n'name': 'entityName',\n'type': 'STRING',\n'value':'"
         << entity <<
         "'\n},\n{\n'name': 'full type',\n'type': 'STRING',\n'value': '"
         << nodeNameLower 
         << "." << predicate
         << "'\n},\n{\n'name': 'state',\n'type': 'STRING',\n'value':"
         << " 'ACTIVE'\n},\n{\n'name': 'object',\n'value': 'OBJECT:"
         << nodeNameReg
         << "(6)'\n},\n{\n'name': 'duration',\n'type': 'INT',\n'value': '"
         << durationVal
         << "'\n},\n{\n'name': 'start',\n'type': 'INT',\n'value': '"
         << startVal
         << "'\n},\n{\n'name': 'end',\n'type': 'INT',\n'value': '"
         << endVal
         << "'\n},\n{\n'name': 'value',\n'type': 'INT',\n'value': '"
         << newVal
         << "'\n},\n{\n'name': 'children',\n'type': 'INT',\n'value': '"
         << childrenVal
         << "'\n},\n{\n'name': 'localvariables',\n'type': 'INT',\n'value': '"
         << localVarsVal <<"'\n}\n]\n},\n";
      return newTemplate.str();
   }

   void GanttListener::generateTempOutputFiles(const string& rootName, 
                                               const string& JSONStream, 
                                               const string& currDir, 
                                               const string& ganttDir)
   {
      if (m_outputHTML == true)
      {
         createHTMLFile(rootName, currDir, ganttDir);
         m_outputHTML = false;
      }
      deliverPartialJSON(rootName, JSONStream, currDir); 
      debugMsg("GanttViewer:printProgress", 
         "finished gathering data; JSON and HTML stored");
   }

   void GanttListener::generateFinalOutputFiles(const string& rootName, 
                                                const string& JSONStream, 
                                                const string& nodeIDNum, 
                                                const string& currDir, 
                                                const string& ganttDir, 
                                                bool state)
   {
      if(nodeIDNum == "1")
      { 
         if (m_outputHTML == true)
         {
            createHTMLFile(rootName, currDir, ganttDir);
            m_outputHTML = false;
         }
         deliverJSONAsFile(rootName, JSONStream, currDir);
         debugMsg("GanttViewer:printProgress", 
            "finished gathering data; JSON and HTML stored");
      }
      else
      {  
         if (state == false)
         {
            generateTempOutputFiles(rootName, JSONStream, currDir, 
               ganttDir);
         }
      }
   }

   void GanttListener::processOutputData(vector<GanttListener::NodeObj>& nodes, 
                                         const NodeId& nodeId, 
                                         const string& curr_dir, 
                                         const string& curr_plexil_dir,
                                         double start_time, string& parent, bool state)
   {
      string myPredicate, myEntity, myNodeNameLower, myNodeNameReg, myNewVal;
      string myChildrenVal, myLocalVarsVal, myNodeIDString, myStartVal, myEndVal;
      string myDurationVal;

      //make sure the temporary variables are cleaned out
      m_Id = " ";
      m_StartValdbl = -1;
      m_Type = " ";
      m_Val = " ";
      // find the node it corresponds to in nodes vector
      string tempId = nodeId->getNodeId().toString();
      string tempType = nodeId->getType().toString();
      string tempParent = "invalid_parent_id";
      string myRootNodeStr = nodes[0].name;
      if(nodeId->getParent().isId()) {
         tempParent = nodeId->getParent()->getNodeId().toString();
      }
      for(size_t i=0; i<nodes.size(); i++) 
      {   
         if(tempParent != "invalid_parent_id") 
         {
            if(tempId==nodes[i].name && tempType==nodes[i].type && 
               tempParent==nodes[i].parent) 
            {
               m_index = i;
            }
         }
         else 
         {
            if(tempId==nodes[i].name && tempType==nodes[i].type) 
            {
               m_index = i;
            }
         }
      }

      processTempValsForNode(nodes, nodeId, m_index, start_time, 
         m_EndValdbl, m_DurationValdbl, 
         parent, m_LocalVarsAfter); 
      // add temp values to node
      prepareDataForJSONObj(nodes, m_index, m_EndValdbl, 
         m_DurationValdbl, parent, m_LocalVarsAfter, 
         myPredicate, myEntity, myNodeNameLower, myNodeNameReg, 
         myNewVal, myChildrenVal, myLocalVarsVal, myNodeIDString, myStartVal, 
         myEndVal, myDurationVal);

      // add JSON object to existing array
      m_fullTemplate += produceSingleJSONObj(myPredicate, myEntity, 
         myNodeNameLower, myNodeNameReg, myNewVal, myChildrenVal, myLocalVarsVal, 
         myNodeIDString, myStartVal, myEndVal, myDurationVal);
      generateFinalOutputFiles(myRootNodeStr, m_fullTemplate, 
         myNodeIDString, curr_dir, curr_plexil_dir, state);

      debugMsg("GanttViewer:printProgress", "Token added for node " +
         myEntity + "." + myPredicate);
   }
   
   GanttListener myListener;

   /** executed when the plan is added 
   *  gets the current directory and environment variables, 
   *  sets the header of the template,
   *  sets the start time of the plan's execution
   *  for use in file name
   **/
   // void GanttListener::implementNotifyAddPlan(const PlexilNodeId& /* plan */, 
   //                                            const LabelStr& /* parent */) const 
   // {
   //    int start = 0;
   //    std::ostringstream uFileName;
   //    uFileName.precision(10);
   //    uFileName << start;
   //    setUniqueFileName(uFileName.str());
   //    //reset startTime; it will be set when first node executes
   //    debugMsg("GanttViewer:printProgress", 
   //       "GanttListener notified of plan; start time for filename set");
   // }

   /** executed when nodes transition state
   *  resets the start time so it can be used in temporal calculations,
   *  grabs info from nodes in executing state,
   *  grabs info from nodes in finished state,
   *  nodes info is stored in each node's NodeObj struct
   **/
   void GanttListener::implementNotifyNodeTransition(NodeState /* prevState */, 
                                                     const NodeId& nodeId) const
   {
      string workingDir, ganttDirectory;

      workingDir = myListener.m_currentWorkingDir;
      ganttDirectory = myListener.m_plexilGanttDirectory;
      
      std::stringstream ss; 
      ss << nodeId;
      if (myListener.m_first_time)
      {
         myListener.m_first_node_ID = ss.str();
         myListener.m_first_time = false;
      }
      //startTime is when first node executes
      if (myListener.m_startTime == -1) {
         myListener.m_startTime = nodeId->getCurrentStateStartTime();
      }
      myListener.m_parent = " ";

      //get state
      const NodeState& newState = nodeId->getState();
      if(newState == EXECUTING_STATE) {  
         //setup NodeObj and add to vector
         myListener.m_nodes.push_back(myListener.createNodeObj(nodeId, 
                                                               myListener.m_startTime, 
                                                               myListener.m_nodeCounter, 
                                                               myListener.m_actualId, 
                                                               myListener.m_stateMap, 
                                                               myListener.m_counterMap, 
                                                               myListener.m_parent)
         );
      }

      if (newState == FAILING_STATE || newState == FINISHED_STATE)
      {
         if (newState == FAILING_STATE)
         {
            myListener.m_planFailureState = true;
         }
         myListener.processOutputData(myListener.m_nodes, nodeId, workingDir, 
            ganttDirectory, myListener.m_startTime, myListener.m_parent, 
            myListener.m_planFailureState);
         if (ss.str() == myListener.m_first_node_ID 
            && newState == FINISHED_STATE)
         {
            launch(myListener.m_HTMLFilePath);
         }
      }
   }

   extern "C" {
      void initGanttListener() {
         REGISTER_EXEC_LISTENER(GanttListener, "GanttListener");
      }
   }
}
