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

  //
  // Wrapper providing type conversions for getValue for templatized Expression derivatives
  //
  // If you wonder what's going on here, search for "Curiously Recurring Template Pattern".
  //

  template <class C>
  class ExpressionWrapper : public virtual Expression
  {
  public:
    // The only boilerplate code in this approach.
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

    bool getValue(std::vector<bool> &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    // bool getValue(std::vector<uint16_t> &result) const
    // {
    //   return static_cast<const C *>(this)->getValueImpl(result);
    // }

    bool getValue(std::vector<int32_t> &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(std::vector<double> &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }

    bool getValue(std::vector<std::string> &result) const
    {
      return static_cast<const C *>(this)->getValueImpl(result);
    }
  };

  //
  // Templatized Expression implementations should derive from this template
  //

  template <typename T>
  class ExpressionImpl : public ExpressionWrapper<ExpressionImpl<T> >
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

    // Conversion wrapper
    template <typename U>
    bool getValueImpl(U &result) const;
  };
  
}

#endif // PLEXIL_EXPRESSION_IMPL_HH
