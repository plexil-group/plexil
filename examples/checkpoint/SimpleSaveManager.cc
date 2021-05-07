#include "SimpleSaveManager.hh"
#include "Subscriber.hh"

// PLEXIL includes
#include "AdapterExecInterface.hh" // g_execInterface
#include "Debug.hh"
#include "pugixml.hpp"

// POSIX includes
#include <dirent.h>
#include <sys/stat.h> //mkdir

// C++ Standard Library includes
#include <algorithm>    // std::min std::max
#include <iostream>
#include <limits> //numeric_limits
#include <sstream> // in to_string

// C library includes 
#include <climits>
#include <cerrno>
#include <cstdio>
#include <cstdlib>     /* strtod */

using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::map;
using namespace PLEXIL;


#define debug(msg) debugMsg("SimpleSaveManager"," "<<msg)
#define debugLock(msg) debugMsg("SimpleSaveManager:lock"," "<<msg)
#define LOCK debugLock("Locking"); m_data_lock.lock()
#define UNLOCK debugLock("Unlocking"); m_data_lock.unlock()
/////////////////////// Helper functions //////////////////////////


template<typename T>
string to_string(const T& value)
{
    std::ostringstream oss;
    oss << std::fixed << value;
    return oss.str();
}

const string time_to_string(const Nullable<Real> &time){
  if(time.has_value()) return to_string(time.value());
  else return "";
}

const Nullable<Real> string_to_time(const char* time){
  if(strcmp(time,"")==0) return Nullable<Real>();
  Real time_d = std::strtod(time,NULL);
  return Nullable<Real>(time_d);
}

bool is_number(const string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}


int mkdir_p(const char *path)
{
    /* From  https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950*/
    const size_t len = strlen(path);
    char _path[PATH_MAX];
    char *p; 

    errno = 0;

    /* Copy string so its mutable */
    if (len > sizeof(_path)-1) {
        errno = ENAMETOOLONG;
        return -1; 
    }   
    strcpy(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/') {
            /* Temporarily truncate */
            *p = '\0';

            if (mkdir(_path, S_IRWXU) != 0) {
                if (errno != EEXIST)
                    return -1; 
            }

            *p = '/';
        }
    }   

    if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
            return -1; 
    }   

    return 0;
}

//////////////////////// Class Features ////////////////////////////


void  SimpleSaveManager::setData(vector<BootData> *data, int32_t *num_total_boots){
  LOCK;
  m_data_vector = data;
  m_num_total_boots = num_total_boots;
  UNLOCK;
}
void  SimpleSaveManager::useTime(bool use_time){
  LOCK;
  m_use_time = use_time;
  UNLOCK;
}

void SimpleSaveManager::setConfig(const pugi::xml_node* configXml){
  // Prevent writes from occuring concurrently with a directory change
  LOCK;
  m_directory_set = true;
  bool found_directory = false;
  if(configXml==NULL){
    cerr<<"SimpleSaveManager: No configuration specified, defaulting to directory = ./saves"<<endl;
    m_file_directory = "./saves";
  }
  else{
    for(pugi::xml_attribute attr = configXml->first_attribute();
	attr;
	attr = attr.next_attribute()){

      if((string) attr.name() == (string) "Directory"){
	m_file_directory = attr.value();
	found_directory = true;
      }

      if((string) attr.name() == (string) "RemoveOldSaves"){
	string remove_old_saves = attr.value();
	std::transform(remove_old_saves.begin(),
			remove_old_saves.end(),
			remove_old_saves.begin(), ::tolower);
	// Defaults to true
	if(remove_old_saves == "false") m_remove_old_saves = false;
	else m_remove_old_saves = true;
      }
    }
    if(!found_directory){
      // No directory attribute found
      cerr << "SimpleSaveManager: No \"Directory\" attribute found in configuration, defaulting to ./saves"<<endl;
      m_file_directory = "./saves";
    }
  }
  UNLOCK;
}

  

void SimpleSaveManager::setOK(bool b,Integer boot_num,Command *cmd){
  // Prevents us from enqueuing a command while write is writing
  LOCK;
  m_queued_commands.push_back(cmd);
  UNLOCK;
  writeOut();
}

void SimpleSaveManager::setCheckpoint(const string& checkpoint_name, bool value,string& info, Nullable<Real> time, Command *cmd){
  LOCK;
  m_queued_commands.push_back(cmd);
  UNLOCK;
  writeOut();
}

void SimpleSaveManager::succeedCommands(){
  debug("sending success to "<< m_queued_commands.size() << " command(s)");
  for(std::vector<Command*>::iterator it = m_queued_commands.begin(); it != m_queued_commands.end();it++)
  {
    if(*it != NULL){
      publishCommandSuccess(*it);
    }
  }
  m_queued_commands.clear();
}

void SimpleSaveManager::loadCrashes(){
  LOCK;
  if(m_have_read){
    cerr << "Aleady loaded crashes, this operation only supported once" <<endl;
    UNLOCK;
    return;
  }
  
  m_have_read = true;
  if(!m_directory_set){
    cerr << "SaveManager configuration never loaded, defaulting to directory = ./saves" <<endl;
    m_file_directory = "./saves";
  }

  m_data_vector->clear();
  // Include current boot with current time, no checkpoints
  Nullable<Real> time;
  if(m_use_time){
    // Use queryTime here because this is likely the first time we are reading the time
    time.set_value(g_execInterface->queryTime());
    if(time.value()==std::numeric_limits<double>::min()) time.nullify();
  }
  BootData boot_d = {time,
		     Nullable<Real>(),
		     false,
		     map<const string,CheckpointData>()};
  m_data_vector->push_back(boot_d);

  
  pair<long,long> oldest_newest = findOldestNewestFiles();
  if(oldest_newest.second == -1){ // No files found
    debug("no backup found, proceeding assuming first bootup");
    *m_num_total_boots = 1;
  }
  
  else{ // Load rest of boots from XML
    // Necessary to assign before calling c_str in order to avoid the short temporary lifetime
    string cpp_file_name = (m_file_directory+"/"+to_string(oldest_newest.second)+"_save.xml");
    const char *file_name = cpp_file_name.c_str();
    
    debug("parsing "<<file_name);
    // Load previous boots
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(file_name);
    if(!result){ 
      debug("XML [" << file_name << "] parsed with errors\n "
	    <<"Error description: " << result.description());
    }
    else{
      // Read root attributes
      pugi::xml_node root = doc.child("SimpleSaveManager_Save");
      *m_num_total_boots = root.attribute("num_total_boots").as_int()+1;
      pugi::xml_node boot_node = root.first_child();

      // Iterate over boots
      int boot_n = 1;
      while(boot_node){
	Nullable<Real> time_of_boot = string_to_time(boot_node.attribute("time_of_boot").as_string());
	Nullable<Real> time_of_crash = string_to_time(boot_node.attribute("time_of_crash").as_string());
	bool is_ok = boot_node.attribute("is_ok").as_bool();
	// Read checkpoints
	map<const string,CheckpointData> checkpoints;
	pugi::xml_node checkpoint_node = boot_node.first_child();
	while(checkpoint_node){
	  bool state = checkpoint_node.attribute("state").as_bool();
	  Nullable<Real> time = string_to_time(checkpoint_node.attribute("time").as_string());
	  string info = checkpoint_node.attribute("info").as_string();
	  string name = checkpoint_node.name();
	  CheckpointData checkpoint = {
				 state,
				   time,
				 info};
	  checkpoints.insert(std::make_pair(
			       name,
			       checkpoint));
	  checkpoint_node = checkpoint_node.next_sibling();
	}

	BootData boot ={
	  time_of_boot,
	  time_of_crash,
	  is_ok,checkpoints};
	
	m_data_vector->push_back(boot);
	boot_n++;
	boot_node = boot_node.next_sibling();
      }
    }
  }
  UNLOCK;
}

bool SimpleSaveManager::writeOut(){

  LOCK;

  bool retval;  
  string save_name = "";
  pair<long,long> oldest_newest = findOldestNewestFiles();
  // No files found
  if(oldest_newest.second == -1){
    save_name = m_file_directory+"/1_save.xml";
  }
  else{
    save_name = m_file_directory+"/"+to_string(oldest_newest.second+1)+"_save.xml"; 
  }
  // If multiple valid files, delete the oldest (never delete the only remaining vaid file)
  if(oldest_newest.first != oldest_newest.second && m_remove_old_saves){
    string to_remove = m_file_directory+"/"+to_string(oldest_newest.first)+"_save.xml";
    remove(to_remove.c_str());
    debug("removing" <<to_remove);
  }

  debug("writing to "<<save_name);

  // The actual write has begun, any future writes could potentially feature new data
  retval = writeToFile(save_name);
  succeedCommands(); // Return COMMAND_SUCCESS to all commands
  UNLOCK;
  return retval;
}

bool SimpleSaveManager::writeToFile(const string& location){
  vector<BootData> data_clone = vector<BootData>(*m_data_vector);
  // Generate new XML document in memory
  pugi::xml_document doc;
  // Generate XML declaration
  pugi::xml_node declarationNode = doc.append_child(pugi::node_declaration);
  declarationNode.append_attribute("version") = "1.0";
  declarationNode.append_attribute("encoding") = "UTF-8";
  
  pugi::xml_node root = doc.append_child("SimpleSaveManager_Save");
  
  // Actually build XML
  
  // Root attributes
  root.append_attribute("num_total_boots").set_value(*m_num_total_boots);
  
  //Boots
  int boot_n = 0;
  for(vector<BootData>::iterator boot = data_clone.begin();
      boot != data_clone.end();
      boot++){
    pugi::xml_node curr_boot = root.append_child(("boot_"+to_string(boot_n)).c_str());
    curr_boot.append_attribute("time_of_boot").set_value(
      time_to_string(boot->boot_time).c_str());
    if(boot_n==0){
       Nullable<Real> time;
       if(m_use_time){
	 // Use currentTime not queryTime (which is guaranteed to be up-to-date)
	 // because the TimeAdapter may have quit by this point
	 time.set_value(g_execInterface->currentTime());
	 if(time.value()==std::numeric_limits<double>::min()) time.nullify();
       }
  
       curr_boot.append_attribute("time_of_crash").set_value(
	 time_to_string(time).c_str());
    }
    else{
      curr_boot.append_attribute("time_of_crash").set_value(
	time_to_string(boot->crash_time).c_str());
    }
    
    curr_boot.append_attribute("is_ok").set_value(boot->is_ok);
    // Checkpoints
    map<const string, CheckpointData>::iterator checkpoints;
    for ( checkpoints = boot->checkpoints.begin(); checkpoints != boot->checkpoints.end(); checkpoints++ )
    {
      string name = checkpoints->first;
      CheckpointData data = checkpoints->second;
      pugi::xml_node checkpoint = curr_boot.append_child(name.c_str());
      checkpoint.append_attribute("state").set_value(data.state);
      checkpoint.append_attribute("time").set_value(time_to_string(data.time).c_str());
      checkpoint.append_attribute("info").set_value(data.info.c_str());
    }
    boot_n++;
  }
  // Test if save directory exists
  DIR *test_open = opendir(m_file_directory.c_str());
  if(test_open==NULL){
    cerr << "SimpleSaveManager: Directory "<<m_file_directory<<" directory doesn't exist, creating" << endl;
    if(!mkdir_p(m_file_directory.c_str())){
      cerr << "SimpleSaveManager: Directory creation failed, exiting" <<endl;
      return false;
    }
  }
  
  closedir(test_open);
  // Save XML tree to temporary file
  bool saveSucceeded = doc.save_file((location+".part").c_str());
  // Rename file to indicate that it has been completely saved
  // If we crash during saving, we will just end up with an extraneous file,
  // once it's named correctly we know that it was saved correctly.
  if(saveSucceeded){
    rename((location+".part").c_str(),location.c_str());
  }
  else{
    cerr << "SimpleSaveManager: Saving to "<<location<<".part failed"<<endl;
    return false;
  }
  return true;
}

// Finds the oldest and newest file numbers
// Removes invalid files (those marked as incomplete)
pair<long,long> SimpleSaveManager::findOldestNewestFiles(){
  DIR *dr;
  struct dirent *en;
  long min = LONG_MAX;
  long max = -1;
  dr = opendir(m_file_directory.c_str()); //open directory
  if (dr) {
    while ((en = readdir(dr)) != NULL) {
      string current = en->d_name;
      if(current.size()>9 && current.substr(current.size()-9,9) == "_save.xml"){
	string save_number_s = current.substr(0,current.size()-9);
	if(is_number(save_number_s)){
	  long save_number;
	  std::istringstream(save_number_s) >> save_number; // Converts string to long
	  min = std::min(min,save_number);
	  max = std::max(max,save_number);
	  
	}
	else{
	  remove((m_file_directory+"/"+current).c_str());
	}
      }
    }
    closedir(dr); //close directory
  }
  else{
    cerr << "SimpleSaveManager: " << "Attempting to read from invalid directory: "<<m_file_directory<<endl;
  }
  return std::make_pair(min,max); // If none found, defaults to INT_MAX, -1
}


