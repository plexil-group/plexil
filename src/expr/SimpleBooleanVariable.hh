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

#ifndef PLEXIL_SIMPLE_BOOLEAN_VARIABLE_HH
#define PLEXIL_SIMPLE_BOOLEAN_VARIABLE_HH

#include "GetValueImpl.hh"
#include "Notifier.hh"

namespace PLEXIL {

  /**
   * @class SimpleBooleanVariable
   * @brief A Boolean variable tailored to the requirements of internal flags.
   */

  class SimpleBooleanVariable final :
    public GetValueImpl<Boolean>,
    public Notifier
  {
  public:
    SimpleBooleanVariable();
    // N.B. Name is owned by some other code (usually a literal value).
    SimpleBooleanVariable(char const *name);
    ~SimpleBooleanVariable();

    // Listenable API
    virtual bool isPropagationSource() const override;

    //
    // Essential Expression API
    //

    virtual char const *getName() const override;
    virtual char const *exprName() const override;

    virtual bool isKnown() const override;

    //
    // Assignable and AssignableImpl API
    //

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    virtual bool getValue(Boolean &result) const override;

    /**
     * @brief Set the value for this object.
     * @param val The new value for this object.
     */
    void setValue(Boolean const &val);

    /**
     * @brief Set the value for this object.
     * @param val The new value for this object.
     */
    // virtual void setValue(Value const &val) override;

  protected:

    //
    // Expression internal API
    //
    virtual void printSpecialized(std::ostream &s) const override;

    //
    // Notifier API
    //

    virtual void handleActivate() override;

  private:

    // Copy constructor, assign, moves prohibited
    SimpleBooleanVariable(SimpleBooleanVariable const &) = delete;
    SimpleBooleanVariable(SimpleBooleanVariable &&) = delete;
    SimpleBooleanVariable &operator=(SimpleBooleanVariable const &) = delete;
    SimpleBooleanVariable &operator=(SimpleBooleanVariable &&) = delete;

    char const *m_name;
    bool m_value;
  };

} // namespace PLEXIL

#endif // PLEXIL_SIMPLE_BOOLEAN_VARIABLE_HH
