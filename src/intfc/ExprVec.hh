// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
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

#ifndef PLEXIL_EXPR_VEC_HH
#define PLEXIL_EXPR_VEC_HH

#include "Listenable.hh" // ListenableUnaryOperator type

#include <cstddef> // size_t
#include <iosfwd> // std::ostream

namespace PLEXIL
{
  // Forward references
  class Expression;

  //! \class ExprVec
  //! \brief Pure virtual base class for a family of expression vector classes,
  //!        whose representations vary by size.
  //! \ingroup External-Interface
  class ExprVec
  {
  public:

    //! \brief Virtual destructor.
    virtual ~ExprVec() = default;

    //! \brief Get the size of this vector.
    //! \return The size.
    virtual size_t size() const = 0;

    //! \brief Get the expression at the given index.
    //! \param n The index.
    //! \return Const pointer to the expression.
    virtual Expression const *operator[](size_t n) const = 0;

    //! \brief Get the expression at the given index.
    //! \param n The index.
    //! \return Pointer to the expression.
    virtual Expression *operator[](size_t n) = 0;

    //! \brief Set the expression at the index.
    //! \param i The index.
    //! \param exp Pointer to the expression.
    //! \param garbage true if the expression should be deleted with this object, false otherwise.
    virtual void setArgument(size_t i, Expression *exp, bool garbage) = 0;

    //! \brief Add an expression listener to this object.
    //! \param l Pointer to the listener.
    virtual void addListener(ExpressionListener *l) = 0;

    //! \brief Remove an expression listener from this object.
    //! \param l Pointer to the listener.
    virtual void removeListener(ExpressionListener *l) = 0;

    //! \brief Call a function on all subexpressions of this object.
    //! \param oper A functor; it must implement an operator() method
    //!             of one argument, a pointer to Listenable,
    //!             returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &f) = 0;

    //! \brief Print this object to an output stream.
    //! \param str Reference to the stream
    virtual void print(std::ostream &s) const = 0;

    //! \brief Make this object active if it is not already.
    virtual void activate() = 0;

    //! \brief Request that this object become inactive if it is not already.
    virtual void deactivate() = 0;
  };

  //! \brief Construct an ExprVec of the given size.
  //! \param nargs The size.
  //! \return Pointer to the freshly allocated object.
  //! \ingroup External-Interface
  extern ExprVec *makeExprVec(size_t nargs);

} // namespace PLEXIL

#endif // PLEXIL_EXPR_VEC_HH
