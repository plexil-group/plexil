/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_CONVERSION_OPERATORS_HH
#define PLEXIL_CONVERSION_OPERATORS_HH

#include "OperatorImpl.hh"

namespace PLEXIL
{

  // Variant of OperatorImpl specifically for conversion operators
  
  template <typename NUM>
  class ConversionOperator : public OperatorImpl<NUM>
  {
  public:
    virtual ~ConversionOperator() = default;

    // Overrides
    virtual bool operator()(NUM &result, Expression const *arg0, Expression const *arg1) const;
    virtual bool operator()(NUM &result, Function const &args) const;

    virtual bool calc(NUM &result, Expression const *arg) const;
    virtual bool calcInternal(Real &result, Expression const *arg) const = 0;

  protected:
    ConversionOperator(std::string const &name);

  private:
    // Unimplemented
    ConversionOperator() = delete;
    ConversionOperator(ConversionOperator const &) = delete;
    ConversionOperator(ConversionOperator &&) = delete;
    ConversionOperator &operator=(ConversionOperator const &) = delete;
    ConversionOperator &operator=(ConversionOperator &&) = delete;
  };

  //
  // Real to Integer conversions
  //

  template <typename NUM>
  class Ceiling : public ConversionOperator<NUM>
  {
  public:
    Ceiling();
    virtual ~Ceiling() = default;
    virtual bool calcInternal(Real &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(Ceiling, NUM);

  private:
    Ceiling(const Ceiling &) = delete;
    Ceiling(Ceiling &&) = delete;
    Ceiling &operator=(const Ceiling &) = delete;
    Ceiling &operator=(Ceiling &&) = delete;
  };

  template <typename NUM>
  class Floor : public ConversionOperator<NUM>
  {
  public:
    Floor();
    virtual ~Floor() = default;
    virtual bool calcInternal(Real &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(Floor, NUM);

  private:
    Floor(const Floor &) = delete;
    Floor(Floor &&) = delete;
    Floor &operator=(const Floor &) = delete;
    Floor &operator=(Floor &&) = delete;
  };

  template <typename NUM>
  class Round : public ConversionOperator<NUM>
  {
  public:
    Round();
    virtual ~Round() = default;
    virtual bool calcInternal(Real &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(Round, NUM);

  private:
    Round(const Round &) = delete;
    Round(Round &&) = delete;
    Round &operator=(const Round &) = delete;
    Round &operator=(Round &&) = delete;
  };

  template <typename NUM>
  class Truncate : public ConversionOperator<NUM>
  {
  public:
    Truncate();
    virtual ~Truncate() = default;
    virtual bool calcInternal(Real &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(Truncate, NUM);

  private:
    Truncate(const Truncate &) = delete;
    Truncate(Truncate &&) = delete;
    Truncate &operator=(const Truncate &) = delete;
    Truncate &operator=(Truncate &&) = delete;
  };

  class RealToInteger : public OperatorImpl<Integer>
  {
  public:
    RealToInteger();
    virtual ~RealToInteger() = default;
    bool calc(Integer &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(RealToInteger, Integer);

  private:
    RealToInteger(const RealToInteger &) = delete;
    RealToInteger(RealToInteger &&) = delete;
    RealToInteger &operator=(const RealToInteger &) = delete;
    RealToInteger &operator=(RealToInteger &&) = delete;
  };

}

#endif // PLEXIL_CONVERSION_OPERATORS_HH
