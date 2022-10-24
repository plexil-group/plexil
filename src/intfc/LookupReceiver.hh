// Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef LOOKUP_RECEIVER_HH
#define LOOKUP_RECEIVER_HH

//
// API for an object which receives data from a LookupNow
// implementation function.
//

#include "ValueType.hh"

namespace PLEXIL
{
  // Forward reference
  class Value;

  //! \class LookupReceiver
  //! \brief Stateless abstract base class representing a callback
  //!        object passed to an interface implementation.
  //! \see StateCacheEntry
  class LookupReceiver
  {
  public:
    //! \brief Virtual destructor.
    virtual ~LookupReceiver() = default;

    //! \brief Assign a new value to this Lookup.
    //! \param val Const reference to the new Value.
    virtual void update(Value const &val) = 0;

    //! \brief Make the value of this Lookup unknown.
    virtual void setUnknown() = 0;

    //! \brief Assign a new value of a specific type to this Lookup.
    //! \param val The new value.
    ///@{
    virtual void update(Boolean val) = 0;
    virtual void update(Integer val) = 0;
    virtual void update(Real val) = 0;
    ///@}

    //! \brief Assign a new string value to this Lookup.
    //! \param val The new value.
    ///@{
    virtual void update(String const &) = 0;
    virtual void update(char const *) = 0;
    ///@}

    //! \brief Assign a new array value to this Lookup.
    //! \param ary The array.
    //! \param size The number of elements in the array.
    ///@{
    virtual void update(Boolean const ary[], size_t size) = 0;
    virtual void update(Integer const ary[], size_t size) = 0;
    virtual void update(Real const ary[], size_t size) = 0;
    virtual void update(String const ary[], size_t size) = 0;
    ///@}
  };

}

#endif // LOOKUP_RECEIVER_HH
