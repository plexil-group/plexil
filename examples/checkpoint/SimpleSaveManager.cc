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
  map<const string, checkpoint_data>>;



#define BOOT_TIME 0
#define CRASH_TIME 1
#define CHECKPOINTS 2

#define C_STATE 0
#define C_TIME 1
#define C_INFO 2

SimpleSaveManager *SimpleSaveManager::m_manager = 0; 
/////////////////////// Helper functions //////////////////////////

const char* time_to_string(const Nullable<Real> &time){
  if(time.has_value()) return std::to_string(time.value()).c_str();
  else return "";
}

const Nullable<Real> string_to_time(const char* time){
  if(strcmp(time,"")) return Nullable<Real>();
  else return Nullable<Real>(atof(time));
}

bool is_number(const string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

//////////////////////// Class Features ////////////////////////////

SimpleSaveManager::~SimpleSaveManager (){
  if (m_manager) {
    delete m_manager;
  }
}

void  SimpleSaveManager::setData(vector<boot_data> *data, bool *safe_to_reboot, int *num_active_crashes, int *num_total_crashes, bool *did_crash){
  m_data_vector = data;
  m_safe_to_reboot = safe_to_reboot;
  m_num_active_crashes = num_active_crashes;
  m_num_total_crashes = num_total_crashes;
  m_did_crash = did_crash;
  
}
void  SimpleSaveManager::setTimeFunction(Nullable<Real> (*time_func)()){
  m_time_func = time_func;
}


// TODO: enforce only called once
void SimpleSaveManager::loadCrashes(){
  
  if(!directory_set){
    cerr << "WARNING: SimpleSaveManager: directory not specified, using default of ./" << endl;
  }
  have_read = true;

  m_data_vector->clear();
  // Include current boot with current time, no checkpoints
  m_data_vector->push_back(std::make_tuple(m_time_func(),Nullable<Real>(),map<const string,checkpoint_data>()));

  
  tuple<int,int> oldest_newest = findOldestNewestFiles();
  if(get<1>(oldest_newest)==-1){ // No files found
    debugMsg("SimpleSaveManager, ", "No backup found, proceeding as if first bootup");
  }
  
  else{ // Load rest of boots from XML
    const char *file_name = (file_directory+std::to_string(get<1>(oldest_newest))+"_save.xml").c_str();
    // Load previous boots
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(file_name);
    if(!result){ 
      cerr << "XML [" << file_name << "] parsed with errors\n";
      cerr << "Error description: " << result.description() << endl;
    }
    else{
      // Read root attributes
      pugi::xml_node root = doc.child("SimpleSaveManager_Save");
      *m_did_crash = root.attribute("safe_to_reboot").as_bool();
      *m_num_active_crashes = root.attribute("num_active_crashes").as_int()+((*m_did_crash)?1:0);
      *m_num_total_crashes = root.attribute("num_total_crashes").as_int()+((*m_did_crash)?1:0);
      pugi::xml_node boot_node = root.first_child();
      // Skip if it wasn't a crash
      if(!(*m_did_crash)) boot_node = boot_node.next_sibling();
      // Iterate over boots
      int boot_n = 1; //TODO: Verify num_active_crashes matches actual number
      while(boot_node){
	Nullable<Real> time_of_boot = string_to_time(root.attribute("time_of_boot").value());
	Nullable<Real> time_of_crash = string_to_time(root.attribute("time_of_crash").value());
	// Read checkpoints
	map<const string,checkpoint_data> checkpoints;
	pugi::xml_node checkpoint_node = boot_node.first_child();
	while(checkpoint_node){
	  bool state = checkpoint_node.attribute("state").as_bool();
	  Nullable<Real> time = string_to_time(checkpoint_node.attribute("time").value());
	  string info = checkpoint_node.attribute("info").as_string();
	  string name = checkpoint_node.name();
	  checkpoints.insert(std::make_pair(name,checkpoint_data(state,time,info)));
	  checkpoint_node = checkpoint_node.next_sibling();
	}

	boot_data boot = boot_data(time_of_boot,time_of_crash,checkpoints);
	m_data_vector->push_back(boot);
	boot_n++;
	boot_node = boot_node.next_sibling();
      }
    }
  }
}

void SimpleSaveManager::setDirectory(const string& directory){
  // Prevent writes from occuring concurrently with a directory change
  data_lock.lock();
  file_directory = directory;
  directory_set = true;
  data_lock.unlock();
}

void SimpleSaveManager::writeOut(){
  // If we were already planning to write out at the next opportunity, keep our course
  if(write_enqueued) return;
  // Log that we are planning to write out
  write_enqueued = true;
  data_lock.lock();
  write_enqueued = false;
  
  string save_name = "";
  tuple<int,int> oldest_newest = findOldestNewestFiles();
  // No files found
  if(get<1>(oldest_newest) == -1){
    save_name = file_directory+"/1_save.xml";
  }
  else{
    save_name = file_directory+"/"+std::to_string(get<1>(oldest_newest)+1)+"_save.xml";
  }
  // If multiple valid files, delete the oldest (never delete the only remaining vaid file)
  if(get<0>(oldest_newest) != get<1>(oldest_newest)){
    remove((file_directory+"/"+std::to_string(get<0>(oldest_newest))+"_save.xml").c_str());
  }
  writeToFile(save_name);
  
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
  root.append_attribute("safe_to_reboot").set_value(*m_safe_to_reboot);
  root.append_attribute("num_active_crashes").set_value(*m_num_active_crashes);
  root.append_attribute("num_total_crashes").set_value(*m_num_total_crashes);
  
  //Boots
  int boot_n = 0;
  for(boot_data boot : data_clone){
    pugi::xml_node curr_boot = root.append_child(("boot_"+std::to_string(boot_n)).c_str());
    curr_boot.append_attribute("boot_num").set_value(boot_n);
    curr_boot.append_attribute("time_of_boot").set_value(time_to_string(get<BOOT_TIME>(boot)));
    if(boot_n==0){
      curr_boot.append_attribute("time_of_crash").set_value(time_to_string(m_time_func()));
    }
    else{
      curr_boot.append_attribute("time_of_crash").set_value(time_to_string(get<CRASH_TIME>(boot)));
    }
    // Checkpoints
    map<const string, checkpoint_data>::iterator checkpoints;
    for ( checkpoints = get<CHECKPOINTS>(boot).begin(); checkpoints != get<CHECKPOINTS>(boot).end(); checkpoints++ )
    {
      string name = checkpoints->first;
      checkpoint_data data = checkpoints->second;
      pugi::xml_node checkpoint = curr_boot.append_child(name.c_str());
      checkpoint.append_attribute("state").set_value(get<C_STATE>(data));
      checkpoint.append_attribute("time").set_value(time_to_string(get<C_TIME>(data)));
      checkpoint.append_attribute("info").set_value(get<C_INFO>(data).c_str());
    }
    boot_n++;
  }
  
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

// Finds the oldest and newest file numbers
// Removes invalid files (those marked as incomplete)
tuple<int,int> SimpleSaveManager::findOldestNewestFiles(){
  DIR *dr;
  struct dirent *en;
  int min = INT_MAX;
  int max = -1;
  dr = opendir(file_directory.c_str()); //open directory
  if (dr) {
    while ((en = readdir(dr)) != NULL) {
      string current = en->d_name;
      if(current.size()>9 && current.substr(current.size()-9,9) == "_save.xml"){
	string save_number_s = current.substr(0,current.size()-9);
	if(is_number(save_number_s)){
	  int save_number = stoi(save_number_s);
	  min = std::min(min,save_number);
	  max = std::max(max,save_number);
	  
	}
	else{
	  remove((file_directory+"/"+current).c_str());
	}
      }
    }
    closedir(dr); //close directory
  }
  else{
    cerr << "SimpleSaveManager: " << "Attempting to read from invalid directory: "<<file_directory<<endl;
  }
  return std::make_tuple(min,max); // If none found, defaults to INT_MAX, -1
}


