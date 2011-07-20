// -*- Mode: C++ -*-
// File: plexil/src/interfaces/UdpAdapter/udp-utils.cc
// Date: Thu Jul  7 16:56:50 2011

//#include <stdio.h>
//#include <iostream>           // cout
//#include <math.h>             // pow()

#include "udp-utils.hh"

namespace PLEXIL
{
  //
  // 32 bit versions of float and int conversions
  //

  long int float_to_long_int (float num)
  {
    //printf("\n\nsizeof(long int) = %d, sizeof(float) = %d\n\n", sizeof(long int), sizeof(float));
    float x = num;
    void* y = &x;
    return *(long int *) y;
  }

  float long_int_to_float (long int num)
  {
    long int x = num;
    void* y = &x;
    return *(float *) y;
  }

  int network_bytes_to_number(unsigned char* buffer, int start_index, int total_bits, bool is_signed=true, bool debug=false)
  {
    int value = 0;
    int i = total_bits - 8;
    int cursor = start_index;
    for (i ; i >= 0 ; i -= 8)
      {
        if (debug) printf("buffer[%d]==%d; shift>> %d bits; ", cursor, buffer[cursor], i);
        value += (buffer[cursor++] & 0xff) << i;
        if (debug) std::cout << "; value=" << value << std::endl;
      }
    if (is_signed && (value >= pow(2.0,total_bits)/2.0))
      {
        value -= (int) pow(2.0, total_bits);
      }
    return value;
  }

  void number_to_network_bytes(int number, unsigned char* buffer, int start_index, int total_bits, bool debug=false)
  {
    int i = total_bits - 8;
    int cursor = start_index;
    for (i ; i >= 0 ; i -= 8)
      {
        if (debug) std::cout << "number=" << number << ": shift>> " << i << " bits; ";
        buffer[cursor++] = (int)((number >> i) & 0xff);
        if (debug) printf("buffer[%d]==%d\n", cursor, buffer[cursor - 1]);
      }
  }

  void encode_long_int(long int long_int, unsigned char* buffer, int start_index)
  // Encode a 32 bit integer (in network byte order)
  {
    number_to_network_bytes(htonl(long_int), buffer, start_index, 32, false);
  }

  void encode_short_int(long int long_int, unsigned char* buffer, int start_index)
  // Encode a 16 bit integer (in network byte order)
  {
    number_to_network_bytes(htons(long_int), buffer, start_index, 16, false);
  }

  long int decode_long_int(unsigned char* buffer, int start_index)
  // Decode a 32 bit integer from the network bytes in host byte order
  {
    long int temp;
    temp = network_bytes_to_number(buffer, 0, 32, false, false);
    return ntohl(temp);
  }

  short int decode_short_int(unsigned char* buffer, int start_index)
  // Decode a 32 bit integer from the network bytes in host byte order
  {
    long int temp;
    temp = network_bytes_to_number(buffer, 0, 16, false, false);
    return ntohs(temp);
  }

  void encode_float(float num, unsigned char* buffer, int start_index)
  // Encode a 32 bit float in network byte order
  {
    long int temp = htonl(float_to_long_int(num));
    number_to_network_bytes(temp, buffer, start_index, 32, false);
  }

  float decode_float(unsigned char* buffer, int start_index)
  // Decode a 32 bit float from network byte order
  {
    // ntohl called in decode_long_int
    long int temp = decode_long_int(buffer, start_index);
    return long_int_to_float(temp);
  }

  void encode_string(const std::string str, unsigned char* buffer, int start_index)
  {
    // Note that this DOES NOT encode a c string.  You can do that on your own.
    str.copy((char*)&buffer[start_index], str.length(), 0);
  }

  std::string decode_string(unsigned char* buffer, int start_index, int length)
  {
    // This decoder stops at \0 or length, which ever comes first.  The \0 is never included.
    std::string str;
    for (int i = start_index ; i < length ; i++ )
      {
        unsigned char c = buffer[i];
        if (c == 0) break;
        str += c;
      }
    return str;
  }

  void print_buffer(unsigned char* buffer, int bytes)
  {
    printf("#(");
    for (int i = 0 ; i < bytes ; i++)
      {
        if (i != 0) printf(" ");
        printf("%d", (unsigned int) buffer[i]);
      }
    printf(")\n");
  }


  int send_message_bind(int local_port, const char* peer_host, int peer_port, const char* buffer, size_t size)
  {
    // Set the local port
    struct sockaddr_in local_addr = {};
    memset((char *) &local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(local_port);
    // Set the peer port
    struct sockaddr_in peer_addr = {};
    memset((char *) &peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_port = htons(peer_port);
    peer_addr.sin_family = AF_INET;

    if (!inet_aton(peer_host, (struct in_addr *)&peer_addr.sin_addr.s_addr))
      {
        perror("inet_aton() return -1 (peer_host bad IP adress format?)");
        return -1;
      }

    int sock = socket(local_addr.sin_family, SOCK_DGRAM, 0);
    if (sock == -1)
      {
        perror("socket() return -1");
        return -1;
      }

    int bind_err = bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (bind_err < 0)
      {
        perror("bind() returned -1");
        return -1;
      }

    if(0)
      {
        printf("sizeof(short int): %lu, sizeof(int): %lu, sizeof(long int): %lu, sizeof(float): %lu",
               sizeof(short int), sizeof(int), sizeof(long int), sizeof(float));
      }

    ssize_t bytes_sent = 0;
    bytes_sent = sendto(sock, buffer, size, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
    printf("sent %ld bytes\n", (long)bytes_sent);
    close(sock);
    return bytes_sent;
  }

  int send_message_connect(const char* peer_host, int peer_port, const char* buffer, size_t size, bool debug)
  {
    struct sockaddr_in peer_addr = {};
    memset((char *) &peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_port = htons(peer_port);
    peer_addr.sin_family = AF_INET;

    // handle either "localhost" or "127.0.0.1" addresses
    hostent *host_ip = gethostbyname(peer_host);
    if (host_ip == NULL) 
      {
        perror("send_message_connect: gethostbyname failed");
        return -1;
      }

    in_addr *network_ip_address = (in_addr*)host_ip->h_addr;
    std::string ip_addr = inet_ntoa(*network_ip_address);
    if (debug) printf("peer_host: %s, ip_addr: %s\n", peer_host, ip_addr.c_str());

    if (!inet_aton(ip_addr.c_str(), (struct in_addr *)&peer_addr.sin_addr.s_addr))
      {
        perror("inet_aton() return -1 (peer_host bad IP adress format?)");
        return -1;
      }

    int sock = socket(peer_addr.sin_family, SOCK_DGRAM, 0);
    if (sock == -1)
      {
        perror("socket() return -1");
        return -1;
      }

    int connect_err = connect(sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    if (connect_err < 0)
      {
        perror("connect() returned -1");
        return -1;
      }

    ssize_t bytes_sent = 0;
    bytes_sent = send(sock, buffer, size, 0);
    if (debug) printf("sent %ld bytes to %s:%d\n", (long)bytes_sent, peer_host, peer_port);
    close(sock);
    return bytes_sent;
  }
}

// EOF
