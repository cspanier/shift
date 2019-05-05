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
int operator!(const Vec3& left)
{
  return CompareFirstEqualTo(left, Vec3(0.0f));
}

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

Vec3 operator&(const Vec3& left, const Vec3& right)
{
  return Vec3(_mm_and_ps(left.m_v, right.m_v));
}

Vec3 operator%(const Vec3& left, const Vec3& right)
{
  return Vec3(_mm_andnot_ps(left.m_v, right.m_v));
}

Vec3 operator+(const Vec3& left, const Vec3& right)
{
  return Vec3(_mm_add_ps(left.m_v, right.m_v));
}

Vec3 operator-(const Vec3& left, const Vec3& right)
{
  return Vec3(_mm_sub_ps(left.m_v, right.m_v));
}

Vec3 operator*(const Vec3& left, const Vec3& right)
{
  return Vec3(_mm_mul_ps(left.m_v, right.m_v));
}

Vec3 operator*(const Vec3& left, float right)
{
  return Vec3(_mm_mul_ps(left.m_v, _mm_set1_ps(right)));
}

Vec3 operator*(float left, const Vec3& right)
{
  return Vec3(_mm_mul_ps(_mm_set1_ps(left), right.m_v));
}

Vec3 operator/(const Vec3& left, float right)
{
  return left * Reciprocal(Vec3(right));
}

Vec3 operator*(const Vec3& left, int right)
{
#if (SQUISH_USE_SSE == 1)
  ...
#else
  return Vec3(_mm_mul_ps(left.m_v, _mm_cvtepi32_ps(_mm_set1_epi32(right))));
#endif
}

//! Returns a*b + c
Vec3 MultiplyAdd(const Vec3& a, const Vec3& b, const Vec3& c)
{
  return Vec3(_mm_add_ps(_mm_mul_ps(a.m_v, b.m_v), c.m_v));
}

//! Returns -( a*b - c )
Vec3 NegativeMultiplySubtract(const Vec3& a, const Vec3& b, const Vec3& c)
{
  return Vec3(_mm_sub_ps(c.m_v, _mm_mul_ps(a.m_v, b.m_v)));
}

Vec3 HorizontalAdd(const Vec3& a)
{
#if (SQUISH_USE_SSE >= 3)
  __m128 res = a.m_v;

  res = _mm_and_ps(res, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0)));
  res = _mm_hadd_ps(res, res);
  res = _mm_hadd_ps(res, res);

  return Vec3(res);
#else
  __m128 res = a.m_v;

  res = _mm_and_ps(res, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0)));
  res = _mm_add_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP64()));
  res = _mm_add_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP32()));

  return Vec3(res);
#endif
}

Vec3 HorizontalAdd(const Vec3& a, const Vec3& b)
{
#if (SQUISH_USE_SSE >= 3)
  __m128 resc;

  resc = _mm_hadd_ps(a.m_v, b.m_v);
  resc = _mm_and_ps(resc, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0)));
  resc = _mm_hadd_ps(resc, resc);
  resc = _mm_hadd_ps(resc, resc);

  return Vec3(resc);
#else
  __m128 resc;

  resc = _mm_add_ps(a.m_v, b.m_v);
  resc = _mm_and_ps(resc, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0)));
  resc = _mm_add_ps(resc, _mm_shuffle_ps(resc, resc, SQUISH_SSE_SWAP64()));
  resc = _mm_add_ps(resc, _mm_shuffle_ps(resc, resc, SQUISH_SSE_SWAP32()));

  return Vec3(resc);
#endif
}

Vec3 Select(const Vec3& a, const Vec3& b, const Vec3& c)
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
  else
    res = _mm_shuffle_ps( a.m_v, a.m_v, SQUISH_SSE_SHUF( 2, 2, 2, 2 ) );

  return Vec3( res );
#else
  // branch free, and no CPU<->SSEunit transfer
  __m128 mask = _mm_cmpeq_ps(b.m_v, c.m_v);
  __m128 res = _mm_and_ps(a.m_v, mask);

  __m128 r0 = _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(0, 0, 0, 0));
  __m128 r1 = _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 1, 1, 1));
  __m128 r2 = _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(2, 2, 2, 2));

  res = _mm_or_ps(_mm_or_ps(r0, r1), r2);

  return Vec3(res);
#endif
}

Vec3 HorizontalMin(const Vec3& a)
{
  __m128 res = a.m_v;

  res = _mm_min_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(2, 0, 1, 3)));
  res = _mm_min_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 2, 0, 3)));

  return Vec3(res);
}

Vec3 HorizontalMax(const Vec3& a)
{
  __m128 res = a.m_v;

  res = _mm_max_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(2, 0, 1, 3)));
  res = _mm_max_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 2, 0, 3)));

  return Vec3(res);
}

Vec3 HorizontalMaxXY(const Vec3& a)
{
  __m128 res = a.m_v;

  res = _mm_max_ps(_mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(0, 1, 0, 1)),
                   _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 0, 1, 0)));

  return Vec3(res);
}

Vec3 HorizontalMinXY(const Vec3& a)
{
  __m128 res = a.m_v;

  res = _mm_min_ps(_mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(0, 1, 0, 1)),
                   _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 0, 1, 0)));

  return Vec3(res);
}

Vec3 Reciprocal(const Vec3& v)
{
  // get the reciprocal estimate
  __m128 estimate = _mm_rcp_ps(v.m_v);

  // one round of Newton-Rhaphson refinement
  __m128 diff = _mm_sub_ps(_mm_set1_ps(1.0f), _mm_mul_ps(estimate, v.m_v));
  return Vec3(_mm_add_ps(_mm_mul_ps(diff, estimate), estimate));
}

Vec3 ReciprocalSqrt(const Vec3& v)
{
  // get the reciprocal estimate
  __m128 estimate = _mm_rsqrt_ps(v.m_v);

  // one round of Newton-Rhaphson refinement
  __m128 diff = _mm_sub_ps(_mm_set1_ps(3.0f),
                           _mm_mul_ps(estimate, _mm_mul_ps(estimate, v.m_v)));
  return Vec3(_mm_mul_ps(_mm_mul_ps(diff, _mm_set1_ps(0.5f)), estimate));
}

Vec3 Sqrt(const Vec3& v)
{
  return Vec3(_mm_sqrt_ps(v.m_v));
}

Vec3 Length(const Vec3& left)
{
  Vec3 sum = HorizontalAdd(Vec3(_mm_mul_ps(left.m_v, left.m_v)));
  Vec3 sqt = Vec3(_mm_sqrt_ps(sum.m_v));

  return sqt;
}

Vec3 ReciprocalLength(const Vec3& left)
{
  Vec3 sum = HorizontalAdd(Vec3(_mm_mul_ps(left.m_v, left.m_v)));
  Vec3 rsq = ReciprocalSqrt(sum);

  return rsq;
}

Vec3 Normalize(const Vec3& left)
{
  return left * ReciprocalLength(left);
}

Vec3 Normalize(Vec3& x, Vec3& y, Vec3& z)
{
  Vec3 xx = x * x;
  Vec3 yy = y * y;
  Vec3 zz = z * z;

  Vec3 sum = xx + yy + zz;
  Vec3 rsq = ReciprocalSqrt(sum);

  x = x * rsq;
  y = y * rsq;
  z = z * rsq;

  return rsq;
}

Vec3 Dot(const Vec3& left, const Vec3& right)
{
#if (SQUISH_USE_SSE >= 4)
  return Vec3(_mm_dp_ps(left.m_v, right.m_v, 0x77));
#else
  return HorizontalAdd(Vec3(_mm_mul_ps(left.m_v, right.m_v)));
#endif
}

void Dot(const Vec3& left, const Vec3& right, float* r)
{
  Vec3 res = Dot(left, right);

  _mm_store_ss(r, res.m_v);
}

Vec3 Abs(const Vec3& a)
{
  return Vec3(_mm_and_ps(a.m_v, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF))));
}

Vec3 Neg(const Vec3& a)
{
  return Vec3(_mm_or_ps(a.m_v, _mm_castsi128_ps(_mm_set1_epi32(0x80000000))));
}

Vec3 Min(const Vec3& left, const Vec3& right)
{
  return Vec3(_mm_min_ps(left.m_v, right.m_v));
}

Vec3 Max(const Vec3& left, const Vec3& right)
{
  return Vec3(_mm_max_ps(left.m_v, right.m_v));
}

Vec3 UHalfToFloat(const Col3& v)
{
  Vec3 f;

  f.GetX() = UHalfToFloat((std::uint16_t)v.R());
  f.GetY() = UHalfToFloat((std::uint16_t)v.G());
  f.GetZ() = UHalfToFloat((std::uint16_t)v.B());

  return f;
}

Vec3 SHalfToFloat(const Col3& v)
{
  Vec3 f;

  f.GetX() = SHalfToFloat((std::uint16_t)v.R());
  f.GetY() = SHalfToFloat((std::uint16_t)v.G());
  f.GetZ() = SHalfToFloat((std::uint16_t)v.B());

  return f;
}

Vec3 Truncate(const Vec3& v)
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
  return Vec3(truncated);
#else
  // use SSE2 instructions
  return Vec3(_mm_cvtepi32_ps(_mm_cvttps_epi32(v.m_v)));
#endif
}

Vec3 AbsoluteDifference(const Vec3& left, const Vec3& right)
{
  __m128 diff = _mm_sub_ps(left.m_v, right.m_v);
  diff = _mm_and_ps(diff, _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF)));
  return Vec3(diff);
}

Vec3 SummedAbsoluteDifference(const Vec3& left, const Vec3& right)
{
  return HorizontalAdd(AbsoluteDifference(left, right));
}

Vec3 MaximumAbsoluteDifference(const Vec3& left, const Vec3& right)
{
  return HorizontalMax(AbsoluteDifference(left, right));
}

bool CompareAnyLessThan(const Vec3& left, const Vec3& right)
{
  __m128 bits = _mm_cmplt_ps(left.m_v, right.m_v);
  int value = _mm_movemask_ps(bits);
  return (value & 0x7) != 0x0;
}

bool CompareAnyGreaterThan(const Vec3& left, const Vec3& right)
{
  __m128 bits = _mm_cmpgt_ps(left.m_v, right.m_v);
  int value = _mm_movemask_ps(bits);
  return (value & 0x7) != 0x0;
}

bool CompareAllEqualTo(const Vec3& left, const Vec3& right)
{
  __m128 bits = _mm_cmpeq_ps(left.m_v, right.m_v);
  int value = _mm_movemask_ps(bits);
  return (value & 0x7) == 0x7;
}

Col3 CompareAllEqualTo_M8(const Vec3& left, const Vec3& right)
{
  return Col3(
    _mm_cmpeq_epi8(_mm_castps_si128(left.m_v), _mm_castps_si128(right.m_v)));
}

int CompareFirstLessThan(const Vec3& left, const Vec3& right)
{
  return _mm_comilt_ss(left.m_v, right.m_v);
}

int CompareFirstGreaterThan(const Vec3& left, const Vec3& right)
{
  return _mm_comigt_ss(left.m_v, right.m_v);
}

int CompareFirstEqualTo(const Vec3& left, const Vec3& right)
{
  return _mm_comieq_ss(left.m_v, right.m_v);
}

Vec3 TransferZ(const Vec3& left, const Vec3& right)
{
  return Vec3(_mm_shuffle_ps(left.m_v, right.m_v, SQUISH_SSE_SHUF(0, 1, 2, 3)));
}

void LoadAligned(Vec3& a, Vec3& b, const Vec3& c)
{
  a.m_v = c.m_v;
  b.m_v = _mm_shuffle_ps(a.m_v, a.m_v, SQUISH_SSE_SWAP64());
  b.m_v = _mm_and_ps(b.m_v, _mm_castsi128_ps(_mm_setr_epi32(~0, 0, 0, 0)));
}

void LoadAligned(Vec3& a, void const* source)
{
  a.m_v = _mm_load_ps((float const*)source);
  a.m_v = _mm_and_ps(a.m_v, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0)));
}

void LoadUnaligned(Vec3& a, void const* source)
{
  a.m_v = _mm_loadu_ps((float const*)source);
  a.m_v = _mm_and_ps(a.m_v, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0)));
}

void LoadAligned(Vec3& a, Vec3& b, void const* source)
{
  a.m_v = _mm_load_ps((float const*)source);
  b.m_v = _mm_shuffle_ps(a.m_v, a.m_v, SQUISH_SSE_SWAP64());
  b.m_v = _mm_and_ps(b.m_v, _mm_castsi128_ps(_mm_setr_epi32(~0, 0, 0, 0)));
}

void LoadUnaligned(Vec3& a, Vec3& b, void const* source)
{
  a.m_v = _mm_loadu_ps((float const*)source);
  b.m_v = _mm_shuffle_ps(a.m_v, a.m_v, SQUISH_SSE_SWAP64());
  b.m_v = _mm_and_ps(b.m_v, _mm_castsi128_ps(_mm_setr_epi32(~0, 0, 0, 0)));
}

void StoreUnaligned(const Vec3& a, void* destination)
{
  _mm_storeu_ps((float*)destination, a.m_v);
}
}

#endif
