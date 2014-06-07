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

#ifndef PLEXIL_FUNCTION_FACTORY_HH
#define PLEXIL_FUNCTION_FACTORY_HH

#include "ExpressionFactory.hh"

namespace PLEXIL
{

  template <class OP, typename RETURNS>
  class FunctionFactory : public ExpressionFactory
  {
  public:
    FunctionFactory(std::string const &name);
    virtual ~FunctionFactory();

  protected:
    virtual ExpressionId create(PlexilExprId const &expr,
                                NodeConnectorId const &node = NodeConnectorId::noId());
  };

} // namespace PLEXIL

// Convenience macros
#define ENSURE_FUNCTION_FACTORY(CLASS) template class PLEXIL::FunctionFactory<CLASS >;
#define REGISTER_FUNCTION(CLASS,NAME) {new PLEXIL::FunctionFactory<CLASS >(#NAME);}

#endif // PLEXIL_FUNCTION_FACTORY_HH
