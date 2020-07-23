#ifndef _H_SimpleSaveManager
#define _H_SimpleSaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"
#include "SaveManager.hh"
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




class SimpleSaveManager : public SaveManager
{
public:

  virtual void setData( vector<tuple<Nullable<Real>,Nullable<Real>,bool,
		map<const string,
		tuple<bool, 
		Nullable<Real>,string>>>>  *data, int *num_total_boots) override;

  virtual void setTimeFunction(Nullable<Real> (*time_func)()) override;

  virtual void setDirectory(const string& file_directory) override;

  virtual void loadCrashes() override;
  
  virtual bool writeOut() override;

   // Called during each command, managers are expected to send COMMAND_SUCCESS when a command is written to disk
  virtual void setOK(bool b,Integer boot_num,Command *cmd) override;
  virtual void setCheckpoint(const string& checkpoint_name, bool value,string& info, Nullable<Real> time, Command *cmd) override;

private:
  bool writeToFile(const string& location);
  void succeedCommands();
  tuple<long,long> findOldestNewestFiles();
  bool have_read = false;
  bool directory_set = false;
  bool write_enqueued;
  mutex data_lock;
  vector<Command*> queued_commands;
};
#endif
