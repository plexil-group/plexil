// -*- Mode: C++ -*-
// File: plexil/src/interfaces/UdpAdapter/udp-utils.cc
// Date: Thu Jul  7 16:56:50 2011

#include <stdio.h>
#include <iostream>           // cout

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

int bytesToInt(unsigned char* b, unsigned length)
{
  int val = 0;
  int j = 0;
  for (int i = length-1; i >= 0; --i)
    {
      val += (b[i] & 0xFF) << (8*j);
      ++j;
    }
  return val;
}

void intToBytes(int longInt, unsigned char* byteArray)
{
  byteArray[0] = (int)((longInt >> 24) & 0xFF) ;
  byteArray[1] = (int)((longInt >> 16) & 0xFF) ;
  byteArray[2] = (int)((longInt >> 8) & 0xFF);
  byteArray[3] = (int)((longInt & 0xff));
}

int main()
{
  unsigned char* bytes1 = new unsigned char[4];
  unsigned char* bytes2 = new unsigned char[4];
  bytes1[0] = 0x91;                   // 145
  bytes1[1] = 0x16;                   //  22
  bytes1[2] = 0x4D;                   //  77
  bytes1[3] = 0xE4;                   // 228

  std::cout << "#(" << bytes1[0] << " " << bytes1[1] << " " << bytes1[2] << " " << bytes1[3] << ")" << std::endl;
  int temp = bytesToInt(bytes1, 4);
  //std::cout << bytesToInt(bytes, 4); // -1860809244
  std::cout << temp << std::endl;
  //std::cin.get();

  intToBytes(temp, bytes2);
  std::cout << "#(" << bytes2[0] << " " << bytes2[1] << " " << bytes2[2] << " " << bytes2[3] << ")" << std::endl;
  
  delete[] bytes1;
  delete[] bytes2;
  return 0;
}

// EOF
