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

#include "XMLUtils.hh"
#include "Debug.hh"

/**
 * @author Conor McGann
 */

#include <cerrno>
#include <cstring>
#include <cstdlib>

namespace PLEXIL 
{

  bool isXmlBoolean(const char* data)
  {
	bool dummy;
	return isXmlBoolean(data, dummy);
  }

  bool isXmlBoolean(const char* data, bool& value)
  {
	if (strcmp(data, "0")
		|| strcmp(data, "false")) {
	  value = false;
	  return true;
	}
	else if (strcmp(data, "1")
			 || strcmp(data, "true")) {
	  value = true;
	  return true;
	}
	return false;
  }

  bool isNumber(const char* data){
    double dummy;
    return isNumber(data, dummy);
  }

  bool isNumber(const char* data, double& value){
    char * pEnd;
    value = strtod(data, &pEnd);

    // If successful then quit now
    if (pEnd != data)
      return true;
    else
      return false;
  }


  /**
   * @brief Helper method to test if a char* is a valid integer value.
   */
  bool isInt32(const char* data)
  {
	int32_t dummy;
	return isInt32(data, dummy);
  }

  /**
   * @brief Helper method to test if a char* is a valid integer value. If it is, will write it to value.
   */
  bool isInt32(const char* data, int32_t& value)
  {
	char* pEnd;
	value = strtol(data, &pEnd, 10);
	if (errno == EINVAL     // conversion could not be performed
		|| pEnd == data     // string did not start with digits
		|| *pEnd != '\0'     // junk after number
		|| errno == ERANGE) // too large to represent as long
	  return false;
	else return true;
  }

#if !defined(__VXWORKS__) /* platform lacks strtoll */
  /**
   * @brief Helper method to test if a char* is a valid integer value.
   */
  bool isInt64(const char* data)
  {
	int64_t dummy;
	return isInt64(data, dummy);
  }

  /**
   * @brief Helper method to test if a char* is a valid integer value. If it is, will write it to value.
   */
  bool isInt64(const char* data, int64_t& value)
  {
	char* pEnd;
	value = strtoll(data, &pEnd, 10);
	if (errno == EINVAL     // conversion could not be performed
		|| pEnd == data     // string did not start with digits
		|| *pEnd != '\0'     // junk after number
		|| errno == ERANGE) // too large to represent as long
	  return false;
	else return true;
  }

  /**
   * @brief Helper method to test if a char* is a valid 64-bit hexBinary value.
   */
  bool isHexBinary(const char* data)
  {
	uint64_t dummy;
	return isHexBinary(data, dummy);
  }

  /**
   * @brief Helper method to test if a char* is a valid 64-bit hexBinary value. If it is, will write it to value.
   */
  bool isHexBinary(const char* data, uint64_t& value)
  {
	char* pEnd;
	value = strtoll(data, &pEnd, 16);
	if (errno == EINVAL     // conversion could not be performed
		|| pEnd == data     // string did not start with digits
		|| *pEnd != '\0'     // junk after number
		|| errno == ERANGE) // too large to represent as long
	  return false;
	else return true;
  }
#endif

}
