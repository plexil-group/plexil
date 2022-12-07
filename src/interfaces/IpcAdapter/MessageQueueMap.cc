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
 *
 * MessageQueueMap.cc
 *
 *  Created on: Feb 19, 2010
 *      Author: jhogins
 */

#include "MessageQueueMap.hh"

#include "Command.hh"
#include "Debug.hh"
#include "Expression.hh"

#include <utility>

namespace PLEXIL 
{

  MessageQueueMap::MessageQueueMap(AdapterExecInterface& execInterface, bool allowDuplicateMessages) :
    m_mutex(),
    m_execInterface(execInterface),
    m_allowDuplicateMessages(allowDuplicateMessages) {
  }

  MessageQueueMap::~MessageQueueMap() {
  }

  /**
   * @brief Adds the given recipient to the queue to receive the given message.
   * If a recipient already exists for this message, messages will be handed out in the order
   * of the adding of the recipients.
   * @param message The message the recipient is waiting for
   * @param cmd The command instance
   * @note Always executed from the Exec thread.
   */
  void MessageQueueMap::addRecipient(const std::string& message, Command *cmd) {
    debugMsg("MessageQueueMap:addRecipient", ' ' << this << " for \"" << message << "\"");
    PairingQueue* que = ensureQueue(message);
    que->m_recipientQueue.push(cmd);
    updateQueue(que);
    debugMsg("MessageQueueMap:addRecipient", ' ' << this << " added for message \"" << que->m_name << '"');
  }

  /**
   * @brief Adds the given message to its queue. If there is a recipient waiting for the message, it is sent immediately.
   * @param message The message string to be added
   * @note Only called from IpcAdapter::handleMessageMessage().
   */
  void MessageQueueMap::addMessage(const std::string& message) {
    debugMsg("MessageQueueMap:addMessage", ' ' << this << " entered for \"" << message << "\"");
    PairingQueue* pq = ensureQueue(message);
    if (!m_allowDuplicateMessages)
      while (!pq->m_messageQueue.empty())
        pq->m_messageQueue.pop();
    pq->m_messageQueue.emplace(Value(message));
    updateQueue(pq);
    debugMsg("MessageQueueMap:addMessage", ' ' << this << " Message \"" << pq->m_name << "\" added");
  }

  /**
   * @brief Adds the given message with the given parameters to its queue.
   * If there is a recipient waiting for the message, it is sent immediately.
   * @param message The message string to be added
   * @param params The parameters that are to be sent with the message
   * @note Only called from IpcAdapter::handleCommandSequence().
   */
  void MessageQueueMap::addMessage(const std::string& message, const Value& param) {
    debugMsg("MessageQueueMap:addMessage", ' ' << this << " entered for \"" << message << "\"");
    PairingQueue* pq = ensureQueue(message);
    if (!m_allowDuplicateMessages)
      while (!pq->m_messageQueue.empty())
        pq->m_messageQueue.pop();
    pq->m_messageQueue.emplace(param);
    updateQueue(pq);
    debugMsg("MessageQueueMap:addMessage",
             ' ' << this << " Message \"" << pq->m_name << "\" added, value = \"" << param << '"');
  }

  /**
   * @brief Sets the flag that determines whether or not incoming messages
   *        with duplicate strings are queued. If true, all incoming messages are
   *        put into the queue. Oldest instances of the message are distributed first.
   *        If false, new messages with duplicate strings replace older ones; this
   *        will remove all oldest duplicates from the queue immediately as well
   *        as set the behavior for future messages.
   * @param flag If false, duplicates will be replaced with the newest
   *        message. If true, duplicates are queued.
   * @note Only called from IpcAdapter::initialize(). So unlikely to have received any messages
   *       when this is called.
   */
  void MessageQueueMap::setAllowDuplicateMessages(bool flag) {
    debugMsg("MessageQueueMap:setAllowDuplicateMessages", ' ' << this << " to " << flag);
    m_allowDuplicateMessages = flag;
  }

  //! @brief Get the PairingQueue for this message, if it exists.
  MessageQueueMap::PairingQueue* MessageQueueMap::getQueue(const std::string& message)
  {
    std::lock_guard<std::mutex> guard(m_mutex);
    QueueMap::iterator it = m_map.find(message);
    if (m_map.end() != it)
      return it->second.get();
    return nullptr;
  }

  //! @brief Get or construct the PairingQueue for this message.
  MessageQueueMap::PairingQueue * MessageQueueMap::ensureQueue(const std::string& message)
  {
    PairingQueue* result = nullptr;
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      QueueMap::iterator it = m_map.find(message);
      if (m_map.end() == it) {
        it = m_map.emplace(QueueMap::value_type(message,
                                                std::unique_ptr<PairingQueue>(new PairingQueue(message)))).first;
      }
      return it->second.get();
    }
    debugMsg("MessageQueueMap:ensureQueue", " created new queue with name \"" << message << '"');
    return result;
  }

  /**
   * @brief Resolves matches between messages and recipients. Should be called whenever updates occur to a queue.
   */
  void MessageQueueMap::updateQueue(PairingQueue* queue)
  {
    debugMsg("MessageQueueMap:updateQueue", ' ' << queue->m_name << " entered");
    MessageQueue& mq = queue->m_messageQueue;
    RecipientQueue& rq = queue->m_recipientQueue;
    bool valChanged = !mq.empty() && !rq.empty();
    while (!mq.empty() && !rq.empty()) {
      debugMsg("MessageQueueMap:updateQueue", ' ' << queue->m_name << " returning value");
      debugMsg("MessageQueueMap:updateQueue", ' ' << queue->m_name << " returning value");
      m_execInterface.handleCommandReturn(rq.front(), mq.front());
      debugMsg("MessageQueueMap:updateQueue", ' ' << queue->m_name
               << " recipient inactive, ignoring");
      rq.pop();
      mq.pop();
    }
    if (valChanged) {
      debugMsg("MessageQueueMap:updateQueue", " Message \"" << queue->m_name << "\" paired and sent");
      m_execInterface.notifyOfExternalEvent();
    }
  }

}
