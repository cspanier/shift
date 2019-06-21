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

#ifndef SQUISH_SIMD_FLOAT_VEC4_H
#define SQUISH_SIMD_FLOAT_VEC4_H

#include "simd_float.h"

namespace squish
{
class Vec4
{
public:
  Vec4()
  {
  }

  explicit Vec4(float _s) : x(_s), y(_s), z(_s), w(_s)
  {
  }
  explicit Vec4(int _s)
  : x(static_cast<float>(_s)),
    y(static_cast<float>(_s)),
    z(static_cast<float>(_s)),
    w(static_cast<float>(_s))
  {
  }

  Vec4(const float* _x, const float* _y, const float* _z, const float* _w)
  : x(*_x), y(*_y), z(*_z), w(*_w)
  {
  }
  Vec4(const float* _x, const float* _y, const float* _z)
  : x(*_x), y(*_y), z(*_z), w(0.f)
  {
  }
  Vec4(const float* _x, const float* _y) : x(*_x), y(*_y), z(0.f), w(0.f)
  {
  }
  Vec4(const float* _a) : x(*_a), y(*_a), z(*_a), w(*_a)
  {
  }

  Vec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
  {
  }
  Vec4(float _x, float _y, float _z) : x(_x), y(_y), z(_z), w(0.0f)
  {
  }
  Vec4(float _x, float _y) : x(_x), y(_y), z(0.0f), w(0.0f)
  {
  }

  Vec4(Vec4 _x, Vec4 _y, Vec4 _z, Vec4 _w) : x(_x.x), y(_y.x), z(_z.x), w(_w.x)
  {
  }
  Vec4(Vec4 _x, Vec4 _y, Vec4 _z) : x(_x.x), y(_y.x), z(_z.x), w(0.0f)
  {
  }
  Vec4(Vec4 _x, Vec4 _y) : x(_x.x), y(_y.x), z(0.0f), w(0.0f)
  {
  }

  Vec4(Vec3 _v, float _w) : x(_v.x), y(_v.y), z(_v.z), w(_w)
  {
  }

  Vec4(Col4 _c)
  : x(static_cast<float>(_c.r)),
    y(static_cast<float>(_c.g)),
    z(static_cast<float>(_c.b)),
    w(static_cast<float>(_c.a))
  {
  }

  Vec3 GetVec3() const
  {
    return Vec3(x, y, z);
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
  void StoreW(float* _w) const
  {
    *_w = w;
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
  float W() const
  {
    return w;
  }

  float& GetX()
  {
    return x;
  }
  float& GetY()
  {
    return y;
  }
  float& GetZ()
  {
    return z;
  }
  float& GetW()
  {
    return w;
  }
  // let the compiler figure this one out, probably spills to memory
  float& GetO(int o)
  {
    return ((float*)this)[o];
  }

  Vec4 Swap() const
  {
    return Vec4(z, w, x, y);
  }
  Vec4 SplatX() const
  {
    return Vec4(x);
  }
  Vec4 SplatY() const
  {
    return Vec4(y);
  }
  Vec4 SplatZ() const
  {
    return Vec4(z);
  }
  Vec4 SplatW() const
  {
    return Vec4(w);
  }

  template <const int inv>
  void SetXYZW(int _x, int _y, int _z, int _w)
  {
    x = static_cast<float>(inv ? inv - _x : _x);
    y = static_cast<float>(inv ? inv - _y : _y);
    z = static_cast<float>(inv ? inv - _z : _z);
    w = static_cast<float>(inv ? inv - _w : _w);
  }

  template <const int inv>
  void SetXYZWpow2(int _x, int _y, int _z, int _w)
  {
    x = static_cast<float>(1 << (inv ? inv - _x : _x));
    y = static_cast<float>(1 << (inv ? inv - _y : _y));
    z = static_cast<float>(1 << (inv ? inv - _z : _z));
    w = static_cast<float>(1 << (inv ? inv - _w : _w));
  }

  template <const int p>
  void Set(const float val)
  {
    /**/ if (p == 0)
      x = val;
    else if (p == 1)
      y = val;
    else if (p == 2)
      z = val;
    else if (p == 3)
      w = val;
  }

  operator Vec3() const
  {
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
  }

  operator Scr4() const
  {
    return x;
  }

  Vec4 operator-() const
  {
    Vec4 v;

    v.x = -x;
    v.y = -y;
    v.z = -z;
    v.w = -w;

    return v;
  }

  Vec4& operator=(const float& f)
  {
    x = f;
    y = f;
    z = f;
    w = f;

    return *this;
  }

  Vec4& operator+=(const Vec4& v)
  {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;

    return *this;
  }

  Vec4& operator-=(const Vec4& v)
  {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;

    return *this;
  }

  Vec4& operator*=(const Vec4& v)
  {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;

    return *this;
  }

  Vec4& operator*=(float v)
  {
    x *= v;
    y *= v;
    z *= v;
    w *= v;

    return *this;
  }

  Vec4& operator/=(float v)
  {
    *this *= Reciprocal(Vec4(v));
    return *this;
  }

  Vec4& operator/=(int v)
  {
    *this *= Reciprocal(Vec4(v));
    return *this;
  }

  Vec4& operator/=(Vec4 v)
  {
    *this *= Reciprocal(v);
    return *this;
  }

  friend int operator<(const Vec4& left, const Vec4& right)
  {
    return CompareFirstLessThan(left, right);
  }

  friend int operator>(const Vec4& left, const Vec4& right)
  {
    return CompareFirstGreaterThan(left, right);
  }

  friend Vec4 operator&(const Vec4& left, const Vec4& right)
  {
    Vec4 copy(left);

    *((int*)&copy.x) &= *((int*)&right.x);
    *((int*)&copy.y) &= *((int*)&right.y);
    *((int*)&copy.z) &= *((int*)&right.z);
    *((int*)&copy.w) &= *((int*)&right.w);

    return copy;
  }

  friend Vec4 operator+(const Vec4& left, const Vec4& right)
  {
    Vec4 copy(left);
    return copy += right;
  }

  friend Vec4 operator-(const Vec4& left, const Vec4& right)
  {
    Vec4 copy(left);
    return copy -= right;
  }

  friend Vec4 operator*(const Vec4& left, const Vec4& right)
  {
    Vec4 copy(left);
    return copy *= right;
  }

  friend Vec4 operator*(const Vec4& left, float right)
  {
    Vec4 copy(left);

    copy.x *= right;
    copy.y *= right;
    copy.z *= right;
    copy.w *= right;

    return copy;
  }

  friend Vec4 operator*(float left, const Vec4& right)
  {
    Vec4 copy(right);
    return copy * left;
  }

  friend Vec4 operator/(const Vec4& left, float right)
  {
    Vec4 copy(left);
    copy /= right;
    return copy;
  }

  friend Vec4 operator*(const Vec4& left, int right)
  {
    Vec4 copy(left);

    copy.x *= right;
    copy.y *= right;
    copy.z *= right;
    copy.w *= right;

    return copy;
  }

  //! Returns a*b + c
  friend Vec4 MultiplyAdd(const Vec4& a, const Vec4& b, const Vec4& c)
  {
    return a * b + c;
  }

  //! Returns -( a*b - c )
  friend Vec4 NegativeMultiplySubtract(const Vec4& a, const Vec4& b,
                                       const Vec4& c)
  {
    return c - a * b;
  }

  template <const int f, const int t>
  friend Vec4 Shuffle(const Vec4& a)
  {
    Vec4 b = a;

    float* bb = (float*)&b.x;
    float* ba = (float*)&a.x;

    bb[t] = ba[f];

    return b;
  }

  template <const int f, const int t>
  friend Vec4 Exchange(const Vec4& a)
  {
    Vec4 b = a;

    float* bb = (float*)&b.x;
    float* ba = (float*)&a.x;

    std::swap(bb[t], ba[f]);

    return b;
  }

  friend Vec4 Reciprocal(const Vec4& v)
  {
    return Vec4(1.0f / v.x, 1.0f / v.y, 1.0f / v.z, 1.0f / v.w);
  }

  friend Vec4 ReciprocalSqrt(const Vec4& v)
  {
    return Vec4(math::rsqrt(v.x), math::rsqrt(v.y), math::rsqrt(v.z),
                math::rsqrt(v.w));
  }

  friend Vec4 Sqrt(const Vec4& v)
  {
    return Vec4(math::sqrt(v.x), math::sqrt(v.y), math::sqrt(v.z),
                math::sqrt(v.w));
  }

  friend Vec4 Select(const Vec4& a, const Vec4& b, Scr4 c)
  {
    if (b.x == c)
      return Vec4(a.x);
    if (b.y == c)
      return Vec4(a.y);
    if (b.z == c)
      return Vec4(a.z);
    //  if (b.w == c)
    return Vec4(a.w);
  }

  template <const int n>
  friend Vec4 RotateLeft(const Vec4& a)
  {
    return Vec4(n == 1 ? a.y : (n == 2 ? a.z : (n == 3 ? a.w : a.x)),
                n == 1 ? a.z : (n == 2 ? a.w : (n == 3 ? a.x : a.y)),
                n == 1 ? a.w : (n == 2 ? a.x : (n == 3 ? a.y : a.z)),
                n == 1 ? a.x : (n == 2 ? a.y : (n == 3 ? a.z : a.w)));
  }

  friend Vec4 Threshold(const Vec4& a, const Vec4& b)
  {
    return Vec4(a.x >= b.x ? 1.0f : 0.0f, a.y >= b.y ? 1.0f : 0.0f,
                a.z >= b.z ? 1.0f : 0.0f, a.w >= b.w ? 1.0f : 0.0f);
  }

  friend Scr4 HorizontalMin(const Vec4& a)
  {
    return Scr4(
      std::min<float>(std::min<float>(a.x, a.y), std::min<float>(a.z, a.w)));
  }

  friend Scr4 HorizontalMax(const Vec4& a)
  {
    return Scr4(
      std::max<float>(std::max<float>(a.x, a.y), std::max<float>(a.z, a.w)));
  }

  friend Scr4 HorizontalAdd(const Vec4& a)
  {
    return Scr4(a.x + a.y + a.z + a.w);
  }

  friend Scr4 HorizontalAdd(const Vec4& a, const Vec4& b)
  {
    return HorizontalAdd(a + b);
  }

  friend Scr4 Length(const Vec4& left)
  {
    Vec4 sum = (left * left);
    return math::sqrt(sum.x + sum.y + sum.z + sum.w);
  }

  friend Scr4 ReciprocalLength(const Vec4& left)
  {
    Vec4 sum = (left * left);
    return math::rsqrt(sum.x + sum.y + sum.z + sum.w);
  }

  friend Vec4 Normalize(const Vec4& left)
  {
    return left * ReciprocalLength(left);
  }

  friend Scr4 Dot(const Vec4& left, const Vec4& right)
  {
    return HorizontalAdd(left * right);
  }

  friend void Dot(const Vec4& left, const Vec4& right, float* r)
  {
    *r = HorizontalAdd(left * right);
  }

  friend Vec4 Abs(const Vec4& v)
  {
    return Vec4(abs(v.x), abs(v.y), abs(v.z), abs(v.w));
  }

  friend Vec4 Min(const Vec4& left, const Vec4& right)
  {
    return Vec4(
      std::min<float>(left.x, right.x), std::min<float>(left.y, right.y),
      std::min<float>(left.z, right.z), std::min<float>(left.w, right.w));
  }

  friend Vec4 Max(const Vec4& left, const Vec4& right)
  {
    return Vec4(
      std::max<float>(left.x, right.x), std::max<float>(left.y, right.y),
      std::max<float>(left.z, right.z), std::max<float>(left.w, right.w));
  }

  // clamp the output to [0, 1]
  Vec4 Clamp() const
  {
    Vec4 const one(1.0f);
    Vec4 const zero(0.0f);

    return Min(one, Max(zero, *this));
  }

  template <const bool round>
  friend Col4 FloatToInt(const Vec4& v)
  {
    return Col4((int)(v.x > 0.0f ? std::floor(v.x + (round ? 0.5f : 0.0f))
                                 : std::ceil(v.x - (round ? 0.5f : 0.0f))),
                (int)(v.y > 0.0f ? std::floor(v.y + (round ? 0.5f : 0.0f))
                                 : std::ceil(v.y - (round ? 0.5f : 0.0f))),
                (int)(v.z > 0.0f ? std::floor(v.z + (round ? 0.5f : 0.0f))
                                 : std::ceil(v.z - (round ? 0.5f : 0.0f))),
                (int)(v.w > 0.0f ? std::floor(v.w + (round ? 0.5f : 0.0f))
                                 : std::ceil(v.w - (round ? 0.5f : 0.0f))));
  }

  friend Vec4 Truncate(const Vec4& v)
  {
    return Vec4(v.x > 0.0f ? std::floor(v.x) : std::ceil(v.x),
                v.y > 0.0f ? std::floor(v.y) : std::ceil(v.y),
                v.z > 0.0f ? std::floor(v.z) : std::ceil(v.z),
                v.w > 0.0f ? std::floor(v.w) : std::ceil(v.w));
  }

  friend Vec4 AbsoluteDifference(const Vec4& left, const Vec4& right)
  {
    return Abs(left - right);
  }

  friend Scr4 SummedAbsoluteDifference(const Vec4& left, const Vec4& right)
  {
    return HorizontalAdd(AbsoluteDifference(left, right));
  }

  friend Scr4 MaximumAbsoluteDifference(const Vec4& left, const Vec4& right)
  {
    return HorizontalMax(AbsoluteDifference(left, right));
  }

  friend int CompareEqualTo(const Vec4& left, const Vec4& right)
  {
    return (left.x == right.x ? 0x1 : 0x0) | (left.y == right.y ? 0x2 : 0x0) |
           (left.z == right.z ? 0x4 : 0x0) | (left.w == right.w ? 0x8 : 0x0);
  }

  friend bool CompareAnyLessThan(const Vec4& left, const Vec4& right)
  {
    return left.x < right.x || left.y < right.y || left.z < right.z ||
           left.w < right.w;
  }

  friend bool CompareAnyGreaterThan(const Vec4& left, const Vec4& right)
  {
    return left.x > right.x || left.y > right.y || left.z > right.z ||
           left.w > right.w;
  }

  friend int CompareFirstLessThan(const Vec4& left, const Vec4& right)
  {
    return left.x < right.x;
  }

  friend int CompareFirstGreaterThan(const Vec4& left, const Vec4& right)
  {
    return left.x > right.x;
  }

  friend Col4 CompareAllEqualTo_M8(const Vec4& left, const Vec4& right)
  {
    unsigned char* lc = (unsigned char*)&left.x;
    unsigned char* rc = (unsigned char*)&right.x;
    return Col4((lc[0] == rc[0] ? 0x000000FF : 0x00000000) +
                  (lc[1] == rc[1] ? 0x0000FF00 : 0x00000000) +
                  (lc[2] == rc[2] ? 0x00FF0000 : 0x00000000) +
                  (lc[3] == rc[3] ? 0xFF000000 : 0x00000000),
                (lc[4] == rc[4] ? 0x000000FF : 0x00000000) +
                  (lc[5] == rc[5] ? 0x0000FF00 : 0x00000000) +
                  (lc[6] == rc[6] ? 0x00FF0000 : 0x00000000) +
                  (lc[7] == rc[7] ? 0xFF000000 : 0x00000000),
                (lc[8] == rc[8] ? 0x000000FF : 0x00000000) +
                  (lc[9] == rc[9] ? 0x0000FF00 : 0x00000000) +
                  (lc[10] == rc[10] ? 0x00FF0000 : 0x00000000) +
                  (lc[11] == rc[11] ? 0xFF000000 : 0x00000000),
                (lc[12] == rc[12] ? 0x000000FF : 0x00000000) +
                  (lc[13] == rc[13] ? 0x0000FF00 : 0x00000000) +
                  (lc[14] == rc[14] ? 0x00FF0000 : 0x00000000) +
                  (lc[15] == rc[15] ? 0xFF000000 : 0x00000000));
  }

  Vec4 IsOne() const
  {
    Vec4 m;

    *((int*)&m.x) = (x == 1.0f ? ~0 : 0);
    *((int*)&m.y) = (y == 1.0f ? ~0 : 0);
    *((int*)&m.z) = (z == 1.0f ? ~0 : 0);
    *((int*)&m.w) = (w == 1.0f ? ~0 : 0);

    return m;
  }

  Vec4 IsNotOne() const
  {
    Vec4 m;

    *((int*)&m.x) = (x != 1.0f ? ~0 : 0);
    *((int*)&m.y) = (y != 1.0f ? ~0 : 0);
    *((int*)&m.z) = (z != 1.0f ? ~0 : 0);
    *((int*)&m.w) = (w != 1.0f ? ~0 : 0);

    return m;
  }

  Vec4 IsZero() const
  {
    Vec4 m;

    *((int*)&m.x) = (x == 0.0f ? ~0 : 0);
    *((int*)&m.y) = (y == 0.0f ? ~0 : 0);
    *((int*)&m.z) = (z == 0.0f ? ~0 : 0);
    *((int*)&m.w) = (w == 0.0f ? ~0 : 0);

    return m;
  }

  Vec4 IsNotZero() const
  {
    Vec4 m;

    *((int*)&m.x) = (x != 0.0f ? ~0 : 0);
    *((int*)&m.y) = (y != 0.0f ? ~0 : 0);
    *((int*)&m.z) = (z != 0.0f ? ~0 : 0);
    *((int*)&m.w) = (w != 0.0f ? ~0 : 0);

    return m;
  }

  friend Vec4 TransferZW(const Vec4& left, const Vec4& right)
  {
    return Vec4(left.x, left.y, right.z, right.w);
  }

  friend Vec4 TransferW(const Vec4& left, const Vec4& right)
  {
    return Vec4(left.x, left.y, left.z, right.w);
  }

  friend Vec4 KillW(const Vec4& left)
  {
    return Vec4(left.x, left.y, left.z, 0.0f);
  }

  friend Vec4 OnlyW(const Vec4& left)
  {
    return Vec4(0.0f, 0.0f, 0.0f, left.w);
  }

  void SwapXYZW(Vec4& with)
  {
    std::swap(x, with.x);
    std::swap(y, with.y);
    std::swap(z, with.z);
    std::swap(w, with.w);
  }

  void SwapXYZ(Vec4& with)
  {
    std::swap(x, with.x);
    std::swap(y, with.y);
    std::swap(z, with.z);
  }

  void SwapW(Vec4& with)
  {
    std::swap(w, with.w);
  }

  friend void LoadAligned(Vec4& a, Vec4& b, const Vec4& c)
  {
    a.x = c.x;
    a.y = c.y;

    b.z = c.z;
    b.w = c.w;
  }

  friend void LoadAligned(Vec4& a, void const* source)
  {
    a.x = ((float*)source)[0];
    a.y = ((float*)source)[1];

    a.z = ((float*)source)[2];
    a.w = ((float*)source)[3];
  }

  friend void LoadAligned(Vec4& a, Vec4& b, void const* source)
  {
    a.x = ((float*)source)[0];
    a.y = ((float*)source)[1];

    b.z = ((float*)source)[2];
    b.w = ((float*)source)[3];
  }

  friend void LoadUnaligned(Vec4& a, void const* source)
  {
    a.x = ((float*)source)[0];
    a.y = ((float*)source)[1];

    a.z = ((float*)source)[2];
    a.w = ((float*)source)[3];
  }

  friend void LoadUnaligned(Vec4& a, Vec4& b, void const* source)
  {
    a.x = ((float*)source)[0];
    a.y = ((float*)source)[1];

    b.z = ((float*)source)[2];
    b.w = ((float*)source)[3];
  }

  float x;
  float y;
  float z;
  float w;
};

inline Scr4 LengthSquared(const Vec4& v)
{
  return Dot(v, v);
}

inline void LengthSquared(const Vec4& v, float* r)
{
  Dot(v, v, r);
}
}  // namespace squish

#endif  // ndef SQUISH_SIMD_FLOAT_H
