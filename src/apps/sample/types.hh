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

// This is a rudimentary type abstraction for the sample Plexil application.
// Internally, all types in Plexil are represented as doubles.  Unfortunately,
// no information about what actual type a double represents is passed along
// with the double.  Applications thus have to make assumptions about types.
// This abstraction at least allows documentation of expected types, and
// provides encoding and decoding functions for each type.

#ifndef _H__types
#define _H__types

// Useful in contexts where any type may appear.
typedef double Any; 

typedef double Int;
typedef double Real;
typedef double String;
typedef double Bool;
typedef double Array;

Int encodeInt (int);
int decodeInt (Int);

Real encodeReal (float x);
float decodeReal (Real x);

Bool encodeBool (bool x);
bool decodeBool (Bool x);

String encodeString (const std::string& x);
std::string decodeString (String x);

Array encodeArray (const std::vector<Any>& x);
std::vector<Any>& decodeArray (Array x);

#endif
