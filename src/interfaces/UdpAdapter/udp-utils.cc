// -*- Mode: C++ -*-
// File: plexil/src/interfaces/UdpAdapter/udp-utils.cc
// Date: Thu Jul  7 16:56:50 2011

#include <stdio.h>
#include <iostream>           // cout
#include <math.h>             // pow()

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

// (defun network-bytes-to-number (buffer start-index total-bits &key (signed t) (debug nil))
//   "Convert network byte ordered sequence of unsigned bytes to a number."
//   (unless (= (mod total-bits 8) 0)
//     (error "total-bits must be a multiples of eight bit bytes"))
//   (let ((value 0))
//     (loop for i downfrom (- total-bits 8) downto 0 by 8
//         for cursor upfrom start-index
//         do (setf value (dpb (elt buffer cursor)
//                             (byte 8 i) value))
//            (when debug
//              (format t "buffer[~d]==#x~2X; shift<< ~d bits; value=~d~%"
//                      cursor (elt buffer cursor) i value)))
//     ;; Take the two's complement if `signed' (http://en.wikipedia.org/wiki/Two's_complement)
//     (if (and signed (>= value (/ (expt 2 total-bits) 2)))
//         (- value (expt 2 total-bits))
//       value)))

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
      value -= pow(2.0, total_bits);
    }
  return value;
}

// (defun number-to-network-bytes (number total-bits &optional buffer (start-index 0) &key (debug nil))
//   "Convert number to network byte ordered sequence of unsigned bytes characters."
//   (unless (= (mod total-bits 8) 0)
//     (error "Please specify total-bits as total for multiples of eight bit bytes"))
//   (unless buffer
//     (setf buffer (make-array (/ total-bits 8) :element-type '(unsigned-byte 8))))
//   (loop for i downfrom (- total-bits 8) downto 0 by 8
//       for cursor upfrom start-index
//       do (setf (elt buffer cursor) (ldb (byte 8 i) number))
//          (when debug
//            (let ((value (ldb (byte 8 i) number)))
//              (format t "number=~d: shift>> ~d bits; value=~d #x~2X; buffer[~d]==#x~2X~%"
//                      number i value value cursor (elt buffer cursor)))))
//   buffer)

void number_to_network_bytes(int number, int total_bits, unsigned char* buffer, int start_index, bool debug=false)
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

int main()
{
  unsigned char* bytes1 = new unsigned char[4];
  unsigned char* bytes2 = new unsigned char[4];
  bytes1[0] = 0x91;        // 145
  bytes1[1] = 0x16;                   //  22
  bytes1[2] = 0x4D;                   //  77
  bytes1[3] = 0xE4;                   // 228

  //std::cout << "#(" << bytes1[0] << " " << bytes1[1] << " " << bytes1[2] << " " << bytes1[3] << ")" << std::endl;

  printf("\nbytes1==#(%d %d %d %d)\n", bytes1[0], bytes1[1], bytes1[2], bytes1[3]);
  // read the network bytes and extract the 32 bit integer
  int temp = network_bytes_to_number(bytes1, 0, 32, true, true);
  std::cout << "-1860809244 == " << temp << std::endl << std::endl;
  // convert the 32 bit integer back to a byte stream
  number_to_network_bytes(temp, 32, bytes2, 0, true);
  printf("bytes1==#(%d %d %d %d)\n", bytes1[0], bytes1[1], bytes1[2], bytes1[3]);
  printf("bytes2==#(%d %d %d %d)\n\n", bytes2[0], bytes2[1], bytes2[2], bytes2[3]);

  // convert a subset of the network bytes
  temp = network_bytes_to_number(bytes1, 1, 16, true, true);
  std::cout << "5709 == " << temp << std::endl;
  // and write them back shifted to the right 1 place
  number_to_network_bytes(5709, 16, bytes2, 2, true);
  printf("bytes2==#(%d %d %d %d)\n", bytes2[0], bytes2[1], bytes2[2], bytes2[3]);
  
  delete[] bytes1;
  delete[] bytes2;
  return 0;
}

// EOF
