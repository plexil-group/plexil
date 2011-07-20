// -*- Mode: C++ -*-
// File: plexil/src/interfaces/UdpAdapter/udp-tests.cc
// Date: Fri Jul 15 18:01:22 2011

#include "udp-utils.hh"

using namespace PLEXIL;

int main()
{
  unsigned char* bytes1 = new unsigned char[32];
  unsigned char* bytes2 = new unsigned char[32];
  bool debug=false;

  bytes1[0] = 0x91;                   // 145
  bytes1[1] = 0x16;                   //  22
  bytes1[2] = 0x4D;                   //  77
  bytes1[3] = 0xE4;                   // 228

  printf("\nBasic encoding, decoding, and shifting\n");

  printf("\nbytes1==");
  print_buffer(bytes1, 8);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  // read the network bytes and extract the 32 bit integer
  int temp = network_bytes_to_number(bytes1, 0, 32, true, debug);
  std::cout << "-1860809244 == " << temp << std::endl << std::endl;
  // convert the 32 bit integer back to a byte stream
  number_to_network_bytes(temp, bytes2, 0, 32, debug);
  printf("bytes1==");
  print_buffer(bytes1, 8);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  // convert a subset of the network bytes
  temp = network_bytes_to_number(bytes1, 1, 16, true, debug);
  std::cout << "5709 == " << temp << std::endl;

  // and write them back shifted to the next 32 bit boundary
  number_to_network_bytes(temp, bytes2, 4, 16, debug);
  printf("\nbytes2==");
  print_buffer(bytes2, 8);

  // and write them back again shifted to the next 16 bit boundary
  number_to_network_bytes(temp, &bytes2[6], 0, 16, debug);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  printf("\nEncode and decode floats and long ints\n\n");

  float pif = 3.14159;
  int pii = float_to_long_int(pif);
  pif = long_int_to_float(pii);
  printf("pif=%f, pii=%d\n", pif, pii);

  encode_float(pif, bytes2, 0);
  printf("\nencode_float(%f, bytes2, 0)\n", pif);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  encode_long_int(pii, bytes2, 4);
  printf("\nencode_long_int(%d, bytes2, 4)\n", pii);
  printf("bytes2==");
  print_buffer(bytes2, 8);

  pii = decode_long_int(bytes2, 4);
  printf("\npii=decode_long_int(bytes2, 4)\n");
  printf("pif=%f, pii=%d\n", pif, pii);

  pif = decode_float(bytes2, 0);
  printf("\npif=decode_float(bytes2, 0)\n");
  printf("pif=%f, pii=%d\n\n", pif, pii);

  delete[] bytes1;
  delete[] bytes2;
  return 0;
}

// EOF
