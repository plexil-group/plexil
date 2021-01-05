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
#include "Error.hh"
#include "ThreadSemaphore.hh"
#include "TimebaseFactory.hh"

#include <fstream>
#include <iomanip> // std::setprecision()
#include <iostream>

#if defined(HAVE_CMATH)
#include <cmath>  // fabs()
#elif defined(HAVE_MATH_H)
#include <math.h> // fabs()
#endif

using namespace PLEXIL;

ThreadSemaphore testSem;

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

static void wakeup(void *arg)
{
  assertTrue_1(arg != 0);
  ThreadSemaphore *mySem = reinterpret_cast<ThreadSemaphore *>(arg);
  mySem->post();
}

static bool testTimebaseDeadlines(std::string const &name)
{
  std::cout << "testTimebaseDeadlines: Testing " << name << std::endl;
  try {
    Timebase *tb = TimebaseFactory::get(name)->create(wakeup, (void *)  &testSem);
    assertTrue_1(tb->getTickInterval() == 0);
    assertTrue_1(tb->getNextWakeup() == 0);

    // Test 
    tb->start();
    double startTime = tb->getTime();
    double scheduledTime = startTime + 2.0;
    tb->setTimer(scheduledTime);
    // TEMP DEBUG (?)
    std::cout << "scheduledTime is " << std::setprecision(15) << scheduledTime
              << ", getNextWakeup() returns " << std::setprecision(15) << tb->getNextWakeup()
              << std::endl;
    assertTrue_1(eq_within_epsilon(tb->getNextWakeup(), scheduledTime));

    // Wait for wakeup 
    testSem.wait();
    double actualTime = tb->getTime();
    // TEMP DEBUG (?)
    std::cout << "Wakeup scheduled for " << std::setprecision(15) << scheduledTime
              << ", received at " << actualTime
              << ",\n was " << std::setprecision(6) << actualTime - scheduledTime
              << " seconds late" << std::endl;
    // Should be strictly >=, but macOS can wake up early
    assertTrue_1(geq_within_epsilon(actualTime,  scheduledTime));

    // more todo
    tb->stop();
    
    delete tb;

    std::cout << "testTimebaseDeadlines: " << name << " passed" << std::endl;
    return true;
  } catch (Error const &e) {
    std::cerr << "***  Error: " << e.what() << std::endl;
  }

  std::cout << "testTimebaseDeadlines: " << name << " failed" << std::endl;
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

  initTimebaseFactories();

  bool success = true;

  std::vector<std::string> timebaseNames = TimebaseFactory::allFactoryNames();
  for (std::string const &name : timebaseNames) {
    success = success && testTimebaseDeadlines(name);
  }

  std::cout << "Test " << (success ? "succeeded" : "failed") << std::endl;
  return (success ? 0 : 1);
}
