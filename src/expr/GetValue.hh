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

#ifndef PLEXIL_GET_VALUE_HH
#define PLEXIL_GET_VALUE_HH

#include "CommandHandle.hh"
#include "NodeConstants.hh"
#include "ValueType.hh"

namespace PLEXIL
{
  class Value;

  //*
  // @class GetValue
  // @brief Abstract base class defining the getValue() API used
  //        throughout the PLEXIL code base.
  // @note This API should apply equally to dynamically typed (e.g. Value class)
  //       as well as statically typed objects.
  class GetValue
  {
  protected:
    GetValue() = default;

  private:
    GetValue(GetValue const &) = delete;
    GetValue(GetValue &&) = delete;

  public:
    virtual ~GetValue() = default;
    
    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     * @note May be overridden by derived classes.
     */
    virtual ValueType valueType() const = 0;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note May be overridden by derived classes.
     */
    virtual bool isKnown() const = 0;

    /**
     * @brief Get the value of this object as a Value instance.
     * @return The Value instance.
     */
    virtual Value toValue() const = 0;

    /**
     * @brief Print the object's value to the given stream.
     * @param s The output stream.
     */
    virtual void printValue(std::ostream& s) const = 0;

    //
    // The base class has to explicitly name all the potential types;
    // we can't use a template to declare pure virtual member functions
    // with a default method.
    //

    // 
    // If the above weren't bad enough, to maintain type consistency,
    // we have to use an out parameter instead of a return value for getValue()
    // because a return value might be implicitly promoted to the wrong type.
    // C++ sucks at polymorphism.
    //

    /**
     * @brief Retrieve the value of this object.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown or invalid.
     * @note The value is not copied if the return value is false.
     * @note Derived classes should implement only the appropriate methods.
     */

    virtual bool getValue(Boolean &result) const = 0;
    virtual bool getValue(NodeState &result) const = 0;
    virtual bool getValue(NodeOutcome &result) const = 0;
    virtual bool getValue(FailureType &result) const = 0;
    virtual bool getValue(CommandHandleValue &result) const = 0;
    virtual bool getValue(Integer &result) const = 0;
    virtual bool getValue(Real &result) const = 0;

    virtual bool getValue(String &result) const = 0;

    /**
     * @brief Retrieve a pointer to the (const) value of this object.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown or invalid.
     * @note The pointer is not copied if the return value is false.
     * @note Derived classes should implement only the appropriate methods.
     */

    virtual bool getValuePointer(String const *&ptr) const = 0;

    virtual bool getValuePointer(Array const *&ptr) const = 0;

    virtual bool getValuePointer(BooleanArray const *&ptr) const = 0;
    virtual bool getValuePointer(IntegerArray const *&ptr) const = 0;
    virtual bool getValuePointer(RealArray const *&ptr) const = 0;
    virtual bool getValuePointer(StringArray const *&ptr) const = 0;
  };

}

#endif // PLEXIL_GET_VALUE_HH
