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

#include "simd.h"
#if SQUISH_USE_SSE

namespace squish
{
int operator!(const Vec4& left)
{
  return CompareFirstEqualTo(left, Vec4(0.0f));
}

int operator<(const Vec4& left, const Vec4& right)
{
  return CompareFirstLessThan(left, right);
}

int operator>(const Vec4& left, const Vec4& right)
{
  return CompareFirstGreaterThan(left, right);
}

int operator>=(const Vec4& left, const Vec4& right)
{
  return CompareFirstGreaterEqualTo(left, right);
}

int operator==(const Vec4& left, const Vec4& right)
{
  return CompareFirstEqualTo(left, right);
}

Vec4 operator&(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_and_ps(left.m_v, right.m_v));
}

Vec4 operator%(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_andnot_ps(left.m_v, right.m_v));
}

Vec4 operator+(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_add_ps(left.m_v, right.m_v));
}

Vec4 operator-(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_sub_ps(left.m_v, right.m_v));
}

Vec4 operator*(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_mul_ps(left.m_v, right.m_v));
}

Vec4 operator*(const Vec4& left, float right)
{
  return Vec4(_mm_mul_ps(left.m_v, _mm_set1_ps(right)));
}

Vec4 operator*(float left, const Vec4& right)
{
  return Vec4(_mm_mul_ps(_mm_set1_ps(left), right.m_v));
}

Vec4 operator/(const Vec4& left, float right)
{
  return left * Reciprocal(Vec4(right));
}

Vec4 operator*(const Vec4& left, int right)
{
#if (SQUISH_USE_SSE == 1)
  ...
#else
  return Vec4(_mm_mul_ps(left.m_v, _mm_cvtepi32_ps(_mm_set1_epi32(right))));
#endif
}

//! Returns a*b + c
Vec4 MultiplyAdd(const Vec4& a, const Vec4& b, const Vec4& c)
{
  return Vec4(_mm_add_ps(_mm_mul_ps(a.m_v, b.m_v), c.m_v));
}

//! Returns -( a*b - c )
Vec4 NegativeMultiplySubtract(const Vec4& a, const Vec4& b, const Vec4& c)
{
  return Vec4(_mm_sub_ps(c.m_v, _mm_mul_ps(a.m_v, b.m_v)));
}

Vec4 Threshold(const Vec4& a, const Vec4& b)
{
  __m128 mask = _mm_cmpge_ps(a.m_v, b.m_v);
  __m128 res = _mm_and_ps(_mm_set1_ps(1.0f), mask);

  return Vec4(res);
}

Vec4 Select(const Vec4& a, const Vec4& b, const Vec4& c)
{
#if 0
  __m128 res;
  __m128 bits = _mm_cmpeq_ps( b.m_v, c.m_v );
  int mask = _mm_movemask_ps( bits );

  /* (1 >> 1) = 0
   * (2 >> 1) = 1
   * (4 >> 1) = 2
  mask = (mask & 7) >> 1;
  mask = (mask) * ((1 << 0) + (1 << 2) + (1 << 4) + (1 << 6));
   */

  /**/ if (mask & 1)
    res = _mm_shuffle_ps( a.m_v, a.m_v, SQUISH_SSE_SHUF( 0, 0, 0, 0 ) );
  else if (mask & 2)
    res = _mm_shuffle_ps( a.m_v, a.m_v, SQUISH_SSE_SHUF( 1, 1, 1, 1 ) );
  else if (mask & 4)
    res = _mm_shuffle_ps( a.m_v, a.m_v, SQUISH_SSE_SHUF( 2, 2, 2, 2 ) );
  else
    res = _mm_shuffle_ps( a.m_v, a.m_v, SQUISH_SSE_SHUF( 3, 3, 3, 3 ) );

  return Vec3( res );
#else
  // branch free, and no CPU<->SSEunit transfer
  __m128 mask = _mm_cmpeq_ps(b.m_v, c.m_v);
  __m128 res = _mm_and_ps(a.m_v, mask);

  __m128 r0 = _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(0, 0, 0, 0));
  __m128 r1 = _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 1, 1, 1));
  __m128 r2 = _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(2, 2, 2, 2));
  __m128 r3 = _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(3, 3, 3, 3));

  res = _mm_or_ps(_mm_or_ps(r0, r1), _mm_or_ps(r2, r3));

  return Vec4(res);
#endif
}

Vec4 HorizontalAdd(const Vec4& a)
{
#if (SQUISH_USE_SSE >= 4) && 0
  __m128 res = a.m_v;

  res = _mm_dp_ps(res, _mm_set1_ps(1.0f), 0xFF);

  return Vec4(res);
#elif (SQUISH_USE_SSE >= 3)
  __m128 res = a.m_v;

  res = _mm_hadd_ps(res, res);
  res = _mm_hadd_ps(res, res);

  return Vec4(res);
#else
  __m128 res = a.m_v;

  res = _mm_add_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP64()));
  res = _mm_add_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP32()));

  return Vec4(res);
#endif
}

Vec4 HorizontalAdd(const Vec4& a, const Vec4& b)
{
#if (SQUISH_USE_SSE >= 3)
  __m128 resc;

  resc = _mm_hadd_ps(a.m_v, b.m_v);
  resc = _mm_hadd_ps(resc, resc);
  resc = _mm_hadd_ps(resc, resc);

  return Vec4(resc);
#else
  __m128 resc;

  resc = _mm_add_ps(a.m_v, b.m_v);
  resc = _mm_add_ps(resc, _mm_shuffle_ps(resc, resc, SQUISH_SSE_SWAP64()));
  resc = _mm_add_ps(resc, _mm_shuffle_ps(resc, resc, SQUISH_SSE_SWAP32()));

  return Vec4(resc);
#endif
}

Vec4 HorizontalMin(const Vec4& a)
{
  __m128 res = a.m_v;

  res = _mm_min_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP64()));
  res = _mm_min_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP32()));

  return Vec4(res);
}

Vec4 HorizontalMax(const Vec4& a)
{
  __m128 res = a.m_v;

  res = _mm_max_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP64()));
  res = _mm_max_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP32()));

  return Vec4(res);
}

Vec4 HorizontalMaxXY(const Vec4& a)
{
  __m128 res = a.m_v;

  res = _mm_max_ps(_mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(0, 1, 0, 1)),
                   _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 0, 1, 0)));

  return Vec4(res);
}

Vec4 HorizontalMinXY(const Vec4& a)
{
  __m128 res = a.m_v;

  res = _mm_min_ps(_mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(0, 1, 0, 1)),
                   _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 0, 1, 0)));

  return Vec4(res);
}

Vec4 Reciprocal(const Vec4& v)
{
  // get the reciprocal estimate
  __m128 estimate = _mm_rcp_ps(v.m_v);

  // one round of Newton-Rhaphson refinement
  __m128 diff = _mm_sub_ps(_mm_set1_ps(1.0f), _mm_mul_ps(estimate, v.m_v));
  return Vec4(_mm_add_ps(_mm_mul_ps(diff, estimate), estimate));
}

Vec4 ReciprocalSqrt(const Vec4& v)
{
  // get the reciprocal estimate
  __m128 estimate = _mm_rsqrt_ps(v.m_v);

  // one round of Newton-Rhaphson refinement
  __m128 diff = _mm_sub_ps(_mm_set1_ps(3.0f),
                           _mm_mul_ps(estimate, _mm_mul_ps(estimate, v.m_v)));
  return Vec4(_mm_mul_ps(_mm_mul_ps(diff, _mm_set1_ps(0.5f)), estimate));
}

Vec4 Sqrt(const Vec4& v)
{
  return Vec4(_mm_sqrt_ps(v.m_v));
}

Vec4 Length(const Vec4& left)
{
  Vec4 sum = HorizontalAdd(Vec4(_mm_mul_ps(left.m_v, left.m_v)));
  Vec4 sqt = Vec4(_mm_sqrt_ps(sum.m_v));

  return sqt;
}

Vec4 ReciprocalLength(const Vec4& left)
{
  Vec4 sum = HorizontalAdd(Vec4(_mm_mul_ps(left.m_v, left.m_v)));
  Vec4 rsq = ReciprocalSqrt(sum);

  return rsq;
}

Vec4 Normalize(const Vec4& left)
{
  Vec4 sum = HorizontalAdd(Vec4(_mm_mul_ps(left.m_v, left.m_v)));
  Vec4 rsq = ReciprocalSqrt(sum);

  return left * rsq;
}

Vec4 Normalize(Vec4& x, Vec4& y, Vec4& z)
{
  Vec4 xx = x * x;
  Vec4 yy = y * y;
  Vec4 zz = z * z;

  Vec4 sum = xx + yy + zz;
  Vec4 rsq = ReciprocalSqrt(sum);

  x = x * rsq;
  y = y * rsq;
  z = z * rsq;

  return rsq;
}

Vec4 Dot(const Vec4& left, const Vec4& right)
{
#if (SQUISH_USE_SSE >= 4)
  return Vec4(_mm_dp_ps(left.m_v, right.m_v, 0xFF));
#else
  return HorizontalAdd(Vec4(_mm_mul_ps(left.m_v, right.m_v)));
#endif
}

void Dot(const Vec4& left, const Vec4& right, float* r)
{
  Vec4 res = Dot(left, right);

  _mm_store_ss(r, res.m_v);
}

Vec4 Abs(const Vec4& a)
{
  return Vec4(_mm_and_ps(a.m_v, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF))));
}

Vec4 Neg(const Vec4& a)
{
  return Vec4(_mm_or_ps(a.m_v, _mm_castsi128_ps(_mm_set1_epi32(0x80000000))));
}

Vec4 Min(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_min_ps(left.m_v, right.m_v));
}

Vec4 Max(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_max_ps(left.m_v, right.m_v));
}

Vec4 Truncate(const Vec4& v)
{
#if (SQUISH_USE_SSE == 1)
  // convert to ints
  __m128 input = v.m_v;
  __m64 lo = _mm_cvttps_pi32(input);
  __m64 hi = _mm_cvttps_pi32(_mm_movehl_ps(input, input));

  // convert to floats
  __m128 part = _mm_movelh_ps(input, _mm_cvtpi32_ps(input, hi));
  __m128 truncated = _mm_cvtpi32_ps(part, lo);

  // clear out the MMX multimedia state to allow FP calls later
  _mm_empty();

  return Vec4(truncated);
#else
  // use SSE2 instructions
  return Vec4(_mm_cvtepi32_ps(_mm_cvttps_epi32(v.m_v)));
#endif
}

Vec4 AbsoluteDifference(const Vec4& left, const Vec4& right)
{
  __m128 diff = _mm_sub_ps(left.m_v, right.m_v);
  diff = _mm_and_ps(diff, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)));
  return Vec4(diff);
}

Vec4 SummedAbsoluteDifference(const Vec4& left, const Vec4& right)
{
  return HorizontalAdd(AbsoluteDifference(left, right));
}

Vec4 MaximumAbsoluteDifference(const Vec4& left, const Vec4& right)
{
  return HorizontalMax(AbsoluteDifference(left, right));
}

int CompareEqualTo(const Vec4& left, const Vec4& right)
{
  return _mm_movemask_ps(_mm_cmpeq_ps(left.m_v, right.m_v));
}

int CompareNotEqualTo(const Vec4& left, const Vec4& right)
{
  return _mm_movemask_ps(_mm_cmpneq_ps(left.m_v, right.m_v));
}

int CompareLessThan(const Vec4& left, const Vec4& right)
{
  return _mm_movemask_ps(_mm_cmplt_ps(left.m_v, right.m_v));
}

int CompareGreaterThan(const Vec4& left, const Vec4& right)
{
  return _mm_movemask_ps(_mm_cmpgt_ps(left.m_v, right.m_v));
}

int CompareGreaterEqual(const Vec4& left, const Vec4& right)
{
  return _mm_movemask_ps(_mm_cmpge_ps(left.m_v, right.m_v));
}

bool CompareAnyLessThan(const Vec4& left, const Vec4& right)
{
  __m128 bits = _mm_cmplt_ps(left.m_v, right.m_v);
  int value = _mm_movemask_ps(bits);
  return value != 0x0;
}

bool CompareAnyGreaterThan(const Vec4& left, const Vec4& right)
{
  __m128 bits = _mm_cmpgt_ps(left.m_v, right.m_v);
  int value = _mm_movemask_ps(bits);
  return value != 0x0;
}

bool CompareAllEqualTo(const Vec4& left, const Vec4& right)
{
  __m128 bits = _mm_cmpeq_ps(left.m_v, right.m_v);
  int value = _mm_movemask_ps(bits);
  return value == 0xF;
}

Col4 CompareAllEqualTo_M4(const Vec4& left, const Vec4& right)
{
  return Col4(
    _mm_cmpeq_epi32(_mm_castps_si128(left.m_v), _mm_castps_si128(right.m_v)));
}

Col4 CompareAllEqualTo_M8(const Vec4& left, const Vec4& right)
{
  return Col4(
    _mm_cmpeq_epi8(_mm_castps_si128(left.m_v), _mm_castps_si128(right.m_v)));
}

int CompareFirstLessThan(const Vec4& left, const Vec4& right)
{
  return _mm_comilt_ss(left.m_v, right.m_v);
}

int CompareFirstLessEqualTo(const Vec4& left, const Vec4& right)
{
  return _mm_comile_ss(left.m_v, right.m_v);
}

int CompareFirstGreaterThan(const Vec4& left, const Vec4& right)
{
  return _mm_comigt_ss(left.m_v, right.m_v);
}

int CompareFirstGreaterEqualTo(const Vec4& left, const Vec4& right)
{
  return _mm_comige_ss(left.m_v, right.m_v);
}

int CompareFirstEqualTo(const Vec4& left, const Vec4& right)
{
  return _mm_comieq_ss(left.m_v, right.m_v);
}

Vec4 IsGreaterThan(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_cmpgt_ps(left.m_v, right.m_v));
}

Vec4 IsGreaterEqual(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_cmpge_ps(left.m_v, right.m_v));
}

Vec4 IsNotEqualTo(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_cmpneq_ps(left.m_v, right.m_v));
}

Vec4 TransferW(const Vec4& left, const Vec4& right)
{
  /* [new W, ....., ....., old Z] */
  // m128 u = _mm_unpackhi_ps( left.m_v, right.m_v );
  /* [new W, new W, old Z, old Z] */
  __m128 u = _mm_shuffle_ps(left.m_v, right.m_v, SQUISH_SSE_SHUF(2, 2, 3, 3));
  /* [new W, old Z, old Y, old X] */
  u = _mm_shuffle_ps(left.m_v, u, SQUISH_SSE_SHUF(0, 1, 0, 2));

  return Vec4(u);
}

Vec4 TransferZW(const Vec4& left, const Vec4& right)
{
  return Vec4(_mm_shuffle_ps(left.m_v, right.m_v, SQUISH_SSE_SHUF(0, 1, 2, 3)));
}

Vec4 KillW(const Vec4& left)
{
  return Vec4(
    _mm_and_ps(left.m_v, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0))));
}

Vec4 OnlyW(const Vec4& left)
{
  return Vec4(
    _mm_and_ps(left.m_v, _mm_castsi128_ps(_mm_setr_epi32(0, 0, 0, ~0))));
}

Vec4 CollapseW(const Vec4& x, const Vec4& y, const Vec4& z, const Vec4& w)
{
  return Vec4(_mm_unpackhi_ps(_mm_unpackhi_ps(x.m_v, z.m_v),
                              _mm_unpackhi_ps(y.m_v, w.m_v)));
}

void LoadAligned(Vec4& a, Vec4& b, const Vec4& c)
{
  a.m_v = c.m_v;
  b.m_v = _mm_shuffle_ps(a.m_v, a.m_v, SQUISH_SSE_SWAP64());
}

void LoadAligned(Vec4& a, void const* source)
{
  a.m_v = _mm_load_ps((float const*)source);
}

void LoadUnaligned(Vec4& a, void const* source)
{
  a.m_v = _mm_loadu_ps((float const*)source);
}

void LoadAligned(Vec4& a, Vec4& b, void const* source)
{
  a.m_v = _mm_load_ps((float const*)source);
  b.m_v = _mm_shuffle_ps(a.m_v, a.m_v, SQUISH_SSE_SWAP64());
}

void LoadUnaligned(Vec4& a, Vec4& b, void const* source)
{
  a.m_v = _mm_loadu_ps((float const*)source);
  b.m_v = _mm_shuffle_ps(a.m_v, a.m_v, SQUISH_SSE_SWAP64());
}

void StoreAligned(const Vec4& a, const Vec4& b, Vec4& c)
{
  c.m_v = _mm_unpacklo_ps(a.m_v, b.m_v);
}

void StoreAligned(const Vec4& a, void* destination)
{
  _mm_store_ps((float*)destination, a.m_v);
}

void StoreAligned(const Vec4& a, const Vec4& b, void* destination)
{
  _mm_store_ps((float*)destination, _mm_unpacklo_ps(a.m_v, b.m_v));
}

void StoreUnaligned(const Vec4& a, void* destination)
{
  _mm_storeu_ps((float*)destination, a.m_v);
}

void StoreUnaligned(const Vec4& a, const Vec4& b, void* destination)
{
  _mm_storeu_ps((float*)destination, _mm_unpacklo_ps(a.m_v, b.m_v));
}

Vec4 UHalfToFloat(const Col4& v)
{
  Vec4 f;

  f.GetX() = UHalfToFloat((std::uint16_t)v.R());
  f.GetY() = UHalfToFloat((std::uint16_t)v.G());
  f.GetZ() = UHalfToFloat((std::uint16_t)v.B());
  f.GetW() = UHalfToFloat((std::uint16_t)v.A());

  return f;
}

Vec4 SHalfToFloat(const Col4& v)
{
  Vec4 f;

  f.GetX() = SHalfToFloat((std::uint16_t)v.R());
  f.GetY() = SHalfToFloat((std::uint16_t)v.G());
  f.GetZ() = SHalfToFloat((std::uint16_t)v.B());
  f.GetW() = SHalfToFloat((std::uint16_t)v.A());

  return f;
}
}

#endif
