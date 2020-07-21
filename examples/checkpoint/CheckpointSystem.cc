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
#include "Debug.hh"
#include "AdapterConfiguration.hh" // For access to g_configuration

// Used for indexing into data structure
#define BOOT_TIME 0
#define CRASH_TIME 1
#define IS_OK 2
#define CHECKPOINTS 3

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
#define debug(msg) debugMsg("CheckpointSystem"," "<<msg)
#define RLOCK debug("Locking reader");
#define RUNLOCK debug("Unlocking reader");
#define WLOCK debug("Locking writer");
#define WUNLOCK debug("Unlocking writer");

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
bool CheckpointSystem::valid_boot(Integer boot_num){
  return !(boot_num < 0 || boot_num > num_active_crashes);
}

bool CheckpointSystem::valid_checkpoint(const string& checkpoint_name,Integer boot_num){
  map<const string, tuple<bool,Nullable<Real>,string>> checkpoints = get<CHECKPOINTS>(data_vector.at(boot_num));
  return checkpoints.find(checkpoint_name) != checkpoints.end();
}

static Nullable<Real> get_time(){
  if(time_adapter==NULL)  return Nullable<Real>();
  time_adapter->lookupNow(State::timeState(), time_cache);
  if(!time_cache.isKnown()) return Nullable<Real>();

  // Extract time from time_cache
  Value time_value = time_cache.cachedValue()->toValue();
  Real r;
  time_value.getValue(r);
  return Nullable<Real>(r);
}

Value time_to_Value(Nullable<Real> time){
  if(time.has_value()) return time.value();
  else return Unknown;
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
  manager.setTimeFunction(get_time);
  manager.setData(&data_vector,&num_active_crashes,&num_total_crashes);
  manager.loadCrashes();
}


void CheckpointSystem::setDirectory(const string& file_directory){
    manager.setDirectory(file_directory);
}

////////////////////////////////// Lookups /////////////////////////////////////
bool CheckpointSystem::didCrash(){
  RLOCK rw.begin_read();
  bool retval;
  if(num_total_crashes==0) retval = false;
  else{
    Value lastCrash = getIsOK(1);
    if(lastCrash==Unknown) retval = false;
    else{
      bool inverse_retval;
      lastCrash.getValue(inverse_retval);
      retval = !inverse_retval;
    }
  }
  RUNLOCK rw.end_read();
  return retval;
}


Integer CheckpointSystem::numActiveCrashes(){
  Integer retval;
  RLOCK rw.begin_read();
  retval = num_active_crashes;
  RUNLOCK rw.end_read();
  return retval;
}


Integer CheckpointSystem::numTotalCrashes(){
  Integer retval;
  RLOCK rw.begin_read();
  retval = num_total_crashes;
  RUNLOCK rw.end_read();
  return retval;
}


Value CheckpointSystem::getCheckpointState(const string& checkpoint_name,Integer boot_num){
  RLOCK rw.begin_read();
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
  RUNLOCK rw.end_read();
  return retval;
}

Value CheckpointSystem::getCheckpointTime(const string& checkpoint_name, Integer boot_num){
  RLOCK rw.begin_read();
  Value retval;
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      Nullable<Real> time = get<C_TIME>(get<CHECKPOINTS>(data_vector.at(boot_num)).at(checkpoint_name));
      retval = time_to_Value(time);
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
  RUNLOCK rw.end_read();
  return retval;
}

Value CheckpointSystem::getCheckpointInfo(const string& checkpoint_name, Integer boot_num){
  RLOCK rw.begin_read();
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
  RUNLOCK rw.end_read();
  return retval;
}

// Returns the latest boot number where the checkpoint was true, -1 if none found
Value CheckpointSystem::getCheckpointLastPassed(const string& checkpoint_name){
  RLOCK rw.begin_read();
  Value retval = -1;
  for (Integer i=0;i<data_vector.size();i++){
    map<const string, tuple<bool, Nullable<Real>,string>> checkpoints = get<CHECKPOINTS>(data_vector[i]);
    if(checkpoints.find(checkpoint_name)!=checkpoints.end()){
      retval = i;
      break;
    }
  }
  RUNLOCK rw.end_read();
  return retval;
}


Value CheckpointSystem::getTimeOfBoot(Integer boot_num){
  RLOCK rw.begin_read();
  Value retval;
  if(valid_boot(boot_num)){
    Nullable<Real> time = get<BOOT_TIME>(data_vector.at(boot_num));
    retval = time_to_Value(time);
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  RUNLOCK rw.end_read();
  return retval;
}


Value CheckpointSystem::getTimeOfCrash(Integer boot_num){
  RLOCK rw.begin_read();
  Value retval;
  if(valid_boot(boot_num)){
    Nullable<Real> time = get<CRASH_TIME>(data_vector.at(boot_num));
    retval = time_to_Value(time);
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  RUNLOCK rw.end_read();
  return retval;
}

Value CheckpointSystem::getIsOK(Integer boot_num){
  RLOCK rw.begin_read();
  Value retval;
  if(valid_boot(boot_num)){
    retval = get<IS_OK>(data_vector.at(boot_num));
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  RUNLOCK rw.end_read();
  return retval;
}
//////////////////////////////////////// Commands /////////////////////////////////////////////

Value CheckpointSystem::setCheckpoint(const string& checkpoint_name, bool value,string& info){
  WLOCK rw.begin_write();
  map<const string,tuple<bool,Nullable<Real>,string>> checkpoints = get<CHECKPOINTS>(data_vector.at(0));
  Value retval;
  // If checkpoint not set, checkpoint was not reached
  if(checkpoints.find(checkpoint_name)==checkpoints.end()) retval = false;
  else retval = get<C_STATE>(checkpoints.at(checkpoint_name));
  Nullable<Real> time = get_time();
  // This inserts the element if none exists, and overrides if it exists
  checkpoints[checkpoint_name] = std::make_tuple(value,time,info);
  manager.writeOut();
  // Publish changes to overloaded Checkpoint lookups
  publish("Checkpoint",value,checkpoint_name);
  publish("Checkpoint",value,checkpoint_name,0); // Lookup for boot 0
  publish("CheckpointTime",time_to_Value(time),checkpoint_name);
  publish("CheckpointTime",time_to_Value(time),checkpoint_name,0);
  publish("CheckpointInfo",info,checkpoint_name);
  publish("CheckpointInfo",info,checkpoint_name,0);
  WUNLOCK rw.end_write();
  return retval;
}


Value CheckpointSystem::setOK(Integer boot_num, bool b){
 WLOCK rw.begin_write();
 Value retval;
   if(valid_boot(boot_num)){
     retval = get<IS_OK>(data_vector.at(boot_num));
     get<IS_OK>(data_vector.at(boot_num)) = b;
     manager.writeOut();
     publish("Is_OK",b,boot_num);
   }
   else{
     cerr <<"CheckpointSystem:"<<" Invalid boot number: "<<std::to_string(boot_num)<<endl;
     retval = Unknown;
   }
  WUNLOCK rw.end_write();
  return retval;
}

bool CheckpointSystem::flush(){
  WLOCK rw.begin_write();
  manager.writeOut();
  WUNLOCK rw.end_write();
  return true; // TODO return false if error

}
