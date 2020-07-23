#include "SimpleSaveManager.hh"
#include <iostream>
#include <dirent.h>
#include <tuple>
#include <algorithm>    // std::min std::max
#include <stdio.h>
#include <stdlib.h>     /* atof */
#include <climits>
#include "Debug.hh"

#include "pugixml.hpp"
#include "plexil-stdint.h"

using std::mutex;
using std::cerr;
using std::endl;
using std::tuple;
using std::get;
using std::string;
using std::cout;


// Used for indexing into data structure
using checkpoint_data = tuple<bool,Nullable<Real>,string>;

using boot_data = tuple<
  Nullable<Real>,
  Nullable<Real>,
  bool,
  map<const string, checkpoint_data>>;



#define BOOT_TIME 0
#define CRASH_TIME 1
#define IS_OK 2
#define CHECKPOINTS 3

#define C_STATE 0
#define C_TIME 1
#define C_INFO 2

#define debug(msg) debugMsg("SimpleSaveManager"," "<<msg)
#define LOCK debug("Locking")
#define UNLOCK debug("Unlocking")
/////////////////////// Helper functions //////////////////////////

const string time_to_string(const Nullable<Real> &time){
  if(time.has_value()) return std::to_string(time.value());
  else return "";
}

const Nullable<Real> string_to_time(const char* time){
  if(strcmp(time,"")==0) return Nullable<Real>();
  else return Nullable<Real>(atof(time));
}

bool is_number(const string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

//////////////////////// Class Features ////////////////////////////


void  SimpleSaveManager::setData(vector<boot_data> *data, int32_t *num_total_boots){
  LOCK;
  data_lock.lock();
  m_data_vector = data;
  m_num_total_boots = num_total_boots;
  UNLOCK;
  data_lock.unlock();
}
void  SimpleSaveManager::setTimeFunction(Nullable<Real> (*time_func)()){
  m_time_func = time_func;
}

void SimpleSaveManager::setDirectory(const string& directory){
  // Prevent writes from occuring concurrently with a directory change
  LOCK;
  data_lock.lock();
  m_file_directory = directory;
  directory_set = true;
  debug("directory set to "<<directory);
  data_lock.unlock();
  UNLOCK;
}



// TODO: enforce only called once
void SimpleSaveManager::loadCrashes(){
  LOCK;
  data_lock.lock();
  if(!directory_set){
    debug("directory not specified, using default of ./");
  }
  have_read = true;

  m_data_vector->clear();
  // Include current boot with current time, no checkpoints
  m_data_vector->push_back(std::make_tuple(m_time_func(),Nullable<Real>(),false,map<const string,checkpoint_data>()));

  
  tuple<long,long> oldest_newest = findOldestNewestFiles();
  if(get<1>(oldest_newest)==-1){ // No files found
    debug("no backup found, proceeding assuming first bootup");
    *m_num_total_boots = 1;
  }
  
  else{ // Load rest of boots from XML
    // Necessary to assign before calling c_str in order to avoid the short temporary lifetime
    string cpp_file_name = (m_file_directory+"/"+std::to_string(get<1>(oldest_newest))+"_save.xml");
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
	map<const string,checkpoint_data> checkpoints;
	pugi::xml_node checkpoint_node = boot_node.first_child();
	while(checkpoint_node){
	  bool state = checkpoint_node.attribute("state").as_bool();
	  Nullable<Real> time = string_to_time(checkpoint_node.attribute("time").as_string());
	  string info = checkpoint_node.attribute("info").as_string();
	  string name = checkpoint_node.name();
	  checkpoints.insert(std::make_pair(name,checkpoint_data(state,time,info)));
	  checkpoint_node = checkpoint_node.next_sibling();
	}

	boot_data boot = boot_data(time_of_boot,time_of_crash,is_ok,checkpoints);
	m_data_vector->push_back(boot);
	boot_n++;
	boot_node = boot_node.next_sibling();
      }
    }
  }
  UNLOCK;
  data_lock.unlock();
}

void SimpleSaveManager::writeOut(){
  // If we were already planning to write out at the next opportunity, keep our course
  if(write_enqueued) return;
  // Log that we are planning to write out
  write_enqueued = true;
  LOCK;
  data_lock.lock();
  write_enqueued = false;
  
  string save_name = "";
  tuple<long,long> oldest_newest = findOldestNewestFiles();
  // No files found
  if(get<1>(oldest_newest) == -1){
    save_name = m_file_directory+"/1_save.xml";
  }
  else{
    save_name = m_file_directory+"/"+std::to_string(get<1>(oldest_newest)+1)+"_save.xml"; 
  }
  // If multiple valid files, delete the oldest (never delete the only remaining vaid file)
  if(get<0>(oldest_newest) != get<1>(oldest_newest)){
    string to_remove = m_file_directory+"/"+std::to_string(get<0>(oldest_newest))+"_save.xml";
    remove(to_remove.c_str());
    debug("removing" <<to_remove);
  }

  debug("writing to "<<save_name);
  writeToFile(save_name);
  debug("write out successful");
  UNLOCK;
  data_lock.unlock();
}

//TODO: verify crashes during saving are fine
void SimpleSaveManager::writeToFile(const string& location){
  vector<boot_data> data_clone = vector<boot_data>(*m_data_vector);
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
  for(boot_data boot : data_clone){
    pugi::xml_node curr_boot = root.append_child(("boot_"+std::to_string(boot_n)).c_str());
    curr_boot.append_attribute("time_of_boot").set_value(
      time_to_string(get<BOOT_TIME>(boot)).c_str());
    if(boot_n==0){
      curr_boot.append_attribute("time_of_crash").set_value(
	time_to_string(m_time_func()).c_str());
    }
    else{
      curr_boot.append_attribute("time_of_crash").set_value(
	time_to_string(get<CRASH_TIME>(boot)).c_str());
    }
    curr_boot.append_attribute("is_ok").set_value(get<IS_OK>(boot));
    // Checkpoints
    map<const string, checkpoint_data>::iterator checkpoints;
    for ( checkpoints = get<CHECKPOINTS>(boot).begin(); checkpoints != get<CHECKPOINTS>(boot).end(); checkpoints++ )
    {
      string name = checkpoints->first;
      checkpoint_data data = checkpoints->second;
      pugi::xml_node checkpoint = curr_boot.append_child(name.c_str());
      checkpoint.append_attribute("state").set_value(get<C_STATE>(data));
      checkpoint.append_attribute("time").set_value(time_to_string(get<C_TIME>(data)).c_str());
      checkpoint.append_attribute("info").set_value(get<C_INFO>(data).c_str());
    }
    boot_n++;
  }
  // Test if save directory exists
  DIR *test_open = opendir(m_file_directory.c_str());
  if(test_open==NULL){
    cerr << "SimpleSavemanager: Saving to "<<m_file_directory<<" failed, directory doesn't exist" << endl;
  }
  else{
    closedir(test_open);
    // Save XML tree to temporary file
    bool saveSucceeded = doc.save_file((location+".part").c_str());
    // Rename file to indicate that it has been completely saved
    if(saveSucceeded){
      rename((location+".part").c_str(),location.c_str());
    }
    else{
      cerr << "SimpleSaveManager: Saving to "<<location<<".part failed"<<endl;
    }
  }
}

// Finds the oldest and newest file numbers
// Removes invalid files (those marked as incomplete)
tuple<long,long> SimpleSaveManager::findOldestNewestFiles(){
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
	  long save_number = stoi(save_number_s);
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
  return std::make_tuple(min,max); // If none found, defaults to INT_MAX, -1
}


