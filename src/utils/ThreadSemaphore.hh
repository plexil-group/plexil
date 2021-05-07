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

#ifndef PLEXIL_THREAD_SEMAPHORE_HH
#define PLEXIL_THREAD_SEMAPHORE_HH

#include <memory>

namespace PLEXIL 
{

  // Forward reference
  class ThreadSemaphoreImpl;

  /**
     @class ThreadSemaphore
     @brief Provides a cross-platform API to binary thread semaphores.
     @author Chuck Fry
   */

  class ThreadSemaphore
  {
  public:
    //! Default constructor.
    ThreadSemaphore();

    //! Destructor.
    ~ThreadSemaphore() = default;

    //! Causes the caller's thread to block until post() is called on the semaphore.
    //! @return 0 if successful, error number otherwise.
    //! @note Error number is platform dependent.
    int wait();

    //! Causes the semaphore to unblock a thread currently waiting on it.
    //! @return 0 if successful, error number otherwise.
    //! @note Error number is platform dependent.
    int post();

  private:

    //! Pointer to the implementation object.
    std::unique_ptr<ThreadSemaphoreImpl> m_impl;
  };

  // Pure virtual base class for the implementation
  class ThreadSemaphoreImpl
  {
  public:
    virtual int wait() = 0;
    virtual int post() = 0;
    virtual ~ThreadSemaphoreImpl() = default;
  };

} // namespace PLEXIL

#endif // PLEXIL_THREAD_SEMAPHORE_HH
