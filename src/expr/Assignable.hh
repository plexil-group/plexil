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

#include "NotifierImpl.hh"

namespace PLEXIL {
  
  // Forward declarations
  class Assignable;
  typedef Id<Assignable> AssignableId;

  /**
   * @class Assignable
   * @brief Mixin class for all expressions which can be assigned to by a plan.
   * @note Examples include variables, array references, aliases for InOut variables, etc.
   */
  class Assignable : public virtual Expression
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

    inline const AssignableId &getAssignableId() const
    {
      return m_aid;
    }

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
     * @brief Set the current value of this variable to "unknown".
     * @note May cause change notifications to occur.
     */
    virtual void setUnknown() = 0;

    /**
     * @brief Set the value for this expression.
     * @param val The new value for this expression.
     * @note May cause change notifications to occur.
     * @note Each default method reports a type error.
     * @deprecated These are being replaced with the setValue(ExpressionId const &) method below.
     */
    virtual void setValue(const double &val);
    virtual void setValue(const int32_t &val);
    virtual void setValue(const uint16_t &val);
    virtual void setValue(const bool &val);
    virtual void setValue(const std::string &val);
    virtual void setValue(const char *val);

    virtual void setValue(const std::vector<bool> &val);
    virtual void setValue(const std::vector<int32_t> &val);
    virtual void setValue(const std::vector<double> &val);
    virtual void setValue(const std::vector<std::string> &val);

    /**
     * @brief Set the value for this expression from another expression.
     * @param valex The expression from which to obtain the new value.
     * @note May cause change notifications to occur.
     */
    virtual void setValue(ExpressionId const &valex) = 0;

    /**
     * @brief Retrieve a writable ponter to the value.
     * @param valuePtr Reference to the pointer variable
     * @return True if the value is known, false if unknown or invalid.
     * @note Default method returns false and reports a type error.
     */
    virtual bool getMutableValuePointer(std::string *& ptr);

    /**
     * @brief Retrieve the (writable) value vector and known vector for array-valued expressions.
     * @param valuePtr Reference to the pointer variable to receive the value vector.
     * @param knownPtr Reference to the pointer variable to receive the known vector.
     * @return True if the value is known, false if unknown or invalid.
     * @note Each default method returns false and reports a type error.
     */
    virtual bool getMutableArrayContents(std::vector<bool> *&valuePtr,
                                         std::vector<bool> *&knownPtr);
    virtual bool getMutableArrayContents(std::vector<int32_t> *&valuePtr,
                                         std::vector<bool> *&knownPtr);
    virtual bool getMutableArrayContents(std::vector<double> *&valuePtr,
                                         std::vector<bool> *&knownPtr);
    virtual bool getMutableArrayContents(std::vector<std::string> *&valuePtr,
                                         std::vector<bool> *&knownPtr);

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
     * @return The AssignableId of the base variable
     * @note Used by the assignment node conflict resolution logic.
     */
    virtual const AssignableId& getBaseVariable() const = 0;

  private:

    AssignableId m_aid;
  };

} // namespace PLEXIL

#endif // PLEXIL_ASSIGNABLE_HH
