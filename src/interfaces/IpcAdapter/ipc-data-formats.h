/* Copyright (c) 2006-2009, Universities Space Research Association (USRA).
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

/*
 * Data formats used by IpcAdapter
 */

struct PlexilMsgBase
{
  uint16_t msgType;
  uint16_t count;   /* either # of msgs to follow, or index of this msg in the sequence */
  uint32_t serial;  /* uniquely IDs this message sequence from this sender */
  const char* senderUID;  /* uniquely IDs the sender's IP and process */
};

#define MSG_BASE "PlexilMessageBase"
#define MSG_BASE_FORMAT "{ushort, ushort, uint, string}"

/*
 * Used for return values
 * Followed by count data messages
 */

struct PlexilReturnValuesMsg
{
  struct PlexilMsgBase header;
  uint32_t requestSerial;
  const char* requesterUID;
};

#define RETURN_VALUE_MSG "PlexilReturnValueMessage"
#define RETURN_VALUE_MSG_FORMAT "{ushort, ushort, uint, string, uint, string}"

/*
 * Used for numeric argument or return values
 */

struct PlexilNumericValueMsg
{
  struct PlexilMsgBase header;
  double doubleValue;
};

#define NUMERIC_VALUE_MSG "PlexilNumericValue"
#define NUMERIC_VALUE_MSG_FORMAT "{ushort, ushort, uint, string, double}"

/*
 * When used for commands, messages, lookups:
 *  followed by count parameter values
 * When used for planner updates: 
 *  string value is node name, followed by count name/value pairs
 * When used for plans, plan files, libraries, library files: 
 *  stands alone, count ignored
 * When used for numeric argument or return values:
 *  preceded by corresponding message type, 
 *  count indicates position in sequence
 */

struct PlexilStringValueMsg
{
  struct PlexilMsgBase header;
  const char* stringValue;
};

#define STRING_VALUE_MSG "PlexilStringValue"
#define STRING_VALUE_MSG_FORMAT "{ushort, ushort, uint, string, string}"

/*
 * Arrays and other argument types here (NYI)
 */

struct PairHeader
{
  struct PlexilMsgBase datumHeader;
  const char* pairName;
};

struct NumericPair
{
  struct PairHeader pairHeader;
  double pairDoubleValue;
};

#define NUMERIC_PAIR_MSG "PlexilNumericPair"
#define NUMERIC_PAIR_MSG_FORMAT "{ushort, ushort, uint, string, string, double}"

struct StringPair
{
  struct PairHeader pairHeader;
  double stringDoubleValue;
};

#define STRING_PAIR_MSG "PlexilStringPair"
#define STRING_PAIR_MSG_FORMAT "{ushort, ushort, uint, string, string, string}"

typedef enum
  {
    PlexilMsgType_uninited=0,

    // PlexilMsgBase
    // These messages are complete unto themselves
    PlexilMsgType_NotifyExec,
    PlexilMsgType_TerminateChangeLookup,

    // PlexilStringValueMsg
    // These have an operation name and an argument count
    PlexilMsgType_Command,
    PlexilMsgType_Message,
    PlexilMsgType_LookupNow,
    PlexilMsgType_LookupOnChange,

    // PlexilReturnValuesMsg
    // These have a unique ID of the requested operation
    // and count of values (which may be 0).
    PlexilMsgType_ReturnValues,

    // PlexilStringValueMsg
    // These have one (non-empty?) string datum, the plan or file name
    // Count must be 0
    PlexilMsgType_AddPlan,
    PlexilMsgType_AddPlanFile,
    PlexilMsgType_AddLibrary,
    PlexilMsgType_AddLibraryFile,

    // PlexilStringValueMsg
    // These have a node name and a pair count (which may be 0?)
    PlexilMsgType_PlannerUpdate,

    // PlexilNumericValueMsg
    // A simple numeric datum
    // Count indicates position in sequence
    PlexilMsgType_NumericValue,

    // PlexilStringValueMsg
    // A simple string datum
    // Count indicates position in sequence
    PlexilMsgType_StringValue,

    // A pair of a name and a numeric value
    // Count indicates position in sequence
    PlexilMsgType_PairNumeric,

    // A pair of a name and a string value
    // Count indicates position in sequence
    PlexilMsgType_PairString,

    PlexilMsgType_limit
  }
  PlexilMsgType;

inline bool msgTypeIsValid(const PlexilMsgType mtyp)
{
  return (mtyp > PlexilMsgType_uninited) && (mtyp < PlexilMsgType_limit);
}

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
