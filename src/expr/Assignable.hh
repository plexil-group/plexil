// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
// All rights reserved.
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

#ifndef PLEXIL_ASSIGNABLE_HH
#define PLEXIL_ASSIGNABLE_HH

#include "Expression.hh"

namespace PLEXIL
{

  // Forward declaration
  class Variable;

  //! \class Assignable
  //! \brief Pure virtual mixin class for all expressions which can be assigned to by a plan.
  //! \note Examples include variables, array references used as assignment targets, etc.
  //! \ingroup Expressions

  class Assignable : virtual public Expression
  {
  public:

    //
    // Core Assignable API
    // Every Assignable must implement these behaviors
    //

    //! \brief Temporarily store the current value of this variable.
    //! \note Used to implement recovery from failed Assignment nodes.
    virtual void saveCurrentValue() = 0;

    //! \brief Restore the value set aside by saveCurrentValue().
    //! \note Used to implement recovery from failed Assignment nodes.
    virtual void restoreSavedValue() = 0;

    //! \brief Read the saved value of this variable.
    //! \return The saved value.
    virtual Value getSavedValue() const = 0;

    //! \brief Get the real variable for which this may be a proxy.
    //! \return Pointer to the base variable.
    //! \note Used by the assignment node conflict resolution logic.
    virtual Variable *getBaseVariable() = 0;

    //! \brief Get the real variable for which this may be a proxy.
    //! \return Const pointer to the base variable.
    //! \note Used by the assignment node conflict resolution logic.
    virtual Variable const *getBaseVariable() const = 0;

    //
    // SetValue API subset actually used in Exec and test suites
    //

    //! \brief Set the value of this expression to unknown.
    virtual void setUnknown() = 0;

    //! \brief Set the value for this object from a Value instance.
    //! \param val Const reference to the new value.
    virtual void setValue(Value const &val) = 0;

    //
    // Overrides to Expression member functions
    //

    //! \brief Query whether this expression is assignable in the %PLEXIL language.
    //! \return True if assignable, false otherwise.
    virtual bool isAssignable() const
    {
      return true;
    }

    //! \brief Get a pointer to this expression as an instance of Assignable.
    //! \return The pointer. Will be NULL if the expression does not permit assignment.
    virtual Assignable *asAssignable()
    {
      return this;
    }

    //! \brief Get a const pointer to this expression as an instance of Assignable.
    //! \return The pointer. Will be NULL if not the expression does not permit assignment.
    virtual Assignable const *asAssignable() const
    {
      return this;
    }

  };

} // namespace PLEXIL

#endif // PLEXIL_ASSIGNABLE_HH
