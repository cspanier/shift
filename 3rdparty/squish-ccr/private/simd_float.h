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

#ifndef SQUISH_SIMD_FLOAT_H
#define SQUISH_SIMD_FLOAT_H

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace squish
{
// scalar types
typedef float Scr3;
typedef float Scr4;

static float Reciprocal(float v)
{
  return math::rcp(v);
}

static float ReciprocalSqrt(float v)
{
  return math::rsqrt(v);
}

static float Sqrt(float v)
{
  return math::sqrt(v);
}

static float Min(float a, float b)
{
  return std::min(a, b);
}

static float Max(float a, float b)
{
  return std::max(a, b);
}

static float Abs(float v)
{
  return std::abs(v);
}

inline int CompareFirstLessThan(Scr4 left, Scr4 right)
{
  return left < right;
}

inline Scr4 Threshold(Scr4 a, Scr4 b)
{
  return a >= b ? 1.0f : 0.0f;
}
}

#endif  // ndef SQUISH_SIMD_FLOAT_H
