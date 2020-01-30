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

#ifndef PLEXIL_USER_VARIABLE_HH
#define PLEXIL_USER_VARIABLE_HH

#include "Assignable.hh"
#include "GetValueImpl.hh"
#include "Notifier.hh"

#include <vector>

namespace PLEXIL 
{

  /**
   * @class UserVariable
   * @brief Templatized class for user-created plan variables.
   */

  // Scalar case
  template <typename T>
  class UserVariable final :
    public Assignable,
    public GetValueImpl<T>,
    public Notifier
  {
  public:

    /**
     * @brief Default constructor.
     */
    UserVariable();

    /**
     * @brief Constructor with initial value.
     * @param val The initial value.
     */
    UserVariable(T const &initVal);

    /**
     * @brief Constructor for plan loading.
     * @param name The name of this variable in the parent node.
     */
    UserVariable(char const *name);
    
    /**
     * @brief Destructor.
     */
    virtual ~UserVariable();

    // Listenable API
    virtual bool isPropagationSource() const override;

    //
    // Essential Expression API
    //

    virtual bool isAssignable() const override;

    virtual Assignable const *asAssignable() const override;
    virtual Assignable *asAssignable() override;

    virtual char const *getName() const override;

    virtual char const *exprName() const override;

    //
    // GetValueImpl API
    //

    virtual bool isKnown() const override;

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    virtual bool getValue(T &result) const override;

    //
    // Assignable API
    //

    virtual void saveCurrentValue() override;

    virtual void restoreSavedValue() override;

    virtual Value getSavedValue() const override;

    virtual void setInitializer(Expression *expr, bool garbage) override;

    virtual void setUnknown() override;

    virtual void setValue(Value const &val) override;

    virtual Assignable *getBaseVariable() override;
    virtual Assignable const *getBaseVariable() const override;

    virtual bool isInUse() const override;
    virtual bool reserve(Node *node) override;
    virtual void release() override;

    virtual void addWaitingNode(Node *node) override;
    virtual void removeWaitingNode(Node *node) override;

    /**
     * @brief Set the value for this object.
     * @param val The expression with the new value for this object.
     */
    virtual void setValue(Expression const &val);

    virtual void handleActivate() override;

    virtual void handleDeactivate() override;

    virtual void printSpecialized(std::ostream &s) const override;

  protected:
    
    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     */
    void setValueImpl(T const &value);

  private:

    //! Nodes waiting to assign to this variable. Usually empty.
    std::vector<Node *> m_waiters;

    // N.B. Ordering is suboptimal for bool because of required padding;
    // fine for Integer and Real
    T m_value;
    T m_savedValue;   // for undoing assignment 

    Expression *m_initializer;
    char const *m_name;

    Node *m_user;

    bool m_known;
    bool m_savedKnown;
    bool m_initializerIsGarbage;

  };

  // String case
  template <>
  class UserVariable<String> final :
    public Assignable,
    public GetValueImpl<String>,
    public Notifier
  {
  public:

    /**
     * @brief Default constructor.
     */
    UserVariable();

    /**
     * @brief Constructor with initial value.
     * @param val The initial value.
     */
    explicit UserVariable(String const &initVal);

    /**
     * @brief Constructor for plan loading.
     * @param name The name of this variable in the parent node.
     */
    UserVariable(char const *name);

    /**
     * @brief Destructor.
     */
    virtual ~UserVariable();

    // Listenable API
    virtual bool isPropagationSource() const override;

    //
    // Essential Expression API
    //

    virtual bool isAssignable() const override;

    virtual Assignable const *asAssignable() const override;
    virtual Assignable *asAssignable() override;

    virtual char const *getName() const override;

    virtual char const *exprName() const override;

    virtual bool isKnown() const override;

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    virtual bool getValue(String &result) const override;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(String const *&ptr) const override;
    template <typename U>
    bool getValuePointer(U const *&ptr) const;

    //
    // Assignable API
    //

    virtual void saveCurrentValue() override;

    virtual void restoreSavedValue() override;

    virtual Value getSavedValue() const override;

    virtual void setInitializer(Expression *expr, bool garbage) override;

    virtual void setUnknown() override;

    virtual void setValue(Value const &val) override;

    virtual Assignable *getBaseVariable() override;
    virtual Assignable const *getBaseVariable() const override;

    virtual bool isInUse() const override;
    virtual bool reserve(Node *node) override;
    virtual void release() override;

    virtual void addWaitingNode(Node *node) override;
    virtual void removeWaitingNode(Node *node) override;

    /**
     * @brief Set the value for this object.
     * @param val The expression with the new value for this object.
     */
    virtual void setValue(Expression const &val);

    virtual void handleActivate() override;

    virtual void handleDeactivate() override;

    virtual void printSpecialized(std::ostream &s) const override;

  protected:

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     */
    virtual void setValueImpl(String const &value);

  private:

    //! Nodes waiting to assign to this variable. Usually empty.
    std::vector<Node *> m_waiters;

    String m_value;
    String m_savedValue;   // for undoing assignment 

    Expression *m_initializer;
    char const *m_name;

    Node *m_user;

    bool m_known;
    bool m_savedKnown;
    bool m_initializerIsGarbage;

  };

  //
  // Convenience typedefs 
  //

  typedef UserVariable<Boolean>     BooleanVariable;
  typedef UserVariable<Integer>     IntegerVariable;
  typedef UserVariable<Real>        RealVariable;
  typedef UserVariable<String>      StringVariable;

} // namespace PLEXIL

#endif // PLEXIL_USER_VARIABLE_HH
