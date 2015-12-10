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

#ifndef PLEXIL_USER_VARIABLE_HH
#define PLEXIL_USER_VARIABLE_HH

#include "AssignableImpl.hh"
#include "ExpressionImpl.hh"
#include "VariableConflictSet.hh"

namespace PLEXIL 
{

  /**
   * @class UserVariable
   * @brief Templatized class for user-created plan variables.
   */

  // Scalar case
  template <typename T>
  class UserVariable :
    public NotifierImpl,
    public ExpressionImpl<T>,
    public AssignableImpl<T>
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
     * @param node The node to which this variable belongs (default none).
     * @param name The name of this variable in the parent node.
     */
    UserVariable(NodeConnector *node,
                 char const *name = "");
    
    /**
     * @brief Destructor.
     */
    virtual ~UserVariable();

    //
    // Essential Expression API
    //

    char const *getName() const;

    char const *exprName() const;

    bool isKnown() const;

    //
    // Assignable and AssignableImpl API
    //

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    bool getValueImpl(T &result) const;

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     * @note Type conversions must go on derived classes.
     */
    void setValueImpl(T const &value);

    /**
     * @brief Set the current value unknown.
     */
    void setUnknown();

    /**
     * @brief Reset to initial status.
     */
    void reset();

    void saveCurrentValue();

    void restoreSavedValue();

    Value getSavedValue() const;

    void setName(const std::string &);

    NodeConnector *getNode();
    NodeConnector const *getNode() const;

    Assignable *getBaseVariable();
    Assignable const *getBaseVariable() const;

    /**
     * @brief Set the expression from which this object gets its initial value.
     * @param expr Pointer to an Expression.
     * @param garbage True if the expression should be deleted with this object, false otherwise.
     */
    void setInitializer(Expression *expr, bool garbage);

    /**
     * @brief Get the conflict set of nodes assigning to this object.
     * @return Reference to conflict set, if any.
     * @note Default method returns NULL.
     */
    virtual VariableConflictSet &getConflictSet();

    void handleActivate();

    void handleDeactivate();

    void printSpecialized(std::ostream &s) const;

  private:

    // N.B. Ordering is suboptimal for bool because of required padding;
    // fine for int32_t and double
    T m_value;
    T m_savedValue;   // for undoing assignment 

    Expression *m_initializer;
    char const *m_name;
    VariableConflictSet m_conflicts;

    // Only used by LuvListener at present. Eliminate?
    NodeConnector *m_node;

    bool m_known;
    bool m_savedKnown;
    bool m_initializerIsGarbage;

  };

  // String case
  template <>
  class UserVariable<std::string> :
    public NotifierImpl,
    public ExpressionImpl<std::string>,
    public AssignableImpl<std::string>
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
    UserVariable(std::string const &initVal);

    /**
     * @brief Constructor for plan loading.
     * @param node The node to which this variable belongs (default none).
     * @param name The name of this variable in the parent node.
     */
    UserVariable(NodeConnector *node,
                 char const *name = "");
    
    /**
     * @brief Destructor.
     */
    virtual ~UserVariable();

    //
    // Essential Expression API
    //

    char const *getName() const;

    char const *exprName() const;

    bool isKnown() const;

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    bool getValueImpl(std::string &result) const;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    bool getValuePointerImpl(std::string const *&ptr) const;
    template <typename U>
    bool getValuePointerImpl(U const *&ptr) const;

    /**
     * @brief Retrieve a pointer to the (modifiable) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown or invalid.
     */
    bool getMutableValuePointerImpl(std::string *&ptr);

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     * @note Type conversions must go on derived classes.
     */
    void setValueImpl(std::string const &value);

    /**
     * @brief Set the current value unknown.
     */
    void setUnknown();

    /**
     * @brief Reset to initial status.
     */
    void reset();

    void saveCurrentValue();

    void restoreSavedValue();

    Value getSavedValue() const;

    void setName(const std::string &);

    NodeConnector *getNode();
    NodeConnector const *getNode() const;

    Assignable *getBaseVariable();
    Assignable const *getBaseVariable() const;

    /**
     * @brief Set the expression from which this object gets its initial value.
     * @param expr Pointer to an Expression.
     * @param garbage True if the expression should be deleted with this object, false otherwise.
     */
    void setInitializer(Expression *expr, bool garbage);

    /**
     * @brief Get the conflict set of nodes assigning to this object.
     * @return Pointer to conflict set, if any.
     * @note Default method returns NULL.
     */
    virtual VariableConflictSet &getConflictSet();

    void handleActivate();

    void handleDeactivate();

    void printSpecialized(std::ostream &s) const;

  private:

    std::string m_value;
    std::string m_savedValue;   // for undoing assignment 

    Expression *m_initializer;
    char const *m_name;
    VariableConflictSet m_conflicts;

    // Only used by LuvListener at present. Eliminate?
    NodeConnector *m_node;

    bool m_known;
    bool m_savedKnown;
    bool m_initializerIsGarbage;

  };

  //
  // Convenience typedefs 
  //

  typedef UserVariable<bool>        BooleanVariable;
  typedef UserVariable<int32_t>     IntegerVariable;
  typedef UserVariable<double>      RealVariable;
  typedef UserVariable<std::string> StringVariable;

} // namespace PLEXIL

#endif // PLEXIL_USER_VARIABLE_HH
