/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "Mutable.hh"

namespace PLEXIL {

  /**
   * @class Assignable
   * @brief Base class for all expressions which can be assigned to by a plan.
   * @note Examples include variables, array references, aliases for InOut variables, etc.
   */
  class Assignable : public Mutable
  {
  public:

    /**
     * @brief Default constructor.
     */
    Assignable();

    /**
     * @brief Destructor.
     */
    virtual ~Assignable();

    /**
     * @brief Query whether this expression is assignable.
     * @return True if assignable, false otherwise.
     */
    bool isAssignable() const;

    //
    // Core Assignable API
    // Every Assignable must implement these behaviors
    //

    /**
     * @brief Set the value of this expression back to the initial value with which it was
     *        created.
     */
    virtual void reset() = 0;

    /**
     * @brief Set the value for this expression.  This may cause notifications to fire, which
     *        may in turn cause other Expressions to change value.
     * @param val The new value for this expression.
     * @note Each default method returns a type error.
     */
    virtual void setValue(const double& val);
    virtual void setValue(const int32_t& val);
    virtual void setValue(const uint16_t& val);
    virtual void setValue(const bool& val);
    virtual void setValue(const std::string& val);
    // TODO: Array types
    
    // FIXME
    /**
     * @brief Check to make sure a value is appropriate for this expression.
     * @param value The new value for this variable.
     * @note Should only be defined for the value type appropriate
     * to the expression.
     * @note Default method returns true.
     */
    virtual bool checkValue(const double& value);
    virtual bool checkValue(const int32_t& value);
    virtual bool checkValue(const uint16_t& value);
    virtual bool checkValue(const bool& value);
    virtual bool checkValue(const std::string& value);
    // TODO: Array types

    /**
     * @brief Temporarily stores the previous value of this variable.
     * @note Used to implement recovery from failed Assignment nodes.
     */
    virtual void saveCurrentValue() = 0;

    /**
     * @brief Restore the value set aside by saveCurrentValue().
     * @note Used to implement recovery from failed Assignment nodes.
     */
    virtual void restoreSavedValue() = 0;

    /**
     * @brief Get the name of this variable, as declared in the node that owns it.
     */
    virtual const std::string& getName() const = 0;

    /**
     * @brief Get the node that owns this expression.
     * @return The NodeId of the parent node; may be noId.
     * @note Used by LuvFormat::formatAssignment().  
     * @note Default method returns noId().
     */
    virtual const NodeId& getNode() const;

    /**
     * @brief Get the real variable for which this may be a proxy.
     * @return The ExpressionId of the base variable
     * @note Used by the assignment node conflict resolution logic.
     */
    virtual const ExpressionId& getBaseVariable() const = 0;

  };

} // namespace PLEXIL

#endif // PLEXIL_ASSIGNABLE_HH
