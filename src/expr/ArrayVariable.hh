/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_ARRAY_VARIABLE_HH
#define PLEXIL_ARRAY_VARIABLE_HH

#include "Assignable.hh"
#include "GetValueImpl.hh"
#include "NotifierImpl.hh"

namespace PLEXIL
{

  /**
   * @class ArrayVariable
   * @brief A class similar to UserVariable, which adds accessors required
   *        by the ArrayReference and MutableArrayReference expression classes.
   */

  template <typename T>
  class ArrayVariable :
    public Assignable,
    public GetValueImpl<ArrayImpl<T> >,
    public NotifierImpl
  {
  public:

    /**
     * @brief Default constructor.
     */
    ArrayVariable();

    /**
     * @brief Constructor with initial value.
     * @param initVal The initial value.
     */
    ArrayVariable(ArrayImpl<T> const & initVal);

    /**
     * @brief Constructor for plan loading.
     * @param node The node to which this variable belongs (default none).
     * @param name The name of this variable in the parent node.
     */
    ArrayVariable(NodeConnector *node,
                  char const *name = "",
                  Expression *size = NULL,
                  bool sizeIsGarbage = false);

    virtual ~ArrayVariable();

    //
    // Essential Expression API
    //

    bool isAssignable() const;

    Assignable const *asAssignable() const;
    Assignable *asAssignable();

    char const *getName() const;

    char const *exprName() const;

    bool isKnown() const;

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

    NodeConnector const *getNode() const;
    NodeConnector *getNode();

    Expression *getBaseVariable();
    Expression const *getBaseVariable() const;

    /**
     * @brief Set the expression from which this object gets its initial value.
     * @param expr Pointer to an Expression.
     * @param garbage True if the expression should be deleted with this object, false otherwise.
     */
    void setInitializer(Expression *expr, bool garbage);

    /**
     * @brief Set the value for this object.
     * @param val The new value for this object.
     */
    virtual void setValue(Value const &val);

    /**
     * @brief Set the value for this object.
     * @param val The expression with the new value for this object.
     */
    virtual void setValue(Expression const &val);

    /**
     * @brief Retrieve a pointer to the non-const value.
     * @param valuePtr Reference to the pointer variable
     * @return True if the value is known, false if unknown or invalid.
     */
    virtual bool getMutableValuePointer(Array *&ptr);

    void handleActivate();

    void handleDeactivate();

    void printSpecialized(std::ostream &s) const;

  protected:

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     */
    void setValueImpl(ArrayImpl<T> const &value);

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    bool getValuePointerImpl(ArrayImpl<T> const *&ptr) const;

  private:

    /**
     * @brief Pre-allocate storage based on the current value of the size expression.
     */
    void reserve();

    ArrayImpl<T> m_value;
    ArrayImpl<T> m_savedValue;   // for undoing assignment 

    Expression *m_size;
    Expression *m_initializer;
    char const *m_name;
    size_t m_maxSize;
    
    // Only used by LuvListener at present. Eliminate?
    NodeConnector *m_node;

    bool m_known;
    bool m_savedKnown;
    bool m_sizeIsGarbage;
    bool m_initializerIsGarbage;
  };

  //
  // Convenience typedefs 
  //

  typedef ArrayVariable<bool>        BooleanArrayVariable;
  typedef ArrayVariable<int32_t>     IntegerArrayVariable;
  typedef ArrayVariable<double>      RealArrayVariable;
  typedef ArrayVariable<std::string> StringArrayVariable;

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_VARIABLE_HH
