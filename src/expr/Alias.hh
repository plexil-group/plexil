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

#ifndef PLEXIL_ALIAS_HH
#define PLEXIL_ALIAS_HH

#include "Assignable.hh"
#include "NotifierImpl.hh"

namespace PLEXIL
{
  /**
   * @class Alias
   * @brief A read-only proxy for another expression.
   * @note Most commonly used in library nodes, but also anywhere
   *       read-only access to a mutable expression is needed.
   */
  class Alias : public NotifierImpl
  {
  public:
    Alias(const NodeId &node, // *** is this needed?? ***
          const std::string &name,
          const ExpressionId &original); // I *believe* original can never be garbage
    virtual ~Alias();

    //
    // Expression API
    //

    const char *exprName() const;
    const ValueType valueType() const;
    bool isKnown() const;
    bool isAssignable() const;
    bool isConstant() const;
    void printValue(std::ostream &s) const;

    /**
     * @brief Retrieve the value of this Expression.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown or invalid.
     */
    bool getValue(bool &var) const;
    bool getValue(double &var) const;
    bool getValue(uint16_t &var) const;
    bool getValue(int32_t &var) const;
    bool getValue(std::string &var) const;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown or invalid.
     */
    bool getValuePointer(std::string const *&ptr) const;
    bool getValuePointer(Array<bool> const *&ptr) const;
    bool getValuePointer(Array<int32_t> const *&ptr) const;
    bool getValuePointer(Array<double> const *&ptr) const;
    bool getValuePointer(Array<std::string> const *&ptr) const;
    
  protected:

    //
    // Notification API
    //
    // N.B. Alias should never publish changes to its original expression.
    // It can pass on change notifications from the original.
    //
    void handleActivate();
    void handleDeactivate();
  
  private:
    // Disallow default, copy, assign
    Alias();
    Alias(const Alias &);
    Alias &operator=(const Alias &);

    // The expression being aliased.
    ExpressionId m_exp;

  protected:
    // Parent node
    NodeId m_node;
    // Name in the parent node
    const std::string m_name;
  };

  /**
   * @class InOutAlias
   * @brief A read-write proxy for another expression.
   * @note Most commonly used in library nodes.
   */
  class InOutAlias : public Alias, public Assignable
  {
  public:
    InOutAlias(const NodeId &node,
               const std::string &name,
               const ExpressionId &original); // I *believe* original can never be garbage;
    virtual ~InOutAlias();

    const char *exprName() const;
    bool isAssignable() const;

    //
    // Assignable API
    //
    // Most of these are simply forwarded to the target.
    //

    /**
     * @brief Set the value of this expression back to the initial value with which it was
     *        created.
     * @brief No-op for InOutAlias.
     */
    void reset();

    /**
     * @brief Set the current value of this variable to "unknown".
     */
    void setUnknown();

    /**
     * @brief Set the value for this expression.
     * @param val The new value for this expression.
     * @deprecated These are being replaced with the setValue(ExpressionId const &) method below.
     */
    void setValue(const double &val);
    void setValue(const int32_t &val);
    void setValue(const uint16_t &val);
    void setValue(const bool &val);
    void setValue(const std::string &val);
    void setValue(const char *val); // Convenience method

    void setValue(const Array<bool> &val);
    void setValue(const Array<int32_t> &val);
    void setValue(const Array<double> &val);
    void setValue(const Array<std::string> &val);

    /**
     * @brief Set the value for this expression from another expression.
     * @param valex The expression from which to obtain the new value.
     * @note May cause change notifications to occur.
     */
    void setValue(ExpressionId const &valex);

    /**
     * @brief Retrieve a writable ponter to the value.
     * @param valuePtr Reference to the pointer variable
     * @return True if the value is known, false if unknown or invalid.
     * @note Default method returns false and reports a type error.
     */
    bool getMutableValuePointer(std::string *& ptr);
    bool getMutableValuePointer(Array<bool> *& ptr);
    bool getMutableValuePointer(Array<int32_t> *& ptr);
    bool getMutableValuePointer(Array<double> *& ptr);
    bool getMutableValuePointer(Array<std::string> *& ptr);

    /**
     * @brief Temporarily stores the previous value of this variable.
     * @note Used to implement recovery from failed Assignment nodes.
     */
    void saveCurrentValue();

    /**
     * @brief Restore the value set aside by saveCurrentValue().
     * @note Used to implement recovery from failed Assignment nodes.
     */
    void restoreSavedValue();

    /**
     * @brief Get the name of this variable, as declared in the node that owns it.
     */
    const std::string& getName() const;

    /**
     * @brief Get the node that owns this expression.
     * @return The NodeId of the parent node; may be noId.
     * @note Used by LuvFormat::formatAssignment().  
     * @note Default method returns noId().
     */
    const NodeId& getNode() const;

    /**
     * @brief Get the real variable for which this may be a proxy.
     * @return The AssignableId of the base variable
     * @note Used by the assignment node conflict resolution logic.
     */
    const AssignableId& getBaseVariable() const;
  
  private:
    // Default, copy, assign disallowed
    InOutAlias();
    InOutAlias(const InOutAlias &);
    InOutAlias &operator=(const InOutAlias &);

    // The original expression as an Assignable.
    AssignableId m_target;
  };

} // namespace PLEXIL

#endif // PLEXIL_ALIAS_HH
