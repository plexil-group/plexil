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

#ifndef PLEXIL_ASSIGNABLE_HH
#define PLEXIL_ASSIGNABLE_HH

#include "Expression.hh"

#include <vector>

namespace PLEXIL {
  
  // Forward declarations
  class Variable;

  //! @class Assignable
  //! Pure virtual mixin class defining the API for all expressions
  //! which can be assigned to by a plan.
  //! @note In addition to ordinary variables, other expressions such
  //!       as array references, aliases for InOut variables, etc. also
  //!       implement the Assignable API.
  //! @note This class has no state of its own.
  class Assignable : virtual public Expression
  {
  public:

    //! Virtual destructor.
    virtual ~Assignable() = default;

    //
    // Core Assignable API
    // Every Assignable must implement these behaviors
    //

    //! Temporarily stores the previous value of this variable.
    //! @note Used to implement recovery from failed Assignment nodes.
    virtual void saveCurrentValue() = 0;

    //! Restore the value set aside by saveCurrentValue().
    //! @note Used to implement recovery from failed Assignment nodes.
    virtual void restoreSavedValue() = 0;

    //! Read the saved value of this variable.
    //! @return The saved value.
    //! @note Only used by Assignment::retract().
    virtual Value getSavedValue() const = 0;

    //
    // SetValue API subset actually used in Exec and test suites
    //

    //1 Set this expression's value unknown.
    virtual void setUnknown() = 0;

    //! Set the value of this expression.
    //! @param val The new value for this object.
    virtual void setValue(Value const &val) = 0;

    //
    // Interface to PlexilExec conflict resolution logic
    //

    //! @brief Get the real variable for which this expression may be a proxy.
    //! @return Pointer to the base variable.
    virtual Variable *getBaseVariable() = 0;
    virtual Variable const *getBaseVariable() const = 0;

  protected:

    //! Default constructor, only accessible to derived classes.
    Assignable() = default;

  private:

    // Not implemented
    Assignable(Assignable const &) = delete;
    Assignable(Assignable &&) = delete;
    Assignable &operator=(Assignable const &) = delete;
    Assignable &operator=(Assignable &&) = delete;
  };

} // namespace PLEXIL

#endif // PLEXIL_ASSIGNABLE_HH
