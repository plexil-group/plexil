/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

#ifndef VALUE_QUEUE_HH
#define VALUE_QUEUE_HH

#include "Expression.hh"

namespace PLEXIL
{

  //
  // Value queue
  //

  /**
   * @brief Represents the type of a ValueQueue::QueueEntry instance.
   */
  enum QueueEntryType {
	queueEntry_EMPTY,
	queueEntry_MARK,
	queueEntry_LOOKUP_VALUES,
	queueEntry_RETURN_VALUE,
	queueEntry_PLAN,
	queueEntry_LIBRARY,
	queueEntry_ERROR
  };

  /**
   * @brief A private internal class where the InterfaceManager temporarily
   *        stores the results of asynchronous operations on the world outside
   *        the Exec.
   */
  class ValueQueue
  {
  public:
	ValueQueue();
	~ValueQueue();

	// Inserts the new expression/value pair into the queue
	void enqueue(const ExpressionId & exp, double newValue);
	void enqueue(const State & state, double newValue);
	void enqueue(PlexilNodeId newPlan, const LabelStr & parent);
	void enqueue(PlexilNodeId newlibraryNode);

	/**
	 * @brief Atomically check head of queue and dequeue if appropriate
	 * @return Type of entry dequeued; queueEntry_EMPTY and queueEntry_MARK
	 * indicate nothing of interest was dequeued
	 */
	QueueEntryType dequeue(double& newValue, 
						   State& state,
						   ExpressionId& exp,
						   PlexilNodeId& plan,
						   LabelStr& planParent,
						   unsigned int& sequence);

	// returns true iff the queue is empty
	bool isEmpty() const;

	//* Inserts a marker expression into the queue and returns its sequence number
	unsigned int mark();

	// Remove queue head and ignore (presumably a mark)
	void pop();

  private:
	// deliberately unimplemented
	ValueQueue(const ValueQueue &);
	ValueQueue & operator=(const ValueQueue &);

	//
	// Internal methods
	//
	class QueueEntry;
	typedef Id<QueueEntry> QueueEntryId;

	//* Insert the given entry into the queue.
	void insert(QueueEntryId entry);

	//* Remove the current head and return it.
	QueueEntryId behead();

	//* Add the entry to the free list.
	void free(QueueEntryId entry);

	//* Allocate from the free list if possible, else construct a new entry.
	QueueEntryId allocate();

	//
	// Member variables
	//

	DECLARE_STATIC_CLASS_CONST(State, 
							   NULL_STATE_KEY,
							   State(EMPTY_LABEL(), std::vector<double>()));

	/**
	 * @brief Represents one entry in a ValueQueue.  
	 *        A private class internal to ValueQueue.
	 */
	class QueueEntry
	{
	public:
	  /*
	   * @brief Constructor for an empty QueueEntry of an arbitrary QueueEntryType.
	   */
	  QueueEntry()
		: id(this),
		  next(),
		  type(queueEntry_EMPTY),
		  value(),
		  expression(),
		  state(),
		  plan(),
		  parent(EMPTY_LABEL()),
		  sequence(0)
	  {
	  }

	  /**
	   * @brief Destructor.
	   */
	  ~QueueEntry()
	  {
	  }

	  /**
	   * @brief Reset to initialized state for reuse.
	   */
	  void clear()
	  {
		// Only clear the fields that were active
		switch (type) {
		case queueEntry_RETURN_VALUE:
		  value = Expression::UNKNOWN();
		  expression = ExpressionId::noId();
		  break;
			
		case queueEntry_LOOKUP_VALUES:
		  value = Expression::UNKNOWN();
		  state.first = EMPTY_LABEL();
		  state.second.clear();
		  break;

		case queueEntry_PLAN:
		  parent = EMPTY_LABEL();
		  // fall through to...
		case queueEntry_LIBRARY:
		  plan = PlexilNodeId::noId();
		  break;

		case queueEntry_MARK:
		  sequence = 0;
		  break;

		default:
		  break;
		}
		// Now can clear type
		type = queueEntry_EMPTY;
	  }

	  const QueueEntryId& getId() const
	  {
		return id;
	  }
	
	  QueueEntryId id; //* ID of this entry.
	  QueueEntryId next; //* Link to the next queue (or free list) entry.
	  QueueEntryType type; //* What kind of queue entry this is.
	  // Data for value queue
	  double value; //* The value being returned.
	  ExpressionId expression; //* The expression to which this value belongs for command returns.
	  State state; //* The state to which this value belongs for lookup returns.
	  // Data for plan queue
	  PlexilNodeId plan; //* The intermediate representation of this plan or library node.
	  LabelStr parent; //* The parent node ID under which to store the plan (NYI).
	  // Data for mark
	  unsigned int sequence; //* The count of marks issued to date.
	};

	//
	// Queue data
	//
	QueueEntryId m_head; //* The next entry to be dequeued.
	QueueEntryId m_tail; //* The current end of the queue.
	QueueEntryId m_freeList; //* Stack of recycled entries for later reuse.

	/** 
	 * @brief Pointer to a mutex to prevent collisions between threads.
	 * @note Implemented as a pointer so isEmpty() can be const.
	 */
	ThreadMutex * m_mutex;

	//* Serial number for marks
	unsigned int m_markCount;
  };

}

#endif // VALUE_QUEUE_HH
