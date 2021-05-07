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

#ifndef PLEXIL_MUTEX_HH
#define PLEXIL_MUTEX_HH

#include "Reservable.hh"

#include <iosfwd>
#include <string>
#include <vector>

namespace PLEXIL
{
  //
  // Forward references
  //

  class Mutex final : public Reservable
  {
  public:

    //! Public constructor.
    //! @param name Name of the mutex.
    Mutex(char const *name);

    //! Virtual destructor.
    virtual ~Mutex() = default;

    //! Get the name of this mutex.
    //! @return Const reference to the name string.
    std::string const &getName() const;

    //! Print the mutex's name and state.
    //! @param stream Stream to print to.
    //! @param indent Number of spaces to indent from the left margin.
    void print(std::ostream &stream, const unsigned int indent = 0) const;

  private:

    // Not implemented
    Mutex() = delete;
    Mutex(Mutex const &) = delete;
    Mutex(Mutex &&) = delete;
    Mutex &operator=(Mutex const &) = delete;
    Mutex &operator=(Mutex &&) = delete;

    std::string const m_name;
  };

  std::ostream& operator<<(std::ostream &stream, Mutex const &m);

  /**
   * @brief Find the named global Mutex, if it exists.
   * @param name The name of the mutex.
   * @return Pointer to the named Mutex, or nullptr.
   */

  Mutex *getGlobalMutex(char const *name);

  /**
   * @brief Find the named global Mutex. If it does not exist, create it.
   * @param name The name of the mutex.
   * @return Pointer to the named Mutex.
   * @note Should never return null.
   */

  Mutex *ensureGlobalMutex(char const *name);

}

#endif // PLEXIL_MUTEX_HH
