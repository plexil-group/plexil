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

#ifndef _H__system
#define _H__system
#include <map>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::map;

// Subscriber type, templated function that takes a state_name, a value, and 0 or more parameters
template<typename StateType, typename ... ParamTypes>
using Subscriber = void (*) (const string& state_name, StateType val, ParamTypes ... args);


// Maps arguments to a list of subscribed functions, for use in setSubscriber and publish
// It has to be external so that each specialization of setSubscriber and publish
// can access the same datastructure without requiring the user to implement any oddities.
extern map<vector<string>,vector<void* (*)()>> subscribers;



// Defining the templated functions here (not in a .cc file) allows us to instantiate setSubscriber instances in other classes, like setSubscriber<int>
// Yes it's messy but there are three solutions:
// 1. Define the entire setSubscriber function (and associated data structures, etc.) in this header
// 2. Include subscriber.cc in any file that calls setSubscriber
// 3. Include subscriber.cc in this header and don't compile it separately

// Picking option one seemed the cleanest


// Register a subscriber
template<typename StateType, typename ... ParamTypes>
void setSubscriber (Subscriber<StateType,ParamTypes...> subscriber){

  // Create a vector made of std::strings that detail the StateType and then ParamTypes
  vector<string> signature{string(typeid(StateType).name()),string(typeid(ParamTypes).name())...};

  // Generic pointer so map can hold all subscribed functions
  void* (*generic_pointer)() = reinterpret_cast<void*(*)()>(subscriber);

  if (subscribers.find (signature) == subscribers.end() ){
    vector<void* (*) ()> new_vector;
    subscribers.insert(std::make_pair(signature,new_vector));
  }
  subscribers[signature].push_back(generic_pointer);
}



// Publish a state change to the appropriate subscriber
template<typename StateType, typename ... ParamTypes>
void publish(const string& state_name, StateType val, ParamTypes ... args){

  // Create a vector made of std::strings that detail the StateType and then ParamTypes
  vector<string> signature{string(typeid(StateType).name()),string(typeid(ParamTypes).name())...};
  
  // Retrieve vector of reception functions
  vector<void* (*)()> receivers = subscribers.at(signature);
  
  // Use parameters to cast each function to the correct type and call it
  for(auto &generic_receiver : receivers){
    Subscriber<StateType,ParamTypes...> subscriber;
    subscriber = reinterpret_cast<void (*) (const string& state_name, StateType val, ParamTypes ... args)>(generic_receiver);
    subscriber(state_name,val,args...);
  }
}
#endif
