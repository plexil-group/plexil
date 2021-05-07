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

#include "udp-utils.hh"

#include "Error.hh" // assertTrue_1()

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>        // htonl(), htons(), ntohl(), ntohs()
#endif

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#if defined(HAVE_CSTRING)
#include <cstring>            // memset()
#elif defined(HAVE_STRING_H)
#include <cstring>            // memset()
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>            // gethostbyname (OBSOLETE)
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>  // close()
#endif

namespace PLEXIL
{

  //
  // UNSAFE 32 bit versions of float and int conversions
  //

  static int32_t float_to_int32_t (float num)
  {
    //printf("\n\nsizeof(int32_t) = %d, sizeof(float) = %d\n\n", sizeof(int32_t), sizeof(float));
    float x = num;
    void* y = &x;
    return *(int32_t *) y;
  }

  static float int32_t_to_float (int32_t num)
  {
    int32_t x = num;
    void* y = &x;
    return *(float *) y;
  }

  // N.B. encode_* and decode_* use an intermediate buffer to get
  // around potential hardware restrictions on unaligned 32- and
  // 16-bit loads and stores.

  // Encode a 32 bit integer (in network byte order)
  void encode_int32_t(int32_t long_int, unsigned char* buffer, size_t start_index)
  {
    uint32_t temp = htonl(long_int);
    unsigned char *tempbuf = (unsigned char *) &temp;
    memmove(&buffer[start_index], tempbuf, sizeof(uint32_t));
  }

  // Encode a 16 bit integer (in network byte order)
  void encode_short_int(short int num, unsigned char* buffer, size_t start_index)
  {
    uint16_t temp = htons(num);
    unsigned char *tempbuf = (unsigned char *) &temp;
    memmove(&buffer[start_index], tempbuf, sizeof(uint16_t));
  }

  // Decode a 32 bit integer from the network bytes in host byte order
  int32_t decode_int32_t(const unsigned char* buffer, size_t start_index)
  {
    uint32_t temp;
    unsigned char *tempbuf = (unsigned char *) &temp;
    memmove(tempbuf, &buffer[start_index], sizeof(uint32_t));
    return (int32_t) ntohl(temp);
  }

  // Decode a 32 bit integer from the network bytes in host byte order
  short int decode_short_int(const unsigned char* buffer, size_t start_index)
  {
    uint16_t temp;
    unsigned char *tempbuf = (unsigned char *) &temp;
    memmove(tempbuf, &buffer[start_index], sizeof(uint16_t));
    return (short int) ntohs(temp);
  }

  // Encode a 32 bit float in network byte order
  void encode_float(float num, unsigned char* buffer, size_t start_index)
  {
    encode_int32_t(float_to_int32_t(num), buffer, start_index);
  }

  // Decode a 32 bit float from network byte order
  float decode_float(const unsigned char* buffer, size_t start_index)
  {
    // ntohl called in decode_int32_t
    int32_t temp = decode_int32_t(buffer, start_index);
    return int32_t_to_float(temp);
  }

  void encode_string(const std::string& str, unsigned char* buffer, size_t start_index)
  {
    // Note that this DOES NOT encode a c string.  You can do that on your own.
    str.copy((char *) &buffer[start_index], str.length(), 0);
  }

  std::string decode_string(const unsigned char* buffer, size_t start_index, int length)
  {
    // This decoder stops at \0 or length, which ever comes first.  The \0 is never included.
    std::string str;
    str.reserve(length);
    const unsigned char *bufptr = &buffer[start_index];
    for (int i = 0; i < length; i++) {
      unsigned char c = *bufptr++;
      if (c == 0)
	break;
      str += c;
    }
    return str;
  }

  void print_buffer(const unsigned char* buffer, size_t bytes, bool fancy)
  {
    printf("#(");
    for (size_t i = 0 ; i < bytes ; i++) {
      if (i != 0)
        printf(" "); // print a space prior to the char to ease reading
      {
        if ((fancy==true) && ((32 < buffer[i]) || (buffer[i] > 126)))
          printf("%c", (unsigned int) buffer[i]);
        else
          printf("%d", (unsigned int) buffer[i]);
      }
    }
    printf(")\n");
  }

  //! Get an IPv4 address for the given host name.
  //! @param host Name of the host, in a format suitable for gethostbyname().
  //! @param debug Print debug information if true.
  //! @return An IPv4 address for the host, in network byte order;
  //          0 if not found or error.
  in_addr_t parse_hostname(const char *host, bool debug)
  {
    // Hostname cannot be null or empty
    assertTrue_1(host && *host);

    // handle either "localhost" or "127.0.0.1" addresses
    hostent *host_ip = gethostbyname(host);
    if (!host_ip) {
      perror("parse_hostname: gethostbyname failed");
      return 0;
    }

    if (host_ip->h_addrtype != AF_INET) {
      printf("parse_hostname: No IPv4 address found for %s\n", host);
      return 0;
    }

    struct in_addr *network_ip_address = (struct in_addr *) host_ip->h_addr;
    if (debug) {
      std::string ip_addr = inet_ntoa(*network_ip_address);
      printf("  parse_hostname: peer_host==%s, ip_addr==%s\n", host, ip_addr.c_str());
    }

    return network_ip_address->s_addr;
  }

  //! Prepare an IPv4 sockaddr struct.
  //! @param sockaddr Pointer to the struct to be initialized.
  //! @param ip_addr The IPv4 address, in network byte order.
  //! @param port The port number.
  void init_sockaddr_in(struct sockaddr_in *sa, in_addr_t ip_addr, in_port_t port)
  {
    memset((char *) sa, 0, sizeof(struct sockaddr_in));
    sa->sin_family = AF_INET;
    sa->sin_addr.s_addr = ip_addr;
    sa->sin_port = htons(port);
  }

  int send_message_connect(const char* peer_host, int peer_port, const char* buffer, size_t size, bool debug)
  {
    in_addr_t peer_ip = parse_hostname(peer_host, debug);
    if (!peer_ip) {
      perror("send_message_connect: parse_hostname failed");
      return -1;
    }

    struct sockaddr_in peer_addr = {};
    init_sockaddr_in(&peer_addr, peer_ip, peer_port);

    int sock = socket(peer_addr.sin_family, SOCK_DGRAM, 0);
    if (sock == -1) {
      perror("socket() returned -1");
      return -1;
    }

    int connect_err = connect(sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    if (connect_err < 0) {
      perror("connect() returned -1");
      return -1;
    }

    ssize_t bytes_sent = 0;
    bytes_sent = send(sock, buffer, size, 0);
    if (debug)
      printf("  send_message_connect: sent %ld bytes to %s:%d\n", (long)bytes_sent, peer_host, peer_port);
    close(sock);
    return bytes_sent;
  }

}

// EOF
