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

#ifndef _H_CheckpointSystem
#define _H_CheckpointSystem
#include "Value.hh"
#include "Nullable.hh"
#include "StateCacheEntry.hh"
#include "InterfaceAdapter.hh"
#include "SimpleSaveManager.hh"
#include "ValueType.hh"
#include "ReadWriteLock.hh"

#include <iostream>


using namespace PLEXIL;

using std::string;
using std::vector;
using std::tuple;
using std::map;

// This is a class that stores, operates on, and provides information
// about crashes and checkpoints


class CheckpointSystem
{
public:

  //Prohibits copying or assigning
  CheckpointSystem (const CheckpointSystem&) = delete;
  CheckpointSystem& operator= (const CheckpointSystem&) = delete;

  ~CheckpointSystem ()
  {
    if (m_system) {
      delete m_system;
    }
    delete manager;
  }


  static CheckpointSystem *getInstance () {
    if (!m_system) {
      m_system = new CheckpointSystem;
    }
    return m_system;
  }
  
  // Lookups
  bool didCrash();
  Integer numAccessibleBoots();
  Integer numTotalBoots();
  Integer numUnhandledBoots();
  Value getCheckpointState(const string& checkpoint_name,Integer boot_num);
  Value getCheckpointTime(const string& checkpoint_name, Integer boot_num);
  Value getCheckpointInfo(const string& checkpoint_name, Integer boot_num);
  Value getCheckpointLastPassed(const string& checkpoint_name);
  Value getTimeOfBoot(Integer boot_num);
  Value getTimeOfCrash(Integer boot_num);
  Value getIsOK(Integer boot_num);

  
  // Commands
  Value setCheckpoint(const string& checkpoint_name, bool value, string& info);
  Value setOK(bool b, Integer boot_num);
  bool flush();

  // Helper
  void start();
  void setDirectory(const string& file_directory);
private:
  CheckpointSystem(): manager(new SimpleSaveManager) {}
  static CheckpointSystem *m_system;

  // Current boot information
  int num_total_boots;

  ReadWriteLock rw;
  SaveManager* const manager;
  
  
  // Data structure that tracks boot-specific metadata and checkpoints
  vector<tuple<Nullable<Real>, /*time of boot*/
	       Nullable<Real>, /*time of crash*/
	       bool, /*is_OK*/
	       map< /*map of checkpoint info*/
		 const string, /*checkpoint name*/
		 tuple<bool, /*state of checkpoint*/
		       Nullable<Real>,/*time of checkpoint activation*/
		       string>>>> /*user-defined checkpoint info*/
  data_vector;
  
  
  // Helper functions

  bool valid_boot(Integer boot_num);
  bool valid_checkpoint(const string& checkpoint_name,Integer boot_num);

};

static Nullable<Real> get_time();
#endif
