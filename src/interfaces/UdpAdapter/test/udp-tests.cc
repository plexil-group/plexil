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

#include "plexil-inttypes.h"  // fixed width integer formats

#include "udp-utils.hh"
#include "UdpEventLoop.hh"

#include <iostream>
#include <thread>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h> // IPPROTO_UDP
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h> // AF_INET, SOCK_DGRAM
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>   // for usleep()
#endif


using namespace PLEXIL;

static bool testHostnameParsing()
{
  std::cout << "Testing hostname parsing" << std::endl;
  bool result = true;

  // nnn.nnn.nnn.nnn format
  char text_127_0_0_1[] = "127.0.0.1";
  in_addr_t addr_127_0_0_1 = parse_hostname(text_127_0_0_1);
  if (!addr_127_0_0_1) {
    std::cerr << "Parsing of \"" << text_127_0_0_1 << "\" failed" << std::endl;
    result = false;
  }
  else if (addr_127_0_0_1 != (in_addr_t) 0x0100007F) {
    std::cerr << "Parsing of \"" << text_127_0_0_1 << "\" returned incorrect result" << std::endl;
    result = false;
  }
  else {
    std::cout << "Parsing of \"" << text_127_0_0_1 << "\" succeeded" << std::endl;
  }

  // localhost
  char text_localhost[] = "localhost";
  in_addr_t addr_localhost = parse_hostname(text_localhost);
  if (!addr_localhost) {
    std::cerr << "Parsing of \"" << text_localhost << "\" failed" << std::endl;
    result = false;
  }
  else if (addr_localhost != addr_127_0_0_1) {
    std::cerr << "Result for \"" << text_localhost << "\" doesn't match result for \""
              << text_127_0_0_1 << std::endl;
    result = false;
  }
  else {
        std::cout << "Parsing of \"" << text_localhost << "\" succeeded" << std::endl;
  }

  // more later

  std::cout << std::endl;
  return result;
}

static bool testEncodeDecode()
{
  unsigned char bytes1[32];
  unsigned char bytes2[32];

  bytes1[0] = 0x91;                   // 145
  bytes1[1] = 0x16;                   //  22
  bytes1[2] = 0x4D;                   //  77
  bytes1[3] = 0xE4;                   // 228
  bytes1[4] = 0;                      // null char for string test

  printf("\nBasic encoding, decoding, and shifting\n");

  printf("\nsizeof(short int): %zu, sizeof(int): %zu, sizeof(int32_t): %zu, sizeof(float): %zu\n",
         sizeof(short int), sizeof(int), sizeof(int32_t), sizeof(float));

  printf("\nbytes1==");
  print_buffer(bytes1, 8);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  // read the network bytes and extract the 32 bit integer
  printf("\Encode and decode int32_t\n");
  int32_t temp = decode_int32_t(bytes1, 0);
  std::cout << "-1860809244 == " << temp << std::endl << std::endl;
  // convert the 32 bit integer back to a byte stream
  encode_int32_t(temp, bytes2, 0);
  printf("bytes1==");
  print_buffer(bytes1, 8);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  // convert a subset of the network bytes
  printf("\nEncode and decode short_int\n");
  temp = decode_short_int(bytes1, 1);
  std::cout << "5709 == " << temp << std::endl;

  // and write them back shifted to the next 32 bit boundary
  encode_short_int(temp, bytes2, 4);
  printf("\nbytes2==");
  print_buffer(bytes2, 8);

  // and write them back again shifted to the next 16 bit boundary
  encode_short_int(temp, &bytes2[6], 0);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  printf("\nEncode and decode strings\n\n");

  printf("encode_string(\"I am\", bytes1, 0);\n");
  encode_string("I am", bytes1, 0);
  printf("bytes1==");
  print_buffer(bytes1, 8);
  std::string str = decode_string(bytes1, 0, 8);
  printf("decode_string(bytes1, 0, 8); == \"%s\"\n", str.c_str());
  
  printf("\nEncode and decode floats and ints\n\n");

  float pif = 3.14159;
  encode_float(pif, bytes2, 0);
  printf("encode_float(%f, bytes2, 0)\n", pif);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  int32_t pii = decode_int32_t(bytes2, 0);
  printf("pif=%f, pii=%d\n", pif, pii);

  encode_int32_t(pii, bytes2, 4);
  printf("\nencode_int32_t(%d, bytes2, 4)\n", pii);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  pii = decode_int32_t(bytes2, 4);
  printf("\npii=decode_int32_t(bytes2, 4)\n");
  printf("pif=%f, pii=%d\n", pif, pii);

  pif = decode_float(bytes2, 0);
  printf("\npif=decode_float(bytes2, 0)\n");
  printf("pif=%f, pii=%d\n", pif, pii);

  return true;
}

static int test_input_wait_thread(udp_thread_params *params)
{
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 1) {
    char buf[50];
    sprintf(buf, "test_input_wait_thread: socket() returned -1");
    perror(buf);
    return sock;
  }
  params->sock = sock;
  return wait_for_input(params->local_port, params->buffer, params->size, sock, params->debug);
}

static bool testSendReceive()
{
  unsigned char bytes1[32];
  unsigned char bytes2[32];

  printf("\nSend and receive some UDP buffers\n\n");

  int32_t local_port = 9876;
  char remote_host[] = "localhost";
  int32_t remote_port = 8031;

  encode_string("  This is yet another test  ", bytes1, 0);

  // Socket for the thread waiting for input
  // Parameters for the thread waiting for input
  udp_thread_params the_params = { bytes2, 32, remote_port, 0, true };
  std::thread thread_handle(test_input_wait_thread, &the_params);

  // Wait for listener to establish socket
  do {
    usleep(1000);
  } while (!the_params.sock);

  if (0 > send_message_connect(remote_host, remote_port, (const char*)bytes1, 4*sizeof(bytes1), true)) {
    printf("send_message_connect failed\n");
    return 1;
  }

  usleep(100);
  if (0 > send_message_bind(local_port, remote_host, remote_port+1, (const char*)bytes1, 4*sizeof(bytes1), true)) {
    printf("send_message_bind failed\n");
    return 1;
  }

  // Wait for wait_for_input to return
  thread_handle.join();

  printf("\n");
  print_buffer(bytes1, 32, true);
  print_buffer(bytes2, 32, true);

  printf("\nDone.\n\n");
  return true;
}

// Event loop listener function
static void eventListener(in_port_t port,
                          const void *buffer,
                          size_t length,
                          const struct sockaddr *address,
                          socklen_t address_len)
{
  std::cout << "Event listener: received " << length << " bytes on port " << port << std::endl;
}

static bool testEventLoop()
{
  in_port_t local_port = 9876;
  char remote_host[] = "localhost";
  in_port_t remote_port = 8031;


  std::cout << "Test UdpEventLoop" << std::endl;
  std::unique_ptr<UdpEventLoop> loop = makeUdpEventLoop();
  std::cout << "Test start" << std::endl;
  if (loop->start()) {
    std:: cout << "Loop start succeeded" << std::endl;
  }
  else {
    std:: cout << "Loop start failed. Ending test." << std::endl;
    return false;
  }
  std::cout << "Test openListener" << std::endl;
  if (loop->openListener(remote_port, 512, eventListener)) {
    std::cout << "openListener succeeded" << std::endl;
  }
  else {
    std::cout << "openListener failed. Ending test." << std::endl;
    return false;
  }

  // Test listener
  // TODO

  std::cout << "Test closeListener" << std::endl;
  loop->closeListener(remote_port);

  std::cout << "Test stop" << std::endl;
  loop->stop();

  return true;
}

int main()
{
  testEncodeDecode();

  if (!testHostnameParsing()) {
    std::cerr << "Hostname parsing failed. Aborting test." << std::endl;
    return 1;
  }

  testSendReceive();

  testEventLoop();

  return 0;
}

// EOF
