
#ifndef _H_SaveManager
#define _H_SaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"
#include <map>
#include <mutex>
using namespace PLEXIL;

using std::string;
using std::vector;
using std::tuple;
using std::map;
using std::mutex;

// Datastructure that tracks boot metadata and checkpoint information
using checkpoint_data = vector<tuple<
				 Nullable<Real>,
				 Nullable<Real>,
				 map<const string, tuple<bool,Nullable<Real>,string>>>>;
class SaveManager
{
public:

  //Prohibits copying or assigning
  SaveManager (const SaveManager&) = delete;
  SaveManager& operator= (const SaveManager&) = delete;
  ~SaveManager();
  
  static SaveManager *getInstance () {
    if (!m_manager) {
      m_manager = new SaveManager;
    }
    return m_manager;
  }
 
 
  void loadCrashes(checkpoint_data &data_vector, Nullable<Real> (*time_func)());
  
  void setDirectory(const string& file_directory);

  void writeOut();

private:
  SaveManager();
  static SaveManager* m_manager;
  string file_directory = "./";
  bool have_read = false;
  bool directory_set = false;
  bool write_enqueued;
  mutex data_lock;
  Nullable<Real> (*time_func) ();
};
#endif
