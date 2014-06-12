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

#include "ExpressionConstants.hh"

namespace PLEXIL
{
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(BooleanConstant, FALSE_CONSTANT, false);
  DEFINE_GLOBAL_CONST_WITH_CLEANUP(BooleanConstant, TRUE_CONSTANT, true);
  DEFINE_GLOBAL_EMPTY_CONST_WITH_CLEANUP(BooleanConstant, UNKNOWN_BOOLEAN_CONSTANT);

  ExpressionId const &FALSE_EXP()
  {
    static ExpressionId sl_falseId(FALSE_CONSTANT().getId());
    return sl_falseId;
  }

  ExpressionId const &TRUE_EXP()
  {
    static ExpressionId sl_trueId(TRUE_CONSTANT().getId());
    return sl_trueId;
  }

  ExpressionId const &UNKNOWN_BOOLEAN_EXP()
  {
    static ExpressionId sl_unknownId(UNKNOWN_BOOLEAN_CONSTANT().getId());
    return sl_unknownId;
  }

} // namespace PLEXIL

