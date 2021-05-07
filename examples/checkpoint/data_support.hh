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

#ifndef _H_data_support
#define _H_data_support

// This defines a datastructure for describing boot information which is used by both SaveManager and CheckpointSystem
#include "Nullable.hh"
#include "ValueType.hh"
#include <map>

// TODO: Evaluate if Nullable<> is necessary or if we should just use 0.0 = no time (which is what we have to assume to use InterfaceManager::queryTime())
// Alternatively: go back to old system with lookups 
struct CheckpointData{
  bool state; /*state of checkpoint*/
  Nullable<PLEXIL::Real> time; /*time of last checkpoint modification*/
  std::string info; /*user-defined checkpoint info*/
};

struct BootData{
  Nullable<PLEXIL::Real> boot_time; /*time of boot*/
  Nullable<PLEXIL::Real> crash_time;  /*time of crash*/
  bool is_ok;  /*is_OK*/
  std::map<const std::string, /*checkpoint name*/
      CheckpointData> checkpoints; /*map of checkpoint info*/
};



#endif
