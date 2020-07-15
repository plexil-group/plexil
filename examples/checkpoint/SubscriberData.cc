#include "Subscriber.hh"

// Maps arguments to a list of subscribed functions
// This has to be in a separate file such that all
// instantiations of the templated subscribe() and publish()
// functions refer to the same object
std::map<std::vector<std::string>,std::vector<void* (*)()>> subscribers;
