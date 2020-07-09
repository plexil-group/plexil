/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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
#include "Debug.hh"

using std::string;


// Helper struct for hashing a vector (to use as the key in the unordered_map)
struct VectorHasher {
  int operator()(const std::vector<string> &V) const {
        int hash = V.size();
	std::hash<string> hasher;
	for(string const &s : V) {
	  hash ^= hasher(s) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

//Maps arguments to functions
static std::unordered_map<std::vector<string>,void* (*)(),VectorHasher> subscribers;


// Register a subscriber
// Note that only one subscriber can be registered for each value/parameter combination
// This was the original functionality and has been preserved, but could be changed easily
// by switching from a single function pointer to a vector of function pointers as the map value type
template<class ValueType, class ... ParamTypes>
void setSubscriber (void (*receiver) (const string& state_name, ValueType val, ParamTypes ... args)){

  // Create a vector made of std::strings that detail the ValueType and then ParamTypes
  std::vector<string> signature{string(typeid(ValueType).name()),string(typeid(ParamTypes).name())...};
 
  void* (*generic_pointer)() = reinterpret_cast<void*(*)()>(receiver);
  subscribers.emplace(signature,generic_pointer);
}

// Publish a state change to the appropriate subscriber

template<class ValueType, class ... ParamTypes>
void publish(const std::string& state_name, ValueType val, ParamTypes ... args){
  // Create a vector made of std::strings that detail the ValueType and then ParamTypes
  std::vector<string> signature (string(typeid(ValueType).name()),string(typeid(ParamTypes).name())...);

  // Use that vector to retrieve the correct function and cast it to the correct type
  void (*receiver) (const string& state_name, ValueType val, ParamTypes ... args);
  receiver = reinterpret_cast<void (*) (const string& state_name, ValueType val, ParamTypes ... args)>(subscribers.at(signature));
  // Calls the receiver function
  receiver(state_name,val,args...);
}
