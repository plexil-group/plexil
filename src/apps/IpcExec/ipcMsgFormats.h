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

#include <stdlib.h>
#ifdef macintosh
extern int fileno(FILE *);
#endif
#if !defined(__sgi) && !defined(WIN32) && !defined(REDHAT_52)
#define UNUSED_PRAGMA
#endif

typedef int INTMSG_TYPE, *INTMSG_PTR;
#define INTMSG        "Integer"
#define INTMSG_FORMAT "int"

typedef char *STRMSG_TYPE, **STRMSG_PTR;
#define STRMSG        "String"
#define STRMSG_FORMAT "string"



//Attempt at a universal packet type.

//typedef struct {
//  char *uniqueID;
//  int msgRef;
//  char *cmdName;
//  char *args; /* Variable length array of type char */
//} PPACKET_TYPE, *PPACKET_PTR;
//
//#define PPACKET_NAME "PLEXIL_PKT"
////Not sure if the variable length array of strings is done correctly
//#define PPACKET_FORMAT "{string, int, string, {string, <string, 1>}}"
//
//typedef PPACKET_TYPE MSG_TYPE, *MSG_PTR;
//#define MSG        "PlexilPacket"
//#define MSG_FORMAT PPACKET_NAME
