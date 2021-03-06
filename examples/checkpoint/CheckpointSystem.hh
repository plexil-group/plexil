/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "data_support.hh"
#include "Nullable.hh"
#include "ReadWriteLock.hh"
#include "SaveManager.hh"

#include "Command.hh"
#include "Value.hh"

#include "pugixml.hpp"

#include <memory>

// This is a class that stores, operates on, and provides information
// about crashes and checkpoints

class CheckpointSystem final
{
public:
  ~CheckpointSystem() = default;

  // Singleton paradigm
  static CheckpointSystem *getInstance();
  
  // Lookups
  bool didCrash();
  PLEXIL::Integer numAccessibleBoots();
  PLEXIL::Integer numTotalBoots();
  PLEXIL::Integer numUnhandledBoots();
  PLEXIL::Value getCheckpointState(const std::string& checkpoint_name,PLEXIL::Integer boot_num);
  PLEXIL::Value getCheckpointTime(const std::string& checkpoint_name, PLEXIL::Integer boot_num);
  PLEXIL::Value getCheckpointInfo(const std::string& checkpoint_name, PLEXIL::Integer boot_num);
  PLEXIL::Value getCheckpointLastPassed(const std::string& checkpoint_name);
  PLEXIL::Value getTimeOfBoot(PLEXIL::Integer boot_num);
  PLEXIL::Value getTimeOfCrash(PLEXIL::Integer boot_num);
  PLEXIL::Value getIsOK(PLEXIL::Integer boot_num);

  // Commands
  void setCheckpoint(const std::string& checkpoint_name, bool value, std::string& info, PLEXIL::Command* cmd);
  void setOK(bool b, PLEXIL::Integer boot_num, PLEXIL::Command *cmd);
  bool flush();

  // Helper
  void start();
  void setSaveConfiguration(const pugi::xml_node* configXml);
  void useTime(bool use_time);

private:
  
  // Only getInstance() static member function should have access
  // to the constructor
  CheckpointSystem();

  //Prohibits copying or assigning
  CheckpointSystem(const CheckpointSystem&) = delete;
  CheckpointSystem(CheckpointSystem&&) = delete;
  CheckpointSystem& operator= (const CheckpointSystem&) = delete;
  CheckpointSystem& operator= (CheckpointSystem&&) = delete;
  
  // Helper functions
  bool valid_boot(PLEXIL::Integer boot_num);
  bool valid_checkpoint(const std::string& checkpoint_name, PLEXIL::Integer boot_num);
  
  // Synchronization control
  ReadWriteLock m_rw;

  // Persistent Storage system
  std::unique_ptr<SaveManager> m_manager;
  
  // Data
  std::vector<BootData> m_data_vector;
  int m_num_total_boots;

  bool m_use_time;
};

#endif
