/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels Fröhling              niels@paradice-insight.us
  Copyright (c) 2019 Christian Spanier                     github@boxie.eu

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to	deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   -------------------------------------------------------------------------- */

#ifndef SQUISH_HDRBLOCK_H
#define SQUISH_HDRBLOCK_H

#include <squish.h>
#include "maths_all.h"

namespace squish
{
void WriteHDRBlock_m1(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_m2(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_m3(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_m4(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_m5(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_m6(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_m7(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_m8(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_m9(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_mA(int partition, Col4 const (&start)[2],
                      Col4 const (&end)[2],
                      std::uint8_t const (&indices)[1][16], void* block);

void WriteHDRBlock_mB(Col4 const (&start)[1], Col4 const (&end)[1],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_mC(Col4 const (&start)[1], Col4 const (&end)[1],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_mD(Col4 const (&start)[1], Col4 const (&end)[1],
                      std::uint8_t const (&indices)[1][16], void* block);
void WriteHDRBlock_mE(Col4 const (&start)[1], Col4 const (&end)[1],
                      std::uint8_t const (&indices)[1][16], void* block);

void ReadHDRBlock_m1(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_m2(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_m3(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_m4(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_m5(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_m6(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_m7(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_m8(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_m9(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_mA(std::uint16_t* rgb, void const* block);

void ReadHDRBlock_mB(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_mC(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_mD(std::uint16_t* rgb, void const* block);
void ReadHDRBlock_mE(std::uint16_t* rgb, void const* block);

void DecompressHDRsBtc6u(std::uint16_t* rgb, void const* block);
void DecompressHDRsBtc6u(float* rgb, void const* block);
}

#endif
