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

#ifndef PLEXIL_EXPRESSION_LISTENER_HH
#define PLEXIL_EXPRESSION_LISTENER_HH

namespace PLEXIL
{

  /**
   * @brief Stateless abstract base class for listeners in the expression notification graph.
   * An expression listener may listen to multiple expressions.
   */

  class ExpressionListener
  {
  public:
    ExpressionListener() = default;
    virtual ~ExpressionListener() = default;
    
    /**
     * @brief Virtual function for notification that an expression's value has changed.
     */
    virtual void notifyChanged() = 0;

  private:
    // Not implemented
    ExpressionListener(ExpressionListener const &) = delete;
    ExpressionListener(ExpressionListener &&) = delete;
    ExpressionListener &operator=(ExpressionListener const &) = delete;
    ExpressionListener &operator=(ExpressionListener &&) = delete;
  };

}

#endif // PLEXIL_EXPRESSION_LISTENER_HH
