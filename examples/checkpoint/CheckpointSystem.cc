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
#include "Guard.hh"
#include "SimpleSaveManager.hh"
#include "Subscriber.hh"
#include "AdapterExecInterface.hh" // g_execInterface
#include "Debug.hh"

#include <iostream>
#include <limits>



using std::string;
using std::vector;
using std::map;

using namespace PLEXIL;
#define debug(msg) debugMsg("CheckpointSystem"," "<<msg)

///////////////////////////// Conveniences //////////////////////////////////

// A prettier name for the "unknown" value.
static Value Unknown;

// A preamble for error messages.
static string error = "Error in checkpoint system: ";



///////////////////////////// Helper Functions //////////////////////////////


// Check that the boot number is valid
bool CheckpointSystem::valid_boot(Integer boot_num){
  return boot_num >= 0 && boot_num < m_data_vector.size();
}

bool CheckpointSystem::valid_checkpoint(const string& checkpoint_name,Integer boot_num){
  map<const string, CheckpointData> checkpoints = m_data_vector.at(boot_num).checkpoints;
  return checkpoints.find(checkpoint_name) != checkpoints.end();
}

Value time_to_Value(Nullable<Real> time){
  if(time.has_value()) return time.value();
  else return Unknown;
}
//////////////////////////////// Class Features ////////////////////////////////
CheckpointSystem::CheckpointSystem(): m_manager(new SimpleSaveManager), m_use_time(true){}


void CheckpointSystem::start(){
  m_manager->setData(&m_data_vector,&m_num_total_boots);
  m_manager->loadCrashes();
}

void CheckpointSystem::useTime(bool use_time){
  debug("Using time? "<<use_time);
  m_manager->useTime(use_time);
  m_use_time = use_time;
}

void CheckpointSystem::setSaveConfiguration(const pugi::xml_node* configXml){
  m_manager->setConfig(configXml);
}


////////////////////////////////// Lookups /////////////////////////////////////
bool CheckpointSystem::didCrash(){
  {
    Guard local_guard(m_rw,read);
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
    return retval;
  }
}


Integer CheckpointSystem::numAccessibleBoots(){
  {
    Guard local_guard(m_rw,read);
    Integer retval;
    retval = m_data_vector.size();
    return retval;
  }
}


Integer CheckpointSystem::numTotalBoots(){
  {
    Guard local_guard(m_rw,read);
    Integer retval;
    retval = m_num_total_boots;
    return retval;
  }
}
Integer CheckpointSystem::numUnhandledBoots(){
  {
    Guard local_guard(m_rw,read);
    Integer retval = 0;
    for(int i=0;i<m_data_vector.size();i++){
      BootData boot = m_data_vector.at(i);
      if(!boot.is_ok) retval++;
    }
    return retval;
  }
}


Value CheckpointSystem::getCheckpointState(const string& checkpoint_name,Integer boot_num){
  {
    Guard local_guard(m_rw,read);
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
      debug( error << "invalid boot number: " << boot_num);
      retval = Unknown;
    }
    return retval;
  }
}

Value CheckpointSystem::getCheckpointTime(const string& checkpoint_name, Integer boot_num){
  {
    Guard local_guard(m_rw,read);
    Value retval;
    if(valid_boot(boot_num)){
      if(valid_checkpoint(checkpoint_name, boot_num)){
	Nullable<Real> time = m_data_vector.at(boot_num).checkpoints.at(checkpoint_name).time;
	retval = time_to_Value(time);
      }
      // If checkpoint doesn't exist, we can't get its time
      else{
	debug(error << "invalid checkpoint name: " << checkpoint_name);
	retval = Unknown;
      }
    }
    // If boot doesn't exist, something's gone wrong
    else{
      debug(error << "invalid boot number: " << boot_num);
      retval = Unknown;
    }
    return retval;
  }
}

Value CheckpointSystem::getCheckpointInfo(const string& checkpoint_name, Integer boot_num){
  {
    Guard local_guard(m_rw,read);
    Value retval;
    if(valid_boot(boot_num)){
      if(valid_checkpoint(checkpoint_name, boot_num)){
	retval = m_data_vector.at(boot_num).checkpoints.at(checkpoint_name).info;
      }
      // If checkpoint doesn't exist, we can't get its info
      else{
	debug(error << "invalid checkpoint name: " << checkpoint_name);
	retval = Unknown;
      }
    }
    // If boot doesn't exist, something's gone wrong
    else{
      debug( error << "invalid boot number: " << boot_num);
      retval = Unknown;
    }
    return retval;
  }
}

// Returns the latest boot number where the checkpoint was true, Unknown if none found
Value CheckpointSystem::getCheckpointLastPassed(const string& checkpoint_name){
  {
    Guard local_guard(m_rw,read);
    Value retval = Unknown;
    for (Integer i=0;i<m_data_vector.size();i++){
      map<const string, CheckpointData> checkpoints = m_data_vector.at(i).checkpoints;
      if(checkpoints.find(checkpoint_name)!=checkpoints.end() && checkpoints.at(checkpoint_name).state){
	retval = i;
	break;
      }
    }
    return retval;
  }
}

Value CheckpointSystem::getTimeOfBoot(Integer boot_num){
  {
    Guard local_guard(m_rw,read);
    Value retval;
    if(valid_boot(boot_num)){
      Nullable<Real> time = m_data_vector.at(boot_num).boot_time;
      retval = time_to_Value(time);
    }
    // If boot doesn't exist, something's gone wrong
    else{
      debug(error << "invalid boot number: " << boot_num);
      retval = Unknown;
    }
    return retval;
  }
}

Value CheckpointSystem::getTimeOfCrash(Integer boot_num){
  {
    Guard local_guard(m_rw,read);
    Value retval;
    if(valid_boot(boot_num)){
      Nullable<Real> time = m_data_vector.at(boot_num).crash_time;
      retval = time_to_Value(time);
    }
    // If boot doesn't exist, something's gone wrong
    else{
      debug(error << "invalid boot number: " << boot_num);
      retval = Unknown;
    }
    return retval;
  }
}
Value CheckpointSystem::getIsOK(Integer boot_num){
  {
    Guard local_guard(m_rw,read);
    Value retval;
    if(valid_boot(boot_num)){
      retval = m_data_vector.at(boot_num).is_ok;
    }
    // If boot doesn't exist, something's gone wrong
    else{
      debug(error << "invalid boot number: " << boot_num);
      retval = Unknown;
    }
    return retval;
  }
}

//////////////////////////////////////// Commands /////////////////////////////////////////////

void CheckpointSystem::setCheckpoint(const string& checkpoint_name, bool value,string& info,Command *cmd){
  {
    Guard local_guard(m_rw,write);
    // queryTime returns 0 if no time adapter can be found
    Nullable<Real> time;
    if(m_use_time){
      time.set_value(g_execInterface->queryTime());
      if(time.value()==std::numeric_limits<double>::min()) time.nullify();
    }
  
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

    // Send back command_received
    publishCommandReceived(cmd);
    debug("Sent COMMAND_RCVD_BY_SYSTEM");
    m_manager->setCheckpoint(checkpoint_name, value, info, time, cmd);
  }  
}


void CheckpointSystem::setOK(bool b, Integer boot_num, Command *cmd){
  {
    Guard local_guard(m_rw,write);
    Value retval;
    if(valid_boot(boot_num)){
      m_data_vector.at(boot_num).is_ok = b;
      debug("Setting is_ok at boot " << boot_num << " to " <<b);
      publish("Is_OK",b,boot_num);
      publishCommandReceived(cmd);
      debug("Sent COMMAND_RCVD_BY_SYSTEM");
      m_manager->setOK(b, boot_num, cmd);
    }
    else{
      publishCommandReceived(cmd);
      debug(error<<" Invalid boot number: "<<boot_num);
      retval = Unknown;
    }
  }
}

bool CheckpointSystem::flush(){
  {
    Guard local_guard(m_rw,write);
    bool retval;
    retval = m_manager->writeOut();
    return retval;
  }
}
