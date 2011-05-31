/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#ifndef H_XML_Utils
#define H_XML_Utils

/**
 * @author Conor McGann
 */

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include "tinyxml.h"
#include "LabelStr.hh"

namespace PLEXIL {

#define IS_TAG(x) (strcmp (tagName, x) == 0)

  /**
   * @brief Utility to extract an argument from an xml element
   */
  LabelStr extractData(const TiXmlElement& element, const LabelStr& argName);

  /**
   * @brief Helper function to parse a given XML string
   */
  TiXmlElement* initXml(const std::string& xmlStr);

  /**
   * @brief Extract text at this node, adding error checks
   */
  const char* getTextChild (const TiXmlElement& element);

  /**
   * @brief Helper method to test if a char* is a valid Boolean value.
   */
  bool isXmlBoolean(const char* data);

  /**
   * @brief Helper method to test if a char* is numeric. If it is, will write it to data.
   */
  bool isXmlBoolean(const char* data, bool& value);

  /**
   * @brief Helper method to test if a char* is numeric.
   */
  bool isNumber(const char* data);

  /**
   * @brief Helper method to test if a char* is numeric. If it is, will write it to data.
   */
  bool isNumber(const char* data, double& value);
}

#endif
