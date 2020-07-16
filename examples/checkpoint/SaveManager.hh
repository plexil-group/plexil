
#ifndef _H_SaveManager
#define _H_SaveManager

#include "Value.hh"
#include "Nullable.hh"
#include "ValueType.hh"

using namespace PLEXIL;

using std::string;
using std::vector;
using std::tuple;
using std::map;


class SaveManager
{
public:
  SaveManager();
  ~SaveManager();
  void loadCrashes(vector<tuple<
		   Nullable<Real>,
		   Nullable<Real>,
		   map<const string, tuple<bool,Nullable<Real>,string>>>> &data_vector,
		   
		   Nullable<Real> (*time_func)());//TODO: find way to make this not rely on CheckpointSystem?
  void setDirectory(const string& file_directory);

private:
  string file_directory = "./";
  bool have_read = false;
  bool directory_set = false;
  Nullable<Real> (*time_func) ();
};
#endif
