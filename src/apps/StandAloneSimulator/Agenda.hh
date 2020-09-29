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

#ifndef STAND_ALONE_SIMULATOR_HH
#define STAND_ALONE_SIMULATOR_HH

#include "plexil-config.h"

// timeval
#ifdef HAVE_SYS_TIME_H 
#include <sys/time.h>
#endif

// size_t
#if defined(HAVE_CSTDDEF)
#include <cstddef>
#elif defined(HAVE_STDDEF_H)
#include <stddef.h>
#endif 

/**
 * @class Agenda The schedule of simulator responses to send.
 */

struct ResponseMessage;

class Agenda
{
public:
  virtual size_t size() const = 0;
  virtual bool empty() const = 0;
  virtual void setSimulatorStartTime(timeval const &tym) = 0;
  virtual timeval nextResponseTime() const = 0;
  virtual ResponseMessage *getNextResponse(timeval &tym) = 0;
  virtual void pop() = 0;
  virtual void scheduleResponse(timeval tym, ResponseMessage *msg) = 0;

  virtual ~Agenda()
  {
  }

};

Agenda *makeAgenda();

#endif // STAND_ALONE_SIMULATOR_HH
