// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_INPUT_QUEUE_HH
#define PLEXIL_INPUT_QUEUE_HH

namespace PLEXIL
{
  struct QueueEntry;

  //! \class InputQueue
  //! \brief Pure virtual base class for an input queue for the PLEXIL Exec.
  //!
  //! The InputQueue API presumes the existence of a free list for
  //! queue entries.  The client should call the release() method to
  //! return entries when they are not in use.  The allocate() method
  //! should prefer to return an entry from the free list.  New
  //! entry instances should be allocated if none are free.
  //!
  //! \ingroup External-Interface
  //! \see QueueEntry
  class InputQueue
  {
  public:

    //! \brief Virtual destructor.
    virtual ~InputQueue() = default;

    //! \brief Is the queue empty?
    //! \return true if empty, false if not.
    virtual bool isEmpty() const = 0;

    //
    // Reader side
    //

    //! \brief Get the head of the queue.
    //! \return Pointer to the first QueueEntry. If empty, returns null.
    virtual QueueEntry *get() = 0;

    //! \brief Flush the queue without examining it.
    virtual void flush() = 0;

    //! \brief Return a queue entry to the free list after use.
    //! \param entry Pointer to the QueueEntry.
    virtual void release(QueueEntry *entry) = 0;

    //
    // Writer side
    //

    //! \brief Get an entry for insertion.
    //! \return Pointer to the QueueEntry.
    virtual QueueEntry *allocate() = 0;

    //! \brief Insert an entry at the tail of the queue.
    //! \param entry Pointer to the QueueEntry.
    virtual void put(QueueEntry *entry) = 0;

  };

} // namespace PLEXIL

#endif // PLEXIL_INPUT_QUEUE_HH
