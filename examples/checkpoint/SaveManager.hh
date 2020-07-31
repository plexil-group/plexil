#ifndef _H_SaveManager
#define _H_SaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"
#include "data_support.hh"
#include "AdapterExecInterface.hh"
#include <map>
#include <string.h>

#include "pugixml.hpp"

class SaveManager
{
public:
  virtual ~SaveManager(){
    // All pointer member variables here are managed by CheckpointSystem
  }

  SaveManager() : m_data_vector(NULL), m_num_total_boots(NULL){};

  SaveManager(const SaveManager &o) : m_data_vector(o.m_data_vector),
				      m_num_total_boots(o.m_num_total_boots),
				      m_execInterface(o.m_execInterface),
				      m_time_func(o.m_time_func){}

  SaveManager & operator=(const SaveManager &o){
    m_data_vector = o.m_data_vector;
    m_num_total_boots = o.m_num_total_boots;
    m_execInterface = o.m_execInterface;
    m_time_func = o.m_time_func;
    return *this;
  }
  
  
  virtual void setData( std::vector<BootData>  *data, int *num_total_boots){
    m_data_vector = data;
    m_num_total_boots = num_total_boots;
  }
  virtual void setTimeFunction(Nullable<PLEXIL::Real> (*time_func)()){
    m_time_func = time_func;
  }
  virtual void setExecInterface(PLEXIL::AdapterExecInterface *execInterface){
    m_execInterface = execInterface;
  }

  // configXml is the "Config" node which is the child of the CheckpointAdapter
  // node. If no config is specified, this will be NULL
  virtual void setConfig(const pugi::xml_node* configXml) = 0;
  
  virtual void loadCrashes() = 0;
  
  virtual bool writeOut() = 0;
 

  // Called during each command (AFTER the relevant change to data_vector)
  // managers are expected to send COMMAND_SUCCESS when the provided command is written to disk
  virtual void setOK(bool b,PLEXIL::Integer boot_num,
		     PLEXIL::Command *cmd) = 0;
  
  virtual void setCheckpoint(const std::string& checkpoint_name,
			     bool value,std::string& info,
			     Nullable<PLEXIL::Real> time,
			     PLEXIL::Command *cmd) = 0;

protected:
  
  // Data, shared with CheckpointSystem
  std::vector<BootData>  *m_data_vector;
  int32_t *m_num_total_boots;

  // Should be used ONLY to return COMMAND_SUCCESS for commands provided in setOK and setCheckpoint when they are written to disk
  PLEXIL::AdapterExecInterface * m_execInterface;

  Nullable<PLEXIL::Real>(*m_time_func)();
  
};
#endif
