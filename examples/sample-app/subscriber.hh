#ifndef _H__system
#define _H__system

#include <string>
#include <unordered_map>
#include <typeinfo>
#include <vector>
#include <iostream>

// Subscriber type, templated function that takes a state_name, a value, and 0 or more parameters
template<class ValueType, class ... ParamTypes>
using Subscribe = void (*) (const std::string& state_name, ValueType val, ParamTypes ... args);

// Setters for the subscriber
template<class ValueType, class ... ParamTypes>
void setSubscriber (void (*receiver) (const std::string& state_name, ValueType val, ParamTypes ... args));

template<class ValueType, class ... ParamTypes>
void publish(const std::string& state_name, ValueType val, ParamTypes ... args);
#endif
