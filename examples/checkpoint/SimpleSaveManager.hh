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
#include "pugixml.hpp"

class SimpleSaveManager : public SaveManager
{
public:
  
  SimpleSaveManager() : m_have_read(false), m_file_directory("./saves"), m_remove_old_saves(true) {}
  ~SimpleSaveManager(){
    // Pointers in m_queued_commands are managed elsewhere
  }
  
  virtual void setData( std::vector<BootData>  *data, int *num_total_boots);


  virtual void setConfig(const pugi::xml_node* configXml);

  virtual void useTime(bool use_time);

  virtual void loadCrashes();
  
  virtual bool writeOut();

  // Enqueues the command for the next write
  virtual void setOK(bool b,PLEXIL::Integer boot_num,PLEXIL::Command *cmd);
  virtual void setCheckpoint(const std::string& checkpoint_name, bool value, std::string& info, Nullable<PLEXIL::Real> time, PLEXIL::Command *cmd);

private:
  // Disallow copy
  SimpleSaveManager & operator=(const SimpleSaveManager&);
  SimpleSaveManager(const SimpleSaveManager&);
  
  bool writeToFile(const std::string& location);
  // Returns success to all commands which were committed to disk
  // This is the ONLY place m_execInterface is used
  void succeedCommands();
  std::pair<long,long> findOldestNewestFiles();

  
  
  bool m_have_read;
  std::string m_file_directory;
  
  bool m_directory_set;
  bool m_remove_old_saves;
  PLEXIL::ThreadMutex m_data_lock;
  std::vector<PLEXIL::Command*> m_queued_commands;
};
#endif
