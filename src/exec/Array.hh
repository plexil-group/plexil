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

#ifndef PLEXIL_ARRAY_HH
#define PLEXIL_ARRAY_HH

#include "ExecDefs.hh"
#include "Variable.hh"

namespace PLEXIL
{

  /**
   * An abstract base class representing a variable-like object
   * which stores an array.
   */
  class ArrayVariableBase :
	public virtual Variable
  {
  public:
	ArrayVariableBase();

	virtual ~ArrayVariableBase();

	/**
	 * @brief Get the maximum size of this array.
	 */
    virtual unsigned long maxSize() const = 0;

	/**
	 * @brief Get the element at the given index.
	 */
    virtual double lookupValue(unsigned long index) const = 0;

    /**
     * @brief Set one element of this array from the given value.
     * @note Value must be an array or UNKNOWN.
     * @note Index must be less than maximum length
     */
    virtual void setElementValue(unsigned index, const double value) = 0;

    /**
     * @brief Retrieve the element type of this array.
     * @return The element type of this array.
     */
    virtual PlexilType getElementType() const = 0;

    /**
     * @brief Check to make sure an element value is appropriate for this array.
     */
    virtual bool checkElementValue(const double val) = 0;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return ARRAY; }

	/**
	 * @brief Report whether the expression is an array.
	 * @return True if an array, false otherwise.
	 */
	virtual bool isArray() const { return true; }

	const ArrayVariableId& getArrayId() const { return m_avid; }

  protected:

  private:
	// deliberately unimplemented
	ArrayVariableBase(const ArrayVariableBase&);
	ArrayVariableBase& operator=(const ArrayVariableBase&);

	ArrayVariableId m_avid;
  };

  class ArrayAliasVariable :
	public ArrayVariableBase,
	public AliasVariable
  {
  public:
	ArrayAliasVariable(const std::string& name, 
					   const NodeConnectorId& nodeConnector,
					   const ExpressionId& exp,
					   bool expIsGarbage,
					   bool isConst);
    virtual ~ArrayAliasVariable();

    /**
     * @brief Get a string representation of this Expression.
     * @return The string representation.
     */
    void print(std::ostream& s) const;

	// Defined in multiple superclasses
	PlexilType getValueType() const;

	// ArrayVariable API
	virtual unsigned long maxSize() const;
	virtual double lookupValue(unsigned long index) const;
    virtual void setElementValue(unsigned index, const double value);
    virtual PlexilType getElementType() const;
    virtual bool checkElementValue(const double val);

  protected:

  private:
	// deliberately unimplemented
	ArrayAliasVariable();
	ArrayAliasVariable(const ArrayAliasVariable&);
	ArrayAliasVariable& operator=(const ArrayAliasVariable&);

	ArrayVariableId m_originalArray;
  };

  class ArrayVariable :
	public ArrayVariableBase,
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

    void print(std::ostream& s) const;
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

  protected:

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

  // Access to an element of an array

  class ArrayElement : public Variable
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

    void print(std::ostream& s) const;

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
     * @brief Get the node that owns this expression.
     * @return The NodeId of the parent node; may be noId.
	 * @note Used by LuvFormat::formatAssignment().  
     */
    const NodeId& getNode() const { return m_node; }

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     */
    virtual void handleChange(const ExpressionId& exp);

    void handleActivate(const bool changed);

    void handleDeactivate(const bool changed);

    double recalculate();

	/**
	 * @brief Get the real variable for which this may be a proxy.
	 * @return The VariableId of the base variable
	 * @note Used by the assignment node conflict resolution logic.
	 */
	virtual const VariableId& getBaseVariable() const
	{
	  return m_arrayVariable->getBaseVariable();
	}

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

    ArrayVariableId m_arrayVariable;
    ExpressionId m_index;
	const NodeId m_node;
    bool m_deleteIndex;
    DerivedVariableListener m_listener;
  };

}

#endif // PLEXIL_ARRAY_HH
