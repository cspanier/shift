/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels FrÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ¶hling              niels@paradice-insight.us

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

#ifndef SQUISH_SIMD_H
#define SQUISH_SIMD_H

#include <cstdint>

namespace squish
{
// FloatTo...
extern unsigned short uhLUTb[1 << 9];
extern char uhLUTs[1 << 9];
extern unsigned short shLUTb[1 << 9];
extern char shLUTs[1 << 9];

static inline std::uint16_t FloatToUHalf(float c)
{
  unsigned int f = *((unsigned int*)&c);
  return uhLUTb[(f >> 23) & 0x01FF] +
         (std::uint16_t)((f & 0x007FFFFF) >> uhLUTs[(f >> 23) & 0x01FF]);
}
static inline std::uint16_t FloatToSHalf(float c)
{
  unsigned int f = *((unsigned int*)&c);
  return shLUTb[(f >> 23) & 0x01FF] +
         (std::uint16_t)((f & 0x007FFFFF) >> shLUTs[(f >> 23) & 0x01FF]);
}

// ...ToFloat
extern unsigned int uhLUTo[1 << 5];
extern unsigned int uhLUTm[1 << 12];
extern unsigned int uhLUTe[1 << 5];
extern unsigned int shLUTo[1 << 6];
extern unsigned int shLUTm[1 << 11];
extern unsigned int shLUTe[1 << 6];

static inline float UHalfToFloat(std::uint16_t h)
{
  unsigned int c = uhLUTm[uhLUTo[h >> 11] + (h & 0x07FF)] + uhLUTe[h >> 11];
  return *((float*)&c);
}
static inline float SHalfToFloat(std::uint16_t h)
{
  unsigned int c = shLUTm[shLUTo[h >> 10] + (h & 0x03FF)] + shLUTe[h >> 10];
  return *((float*)&c);
}
};

#if SQUISH_USE_SSE
#include "simd_sse_col3.h"
#include "simd_sse_col4.h"
#include "simd_sse_col8.h"
#include "simd_sse_vec3.h"
#include "simd_sse_vec4.h"
#else
#include "simd_float_col3.h"
#include "simd_float_col4.h"
#include "simd_float_col8.h"
#include "simd_float_vec3.h"
#include "simd_float_vec4.h"
#endif

#endif  // ndef SQUISH_SIMD_H
