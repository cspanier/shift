/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels FrÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½hling              niels@paradice-insight.us

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

#ifndef SQUISH_SIMD_SSE_H
#define SQUISH_SIMD_SSE_H

#include <xmmintrin.h>
#if (SQUISH_USE_SSE > 1)
#include <emmintrin.h>
#endif
#if (SQUISH_USE_SSE >= 3)
#include <pmmintrin.h>
#endif
#if (SQUISH_USE_SSE >= 4)
#include <smmintrin.h>
#endif
#if (SQUISH_USE_XSSE == 3)
#include <tmmintrin.h>
#endif
#if (SQUISH_USE_XSSE == 4)
#if defined(_MSC_VER) && (_MSC_VER > 1300)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif
#endif

#include "squish.h"

#define SQUISH_SSE_SPLAT(a) ((a) | ((a) << 2) | ((a) << 4) | ((a) << 6))
#define SQUISH_SSE_SHUF(x, y, z, w) ((x) | ((y) << 2) | ((z) << 4) | ((w) << 6))
#define SQUISH_SSE_SWAP64() SQUISH_SSE_SHUF(2, 3, 0, 1)
#define SQUISH_SSE_SWAP32() SQUISH_SSE_SHUF(3, 2, 1, 0)
#define SQUISH_SSE_SWAP16() SQUISH_SSE_SHUF(3, 2, 1, 0)
#define _mm_shuffle_epi16(r, s) \
  _mm_shufflelo_epi16(_mm_shufflehi_epi16(res, s), s)

namespace squish
{
class Vec3;
class Vec4;

// scalar types
using Scr3 = Vec3;
using Scr4 = Vec4;
}

#endif
