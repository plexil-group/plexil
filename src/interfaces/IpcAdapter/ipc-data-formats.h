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

typedef enum
  {
    PlexilMsgType_uninited=0,
    PlexilMsgType_Command,
    PlexilMsgType_Message,
    PlexilMsgType_LookupNow,
    PlexilMsgType_LookupOnChange,
    PlexilMsgType_PlannerUpdate,
    PlexilMsgType_AddPlan,
    PlexilMsgType_AddPlanFile,
    PlexilMsgType_AddLibrary,
    PlexilMsgType_AddLibraryFile,
    PlexilMsgType_ReturnValues,
    PlexilMsgType_NumericValue,
    PlexilMsgType_StringValue,
    PlexilMsgType_NotifyExec,
    PlexilMsgType_PairNumeric,
    PlexilMsgType_PairString,
    PlexilMsgType_limit
  }
  PlexilMsgType;

struct PlexilMsgBase
{
  uint32_t msgType;
  uint32_t serial;
  char* senderUID;
};

#define MSG_BASE "PlexilMessageBase"
#define MSG_BASE_FORMAT "{uint, uint, string}"

/*
 * Used for command, message, lookups, plans, libraries
 */

struct PlexilOperatorMsg
{
  struct PlexilMsgBase header;
  char* opName;
  uint32_t nArguments;
};

#define OPERATOR_MSG "PlexilOperatorMessage"
#define OPERATOR_MSG_FORMAT "{uint, uint, string, string, uint}"

struct DatumHeader
{
  struct PlexilMsgBase header;
  uint32_t argNumber;
};

struct NumericValue
{
  struct DatumHeader datumHeader;
  double doubleValue;
};

#define NUMERIC_VALUE "PlexilNumericValue"
#define NUMERIC_VALUE_FORMAT "{uint, uint, string, double}"

struct StringValue
{
  DatumHeader argHeader;
  char* stringValue;
};

#define STRING_VALUE "PlexilStringValue"
#define STRING_VALUE_FORMAT "{uint, uint, string, string}"

/*
 * Arrays and other argument types here (NYI)
 */

struct PairHeader
{
  struct DatumHeader datumHeader;
  char* pairName;
};

struct NumericPair
{
  struct PairHeader pairHeader;
  double pairDoubleValue;
};

#define NUMERIC_PAIR "PlexilNumericPair"
#define NUMERIC_PAIR_FORMAT "{uint, uint, string, string, double}"

struct StringPair
{
  struct PairHeader pairHeader;
  double stringDoubleValue;
};

#define STRING_PAIR "PlexilStringPair"
#define STRING_PAIR_FORMAT "{uint, uint, string, string, string}"

