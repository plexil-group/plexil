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

//
// An object which receives data from a LookupNow implementation function
//

#ifndef LOOKUP_RECEIVER_HH
#define LOOKUP_RECEIVER_HH

#include "ValueType.hh"

namespace PLEXIL
{
  // Forward reference
  class Value;

  //!
  // @class LookupReceiver
  // @brief A callback object passed to an interface implementation.
  //

  class LookupReceiver
  {
  public:
    virtual ~LookupReceiver() = default;

    // PLEXIL internal representation
    virtual void update(Value const &) = 0;

    virtual void setUnknown() = 0;

    // Convenience overloads
    virtual void update(Boolean) = 0;
    virtual void update(Integer) = 0;
    virtual void update(Real) = 0;

    virtual void update(String const &) = 0;
    // virtual void update(String &&) = 0; // ?
    virtual void update(char const *) = 0;

    virtual void update(Boolean const ary[], size_t size) = 0;
    virtual void update(Integer const ary[], size_t size) = 0;
    virtual void update(Real const ary[], size_t size) = 0;
    virtual void update(String const ary[], size_t size) = 0;

  protected:
    // Default constructor only available to implementation class
    LookupReceiver() = default;

  private:
    // Copy disallowed
    LookupReceiver(LookupReceiver const &) = default;
    LookupReceiver(LookupReceiver &&) = default;

    // Assignment disallowed
    LookupReceiver &operator=(LookupReceiver const &) = default;
    LookupReceiver &operator=(LookupReceiver &&) = default;
  };

}

#endif // LOOKUP_RECEIVER_HH
