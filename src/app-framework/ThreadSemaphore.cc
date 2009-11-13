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

#include "ThreadSemaphore.hh"
#include "Error.hh"

#ifdef PLEXIL_USE_POSIX_SEMAPHORES
#include <errno.h>
#endif

namespace PLEXIL
{

#ifdef PLEXIL_USE_POSIX_SEMAPHORES

  //
  // POSIX implementation
  //

  ThreadSemaphore::ThreadSemaphore()
  {
    int status = sem_init(&m_posix_sem, 0, 0);
    assertTrueMsg(status != -1,
		  "ThreadSemaphore (POSIX) constructor: sem_init failed, errno = "
		  << errno);
  }

  ThreadSemaphore::~ThreadSemaphore()
  {
    int status = sem_destroy(&m_posix_sem);
    assertTrueMsg(status != -1,
		  "ThreadSemaphore (POSIX) destructor: sem_destroy failed, errno = "
		  << errno);
  }

  bool ThreadSemaphore::wait()
  {
    int status;
    // If the wait fails due to a signal, ignore the error (EINTR).
    // If the error is not EINTR, stop the thread.
    while (((status = sem_wait(&m_posix_sem)) == -1) && (errno == EINTR))
      continue;
    
    assertTrueMsg(status != -1,
		  "ThreadSemaphore::wait: sem_wait failed, errno = " << errno);
    return true;
  }

  int ThreadSemaphore::post()
  {
    int status = sem_post(&m_posix_sem);
    if (status == -1)
      return errno;
    else return 0;
  }

#endif // PLEXIL_USE_POSIX_SEMAPHORES

#ifdef PLEXIL_USE_MACH_SEMAPHORES

  //
  // MACH implementation
  //

  ThreadSemaphore::ThreadSemaphore()
  {
    m_mach_owning_task = mach_task_self();
    kern_return_t status = 
      semaphore_create(m_mach_owning_task,
		       &m_mach_sem,
		       SYNC_POLICY_FIFO,
		       0);
    assertTrueMsg(status == KERN_SUCCESS,
		  "ThreadSemaphore (MACH) constructor: semaphore_create failed, status = "
		  << status);
  }

  ThreadSemaphore::~ThreadSemaphore()
  {
    kern_return_t status = 
      semaphore_destroy(m_mach_owning_task,
			m_mach_sem);
    assertTrueMsg(status == KERN_SUCCESS,
		  "ThreadSemaphore (MACH) destructor: semaphore_destroy failed, status = "
		  << status);
  }

  bool ThreadSemaphore::wait()
  {
    kern_return_t status = semaphore_wait(m_mach_sem);
    if (status == KERN_SUCCESS)
      return true;
    assertTrueMsg(status == KERN_ABORTED,
		  "ThreadSemaphore::wait: semaphore_wait failed, status = "
		  << status);
    // thread was canceled
    return false;
  }

  int ThreadSemaphore::post()
  {
    kern_return_t status = semaphore_signal(m_mach_sem);
    return status;
  }

#endif // PLEXIL_USE_MACH_SEMAPHORES

} // namespace PLEXIL
