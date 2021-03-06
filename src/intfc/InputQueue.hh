/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_INPUT_QUEUE_HH
#define PLEXIL_INPUT_QUEUE_HH

namespace PLEXIL
{
  struct QueueEntry;

  /**
   * @class InputQueue
   * @brief API for an input queue for the exec.
   */
  class InputQueue
  {
  public:
    InputQueue() = default;
    virtual ~InputQueue() = default;

    // Simple query
    virtual bool isEmpty() const = 0;

    //
    // Reader side
    //

    // Get the head of the queue. If empty, returns nullptr.
    virtual QueueEntry *get() = 0;

    // Flush the queue without examining it.
    virtual void flush() = 0;

    // Return an entry to the free list after use.
    virtual void release(QueueEntry *entry) = 0;

    //
    // Writer side
    //

    // Get an entry for insertion. Will allocate if none on the free list.
    virtual QueueEntry *allocate() = 0;

    // Insert an entry on the queue.
    virtual void put(QueueEntry *entry) = 0;

  private:

    // Disallow copy, assign
    InputQueue(InputQueue const &) = delete;
    InputQueue(InputQueue &&) = delete;
    InputQueue &operator=(InputQueue const &) = delete;
    InputQueue &operator=(InputQueue &&) = delete;
  };

} // namespace PLEXIL

#endif // PLEXIL_INPUT_QUEUE_HH
