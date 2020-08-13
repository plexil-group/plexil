#ifndef _H_SaveManager
#define _H_SaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"
#include "Command.hh"
#include "data_support.hh"
#include <map>
#include <string.h>

#include "pugixml.hpp"

class SaveManager
{
public:
  virtual ~SaveManager(){
    // All pointer member variables here are managed by CheckpointSystem
  }

  SaveManager() : m_data_vector(NULL), m_num_total_boots(NULL), m_use_time(true) {};

  SaveManager(const SaveManager &o) : m_data_vector(o.m_data_vector),
				      m_num_total_boots(o.m_num_total_boots),
				      m_use_time(true){}

  SaveManager & operator=(const SaveManager &o){
    m_data_vector = o.m_data_vector;
    m_num_total_boots = o.m_num_total_boots;
    return *this;
  }
  
  
  virtual void setData( std::vector<BootData>  *data, int *num_total_boots){
    m_data_vector = data;
    m_num_total_boots = num_total_boots;
  }
  virtual void useTime(bool use_time){
    m_use_time = use_time;
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

  bool m_use_time;
  
};
#endif
