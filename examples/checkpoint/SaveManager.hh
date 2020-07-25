#ifndef _H_SaveManager
#define _H_SaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"
#include "DataVector.hh"
#include "AdapterExecInterface.hh"
#include <map>
#include <string.h>
using namespace PLEXIL;

using std::string;
using std::vector;
using std::map;



class SaveManager
{
public:


  SaveManager() : m_file_directory("./") {}
  
  virtual void setData( vector<boot_data>  *data, int *num_total_boots){
    m_data_vector = data;
    m_num_total_boots = num_total_boots;
  }
  virtual void setTimeFunction(Nullable<Real> (*time_func)()){
    m_time_func = time_func;
  }
  virtual void setDirectory(const string& file_directory){
    m_file_directory = file_directory;
  }
  virtual void setExecInterface(AdapterExecInterface *execInterface){
    m_execInterface = execInterface;
  }
  
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

  string m_file_directory;
  
};
#endif
