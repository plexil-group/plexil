// -*- Mode: C++ -*-
// File: plexil/src/interfaces/UdpAdapter/outgingMsg.cc
// Date: Thu Jul  7 09:25:52 2011

#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define HALF_SECOND 500000

int send_message_connect(const char* peer_host, int peer_port, const char* buffer, size_t size);
int send_message_bind(int local_port, const char* peer_host, int peer_port, const char* buffer, size_t size);

// set the local port (for incoming msgs) and family (udp)
// (zero out the sockaddr_in struct)
// set the incoming port and address
// create the socket
// connect the socket (specifies the peer address and port to which to send (and fromw which to receive))
//   send(int sock, const void *buffer, size_t len, 0);
//   recv(int sock, void *buffer, size_t len, 0)
// bind the socket (assings the sockaddr_in to the socket)
//   sendto(in sock, const void *buffer, size_t len, 0, const struct sockaddr *dest_addr, socklen_t dest_len);
//   recvfrom(int sock, void *restrict buffer, size_t len, 0, struct sockaddr *restrict address, socklen_t *restrict address_len);

int main(void)
{
  int result = 0;
  char local_host[]  = "127.0.0.1";
  int local_port = 9876;
  char peer_host[] = "127.0.0.1";
  int peer_port = 1777;

  char send_buffer1[] = "this is a connect test";
  char send_buffer2[] = "this is a bind test";
  size_t send_size = 0;

  //char recv_buffer[8192];
  //memset((char *) &recv_buffer, 0, sizeof(recv_buffer));
  //size_t recv_size = sizeof(recv_buffer);

  send_size = sizeof(send_buffer1);
  result = send_message_connect(peer_host, peer_port, send_buffer1, send_size);
  printf("result: %d\n", result);

  usleep(HALF_SECOND);

  send_size = sizeof(send_buffer2);
  result = send_message_bind(local_port, peer_host, peer_port, send_buffer2, send_size);
  printf("result: %d\n", result);
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

  //int connect_err = connect(sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
  //if (connect_err < 0)
  //  {
  //    perror("connect() returned -1");
  //    return -1;
  //  }

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

int send_message_connect(const char* peer_host, int peer_port, const char* buffer, size_t size)
{
  struct sockaddr_in peer_addr = {};
  memset((char *) &peer_addr, 0, sizeof(peer_addr));
  peer_addr.sin_port = htons(peer_port);
  peer_addr.sin_family = AF_INET;

  if (!inet_aton(peer_host, (struct in_addr *)&peer_addr.sin_addr.s_addr))
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

  //int bind = bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr));
  //if (bind < 0)
  //  {
  //    perror("bind() returned -1");
  //    return -1;
  //  }

  if(0)
    {
      printf("sizeof(short int): %lu, sizeof(int): %lu, sizeof(long int): %lu, sizeof(float): %lu",
             sizeof(short int), sizeof(int), sizeof(long int), sizeof(float));
    }

  ssize_t bytes_sent = 0;
  bytes_sent = send(sock, buffer, size, 0);
  printf("sent %ld bytes\n", (long)bytes_sent);
  close(sock);
  return bytes_sent;
}

// EOF
