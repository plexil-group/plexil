/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

#ifndef VARIABLES_HH
#define VARIABLES_HH

#include "Variable.hh"

namespace PLEXIL
{

  /**
   * variables MUST implement:
   * bool checkValue(const double val)
   *
   * variables MAY implement:
   * destructor
   * PlexilType getValueType() const
   * void setValue(const double val)
   * std::string toString() const
   * std::string valueString() const (BUT SHOULD RESPECT m_active!!)
   * void handleLock()
   * void handleUnlock()
   * void handleActivate(const bool changed)
   * void handleDeactivate(const bool changed)
   * handleReset()
   *
   */

  class ArrayVariable :
	public EssentialArrayVariable,
	public VariableImpl // ???
  {
  public:
    ArrayVariable(unsigned long maxSize, 
				  PlexilType type, 
                  const bool isConst = false);
    ArrayVariable(unsigned long maxSize, 
				  PlexilType type, 
                  std::vector<double>& values,
                  const bool isConst = false);
    ArrayVariable(const PlexilExprId& expr, 
                  const NodeConnectorId& node,
                  const bool isConst = false);

    virtual ~ArrayVariable();

    std::string toString() const;
    double lookupValue(unsigned long index) const;
    unsigned long maxSize() const {return m_maxSize;}

    /**
     * @brief Set the contents of this array from the given values.
     */
    void setValues(std::vector<double>& values);

    /**
     * @brief Set the contents of this array from the given array value.
     */
    void setValues(const double key);

    /**
     * @brief Set the contents of this array from the given value.
     * @note Value must be an array or UNKNOWN.
     */
    void setValue(const double value);

    /**
     * @brief Set the value of this expression back to the initial value with which it was
     *        created.
     */
    void reset();

    /**
     * @brief Set one element of this array from the given value.
     * @note Value must be an array or UNKNOWN.
     * @note Index must be less than maximum length
     */
    void setElementValue(unsigned index, const double value);

    /**
     * @brief Retrieve the element type of this array.
     * @return The element type of this array.
     */
    PlexilType getElementType() const { return m_type; }

    /**
     * @brief Check to make sure an element value is appropriate for this array.
     */
    bool checkElementValue(const double val);

    /**
     * @brief Notify this array that an element's value has changed.
     * @param elt The changed element.
     */
    virtual void handleElementChanged(const ExpressionId& elt);

  protected:

    /**
     * @brief Notify listeners that an element has changed.
     * @param elt The changed element expression.
     */
    virtual void publishElementChange(const ExpressionId& elt);

  private:

    unsigned long       m_maxSize;
    PlexilType          m_type;
    std::vector<double>	m_initialVector;

    /**
     * @brief Check to make sure a value is appropriate for this variable.
     */
    bool checkValue(const double val);

    /**
     * @brief Check to make sure the index is appropriate for this array.
     */
    bool checkIndex(const unsigned index) const 
    { return index < m_maxSize; }

  };

  typedef Id<ArrayVariable> ArrayVariableId;

  class StringVariable : public VariableImpl {
  public:
    StringVariable(const bool isConst = false);
    StringVariable(const std::string& value, const bool isConst = false);
    StringVariable(const char* value, const bool isConst);
    StringVariable(const double value, const bool isConst = false);
    StringVariable(const LabelStr& value, const bool isConst = false);
    StringVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		   const bool isConst = false);
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return STRING; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class RealVariable : public VariableImpl {
  public:
    static ExpressionId& ZERO_EXP();
    static ExpressionId& ONE_EXP();
    static ExpressionId& MINUS_ONE_EXP();

    RealVariable(const bool isConst = false);
    RealVariable(const double value, const bool isConst = false);
    RealVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		 const bool isConst = false);
    std::string toString() const;
    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return REAL; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class IntegerVariable : public VariableImpl {
  public:
    static ExpressionId& ZERO_EXP();
    static ExpressionId& ONE_EXP();
    static ExpressionId& MINUS_ONE_EXP();

    IntegerVariable(const bool isConst = false);
    IntegerVariable(const double value, const bool isConst = false);
    IntegerVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		    const bool isConst = false);
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return INTEGER; }

  protected:
  private:
    bool checkValue(const double val);
  };

  // Access to an element of an array

  class ArrayElement : public DerivedVariable
  {
  public:
    /**
     * @brief Constructor.
     * @param expr The PlexilArrayElementId expression used as a template.
     * @param node The NodeConnectorId used to find variable references.
     */
    ArrayElement(const PlexilExprId& expr, const NodeConnectorId& node);

    /**
     * @brief Destructor.
     */
    virtual ~ArrayElement();

    std::string toString() const;

    /**
     * @brief Set the value of this expression back to the initial value with which it was
     *        created.
     */
    virtual void reset();

    /**
     * @brief Sets the value of the array element.  
     *        Will throw an error if the variable was constructed with isConst == true.
     * @param value The new value for this array element.
     */
    void setValue(const double value);

    /**
     * @brief Gets the const-ness of this variable.
     * @return True if this variable is const, false otherwise.
     */
    bool isConst() const 
    {
      return m_arrayVariable->isConst();
    }

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

    /**
     * @brief Notify listeners that the value of this expression has changed.
     */
    virtual void publishChange();

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     */
    virtual void handleChange(const ExpressionId& exp);

    void handleActivate(const bool changed);

    void handleDeactivate(const bool changed);

    double recalculate();

  protected:

  private:

    // Deliberately unimplemented
    ArrayElement();
    ArrayElement(const ArrayElement&);
    ArrayElement& operator=(const ArrayElement&);
    
    /**
     * @brief Check to make sure a value is appropriate for this expression.
     */
    bool checkValue(const double value);

    EssentialArrayVariableId m_arrayVariable;
    ExpressionId m_index;
    bool m_deleteIndex;
    DerivedVariableListener m_listener;
  };

  class TimepointVariable : public ConstVariableWrapper 
  {
  public:
    TimepointVariable(const PlexilExprId& expr, const NodeConnectorId& node);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return TIME; }

  private:
	// Deliberately unimplemented
	TimepointVariable();
	TimepointVariable(const TimepointVariable&);
	TimepointVariable& operator=(const TimepointVariable&);
  };

}

#endif // VARIABLES_HH
