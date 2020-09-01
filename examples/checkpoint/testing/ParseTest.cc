#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <string.h>
#include <sstream>      // std::istringstream

using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::cerr;

// Returns true if any string in v contains t
bool contains(const vector<string>& v, const string& t){
  for(int i=0;i<v.size();i++){
    if(v[i].find(t)!=string::npos) return true;
  }
  return false;
}

double getDouble(const string& s){
  double d;
  std::istringstream(s.substr(s.find("=")+1)) >> d;
  return d;
}

int getInt(const string& s){
  int i;
  std::istringstream(s.substr(s.find("=")+1)) >> i;
  return i;
}

string getString(const string& s){
  return s.substr(s.find("=")+1);
}

// For second run
#define NUMBERACCESSIBLE 5
#define NUMBERTOTAL 6
#define NUMBERUNHANDLED 7
#define CHECKPOINT1STATE 8
#define CHECKPOINT1INFO 9
#define OK1STATE 10
#define STARTTIME 11
#define ENDTIME 12

#define FSTARTTIME 5
#define FCHECKPOINT1STATE 8
#define FCHECKPOINT1INFO 9
#define FCHECKPOINT1HANDLE 12
#define FOK1STATE 16
#define FOK1HANDLE 20

int main(int argc, char *argv[]) {
  if(argc!=3){
    cout<<"incorrect number of arguments, expecting 2"<<endl;
    return 1;
  }
  // Split each argument into components
  vector<string> first_run;
  vector<string> second_run;
 
  char * token = strtok(argv[1], "|");
  while( token != NULL ) {
    first_run.push_back(token);
    token = strtok(NULL, "|");
  }
  token = strtok(argv[2], "|");
  while( token != NULL ) {
    second_run.push_back(token);
    token = strtok(NULL, "|");
  }

  // Verify second run finished
  if(!contains(second_run,"END---")){
    cout<<"Second run didn't terminate, likely due to error"<<endl;
    return 2;
  }
  
  // Verify that flushes returned successful codes 
  if(contains(first_run,"FLUSHSUCCESS=0")){
    cout<<"Flush in run 1 failed" << endl;
    return 2;
  }
  if(contains(second_run,"FLUSHSUCCESS=0")){
    cout<<"Flush in run 2 failed"<<endl;
    return 2;
  }

  // If flushed, check that at least one write persisted
  if(contains(first_run,"FLUSH1END")){
    if(getInt(second_run[NUMBERACCESSIBLE])!=2){
      cout<<"Run 1 flushed, expected 2 accessible boots, got "
	  <<getInt(second_run[NUMBERACCESSIBLE])<<endl;
      return 2;
    }
    if(getInt(second_run[NUMBERTOTAL])!=2){
      cout<<"Run 1 flushed, expected 2 total boots, got "
	  <<getInt(second_run[NUMBERTOTAL])<<endl;
      return 2;
    }
  }

  // Verify changes from is_ok have persisted if set (via unhandled boots)
  if(contains(first_run,"FLUSH3END")){
    if(getInt(second_run[NUMBERUNHANDLED])!=1){
      cout<<"Run 1 flushed is_ok, expected 1 uhandled boots, got "
	  <<getInt(second_run[NUMBERUNHANDLED])<<endl;
      return 2;
    }
  }
  // Verify is_ok not written if not set  (via unhandled boots)
  else if(!contains(first_run,"OK1BEGIN") && contains(first_run,"FLUSH1END")){
    if(getInt(second_run[NUMBERUNHANDLED])!=2){
      cout<<"Run 1 never set is_ok, expected 2 uhandled boots, got "
	  <<getInt(second_run[NUMBERUNHANDLED])<<endl;
      return 2;
    }
  }

  // Verify checkpoint correctly set and read
  if(contains(first_run,"CHECKPOINT1STATE")){
    // Compare returns 0 when identical
    if(getString(first_run[FCHECKPOINT1STATE]).compare("1")){
      cout<<"Checkpoint 1 was set but not read to same value"<<endl;
      return 2;
    }
  }
  if(contains(first_run,"CHECKPOINT1INFO")){
    if(getString(first_run[FCHECKPOINT1INFO]).compare("valid")){
      cout<<"Checkpoint 1 was set but not read to have the same information"<<endl;
      return 2;
    }
  }

  // If first run set checkpoint, verify that changes have persisted
  if(contains(first_run,"FLUSH2END")){
    if(getString(second_run[CHECKPOINT1STATE]).compare("1")){
      cout<<"Checkpoint 1 was set but change did not persist"<<endl;
      return 2;
    }
    if(getString(second_run[CHECKPOINT1INFO]).compare("valid")){
      cout<<"Checkpoint 1 was set but not read to have the same information"<<endl;
      return 2;
    }
  }
  // If first run didn't set checkpoint, verify that not written to disk
  if(!contains(first_run,"CHECKPOINT1BEGIN")){
    if(getString(second_run[CHECKPOINT1STATE]).compare("1")==0){
      cout<<"Checkpoint 1 not set, but is saved to disk"<<endl;
    }
    if(getString(second_run[CHECKPOINT1INFO]).compare("valid")==0){
      cout<<"Checkpoint 1 not set but its informaton is saved to disk"<<endl;
      return 2;
    }
  }


  // If is_ok set and read, check it was correct
  if(contains(first_run,"OK1STATE")){
    if(getString(first_run[FOK1STATE]).compare("1")){
      cout<<"Is_ok was set but was read to "<<getString(first_run[FOK1STATE])<<endl;
      return 2;
    }
  }
  // If first run set is_ok, verify that changes have persisted
  if(contains(first_run,"FLUSH3END")){
    if(getString(second_run[OK1STATE]).compare("1")){
      cout<<"Is_ok was set but change did not persist"<<endl;
      return 2;
    }
  }
  
  // If first run didn't set is_ok, check that it was not saved to disk
  if(!contains(first_run,"OK1BEGIN")){
    if(getString(second_run[OK1STATE]).compare("1")==0){
      cout<< "Is_ok not set, but set to true on disk"<<endl;
      return 2;
    }
  }

  // Verify that start time is correct and end time is sensible
  if(contains(first_run,"STARTTIME")){
    if(getString(first_run[FSTARTTIME])
       .compare(getString(second_run[STARTTIME]))){

      cout<<"Expected start time readings to be identical, got " <<
	getString(first_run[FSTARTTIME])<<" and " <<
	getString(second_run[STARTTIME])<<endl;
      return 2;
    }
    
    if(getDouble(first_run[FSTARTTIME])>getDouble(second_run[ENDTIME])){
      cout<<"Expected end time to be greater than start time, got "<<
	getString(first_run[FSTARTTIME])<<" and " <<
	getString(second_run[ENDTIME])<<endl;
      return 2;
    }
  }

  // Verify that Checkpoint handle was set to success
  if(contains(first_run,"CHECKPOINT1HANDLE")){
    if(getString(first_run[FCHECKPOINT1HANDLE])
       .compare("0")==0){
      cout<<"Expected checkpoint 1 handle to be COMMAND_SUCCESS after flush, was not"<<endl;
      return 2;
    }
  }

  // Verify that Set_OK handle was set to success
  if(contains(first_run,"OK1HANDLE")){
    if(getString(first_run[FOK1HANDLE])
       .compare("0")==0){
      cout<<"Expected set_ok 1 handle to be COMMAND_SUCCESS after flush, was not"<<endl;
      return 2;
    }
  }
  
  return 0;
}
