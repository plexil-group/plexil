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


#include "subscriber.hh"
#include "checkpoint_system.hh"

using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::tuple;
using std::map;
usint std::get;

///////////////////////////// Conveniences //////////////////////////////////

// A prettier name for the "unknown" value.
static Value Unknown;

// A preamble for error messages.
static string error = "Error in checkpoint system: ";


//////////////////////////////// State /// //////////////////////////////////


// Current boot information
static bool safe_to_reboot;
static bool did_crash;
static bool num_active_crashes;
static bool num_total_crashes;

// Data structure that tracks boot-specific metadata and checkpoints
static vector<tuple<int, /*time of boot*/
		    int, /*time of crash*/
		    map< /*map of checkpoint info*/
		      const string&, /*checkpoint name*/
		      tuple<bool, /*state of checkpoint*/
			    int>> data_vector;/*time of checkpoint activation*/


// Load all previous active crashes
void load_crashes(const string& directory){

  //Do some publishing
}

// Check that the boot number is valid
bool valid_boot(int32_t boot_num){
  return (boot_num < 0 || boot_num > num_active_crashes);
}

bool valid_checkpoint(const std::string checkpoint_name,int32_t boot_num){
  map<const string&, tuple<bool,int>> checkpoints = get<2>(data_vector.at(boot_num));
  return checkpoints.find(checkpoint) == checkpoints.end());
}

////////////////////////////////// Lookups /////////////////////////////////////
bool did_crash(){
  return did_crash;
}
int32_t num_active_crashes(){
  return num_active_crashes;
}
int32_t num_total_crashes(){
  return num_total_crashes;
}
Value get_checkpoint_state(const std::string checkpoint_name,int32_t boot_num){
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      return get<0>(get<2>(data_vector.at(boot_num)).at(checkpoint_name));
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
Value get_checkpoint_time(const std::string checkpoint_name, int32_t boot_num){
  if(valid_boot(boot_num)){
    if(valid_checkpoint(checkpoint_name, boot_num)){
      return get<1>(get<2>(data_vector.at(boot_num)).at(checkpoint_name));
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
int32_t get_time_of_boot(int32_t boot_num){
  if(valid_boot(boot_num)){
    return get<0>(data_vector.at(boot_num));
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << errror << "invalid boot number: " << boot_num << endl;
    return Unknown;
  }
}
int32_t get_time_of_crash(int32_t boot_num){
  if(valid_boot(boot_num)){
    return get<1>(data_vector.at(boot_num));
  }
  // If boot doesn't exist, something's gone wrong
  else{
    cerr << errror << "invalid boot number: " << boot_num << endl;
    return Unknown;
  }
}

//Commands
Value set_checkpoint(const std::string& checkpoint_name);
Value set_safe_reboot(bool b);
Value delete_crash(int32_t crash_number);
