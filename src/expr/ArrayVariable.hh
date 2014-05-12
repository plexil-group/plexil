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
  class ArrayVariable : public UserVariable<std::vector<T> >
  {
  public:
    ArrayVariable();

    // Regression testing only - to be removed
    ArrayVariable(std::vector<T> const & initVal);

    /**
     * @brief Constructor for plan loading.
     * @param node The node to which this variable belongs (default none).
     * @param name The name of this variable in the parent node.
     */
    ArrayVariable(const NodeId &node,
                  const std::string &name = "",
                  const ExpressionId &size = ExpressionId::noId(),
                  const ExpressionId &initializer = ExpressionId::noId(),
                  bool sizeIsGarbage = false,
                  bool initializerIsGarbage = false);

    virtual ~ArrayVariable();

    // Specializations from UserVariable
    void reset();
    void handleActivate();
    void setValue(std::vector<T> const &val);
    void setValue(ExpressionId const &valex);
    void saveCurrentValue();
    void restoreSavedValue();

    /**
     * @brief Retrieve the value vector and the known vector for array-valued expressions.
     * @param valuePtr Reference to the pointer variable to receive the value vector.
     * @param knownPtr Reference to the pointer variable to receive the known vector.
     * @return True if the value is known, false if unknown or invalid.
     */
    bool getArrayContentsImpl(std::vector<T> const *&valuePtr,
                              std::vector<bool> const *&knownPtr) const;

    /**
     * @brief Retrieve the (writable) value vector and known vector for array-valued expressions.
     * @param valuePtr Reference to the pointer variable to receive the value vector.
     * @param knownPtr Reference to the pointer variable to receive the known vector.
     * @return True if the value is known, false if unknown or invalid.
     */
    bool getMutableArrayContents(std::vector<T> *&valuePtr,
                                 std::vector<bool> *&knownPtr);

    /**
     * @brief Get a pointer to the vector of element-known flags.
     * @param ptr Place to store the pointer.
     * @return True if array value itself is known, false if unknown or invalid.
     * @note Return value of false means no pointer was assigned.
     */
    bool getMutableKnownVectorPointer(std::vector<bool> *&ptr);

  private:
    // Convenience typedefs
    typedef UserVariable<std::vector<T> > Superclass;

    /**
     * @brief Pre-allocate storage based on the current value of the size expression.
     */
    void reserve();

    std::vector<bool> m_elementKnown;
    std::vector<bool> m_savedElementKnown;

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
