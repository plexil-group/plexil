/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

// This is a barebones publish-subscribe facility for the sample PLEXIL
// application.  It provides a set of subscription functions specific to various
// combinations of return type and parameters.

#ifndef _H__system
#define _H__system

#include "Value.hh"
#include <string>

using namespace PLEXIL;
using std::string;

// Subscriber types
typedef void (* SubscribeBool) (const string& state_name, bool val);

typedef void (* SubscribeBoolString) (const string& state_name, bool val,
				      const string& checkpoint_name);

typedef void (* SubscribeBoolStringInt) (const string& state_name, bool val,
					 const string& checkpoint_name, int boot);


typedef void (* SubscribeValueString) (const string& state_name, Value val,
				       const string& checkpoint_name);

typedef void (* SubscribeValueStringInt) (const string& state_name, Value val,
					  const string& checkpoint_name,int boot);


typedef void (* SubscribeStringString) (const string& state_name, const string& val,
					const string& checkpoint_name);

typedef void (* SubscribeStringStringInt) (const string& state_name, const string& val,
					   const string& checkpoint_name,int boot);

// Setters for subscribers of each supported type signature
void setSubscriber (SubscribeBool);
void setSubscriber (SubscribeBoolString);
void setSubscriber (SubscribeBoolStringInt);
void setSubscriber (SubscribeValueString);
void setSubscriber (SubscribeValueStringInt);
void setSubscriber (SubscribeStringString);
void setSubscriber (SubscribeStringStringInt);

// Publish a state name, which notifies the subscriber.
void publish (const string& state_name, bool val);

void publish (const string& state_name, bool val,
	      const string& checkpoint_name);

void publish (const string& state_name,  bool val,
	      const string& checkpoint_name, int boot);


void publish (const string& state_name, Value val,
	      const string& checkpoint_name);

void publish (const string& state_name, Value val,
	      const string& checkpoint_name,int boot);


void publish (const string& state_name, const string& val,
	      const string& checkpoint_name);

void publish (const string& state_name, const string& val,
	      const string& checkpoint_name, int boot);

#endif
