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

#ifndef PLEXIL_EXPRESSION_IMPL_HH
#define PLEXIL_EXPRESSION_IMPL_HH

#include "Expression.hh"

namespace PLEXIL
{

  /**
   * @class ExpressionAdapter
   * @brief Adapter (Design Patterns pp. 139-150) around templatized Expression derivatives,
   *        isolating the Expression API from the implementation.
   * @note This class should NEVER be explicitly mentioned outside this file.
   * @note If the template trickery confuses you, do a web search for "Curiously Recurring Template Pattern".
   */

  template <class C>
  class ExpressionAdapter : public virtual Expression
  {
  public:

    /**
     * @brief Retrieve the value of this Expression in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    bool getValue(bool &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(uint16_t &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(int32_t &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(double &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(std::string &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    bool getValuePointer(std::string const *&ptr) const
    {
      return static_cast<const C *>(this)->getValuePointerImpl(ptr);
    }

    bool getArrayContents(std::vector<bool> const *&valuePtr,
                          std::vector<bool> const *&knownPtr) const
    {
      return static_cast<const C *>(this)->getArrayContentsImpl(valuePtr, knownPtr);
    }

    bool getArrayContents(std::vector<int32_t> const *&valuePtr,
                          std::vector<bool> const *&knownPtr) const
    {
      return static_cast<const C *>(this)->getArrayContentsImpl(valuePtr, knownPtr);
    }

    bool getArrayContents(std::vector<double> const *&valuePtr,
                          std::vector<bool> const *&knownPtr) const
    {
      return static_cast<const C *>(this)->getArrayContentsImpl(valuePtr, knownPtr);
    }

    bool getArrayContents(std::vector<std::string> const *&valuePtr,
                          std::vector<bool> const *&knownPtr) const
    {
      return static_cast<const C *>(this)->getArrayContentsImpl(valuePtr, knownPtr);
    }

  };

  /**
   * @class ExpressionImpl
   * @brief A pure virtual interface mixin class, specializing the expression based on value type.
   * @note Templatized Expression implementations should derive from this template.
  */

  template <typename T>
  class ExpressionImpl : public ExpressionAdapter<ExpressionImpl<T> >
  {
  public:
    /**
     * @brief Query whether the expression's value is known.
     * @return True if known, false otherwise.
     * @note Default method uses getValueImpl().
     */
    bool isKnown() const;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    const ValueType valueType() const;

	/**
	 * @brief Print the expression's value to the given stream.
	 * @param s The output stream.
	 */
    void printValue(std::ostream &) const; // may be overridden?

    /**
     * @brief Retrieve the value of this Expression in its native type.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValueImpl(T &result) const = 0;

    /**
     * @brief Retrieve the value of this Expression as a pointer to const.
     * @param ptr Reference to the pointer variable.
     * @return True if known, false if unknown.
     * @note Default method returns false and reports a "not implemented" error.
     */
    virtual bool getValuePointerImpl(T const *& ptr) const = 0;

    /**
     * @brief Retrieve the value vector and the known vector for array-valued expressions.
     * @param valuePtr Reference to the pointer variable to receive the value vector.
     * @param knownPtr Reference to the pointer variable to receive the known vector.
     * @return True if the value is known, false if unknown or invalid.
     * @note Default method returns false and reports a "not implemented" error.
     */
    virtual bool getArrayContentsImpl(T const *&valuePtr,
                                      std::vector<bool> const *&knownPtr) const;


    // Conversion wrapper, error if particular conversion not supported
    template <typename U>
    bool getValueImpl(U &result) const;

    // Error for wrong type call
    template <typename U>
    bool getValuePointerImpl(U const *&) const;

    // Default method returns false and reports a type error.
    template <typename U>
    bool getArrayContentsImpl(U const *&valuePtr,
                              std::vector<bool> const *&knownPtr) const;
  };
  
}

#endif // PLEXIL_EXPRESSION_IMPL_HH
