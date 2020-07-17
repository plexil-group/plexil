/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "CheckpointSystem.hh"
#include "State.hh"
#include "CachedValue.hh"
#include "Subscriber.hh"
#include "AdapterConfiguration.hh" // For access to g_configuration

// Used for indexing into data structure
#define BOOT_TIME 0
#define CRASH_TIME 1
#define CHECKPOINTS 2

#define C_STATE 0
#define C_TIME 1
#define C_INFO 2

using std::cout;
using std::cerr;
using std::endl;
using std::get;
using std::string;

//TODO:remove
#define DEBUG cout<<"System "<<__LINE__<<endl;

///////////////////////////// Conveniences //////////////////////////////////

// A prettier name for the "unknown" value.
static Value Unknown;

// A preamble for error messages.
static string error = "Error in checkpoint system: ";

// A time adapter and StateCacheEntry for getting the time
static InterfaceAdapter* time_adapter = NULL;
static StateCacheEntry time_cache;

///////////////////////////// Helper Functions //////////////////////////////


// Check that the boot number is valid
bool CheckpointSystem::valid_boot(int32_t boot_num){
  return !(boot_num < 0 || boot_num > num_active_crashes);
}

bool CheckpointSystem::valid_checkpoint(const string& checkpoint_name,int32_t boot_num){
  map<const string, tuple<bool,Nullable<Real>,string>> checkpoints = get<CHECKPOINTS>(data_vector.at(boot_num));
  return checkpoints.find(checkpoint_name) != checkpoints.end();
}

static Nullable<Real> get_time(){
  if(time_adapter==NULL)  return Nullable<Real>();
  time_adapter->lookupNow(State("time",1), time_cache);
  if(!time_cache.isKnown()) return Nullable<Real>();

  // Extract time from time_cache
  Value time_value = time_cache.cachedValue()->toValue();
  Real r;
  time_value.getValue(r);
  return Nullable<Real>(r);
}


//////////////////////////////// Class Features ////////////////////////////////

CheckpointSystem::CheckpointSystem ()
{
  
}

CheckpointSystem::~CheckpointSystem ()
{
  if (m_system) {
    delete m_system;
  }
}


void CheckpointSystem::start(){
  time_adapter = g_configuration->getLookupInterface("time");
  SaveManager::getInstance()->loadCrashes(data_vector,get_time);
}


void CheckpointSystem::setDirectory(const string& file_directory){
    SaveManager::getInstance()->setDirectory(file_directory);
}

////////////////////////////////// Lookups /////////////////////////////////////
bool CheckpointSystem::didCrash(){
  return did_crash;
}


int32_t CheckpointSystem::numActiveCrashes(){
  int32_t retval;
  rw.begin_read();
  retval = num_active_crashes;
  rw.end_read();
  return retval;
}


int32_t CheckpointSystem::numTotalCrashes(){
  return num_total_crashes;
}


Value CheckpointSystem::getCheckpointState(const string& checkpoint_name,int32_t boot_num){
  rw.begin_read();
  Value retval;
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      retval =  get<C_STATE>(get<CHECKPOINTS>(data_vector.at(boot_num)).at(checkpoint_name));
    }
    // If checkpoint hasn't been registered as true, it hasn't been reached
    else{
      retval = false;
    }
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  rw.end_read();
  return retval;
}

Value CheckpointSystem::getCheckpointTime(const string& checkpoint_name, int32_t boot_num){
  rw.begin_read();
  Value retval;
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      Nullable<Real> time = get<C_TIME>(get<CHECKPOINTS>(data_vector.at(boot_num)).at(checkpoint_name));
      if(time.has_value()) retval = time.value();
      else retval = Unknown;
    }
    // If checkpoint doesn't exist, we can't get its time
    else{
       cerr << error << "invalid checkpoint name: " << checkpoint_name << endl;
       retval = Unknown;
    }
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  rw.end_read();
  return retval;
}

Value CheckpointSystem::getCheckpointInfo(const string& checkpoint_name, int32_t boot_num){
  rw.begin_read();
  Value retval;
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      retval = get<C_INFO>(get<CHECKPOINTS>(data_vector.at(boot_num)).at(checkpoint_name));
    }
    // If checkpoint doesn't exist, we can't get its time
    else{
       cerr << error << "invalid checkpoint name: " << checkpoint_name << endl;
       retval = Unknown;
    }
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  rw.end_read();
  return retval;
}

// Returns the latest boot number where the checkpoint was true, -1 if none found
Value CheckpointSystem::getCheckpointLastPassed(const string& checkpoint_name){
  rw.begin_read();
  Value retval = -1;
  for (int i=0;i<data_vector.size();i++){
    map<const string, tuple<bool, Nullable<Real>,string>> checkpoints = get<CHECKPOINTS>(data_vector[i]);
    if(checkpoints.find(checkpoint_name)!=checkpoints.end()){
      retval = i;
      break;
    }
  }
  rw.end_read();
  return retval;
}


Value CheckpointSystem::getTimeOfBoot(int32_t boot_num){
  rw.begin_read();
  Value retval;
  if(valid_boot(boot_num)){
    Nullable<Real> time = get<BOOT_TIME>(data_vector.at(boot_num));
    if(time.has_value()) retval = time.value();
    else retval =Unknown;
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  rw.end_read();
  return retval;
}


Value CheckpointSystem::getTimeOfCrash(int32_t boot_num){
  rw.begin_read();
  Value retval;
  if(valid_boot(boot_num)){
    Nullable<Real> time = get<CRASH_TIME>(data_vector.at(boot_num));
    if(time.has_value()) retval = time.value();
    else retval = Unknown;
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  rw.end_read();
  return retval;
}

Value CheckpointSystem::getSafeReboot(){
  return safe_to_reboot;
}
//////////////////////////////////////// Commands /////////////////////////////////////////////

Value CheckpointSystem::setCheckpoint(const string& checkpoint_name, bool value,string& info){
  rw.begin_write();
  map<const string,tuple<bool,Nullable<Real>,string>> checkpoints = get<CHECKPOINTS>(data_vector.at(0));
  Value retval;
  // If checkpoint not set, checkpoint was not reached
  if(checkpoints.find(checkpoint_name)==checkpoints.end()) retval = false;
  else retval = get<C_STATE>(checkpoints.at(checkpoint_name));
  
  // This inserts the element if none exists, and overrides if it exists
  checkpoints[checkpoint_name] = std::make_tuple(value,get_time(),info);
  rw.end_write();
  return retval;
}


Value CheckpointSystem::setSafeReboot(bool b){
  Value retval = safe_to_reboot;
  safe_to_reboot = b;
  return retval;
}


Value CheckpointSystem::deleteCrash(int32_t boot_num){
  rw.begin_write();
  Value retval;
  // Can't delete the current boot
  if(valid_boot(boot_num) && boot_num>0){
    // Deletes the boot_num'th element
    data_vector.erase(data_vector.begin()+boot_num);
    retval = true;
  }
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = false;
  }
  rw.end_write();
  return retval;
}
