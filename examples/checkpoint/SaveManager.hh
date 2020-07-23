#ifndef _H_SaveManager
#define _H_SaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"
#include "AdapterExecInterface.hh"
#include <map>
#include <tuple>
#include <mutex>
#include <string.h>
using namespace PLEXIL;

using std::string;
using std::vector;
using std::tuple;
using std::map;
using std::mutex;




class SaveManager
{
public:

  virtual void setData( vector<tuple<Nullable<Real>,Nullable<Real>,bool,
		map<const string,
		tuple<bool, 
			Nullable<Real>,string>>>>  *data, int *num_total_boots){
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

  // Called during each command, managers are expected to send COMMAND_SUCCESS when a command is written to disk
  virtual void setOK(bool b,Integer boot_num,Command *cmd) = 0;
  virtual void setCheckpoint(const string& checkpoint_name, bool value,string& info, Nullable<Real> time, Command *cmd) = 0;

protected:
  
  // Data structure that tracks boot-specific metadata and checkpoints
  vector<tuple<Nullable<Real>, /*time of boot*/
	       Nullable<Real>, /*time of crash*/
	       bool, /*Is OK*/
	       map< /*map of checkpoint info*/
		 const string, /*checkpoint name*/
		 tuple<bool, /*state of checkpoint*/
		       Nullable<Real>,/*time of checkpoint activation*/
		       string>>>> /*user-defined checkpoint info*/
  *m_data_vector;

  int32_t *m_num_total_boots;
  AdapterExecInterface * m_execInterface;

  Nullable<Real>(*m_time_func)();

  string m_file_directory = "./";
  
};
#endif
