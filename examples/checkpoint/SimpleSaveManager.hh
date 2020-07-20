#ifndef _H_SimpleSaveManager
#define _H_SimpleSaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"
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




class SimpleSaveManager
{
public:

  ~SimpleSaveManager();

  void setData( vector<tuple<Nullable<Real>,Nullable<Real>,
		map<const string,
		tuple<bool, 
		Nullable<Real>,string>>>>  *data,
		bool *safe_to_reboot, int *num_active_crashes, int *num_total_crashes, bool *did_crash);
  void setTimeFunction(Nullable<Real> (*time_func)());

  void setDirectory(const string& file_directory);

  void loadCrashes();
  
  void writeOut();

private:
  static SimpleSaveManager* m_manager;
  bool *m_safe_to_reboot;
  int *m_num_active_crashes;
  int *m_num_total_crashes;
  bool *m_did_crash;

  tuple<int,int> findOldestNewestFiles();
  void writeToFile(const string& location);
  
  // Data structure that tracks boot-specific metadata and checkpoints
  vector<tuple<Nullable<Real>, /*time of boot*/
	       Nullable<Real>, /*time of crash*/
	       map< /*map of checkpoint info*/
		 const string, /*checkpoint name*/
		 tuple<bool, /*state of checkpoint*/
		       Nullable<Real>,/*time of checkpoint activation*/
		       string>>>> /*user-defined checkpoint info*/
  *m_data_vector;

  Nullable<Real>(*m_time_func)();
  string file_directory = "./";
  bool have_read = false;
  bool directory_set = false;
  bool write_enqueued;
  mutex data_lock;
  Nullable<Real> (*time_func) ();
};
#endif
