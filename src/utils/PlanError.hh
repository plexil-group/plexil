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

#ifndef PLEXIL_PLAN_ERROR_HH
#define PLEXIL_PLAN_ERROR_HH

#include "Error.hh"

namespace PLEXIL
{

  struct PlanError : public Error
  {
  public:

    /**
       @brief Build a PlanError object from the information given, including an extra message.
    */
    PlanError(const std::string& condition,
              const std::string& msg,
              const std::string& file,
              const int& line);
    
    PlanError(PlanError const &orig) = default;
    PlanError(PlanError &&orig) = default;

    virtual ~PlanError() PLEXIL_NOEXCEPT = default;

    PlanError &operator=(PlanError const &other) = default;
    PlanError &operator=(PlanError &&other) = default;

    bool operator==(const PlanError &other);

    /**
     * Report and throw the exception, or assert.
     */
    PLEXIL_NORETURN void report();

    /**
     * Indicate that errors should throw exceptions rather than
     * complaining and aborting.
     */
    static void doThrowExceptions();

    /**
     * Indicate that errors should complain and abort rather than throw
     * exceptions.
     */
    static void doNotThrowExceptions();

    /**
     * Are errors set to throw exceptions?
     * @return true if so; false if errors will complain and abort.
     */
    static bool throwEnabled();

  private:

    PlanError(); // not implemented

  };

} // namespace PLEXIL


/**
 * @def reportPlanError
 * @brief Unconditionally create an error message.
 * @param msg Anything suitable as the right-hand side of <<.
 */
#define reportPlanError(msg) { \
  std::ostringstream sstr; \
  sstr << msg; \
  PLEXIL::PlanError("", sstr.str(), __FILE__, __LINE__).report(); \
}

/**
 * @def checkPlanError
 * @brief Test a condition and create an error if false.
 * @param cond Expression that yields a true/false result.
 * @param msg Anything suitable as the right-hand side of <<.
 */
#define checkPlanError(cond, msg) { \
  if (!(cond)) { \
    std::ostringstream sstr; \
    sstr << msg; \
    PLEXIL::PlanError(#cond, sstr.str(), __FILE__, __LINE__).report(); \
  } \
}

#endif // PLEXIL_PLAN_ERROR_HH
