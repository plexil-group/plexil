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
/*
// Backwards compatibility types
using SubscribeInt = Subscribe<int>;
using SubscribeReal = Subscribe<float>;
using SubscribeString = Subscribe<const std::string&>;
using SubscribeBoolString = Subscribe<bool, const std::string&>;
using SubscribeBoolIntInt = Subscribe<bool, int, int>;


// Setters for subscriber.
void setSubscriberInt (SubscribeInt);
void setSubscriberReal (SubscribeReal);
void setSubscriberString (SubscribeString);
void setSubscriberBoolString (SubscribeBoolString);
void setSubscriberBoolIntInt (SubscribeBoolIntInt);
*/
// Setters for the subscriber
template<class ValueType, class ... ParamTypes>
void setSubscriber (void (*receiver) (const std::string& state_name, ValueType val, ParamTypes ... args));

template<class ValueType, class ... ParamTypes>
void publish(const std::string& state_name, ValueType val, ParamTypes ... args);
#endif
