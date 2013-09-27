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

#ifndef _H_GanttListener
#define _H_GanttListener

#include "ExecDefs.hh"
#include "ExecListener.hh"

using std::string;
using std::map;
using std::vector;

namespace PLEXIL
{
   // Provides output from execution useful for debugging a Plexil plan.
   class GanttListener : public ExecListener
   {
   public:
      GanttListener();
      GanttListener(const pugi::xml_node& xml);
      virtual ~GanttListener();
      // Capture and report about useful node state transitions.
      void implementNotifyNodeTransition (NodeState prevState,
         const NodeId& node) const;
      // void implementNotifyAddPlan(const PlexilNodeId& plan, 
      //    const LabelStr& parent) const;
   private:
      struct NodeObj {
         double start;
         double end;
         double duration;
         string name;
         string type;
         string val;
         string parent;
         int id;
         string localvariables;
         string children;
         vector<string> localvarsvector;
         NodeObj (double start_val, 
                  double end_val,
                  double duration_val,
                  const string& id,  
                  const string& type_id, 
                  const string& val_str,
                  const string& parent_str, 
                  int id_val, 
                  const string& loc_var,
                  const string& child_str, 
                  vector<string>& loc_var_vec
                  )
                  :start(start_val),
                  end(end_val),
                  duration(duration_val), 
                  name(id),
                  type(type_id),
                  val(val_str),
                  parent(parent_str),
                  id(id_val),
                  localvariables(loc_var),
                  children(child_str),
                  localvarsvector(loc_var_vec)
                  { }
      };
      // Disallow copy, and assignment
      GanttListener(const GanttListener&);
      GanttListener& operator=(const GanttListener&);
      string m_uniqueFileName;
      string m_HTMLFilePath;
      bool m_outputFinalJSON;
      vector<NodeObj> m_nodes;
      double m_StartValdbl, m_EndValdbl, m_DurationValdbl;
      string m_fullTemplate;
      string m_Id, m_Type, m_Val;
      string m_LocalVarsAfter;
      int m_index;
      bool m_outputHTML;
      string m_plexilGanttDirectory;
      string m_currentWorkingDir;
      double m_startTime;
      map<NodeId, int> m_stateMap, m_counterMap;
      string m_parent;
      bool m_planFailureState;
      int m_nodeCounter;
      int m_actualId;
      void getGanttDir();
      void getCurrDir();
      void setUniqueFileName();
      void createHTMLFile(const string& r_name, const string& cur_dir, const string& gantt_dir);
      void deliverJSONAsFile(const string& r_name, const string& jstream, 
                             const string& cur_dir);
      void deliverPartialJSON(const string& r_name, const string& jstream, 
                              const string& cur_dir);
      NodeObj createNodeObj(const NodeId& nodeId, double& time, 
         int& nodeCounter, int& actualId, 
         map<NodeId, int>& stateMap, 
         map<NodeId, int>& counterMap, 
         string& myParent);
      void getFinalLocalVar(const vector<GanttListener::NodeObj>& nodes, 
                         const NodeId& nodeId, 
                         int index, string& myLocalVarsAfter);
      void processTempValsForNode(const vector<GanttListener::NodeObj>& nodes, 
         const NodeId& nodeId, int index, double time, 
         double& myEndValdbl,double& myDurationValdbl, 
         string& myParent, string& myLocalVarsAfter);
      void prepareDataForJSONObj(vector<GanttListener::NodeObj>& nodes, int index, 
         double& myEndValdbl, double& myDurationValdbl, 
         const string& myParent, const string& myLocalVarsAfter, 
         string& predicate, string& entity, string& nodeNameLower,
         string& nodeNameReg, string& newVal, 
         string& childrenVal, string& localVarsVal, 
         string& nodeIDString, string& startVal, string& endVal,
         string& durationVal);
      void generateTempOutputFiles(const string& rootName, const string& JSONStream, 
                                const string& currDir, 
                                const string& ganttDir);
      void generateFinalOutputFiles(const string& rootName, const string& JSONStream, 
                                 const string& nodeIDNum, const string& currDir, 
                                 const string& ganttDir, bool state);
      void processOutputData(vector<GanttListener::NodeObj>& nodes, const NodeId& nodeId, 
                    const string& curr_dir, const string& curr_plexil_dir,
                    double start_time, string& parent, bool state);
   };
}

extern "C" 
{
   void initGanttListener();
}


#endif // _H_GanttListener
