// -*- Mode: C++ -*-
// File: plexil/src/interfaces/UdpAdapter/udp-utils.hh
// Date: Fri Jul 15 13:40:53 2011

#include <stdio.h>
#include <iostream>           // cout
#include <math.h>             // pow()

namespace PLEXIL
{
  long int float_to_long_int (float num);
  float long_int_to_float (long int num);
  int network_bytes_to_number(unsigned char* buffer, int start_index, int total_bits, bool is_signed, bool debug);
  int network_bytes_to_number(unsigned char* buffer, int start_index, int total_bits, bool is_signed, bool debug);
  void encode_long_int(long int num, unsigned char* buffer, int start_index);
  long int decode_long_int(unsigned char* buffer, int start_index);
  void encode_float(float num, unsigned char* buffer, int start_index);
  float decode_float(unsigned char* buffer, int start_index);
  int udp_tests(void);
  //void reverse_bytes(unsigned char* buffer, int start_index, int num_bytes, bool debug);
}

// EOF
