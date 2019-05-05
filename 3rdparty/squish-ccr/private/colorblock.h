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

#ifndef SQUISH_colorBLOCK_H
#define SQUISH_colorBLOCK_H

#include <squish.h>
#include "maths_all.h"

namespace squish {

// -----------------------------------------------------------------------------
  void WritecolorBlock3(const Vec3& start, const Vec3& end, std::uint8_t const* indices, void* block);
  void WritecolorBlock4(const Vec3& start, const Vec3& end, std::uint8_t const* indices, void* block);
  
  void DecompresscolorsBtc1u(std::uint8_t * rgba, void const* block, bool isBtc1);
  void DecompresscolorsBtc1u(std::uint16_t* rgba, void const* block, bool isBtc1);
  void DecompresscolorsBtc1u(float* rgba, void const* block, bool isBtc1);
} // namespace squish

#endif // ndef SQUISH_colorBLOCK_H