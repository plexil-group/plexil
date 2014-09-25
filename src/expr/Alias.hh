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
  // Forward declaration
  class Value;

  /**
   * @class Alias
   * @brief A read-only proxy for another expression.
   * @note Most commonly used in library nodes, but also anywhere
   *       read-only access to a mutable expression is needed.
   */
  class Alias : public NotifierImpl
  {
  public:
    Alias(NodeConnector *node, // *** is this needed?? ***
          const std::string &name,
          Expression *original = NULL,
          bool garbage = false);
    virtual ~Alias();

    /**
     * @brief Set the expression to which the Alias points.
     * @param exp The target expression.
     * @param garbage Whether the expression should be deleted with the Alias.
     * @return False if the Alias already has a target expression, true otherwise.
     */
    virtual bool setTarget(Expression *exp, bool garbage = false);

    //
    // Expression API
    //

    const char *exprName() const;
    const ValueType valueType() const;
    bool isKnown() const;
    virtual bool isAssignable() const;
    bool isConstant() const;
    Expression *getBaseExpression();
    Expression const *getBaseExpression() const;

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
    bool getValuePointer(Array const *&ptr) const;
    bool getValuePointer(BooleanArray const *&ptr) const;
    bool getValuePointer(IntegerArray const *&ptr) const;
    bool getValuePointer(RealArray const *&ptr) const;
    bool getValuePointer(StringArray const *&ptr) const;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    Value toValue() const;

  protected:

    //
    // Notification API
    //
    // N.B. Alias should never publish changes to its original expression.
    // It can pass on change notifications from the original.
    //
    void handleActivate();
    void handleDeactivate();

    // The expression being aliased.
    Expression *m_exp;
  
  private:
    // Disallow default, copy, assign
    Alias();
    Alias(const Alias &);
    Alias &operator=(const Alias &);

  protected:
    // Parent node
    NodeConnector *m_node;
    // Name in the parent node
    const std::string m_name;

  private:
    bool m_garbage;
  };

  /**
   * @class InOutAlias
   * @brief A read-write proxy for another expression.
   * @note Most commonly used in library nodes.
   */
  class InOutAlias : public Alias, public Assignable
  {
  public:
    InOutAlias(NodeConnector *node,
               const std::string &name,
               Expression *original = NULL,
               bool garbage = false);
    virtual ~InOutAlias();

    /**
     * @brief Set the expression to which the InOutAlias points.
     * @param exp The target expression.
     * @param garbage Whether the expression should be deleted with the Alias.
     * @return False if the InOutAlias already has a target expression or the new target is not
     *         assignable, true otherwise.
     */
    virtual bool setTarget(Expression *exp, bool garbage = false);

    const char *exprName() const;
    virtual bool isAssignable() const;

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
     * @deprecated These are being replaced with the setValue(Expression const *) method below.
     */
    void setValue(const double &val);
    void setValue(const int32_t &val);
    void setValue(const uint16_t &val);
    void setValue(const bool &val);
    void setValue(const std::string &val);
    void setValue(const char *val); // Convenience method

    void setValue(BooleanArray const &val);
    void setValue(IntegerArray const &val);
    void setValue(RealArray const &val);
    void setValue(StringArray const &val);

    /**
     * @brief Set the value for this expression from another expression.
     * @param valex The expression from which to obtain the new value.
     * @note May cause change notifications to occur.
     */
    void setValue(Expression const *valex);

    /**
     * @brief Set the value for this expression from a generic Value.
     * @param val The Value.
     * @note May cause change notifications to occur.
     */
    void setValue(Value const &value);

    /**
     * @brief Retrieve a writable ponter to the value.
     * @param valuePtr Reference to the pointer variable
     * @return True if the value is known, false if unknown or invalid.
     * @note Default method returns false and reports a type error.
     */
    bool getMutableValuePointer(std::string *& ptr);
    bool getMutableValuePointer(Array *& ptr);
    bool getMutableValuePointer(BooleanArray *& ptr);
    bool getMutableValuePointer(IntegerArray *& ptr);
    bool getMutableValuePointer(RealArray *& ptr);
    bool getMutableValuePointer(StringArray *& ptr);

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
     * @return The parent node; may be NULL.
     * @note Used by LuvFormat::formatAssignment().  
     * @note Default method returns noId().
     */
    NodeConnector const *getNode() const;
    NodeConnector *getNode();

    /**
     * @brief Get the real variable for which this may be a proxy.
     * @return Pointer to the base variable as an Assignable.
     */
    Assignable *getBaseVariable();
    Assignable const *getBaseVariable() const;
  
  private:
    // Default, copy, assign disallowed
    InOutAlias();
    InOutAlias(const InOutAlias &);
    InOutAlias &operator=(const InOutAlias &);

    // The original expression as an Assignable.
    Assignable *m_target;
  };

} // namespace PLEXIL

#endif // PLEXIL_ALIAS_HH
