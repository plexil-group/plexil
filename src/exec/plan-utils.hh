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

// Utilities for Plexil plans.

#include "Value.hh"
#include <vector>

namespace PLEXIL 
{

// Printing Commands

//! \brief Print arguments to std::cout verbatim.
//! \param args Const reference to vector of Value instances.
void print (const std::vector<Value>& args);

//! \brief Print arguments to std::cout in a "pretty" way.
//! \param args Const reference to vector of Value instances.
void pprint (const std::vector<Value>& args);

//! \brief Print arguments to a PLEXIL string verbatim.
//! \param args Const reference to vector of Value instances.
//! \return A Value instance containing the result String.
Value printToString(const std::vector<Value>& args);

//! \brief Print arguments to a PLEXIL string in a "pretty" way.
//! \param args Const reference to vector of Value instances.
//! \return A Value instance containing the result String.
Value pprintToString(const std::vector<Value>& args);

}
