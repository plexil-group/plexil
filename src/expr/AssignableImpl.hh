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

#ifndef PLEXIL_ASSIGNABLE_IMPL_HH
#define PLEXIL_ASSIGNABLE_IMPL_HH

#include "Assignable.hh"
#include "ArrayImpl.hh"
#include "Value.hh"

namespace PLEXIL
{
  /**
   * @class AssignableShim
   * @brief CRTP shim between virtual base class Assignable and its typed derivations.
   */
  template <class IMPL>
  class AssignableShim : public Assignable
  {
  public:
    AssignableShim() = default;
    ~AssignableShim() = default;

    //
    // setValue
    //

    void setValue(Expression const *valex)
    {
      static_cast<IMPL *>(this)->setValueImpl(valex);
    }

    // Convenience member function
    void setValue(char const *val)
    {
      static_cast<IMPL *>(this)->setValueImpl(String(val));
    }

    // This didn't work for a variety of reasons.
    // template <typename V>
    // void setValue(V const &val)
    // {
    //   static_cast<IMPL *>(this)->setValueImpl(val);
    // }

    // Local macro
#define DEFINE_SET_VALUE_METHOD(_type_) \
    void setValue(_type_ const &val) \
    {static_cast<IMPL *>(this)->setValueImpl(val);}

    DEFINE_SET_VALUE_METHOD(Value)
    DEFINE_SET_VALUE_METHOD(Boolean)
    DEFINE_SET_VALUE_METHOD(Integer)
    DEFINE_SET_VALUE_METHOD(Real)
    DEFINE_SET_VALUE_METHOD(NodeState)
    DEFINE_SET_VALUE_METHOD(NodeOutcome)
    DEFINE_SET_VALUE_METHOD(FailureType)
    DEFINE_SET_VALUE_METHOD(CommandHandleValue)
    DEFINE_SET_VALUE_METHOD(String)
    DEFINE_SET_VALUE_METHOD(BooleanArray)
    DEFINE_SET_VALUE_METHOD(IntegerArray)
    DEFINE_SET_VALUE_METHOD(RealArray)
    DEFINE_SET_VALUE_METHOD(StringArray)

#undef DEFINE_SET_VALUE_METHOD

    //
    // getMutableValuePointer
    //

    // Local macro
#define DEFINE_GET_MUTABLE_VALUE_POINTER_METHOD(_type_) \
    bool getMutableValuePointer(_type_ *&ptr) \
    {return static_cast<IMPL *>(this)->getMutableValuePointerImpl(ptr);}

    DEFINE_GET_MUTABLE_VALUE_POINTER_METHOD(String)
    DEFINE_GET_MUTABLE_VALUE_POINTER_METHOD(Array)
    DEFINE_GET_MUTABLE_VALUE_POINTER_METHOD(BooleanArray)
    DEFINE_GET_MUTABLE_VALUE_POINTER_METHOD(IntegerArray)
    DEFINE_GET_MUTABLE_VALUE_POINTER_METHOD(RealArray)
    DEFINE_GET_MUTABLE_VALUE_POINTER_METHOD(StringArray)

#undef DEFINE_GET_MUTABLE_VALUE_POINTER_METHOD

  };

  /**
   * @class AssignableImpl
   * @brief Typed implementation derived class of Assignable.
   */

  // Scalar case
  template <typename T>
  class AssignableImpl : public AssignableShim<AssignableImpl<T> >
  {
  public:
    AssignableImpl() = default;
    virtual ~AssignableImpl() = default;

    // To be defined by derived classes.
    virtual void setValueImpl(T const &val) = 0;

    // Type mismatch methods.
    virtual void setValueImpl(char const *val);

    // Generalized type mismatch
    template <typename U>
    void setValueImpl(U const &val);

    // Delegate to typed setValueImpl() methods.
    void setValueImpl(Expression const *valex);
    void setValueImpl(Value const &val);

    // Error for scalar types
    template <typename U>
    bool getMutableValuePointerImpl(U *& ptr);
  };

  // Special case for string
  template <>
  class AssignableImpl<String> : public AssignableShim<AssignableImpl<String> >
  {
  public:
    AssignableImpl() = default;
    virtual ~AssignableImpl() = default;

    // To be defined by derived classes.
    virtual void setValueImpl(String const &val) = 0;

    // Type mismatch methods.
    // Can be overridden for conversions (e.g. for string).
    virtual void setValueImpl(char const *val);

    // Generalized type mismatch
    template <typename U>
    void setValueImpl(U const &val);

    // Delegate to typed setValueImpl() methods.
    void setValueImpl(Expression const *valex);
    void setValueImpl(Value const &val);

    virtual bool getMutableValuePointerImpl(String *& ptr) = 0;

    // Type mismatch
    template <typename U>
    bool getMutableValuePointerImpl(U *& ptr);
  };

  // Array variant
  template <typename T>
  class AssignableImpl<ArrayImpl<T> > : public AssignableShim<AssignableImpl<ArrayImpl<T> > >
  {
  public:
    AssignableImpl() = default;
    virtual ~AssignableImpl() = default;

    // To be defined by derived classes.
    virtual void setValueImpl(ArrayImpl<T> const &val) = 0;

    // Type mismatch
    void setValueImpl(char const *val);

    // Generalized type mismatch
    template <typename U>
    void setValueImpl(U const &val);

    // Delegate to typed setValueImpl() methods.
    void setValueImpl(Expression const *valex);
    void setValueImpl(Value const &val);

    // To be defined by derived classes.
    virtual bool getMutableValuePointerImpl(ArrayImpl<T> *& ptr) = 0;

    // Downcast wrapper for above
    bool getMutableValuePointerImpl(Array *&ptr);

    // Type mismatch
    template <typename U>
    bool getMutableValuePointerImpl(U *& ptr);
  };

} // namespace PLEXIL

#endif // PLEXIL_ASSIGNABLE_IMPL_HH
