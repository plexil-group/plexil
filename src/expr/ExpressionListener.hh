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

#ifndef PLEXIL_EXPRESSION_LISTENER_HH
#define PLEXIL_EXPRESSION_LISTENER_HH

namespace PLEXIL
{

  //! \class ExpressionListener
  //! \brief Pure virtual base class for listeners in the change notification graph.

  //! ExpressionListener has no state. It only defines a pure virtual
  //! member function, notifyChanged.

  //! Instances derived from ExpressionListener can receive change
  //! notifications via notifyChanged.  The Listenable virtual base
  //! class defines the API of objects to which an ExpressionListener
  //! instance is intended to be attached.

  //! \see Listenable::addListener
  //! \see Listenable::removeListener
  //! \ingroup Expressions

  class ExpressionListener
  {
  public:

    //! \brief Virtual destructor.
    virtual ~ExpressionListener() = default;

    //! \brief Notify this object of a change.
    virtual void notifyChanged() = 0;

  };

}

#endif // PLEXIL_EXPRESSION_LISTENER_HH
