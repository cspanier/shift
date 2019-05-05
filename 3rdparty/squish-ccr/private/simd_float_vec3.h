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

#ifndef SQUISH_SIMD_FLOAT_VEC3_H
#define SQUISH_SIMD_FLOAT_VEC3_H

#include "simd_float.h"
#include "simd_float_col3.h"

namespace squish
{
#define VEC4_CONST(X) Vec4(X)

class Vec3;

int operator<(const Vec3& left, const Vec3& right);
int operator>(const Vec3& left, const Vec3& right);
int operator==(const Vec3& left, const Vec3& right);
Vec3 operator+(const Vec3& left, const Vec3& right);
Vec3 operator+(const Vec3& left, float right);
Vec3 operator-(const Vec3& left, const Vec3& right);
Vec3 operator*(const Vec3& left, const Vec3& right);
Vec3 operator*(const Vec3& left, float right);
Vec3 operator*(float left, const Vec3& right);
Vec3 operator/(const Vec3& left, const Vec3& right);
Vec3 operator/(const Vec3& left, float right);
Vec3 Reciprocal(const Vec3& v);
Vec3 ReciprocalSqrt(const Vec3& v);
Vec3 Sqrt(const Vec3& v);
Vec3 Select(const Vec3& a, const Vec3& b, Scr3 c);

template <const int n>
Vec3 RotateLeft(const Vec3& a);

Scr3 HorizontalMax(const Vec3& a);
Scr3 HorizontalAdd(const Vec3& a);
Scr3 HorizontalAdd(const Vec3& a, const Vec3& b);
Vec3 Normalize(const Vec3& left);
Scr3 Dot(const Vec3& left, const Vec3& right);
void Dot(const Vec3& left, const Vec3& right, float* r);
Vec3 Abs(const Vec3& v);
Vec3 Min(const Vec3& left, const Vec3& right);
Vec3 Max(const Vec3& left, const Vec3& right);

template <const bool round>
Col3 FloatToInt(const Vec3& v);

Vec3 Truncate(const Vec3& v);
Vec3 AbsoluteDifference(const Vec3& left, const Vec3& right);
Scr3 SummedAbsoluteDifference(const Vec3& left, const Vec3& right);
Scr3 MaximumAbsoluteDifference(const Vec3& left, const Vec3& right);
int CompareEqualTo(const Vec3& left, const Vec3& right);
bool CompareAnyLessThan(const Vec3& left, const Vec3& right);
bool CompareAnyGreaterThan(const Vec3& left, const Vec3& right);
bool CompareAllEqualTo(const Vec3& left, const Vec3& right);
int CompareFirstLessThan(const Vec3& left, const Vec3& right);
int CompareFirstGreaterThan(const Vec3& left, const Vec3& right);
int CompareFirstEqualTo(const Vec3& left, const Vec3& right);
inline Scr3 LengthSquared(const Vec3& v);
inline void LengthSquared(const Vec3& v, float* r);

class Vec3
{
public:
  typedef Vec3& aArg;

  Vec3()
  {
  }

  explicit Vec3(float _s)
  {
    x = _s;
    y = _s;
    z = _s;
  }

  Vec3(const float* _x, const float* _y, const float* _z)
  {
    x = *_x;
    y = *_y;
    z = *_z;
  }
  Vec3(float _x, float _y, float _z)
  {
    x = _x;
    y = _y;
    z = _z;
  }
  Vec3(Vec3 _x, Vec3 _y, Vec3 _z)
  {
    x = _x.x;
    y = _y.x;
    z = _z.x;
  }

  Vec3(Col3& c)
  {
    x = (float)c.r;
    y = (float)c.g;
    z = (float)c.b;
  }

  void StoreX(float* _x) const
  {
    *_x = x;
  }
  void StoreY(float* _y) const
  {
    *_y = y;
  }
  void StoreZ(float* _z) const
  {
    *_z = z;
  }

  float X() const
  {
    return x;
  }
  float Y() const
  {
    return y;
  }
  float Z() const
  {
    return z;
  }

  Vec3 operator-() const
  {
    return Vec3(-x, -y, -z);
  }

  Vec3& operator+=(const Vec3& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }

  Vec3& operator+=(float v)
  {
    x += v;
    y += v;
    z += v;
    return *this;
  }

  Vec3& operator-=(const Vec3& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }

  Vec3& operator*=(const Vec3& v)
  {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
  }

  Vec3& operator*=(float s)
  {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }

  Vec3& operator/=(const Vec3& v)
  {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
  }

  Vec3& operator/=(float s)
  {
    float t = 1.0f / s;
    x *= t;
    y *= t;
    z *= t;
    return *this;
  }

  Vec3& operator/=(int s)
  {
    float t = 1.0f / s;
    x *= t;
    y *= t;
    z *= t;
    return *this;
  }

  // clamp the output to [0, 1]
  Vec3 Clamp() const
  {
    Vec3 const one(1.0f);
    Vec3 const zero(0.0f);

    return Min(one, Max(zero, *this));
  }

  union {
    float x;
    float r;
  };
  union {
    float y;
    float g;
  };
  union {
    float z;
    float b;
  };
};

int operator<(const Vec3& left, const Vec3& right)
{
  return CompareFirstLessThan(left, right);
}

int operator>(const Vec3& left, const Vec3& right)
{
  return CompareFirstGreaterThan(left, right);
}

int operator==(const Vec3& left, const Vec3& right)
{
  return CompareFirstEqualTo(left, right);
}

Vec3 operator+(const Vec3& left, const Vec3& right)
{
  Vec3 copy(left);
  return copy += right;
}

Vec3 operator+(const Vec3& left, float right)
{
  Vec3 copy(left);
  return copy += right;
}

Vec3 operator-(const Vec3& left, const Vec3& right)
{
  Vec3 copy(left);
  return copy -= right;
}

Vec3 operator*(const Vec3& left, const Vec3& right)
{
  Vec3 copy(left);
  return copy *= right;
}

Vec3 operator*(const Vec3& left, float right)
{
  Vec3 copy(left);
  return copy *= right;
}

Vec3 operator*(float left, const Vec3& right)
{
  Vec3 copy(right);
  return copy *= left;
}

Vec3 operator/(const Vec3& left, const Vec3& right)
{
  Vec3 copy(left);
  return copy /= right;
}

Vec3 operator/(const Vec3& left, float right)
{
  Vec3 copy(left);
  return copy /= right;
}

Vec3 Reciprocal(const Vec3& v)
{
  return Vec3(1.0f / v.x, 1.0f / v.y, 1.0f / v.z);
}

Vec3 ReciprocalSqrt(const Vec3& v)
{
  return Vec3(math::rsqrt(v.x), math::rsqrt(v.y), math::rsqrt(v.z));
}

Vec3 Sqrt(const Vec3& v)
{
  return Vec3(math::sqrt(v.x), math::sqrt(v.y), math::sqrt(v.z));
}

Vec3 Select(const Vec3& a, const Vec3& b, Scr3 c)
{
  if (b.x == c)
    return Vec3(a.x);
  if (b.y == c)
    return Vec3(a.y);
  //  if (b.z == c)
  return Vec3(a.z);
}

template <const int n>
Vec3 RotateLeft(const Vec3& a)
{
  return Vec3(n == 1 ? a.y : (n == 2 ? a.z : a.x),
              n == 1 ? a.z : (n == 2 ? a.x : a.y),
              n == 1 ? a.x : (n == 2 ? a.y : a.z));
}

Scr3 HorizontalMin(const Vec3& a)
{
  return Scr3(std::min<float>(std::min<float>(a.x, a.y), a.z));
}

Scr3 HorizontalMax(const Vec3& a)
{
  return Scr3(std::max<float>(std::max<float>(a.x, a.y), a.z));
}

Scr3 HorizontalAdd(const Vec3& a)
{
  return Scr3(a.x + a.y + a.z);
}

Scr3 HorizontalAdd(const Vec3& a, const Vec3& b)
{
  return HorizontalAdd(a + b);
}

Vec3 Normalize(const Vec3& left)
{
  Vec3 sum = (left * left);
  float rsq = math::rsqrt(sum.x + sum.y + sum.z);

  return left * rsq;
}

Scr3 Dot(const Vec3& left, const Vec3& right)
{
  return HorizontalAdd(left * right);
}

void Dot(const Vec3& left, const Vec3& right, float* r)
{
  *r = HorizontalAdd(left * right);
}

Vec3 Abs(const Vec3& v)
{
  return Vec3(abs(v.x), abs(v.y), abs(v.z));
}

Vec3 Min(const Vec3& left, const Vec3& right)
{
  return Vec3(std::min<float>(left.x, right.x),
              std::min<float>(left.y, right.y),
              std::min<float>(left.z, right.z));
}

Vec3 Max(const Vec3& left, const Vec3& right)
{
  return Vec3(std::max<float>(left.x, right.x),
              std::max<float>(left.y, right.y),
              std::max<float>(left.z, right.z));
}

template <const bool round>
Col3 FloatToInt(const Vec3& v)
{
  return Col3((int)(v.x > 0.0f ? std::floor(v.x + (round ? 0.5f : 0.0f))
                               : std::ceil(v.x - (round ? 0.5f : 0.0f))),
              (int)(v.y > 0.0f ? std::floor(v.y + (round ? 0.5f : 0.0f))
                               : std::ceil(v.y - (round ? 0.5f : 0.0f))),
              (int)(v.z > 0.0f ? std::floor(v.z + (round ? 0.5f : 0.0f))
                               : std::ceil(v.z - (round ? 0.5f : 0.0f))));
}

Vec3 Truncate(const Vec3& v)
{
  return Vec3(v.x > 0.0f ? std::floor(v.x) : std::ceil(v.x),
              v.y > 0.0f ? std::floor(v.y) : std::ceil(v.y),
              v.z > 0.0f ? std::floor(v.z) : std::ceil(v.z));
}

Vec3 AbsoluteDifference(const Vec3& left, const Vec3& right)
{
  return Abs(left - right);
}

Scr3 SummedAbsoluteDifference(const Vec3& left, const Vec3& right)
{
  return HorizontalAdd(AbsoluteDifference(left, right));
}

Scr3 MaximumAbsoluteDifference(const Vec3& left, const Vec3& right)
{
  return HorizontalMax(AbsoluteDifference(left, right));
}

int CompareEqualTo(const Vec3& left, const Vec3& right)
{
  return (left.x == right.x ? 0x1 : 0x0) | (left.y == right.y ? 0x2 : 0x0) |
         (left.z == right.z ? 0x4 : 0x0);
}

bool CompareAnyLessThan(const Vec3& left, const Vec3& right)
{
  return left.x < right.x || left.y < right.y || left.z < right.z;
}

bool CompareAnyGreaterThan(const Vec3& left, const Vec3& right)
{
  return left.x > right.x || left.y > right.y || left.z > right.z;
}

bool CompareAllEqualTo(const Vec3& left, const Vec3& right)
{
  return left.x == right.x && left.y == right.y && left.z == right.z;
}

int CompareFirstLessThan(const Vec3& left, const Vec3& right)
{
  return left.x < right.x;
}

int CompareFirstGreaterThan(const Vec3& left, const Vec3& right)
{
  return left.x > right.x;
}

int CompareFirstEqualTo(const Vec3& left, const Vec3& right)
{
  return left.x == right.x;
}

inline Scr3 LengthSquared(const Vec3& v)
{
  return Dot(v, v);
}

inline void LengthSquared(const Vec3& v, float* r)
{
  Dot(v, v, r);
}
}  // namespace squish

#endif  // ndef SQUISH_SIMD_FLOAT_H
