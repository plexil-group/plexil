#ifndef _H_SaveManager
#define _H_SaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"
#include "DataVector.hh"
#include "AdapterExecInterface.hh"
#include <map>
#include <string.h>

#include "pugixml.hpp"

using namespace PLEXIL;

using std::string;
using std::vector;
using std::map;



class SaveManager
{
public:
  
  virtual void setData( vector<boot_data>  *data, int *num_total_boots){
    m_data_vector = data;
    m_num_total_boots = num_total_boots;
  }
  virtual void setTimeFunction(Nullable<Real> (*time_func)()){
    m_time_func = time_func;
  }
  virtual void setExecInterface(AdapterExecInterface *execInterface){
    m_execInterface = execInterface;
  }

  // configXml is the "Config" node which is the child of the CheckpointAdapter
  // node. If no config is specified, this will be NULL
  virtual void setConfig(const pugi::xml_node* configXml) = 0;
  
  virtual void loadCrashes() = 0;
  
  virtual bool writeOut() = 0;
 

  // Called during each command (AFTER the relevant change to data_vector)
  // managers are expected to send COMMAND_SUCCESS when the provided command is written to disk
  virtual void setOK(bool b,Integer boot_num,Command *cmd) = 0;
  virtual void setCheckpoint(const string& checkpoint_name, bool value,string& info, Nullable<Real> time, Command *cmd) = 0;

protected:
  
  // Data structure that tracks boot-specific metadata and checkpoints
  vector<boot_data>  *m_data_vector;

  int32_t *m_num_total_boots;
  // Should be used ONLY to return COMMAND_SUCCESS for commands provided in setOK and setCheckpoint when they are written to disk
  AdapterExecInterface * m_execInterface;

  Nullable<Real>(*m_time_func)();
  
};
#endif
