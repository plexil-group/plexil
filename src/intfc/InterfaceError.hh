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

#ifndef PLEXIL_INTERFACE_ERROR_HH
#define PLEXIL_INTERFACE_ERROR_HH

#include "Error.hh"

namespace PLEXIL
{

  //! \class InterfaceError
  //! \brief Exception class for reporting errors in the external interface.
  //! \ingroup External-Interface
  class InterfaceError : public Error
  {
  public:

    //! \brief Constructor.  Builds an InterfaceError instance from
    //!        the information given, including an extra message.
    //! \param condition Text of the condition expression which failed.
    //! \param msg The message.
    //! \param file File name in which the error was detected.
    //! \param line The line number at which the error was detected.
    InterfaceError(const std::string& condition,
                   const std::string& msg,
                   const std::string& file,
                   const int& line);

    //! \brief Copy constructor.
    //! \param orig The instance being copied.
    InterfaceError(InterfaceError const &orig) = default;

    //! \brief Copy constructor.
    //! \param orig The instance being moved.
    InterfaceError(InterfaceError &&orig) = default;

    //! \brief Copy assignment operator.
    //! \param other The instance being copied.
    InterfaceError &operator=(InterfaceError const &other) = default;

    //! \brief Move assignment operator.
    //! \param other The instance being moved.
    InterfaceError &operator=(InterfaceError &&other);

    //! \brief Virtual destructor.
    virtual ~InterfaceError() PLEXIL_NOEXCEPT = default;

    //! \brief Equality comparison operator.
    //! \param other Const reference to the InterfaceError being compared.
    //! \return True if the two InterfaceError instances are equivalent, false otherwise.
    bool operator==(const InterfaceError &other);

    //! \brief Report this exception as specified by the throwEnabled() method.
    //! \see InterfaceError::throwEnabled
    //! \see InterfaceError::doThrowExceptions
    //! \see InterfaceError::doNotThrowExceptions
    PLEXIL_NORETURN void report();

    //! \brief Request that report() should throw exceptions.
    //! \see InterfaceError::report
    //! \see InterfaceError::throwEnabled
    static void doThrowExceptions();

    //! \brief Request that report() should call assert().
    //! \see InterfaceError::report
    //! \see InterfaceError::throwEnabled
    static void doNotThrowExceptions();

    //! \brief Should report() throw exceptions?
    //! \return true to throw exceptions, false to assert.
    //! \see InterfaceError::report
    static bool throwEnabled();

  private:

    InterfaceError() = delete; // not implemented

  };

} // namespace PLEXIL

//! \brief Unconditionally construct an InterfaceError and report it.
//! \param msg Anything suitable as the right-hand side of std::ostream::operator<<().
//! \ingroup External-Interface
#define reportInterfaceError(msg) { \
  std::ostringstream sstr; \
  sstr << msg; \
  PLEXIL::InterfaceError("", sstr.str(), __FILE__, __LINE__).report(); \
}

//! \brief Test a condition. If false, construct an InterfaceError and report it.
//! \param cond Expression that yields a true/false result.
//! \param msg Anything suitable as the right-hand side of std::ostream::operator<<().
//! \ingroup External-Interface
#define checkInterfaceError(cond, msg) { \
  if (!(cond)) { \
    std::ostringstream sstr; \
    sstr << msg; \
    PLEXIL::InterfaceError(#cond, sstr.str(), __FILE__, __LINE__).report(); \
  } \
}

#endif // PLEXIL_INTERFACE_ERROR_HH
