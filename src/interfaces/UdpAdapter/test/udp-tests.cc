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

#include "ThreadSpawn.hh"
#include "udp-utils.hh"

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

int main()
{
  unsigned char* bytes1 = new unsigned char[32];
  unsigned char* bytes2 = new unsigned char[32];

  bytes1[0] = 0x91;                   // 145
  bytes1[1] = 0x16;                   //  22
  bytes1[2] = 0x4D;                   //  77
  bytes1[3] = 0xE4;                   // 228

  printf("\nBasic encoding, decoding, and shifting\n");

  printf("\nsizeof(short int): %zu, sizeof(int): %zu, sizeof(int32_t): %zu, sizeof(float): %zu\n",
         sizeof(short int), sizeof(int), sizeof(int32_t), sizeof(float));

  printf("\nbytes1==");
  print_buffer(bytes1, 8);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  // read the network bytes and extract the 32 bit integer
  int32_t temp = decode_int32_t(bytes1, 0);
  std::cout << "-1860809244 == " << temp << std::endl << std::endl;
  // convert the 32 bit integer back to a byte stream
  encode_int32_t(temp, bytes2, 0);
  printf("bytes1==");
  print_buffer(bytes1, 8);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  // convert a subset of the network bytes
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

  printf("\nSend and receive some UDP buffers\n\n");

  int32_t local_port = 9876;
  char remote_host[] = "localhost";
  int32_t remote_port = 8031;

  encode_string("  This is yet another test  ", bytes1, 0);

  // Socket for the thread waiting for input
  // Parameters for the thread waiting for input
  udp_thread_params the_params = { bytes2, 32, remote_port, 0, true };
  pthread_t thread_handle;
  threadSpawn((THREAD_FUNC_PTR) test_input_wait_thread, &the_params, thread_handle);

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
  int myErrno = pthread_join(thread_handle, NULL);
  if (myErrno != 0)
    printf("pthread_join(thread_handle) returned %d\n", myErrno);

  printf("\n");
  print_buffer(bytes1, 32, true);
  print_buffer(bytes2, 32, true);

  printf("\nDone.\n\n");

  delete[] bytes1;
  delete[] bytes2;
  return 0;
}

// EOF
