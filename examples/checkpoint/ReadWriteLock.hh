/* Copyright (c) 2020-2020, Universities Space Research Association (USRA).
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

#ifndef _H__ReadWrite
#define _H__ReadWrite

#include "ThreadMutex.hh"

// Implements a write-favoring read-write lock
class ReadWriteLock{
public:

  ReadWriteLock() : r_count(0){}
  // Using default destructor
  
  void begin_read(){
    // Block if there is a writer writing
    turn_lock.lock();
    turn_lock.unlock();
    r_lock.lock();
    r_count++;
    if(r_count == 1){ // First in
      w_lock.lock();
    }
    r_lock.unlock();
  }

  void end_read(){
    r_lock.lock();
    r_count--;
    if(r_count == 0){ // Last out
      w_lock.unlock();
    }
    r_lock.unlock();
  }

  void begin_write(){
    turn_lock.lock();
    w_lock.lock();
  }

  void end_write(){
    turn_lock.unlock();
    w_lock.unlock();
  }
private:
  // Disallow copy
  ReadWriteLock & operator=(const ReadWriteLock&);
  ReadWriteLock(const ReadWriteLock&);
  
  int r_count; // Count of readers
  PLEXIL::ThreadMutex r_lock; // Protects access to r_count
  PLEXIL::ThreadMutex w_lock; // Protects writes to data
  PLEXIL::ThreadMutex turn_lock; // Write is awaiting a turn
};




#endif
