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

#ifndef PLEXIL_ARRAY_VARIABLE_HH
#define PLEXIL_ARRAY_VARIABLE_HH

#include "UserVariable.hh"

namespace PLEXIL
{

  /**
   * @class ArrayVariable
   * @brief A class derived from UserVariable, which adds accessors required
   *        by the ArrayReference and MutableArrayReference expression classes.
   */

  template <typename T>
  class ArrayVariable : public UserVariable<ArrayImpl<T> >
  {
  public:
    ArrayVariable();

    // Regression testing only - to be removed
    ArrayVariable(ArrayImpl<T> const & initVal);

    /**
     * @brief Constructor for plan loading.
     * @param node The node to which this variable belongs (default none).
     * @param name The name of this variable in the parent node.
     */
    ArrayVariable(const NodeConnectorId &node,
                  const std::string &name = "",
                  const ExpressionId &size = ExpressionId::noId(),
                  const ExpressionId &initializer = ExpressionId::noId(),
                  bool sizeIsGarbage = false,
                  bool initializerIsGarbage = false);

    virtual ~ArrayVariable();

    // Specializations from UserVariable
    void handleActivate();
    bool getValuePointerImpl(Array const *&ptr) const;
    bool getMutableValuePointer(Array *&ptr);

  private:
    // Convenience typedefs
    typedef UserVariable<ArrayImpl<T> > Superclass;

    /**
     * @brief Pre-allocate storage based on the current value of the size expression.
     */
    void reserve();

    ExpressionId m_size;
    bool m_sizeIsGarbage;
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
