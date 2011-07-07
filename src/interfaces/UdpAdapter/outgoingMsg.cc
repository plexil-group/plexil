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

int main(void)
{
  // Passed in args
  //std::string host "localhost";
  char peer_host[] = "127.0.0.1";
  //in local_port 9876;
  int peer_port = 1777;
  char msg[] = "this is a test";
  //std::string msg = "this is a test";

  //struct sockaddr_in local_addr = {}; 
  struct sockaddr_in peer_addr = {};
  //memset((char *) &local_addr, 0, sizeof(peer_addr));
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

//   int bind = bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr));
//   if (bind < 0)
//     {
//       perror("bind() returned -1");
//       return -1;
//     }

  if(1)
    {
      printf("sizeof(short int): %lu, sizeof(int): %lu, sizeof(long int): %lu, sizeof(float): %lu",
             sizeof(short int), sizeof(int), sizeof(long int), sizeof(float));
    }

  ssize_t bytes_sent = 0;
  bytes_sent = send(sock, &msg, sizeof(msg) - 1, 0);
  printf("\nsent %ld bytes\n", (long)bytes_sent);

}

//int make_udp_socket (int port)
//{
  //int _socket = 0;
  //struct sockaddr_in local_address = {};
  //struct sockaddr_in peer_address = {};

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
//}

// EOF
