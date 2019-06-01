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

#ifndef SQUISH_SIMD_SSE_VEC3_H
#define SQUISH_SIMD_SSE_VEC3_H

#include "simd_sse.h"
#include "simd_sse_col3.h"

namespace squish
{
class Vec3;

int operator!(const Vec3& left);
int operator<(const Vec3& left, const Vec3& right);
int operator>(const Vec3& left, const Vec3& right);
int operator==(const Vec3& left, const Vec3& right);
Vec3 operator&(const Vec3& left, const Vec3& right);
Vec3 operator%(const Vec3& left, const Vec3& right);
Vec3 operator+(const Vec3& left, const Vec3& right);
Vec3 operator-(const Vec3& left, const Vec3& right);
Vec3 operator*(const Vec3& left, const Vec3& right);
Vec3 operator*(const Vec3& left, float right);
Vec3 operator*(float left, const Vec3& right);
Vec3 operator/(const Vec3& left, float right);
Vec3 operator*(const Vec3& left, int right);
Vec3 MultiplyAdd(const Vec3& a, const Vec3& b, const Vec3& c);
Vec3 NegativeMultiplySubtract(const Vec3& a, const Vec3& b, const Vec3& c);

template <const int f, const int t>
Vec3 Shuffle(const Vec3& a);

template <const int f, const int t>
Vec3 Exchange(const Vec3& a);

template <const int n>
Vec3 RotateLeft(const Vec3& a);

Vec3 HorizontalAdd(const Vec3& a);
Vec3 HorizontalAdd(const Vec3& a, const Vec3& b);
Vec3 Select(const Vec3& a, const Vec3& b, const Vec3& c);
Vec3 HorizontalMin(const Vec3& a);
Vec3 HorizontalMax(const Vec3& a);
Vec3 HorizontalMaxXY(const Vec3& a);
Vec3 HorizontalMinXY(const Vec3& a);
Vec3 Reciprocal(const Vec3& v);
Vec3 ReciprocalSqrt(const Vec3& v);
Vec3 Sqrt(const Vec3& v);
Vec3 Length(const Vec3& left);
Vec3 ReciprocalLength(const Vec3& left);
Vec3 Normalize(const Vec3& left);
Vec3 Normalize(Vec3& x, Vec3& y, Vec3& z);

template <const bool disarm>
Vec3 Complement(const Vec3& left);

template <const bool disarm>
Vec3 Complement(Vec3& left, Vec3& right);

template <const bool disarm>
Vec3 ComplementPyramidal(Vec3& left);

template <const bool disarm>
Vec3 ComplementPyramidal(Vec3& left, Vec3& right);

Vec3 Dot(const Vec3& left, const Vec3& right);
void Dot(const Vec3& left, const Vec3& right, float* r);
Vec3 Abs(const Vec3& a);
Vec3 Neg(const Vec3& a);
Vec3 Min(const Vec3& left, const Vec3& right);
Vec3 Max(const Vec3& left, const Vec3& right);

template <const bool round>
Col3 FloatToInt(const Vec3& v);

template <const bool round>
Col3 FloatToUHalf(const Vec3& v);

template <const bool round>
Col3 FloatToSHalf(const Vec3& v);

Vec3 UHalfToFloat(const Col3& v);
Vec3 SHalfToFloat(const Col3& v);

Vec3 Truncate(const Vec3& v);
Vec3 AbsoluteDifference(const Vec3& left, const Vec3& right);
Vec3 SummedAbsoluteDifference(const Vec3& left, const Vec3& right);
Vec3 MaximumAbsoluteDifference(const Vec3& left, const Vec3& right);
bool CompareAnyLessThan(const Vec3& left, const Vec3& right);
bool CompareAnyGreaterThan(const Vec3& left, const Vec3& right);
bool CompareAllEqualTo(const Vec3& left, const Vec3& right);
Col3 CompareAllEqualTo_M8(const Vec3& left, const Vec3& right);
int CompareFirstLessThan(const Vec3& left, const Vec3& right);
int CompareFirstGreaterThan(const Vec3& left, const Vec3& right);
int CompareFirstEqualTo(const Vec3& left, const Vec3& right);
Vec3 TransferZ(const Vec3& left, const Vec3& right);
void LoadAligned(Vec3& a, Vec3& b, const Vec3& c);
void LoadAligned(Vec3& a, void const* source);
void LoadUnaligned(Vec3& a, void const* source);
void LoadAligned(Vec3& a, Vec3& b, void const* source);
void LoadUnaligned(Vec3& a, Vec3& b, void const* source);
void StoreUnaligned(const Vec3& a, void* destination);

class Vec3
{
public:
  Vec3()
  {
  }

  explicit Vec3(__m128 v) : m_v(v)
  {
  }

  Vec3(const Vec3& arg) : m_v(arg.m_v)
  {
  }

  Vec3& operator=(const Vec3& arg)
  {
    m_v = arg.m_v;
    return *this;
  }

  explicit Vec3(float s) : m_v(_mm_set1_ps(s))
  {
  }
  explicit Vec3(int s) : m_v(_mm_cvtepi32_ps(_mm_set1_epi32(s)))
  {
  }

  Vec3(const float* x, const float* y, const float* z)
  {
    m_v = _mm_unpacklo_ps(_mm_load_ss(x), _mm_load_ss(y));
    m_v = _mm_movelh_ps(m_v, _mm_load_ss(z));
  }

  Vec3(bool x, bool y, bool z)
  : m_v(_mm_castsi128_ps(_mm_setr_epi32(x ? ~0 : 0, y ? ~0 : 0, z ? ~0 : 0, 0)))
  {
  }

  Vec3(float x, float y, float z) : m_v(_mm_setr_ps(x, y, z, 0.0f))
  {
  }
  Vec3(float x, float y) : m_v(_mm_setr_ps(x, y, 0.0f, 0.0f))
  {
  }
  Vec3(const Vec3& x, const Vec3& y, const Vec3& z)
  : m_v(_mm_unpacklo_ps(_mm_unpacklo_ps(x.m_v, z.m_v), y.m_v))
  {
  }
  Vec3(const Vec3& x, const Vec3& y)
  : m_v(_mm_unpacklo_ps(_mm_unpacklo_ps(x.m_v, y.m_v), _mm_set1_ps(0.0f)))
  {
  }

  Vec3(const Col3& c) : m_v(_mm_cvtepi32_ps(c.m_v))
  {
  }

  void StoreX(float* x) const
  {
    _mm_store_ss(x, m_v);
  }
  void StoreY(float* y) const
  {
    _mm_store_ss(y, _mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(1)));
  }
  void StoreZ(float* z) const
  {
    _mm_store_ss(z, _mm_movehl_ps(m_v, m_v));
  }

  float X() const
  {
    return ((const float*)&m_v)[0];
  }
  float Y() const
  {
    return ((const float*)&m_v)[1];
  }
  float Z() const
  {
    return ((const float*)&m_v)[2];
  }

  float& GetX()
  {
    return ((float*)&m_v)[0];
  }
  float& GetY()
  {
    return ((float*)&m_v)[1];
  }
  float& GetZ()
  {
    return ((float*)&m_v)[2];
  }
  // let the compiler figure this one out, probably spills to memory
  float& GetO(int o)
  {
    return ((float*)&m_v)[o];
  }

  Vec3 SplatX() const
  {
    return Vec3(_mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(0)));
  }
  Vec3 SplatY() const
  {
    return Vec3(_mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(1)));
  }
  Vec3 SplatZ() const
  {
    return Vec3(_mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(2)));
  }

  template <const int inv>
  void SetXYZ(int x, int y, int z)
  {
    __m128i v = _mm_setzero_si128();

    v = _mm_insert_epi16(v, x, 0);
    v = _mm_insert_epi16(v, y, 2);
    v = _mm_insert_epi16(v, z, 4);

    if (inv)
    {
      v = _mm_sub_epi32(_mm_set1_epi32(inv), v);
    }

    m_v = _mm_cvtepi32_ps(v);
  }

  template <const int inv>
  void SetXYZpow2(int x, int y, int z)
  {
    __m128i v = _mm_setzero_si128();

    v = _mm_insert_epi16(v, x, 0);
    v = _mm_insert_epi16(v, y, 2);
    v = _mm_insert_epi16(v, z, 4);

    if (inv)
    {
      v = _mm_sub_epi32(_mm_set1_epi32(inv), v);
    }

    v = _mm_slli_epi32(v, 23);
    v = _mm_add_epi32(v, _mm_castps_si128(_mm_set1_ps(1.0f)));

    m_v = _mm_castsi128_ps(v);
  }

  Vec3& operator+=(const Vec3& v)
  {
    m_v = _mm_add_ps(m_v, v.m_v);
    return *this;
  }

  Vec3& operator-=(const Vec3& v)
  {
    m_v = _mm_sub_ps(m_v, v.m_v);
    return *this;
  }

  Vec3& operator*=(const Vec3& v)
  {
    m_v = _mm_mul_ps(m_v, v.m_v);
    return *this;
  }

  Vec3& operator/=(const Vec3& v)
  {
    *this *= Reciprocal(v);
    return *this;
  }

  Vec3& operator/=(float v)
  {
    *this *= Reciprocal(Vec3(v));
    return *this;
  }

  Vec3& operator/=(int v)
  {
    *this *= Reciprocal(Vec3(v));
    return *this;
  }

  // clamp the output to [0, 1]
  Vec3 Clamp() const
  {
    Vec3 const one(1.0f);
    Vec3 const zero(0.0f);

    return Min(one, Max(zero, *this));
  }

  Vec3 IsOne() const
  {
    return Vec3(_mm_cmpeq_ps(m_v, _mm_set1_ps(1.0f)));
  }

  Vec3 IsNotOne() const
  {
    return Vec3(_mm_cmpneq_ps(m_v, _mm_set1_ps(1.0f)));
  }

  void SwapXYZ(Vec3& with)
  {
    /* inplace swap based on xors */
    m_v = _mm_xor_ps(m_v, with.m_v);
    with.m_v = _mm_xor_ps(with.m_v, m_v);
    m_v = _mm_xor_ps(m_v, with.m_v);
  }

  __m128 m_v;
};

inline Vec3 LengthSquared(const Vec3& v)
{
  return Dot(v, v);
}

inline void LengthSquared(const Vec3& v, float* r)
{
  Dot(v, v, r);
}

template <const int f, const int t>
Vec3 Shuffle(const Vec3& a)
{
  if (f == t)
    return a;

  return Vec3(_mm_castsi128_ps(
    _mm_shuffle_epi32(_mm_castps_si128(a.m_v),
                      SQUISH_SSE_SHUF((t == 0 ? f : 0), (t == 1 ? f : 1),
                                      (t == 2 ? f : 2), (t == 3 ? f : 3)))));
}

template <const int f, const int t>
Vec3 Exchange(const Vec3& a)
{
  if (f == t)
    return a;

  return Vec3(_mm_castsi128_ps(_mm_shuffle_epi32(
    _mm_castps_si128(a.m_v),
    SQUISH_SSE_SHUF(
      (t == 0 ? f : (f == 0 ? t : 0)), (t == 1 ? f : (f == 1 ? t : 1)),
      (t == 2 ? f : (f == 2 ? t : 2)), (t == 3 ? f : (f == 3 ? t : 3))))));
}

template <const int n>
Vec3 RotateLeft(const Vec3& a)
{
  return Vec3(_mm_shuffle_ps(
    a.m_v, a.m_v, SQUISH_SSE_SHUF((n + 0) % 3, (n + 1) % 3, (n + 2) % 3, 3)));
}

template <const bool disarm>
Vec3 Complement(const Vec3& left)
{
  __m128 ren, res, rez;

  ren = left.m_v;
  rez = _mm_set1_ps(1.0f);
  res = _mm_mul_ps(left.m_v, left.m_v);
#if (SQUISH_USE_SSE >= 3)
  res = _mm_hadd_ps(res, res);
#else
  res = _mm_add_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 0, 1, 0)));
#endif
  if (!disarm)
  {
    if (_mm_comigt_ss(res, rez))
    {
      res = ReciprocalSqrt(Vec3(res)).m_v;
      res = _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(0, 0, 0, 0));

      ren = _mm_mul_ps(ren, res);
      res = rez;
    }
  }

  rez = _mm_sub_ps(rez, _mm_min_ps(rez, res));
  rez = _mm_sqrt_ps(rez);
  res = _mm_movelh_ps(left.m_v, rez);

  // sqrt(1.0f - (x*x + y*y))
  return Vec3(res);
}

template <const bool disarm>
Vec3 Complement(Vec3& left, Vec3& right)
{
  if (!disarm)
  {
    Vec3 len = (left * left) + (right * right);
    Vec3 adj = ReciprocalSqrt(Max(Vec3(1.0f), len));

    left *= adj;
    right *= adj;

    return Sqrt(Vec3(1.0f) - Min(Vec3(1.0f), len));
  }
  else
  {
    Vec3 len = (left * left) + (right * right);
    return Sqrt(Vec3(1.0f) - len);
  }
}

template <const bool disarm>
Vec3 ComplementPyramidal(Vec3& left)
{
  // 1 - max(abs(l, r)) == 1 + min(-abs(l, r))
  Vec3 res = HorizontalMinXY(Neg(left)) + Vec3(1.0f);

  res = Normalize(TransferZ(left, res));

  return res;
}

template <const bool disarm>
Vec3 ComplementPyramidal(Vec3& left, Vec3& right)
{
  // 1 - max(abs(l, r)) == 1 + min(-abs(l, r))
  Vec3 res = Min(Neg(left), Neg(right)) + Vec3(1.0f);

  Normalize(left, right, res);

  return res;
}

template <const bool round>
Col3 FloatToInt(const Vec3& v)
{
#if (SQUISH_USE_SSE == 1)
  ...
#else
  // use SSE2 instructions
  if (round)
    return Col3(_mm_cvtps_epi32(v.m_v));
  else
    return Col3(_mm_cvttps_epi32(v.m_v));
#endif
}

template <const bool round>
Col3 FloatToUHalf(const Vec3& v)
{
  Col3 h;

  h.GetR() = FloatToUHalf(v.X());
  h.GetG() = FloatToUHalf(v.Y());
  h.GetB() = FloatToUHalf(v.Z());

  return h;
}

template <const bool round>
Col3 FloatToSHalf(const Vec3& v)
{
  Col3 h;

  h.GetR() = FloatToSHalf(v.X());
  h.GetG() = FloatToSHalf(v.Y());
  h.GetB() = FloatToSHalf(v.Z());

  return h;
}
}

#endif
