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

#ifndef SQUISH_NORMAL_H
#define SQUISH_NORMAL_H

#include <squish.h>
#include <limits.h>

namespace squish {
  void CompressNormalsBtc5u(std::uint8_t  const* xyzd, int mask, void* blockx, void* blocky, int flags);
  void CompressNormalsBtc5s(std::int8_t  const* xyzd, int mask, void* blockx, void* blocky, int flags);

  void CompressNormalsBtc5u(std::uint16_t const* xyzd, int mask, void* blockx, void* blocky, int flags);
  void CompressNormalsBtc5s(std::int16_t const* xyzd, int mask, void* blockx, void* blocky, int flags);

  void CompressNormalsBtc5u(float const* xyzd, int mask, void* blockx, void* blocky, int flags);
  void CompressNormalsBtc5s(float const* xyzd, int mask, void* blockx, void* blocky, int flags);

  void DecompressNormalsBtc5u(std::uint8_t * xyzd, void const* blockx, void const* blocky);
  void DecompressNormalsBtc5s(std::int8_t * xyzd, void const* blockx, void const* blocky);

  void DecompressNormalsBtc5u(std::uint16_t* xyzd, void const* blockx, void const* blocky);
  void DecompressNormalsBtc5s(std::int16_t* xyzd, void const* blockx, void const* blocky);

  void DecompressNormalsBtc5u(float* xyzd, void const* blockx, void const* blocky);
  void DecompressNormalsBtc5s(float* xyzd, void const* blockx, void const* blocky);
} // namespace squish

#endif // ndef SQUISH_ALPHA_H
