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


#include "Subscriber.hh"
#include "CheckpointSystem.hh"
// Used for indexing into data structure
#define BOOT_TIME 0
#define CRASH_TIME 1
#define CHECKPOINTS 2

#define C_STATE 0
#define C_TIME 1

using std::cerr;
using std::endl;
using std::get;

///////////////////////////// Conveniences //////////////////////////////////

// A prettier name for the "unknown" value.
static Value Unknown;

// A preamble for error messages.
static string error = "Error in checkpoint system: ";

///////////////////////////// Helper Functions //////////////////////////////



// Load all previous active crashes
bool SampleSystem::load_crashes(const string& directory){

  //Do some publishing
}

// Check that the boot number is valid
bool SampleSystem::valid_boot(int32_t boot_num){
  return (boot_num < 0 || boot_num > num_active_crashes);
}

bool SampleSystem::valid_checkpoint(const string checkpoint_name,int32_t boot_num){
  map<const string&, tuple<bool,int>> checkpoints = get<CHECKPOINTS>(data_vector.at(boot_num));
  return checkpoints.find(checkpoint) == checkpoints.end();
}

//////////////////////////////// Class Features ////////////////////////////////

SampleSystem::SampleSystem ()
{
  load_crashes("PLACEHOLDER");
}

SampleSystem::~SampleSystem ()
{
  if (m_system) {
    delete m_system;
  }
}

////////////////////////////////// Lookups /////////////////////////////////////
bool SampleSystem::didCrash(){
  return did_crash;
}


int32_t SampleSystem::numActiveCrashes(){
  return num_active_crashes;
}


int32_t SampleSystem::numTotalCrashes(){
  return num_total_crashes;
}


Value SampleSystem::getCheckpointState(const std::string checkpoint_name,int32_t boot_num){
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      return get<C_STATE>(get<CHECKPOINTS>(data_vector.at(boot_num)).at(checkpoint_name));
    }
    // If checkpoint hasn't been registered as true, it hasn't been reached
    else{
      return false;
    }
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << errror << "invalid boot number: " << boot_num << endl;
    return Unknown;
  }
}

V
alue SampleSystem::getCheckpointTime(const std::string checkpoint_name, int32_t boot_num){
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      return get<C_TIME>(get<CHECKPOINTS>(data_vector.at(boot_num)).at(checkpoint_name));
    }
    // If checkpoint doesn't exist, we can't get its time
    else{
       cerr << errror << "invalid checkpoint name: " << checkpoint_name << endl;
       return Unknown;
    }
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << errror << "invalid boot number: " << boot_num << endl;
    return Unknown;
  }
}


Value SampleSystem::getTimeOfBoot(int32_t boot_num){
  if(valid_boot(boot_num)){
    return get<BOOT_TIME>(data_vector.at(boot_num));
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << errror << "invalid boot number: " << boot_num << endl;
    return Unknown;
  }
}


int32_t SampleSystem::getTimeOfCrash(int32_t boot_num){
  if(valid_boot(boot_num)){
    return get<CRASH_TIME>(data_vector.at(boot_num));
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << errror << "invalid boot number: " << boot_num << endl;
    return Unknown;
  }
}

//Commands
Value SampleSystem::setCheckpoint(const std::string& checkpoint_name, bool value){
  map<const string&,tuple<bool,int>> checkpoints = get<CHECKPOINTS>(data_vector.at(0));
  Value retval;
  // If checkpoint not set, checkpoint was not reached
  if(checkpoints.find(checkpoint_name)==checkpoints.end()) retval = false;
  else retval = checkpoints.at(checkpoint_name);
  // This inserts the element if none exists
  checkpoints[checkpoint_name] = value;
  return retval;
}


Value SampleSystem::setSafeReboot(bool b){
  Value retval = safe_to_reboot;
  safe_to_reboot = b;
  return retval;
}


Value SampleSystem::deleteCrash(int32_t boot_num){
  // Can't delete the current boot
  if(valid_boot(boot_num) && boot_num>0){
    data_vector.erase(boot_num);
    return true;
  }
  else{
    cerr << errror << "invalid boot number: " << boot_num << endl;
    return false;
  }
}
