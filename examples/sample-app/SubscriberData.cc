#include "Subscriber.hh"

// Maps arguments to a list of subscribed functions
// See subscriber.hh as to why this is necessary
std::map<std::vector<std::string>,std::vector<void* (*)()>> subscribers;
