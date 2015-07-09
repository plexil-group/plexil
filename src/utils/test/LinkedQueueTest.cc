/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

#include "Error.hh"
#include "LinkedQueue.hh"

using namespace PLEXIL;

class QueueTest : public QueueItem<QueueTest>
{
public:
  int value;

  QueueTest(int n) : value(n) {}
};

bool LinkedQueueTest()
{
  LinkedQueue<QueueTest> testq;

  // Basics
  assertTrue_1(testq.empty());
  assertTrue_1(testq.size() == 0);
  assertTrue_1(testq.front() == NULL);

  // Pop empty queue should be safe
  testq.pop();
  assertTrue_1(testq.empty());
  assertTrue_1(testq.size() == 0);
  assertTrue_1(testq.front() == NULL);

  {
    // Push an item
    QueueTest *won = new QueueTest(1);
    assertTrue_1(won->value == 1);
    testq.push(won);
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.size() == 1);
    assertTrue_1(testq.front() != NULL);
    assertTrue_1(testq.front()->value == 1);
    assertTrue_1(testq.front()->next() == NULL);

    testq.pop();
    assertTrue_1(testq.empty());
    assertTrue_1(testq.size() == 0);
    assertTrue_1(testq.front() == NULL);

    // Pop empty queue should be safe
    testq.pop();
    assertTrue_1(testq.empty());
    assertTrue_1(testq.size() == 0);
    assertTrue_1(testq.front() == NULL);

    delete won;
  }

  // Push a bunch of items
  int n = 10;
  for (int i = 1; i <= n; ++i) {
    testq.push(new QueueTest(i));
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.size() == i);
    assertTrue_1(testq.front() != NULL);
  }

  // Step through the queue
  QueueTest *item = testq.front();
  for (int i = 1; i <= n; ++i) {
    assertTrue_1(item != NULL);
    assertTrue_1(item->value == i);
    item = static_cast<QueueTest *>(item->next());
  }
  // Should have reached end
  assertTrue_1(item == NULL);

  for (int i = 1; i <= n; ++i) {
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.front() != NULL);
    item = testq.front();
    assertTrue_1(item->value == i);
    testq.pop();
    delete item;
  }
  item = NULL;

  assertTrue_1(testq.empty());

  return true;
}

