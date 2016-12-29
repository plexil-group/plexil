/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_MUTEX_HH
#define PLEXIL_MUTEX_HH

#include <string>
#include <vector>

namespace PLEXIL
{
  //
  // Forward references
  //

  class ExpressionListener;
  class Node;

  class Mutex
  {
  public:
    ~Mutex();

    /**
     * @brief Attempt to acquire this mutex for the named node.
     * @param acquirer The node wishing to acquire the mutex.
     * @return true if successful, false if the mutex is already held.
     * @note Failure to acquire places the node on a queue of waiting nodes.
     */

    bool tryAcquire(Node const *acquirer);

    /**
     * @brief Release the mutex so another node can use it.
     */

    void release();

    /**
     * @brief Report which node owns this mutex.
     * @return Pointer to the node; may be null.
     */
    Node const *getHolder() const;

    /**
     * @brief Register the listener to be notified when the mutex becomes available.
     * @param l Pointer to the listener.
     */

    void addListener(ExpressionListener *l);

    /**
     * @brief Remove the listener from the notification list.
     * @param l Pointer to the listener.
     */

    void removeListener(ExpressionListener *l);

  private:

    friend Mutex *createMutex(char const *);

    Mutex(char const *name);

    // Not implemented
    Mutex() = delete;
    Mutex(Mutex const &) = delete;
    Mutex(Mutex &&) = delete;
    Mutex &operator=(Mutex const &) = delete;
    Mutex &operator=(Mutex &&) = delete;

    // Internal functions

    /**
     * @brief Notify all registered listeners when the mutex is released.
     */
    void notifyAvailable();

    std::string m_name;
    Node const *m_holder;
    std::vector<ExpressionListener *> m_listeners;
  };

  /**
   * @brief Find the named Mutex. If it does not exist, create it.
   * @param name The name of the mutex.
   * @return Pointer to the named Mutex.
   * @note Should never return null.
   */

  Mutex *ensureMutex(char const *name);

}

#endif // PLEXIL_MUTEX_HH
