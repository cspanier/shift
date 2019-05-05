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

#ifndef SQUISH_SIMD_FLOAT_COL8_H
#define SQUISH_SIMD_FLOAT_COL8_H

#include "simd_float.h"

namespace squish
{
class Col8
{
public:
  Col8()
  {
  }

  Col8(Col8 const& arg)
  {
    int i = 7;
    do
    {
      s[i] = arg.s[i];
    } while (--i >= 0);
  }

  Col8& operator=(Col8 const& arg)
  {
    int i = 7;
    do
    {
      s[i] = arg.s[i];
    } while (--i >= 0);
    return *this;
  }

  explicit Col8(Col4& v)
  {
    s[0] = s[1] = (short)v.r;
    s[2] = s[3] = (short)v.g;
    s[4] = s[5] = (short)v.b;
    s[6] = s[7] = (short)v.a;
  }

  explicit Col8(int v)
  {
    s[0] = s[1] = s[2] = s[3] = s[4] = s[5] = s[6] = s[7] = (short)v;
  }
  explicit Col8(short v)
  {
    s[0] = s[1] = s[2] = s[3] = s[4] = s[5] = s[6] = s[7] = v;
  }

  Col8(int a, int b, int c, int d, int e, int f, int g, int h)
  {
    s[0] = (short)a;
    s[1] = (short)b;
    s[2] = (short)c;
    s[3] = (short)d;
    s[4] = (short)e;
    s[5] = (short)f;
    s[6] = (short)g;
    s[7] = (short)h;
  }
  Col8(std::uint16_t a, std::uint16_t b, std::uint16_t c, std::uint16_t d,
       std::uint16_t e, std::uint16_t f, std::uint16_t g, std::uint16_t h)
  {
    s[0] = a;
    s[1] = b;
    s[2] = c;
    s[3] = d;
    s[4] = e;
    s[5] = f;
    s[6] = g;
    s[7] = h;
  }

  int Get0() const
  {
    return s[0];
  }

  const std::uint16_t& operator[](int pos) const
  {
    return s[pos];
  }

  Col8& operator*=(const Col8& v)
  {
    int i = 7;
    do
    {
      s[i] *= v.s[i];
    } while (--i >= 0);
    return *this;
  }

  friend Col8 operator>>(const Col8& left, int right)
  {
    Col8 res;
    int i = 7;
    do
    {
      res.s[i] = left.s[i] >> right;
    } while (--i >= 0);
    return res;
  }

  friend Col8 operator<<(const Col8& left, int right)
  {
    Col8 res;
    int i = 7;
    do
    {
      res.s[i] = left.s[i] << right;
    } while (--i >= 0);
    return res;
  }

  friend Col8 operator+(const Col8& left, const Col8& right)
  {
    Col8 res;
    int i = 7;
    do
    {
      res.s[i] = left.s[i] + right.s[i];
    } while (--i >= 0);
    return res;
  }

  friend Col8 operator-(const Col8& left, const Col8& right)
  {
    Col8 res;
    int i = 7;
    do
    {
      res.s[i] = left.s[i] - right.s[i];
    } while (--i >= 0);
    return res;
  }

  friend Col8 operator*(const Col8& left, const Col8& right)
  {
    Col8 res;
    int i = 7;
    do
    {
      res.s[i] = left.s[i] * right.s[i];
    } while (--i >= 0);
    return res;
  }

  friend Col8 operator*(const Col8& left, int right)
  {
    Col8 res;
    int i = 7;
    do
    {
      res.s[i] = (short)(left.s[i] * right);
    } while (--i >= 0);
    return res;
  }

  friend Col8 HorizontalMin(const Col8& a)
  {
    Col8 res;

    res.s[0] = std::min(a.s[0], a.s[1]);
    res.s[2] = std::min(a.s[2], a.s[3]);
    res.s[4] = std::min(a.s[4], a.s[5]);
    res.s[6] = std::min(a.s[6], a.s[7]);

    res.s[0] = std::min(res.s[0], res.s[2]);
    res.s[4] = std::min(res.s[4], res.s[6]);

    res.s[0] = std::min(res.s[0], res.s[4]);
    res.s[1] = res.s[2] = res.s[3] = res.s[4] = res.s[5] = res.s[6] = res.s[7] =
      res.s[0];

    return res;
  }

  friend Col8 HorizontalMax(const Col8& a)
  {
    Col8 res;

    res.s[0] = std::max(a.s[0], a.s[1]);
    res.s[2] = std::max(a.s[2], a.s[3]);
    res.s[4] = std::max(a.s[4], a.s[5]);
    res.s[6] = std::max(a.s[6], a.s[7]);

    res.s[0] = std::max(res.s[0], res.s[2]);
    res.s[4] = std::max(res.s[4], res.s[6]);

    res.s[0] = std::max(res.s[0], res.s[4]);
    res.s[1] = res.s[2] = res.s[3] = res.s[4] = res.s[5] = res.s[6] = res.s[7] =
      res.s[0];

    return res;
  }

  friend Col4 Expand(const Col8& a, int ia)
  {
    return Col4(a.s[ia - 0], a.s[ia - 1], a.s[ia - 2], a.s[ia - 3]);
  }

  friend Col4 Repeat(const Col8& a, int ia)
  {
    return Col4(a.s[ia], a.s[ia], a.s[ia], a.s[ia]);
  }

  friend Col4 Interleave(const Col8& a, const Col8& b, int ia, int ib)
  {
    return Col4(a.s[ia], b.s[ib], a.s[ia], b.s[ib]);
  }

  friend Col4 Replicate(const Col8& a, const Col8& b, int ia, int ib)
  {
    return Col4(a.s[ia], a.s[ia], b.s[ib], b.s[ib]);
  }

  short s[8];
};
}  // namespace squish

#endif  // ndef SQUISH_SIMD_FLOAT_H
