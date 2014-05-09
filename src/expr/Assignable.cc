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

#include "Assignable.hh"

namespace PLEXIL {

  //
  // Assignable
  //

  Assignable::Assignable()
    : Expression(),
      m_aid(this, m_id)
  {
  }

  Assignable::~Assignable()
  {
    m_aid.removeDerived(m_id);
  }

  bool Assignable::isAssignable() const
  {
    return true;
  }

  //
  // checkValue() default methods
  //
  // These methods always return true. This should be appropriate for most
  // user-defined variables.
  //
  // The intent is that user variables (and others) will be defined as templates,
  // with only the specific variables' template type supported,
  // and optionally others as appropriate to the type.
  //

  bool Assignable::checkValue(const bool & /* value */)
  {
    return true;
  }

  bool Assignable::checkValue(const uint16_t & /* value */)
  {
    return true;
  }

  bool Assignable::checkValue(const int32_t & /* value */)
  {
    return true;
  }

  bool Assignable::checkValue(const double & /* value */)
  {
    return true;
  }

  bool Assignable::checkValue(const std::string & /* value */)
  {
    return true;
  }

  bool Assignable::checkValue(const std::vector<bool> & /* value */)
  {
    return true;
  }

  bool Assignable::checkValue(const std::vector<uint16_t> & /* value */)
  {
    return true;
  }

  bool Assignable::checkValue(const std::vector<int32_t> & /* value */)
  {
    return true;
  }

  bool Assignable::checkValue(const std::vector<double> & /* value */)
  {
    return true;
  }

  bool Assignable::checkValue(const std::vector<std::string> & /* value */)
  {
    return true;
  }

  //
  // getMutableValuePointer() default methods
  //
  bool Assignable::getMutableValuePointer(std::vector<bool> *& /* ptr */)
  {
    check_error_2(ALWAYS_FAIL, "getMutableValuePointer: Variable is not a BooleanArray");
    return false;
  }

  bool Assignable::getMutableValuePointer(std::vector<int32_t> *& /* ptr */)
  {
    check_error_2(ALWAYS_FAIL, "getMutableValuePointer: Variable is not a BooleanArray");
    return false;
  }

  bool Assignable::getMutableValuePointer(std::vector<double> *& /* ptr */)
  {
    check_error_2(ALWAYS_FAIL, "getMutableValuePointer: Variable is not a BooleanArray");
    return false;
  }

  bool Assignable::getMutableValuePointer(std::vector<std::string> *& /* ptr */)
  {
    check_error_2(ALWAYS_FAIL, "getMutableValuePointer: Variable is not a BooleanArray");
    return false;
  }

  //
  // setValue() default methods
  //

  void Assignable::setValue(const double & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: Real is not a valid type for this expression");
  }

  void Assignable::setValue(const int32_t & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: Integer is not a valid type for this expression");
  }

  void Assignable::setValue(const uint16_t & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: Internal value is not a valid type for this expression");
  }

  void Assignable::setValue(const bool & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: Boolean is not a valid type for this expression");
  }

  void Assignable::setValue(const std::string & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: String is not a valid type for this expression");
  }

  void Assignable::setValue(const char * /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: Character string is not a valid type for this expression");
  }

  void Assignable::setValue(const std::vector<bool>  & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: BooleanArray is not a valid type for this expression");
  }

  // void Assignable::setValue(const std::vector<uint16_t> & /* val */)
  // {
  //   assertTrue_2(ALWAYS_FAIL, "setValue: Not a valid type for this expression");
  // }

  void Assignable::setValue(const std::vector<int32_t>  & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: IntegerArray is not a valid type for this expression");
  }

  void Assignable::setValue(const std::vector<double> & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: RealArray is not a valid type for this expression");
  }

  void Assignable::setValue(const std::vector<std::string> & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "setValue: String is not a valid type for this expression");
  }

  /**
   * @brief Get a pointer to the vector of element-known flags.
   * @param ptr Place to store the pointer.
   * @return True if array value itself is known, false if unknown or invalid.
   * @note Default method.
   */
  bool Assignable::getMutableKnownVectorPointer(std::vector<bool> *&ptr)
  {
    check_error_2(ALWAYS_FAIL, "getMutableKnownVectorPointer not implemented for this expression");
    return false;
  }

  /**
   * @brief Get the node that owns this expression.
   * @return The NodeId of the parent node; may be noId.
   * @note Used by LuvFormat::formatAssignment().  
   * @note Default method.
   */
  const NodeId& Assignable::getNode() const
  {
    return NodeId::noId(); 
  }

} // namespace PLEXIL
