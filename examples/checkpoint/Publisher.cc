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

#include "Subscriber.hh"

// The checkpoint adapter to publish to
static CheckpointAdapter *instance = 0;

void setSubscriber(CheckpointAdapter *i) {
  instance = i;
}

// The overloaded publish function, one for each number of Values found in this application

void publish (const std::string& state_name,
	      const PLEXIL::Value& val){
  
  instance->receiveValue(state_name,val);
}


void publish (const std::string& state_name,
	      const PLEXIL::Value& val,
	      const PLEXIL::Value& arg){
  
  instance->receiveValue(state_name,val,arg);
}

void publish (const std::string& state_name,
	      const PLEXIL::Value& val,
	      const PLEXIL::Value& arg1,
	      const PLEXIL::Value& arg2){
  
  instance->receiveValue(state_name,val,arg1,arg2);
}

void publishCommandReceived (PLEXIL::Command* cmd){
  instance->receiveCommandReceived(cmd);
}

void publishCommandSuccess (PLEXIL::Command* cmd){
  instance->receiveCommandSuccess(cmd);
}
