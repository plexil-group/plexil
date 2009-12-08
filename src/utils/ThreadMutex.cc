/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#include "ThreadMutex.hh"
#include "Error.hh"
#include <errno.h>

namespace PLEXIL
{
  ThreadMutex::ThreadMutex()
  {
    pthread_mutexattr_t mta;

    int rv = pthread_mutexattr_init(&mta);
    assertTrue(rv != ENOMEM, "No memory for mutex attribute init.");
    assertTrue(rv == 0, "Error initializing mutex attribute structure.");

    rv = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_NORMAL);
    assertTrue(rv != EINVAL, "PTHREAD_MUTEX_NORMAL is an invalid value");
    assertTrue(0 == rv, "Could not set the mutex attribute.");

    rv = pthread_mutex_init(&m_mutex, &mta);
    assertTrue(0 == rv, "Could not initialize the mutex.");
  }

  ThreadMutex::~ThreadMutex()
  {
    int rv = pthread_mutex_destroy(&m_mutex);
    assertTrue(rv != EBUSY, "Attempted to destroy mutex while locked or referenced.");
    assertTrue(0 == rv, "Could not destroy the mutex.")
  }

  void ThreadMutex::lock()
  {
    int rv = pthread_mutex_lock(&m_mutex);
    assertTrue(rv != EDEADLK, "Deadlock detected, or attempt to lock mutex that is already locked by this thread.");
    assertTrue(0 == rv, "Could not lock the mutex.");
  }

  bool ThreadMutex::trylock()
  {
    int rv = pthread_mutex_trylock(&m_mutex);
    if (0 == rv)
      return true;
    assertTrue(EBUSY == rv, "Could not trylock the mutex.");
    return false;
  }

  void ThreadMutex::unlock()
  {
    int rv = pthread_mutex_unlock(&m_mutex);
    assertTrue(rv != EPERM, "Attempt to unlock mutex that is locked by another thread.");
    assertTrue(0 == rv, "Could not unlock the mutex.");
  }

}
