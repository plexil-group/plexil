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

#ifndef PLEXIL_ARRAY_REFERENCE_HH
#define PLEXIL_ARRAY_REFERENCE_HH

#include "AssignableImpl.hh"
#include "ExpressionImpl.hh"

namespace PLEXIL {

  // Forward reference
  class Value;

  template <typename T>
  class ArrayReference : public NotifierImpl, public ExpressionImpl<T>
  {
  public:
    ArrayReference(const ExpressionId &ary,
                   const ExpressionId &idx,
                   bool aryIsGarbage = false,
                   bool idxIsGarbage = false);

    ~ArrayReference();

    //
    // Essential Expression API
    //

    const std::string &getName() const;
    const char *exprName() const;
    bool isKnown() const;
    bool isConstant() const;
    ExpressionId const &getBaseExpression() const;

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     * @note Limited type conversions supported.
     * @note Unimplemented conversions will cause a link time error.
     */
    bool getValueImpl(T &result) const;

    /**
     * @brief Get a pointer to the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    bool getValuePointerImpl(T const *&ptr) const;

    void handleActivate();
    void handleDeactivate();

  protected:
    // State shared with MutableArrayReference
    ExpressionId m_array;
    ExpressionId m_index;

    bool m_arrayIsGarbage;
    bool m_indexIsGarbage;

  private:
    // Disallow default, copy, assignment
    ArrayReference();
    ArrayReference(const ArrayReference &);
    ArrayReference &operator=(const ArrayReference &);

    // Internal function
    bool selfCheck(ArrayImpl<T> const *&valuePtr,
                   size_t &idx) const;
  };

  /**
   * @class MutableArrayReference
   * @brief Expression class that represents a modifiable location in an array.
   */

  // TODO: Support exec listener for assignments

  template <typename T>
  class MutableArrayReference : public ArrayReference<T>, public AssignableImpl<T>
  {
  public:
    MutableArrayReference(const ExpressionId &ary,
                          const ExpressionId &idx,
                          bool aryIsGarbage = false,
                          bool idxIsGarbage = false);

    ~MutableArrayReference();

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     */
    void setValueImpl(T const &value);

    /**
     * @brief Assign the current value to UNKNOWN.
     */
    void setUnknown();

    /**
     * @brief Get a writeable pointer to the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    bool getMutableValuePointerImpl(T *&ptr);

    /**
     * @brief Reset the expression.
     */
    void reset();

    void saveCurrentValue();

    void restoreSavedValue();

    const NodeConnectorId &getNode() const;

    Assignable *getBaseVariable();
    Assignable const *getBaseVariable() const;

  private:
    // Default, copy, assignment disallowed
    MutableArrayReference();
    MutableArrayReference(const MutableArrayReference &);
    MutableArrayReference &operator=(const MutableArrayReference &);

    // Internal function
    bool mutableSelfCheck(ArrayImpl<T> *&ary, size_t &idx);

    Assignable *m_mutableArray;
    T m_savedValue;
    bool m_savedKnown;
  };

  //
  // Convenience typedefs
  //
  typedef ArrayReference<bool>        BooleanArrayReference;
  typedef ArrayReference<int32_t>     IntegerArrayReference;
  typedef ArrayReference<double>      RealArrayReference;
  typedef ArrayReference<std::string> StringArrayReference;

  typedef MutableArrayReference<bool>        BooleanMutableArrayReference;
  typedef MutableArrayReference<int32_t>     IntegerMutableArrayReference;
  typedef MutableArrayReference<double>      RealMutableArrayReference;
  typedef MutableArrayReference<std::string> StringMutableArrayReference;

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_REFERENCE_HH
