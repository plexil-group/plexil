#include "subscriber.hh"
#include "Debug.hh"


using std::string;
using std::vector;
using std::unordered_map;

// Helper struct for hashing a vector (to use as the key in the unordered_map)
struct VectorHasher {
  int operator()(const vector<string> &V) const {
        int hash = V.size();
	std::hash<string> hasher;
	for(string const &s : V) {
	  hash ^= hasher(s) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

//Maps arguments to a list of subscribed functions
static unordered_map<vector<string>,vector<void* (*)()>,VectorHasher> subscribers;


// Register a subscriber
template<class ValueType, class ... ParamTypes>
void setSubscriber (void (*receiver) (const string& state_name, ValueType val, ParamTypes ... args)){

  // Create a vector made of std::strings that detail the ValueType and then ParamTypes
  vector<string> signature{string(typeid(ValueType).name()),string(typeid(ParamTypes).name())...};
 
  void* (*generic_pointer)() = reinterpret_cast<void*(*)()>(receiver);

  // Check if vector of functions associated with this signature already exists in the map
  auto got = subscribers.find (signature);
  // If vector does not exist, make it
  if ( got == subscribers.end() ){
    vector<void* (*) ()> new_vector;
    subscribers.emplace(signature,new_vector);
  }
  // Add the new subscribed function to the list of subscribed values for the signature
  subscribers[signature].push_back(generic_pointer);
}


// Publish a state change to the appropriate subscriber

template<class ValueType, class ... ParamTypes>
void publish(const string& state_name, ValueType val, ParamTypes ... args){
  // Create a vector made of std::strings that detail the ValueType and then ParamTypes
  vector<string> signature (string(typeid(ValueType).name()),string(typeid(ParamTypes).name())...);


  // Retrieve vector of reception functions
  vector<void* (*)()> receivers = subscribers.at(signature);
  
  // Use parameters to cast each function to the correct type and call it
  for(auto &generic_receiver : receivers){
    void (*receiver) (const string& state_name, ValueType val, ParamTypes ... args);
    receiver = reinterpret_cast<void (*) (const string& state_name, ValueType val, ParamTypes ... args)>(generic_receiver);
    // Calls the receiver function
    receiver(state_name,val,args...);
  }
}
