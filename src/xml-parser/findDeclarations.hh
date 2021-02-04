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

// Utilities to find declarations of PLEXIL objects in XML

#ifndef PLEXIL_FIND_DECLARATIONS_HH
#define PLEXIL_FIND_DECLARATIONS_HH

#include "pugixml.hpp"

namespace PLEXIL
{

  // Search upward from elt for a containing Node element.
  pugi::xml_node const findContainingNodeElement(pugi::xml_node const elt);

  // Look for a declaration with the given name in the element.
  pugi::xml_node const findNameInDeclarations(pugi::xml_node const elt,
                                              char const *name);
  
  // Find the first in-scope variable declaration with the given variable name.
  pugi::xml_node const findVariableDeclaration(pugi::xml_node const elt,
                                               char const *name);

  // Look for a declaration with the given tag and name in the element
  pugi::xml_node const findTagInDeclarations(pugi::xml_node const elt,
                                             char const *tag,
                                             char const *name);
  
  // Find the first in-scope array declaration with the given variable name.
  pugi::xml_node const findArrayDeclaration(pugi::xml_node const elt, char const *name);

} // namespace PLEXIL

#endif // PLEXIL_FIND_DECLARATIONS_HH
