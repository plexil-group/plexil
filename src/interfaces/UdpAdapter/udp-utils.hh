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

#include "plexil-stdint.h" // int32_t; includes plexil-config.h

#include <string>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> // in_addr_t, in_port_t
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

  //! Get an IPv4 address for the given host name.
  //! @param host Name of the host, in a format suitable for gethostbyname().
  //! @param debug Print debug information if true.
  //! @return An IPv4 address for the host, in network byte order;
  //!         0 if not found.
  in_addr_t parse_hostname(const char *host, bool debug = false);

  //! Prepare an IPv4 sockaddr struct.
  //! @param sockaddr Pointer to the struct to be initialized.
  //! @param ip_addr The IPv4 address.
  //! @param port The port number.
  void init_sockaddr_in(struct sockaddr_in *sa, in_addr_t ip_addr, in_port_t port);

  //! Send the message as a UDP datagram to the named peer host and port.
  //! @param peer_host Host name, to be parsed by gethostbyname().
  //! @param peer_port The destination port on the other host.
  //! @param buffer The initial message to send to the other host.
  //! @param size The size of the message to send.
  //! @param debug If true, print debugging information; if false, don't.
  //! @return The file descriptor opened with the other host; -1 in event of failure.
  int send_message_connect(const char* peer_host, int peer_port, const char* buffer, size_t size, bool debug=false);

}

// EOF
