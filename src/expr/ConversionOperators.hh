/* Copyright (c) 2006-2019, Universities Space Research Association (USRA).
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
  
  class ConversionOperator : public Operator
  {
  public:
    virtual ~ConversionOperator();

    virtual bool checkArgCount(size_t count) const;
    virtual bool checkArgTypes(Function const *ev) const;

    ValueType valueType() const;
    void *allocateCache() const;
    void deleteCache(void *ptr) const;

    bool isKnown(Function const &exprs) const;
    void printValue(std::ostream &s, Function const &exprs) const;
    Value toValue(Function const &exprs) const;

    virtual bool operator()(Integer &result, Expression const *arg) const;
    virtual bool operator()(Integer &result, Expression const *arg0, Expression const *arg1) const;
    virtual bool operator()(Integer &result, Function const &args) const;

    virtual bool operator()(Real &result, Expression const *arg) const;
    virtual bool operator()(Real &result, Expression const *arg0, Expression const *arg1) const;
    virtual bool operator()(Real &result, Function const &args) const;

    virtual bool calc(Real &result, Expression const *arg) const = 0;

  protected:
    ConversionOperator(std::string const &name);

  private:
    // Unimplemented
    ConversionOperator();
    ConversionOperator(ConversionOperator const &);
    ConversionOperator &operator=(ConversionOperator const &);
  };

  //
  // Real to Integer conversions
  //

  class Ceiling : public ConversionOperator
  {
  public:
    Ceiling();
    ~Ceiling();
    virtual bool calc(Real &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(Ceiling, Integer);

  private:
    Ceiling(const Ceiling &);
    Ceiling &operator=(const Ceiling &);
  };

  class Floor : public ConversionOperator
  {
  public:
    Floor();
    ~Floor();
    virtual bool calc(Real &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(Floor, Integer);

  private:
    Floor(const Floor &);
    Floor &operator=(const Floor &);
  };

  // Believe it or not, VxWorks 6.8 for PowerPC doesn't have round() or trunc()
#if !defined(__VXWORKS__)

  class Round : public ConversionOperator
  {
  public:
    Round();
    ~Round();
    virtual bool calc(Real &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(Round, Integer);

  private:
    Round(const Round &);
    Round &operator=(const Round &);
  };

  class Truncate : public ConversionOperator
  {
  public:
    Truncate();
    ~Truncate();
    virtual bool calc(Real &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(Truncate, Integer);

  private:
    Truncate(const Truncate &);
    Truncate &operator=(const Truncate &);
  };
#endif // !defined(__VXWORKS__)

  class RealToInteger : public OperatorImpl<Integer>
  {
  public:
    RealToInteger();
    ~RealToInteger();
    virtual bool checkArgCount(size_t count) const;
    bool checkArgTypes(Function const *ev) const;
    bool calc(Integer &result, Expression const *arg) const;
    DECLARE_OPERATOR_STATIC_INSTANCE(RealToInteger, Integer);

  private:
    RealToInteger(const RealToInteger &);
    RealToInteger &operator=(const RealToInteger &);
  };

}

#endif // PLEXIL_CONVERSION_OPERATORS_HH
