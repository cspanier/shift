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

#ifndef SQUISH_SIMD_FLOAT_COL3_H
#define SQUISH_SIMD_FLOAT_COL3_H

#include "simd_float.h"

namespace squish
{
class Col3;

Col3 operator>>(const Col3& left, int n);
Col3 operator<<(const Col3& left, int n);
Col3 operator+(const Col3& left, const Col3& right);
Col3 operator-(const Col3& left, const Col3& right);
Col3 operator*(const Col3& left, const Col3& right);
Col3 operator*(const Col3& left, int right);
Col3 operator*(int left, const Col3& right);
Col3 operator/(const Col3& left, const Col3& right);
Col3 operator/(const Col3& left, int right);
Col3 Dot(const Col3& left, const Col3& right);
void Dot(const Col3& left, const Col3& right, int* r);
Col3 Min(const Col3& left, const Col3& right);
Col3 Max(const Col3& left, const Col3& right);
void PackBytes(const Col3& v, unsigned int& loc);

class Col3
{
public:
  Col3();

  explicit Col3(int _s);

  Col3(int _x, int _y, int _z);

  Col3(int _x, int _y);

  int R() const
  {
    return x;
  }
  int G() const
  {
    return y;
  }
  int B() const
  {
    return z;
  }

  Col3 operator-() const;

  Col3& operator>>=(const int n);

  Col3& operator<<=(const int n);

  Col3& operator+=(const Col3& v);

  Col3& operator-=(const Col3& v);

  Col3& operator*=(const Col3& v);

  Col3& operator*=(int s);

  Col3& operator/=(const Col3& v);

  Col3& operator/=(int s);

  // clamp the output to [0, 1]
  Col3 Clamp() const;

  friend class Col4;
  friend class Vec3;

  union {
    int x;
    int r;
  };
  union {
    int y;
    int g;
  };
  union {
    int z;
    int b;
  };
};

Col3 operator>>(const Col3& left, int n)
{
  Col3 copy(left);
  return copy >>= n;
}

Col3 operator<<(const Col3& left, int n)
{
  Col3 copy(left);
  return copy <<= n;
}

Col3 operator+(const Col3& left, const Col3& right)
{
  Col3 copy(left);
  return copy += right;
}

Col3 operator-(const Col3& left, const Col3& right)
{
  Col3 copy(left);
  return copy -= right;
}

Col3 operator*(const Col3& left, const Col3& right)
{
  Col3 copy(left);
  return copy *= right;
}

Col3 operator*(const Col3& left, int right)
{
  Col3 copy(left);
  return copy *= right;
}

Col3 operator*(int left, const Col3& right)
{
  Col3 copy(right);
  return copy *= left;
}

Col3 operator/(const Col3& left, const Col3& right)
{
  Col3 copy(left);
  return copy /= right;
}

Col3 operator/(const Col3& left, int right)
{
  Col3 copy(left);
  return copy /= right;
}

Col3 Dot(const Col3& left, const Col3& right)
{
  return Col3(left.x * right.x + left.y * right.y + left.z * right.z);
}

void Dot(const Col3& left, const Col3& right, int* r)
{
  *r = left.x * right.x + left.y * right.y + left.z * right.z;
}

Col3 Min(const Col3& left, const Col3& right)
{
  return Col3(std::min<int>(left.x, right.x), std::min<int>(left.y, right.y),
              std::min<int>(left.z, right.z));
}

Col3 Max(const Col3& left, const Col3& right)
{
  return Col3(std::max<int>(left.x, right.x), std::max<int>(left.y, right.y),
              std::max<int>(left.z, right.z));
}

void PackBytes(const Col3& v, unsigned int& loc)
{
  loc = v.b;
  loc <<= 8;
  loc += v.g;
  loc <<= 8;
  loc += v.r;
  loc <<= 0;
}

Col3::Col3()
{
}

Col3::Col3(int _s)
{
  x = _s;
  y = _s;
  z = _s;
}

Col3::Col3(int _x, int _y, int _z)
{
  x = _x;
  y = _y;
  z = _z;
}

Col3::Col3(int _x, int _y)
{
  x = _x;
  y = _y;
  z = 0;
}

Col3 Col3::operator-() const
{
  return Col3(-x, -y, -z);
}

Col3& Col3::operator>>=(const int n)
{
  x >>= n;
  y >>= n;
  z >>= n;
  return *this;
}

Col3& Col3::operator<<=(const int n)
{
  x <<= n;
  y <<= n;
  z <<= n;
  return *this;
}

Col3& Col3::operator+=(const Col3& v)
{
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

Col3& Col3::operator-=(const Col3& v)
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}

Col3& Col3::operator*=(const Col3& v)
{
  x *= v.x;
  y *= v.y;
  z *= v.z;
  return *this;
}

Col3& Col3::operator*=(int s)
{
  x *= s;
  y *= s;
  z *= s;
  return *this;
}

Col3& Col3::operator/=(const Col3& v)
{
  x /= v.x;
  y /= v.y;
  z /= v.z;
  return *this;
}

Col3& Col3::operator/=(int s)
{
  int t = s;
  x /= t;
  y /= t;
  z /= t;
  return *this;
}

// clamp the output to [0, 1]
Col3 Col3::Clamp() const
{
  Col3 const one(0xFF);
  Col3 const zero(0x00);

  return Min(one, Max(zero, *this));
}
}  // namespace squish

#endif  // ndef SQUISH_SIMD_FLOAT_H
