/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include <iostream>           // cout

#include "plexil-stdint.h"

#if defined(HAVE_CSTDIO)
#include <cstdio>
#elif defined(HAVE_STDIO_H)
#include <stdio.h>
#endif

namespace PLEXIL
{

  struct udp_thread_params
  {
    unsigned char* buffer;
    size_t size;
    int local_port;
    int sock;
    bool debug;
  };

  // Encode a 32 bit integer (in network byte order)
  void encode_int32_t(int32_t num, unsigned char* buffer, size_t start_index);

  // Encode a 16 bit integer (in network byte order)
  void encode_short_int(short int num, unsigned char* buffer, size_t start_index);

  // Decode a 32 bit integer from the network bytes in host byte order
  int32_t decode_int32_t(const unsigned char* buffer, size_t start_index);

  // Decode a 32 bit integer from the network bytes in host byte order
  short int decode_short_int(const unsigned char* buffer, size_t start_index);

  // Encode a 32 bit float in network byte order
  void encode_float(float num, unsigned char* buffer, size_t start_index);

  // Decode a 32 bit float from network byte order
  float decode_float(const unsigned char* buffer, size_t start_index);

  // Note that this DOES NOT encode a c string.  You can do that on your own.
  void encode_string(const std::string& str, unsigned char* buffer, size_t start_index);

  // This decoder stops at \0 or length, which ever comes first.  The \0 is never included.
  std::string decode_string(const unsigned char* buffer, size_t start_index, int length);

  void print_buffer(const unsigned char* buffer, size_t bytes, bool fancy=false);

  int send_message_connect(const char* peer_host, int peer_port, const char* buffer, size_t size, bool debug=false);
  int send_message_bind(int local_port, const char* peer_host, int peer_port, const char* buffer, size_t size, bool debug=false);
  int wait_for_input_on_thread(udp_thread_params* params);
  int wait_for_input(int local_port, unsigned char* buffer, size_t size, int sock, bool debug=false);
}

// EOF
