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
  class UserVariable : public Assignable, public ExpressionImpl<T>
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
                 const std::string &name = "");
    
    /**
     * @brief Destructor.
     */
    virtual ~UserVariable();

    //
    // Essential Expression API
    //

    const char *exprName() const;
    const ValueType valueType() const;

    bool isKnown() const;

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     * @note Limited type conversions supported.
     * @note Unimplemented conversions will cause a link time error.
     */
    bool getValueImpl(T &result) const;

    /**
     * @brief Assign the initial value.
     * @param value The value to assign.
     * @note Type conversions must go on derived classes.
     */
    void setInitialValue(const T &value);

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     * @note Type conversions must go on derived classes.
     */
    void setValue(const T &value);

    void setInitialUnknown();

    void setUnknown();

    void reset();

    void saveCurrentValue();

    void restoreSavedValue();

    const std::string& getName() const;

    const NodeId &getNode() const;

    const ExpressionId& getBaseVariable() const;

    // Simple variables have no dependencies, therefore this method does nothing.
    void notifyChanged();

  protected: // private?
    
    // Only used by LuvListener at present. Eliminate?
    NodeId m_node;

    std::string m_name;

    T m_value;
    T m_initialValue; // for reset()
    T m_savedValue;   // for undoing assignment 

    bool m_known;
    bool m_initialKnown;
    bool m_savedKnown;

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
                   const std::string &name = "");
    
    /**
     * @brief Destructor.
     */
    virtual ~StringVariable();

    // The reason this class exists.
    void setInitialValue(const char *value);

    // Necessary because C++ sucks.
    void setInitialValue(const std::string &value);

    // The reason this class exists.
    void setValue(const char *value);

    // Necessary because C++ sucks.
    void setValue(const std::string &value);
  };

  //
  // Convenience typedefs 
  //

  typedef UserVariable<bool> BooleanVariable;
  typedef UserVariable<int32_t> IntegerVariable;
  typedef UserVariable<double> RealVariable;

} // namespace PLEXIL

#endif // PLEXIL_USER_VARIABLE_HH
