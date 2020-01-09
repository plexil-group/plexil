/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_CONVERSION_FUNCTION_FACTORY_HH
#define PLEXIL_CONVERSION_FUNCTION_FACTORY_HH

#include "FunctionFactory.hh"

namespace PLEXIL
{

  /**
   * @class ConversionFunctionFactory
   * @brief A specialization of ExpressionFactory which selects the appropriate
   * Function and Operator templates, based on the parameter type(s).
   */
  class ConversionFunctionFactory : public FunctionFactory
  {
  public:
    ConversionFunctionFactory(Operator const *integerOp,
                              Operator const *realOp,
                              std::string const &name);
    ~ConversionFunctionFactory();
    
    virtual ValueType check(char const *nodeId, pugi::xml_node const expr) const;

    Expression *allocate(pugi::xml_node const expr,
                         NodeConnector *node,
                         bool & wasCreated,
                         ValueType returnType = UNKNOWN_TYPE) const;

  protected:
    virtual Operator const *selectOperator(ValueType type) const;

    Operator const *m_intOp;
    Operator const *m_realOp;

  private:
    // Not implemented
    ConversionFunctionFactory();
    ConversionFunctionFactory(ConversionFunctionFactory const &);
    ConversionFunctionFactory &operator=(ConversionFunctionFactory const &);
  };

// Convenience macro
#define REGISTER_CONVERSION_FUNCTION(CLASS, NAME) \
  new PLEXIL::ConversionFunctionFactory(CLASS<Integer>::instance(), CLASS<Real>::instance(), #NAME)

}

#endif // PLEXIL_CONVERSION_FUNCTION_FACTORY_HH
