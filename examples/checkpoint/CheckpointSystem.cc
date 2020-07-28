/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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
#include "StateCacheEntry.hh"
#include "CachedValue.hh"
#include "Subscriber.hh"
#include "Debug.hh"
#include "AdapterConfiguration.hh" // For access to g_configuration

#include <iostream>



using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::map;

using namespace PLEXIL;
#define debug(msg) debugMsg("CheckpointSystem"," "<<msg)
#define RLOCK debug("Locking reader"); m_rw.begin_read()
#define RUNLOCK debug("Unlocking reader"); m_rw.end_read()
#define WLOCK debug("Locking writer") m_rw.begin_write();
#define WUNLOCK debug("Unlocking writer") m_rw.end_write();

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
  return boot_num >= 0 && boot_num < m_data_vector.size();
}

bool CheckpointSystem::valid_checkpoint(const string& checkpoint_name,Integer boot_num){
  map<const string, CheckpointData> checkpoints = m_data_vector.at(boot_num).checkpoints;
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


void CheckpointSystem::start(){
  time_adapter = g_configuration->getLookupInterface("time");
  m_manager->setTimeFunction(get_time);
  m_manager->setData(&m_data_vector,&m_num_total_boots);
  m_manager->loadCrashes();
}


void CheckpointSystem::setSaveConfiguration(const pugi::xml_node* configXml){
    m_manager->setConfig(configXml);
}

void CheckpointSystem::setExecInterface(AdapterExecInterface* execInterface){
  m_manager->setExecInterface(execInterface);
}

////////////////////////////////// Lookups /////////////////////////////////////
bool CheckpointSystem::didCrash(){
  RLOCK;
  bool retval;
  // If we don't have a previous boot
  if(m_num_total_boots==1) retval = false;
  else{
    Value lastCrash = getIsOK(1);
    if(lastCrash==Unknown) retval = false;
    else{
      bool inverse_retval;
      lastCrash.getValue(inverse_retval);
      retval = !inverse_retval;
    }
  }
  RUNLOCK;
  return retval;
}


Integer CheckpointSystem::numAccessibleBoots(){
  RLOCK;
  Integer retval;
  retval = m_data_vector.size();
  RUNLOCK;
  return retval;
}


Integer CheckpointSystem::numTotalBoots(){
  RLOCK;
  Integer retval;
  retval = m_num_total_boots;
  RUNLOCK;
  return retval;
}

Integer CheckpointSystem::numUnhandledBoots(){
  RLOCK;
  Integer retval = 0;
  for(int i=0;i<m_data_vector.size();i++){
    BootData boot = m_data_vector.at(i);
    if(!boot.is_ok) retval++;
  }
  RUNLOCK;
  return retval;
}


Value CheckpointSystem::getCheckpointState(const string& checkpoint_name,Integer boot_num){
  RLOCK;
  Value retval;
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      retval =  m_data_vector.at(boot_num).checkpoints.at(checkpoint_name).state;
    }
    else{
      retval = Unknown;
    }
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  RUNLOCK;
  return retval;
}

Value CheckpointSystem::getCheckpointTime(const string& checkpoint_name, Integer boot_num){
  RLOCK;
  Value retval;
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      Nullable<Real> time = m_data_vector.at(boot_num).checkpoints.at(checkpoint_name).time;
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
  RUNLOCK;
  return retval;
}

Value CheckpointSystem::getCheckpointInfo(const string& checkpoint_name, Integer boot_num){
  RLOCK;
  Value retval;
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      retval = m_data_vector.at(boot_num).checkpoints.at(checkpoint_name).info;
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
  RUNLOCK;
  return retval;
}

// Returns the latest boot number where the checkpoint was true, Unknown if none found
Value CheckpointSystem::getCheckpointLastPassed(const string& checkpoint_name){
  RLOCK;
  Value retval = Unknown;
  for (Integer i=0;i<m_data_vector.size();i++){
    map<const string, CheckpointData> checkpoints = m_data_vector.at(i).checkpoints;
    if(checkpoints.find(checkpoint_name)!=checkpoints.end() && checkpoints.at(checkpoint_name).state){
      retval = i;
      break;
    }
  }
  RUNLOCK;
  return retval;
}


Value CheckpointSystem::getTimeOfBoot(Integer boot_num){
  RLOCK;
  Value retval;
  if(valid_boot(boot_num)){
    Nullable<Real> time = m_data_vector.at(boot_num).boot_time;
    retval = time_to_Value(time);
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  RUNLOCK;
  return retval;
}


Value CheckpointSystem::getTimeOfCrash(Integer boot_num){
  RLOCK;
  Value retval;
  if(valid_boot(boot_num)){
    Nullable<Real> time = m_data_vector.at(boot_num).crash_time;
    retval = time_to_Value(time);
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  RUNLOCK;
  return retval;
}

Value CheckpointSystem::getIsOK(Integer boot_num){
  RLOCK;
  Value retval;
  if(valid_boot(boot_num)){
    retval = m_data_vector.at(boot_num).is_ok;
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << error << "invalid boot number: " << boot_num << endl;
    retval = Unknown;
  }
  RUNLOCK;
  return retval;
}
//////////////////////////////////////// Commands /////////////////////////////////////////////

void CheckpointSystem::setCheckpoint(const string& checkpoint_name, bool value,string& info,Command *cmd){
  WLOCK;  
  Nullable<Real> time = get_time();
  // This inserts the element if none exists, and overrides if it exists
  const CheckpointData checkpoint = {value,time,info};
  m_data_vector.at(0).checkpoints[checkpoint_name] = checkpoint;


  // Publish changes to overloaded Checkpoint lookups
  publish("Checkpoint",value,checkpoint_name);
  publish("Checkpoint",value,checkpoint_name,0); // Lookup for boot 0
  publish("CheckpointTime",time_to_Value(time),checkpoint_name);
  publish("CheckpointTime",time_to_Value(time),checkpoint_name,0);
  publish("CheckpointInfo",info,checkpoint_name);
  publish("CheckpointInfo",info,checkpoint_name,0);

  m_manager->setCheckpoint(checkpoint_name, value, info, time, cmd);
  
  WUNLOCK;
}


void CheckpointSystem::setOK(bool b, Integer boot_num, Command *cmd){
 WLOCK;
 Value retval;
 if(valid_boot(boot_num)){
   m_data_vector.at(boot_num).is_ok = b;
   debug("Setting is_ok at boot " << boot_num << " to " <<b);
   publish("Is_OK",b,boot_num);
   m_manager->setOK(b, boot_num, cmd);
 }
 else{
   cerr <<"CheckpointSystem:"<<" Invalid boot number: "<<boot_num<<endl;
   retval = Unknown;
 }
 WUNLOCK;
}

bool CheckpointSystem::flush(){
  WLOCK;
  bool retval;
  retval = m_manager->writeOut();
  WUNLOCK;
  return retval;
}
