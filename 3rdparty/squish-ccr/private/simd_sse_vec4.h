/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels FrÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½hling
  niels@paradice-insight.us

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

#ifndef SQUISH_SIMD_SSE_VEC4_H
#define SQUISH_SIMD_SSE_VEC4_H

#include "simd_sse.h"
#include "simd_sse_col3.h"
#include "simd_sse_col4.h"
#include "simd_sse_col8.h"

namespace squish
{
class Vec4;
class Col8;

template <class dtyp>
Vec4 LoVec4(Col8 const& v, const dtyp& dummy);

template <class dtyp>
Vec4 HiVec4(Col8 const& v, const dtyp& dummy);

int operator!(const Vec4& left);
int operator<(const Vec4& left, const Vec4& right);
int operator>(const Vec4& left, const Vec4& right);
int operator>=(const Vec4& left, const Vec4& right);
int operator==(const Vec4& left, const Vec4& right);
Vec4 operator&(const Vec4& left, const Vec4& right);
Vec4 operator%(const Vec4& left, const Vec4& right);
Vec4 operator+(const Vec4& left, const Vec4& right);
Vec4 operator-(const Vec4& left, const Vec4& right);
Vec4 operator*(const Vec4& left, const Vec4& right);
Vec4 operator*(const Vec4& left, float right);
Vec4 operator*(float left, const Vec4& right);
Vec4 operator/(const Vec4& left, float right);
Vec4 operator*(const Vec4& left, int right);
Vec4 MultiplyAdd(const Vec4& a, const Vec4& b, const Vec4& c);
Vec4 NegativeMultiplySubtract(const Vec4& a, const Vec4& b, const Vec4& c);

template <const int a, const int b, const int c, const int d>
Vec4 Merge(const Vec4& lo, const Vec4& hi);

template <const int f, const int t>
Vec4 Shuffle(const Vec4& a);

template <const int f, const int t>
Vec4 Exchange(const Vec4& a);

template <const int n>
Vec4 RotateLeft(const Vec4& a);

Vec4 Threshold(const Vec4& a, const Vec4& b);
Vec4 Select(const Vec4& a, const Vec4& b, const Vec4& c);
Vec4 HorizontalAdd(const Vec4& a);
Vec4 HorizontalAdd(const Vec4& a, const Vec4& b);
Vec4 HorizontalMin(const Vec4& a);
Vec4 HorizontalMax(const Vec4& a);
Vec4 HorizontalMaxXY(const Vec4& a);
Vec4 HorizontalMinXY(const Vec4& a);
Vec4 Reciprocal(const Vec4& v);
Vec4 ReciprocalSqrt(const Vec4& v);
Vec4 Sqrt(const Vec4& v);
Vec4 Length(const Vec4& left);
Vec4 ReciprocalLength(const Vec4& left);
Vec4 Normalize(const Vec4& left);
Vec4 Normalize(Vec4& x, Vec4& y, Vec4& z);

template <const bool disarm, const bool killw>
Vec4 Complement(const Vec4& left);

template <const bool disarm>
Vec4 Complement(Vec4& left, Vec4& right);

template <const bool disarm>
Vec4 ComplementPyramidal(Vec4& left);

template <const bool disarm>
Vec4 ComplementPyramidal(Vec4& left, Vec4& right);

Vec4 Dot(const Vec4& left, const Vec4& right);
void Dot(const Vec4& left, const Vec4& right, float* r);
Vec4 Abs(const Vec4& a);
Vec4 Neg(const Vec4& a);
Vec4 Min(const Vec4& left, const Vec4& right);
Vec4 Max(const Vec4& left, const Vec4& right);

template <const bool round>
Col4 FloatToInt(const Vec4& v);

Vec4 Truncate(const Vec4& v);
Vec4 AbsoluteDifference(const Vec4& left, const Vec4& right);
Vec4 SummedAbsoluteDifference(const Vec4& left, const Vec4& right);
Vec4 MaximumAbsoluteDifference(const Vec4& left, const Vec4& right);
int CompareEqualTo(const Vec4& left, const Vec4& right);
int CompareNotEqualTo(const Vec4& left, const Vec4& right);
int CompareLessThan(const Vec4& left, const Vec4& right);
int CompareGreaterThan(const Vec4& left, const Vec4& right);
int CompareGreaterEqual(const Vec4& left, const Vec4& right);
bool CompareAnyLessThan(const Vec4& left, const Vec4& right);
bool CompareAnyGreaterThan(const Vec4& left, const Vec4& right);
bool CompareAllEqualTo(const Vec4& left, const Vec4& right);
Col4 CompareAllEqualTo_M4(const Vec4& left, const Vec4& right);
Col4 CompareAllEqualTo_M8(const Vec4& left, const Vec4& right);
int CompareFirstLessThan(const Vec4& left, const Vec4& right);
int CompareFirstLessEqualTo(const Vec4& left, const Vec4& right);
int CompareFirstGreaterThan(const Vec4& left, const Vec4& right);
int CompareFirstGreaterEqualTo(const Vec4& left, const Vec4& right);
int CompareFirstEqualTo(const Vec4& left, const Vec4& right);
Vec4 IsGreaterThan(const Vec4& left, const Vec4& right);
Vec4 IsGreaterEqual(const Vec4& left, const Vec4& right);
Vec4 IsNotEqualTo(const Vec4& left, const Vec4& right);
Vec4 TransferW(const Vec4& left, const Vec4& right);
Vec4 TransferZW(const Vec4& left, const Vec4& right);
Vec4 KillW(const Vec4& left);
Vec4 OnlyW(const Vec4& left);
Vec4 CollapseW(const Vec4& x, const Vec4& y, const Vec4& z, const Vec4& w);
void LoadAligned(Vec4& a, Vec4& b, const Vec4& c);
void LoadAligned(Vec4& a, void const* source);
void LoadUnaligned(Vec4& a, void const* source);
void LoadAligned(Vec4& a, Vec4& b, void const* source);
void LoadUnaligned(Vec4& a, Vec4& b, void const* source);
void StoreAligned(const Vec4& a, const Vec4& b, Vec4& c);
void StoreAligned(const Vec4& a, void* destination);
void StoreAligned(const Vec4& a, const Vec4& b, void* destination);
void StoreUnaligned(const Vec4& a, void* destination);
void StoreUnaligned(const Vec4& a, const Vec4& b, void* destination);

class Vec4
{
public:
  Vec4()
  {
  }

  explicit Vec4(__m128 v) : m_v(v)
  {
  }

  Vec4(Vec4 const& arg) : m_v(arg.m_v)
  {
  }

  Vec4(Vec3 const& arg) : m_v(arg.m_v)
  {
  }

  Vec4& operator=(Vec4 const& arg)
  {
    m_v = arg.m_v;
    return *this;
  }

  Vec4& operator=(Vec3 const& arg)
  {
    m_v = arg.m_v;
    return *this;
  }

  operator Vec3()
  {
    return Vec3(m_v);
  }

  explicit Vec4(float s) : m_v(_mm_set1_ps(s))
  {
  }
  explicit Vec4(int s) : m_v(_mm_cvtepi32_ps(_mm_set1_epi32(s)))
  {
  }

  Vec4(const float* x, const float* y, const float* z, const float* w)
  {
    __m128 m_w;

    m_v = _mm_unpacklo_ps(_mm_load_ss(x), _mm_load_ss(y));
    m_w = _mm_unpacklo_ps(_mm_load_ss(z), _mm_load_ss(w));
    m_v = _mm_movelh_ps(m_v, m_w);
  }

  Vec4(const float* x, const float* y, const float* z)
  {
    m_v = _mm_unpacklo_ps(_mm_load_ss(x), _mm_load_ss(y));
    m_v = _mm_movelh_ps(m_v, _mm_load_ss(z));
  }

  Vec4(const float* x, const float* y)
  {
    m_v = _mm_unpacklo_ps(_mm_load_ss(x), _mm_load_ss(y));
    m_v = _mm_movelh_ps(m_v, _mm_set1_ps(0.0f));
  }

  Vec4(const float* x)
  {
    m_v = _mm_load_ss(x);
    m_v = _mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(0));
  }

  Vec4(const unsigned short* x)
  {
    __m128i v = _mm_setzero_si128();

    m_v = _mm_cvtepi32_ps(_mm_insert_epi16(v, *x, 0));
    m_v = _mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(0));
  }

  Vec4(const signed short* x)
  {
    __m128i v = _mm_setzero_si128();

    m_v = _mm_cvtepi32_ps(_mm_insert_epi16(v, *x, 0));
    m_v = _mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(0));
  }

  Vec4(bool x, bool y, bool z, bool w)
  : m_v(_mm_castsi128_ps(
      _mm_setr_epi32(x ? ~0 : 0, y ? ~0 : 0, z ? ~0 : 0, w ? ~0 : 0)))
  {
  }

  Vec4(int x, int y, int z, int w)
  : m_v(_mm_cvtepi32_ps(_mm_setr_epi32(x, y, z, w)))
  {
  }
  Vec4(int x, int y, int z) : m_v(_mm_cvtepi32_ps(_mm_setr_epi32(x, y, z, 0)))
  {
  }
  Vec4(int x, int y) : m_v(_mm_cvtepi32_ps(_mm_setr_epi32(x, y, 0, 0)))
  {
  }

  Vec4(float x, float y, float z, float w) : m_v(_mm_setr_ps(x, y, z, w))
  {
  }
  Vec4(float x, float y, float z) : m_v(_mm_setr_ps(x, y, z, 0.0f))
  {
  }
  Vec4(float x, float y) : m_v(_mm_setr_ps(x, y, 0.0f, 0.0f))
  {
  }

  Vec4(const Vec4& x, const Vec4& y, const Vec4& z, const Vec4& w)
  : m_v(_mm_unpacklo_ps(_mm_unpacklo_ps(x.m_v, z.m_v),
                        _mm_unpacklo_ps(y.m_v, w.m_v)))
  {
  }
  Vec4(const Vec4& x, const Vec4& y, const Vec4& z)
  : m_v(_mm_unpacklo_ps(_mm_unpacklo_ps(x.m_v, z.m_v),
                        _mm_unpacklo_ps(y.m_v, _mm_set1_ps(0.0f))))
  {
  }
  Vec4(const Vec4& x, const Vec4& y)
  : m_v(_mm_movelh_ps(_mm_unpacklo_ps(x.m_v, y.m_v), _mm_set1_ps(0.0f)))
  {
  }

  Vec4(const Vec4& x, const Vec4& y, const Vec4& z, bool w)
  : m_v(_mm_unpacklo_ps(_mm_unpacklo_ps(x.m_v, z.m_v), y.m_v))
  {
    w = w;
  }
  Vec4(const Vec4& x, const Vec4& y, bool z, bool w)
  : m_v(_mm_unpacklo_ps(x.m_v, y.m_v))
  {
    z = z;
    w = w;
  }

  Vec4(const Vec3& v, float w) : m_v(v.m_v)
  {
    m_v = _mm_or_ps(
      _mm_and_ps(m_v, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0))),
      _mm_setr_ps(0.0f, 0.0f, 0.0f, w));
  }

  Vec4(const Col4& c) : m_v(_mm_cvtepi32_ps(c.m_v))
  {
  }

  Vec3 GetVec3() const
  {
    return Vec3(m_v);
  }

  int GetM4() const
  {
    return _mm_movemask_ps(m_v);
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
  void StoreW(float* w) const
  {
    _mm_store_ss(w, _mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(3)));
  }

  float X() const
  {
    return ((float*)&m_v)[0];
  }
  float Y() const
  {
    return ((float*)&m_v)[1];
  }
  float Z() const
  {
    return ((float*)&m_v)[2];
  }
  float W() const
  {
    return ((float*)&m_v)[3];
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
  float& GetW()
  {
    return ((float*)&m_v)[3];
  }
  // let the compiler figure this one out, probably spills to memory
  float& GetO(int o)
  {
    return ((float*)&m_v)[o];
  }

  Vec4 Swap() const
  {
    return Vec4(_mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SWAP64()));
  }
  Vec4 SplatX() const
  {
    return Vec4(_mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(0)));
  }
  Vec4 SplatY() const
  {
    return Vec4(_mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(1)));
  }
  Vec4 SplatZ() const
  {
    return Vec4(_mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(2)));
  }
  Vec4 SplatW() const
  {
    return Vec4(_mm_shuffle_ps(m_v, m_v, SQUISH_SSE_SPLAT(3)));
  }

  template <const int inv>
  void SetXYZW(int x, int y, int z, int w)
  {
    __m128i v = _mm_setzero_si128();

    v = _mm_insert_epi16(v, x, 0);
    v = _mm_insert_epi16(v, y, 2);
    v = _mm_insert_epi16(v, z, 4);
    v = _mm_insert_epi16(v, w, 6);

    if (inv)
    {
      v = _mm_sub_epi32(_mm_set1_epi32(inv), v);
    }

    m_v = _mm_cvtepi32_ps(v);
  }

  template <const int inv>
  void SetXYZWpow2(int x, int y, int z, int w)
  {
    __m128i v = _mm_setzero_si128();

    v = _mm_insert_epi16(v, x, 0);
    v = _mm_insert_epi16(v, y, 2);
    v = _mm_insert_epi16(v, z, 4);
    v = _mm_insert_epi16(v, w, 6);

    if (inv)
    {
      v = _mm_sub_epi32(_mm_set1_epi32(inv), v);
    }

    v = _mm_slli_epi32(v, 23);
    v = _mm_add_epi32(v, _mm_castps_si128(_mm_set1_ps(1.0f)));

    m_v = _mm_castsi128_ps(v);
  }

  template <const int p>
  void Set(const float val)
  {
    __m128 mask = _mm_castsi128_ps(_mm_setr_epi32(
      p != 0 ? ~0 : 0, p != 1 ? ~0 : 0, p != 2 ? ~0 : 0, p != 3 ? ~0 : 0));

    __m128 fill = _mm_setr_ps(p != 0 ? 0.0f : val, p != 1 ? 0.0f : val,
                              p != 2 ? 0.0f : val, p != 3 ? 0.0f : val);

    m_v = _mm_or_ps(_mm_and_ps(m_v, mask), fill);
  }

  Vec4& operator&=(const Vec4& v)
  {
    m_v = _mm_and_ps(m_v, v.m_v);
    return *this;
  }

  Vec4& operator+=(const Vec4& v)
  {
    m_v = _mm_add_ps(m_v, v.m_v);
    return *this;
  }

  Vec4& operator-=(const Vec4& v)
  {
    m_v = _mm_sub_ps(m_v, v.m_v);
    return *this;
  }

  Vec4& operator*=(const Vec4& v)
  {
    m_v = _mm_mul_ps(m_v, v.m_v);
    return *this;
  }

  Vec4& operator*=(float v)
  {
    m_v = _mm_mul_ps(m_v, Vec4(v).m_v);
    return *this;
  }

  Vec4& operator/=(const Vec4& v)
  {
    *this *= Reciprocal(v);
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

  // clamp the output to [0, 1]
  Vec4 Clamp() const
  {
    Vec4 const one(1.0f);
    Vec4 const zero(0.0f);

    return Min(one, Max(zero, *this));
  }

  Vec4 IsOne() const
  {
    return Vec4(_mm_cmpeq_ps(m_v, _mm_set1_ps(1.0f)));
  }

  Vec4 IsNotOne() const
  {
    return Vec4(_mm_cmpneq_ps(m_v, _mm_set1_ps(1.0f)));
  }

  Vec4 IsZero() const
  {
    return Vec4(_mm_cmpeq_ps(m_v, _mm_set1_ps(0.0f)));
  }

  Vec4 IsNotZero() const
  {
    return Vec4(_mm_cmpneq_ps(m_v, _mm_set1_ps(0.0f)));
  }

  void SwapXYZW(Vec4& with)
  {
    /* inplace swap based on xors */
    m_v = _mm_xor_ps(m_v, with.m_v);
    with.m_v = _mm_xor_ps(with.m_v, m_v);
    m_v = _mm_xor_ps(m_v, with.m_v);
  }

  void SwapXYZ(Vec4& with)
  {
    /* [old W, old W, new Z, new Z] */
    __m128 u = _mm_shuffle_ps(m_v, with.m_v, SQUISH_SSE_SHUF(3, 3, 2, 2));
    __m128 v = _mm_shuffle_ps(with.m_v, m_v, SQUISH_SSE_SHUF(3, 3, 2, 2));
    __m128 w = m_v;

    /* [new X, new Y, new Z, old W] */
    m_v = _mm_shuffle_ps(with.m_v, u, SQUISH_SSE_SHUF(0, 1, 2, 0));
    with.m_v = _mm_shuffle_ps(w, v, SQUISH_SSE_SHUF(0, 1, 2, 0));
  }

  void SwapW(Vec4& with)
  {
    /* [old Z, old Z, new W, new W] */
    __m128 u = _mm_shuffle_ps(m_v, with.m_v, SQUISH_SSE_SHUF(2, 2, 3, 3));
    __m128 v = _mm_shuffle_ps(with.m_v, m_v, SQUISH_SSE_SHUF(2, 2, 3, 3));

    /* [old X, old Y, old Z, new W] */
    m_v = _mm_shuffle_ps(m_v, u, SQUISH_SSE_SHUF(0, 1, 0, 2));
    with.m_v = _mm_shuffle_ps(with.m_v, v, SQUISH_SSE_SHUF(0, 1, 0, 2));
  }

  __m128 m_v;
};

template <class dtyp>
Vec4 LoVec4(Col8 const& v, const dtyp& dummy)
{
  return Vec4(LoCol4(v, dummy));
}

template <class dtyp>
Vec4 HiVec4(Col8 const& v, const dtyp& dummy)
{
  return Vec4(HiCol4(v, dummy));
}

template <const bool round>
Col4 FloatToUHalf(const Vec4& v)
{
  Col4 h;

  h.GetR() = FloatToUHalf(v.X());
  h.GetG() = FloatToUHalf(v.Y());
  h.GetB() = FloatToUHalf(v.Z());
  h.GetA() = FloatToUHalf(v.W());

  return h;
}

template <const bool round>
Col4 FloatToSHalf(const Vec4& v)
{
  Col4 h;

  h.GetR() = FloatToSHalf(v.X());
  h.GetG() = FloatToSHalf(v.Y());
  h.GetB() = FloatToSHalf(v.Z());
  h.GetA() = FloatToSHalf(v.W());

  return h;
}

template <const int a, const int b, const int c, const int d>
Vec4 Merge(const Vec4& lo, const Vec4& hi)
{
  return Vec4(_mm_shuffle_ps(lo.m_v, hi.m_v,
                             SQUISH_SSE_SHUF(a % 4, b % 4, c % 4, d % 4)));
}

template <const int f, const int t>
Vec4 Shuffle(const Vec4& a)
{
  if (f == t)
    return a;

  return Vec4(_mm_castsi128_ps(
    _mm_shuffle_epi32(_mm_castps_si128(a.m_v),
                      SQUISH_SSE_SHUF((t == 0 ? f : 0), (t == 1 ? f : 1),
                                      (t == 2 ? f : 2), (t == 3 ? f : 3)))));
}

template <const int f, const int t>
Vec4 Exchange(const Vec4& a)
{
  if (f == t)
    return a;

  return Vec4(_mm_castsi128_ps(_mm_shuffle_epi32(
    _mm_castps_si128(a.m_v),
    SQUISH_SSE_SHUF(
      (t == 0 ? f : (f == 0 ? t : 0)), (t == 1 ? f : (f == 1 ? t : 1)),
      (t == 2 ? f : (f == 2 ? t : 2)), (t == 3 ? f : (f == 3 ? t : 3))))));
}

template <const int n>
Vec4 RotateLeft(const Vec4& a)
{
  return Vec4(_mm_shuffle_ps(
    a.m_v, a.m_v,
    SQUISH_SSE_SHUF((n + 0) % 4, (n + 1) % 4, (n + 2) % 4, (n + 3) % 4)));
}

template <const bool disarm, const bool killw>
Vec4 Complement(const Vec4& left)
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
    // correct xÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½ + yÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½ > 1.0f by
    // renormalization
    if (_mm_comigt_ss(res, rez))
    {
      res = ReciprocalSqrt(Vec4(res)).m_v;
      res = _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(0, 0, 0, 0));

      ren = _mm_mul_ps(ren, res);
      res = rez;
    }
  }

  rez = _mm_sub_ps(rez, res);
  rez = _mm_sqrt_ps(rez);

  if (!killw)
  {
    res = _mm_shuffle_ps(ren, rez, SQUISH_SSE_SHUF(3, 3, 0, 0));
    res = _mm_shuffle_ps(ren, res, SQUISH_SSE_SHUF(0, 1, 2, 0));
  }
  else
  {
    res = _mm_movelh_ps(ren, rez);
    res = _mm_and_ps(res, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0)));
  }

  // sqrt(1.0f - (xÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½ + yÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½))
  return Vec4(res);
}

template <const bool disarm>
Vec4 Complement(Vec4& left, Vec4& right)
{
  if (!disarm)
  {
    Vec4 len = left * left + right * right;
    Vec4 adj = ReciprocalSqrt(Max(Vec4(1.0f), len));

    // correct xÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½ + yÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½ > 1.0f by
    // renormalization
    left *= adj;
    right *= adj;

    // sqrt(1.0f - (xÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½ + yÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½))
    return Sqrt(Vec4(1.0f) - Min(Vec4(1.0f), len));
  }
  else
  {
    Vec4 len = (left * left) + (right * right);

    // disarm xÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½ + yÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½ > 1.0f by
    // letting NaN happen
    // ...

    // sqrt(1.0f - (xÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½ + yÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½))
    return Sqrt(Vec4(1.0f) - len);
  }
}

template <const bool disarm>
Vec4 ComplementPyramidal(Vec4& left)
{
  // 1 - max(abs(l, r)) == 1 + min(-abs(l, r))
  Vec4 res = HorizontalMinXY(Neg(left)) + Vec4(1.0f);

  res = TransferW(Normalize(TransferZW(left, KillW(res))), left);

  return res;
}

template <const bool disarm>
Vec4 ComplementPyramidal(Vec4& left, Vec4& right)
{
  // 1 - max(abs(l, r)) == 1 + min(-abs(l, r))
  Vec4 res = Min(Neg(left), Neg(right)) + Vec4(1.0f);

  Normalize(left, right, res);

  return res;
}

template <const bool round>
Col4 FloatToInt(const Vec4& v)
{
#if (SQUISH_USE_SSE == 1)
  ...
#else
  // use SSE2 instructions
  if (round)
    return Col4(_mm_cvtps_epi32(v.m_v));
  else
    return Col4(_mm_cvttps_epi32(v.m_v));
#endif
}

inline Vec4 LengthSquared(const Vec4& v)
{
  return Dot(v, v);
}

inline void LengthSquared(const Vec4& v, float* r)
{
  Dot(v, v, r);
}

// TODO: figure out how to put static const instances into an incomplete class
// body
namespace Vec4C
{
  const Vec4 zero = Vec4(0.0f);
  const Vec4 one = Vec4(1.0f);
  const Vec4 half = Vec4(0.5f);
}
}

#endif
