#include "SaveManager.hh"
#include <iostream>
#include <mutex>

using std::mutex;
using std::cout;
using std::endl;

SaveManager *SaveManager::m_manager = 0; 

//////////////////////// Class Features ////////////////////////////

SaveManager::SaveManager (){}

SaveManager::~SaveManager ()
{
  if (m_manager) {
    delete m_manager;
  }
}

void SaveManager::loadCrashes(checkpoint_data &data, Nullable<Real>(*time)()){
  time_func = time;
  if(!directory_set){
    cout << "WARNING: SaveManager: directory not specified, using default of ./" << endl;
  }
  have_read = true;
}

void SaveManager::setDirectory(const string& directory){
  // Prevent writes from occuring concurrently with a directory change
  data_lock.lock();
  file_directory = directory;
  directory_set = true;
  data_lock.unlock();
}

void SaveManager::writeOut(){
  // If we were already planning to write out at the next opportunity, keep our course
  if(write_enqueued) return;
  data_lock.lock();
  // We are handling the write
  write_enqueued = false;
  
  data_lock.unlock();
}
