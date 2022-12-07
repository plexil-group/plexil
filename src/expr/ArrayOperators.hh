// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_ARRAY_OPERATORS_HH
#define PLEXIL_ARRAY_OPERATORS_HH

#include "OperatorImpl.hh"

namespace PLEXIL
{

  //! \class ArraySize
  //! \brief Implements the ArraySize operator.
  //! \ingroup Expressions
  class ArraySize : public OperatorImpl<Integer>
  {
  public:
    ArraySize();
    virtual ~ArraySize() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(Integer &result, Expression const *arg) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(ArraySize);

  //! \class ArrayMaxSize
  //! \brief Implements the ArrayMaxSize operator.
  //! \ingroup Expressions
  private:
    // Disallow copy, assign
    ArraySize(ArraySize const &) = delete;
    ArraySize(ArraySize &&) = delete;
    ArraySize &operator=(ArraySize const &) = delete;
    ArraySize &operator=(ArraySize &&) = delete;
  };

  class ArrayMaxSize : public OperatorImpl<Integer>
  {
  public:
    ArrayMaxSize();
    virtual ~ArrayMaxSize() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(Integer &result, Expression const *arg) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(ArrayMaxSize);

  private:
    // Disallow copy, assign
    ArrayMaxSize(ArrayMaxSize const &) = delete;
    ArrayMaxSize(ArrayMaxSize &&) = delete;
    ArrayMaxSize &operator=(ArrayMaxSize const &) = delete;
    ArrayMaxSize &operator=(ArrayMaxSize &&) = delete;
  };

  //! \class AllElementsKnown
  //! \brief Implements the AllElementsKnown operator.
  //! \ingroup Expressions
  class AllElementsKnown : public OperatorImpl<Boolean>
  {
  public:
    AllElementsKnown();
    virtual ~AllElementsKnown() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(Boolean &result, Expression const *arg) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(AllElementsKnown);

  private:
    // Disallow copy, assign
    AllElementsKnown(AllElementsKnown const &) = delete;
    AllElementsKnown(AllElementsKnown &&) = delete;
    AllElementsKnown &operator=(AllElementsKnown const &) = delete;
    AllElementsKnown &operator=(AllElementsKnown &&) = delete;
  };

  //! \class AnyElementsKnown
  //! \brief Implements the AnyElementsKnown operator.
  //! \ingroup Expressions
  class AnyElementsKnown : public OperatorImpl<Boolean>
  {
  public:
    AnyElementsKnown();
    virtual ~AnyElementsKnown() = default;

    bool checkArgCount(size_t count) const;

    bool checkArgTypes(std::vector<ValueType> const &typeVec) const;

    bool operator()(Boolean &result, Expression const *arg) const;

    DECLARE_OPERATOR_STATIC_INSTANCE(AnyElementsKnown);

  private:
    // Disallow copy, assign
    AnyElementsKnown(AnyElementsKnown const &) = delete;
    AnyElementsKnown(AnyElementsKnown &&) = delete;
    AnyElementsKnown &operator=(AnyElementsKnown const &) = delete;
    AnyElementsKnown &operator=(AnyElementsKnown &&) = delete;
  };

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_OPERATORS_HH
