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

#include "LabelStr.hh"
#include <stdint.h> // for uint64_t - NOT <cstdint>

namespace PLEXIL {

#define IS_TAG(x) (strcmp (tagName, x) == 0)

  /**
   * @brief Helper method to test if a char* is a valid Boolean value.
   */
  bool isXmlBoolean(const char* data);

  /**
   * @brief Helper method to test if a char* is a valid Boolean value. If it is, will write it to value.
   */
  bool isXmlBoolean(const char* data, bool& value);

  /**
   * @brief Helper method to test if a char* is numeric.
   */
  bool isNumber(const char* data);

  /**
   * @brief Helper method to test if a char* is numeric. If it is, will write it to value.
   */
  bool isNumber(const char* data, double& value);

  /**
   * @brief Helper method to test if a char* is a valid 32-bit integer value.
   */
  bool isInt32(const char* data);

  /**
   * @brief Helper method to test if a char* is a valid 32-bit integer value. If it is, will write it to value.
   */
  bool isInt32(const char* data, int32_t& value);

  /**
   * @brief Helper method to test if a char* is a valid 64-bit integer value.
   */
  bool isInt64(const char* data);

  /**
   * @brief Helper method to test if a char* is a valid 64-bit integer value. If it is, will write it to value.
   */
  bool isInt64(const char* data, int64_t& value);

  /**
   * @brief Helper method to test if a char* is a valid 64-bit hexBinary value.
   */
  bool isHexBinary(const char* data);

  /**
   * @brief Helper method to test if a char* is a valid hexBinary value. If it is, will write it to value.
   */
  bool isHexBinary(const char* data, uint64_t& value);

}

#endif
