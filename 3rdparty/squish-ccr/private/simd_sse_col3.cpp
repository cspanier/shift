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
int operator<(const Col3& left, const Col3& right)
{
  return CompareFirstLessThan(left, right);
}

int operator>(const Col3& left, const Col3& right)
{
  return CompareFirstGreaterThan(left, right);
}

int operator==(const Col3& left, const Col3& right)
{
  return CompareFirstEqualTo(left, right);
}

Col3 operator~(const Col3& left)
{
  return left ^ Col3(~0);
}

Col3 operator&(const Col3& left, const Col3& right)
{
  return Col3(_mm_and_si128(left.m_v, right.m_v));
}

Col3 operator%(const Col3& left, const Col3& right)
{
  return Col3(_mm_andnot_si128(left.m_v, right.m_v));
}

Col3 operator^(const Col3& left, const Col3& right)
{
  return Col3(_mm_xor_si128(left.m_v, right.m_v));
}

Col3 operator|(const Col3& left, const Col3& right)
{
  return Col3(_mm_or_si128(left.m_v, right.m_v));
}

Col3 operator>>(const Col3& left, int right)
{
  return Col3(_mm_srli_epi32(left.m_v, right));
}

Col3 operator<<(const Col3& left, int right)
{
  return Col3(_mm_slli_epi32(left.m_v, right));
}

Col3 operator+(const Col3& left, const Col3& right)
{
  return Col3(_mm_add_epi32(left.m_v, right.m_v));
}

Col3 operator-(const Col3& left, const Col3& right)
{
  return Col3(_mm_sub_epi32(left.m_v, right.m_v));
}

Col3 operator*(const Col3& left, const Col3& right)
{
#if (SQUISH_USE_SSE >= 4)
  return Col3(_mm_mullo_epi32(left.m_v, right.m_v));
#else
  return Col3(_mm_mullo_epi16(left.m_v, right.m_v));
#endif
}

Col3 operator*(const Col3& left, int right)
{
#if (SQUISH_USE_SSE >= 4)
  return Col3(_mm_mullo_epi32(left.m_v, _mm_set1_epi32(right)));
#else
  return Col3(_mm_mullo_epi16(left.m_v, _mm_set1_epi32(right)));
#endif
}

template <const int n>
Col3 ShiftLeft(const Col3& a);
template <const int n>
Col3 ShiftLeft(const Col3& a)
{
  if ((n) <= 0)
    return Col3(a.m_v);
  if ((n) <= 7)
    return Col3(_mm_slli_epi32(a.m_v, (n)&7));
  if ((n)&7)
    return Col3(_mm_slli_epi32(_mm_slli_si128(a.m_v, (n) >> 3), (n)&7));

  return Col3(_mm_slli_si128(a.m_v, (n) >> 3));
}

template <const int n>
Col3 ShiftRight(const Col3& a);
template <const int n>
Col3 ShiftRight(const Col3& a)
{
  if ((n) <= 0)
    return Col3(a.m_v);
  if ((n) <= 7)
    return Col3(_mm_srli_epi32(a.m_v, (n)&7));
  if ((n)&7)
    return Col3(_mm_srli_epi32(_mm_srli_si128(a.m_v, (n) >> 3), (n)&7));

  return Col3(_mm_srli_si128(a.m_v, (n) >> 3));
}

template <const int n>
Col3 ShiftRightHalf(const Col3& a);
template <const int n>
Col3 ShiftRightHalf(const Col3& a)
{
  return Col3((n) > 0 ? _mm_srli_epi64(a.m_v, (n)) : a.m_v);
}

Col3 ShiftRightHalf(const Col3& a, const int n)
{
  return Col3(_mm_srl_epi64(a.m_v, _mm_cvtsi32_si128(n)));
}

Col3 ShiftRightHalf(const Col3& a, const Col3& b)
{
  return Col3(_mm_srl_epi64(a.m_v, b.m_v));
}

template <const int n>
Col3 ShiftLeftHalf(const Col3& a);
template <const int n>
Col3 ShiftLeftHalf(const Col3& a)
{
  return Col3((n) > 0 ? _mm_slli_epi64(a.m_v, (n)) : a.m_v);
}

Col3 ShiftLeftHalf(const Col3& a, const int n)
{
  return Col3(_mm_sll_epi64(a.m_v, _mm_cvtsi32_si128(n)));
}

template <const int r, const int g, const int b>
Col3 ShiftLeftLo(const Col3& v)
{
  // (1 << r, 1 << g, 1 << b);
  Col3 p2;
  p2.SetRGBpow2<0>(r, g, b);

  //  return Col3( _mm_mullo_epi32( v.m_v, p2.m_v ) );
  return Col3(_mm_mullo_epi16(v.m_v, p2.m_v));
}

template <const int n, const int p>
Col3 MaskBits(const Col3& a);
template <const int n, const int p>
Col3 MaskBits(const Col3& a)
{
  if ((p + n) <= 0)
    return Col3(0);
  if ((p + n) >= 64)
    return a;

  // compile time
  std::int64_t base = ~(0xFFFFFFFFFFFFFFFFULL << ((p + n) & 63));
  //  std::int64_t base =  (0xFFFFFFFFFFFFFFFFULL >> (64 - (p + n) & 63));
  __m128i mask = _mm_setr_epi32((int)(base >> 0), (int)(base >> 32), 0, 0);

  return Col3(_mm_and_si128(a.m_v, mask));
}

Col3 MaskBits(const Col3& a, const int n, const int p)
{
  const int val = 64 - (p + n);

  __m128i shift = _mm_max_epi16(_mm_cvtsi32_si128(val), _mm_set1_epi32(0));
  __m128i mask = _mm_setr_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0);

  mask = _mm_srl_epi64(mask, shift);

  // (0xFFFFFFFFFFFFFFFFULL >> (64 - (p + n) & 63))
  return Col3(_mm_and_si128(a.m_v, mask));
}

template <const int n, const int p>
Col3 CopyBits(const Col3& left, const Col3& right);
template <const int n, const int p>
Col3 CopyBits(const Col3& left, const Col3& right)
{
  if (!(n))
    return left;
  if (!(p))
    return MaskBits<n, 0>(right);
  if (((p) + (n)) >= 64)
    return (left) + ShiftLeftHalf<p>(right);

#if (SQUISH_USE_XSSE == 4)
  return Col3(_mm_inserti_si64(left.m_v, right.m_v, n, p));
#else
  return MaskBits<p, 0>(left) + MaskBits<n, p>(ShiftLeftHalf<p>(right));
  //  return               (left) + MaskBits<n, p>(ShiftLeftHalf<p>(right));
#endif
}

Col3 CopyBits(const Col3& left, Col3& right, const int n, const int p)
{
#if (SQUISH_USE_XSSE == 4)
  /* ---- ---bl xxxx xxxx */
  const int val = (p << 8) + (n << 0);

  right.m_v = _mm_unpacklo_epi64(right.m_v, _mm_cvtsi32_si128(val));
  return Col3(_mm_insert_si64(left.m_v, right.m_v));
#else
  return MaskBits(left, p, 0) + MaskBits(ShiftLeftHalf(right, p), n, p);
  //  return         (left      ) + MaskBits(ShiftLeftHalf(right, p), n, p);
#endif
}

template <const int n, const int p>
Col3 ExtrBits(const Col3& a);
template <const int n, const int p>
Col3 ExtrBits(const Col3& a)
{
  if (!(n))
    return Col3(0);
  if (!(p))
    return MaskBits<n, 0>(a);
  if (((n) + (p)) >= 64)
    return ShiftRightHalf<p>(a);

#if (SQUISH_USE_XSSE == 4)
  return Col3(_mm_extracti_si64(a.m_v, n, p));
#else
  return MaskBits<n, 0>(ShiftRightHalf<p>(a));
#endif
}

Col3 ExtrBits(const Col3& a, const int n, const int p)
{
#if (SQUISH_USE_XSSE == 4)
  /* ---- ----- ---- ---bl */
  const int val = (p << 8) + (n << 0);

  return Col3(_mm_extract_si64(a.m_v, _mm_cvtsi32_si128(val)));
#else
  return MaskBits(ShiftRightHalf(a, p), n, 0);
#endif
}

template <const int n, const int p>
void ExtrBits(const Col3& left, Col3& right);
template <const int n, const int p>
void ExtrBits(const Col3& left, Col3& right)
{
  right = ExtrBits<n, p>(left);
}

template <const int n, const int p>
void ConcBits(const Col3& left, Col3& right);
template <const int n, const int p>
void ConcBits(const Col3& left, Col3& right)
{
  right = ShiftLeft<32>(right);
  if (n > 0)
    right += ExtrBits<n, p>(left);
}

template <const int n, const int p>
void ReplBits(const Col3& left, Col3& right);
template <const int n, const int p>
void ReplBits(const Col3& left, Col3& right)
{
  if (!n)
    return;
  if ((n < 0))
  {
    right = ExtrBits<-n, p>(left);
    right.m_v = _mm_shuffle_epi32(right.m_v, SQUISH_SSE_SHUF(0, 0, 0, 3));
  }
  else
  {
    right = ExtrBits<n, p>(left);
    right.m_v = _mm_shuffle_epi32(right.m_v, SQUISH_SSE_SHUF(0, 0, 0, 0));
  }
}

Col3 Mul16x16u(const Col3& a, const Col3& b)
{
#if (SQUISH_USE_SSE >= 4)
  return Col3(_mm_mullo_epi32(a.m_v, b.m_v));
#else
  __m128i lo = _mm_mullo_epi16(a.m_v, b.m_v);
  __m128i hi = _mm_mulhi_epu16(a.m_v, b.m_v);

  return Col3(_mm_or_si128(lo, _mm_slli_si128(hi, 2)));
#endif
}

Col3 Mul16x16s(const Col3& a, const Col3& b)
{
#if (SQUISH_USE_SSE >= 4)
  return Col3(_mm_mullo_epi32(a.m_v, b.m_v));
#else
  __m128i lo = _mm_mullo_epi16(a.m_v, b.m_v);
  __m128i hi = _mm_mulhi_epi16(a.m_v, b.m_v);

  lo = _mm_and_si128(lo, _mm_set1_epi32(0x0000FFFF));
  hi = _mm_slli_epi32(hi, 16);

  return Col3(_mm_or_si128(lo, hi));
#endif
}

Col3 Div32x16u(const Col3& a, const Col3& b)
{
  return Col3((int)((unsigned int)a.GetR() / (unsigned int)b.GetR()),
              (int)((unsigned int)a.GetG() / (unsigned int)b.GetG()),
              (int)((unsigned int)a.GetB() / (unsigned int)b.GetB()));
}

Col3 Div32x16s(const Col3& a, const Col3& b)
{
  return Col3((int)((int)a.GetR() / (int)b.GetR()),
              (int)((int)a.GetG() / (int)b.GetG()),
              (int)((int)a.GetB() / (int)b.GetB()));
}

Col3 Div32x16sr(const Col3& a, const Col3& b)
{
  Col3 r = a + (b >> 1);

  return Col3((int)((int)r.GetR() / (int)b.GetR()),
              (int)((int)r.GetG() / (int)b.GetG()),
              (int)((int)r.GetB() / (int)b.GetB()));
}

//! Returns a*b + c
Col3 MultiplyAdd(const Col3& a, const Col3& b, const Col3& c)
{
#if (SQUISH_USE_SSE >= 4)
  return Col3(_mm_add_epi32(_mm_mullo_epi32(a.m_v, b.m_v), c.m_v));
#else
  return Col3(_mm_add_epi32(_mm_mullo_epi16(a.m_v, b.m_v), c.m_v));
#endif
}

//! Returns -( a*b - c )
Col3 NegativeMultiplySubtract(const Col3& a, const Col3& b, const Col3& c)
{
#if (SQUISH_USE_SSE >= 4)
  return Col3(_mm_sub_epi32(c.m_v, _mm_mullo_epi32(a.m_v, b.m_v)));
#else
  return Col3(_mm_sub_epi32(c.m_v, _mm_mullo_epi16(a.m_v, b.m_v)));
#endif
}

template <const int f, const int t>
Col3 Shuffle(const Col3& a);
template <const int f, const int t>
Col3 Shuffle(const Col3& a)
{
  if (f == t)
    return a;

  return Col3(_mm_shuffle_epi32(
    a.m_v, SQUISH_SSE_SHUF((t == 0 ? f : 0), (t == 1 ? f : 1), (t == 2 ? f : 2),
                           (t == 3 ? f : 3))));
}

template <const int f, const int t>
Col3 Exchange(const Col3& a);
template <const int f, const int t>
Col3 Exchange(const Col3& a)
{
  if (f == t)
    return a;

  return Col3(
    _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SHUF((t == 0 ? f : (f == 0 ? t : 0)),
                                             (t == 1 ? f : (f == 1 ? t : 1)),
                                             (t == 2 ? f : (f == 2 ? t : 2)),
                                             (t == 3 ? f : (f == 3 ? t : 3)))));
}

Col3 HorizontalAdd(const Col3& a)
{
#if (SQUISH_USE_SSE >= 3)
  __m128i res = a.m_v;

  res = _mm_and_si128(res, _mm_setr_epi32(~0, ~0, ~0, 0));
  res = _mm_hadd_epi32(res, res);
  res = _mm_hadd_epi32(res, res);

  return Col3(res);
#else
  __m128i res = a.m_v;

  res = _mm_and_si128(res, _mm_setr_epi32(~0, ~0, ~0, 0));
  res = _mm_add_epi32(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP64()));
  res = _mm_add_epi32(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP32()));

  return Col3(res);
#endif
}

Col3 HorizontalAdd(const Col3& a, const Col3& b)
{
#if (SQUISH_USE_SSE >= 3)
  __m128i resc;

  resc = _mm_hadd_epi32(a.m_v, b.m_v);
  resc = _mm_and_si128(resc, _mm_setr_epi32(~0, ~0, ~0, 0));
  resc = _mm_hadd_epi32(resc, resc);
  resc = _mm_hadd_epi32(resc, resc);

  return Col3(resc);
#else
  __m128i resa = a.m_v;
  __m128i resb = b.m_v;
  __m128i resc;

  resc = _mm_add_epi32(resa, resb);
  resc = _mm_and_si128(resc, _mm_setr_epi32(~0, ~0, ~0, 0));
  resc = _mm_add_epi32(resc, _mm_shuffle_epi32(resc, SQUISH_SSE_SWAP64()));
  resc = _mm_add_epi32(resc, _mm_shuffle_epi32(resc, SQUISH_SSE_SWAP32()));

  return Col3(resc);
#endif
}

Col3 HorizontalAddTiny(const Col3& a)
{
#if (SQUISH_USE_SSE >= 4) && 0
  __m128 res = _mm_castsi128_ps(a.m_v);

  res = _mm_dp_ps(res, _mm_set1_ps(1.0f), 0x77);

  return Col3(_mm_castps_si128(res));
#elif (SQUISH_USE_SSE >= 3)
  __m128 res = _mm_castsi128_ps(a.m_v);

  // relies on correct de-normal floating-point treatment
  res = _mm_and_ps(res, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0)));
  res = _mm_hadd_ps(res, res);
  res = _mm_hadd_ps(res, res);

  return Col3(_mm_castps_si128(res));
#else
  return HorizontalAdd(a);
#endif
}

Col3 HorizontalAddTiny(const Col3& a, const Col3& b)
{
#if (SQUISH_USE_SSE >= 3)
  __m128 resa = _mm_castsi128_ps(a.m_v);
  __m128 resb = _mm_castsi128_ps(b.m_v);
  __m128 resc;

  // relies on correct de-normal floating-point treatment
  resc = _mm_hadd_ps(resa, resb);
  resc = _mm_and_ps(resc, _mm_castsi128_ps(_mm_setr_epi32(~0, ~0, ~0, 0)));
  resc = _mm_hadd_ps(resc, resc);
  resc = _mm_hadd_ps(resc, resc);

  return Col3(_mm_castps_si128(resc));
#else
  return HorizontalAdd(a, b);
#endif
}

Col3 HorizontalMaxTiny(const Col3& a)
{
#if (SQUISH_USE_SSE >= 4) && 0
  __m128i inv;
  __m128i res;

  inv = _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SHUF(0, 1, 2, 2));
  inv = _mm_xor_si128(inv, _mm_set1_epi32(~0));
  inv = _mm_minpos_epu16(inv);
  res = _mm_xor_si128(inv, _mm_set1_epi32(~0));
  res = _mm_and_si128(inv, _mm_set1_epi32(0x0000FFFF));

  return Col3(res).SplatR();
#else
  __m128 res = _mm_castsi128_ps(a.m_v);

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  res = _mm_max_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(2, 0, 1, 3)));
  res = _mm_max_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 2, 0, 3)));

  return Col3(_mm_castps_si128(res));
#endif
}

Col3 HorizontalMinTiny(const Col3& a)
{
#if (SQUISH_USE_SSE >= 4)
  __m128i res;

  res = _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SHUF(0, 1, 2, 2));
  res = _mm_minpos_epu16(res);
  res = _mm_and_si128(res, _mm_set1_epi32(0x0000FFFF));

  return Col3(res).SplatR();
#else
  __m128 res = _mm_castsi128_ps(a.m_v);

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  res = _mm_min_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(2, 0, 1, 3)));
  res = _mm_min_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SHUF(1, 2, 0, 3)));

  return Col3(_mm_castps_si128(res));
#endif
}

Col3 Dot(const Col3& left, const Col3& right)
{
#if (SQUISH_USE_SSE >= 4)
  return HorizontalAdd(Col3(_mm_mullo_epi32(left.m_v, right.m_v)));
#else
  return HorizontalAdd(Col3(_mm_mullo_epi16(left.m_v, right.m_v)));
#endif
}

Col3 DotTiny(const Col3& left, const Col3& right)
{
#if (SQUISH_USE_SSE >= 4)
  return HorizontalAdd(Col3(_mm_mullo_epi32(left.m_v, right.m_v)));
#else
  return HorizontalAddTiny(Col3(_mm_mullo_epi16(left.m_v, right.m_v)));
#endif
}

Col3 Abs(const Col3& a)
{
  __m128i sign = _mm_srai_epi32(a.m_v, 31);

  return Col3(_mm_sub_epi32(_mm_xor_si128(a.m_v, sign), sign));
}

Col3 Min(const Col3& left, const Col3& right)
{
#if (SQUISH_USE_SSE >= 4)
  return Col3(_mm_min_epi32(left.m_v, right.m_v));
#else
  return Col3(_mm_min_epi16(left.m_v, right.m_v));
#endif
}

Col3 MinTiny(const Col3& left, const Col3& right)
{
  __m128 resa = _mm_castsi128_ps(left.m_v);
  __m128 resb = _mm_castsi128_ps(right.m_v);
  __m128 resc;

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  resc = _mm_min_ps(resa, resb);

  return Col3(_mm_castps_si128(resc));
}

Col3 Max(const Col3& left, const Col3& right)
{
#if (SQUISH_USE_SSE >= 4)
  return Col3(_mm_max_epi32(left.m_v, right.m_v));
#else
  return Col3(_mm_max_epi16(left.m_v, right.m_v));
#endif
}

Col3 MaxTiny(const Col3& left, const Col3& right)
{
  __m128 resa = _mm_castsi128_ps(left.m_v);
  __m128 resb = _mm_castsi128_ps(right.m_v);
  __m128 resc;

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  resc = _mm_max_ps(resa, resb);

  return Col3(_mm_castps_si128(resc));
}

bool CompareFirstLessThan(const Col3& left, const Col3& right)
{
  __m128i bits = _mm_cmplt_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return !!(value & 0x000F);
}

bool CompareFirstGreaterThan(const Col3& left, const Col3& right)
{
  __m128i bits = _mm_cmpgt_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return !!(value & 0x000F);
}

bool CompareFirstEqualTo(const Col3& left, const Col3& right)
{
  __m128i bits = _mm_cmpeq_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return !!(value & 0x000F);
}

bool CompareAnyLessThan(const Col3& left, const Col3& right)
{
  __m128i bits = _mm_cmpeq_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return (value & 0x0FFF) != 0x0000;
}

bool CompareAllEqualTo(const Col3& left, const Col3& right)
{
  __m128i bits = _mm_cmpeq_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return (value & 0x0FFF) == 0x0FFF;
}

Col3 CompareAllEqualTo_M4(const Col3& left, const Col3& right)
{
  return Col3(_mm_cmpeq_epi32(left.m_v, right.m_v));
}

int CompareGreaterThan(const Col3& left, const Col3& right)
{
  return _mm_movemask_epi8(_mm_cmpgt_epi32(left.m_v, right.m_v));
}

Col3 IsOne(const Col3& v)
{
  return Col3(_mm_cmpeq_epi32(v.m_v, _mm_set1_epi32(0x000000FF)));
}

Col3 IsZero(const Col3& v)
{
  return Col3(_mm_cmpeq_epi32(v.m_v, _mm_setzero_si128()));
}

Col3 IsNotZero(const Col3& v)
{
  return Col3(_mm_cmpgt_epi32(v.m_v, _mm_setzero_si128()));
}

void PackBytes(const Col3& a, unsigned int& loc)
{
  __m128i

    r = _mm_packs_epi32(a.m_v, a.m_v);
  r = _mm_packus_epi16(r, r);

  loc = _mm_cvtsi128_si32(r);
}

void PackBytes(const Col3& a, int& loc)
{
  __m128i

    r = _mm_packs_epi32(a.m_v, a.m_v);
  r = _mm_packs_epi16(r, r);

  loc = _mm_cvtsi128_si32(r);
}

void PackWords(const Col3& a, std::uint64_t& loc)
{
  __m128i

#if (SQUISH_USE_SSE >= 4)
    r = _mm_packus_epi32(a.m_v, a.m_v);
#else
    // fix-up/down
    r = _mm_sub_epi32(a.m_v, _mm_set1_epi32(32768));
  r = _mm_packs_epi32(r, r);
  r = _mm_add_epi16(r, _mm_set1_epi16((short)-32768));
#endif

  //    loc = _mm_cvtsi128_si64( r );
  _mm_storel_epi64((__m128i*)&loc, r);
}

void PackWords(const Col3& a, std::int64_t& loc)
{
  __m128i

    r = _mm_packs_epi32(a.m_v, a.m_v);

  //    loc = _mm_cvtsi128_si64( r );
  _mm_storel_epi64((__m128i*)&loc, r);
}

void LoadAligned(Col3& a, Col3& b, const Col3& c)
{
  a.m_v = c.m_v;
  b.m_v = _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SWAP64());
}

void LoadAligned(Col3& a, void const* source)
{
  a.m_v = _mm_load_si128((__m128i const*)source);
}

void LoadAligned(Col3& a, Col3& b, void const* source)
{
  a.m_v = _mm_load_si128((__m128i const*)source);
  b.m_v = _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SWAP64());
}

void LoadUnaligned(Col3& a, Col3& b, void const* source)
{
  a.m_v = _mm_loadu_si128((__m128i const*)source);
  b.m_v = _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SWAP64());
}

void StoreAligned(const Col3& a, const Col3& b, Col3& c)
{
  c.m_v = _mm_unpacklo_epi64(a.m_v, b.m_v);
}

void StoreAligned(const Col3& a, void* destination)
{
  _mm_store_si128((__m128i*)destination, a.m_v);
}

void StoreAligned(const Col3& a, const Col3& b, void* destination)
{
  _mm_store_si128((__m128i*)destination, _mm_unpacklo_epi64(a.m_v, b.m_v));
}

void StoreUnaligned(const Col3& a, void* destination)
{
  _mm_storeu_si128((__m128i*)destination, a.m_v);
}

void StoreUnaligned(const Col3& a, const Col3& b, void* destination)
{
  _mm_storeu_si128((__m128i*)destination, _mm_unpacklo_epi64(a.m_v, b.m_v));
}

void StoreUnaligned(const Col3& a, std::uint8_t* loc)
{
  PackBytes(a, (unsigned int&)(*((unsigned int*)loc)));
}
void StoreUnaligned(const Col3& a, std::uint16_t* loc)
{
  PackWords(a, (std::uint64_t&)(*((std::uint64_t*)loc)));
}
void StoreUnaligned(const Col3& a, std::int8_t* loc)
{
  PackBytes(a, (int&)(*((int*)loc)));
}
void StoreUnaligned(const Col3& a, std::int16_t* loc)
{
  PackWords(a, (std::int64_t&)(*((std::int64_t*)loc)));
}
}

#endif
