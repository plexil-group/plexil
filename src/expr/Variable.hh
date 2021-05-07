// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

//
// Essential behaviors of variables in PLEXIL
//

#ifndef PLEXIL_VARIABLE_HH
#define PLEXIL_VARIABLE_HH

#include "Assignable.hh"
#include "Reservable.hh"

namespace PLEXIL
{

  //! @class Variable
  //! Defines the API which any concrete variable in PLEXIL must implement.
  class Variable : public Assignable, public Reservable
  {
  public:

    //! Virtual destructor.
    virtual ~Variable() = default;

    //! @brief Get the real variable for which this expression may be a proxy.
    //! @return Pointer to the base variable.
    virtual Variable *getBaseVariable() override
    {
      return this;
    }
    
    virtual Variable const *getBaseVariable() const override
    {
      return const_cast<Variable const *>(this);
    }

    //! Set the expression from which this variable gets its initial value.
    //! @param expr Pointer to an Expression.
    //! @param garbage True if the expression should be deleted with this object, false otherwise.
    virtual void setInitializer(Expression *expr, bool garbage) = 0;

  protected:

    //! Default constructor, only accessible to derived classes
    Variable()
      : Assignable(),
        Reservable()
    {
    }

  }; // class Variable

} // namespace PLEXIL

#endif // PLEXIL_VARIABLE_HH
