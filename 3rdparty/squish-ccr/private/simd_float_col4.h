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

#ifndef SQUISH_SIMD_FLOAT_COL4_H
#define SQUISH_SIMD_FLOAT_COL4_H

#include "simd_float.h"
#include "simd_float_col3.h"

namespace squish
{
class Col4;

Col4 operator&(const Col4& left, const Col4& right);
Col4 operator^(const Col4& left, const Col4& right);
Col4 operator|(const Col4& left, const Col4& right);
Col4 operator>>(const Col4& left, int n);
Col4 operator<<(const Col4& left, int n);
Col4 operator+(const Col4& left, const Col4& right);
Col4 operator-(const Col4& left, const Col4& right);
Col4 operator*(const Col4& left, const Col4& right);
Col4 operator*(const Col4& left, int right);

Col4 ShiftLeft(const Col4& a, int n);

template <const int n>
Col4 ShiftLeft(const Col4& a);

Col4 ShiftRight(const Col4& a, int n);

template <const int n>
Col4 ShiftRight(const Col4& a);

template <const int n>
Col4 ShiftRightHalf(const Col4& a);

Col4 ShiftRightHalf(const Col4& a, int n);
Col4 ShiftRightHalf(const Col4& a, const Col4& b);

template <const int n>
Col4 ShiftLeftHalf(const Col4& a);

Col4 ShiftLeftHalf(const Col4& a, const int n);

template <const int r, const int g, const int b, const int a>
Col4 ShiftLeftLo(const Col4& v);

template <const int n, const int p>
Col4 MaskBits(const Col4& a);

Col4 MaskBits(const Col4& a, const int n, const int p);

template <const int n, const int p>
Col4 CopyBits(const Col4& left, const Col4& right);

Col4 CopyBits(const Col4& left, Col4 right, const int n, const int p);

template <const int n, const int p>
Col4 KillBits(const Col4& a);

Col4 KillBits(const Col4& a, const int n, const int p);

template <const int n, const int p>
Col4 InjtBits(const Col4& left, const Col4& right);

Col4 InjtBits(const Col4& left, Col4 right, const int n, const int p);

template <const int n, const int p>
Col4 ExtrBits(const Col4& a);

Col4 ExtrBits(const Col4& a, const int n, const int p);

template <const int n, const int p>
void ExtrBits(const Col4& left, Col4& right);

template <const int n, const int p>
void ConcBits(const Col4& left, Col4& right);

template <const int n, const int p>
void ReplBits(const Col4& left, Col4& right);

Col4 MultiplyAdd(const Col4& a, const Col4& b, const Col4& c);
Col4 NegativeMultiplySubtract(const Col4& a, const Col4& b, const Col4& c);

template <const int f, const int t>
Col4 Shuffle(const Col4& a);

template <const int f, const int t>
Col4 Exchange(const Col4& a);

Col4 HorizontalAdd(const Col4& a);
Col4 HorizontalAdd(const Col4& a, const Col4& b);
Col4 HorizontalAddTiny(const Col4& a);
Col4 HorizontalAddTiny(const Col4& a, const Col4& b);
Col4 Dot(const Col4& left, const Col4& right);
Col4 DotTiny(const Col4& left, const Col4& right);
void Dot(const Col4& left, const Col4& right, int* r);
Col4 Min(const Col4& left, const Col4& right);
Col4 Max(const Col4& left, const Col4& right);
bool CompareAnyLessThan(const Col4& left, const Col4& right);
bool CompareAllEqualTo(const Col4& left, const Col4& right);
Col4 IsNotZero(const Col4& v);
Col4 IsOne(const Col4& v);
Col4 IsZero(const Col4& v);
Col4 TransferA(const Col4& left, const Col4& right);
Col4 KillA(const Col4& left);
void PackBytes(const Col4& v, unsigned int& loc);
void LoadAligned(Col4& a, Col4& b, const Col4& c);
void LoadAligned(Col4& a, void const* source);
void LoadAligned(Col4& a, Col4& b, void const* source);
void LoadUnaligned(Col4& a, Col4& b, void const* source);
void StoreAligned(const Col4& a, const Col4& b, Col4& c);
void StoreAligned(const Col4& a, void* destination);
void StoreAligned(const Col4& a, const Col4& b, void* destination);
void StoreUnaligned(const Col4& a, const Col4& b, void* destination);

class Col4
{
public:
  Col4();

  explicit Col4(int _s);

  explicit Col4(float _s);

  Col4(int _r, int _g, int _b, int _a);

  Col4(int _r, int _g, int _b);

  Col4(Col3 _v, int _w);

  explicit Col4(unsigned int _s);

  explicit Col4(const unsigned int (&_rgba)[4]);

  explicit Col4(std::uint8_t const* _source);

  Col3 GetCol3() const;

  int GetM8() const;

  int GetLong() const;

  Col4 SetLong(int v) const;

  int R() const
  {
    return r;
  }
  int G() const
  {
    return g;
  }
  int B() const
  {
    return b;
  }
  int A() const
  {
    return a;
  }

  Col4 SplatR() const
  {
    return Col4(r);
  }
  Col4 SplatG() const
  {
    return Col4(g);
  }
  Col4 SplatB() const
  {
    return Col4(b);
  }
  Col4 SplatA() const
  {
    return Col4(a);
  }

  template <const int inv>
  void SetRGBA(int _r, int _g, int _b, int _a);

  template <const int inv>
  void SetRGBApow2(int _r, int _g, int _b, int _a);

  template <const int inv>
  void SetRGBApow2(int _v);

  operator Col3() const;

  Col4 operator-() const;

  Col4& operator=(const int& f);

  Col4& operator&=(const Col4& v);

  Col4& operator^=(const Col4& v);

  Col4& operator|=(const Col4& v);

  Col4& operator>>=(const int n);

  Col4& operator<<=(const int n);

  Col4& operator+=(const Col4& v);

  Col4& operator-=(const Col4& v);

  Col4& operator*=(const Col4& v);

  // clamp the output to [0, 1]
  Col4 Clamp() const
  {
    Col4 const one(0xFF);
    Col4 const zero(0x00);

    return Min(one, Max(zero, *this));
  }

  friend class Vec4;
  friend class Col8;

  int r;
  int g;
  int b;
  int a;
};

Col4::Col4()
{
}

Col4::Col4(int _s) : r(_s), g(_s), b(_s), a(_s)
{
}

Col4::Col4(float _s) : r((int)_s), g((int)_s), b((int)_s), a((int)_s)
{
}

Col4::Col4(int _r, int _g, int _b, int _a) : r(_r), g(_g), b(_b), a(_a)
{
}

Col4::Col4(int _r, int _g, int _b) : r(_r), g(_g), b(_b), a(0)
{
}

Col4::Col4(Col3 _v, int _w) : r(_v.r), g(_v.g), b(_v.b), a(_w)
{
}

Col4::Col4(unsigned int _s) : r(_s), g(_s), b(_s), a(_s)
{
}

Col4::Col4(const unsigned int (&_rgba)[4])
: r(_rgba[0]), g(_rgba[1]), b(_rgba[2]), a(_rgba[3])
{
}

Col4::Col4(std::uint8_t const* _source)
: r(_source[0]), g(_source[1]), b(_source[2]), a(_source[3])
{
}

Col3 Col4::GetCol3() const
{
  return Col3(r, g, b);
}

int Col4::GetM8() const
{
  return (r & 0x000000FF ? 0x0001 : 0x0000) +
         (r & 0x0000FF00 ? 0x0002 : 0x0000) +
         (r & 0x00FF0000 ? 0x0004 : 0x0000) +
         (r & 0xFF000000 ? 0x0008 : 0x0000) +
         (g & 0x000000FF ? 0x0010 : 0x0000) +
         (g & 0x0000FF00 ? 0x0020 : 0x0000) +
         (g & 0x00FF0000 ? 0x0040 : 0x0000) +
         (g & 0xFF000000 ? 0x0080 : 0x0000) +
         (b & 0x000000FF ? 0x0100 : 0x0000) +
         (b & 0x0000FF00 ? 0x0200 : 0x0000) +
         (b & 0x00FF0000 ? 0x0400 : 0x0000) +
         (b & 0xFF000000 ? 0x0800 : 0x0000) +
         (a & 0x000000FF ? 0x1000 : 0x0000) +
         (a & 0x0000FF00 ? 0x2000 : 0x0000) +
         (a & 0x00FF0000 ? 0x4000 : 0x0000) +
         (a & 0xFF000000 ? 0x8000 : 0x0000);
}

int Col4::GetLong() const
{
  return r;
}

Col4 Col4::SetLong(int v) const
{
  return Col4(v, 0, 0, 0);
}

template <const int inv>
void Col4::SetRGBA(int _r, int _g, int _b, int _a)
{
  r = (inv ? inv - _r : _r);
  g = (inv ? inv - _g : _g);
  b = (inv ? inv - _b : _b);
  a = (inv ? inv - _a : _a);
}

template <const int inv>
void Col4::SetRGBApow2(int _r, int _g, int _b, int _a)
{
  r = 1 << (inv ? inv - _r : _r);
  g = 1 << (inv ? inv - _g : _g);
  b = 1 << (inv ? inv - _b : _b);
  a = 1 << (inv ? inv - _a : _a);
}

template <const int inv>
void Col4::SetRGBApow2(int _v)
{
  r = g = b = a = 1 << (inv ? inv - _v : _v);
}

Col4::operator Col3() const
{
  Col3 v;

  v.r = r;
  v.g = g;
  v.b = b;

  return v;
}

Col4 Col4::operator-() const
{
  Col4 v;

  v.r = -r;
  v.g = -g;
  v.b = -b;
  v.a = -a;

  return v;
}

Col4& Col4::operator=(const int& f)
{
  r = f;
  g = f;
  b = f;
  a = f;

  return *this;
}

Col4& Col4::operator&=(const Col4& v)
{
  r &= v.r;
  g &= v.g;
  b &= v.b;
  a &= v.a;

  return *this;
}

Col4& Col4::operator^=(const Col4& v)
{
  r ^= v.r;
  g ^= v.g;
  b ^= v.b;
  a ^= v.a;

  return *this;
}

Col4& Col4::operator|=(const Col4& v)
{
  r |= v.r;
  g |= v.g;
  b |= v.b;
  a |= v.a;

  return *this;
}

Col4& Col4::operator>>=(const int n)
{
  r >>= n;
  g >>= n;
  b >>= n;
  a >>= n;

  return *this;
}

Col4& Col4::operator<<=(const int n)
{
  r <<= n;
  g <<= n;
  b <<= n;
  a <<= n;

  return *this;
}

Col4& Col4::operator+=(const Col4& v)
{
  r += v.r;
  g += v.g;
  b += v.b;
  a += v.a;

  return *this;
}

Col4& Col4::operator-=(const Col4& v)
{
  r -= v.r;
  g -= v.g;
  b -= v.b;
  a -= v.a;

  return *this;
}

Col4& Col4::operator*=(const Col4& v)
{
  r *= v.r;
  g *= v.g;
  b *= v.b;
  a *= v.a;

  return *this;
}

Col4 operator&(const Col4& left, const Col4& right)
{
  Col4 copy(left);
  return copy &= right;
}

Col4 operator^(const Col4& left, const Col4& right)
{
  Col4 copy(left);
  return copy ^= right;
}

Col4 operator|(const Col4& left, const Col4& right)
{
  Col4 copy(left);
  return copy |= right;
}

Col4 operator>>(const Col4& left, int n)
{
  Col4 copy(left);
  return copy >>= n;
}

Col4 operator<<(const Col4& left, int n)
{
  Col4 copy(left);
  return copy <<= n;
}

Col4 operator+(const Col4& left, const Col4& right)
{
  Col4 copy(left);
  return copy += right;
}

Col4 operator-(const Col4& left, const Col4& right)
{
  Col4 copy(left);
  return copy -= right;
}

Col4 operator*(const Col4& left, const Col4& right)
{
  Col4 copy(left);
  return copy *= right;
}

Col4 operator*(const Col4& left, int right)
{
  Col4 copy(left);

  copy.r *= right;
  copy.g *= right;
  copy.b *= right;
  copy.a *= right;

  return copy;
}

Col4 ShiftLeft(const Col4& a, int n)
{
  Col4 b;

  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  if (n >= 64)
  {
    bb[1] = (ba[0] << (n - 64));
    bb[0] = 0;
  }
  else
  {
    bb[1] = (ba[1] << (n)) + (ba[0] >> (64 - n));
    bb[0] = (ba[0] << (n));
  }

  return b;
}

template <const int n>
Col4 ShiftLeft(const Col4& a)
{
  return ShiftLeft(a, n);
}

Col4 ShiftRight(const Col4& a, int n)
{
  Col4 b;

  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  if (n >= 64)
  {
    bb[0] = (ba[1] >> (n - 64));
    bb[1] = 0;
  }
  else
  {
    bb[0] = (ba[0] >> (n)) + (ba[1] << (64 - n));
    bb[1] = (ba[1] >> (n));
  }

  return b;
}

template <const int n>
Col4 ShiftRight(const Col4& a)
{
  return ShiftRight(a, n);
}

template <const int n>
Col4 ShiftRightHalf(const Col4& a)
{
  Col4 b;

  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  bb[0] = (ba[0] >> (n));
  bb[1] = (ba[1] >> (n));

  return b;
}

Col4 ShiftRightHalf(const Col4& a, int n)
{
  Col4 b;

  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  bb[0] = (ba[0] >> (n));
  bb[1] = (ba[1] >> (n));

  return b;
}

Col4 ShiftRightHalf(const Col4& a, const Col4& b)
{
  Col4 c;

  std::uint64_t* bc = (std::uint64_t*)&c.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  bc[0] = (ba[0] >> (b.r));
  bc[1] = (ba[1] >> (b.r));

  return c;
}

template <const int n>
Col4 ShiftLeftHalf(const Col4& a)
{
  Col4 b;

  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  bb[0] = (ba[0] << (n));
  bb[1] = (ba[1] << (n));

  return b;
}

Col4 ShiftLeftHalf(const Col4& a, const int n)
{
  Col4 b;

  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  bb[0] = (ba[0] << (n));
  bb[1] = (ba[1] << (n));

  return b;
}

template <const int r, const int g, const int b, const int a>
Col4 ShiftLeftLo(const Col4& v)
{
  Col4 result;

  result.r <<= v.r;
  result.g <<= v.g;
  result.b <<= v.b;
  result.a <<= v.a;

  return result;
}

template <const int n, const int p>
Col4 MaskBits(const Col4& a)
{
  if ((p + n) <= 0)
    return Col4(0);
  if ((p + n) >= 64)
    return a;

  Col4 b;

  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  bb[0] = (ba[0] & (~(0xFFFFFFFFFFFFFFFFLL << (p + n))));
  bb[1] = (ba[1] & 0);

  return b;
}

Col4 MaskBits(const Col4& a, const int n, const int p)
{
  if ((p + n) >= 64)
    return a;

  Col4 b;

  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  bb[0] = (ba[0] & (~(0xFFFFFFFFFFFFFFFFLL << (p + n))));
  bb[1] = (ba[1] & 0);

  return b;
}

template <const int n, const int p>
Col4 CopyBits(const Col4& left, const Col4& right)
{
  if (!n)
    return left;
  if (!p)
    return MaskBits<n, 0>(right);
  if ((p + n) >= 64)
    return (left) + ShiftLeftHalf<p>(right);

  return MaskBits<p, 0>(left) + MaskBits<n, p>(ShiftLeftHalf<p>(right));
}

Col4 CopyBits(const Col4& left, Col4 right, const int n, const int p)
{
  return MaskBits(left, p, 0) + MaskBits(ShiftLeftHalf(right, p), n, p);
}

template <const int n, const int p>
Col4 KillBits(const Col4& a)
{
  if ((p + n) <= 0)
    return Col4(0);
  if ((p + n) >= 64)
    return a;

  Col4 b;

  std::uint64_t base1 = (0xFFFFFFFFFFFFFFFFULL << ((p + 0) & 63));
  std::uint64_t base2 = (0xFFFFFFFFFFFFFFFFULL >> (64 - (p + n) & 63));
  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  bb[0] = (ba[0] & (base1 ^ base2));
  bb[1] = (ba[1] & 0);

  return b;
}

Col4 KillBits(const Col4& a, const int n, const int p)
{
  if ((p + n) >= 64)
    return a;

  Col4 b;

  std::uint64_t base1 = (0xFFFFFFFFFFFFFFFFULL << ((p + 0) & 63));
  std::uint64_t base2 = (0xFFFFFFFFFFFFFFFFULL >> (64 - (p + n) & 63));
  std::uint64_t* bb = (std::uint64_t*)&b.r;
  std::uint64_t* ba = (std::uint64_t*)&a.r;

  bb[0] = (ba[0] & (base1 ^ base2));
  bb[1] = (ba[1] & 0);

  return b;
}

template <const int n, const int p>
Col4 InjtBits(const Col4& left, const Col4& right)
{
  if (!n)
    return left;
  if (!p)
    return MaskBits<n, 0>(right);
  if ((p + n) >= 64)
    return (left) + ShiftLeftHalf<p>(right);

  return KillBits<n, p>(left) + MaskBits<n, p>(ShiftLeftHalf<p>(right));
}

Col4 InjtBits(const Col4& left, Col4 right, const int n, const int p)
{
  return KillBits(left, n, p) + MaskBits(ShiftLeftHalf(right, p), n, p);
}

template <const int n, const int p>
Col4 ExtrBits(const Col4& a)
{
  if (!n)
    return Col4(0);
  if (!p)
    return MaskBits<n, 0>(a);
  if ((n + p) >= 64)
    return ShiftRightHalf<p>(a);

  return MaskBits<n, 0>(ShiftRightHalf<p>(a));
}

Col4 ExtrBits(const Col4& a, const int n, const int p)
{
  return MaskBits(ShiftRightHalf(a, p), n, 0);
}

template <const int n, const int p>
void ExtrBits(const Col4& left, Col4& right)
{
  right = ExtrBits<n, p>(left);
}

template <const int n, const int p>
void ConcBits(const Col4& left, Col4& right)
{
  right = ShiftLeft<32>(right);
  if (n > 0)
    right += ExtrBits<n, p>(left);
}

template <const int n, const int p>
void ReplBits(const Col4& left, Col4& right)
{
  if (!n)
    return;
  if ((n < 0))
  {
    right = ExtrBits<-n, p>(left);
    right = Col4(right.r, right.r, right.r, right.a);
  }
  else
  {
    right = ExtrBits<n, p>(left);
    right = Col4(right.r, right.r, right.r, right.r);
  }
}

//! Returns a*b + c
Col4 MultiplyAdd(const Col4& a, const Col4& b, const Col4& c)
{
  return a * b + c;
}

//! Returns -( a*b - c )
Col4 NegativeMultiplySubtract(const Col4& a, const Col4& b, const Col4& c)
{
  return c - a * b;
}

template <const int f, const int t>
Col4 Shuffle(const Col4& a)
{
  Col4 b = a;

  int* bb = (int*)&b.r;
  int* ba = (int*)&a.r;

  bb[t] = ba[f];

  return b;
}

template <const int f, const int t>
Col4 Exchange(const Col4& a)
{
  Col4 b = a;

  int* bb = (int*)&b.r;
  int* ba = (int*)&a.r;

  std::swap(bb[t], ba[f]);

  return b;
}

Col4 HorizontalAdd(const Col4& a)
{
  return Col4(a.r + a.g + a.b + a.a);
}

Col4 HorizontalAdd(const Col4& a, const Col4& b)
{
  return HorizontalAdd(a) + HorizontalAdd(b);
}

Col4 HorizontalAddTiny(const Col4& a)
{
  return HorizontalAdd(a);
}

Col4 HorizontalAddTiny(const Col4& a, const Col4& b)
{
  return HorizontalAdd(a, b);
}

Col4 Dot(const Col4& left, const Col4& right)
{
  return HorizontalAdd(left * right);
}

Col4 DotTiny(const Col4& left, const Col4& right)
{
  return HorizontalAddTiny(left * right);
}

void Dot(const Col4& left, const Col4& right, int* r)
{
  Col4 res = Dot(left, right);

  *r = res.R();
}

Col4 Min(const Col4& left, const Col4& right)
{
  return Col4(std::min<int>(left.r, right.r), std::min<int>(left.g, right.g),
              std::min<int>(left.b, right.b), std::min<int>(left.a, right.a));
}

Col4 Max(const Col4& left, const Col4& right)
{
  return Col4(std::max<int>(left.r, right.r), std::max<int>(left.g, right.g),
              std::max<int>(left.b, right.b), std::max<int>(left.a, right.a));
}

bool CompareAnyLessThan(const Col4& left, const Col4& right)
{
  return left.r < right.r || left.g < right.g || left.b < right.b ||
         left.a < right.a;
}

bool CompareAllEqualTo(const Col4& left, const Col4& right)
{
  return left.r == right.r && left.g == right.g && left.b == right.b &&
         left.a == right.a;
}

Col4 IsNotZero(const Col4& v)
{
  return Col4(v.r > 0 ? ~0 : 0, v.g > 0 ? ~0 : 0, v.b > 0 ? ~0 : 0,
              v.a > 0 ? ~0 : 0);
}

Col4 IsOne(const Col4& v)
{
  return Col4(v.r == 0xFF ? ~0 : 0, v.g == 0xFF ? ~0 : 0, v.b == 0xFF ? ~0 : 0,
              v.a == 0xFF ? ~0 : 0);
}

Col4 IsZero(const Col4& v)
{
  return Col4(v.r == 0x00 ? ~0 : 0, v.g == 0x00 ? ~0 : 0, v.b == 0x00 ? ~0 : 0,
              v.a == 0x00 ? ~0 : 0);
}

Col4 TransferA(const Col4& left, const Col4& right)
{
  return Col4(left.r, left.g, left.b, right.a);
}

Col4 KillA(const Col4& left)
{
  return Col4(left.r, left.g, left.b, 0xFF);
}

void PackBytes(const Col4& v, unsigned int& loc)
{
  loc = v.a;
  loc <<= 8;
  loc += v.b;
  loc <<= 8;
  loc += v.g;
  loc <<= 8;
  loc += v.r;
  loc <<= 0;
}

void LoadAligned(Col4& a, Col4& b, const Col4& c)
{
  a.r = c.r;
  a.g = c.g;

  b.b = c.b;
  b.a = c.a;
}

void LoadAligned(Col4& a, void const* source)
{
  a.r = ((int*)source)[0];
  a.g = ((int*)source)[1];

  a.b = ((int*)source)[2];
  a.a = ((int*)source)[3];
}

void LoadAligned(Col4& a, Col4& b, void const* source)
{
  a.r = ((int*)source)[0];
  a.g = ((int*)source)[1];

  b.b = ((int*)source)[2];
  b.a = ((int*)source)[3];
}

void LoadUnaligned(Col4& a, Col4& b, void const* source)
{
  a.r = ((int*)source)[0];
  a.g = ((int*)source)[1];

  b.b = ((int*)source)[2];
  b.a = ((int*)source)[3];
}

void StoreAligned(const Col4& a, const Col4& b, Col4& c)
{
  c.r = a.r;
  c.g = a.g;

  c.b = b.b;
  c.a = b.a;
}

void StoreAligned(const Col4& a, void* destination)
{
  ((int*)destination)[0] = a.r;
  ((int*)destination)[1] = a.g;

  ((int*)destination)[2] = a.b;
  ((int*)destination)[3] = a.a;
}

void StoreAligned(const Col4& a, const Col4& b, void* destination)
{
  ((int*)destination)[0] = a.r;
  ((int*)destination)[1] = a.g;

  ((int*)destination)[2] = b.b;
  ((int*)destination)[3] = b.a;
}

void StoreUnaligned(const Col4& a, const Col4& b, void* destination)
{
  ((int*)destination)[0] = a.r;
  ((int*)destination)[1] = a.g;

  ((int*)destination)[2] = b.b;
  ((int*)destination)[3] = b.a;
}
}  // namespace squish

#endif  // ndef SQUISH_SIMD_FLOAT_H
