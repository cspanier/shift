/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels Fröhling              niels@paradice-insight.us
  Copyright (c) 2019 Christian Spanier                     github@boxie.eu

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to  deal in the Software without restriction, including
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

#ifndef SQUISH_PALETTEBLOCK_H
#define SQUISH_PALETTEBLOCK_H

#include <squish.h>
#include "maths_all.h"

namespace squish {

// -----------------------------------------------------------------------------
  void WritePaletteBlock3_m1(int partition, Vec4 const (&start)[3], Vec4 const (&end)[3], int sharedbits, std::uint8_t const (&indices)[1][16], void* block);
  void WritePaletteBlock3_m2(int partition, Vec4 const (&start)[2], Vec4 const (&end)[2], int sharedbits, std::uint8_t const (&indices)[1][16], void* block);
  void WritePaletteBlock3_m3(int partition, Vec4 const (&start)[3], Vec4 const (&end)[3], int sharedbits, std::uint8_t const (&indices)[1][16], void* block);
  void WritePaletteBlock3_m4(int partition, Vec4 const (&start)[2], Vec4 const (&end)[2], int sharedbits, std::uint8_t const (&indices)[1][16], void* block);

  void WritePaletteBlock4_m5(int r, int ix, Vec4 const (&start)[1], Vec4 const (&end)[1], int sharedbits, std::uint8_t const (&indices)[2][16], void* block);
  void WritePaletteBlock4_m6(int rotation , Vec4 const (&start)[1], Vec4 const (&end)[1], int sharedbits, std::uint8_t const (&indices)[2][16], void* block);
  void WritePaletteBlock4_m7(int partition, Vec4 const (&start)[1], Vec4 const (&end)[1], int sharedbits, std::uint8_t const (&indices)[1][16], void* block);
  void WritePaletteBlock4_m8(int partition, Vec4 const (&start)[2], Vec4 const (&end)[2], int sharedbits, std::uint8_t const (&indices)[1][16], void* block);

  void ReadPaletteBlock3_m1(std::uint8_t* rgba, void const* block);
  void ReadPaletteBlock3_m2(std::uint8_t* rgba, void const* block);
  void ReadPaletteBlock3_m3(std::uint8_t* rgba, void const* block);
  void ReadPaletteBlock3_m4(std::uint8_t* rgba, void const* block);

  void ReadPaletteBlock4_m5(std::uint8_t* rgba, void const* block);
  void ReadPaletteBlock4_m6(std::uint8_t* rgba, void const* block);
  void ReadPaletteBlock4_m7(std::uint8_t* rgba, void const* block);
  void ReadPaletteBlock4_m8(std::uint8_t* rgba, void const* block);

  void DecompresscolorsBtc7u(std::uint8_t * rgba, void const* block);
  void DecompresscolorsBtc7u(std::uint16_t* rgba, void const* block);
  void DecompresscolorsBtc7u(float* rgba, void const* block);
} // namespace squish

#endif // ndef SQUISH_PALETTEBLOCK_H