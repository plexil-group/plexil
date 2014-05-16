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

#include "Assignable.hh"
#include "ExpressionImpl.hh"

namespace PLEXIL {

  /**
   * @class UserVariable
   * @brief Templatized class for user-created plan variables.
   */

  // TODO: Support exec listener for assignments

  template <typename T>
  class UserVariable : public NotifierImpl, public ExpressionImpl<T>, public Assignable
  {
  public:

    /**
     * @brief Default constructor.
     */
    UserVariable();

    /**
     * @brief Constructor with initial value (for regression testing).
     * @param val The initial value.
     */
    UserVariable(const T &initVal);

    /**
     * @brief Constructor for plan loading.
     * @param node The node to which this variable belongs (default none).
     * @param name The name of this variable in the parent node.
     */
    UserVariable(const NodeId &node,
                 const std::string &name = "",
                 const ExpressionId &initializer = ExpressionId::noId(),
                 bool initializerIsGarbage = false);
    
    /**
     * @brief Destructor.
     */
    virtual ~UserVariable();

    //
    // Essential Expression API
    //

    const char *exprName() const;

    bool isKnown() const;

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    bool getValueImpl(T &result) const;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    bool getValuePointerImpl(T const *&ptr) const;
    bool getValuePointerImpl(Array const *&ptr) const;

    /**
     * @brief Retrieve a pointer to the (modifiable) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown or invalid.
     */
    bool getMutableValuePointer(T *&ptr);

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     * @note Type conversions must go on derived classes.
     */
    void setValue(T const &value);

    /**
     * @brief Assign a new value from another expression.
     * @param valex The expression from which to obtain the new value.
     */
    void setValue(ExpressionId const &valex);

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

    const std::string& getName() const;

    const NodeId &getNode() const;

    const AssignableId& getBaseVariable() const;

    void handleActivate();

    void handleDeactivate();

  protected:

    ExpressionId m_initializer;
    
    // Only used by LuvListener at present. Eliminate?
    NodeId m_node;

    std::string m_name;

    T m_value;
    T m_savedValue;   // for undoing assignment 

    bool m_known;
    bool m_savedKnown;
    bool m_initializerIsGarbage;

  };

  //
  // Specializations
  //

  //
  // StringVariable
  //
  class StringVariable : public UserVariable<std::string>
  {
  public:

    StringVariable();

    StringVariable(const std::string &initVal);
    StringVariable(const char *initVal); 

    StringVariable(const NodeId &node,
                   const std::string &name = "",
                   const ExpressionId & initializer = ExpressionId::noId(),
                   bool initializerIsGarbage = false);

    /**
     * @brief Destructor.
     */
    virtual ~StringVariable();

    // The reason this class exists.
    void setValue(const char *value);

    // Necessary because C++ sucks.
    void setValue(const std::string &value);
  };

  /**
   * @class ArrayVariable
   * @brief A class derived from UserVariable, which adds accessors required
   *        by the ArrayReference and MutableArrayReference expression classes.
   */

  //
  // Convenience typedefs 
  //

  typedef UserVariable<bool>    BooleanVariable;
  typedef UserVariable<int32_t> IntegerVariable;
  typedef UserVariable<double>  RealVariable;

} // namespace PLEXIL

#endif // PLEXIL_USER_VARIABLE_HH
