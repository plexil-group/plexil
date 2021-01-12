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

#include "DebugMessage.hh"
#include "InterfaceError.hh"
#include "ThreadSemaphore.hh"
#include "TimebaseFactory.hh"

#include <fstream>
#include <iomanip> // std::fixed, std::setprecision()
#include <memory>

#if defined(HAVE_CMATH)
#include <cmath>  // fabs()
#elif defined(HAVE_MATH_H)
#include <math.h> // fabs()
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h> // sleep()
#endif

using namespace PLEXIL;

// Local constants
#ifndef USEC_PER_SEC
#define USEC_PER_SEC (1000000)
#endif

// Comparisons between doubles
static double const EPSILON = 1e-12;
static bool eq_within_epsilon(double a, double b)
{
  return fabs(a - b) < (fabs(a) * EPSILON);
}

static bool geq_within_epsilon(double a, double b)
{
  return (a >= b) || (b - a) < (fabs(a) * EPSILON);
}

// Simple wakeup function
// Posts to a semaphore pointed to by its argument
static void wakeup(void *arg)
{
  assertTrue_1(arg != 0);
  ThreadSemaphore *mySem = reinterpret_cast<ThreadSemaphore *>(arg);
  mySem->post();
}

static bool testGetTime(std::string const &name)
{
  std::cout << "testGetTime: Testing " << name << std::endl;
  // Ensure queryTime() returns 0 when no timebase exists
  assertTrue_1(0 == Timebase::queryTime());
  try {
    ThreadSemaphore testSem;
    std::unique_ptr<Timebase> tb(TimebaseFactory::get(name)->create(wakeup, (void *) &testSem));

    // Check that getTime() method works
    double first_time = tb->getTime();
    assertTrue_1(0 != first_time);
    sleep(1);
    double second_time = tb->getTime();
    assertTrue_1(0 != second_time);
    assertTrue_1(second_time - first_time >= 1.0);
    
    // Check that queryTime() works
    first_time = Timebase::queryTime();
    assertTrue_1(0 != first_time);
    sleep(1);
    second_time = Timebase::queryTime();
    assertTrue_1(0 != second_time);
    assertTrue_1(second_time - first_time >= 1.0);

    // Ensure queryTime() returns 0 when the previous timebase has been deleted
    tb.reset();
    assertTrue_1(0 == Timebase::queryTime());

    std::cout << "testGetTime: " << name << " passed\n" << std::endl;
    return true;
  } catch (Error const &e) {
    std::cerr << "*** Test error: " << e.what() << std::endl;
  }

  std::cout << "\ntestGetTime: " << name << " failed\n" << std::endl;
  return false;
}

static bool testTimebaseDeadlines(std::string const &name)
{
  std::cout << "testTimebaseDeadlines: Testing " << name << std::endl;
  try {
    ThreadSemaphore testSem;
    std::unique_ptr<Timebase> tb(TimebaseFactory::get(name)->create(wakeup, (void *)  &testSem));
    assertTrue_1(tb->getTickInterval() == 0);
    assertTrue_1(tb->getNextWakeup() == 0);

    // Test 
    tb->start();
    double startTime = tb->getTime();
    double scheduledTime = startTime + 2.0;
    tb->setTimer(scheduledTime);

    std::cout << "\nTimer set to "
              << std::fixed << std::setprecision(6) << scheduledTime
              << ", getNextWakeup() returns " << tb->getNextWakeup()
              << std::endl;
    assertTrue_1(eq_within_epsilon(tb->getNextWakeup(), scheduledTime));

    // Wait for wakeup 
    testSem.wait();
    double actualTime = tb->getTime();

    std::cout << "\nWakeup scheduled for "
              << std::fixed << std::setprecision(6) << scheduledTime
              << ", received at " << actualTime
              << ",\n was " << actualTime - scheduledTime
              << " seconds late" << std::endl;

    // Should be strictly >=, but macOS can wake up early
    assertTrue_1(geq_within_epsilon(actualTime,  scheduledTime));

    // more todo
    tb->stop();

    std::cout << "testTimebaseDeadlines: " << name << " passed\n" << std::endl;
    return true;
  } catch (Error const &e) {
    std::cerr << "*** Test error: " << e.what() << std::endl;
  }

  std::cout << "\ntestTimebaseDeadlines: " << name << " failed\n" << std::endl;
  return false;
}

static bool testTimebaseTick(std::string const &name)
{
  std::cout << "testTimebaseTick: Testing " << name << std::endl;
  try {
    ThreadSemaphore testSem;
    std::unique_ptr<Timebase> tb(TimebaseFactory::get(name)->create(wakeup, (void *) &testSem));
    assertTrue_1(tb->getTickInterval() == 0);
    assertTrue_1(tb->getNextWakeup() == 0);

    tb->setTickInterval(USEC_PER_SEC);
    assertTrue_1(tb->getTickInterval() == USEC_PER_SEC);

    // Test tick wakeups
    double startTime = tb->getTime();
    double endTime;
    tb->start();
    std::cout << "\nStart at "
              << std::fixed << std::setprecision(6) << startTime
              << '\n' << std::endl;

    for (int i = 0 ; i < 5; ++i) {
      // Wait for wakeup 
      testSem.wait();
      endTime = tb->getTime();
      std::cout << "Tick at "
                << std::fixed << std::setprecision(6) << endTime
                << std::endl;
    }
    tb->stop();

    std::cout << "\nStarted at "
              << std::fixed << std::setprecision(6) << startTime
              << ", ended at " << endTime
              << ",\n difference was " << endTime - startTime
              << " seconds" << std::endl;

    // Should be strictly >=, but macOS can wake up early
    assertTrue_1(geq_within_epsilon(endTime, startTime + 5.0));
    // Shouldn't be a whole tick late though.
    assertTrue_1(endTime < startTime + 6.0);

    std::cout << "\ntestTimebaseTick: " << name << " passed\n" << std::endl;
    return true;
  } catch (Error const &e) {
    std::cerr << "*** Test error: " << e.what() << std::endl;
  }

  std::cout << "\n testTimebaseTick: " << name << " failed\n" << std::endl;
  return false;
}

int main(int argc, char *argv[])
{
  // Read Debug.cfg in current directory, if it exists
  char debugConfig[] = "Debug.cfg";
  std::ifstream config(debugConfig);
  if (config.good()) {
    PLEXIL::readDebugConfigStream(config);
    std::cout << "Read debug configuration file " << debugConfig << std::endl;
  }
  else {
    std::cout << "Can't open debug configuration file " << debugConfig
              << ", continuing." << std::endl;
  }

  InterfaceError::doThrowExceptions();

  initTimebaseFactories();

  bool success = true;

  std::vector<std::string> timebaseNames = TimebaseFactory::allFactoryNames();

  std::cout << "Testing getTime() and queryTime()" << std::endl;
  for (std::string const &name : timebaseNames) {
    success = success && testGetTime(name);
  }

  std::cout << "Testing deadline timers" << std::endl;
  for (std::string const &name : timebaseNames) {
    success = success && testTimebaseDeadlines(name);
  }

  std::cout << "Testing tick timers" << std::endl;
  for (std::string const &name : timebaseNames) {
    success = success && testTimebaseTick(name);
  }

  std::cout << "Timebase test " << (success ? "succeeded" : "failed") << std::endl;
  return (success ? 0 : 1);
}
