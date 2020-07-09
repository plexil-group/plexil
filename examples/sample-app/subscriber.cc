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
/*
// Backwards compatibility
#define defSubscriber(signature) \
void setSubscriber##signature (Subscribe##signature s) \
{ \
  setSubscriber(s);				\
}

defSubscriber(Int)
defSubscriber(Real)
defSubscriber(String)
defSubscriber(BoolString)
defSubscriber(BoolIntInt)
*/
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
