/* Copyright (c) 2006-2017, Universities Space Research Association (USRA).
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
#include "TestSupport.hh"

using namespace PLEXIL;

struct QueueTest
{
private:
  QueueTest *nxt;

public:

  int value;

  QueueTest(int n)
    : nxt(nullptr),
      value(n)
  {
  }

  QueueTest(QueueTest const &orig)
    : nxt(nullptr),
      value(orig.value)
  {
  }

  ~QueueTest() = default;

  QueueTest *next() const
  {
    return nxt;
  }

  QueueTest **nextPtr()
  {
    return &nxt;
  }

};

struct QueueTestComp
{
public:
  bool operator() (QueueTest const &a, QueueTest const &b) const
  {
    return a.value < b.value;
  }
};

static bool testLinkedQueueBasics()
{
  LinkedQueue<QueueTest> testq;

  // Basics
  assertTrue_1(testq.empty());
  assertTrue_1(testq.size() == 0);
  assertTrue_1(testq.front() == nullptr);

  // Pop empty queue should be safe
  testq.pop();
  assertTrue_1(testq.empty());
  assertTrue_1(testq.size() == 0);
  assertTrue_1(testq.front() == nullptr);

  // Remove of nullptr should be safe
  testq.remove(nullptr);
  assertTrue_1(testq.empty());
  assertTrue_1(testq.size() == 0);
  assertTrue_1(testq.front() == nullptr);

  {
    QueueTest *won = new QueueTest(1);
    assertTrue_1(won->value == 1);

    // Test remove of item not on (empty) queue
    testq.remove(won);
    assertTrue_1(testq.empty());
    assertTrue_1(testq.size() == 0);
    assertTrue_1(testq.front() == nullptr);
    
    // Test push on empty queue
    testq.push(won);
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.size() == 1);
    assertTrue_1(testq.front() != nullptr);
    assertTrue_1(testq.front()->value == 1);
    assertTrue_1(testq.front()->next() == nullptr);

    // Test pop
    testq.pop();
    assertTrue_1(testq.empty());
    assertTrue_1(testq.size() == 0);
    assertTrue_1(testq.front() == nullptr);

    // Push again
    testq.push(won);
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.size() == 1);
    assertTrue_1(testq.front() != nullptr);
    assertTrue_1(testq.front()->value == 1);
    assertTrue_1(testq.front()->next() == nullptr);

    // Remove of nullptr should be safe
    testq.remove(nullptr);
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.size() == 1);
    assertTrue_1(testq.front()->value == 1);

    // Test removal of only item
    testq.remove(won);
    assertTrue_1(testq.empty());
    assertTrue_1(testq.size() == 0);
    assertTrue_1(testq.front() == nullptr);

    // Pop empty queue should still be safe
    testq.pop();
    assertTrue_1(testq.empty());
    assertTrue_1(testq.size() == 0);
    assertTrue_1(testq.front() == nullptr);

    delete won;
  }

  // Push a bunch of items
  int n = 10;
  for (int i = 1; i <= n; ++i) {
    testq.push(new QueueTest(i));
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.size() == (size_t) i);
    assertTrue_1(testq.front() != nullptr);
  }

  // Pop and delete all the items
  while (!testq.empty()) {
    QueueTest *item = testq.front();
    testq.pop();
    delete item;
  }
  
  // Push a bunch of items again
  for (int i = 1; i <= n; ++i) {
    testq.push(new QueueTest(i));
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.size() == (size_t) i);
    assertTrue_1(testq.front() != nullptr);
  }

  // Step through the queue
  QueueTest *item = testq.front();
  for (int i = 1; i <= n; ++i) {
    assertTrue_1(item != nullptr);
    assertTrue_1(item->value == i);
    item = item->next();
  }
  // Should have reached end
  assertTrue_1(item == nullptr);

  // Remove first
  item = testq.front();
  assertTrue_1(item != nullptr);
  assertTrue_1(item->value == 1);
  QueueTest *nxt = item->next();
  assertTrue_1(nxt != nullptr);
  testq.remove(item);
  assertTrue_1(!testq.empty());
  assertTrue_1(testq.size() == (size_t) n - 1);
  assertTrue_1(testq.front() == nxt);
  delete item;

  // Remove from middle
  item = testq.front()->next()->next()->next();
  assertTrue_1(item != nullptr);
  testq.remove(item);
  assertTrue_1(!testq.empty());
  assertTrue_1(testq.size() == (size_t) n - 2);
  delete item;

  // Remove from end
  item = testq.front();
  while (item->next())
    item = item->next();
  // item now points to last
  testq.remove(item);
  assertTrue_1(!testq.empty());
  assertTrue_1(testq.size() == (size_t) n - 3);
  delete item;

  // Try to "remove" an item not in queue
  item = new QueueTest(42);
  testq.remove(item);
  assertTrue_1(!testq.empty());
  assertTrue_1(testq.size() == (size_t) n - 3);
  delete item;

  // Pop and delete remaining
  while (!testq.empty()) {
    assertTrue_1(testq.front() != nullptr);
    item = testq.front();
    testq.pop();
    delete item;
  }
  item = nullptr;

  assertTrue_1(testq.empty());
  assertTrue_1(testq.size() == 0);
  assertTrue_1(testq.front() == nullptr);

  return true;
}

static bool testLinkedQueueFindIf()
{
  LinkedQueue<QueueTest> testq;
  int const n = 10;

  // Populate queue
  for (int i = 1; i <= n; ++i) {
    testq.push(new QueueTest(i));
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.front() != nullptr);
    assertTrue_1(testq.size() == (size_t) i);
  }

  // Try finding first item
  QueueTest *item =
    testq.find_if([](QueueTest const *it) {return it->value == 1;});
  assertTrue_1(item);
  assertTrue_1(item->value == 1);

  // Try finding last
  item = testq.find_if([](QueueTest const *it) {return it->value == 10;});
  assertTrue_1(item);
  assertTrue_1(item->value == 10);

  // Middle
  item = testq.find_if([](QueueTest const *it) {return it->value == 6;});
  assertTrue_1(item);
  assertTrue_1(item->value == 6);

  // Nonexistent
  item = testq.find_if([](QueueTest const *it) {return it->value == 42;});
  assertTrue_1(!item);

  // Check integrity of queue
  // Value should always be increasing
  int countdown = testq.size();
  int countup = 0;
  while ((item = testq.front())) {
    assertTrue_1(item->value > countup);
    countup = item->value;
    testq.pop();
    --countdown;
    delete item;
  }
  assertTrue_1(!countdown);
  assertTrue_1(testq.empty());

  return true;
}

static bool testLinkedQueueRemoveIf()
{
  LinkedQueue<QueueTest> testq;
  int const n = 10;

  // Populate queue
  for (int i = 1; i <= n; ++i) {
    testq.push(new QueueTest(i));
    assertTrue_1(!testq.empty());
    assertTrue_1(testq.front() != nullptr);
    assertTrue_1(testq.size() == (size_t) i);
  }

  // Try removing from front
  QueueTest *temp = testq.front();
  assertTrue_1(temp);
  QueueTest *item =
    testq.remove_if([](QueueTest const *it) {return it->value == 1;});
  assertTrue_1(item);
  assertTrue_1(item->value == 1);
  assertTrue_1(item == temp);
  assertTrue_1(!testq.empty());
  assertTrue_1(testq.size() == (size_t) n - 1);
  assertTrue_1(testq.front()->value == 2);
  delete item;

  // Try removing from back
  temp = testq.find_if([](QueueTest const *it) {return it->value == 10;});
  assertTrue_1(temp);
  item = testq.remove_if([](QueueTest const *it) {return it->value == 10;});
  assertTrue_1(item);
  assertTrue_1(item->value == 10);
  assertTrue_1(item == temp);
  assertTrue_1(!testq.empty());
  assertTrue_1(testq.size() == (size_t) n - 2);
  assertTrue_1(testq.front()->value == 2);
  delete item;

  // From middle
  temp = testq.find_if([](QueueTest const *it) {return it->value == 6;});
  assertTrue_1(temp);
  item = testq.remove_if([](QueueTest const *it) {return it->value == 6;});
  assertTrue_1(item);
  assertTrue_1(item->value == 6);
  assertTrue_1(item == temp);
  assertTrue_1(!testq.empty());
  assertTrue_1(testq.size() == (size_t) n - 3);
  assertTrue_1(testq.front()->value == 2);
  delete item;

  // Check integrity of rest of queue
  // Value should always be increasing
  int countdown = testq.size();
  int countup = 0;
  while ((item = testq.front())) {
    assertTrue_1(item->value > countup);
    countup = item->value;
    testq.pop();
    --countdown;
    delete item;
  }
  assertTrue_1(!countdown);
  assertTrue_1(testq.empty());

  return true;
}

static bool testPriorityQueue()
{
  PriorityQueue<QueueTest, QueueTestComp> testpq;

  // Basics
  assertTrue_1(testpq.empty());
  assertTrue_1(testpq.size() == 0);
  assertTrue_1(testpq.front() == nullptr);
  assertTrue_1(testpq.front_count() == 0);

  // Pop empty queue should be safe
  testpq.pop();
  assertTrue_1(testpq.empty());
  assertTrue_1(testpq.size() == 0);
  assertTrue_1(testpq.front() == nullptr);
  assertTrue_1(testpq.front_count() == 0);

  // Remove of nullptr should be safe
  testpq.remove(nullptr);
  assertTrue_1(testpq.empty());
  assertTrue_1(testpq.size() == 0);
  assertTrue_1(testpq.front() == nullptr);
  assertTrue_1(testpq.front_count() == 0);

  {
    QueueTest *won = new QueueTest(1);
    assertTrue_1(won->value == 1);

    // Test remove of item not on (empty) queue
    testpq.remove(won);
    assertTrue_1(testpq.empty());
    assertTrue_1(testpq.size() == 0);
    assertTrue_1(testpq.front() == nullptr);
    assertTrue_1(testpq.front_count() == 0);
    
    // Test insert on empty queue
    testpq.insert(won);
    assertTrue_1(!testpq.empty());
    assertTrue_1(testpq.size() == 1);
    assertTrue_1(testpq.front() != nullptr);
    assertTrue_1(testpq.front()->value == 1);
    assertTrue_1(testpq.front()->next() == nullptr);
    assertTrue_1(testpq.front_count() == 1);

    // Test pop
    testpq.pop();
    assertTrue_1(testpq.empty());
    assertTrue_1(testpq.size() == 0);
    assertTrue_1(testpq.front() == nullptr);
    assertTrue_1(testpq.front_count() == 0);

    // Insert again
    testpq.insert(won);
    assertTrue_1(!testpq.empty());
    assertTrue_1(testpq.size() == 1);
    assertTrue_1(testpq.front() != nullptr);
    assertTrue_1(testpq.front()->value == 1);
    assertTrue_1(testpq.front()->next() == nullptr);
    assertTrue_1(testpq.front_count() == 1);

    // Remove of nullptr should be safe
    testpq.remove(nullptr);
    assertTrue_1(!testpq.empty());
    assertTrue_1(testpq.size() == 1);
    assertTrue_1(testpq.front()->value == 1);
    assertTrue_1(testpq.front_count() == 1);

    // Test removal of only item
    testpq.remove(won);
    assertTrue_1(testpq.empty());
    assertTrue_1(testpq.size() == 0);
    assertTrue_1(testpq.front() == nullptr);
    assertTrue_1(testpq.front_count() == 0);

    // Pop empty queue should still be safe
    testpq.pop();
    assertTrue_1(testpq.empty());
    assertTrue_1(testpq.size() == 0);
    assertTrue_1(testpq.front() == nullptr);
    assertTrue_1(testpq.front_count() == 0);

    delete won;
  }

  // Insert a bunch of items in order
  int const n = 10; // should be even, see below
  for (int i = 1; i <= n; ++i) {
    testpq.insert(new QueueTest(i));
    assertTrue_1(!testpq.empty());
    assertTrue_1(testpq.size() == (size_t) i);
    assertTrue_1(testpq.front() != nullptr);
    assertTrue_1(testpq.front_count() == 1);
  }

  // Pop and delete all the items,
  // making sure we pop as many as we inserted
  // and all are in order
  int count = 0;
  while (!testpq.empty()) {
    QueueTest *item = testpq.front();
    assertTrue_1(item);
    assertTrue_1(testpq.front_count() == 1);
    ++count;
    assertTrue_1(item->value == count);
    testpq.pop();
    delete item;
  }
  assertTrue_1(count == n);
  assertTrue_1(testpq.front_count() == 0);

  // Insert a bunch of items in reverse order
  for (int i = 1; i <= n; ++i) {
    testpq.insert(new QueueTest(n + 1 - i));
    assertTrue_1(!testpq.empty());
    assertTrue_1(testpq.size() == (size_t) i);
    assertTrue_1(testpq.front() != nullptr);
    assertTrue_1(testpq.front_count() == 1);
  }

  // Pop and delete all the items,
  // making sure we pop as many as we inserted
  // and all are in order
  count = 0;
  while (!testpq.empty()) {
    QueueTest *item = testpq.front();
    assertTrue_1(item);
    assertTrue_1(testpq.front_count() == 1);
    ++count;
    assertTrue_1(item->value == count);
    testpq.pop();
    delete item;
  }
  assertTrue_1(count == n);
  assertTrue_1(testpq.front_count() == 0);

  // Insert a bunch of items again, but in a mixed-up order
  for (int i = 1; i <= n/2; ++i) {
    testpq.insert(new QueueTest(n + 1 - i));
    testpq.insert(new QueueTest(n/2 + 1 - i));
    assertTrue_1(!testpq.empty());
    assertTrue_1(testpq.size() == (size_t) (2 * i));
    assertTrue_1(testpq.front() != nullptr);
    assertTrue_1(testpq.front_count() == 1);
  }

  // Step through the queue,
  // checking that they are now in increasing order
  QueueTest *item = testpq.front();
  for (int i = 1; i <= n; ++i) {
    assertTrue_1(item != nullptr);
    assertTrue_1(item->value == i);
    item = item->next();
  }
  // Should have reached end
  assertTrue_1(item == nullptr);

  // Remove first
  item = testpq.front();
  assertTrue_1(item != nullptr);
  assertTrue_1(item->value == 1);
  QueueTest *temp = item->next();
  assertTrue_1(temp != nullptr);
  testpq.remove(item);
  assertTrue_1(!testpq.empty());
  assertTrue_1(testpq.size() == n - 1);
  assertTrue_1(testpq.front() == temp);
  assertTrue_1(testpq.front_count() == 1);

  // Reinsert and check that it winds up in front
  testpq.insert(item);
  assertTrue_1(!testpq.empty());
  assertTrue_1(testpq.size() == n);
  assertTrue_1(item == testpq.front());
  assertTrue_1(testpq.front_count() == 1);

  // Remove from middle
  item = testpq.front()->next()->next()->next();
  assertTrue_1(item != nullptr);
  testpq.remove(item);
  assertTrue_1(!testpq.empty());
  assertTrue_1(testpq.size() == n - 1);
  assertTrue_1(testpq.front_count() == 1);

  // Reinsert and check that it winds up where it was
  testpq.insert(item);
  assertTrue_1(!testpq.empty());
  assertTrue_1(testpq.size() == n);
  assertTrue_1(item == testpq.front()->next()->next()->next());
  assertTrue_1(testpq.front_count() == 1);

  // Remove from end
  item = testpq.find_if([](QueueTest const *t) { return t->next() == nullptr; });
  assertTrue_1(item);
  testpq.remove(item);
  assertTrue_1(!testpq.empty());
  assertTrue_1(testpq.size() == n - 1);
  assertTrue_1(testpq.front_count() == 1);

  // Insert it back and check it winds up at the end
  testpq.insert(item);
  assertTrue_1(!testpq.empty());
  assertTrue_1(testpq.size() == n);
  assertTrue_1(testpq.front_count() == 1);
  temp = testpq.find_if([](QueueTest const *t) { return t->next() == nullptr; });
  assertTrue_1(temp == item);

  // Try to "remove" a nonexistent item
  item = new QueueTest(42);
  testpq.remove(item);
  assertTrue_1(!testpq.empty());
  assertTrue_1(testpq.size() == n);
  assertTrue_1(testpq.front_count() == 1);
  delete item;

  // Insert duplicate items and check they wind up behind originals
  item = testpq.front();
  temp = new QueueTest(*item);
  testpq.insert(temp);
  assertTrue_1(!testpq.empty());
  assertTrue_1(testpq.size() == n + 1);
  assertTrue_1(item->next() == temp);
  assertTrue_1(testpq.front_count() == 2);

  testpq.remove(temp);
  assertTrue_1(testpq.size() == n);
  assertTrue_1(testpq.front_count() == 1);
  item = testpq.find_if([](QueueTest const *it) {return it->value == 6;});
  assertTrue_1(item->value == 6);
  assertTrue_1(testpq.front_count() == 1);
  temp->value = 6;
  testpq.insert(temp);
  assertTrue_1(testpq.size() == n + 1);
  assertTrue_1(item->next() == temp);
  assertTrue_1(testpq.front_count() == 1);
  testpq.remove(temp);
  assertTrue_1(testpq.size() == n);
  assertTrue_1(item->next() != temp);
  assertTrue_1(testpq.front_count() == 1);

  temp = nullptr;

  // Pop and delete remaining
  while (!testpq.empty()) {
    assertTrue_1(testpq.front() != nullptr);
    assertTrue_1(testpq.front_count() == 1);
    item = testpq.front();
    testpq.pop();
    delete item;
  }
  item = nullptr;

  assertTrue_1(testpq.empty());
  assertTrue_1(testpq.size() == 0);
  assertTrue_1(testpq.front() == nullptr);
  assertTrue_1(testpq.front_count() == 0);

  return true;
}

bool LinkedQueueTest()
{
  Error::doThrowExceptions();

  runTest(testLinkedQueueBasics);
  runTest(testLinkedQueueFindIf);
  runTest(testLinkedQueueRemoveIf);
  runTest(testPriorityQueue);
  return true;
}
