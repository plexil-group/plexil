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

#ifndef PLEXIL_SET_VALUE_HH
#define PLEXIL_SET_VALUE_HH

#include "GetValue.hh"

namespace PLEXIL
{

  /**
   * @class SetValue
   * @brief Stateless abstract base class representing the essential value-setting API for PLEXIL objects.
   */

  class SetValue
  {
  public:
    SetValue() = default;
    virtual ~SetValue() = default;

    /**
     * @brief Set the current value of this variable to "unknown".
     * @note May cause change notifications to occur.
     */
    virtual void setUnknown() = 0;

    /**
     * @brief Set the value for this object.
     * @param val The new value for this object.
     */

    // TODO? Move variants
    virtual void setValue(Boolean const &val) = 0;
    virtual void setValue(Integer const &val) = 0;
    virtual void setValue(Real const &val) = 0;
    virtual void setValue(NodeState const &val) = 0;
    virtual void setValue(NodeOutcome const &val) = 0;
    virtual void setValue(FailureType const &val) = 0;
    virtual void setValue(CommandHandleValue const &val) = 0;

    virtual void setValue(String const &val) = 0;
    virtual void setValue(char const *val) = 0; // convenience

    virtual void setValue(BooleanArray const &val) = 0;
    virtual void setValue(IntegerArray const &val) = 0;
    virtual void setValue(RealArray const &val) = 0;
    virtual void setValue(StringArray const &val) = 0;

    virtual void setValue(GetValue const &val) = 0;

    /**
     * @brief Retrieve a pointer to the non-const value.
     * @param valuePtr Reference to the pointer variable
     * @return True if the value is known, false if unknown or invalid.
     */
    virtual bool getMutableValuePointer(String *&ptr) = 0;
    virtual bool getMutableValuePointer(Array *&ptr) = 0;
    virtual bool getMutableValuePointer(BooleanArray *&ptr) = 0;
    virtual bool getMutableValuePointer(IntegerArray *&ptr) = 0;
    virtual bool getMutableValuePointer(RealArray *&ptr) = 0;
    virtual bool getMutableValuePointer(StringArray *&ptr) = 0;

  };

}

#endif // PLEXIL_SET_VALUE_HH
