#ifndef _H_SimpleSaveManager
#define _H_SimpleSaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"
#include "SaveManager.hh"
#include "ThreadMutex.hh"
#include <map>
#include <utility>      // std::pair, std::make_pair
#include <string.h>
using namespace PLEXIL;

using std::string;
using std::pair;
using std::vector;
using std::map;




class SimpleSaveManager : public SaveManager
{
public:

  SimpleSaveManager() : have_read(false), directory_set(false) {}
  
  virtual void setData( vector<boot_data>  *data, int *num_total_boots);

  virtual void setTimeFunction(Nullable<Real> (*time_func)());

  virtual void setDirectory(const string& file_directory);

  virtual void loadCrashes();
  
  virtual bool writeOut();

  // Enqueues the command for the next write
  virtual void setOK(bool b,Integer boot_num,Command *cmd);
  virtual void setCheckpoint(const string& checkpoint_name, bool value,string& info, Nullable<Real> time, Command *cmd);

private:
  bool writeToFile(const string& location);
  // Returns success to all commands which were committed to disk
  // This is the ONLY place m_execInterface is used
  void succeedCommands();
  pair<long,long> findOldestNewestFiles();
  
  bool have_read;
  bool directory_set;
  bool write_enqueued;
  ThreadMutex data_lock;
  vector<Command*> queued_commands;
};
#endif
