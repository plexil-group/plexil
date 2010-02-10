/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

#ifndef DEFINE_IPC_MESSAGE_TYPES_H
#define DEFINE_IPC_MESSAGE_TYPES_H

#include "ipc.h"
#include "ipc-data-formats.h"

/**
 * @brief Bounds check the supplied message type.
 * @param mtyp The message type value to check.
 * @return true if valid, false if not.
 */
inline bool msgTypeIsValid(const PlexilMsgType mtyp)
{
  return (mtyp > PlexilMsgType_uninited) && (mtyp < PlexilMsgType_limit);
}

/**
 * @brief Return the message format string corresponding to the message type.
 * @param typ The message type.
 * @return Const char pointer to the message format name.
 */
inline const char* msgFormatForType(const PlexilMsgType typ)
{
  switch (typ)
    {
    case PlexilMsgType_NotifyExec:
    case PlexilMsgType_TerminateChangeLookup:

      return MSG_BASE;
      break;

    case PlexilMsgType_AddPlan:
    case PlexilMsgType_AddPlanFile:
    case PlexilMsgType_AddLibrary:
    case PlexilMsgType_AddLibraryFile:
    case PlexilMsgType_Command:
    case PlexilMsgType_Message:
    case PlexilMsgType_LookupNow:
    case PlexilMsgType_LookupOnChange:
    case PlexilMsgType_PlannerUpdate:
    case PlexilMsgType_StringValue:
    case PlexilMsgType_TelemetryValues:

      return STRING_VALUE_MSG;
      break;

    case PlexilMsgType_ReturnValues:

      return RETURN_VALUE_MSG;
      break;

    case PlexilMsgType_NumericValue:

      return NUMERIC_VALUE_MSG;
      break;

    case PlexilMsgType_PairNumeric:
      
      return NUMERIC_PAIR_MSG;
      break;

    case PlexilMsgType_PairString:

      return STRING_PAIR_MSG;
      break;
			  
    default:

      return NULL;
      break;
    }
}

/**
 * @brief Ensure the whole suite of message types is defined
 * @return true if successful, false otherwise
 * @note Caller should ensure IPC_initialize() has been called first
*/
bool definePlexilIPCMessageTypes()
{
  IPC_RETURN_TYPE status;
  if (!IPC_isMsgDefined(MSG_BASE))
    {
      if (IPC_errno != IPC_No_Error)
	return false;
      status = IPC_defineMsg(MSG_BASE, IPC_VARIABLE_LENGTH, MSG_BASE_FORMAT);
      if (status != IPC_OK)
	return false;
    }
  if (!IPC_isMsgDefined(RETURN_VALUE_MSG))
    {
      if (IPC_errno != IPC_No_Error)
	return false;
      status = IPC_defineMsg(RETURN_VALUE_MSG, IPC_VARIABLE_LENGTH, RETURN_VALUE_MSG_FORMAT);
      if (status != IPC_OK)
	return false;
    }
  if (!IPC_isMsgDefined(NUMERIC_VALUE_MSG))
    {
      if (IPC_errno != IPC_No_Error)
	return false;
      status = IPC_defineMsg(NUMERIC_VALUE_MSG, IPC_VARIABLE_LENGTH, NUMERIC_VALUE_MSG_FORMAT);
      if (status != IPC_OK)
	return false;
    }
  if (!IPC_isMsgDefined(STRING_VALUE_MSG))
    {
      if (IPC_errno != IPC_No_Error)
	return false;
      status = IPC_defineMsg(STRING_VALUE_MSG, IPC_VARIABLE_LENGTH, STRING_VALUE_MSG_FORMAT);
      if (status != IPC_OK)
	return false;
    }
  if (!IPC_isMsgDefined(NUMERIC_PAIR_MSG))
    {
      if (IPC_errno != IPC_No_Error)
	return false;
      status = IPC_defineMsg(NUMERIC_PAIR_MSG, IPC_VARIABLE_LENGTH, NUMERIC_PAIR_MSG_FORMAT);
      if (status != IPC_OK)
	return false;
    }
  if (!IPC_isMsgDefined(STRING_PAIR_MSG))
    {
      if (IPC_errno != IPC_No_Error)
	return false;
      status = IPC_defineMsg(STRING_PAIR_MSG, IPC_VARIABLE_LENGTH, STRING_PAIR_MSG_FORMAT);
      if (status != IPC_OK)
	return false;
    }
  return true;
}

#endif /* DEFINE_IPC_MESSAGE_TYPES_H */
