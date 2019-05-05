/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels
  FrÃÂÃÂ¯ÃÂÃÂ¿ÃÂÃÂ½hling
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

#include "simd.h"
#if SQUISH_USE_SSE

namespace squish
{
int operator<(const Col4& left, const Col4& right)
{
  return CompareFirstLessThan(left, right);
}

int operator>(const Col4& left, const Col4& right)
{
  return CompareFirstGreaterThan(left, right);
}

int operator==(const Col4& left, const Col4& right)
{
  return CompareFirstEqualTo(left, right);
}

Col4 operator~(const Col4& left)
{
  return left ^ Col4(~0);
}

Col4 operator&(const Col4& left, const Col4& right)
{
  return Col4(_mm_and_si128(left.m_v, right.m_v));
}

Col4 operator%(const Col4& left, const Col4& right)
{
  return Col4(_mm_andnot_si128(left.m_v, right.m_v));
}

Col4 operator^(const Col4& left, const Col4& right)
{
  return Col4(_mm_xor_si128(left.m_v, right.m_v));
}

Col4 operator|(const Col4& left, const Col4& right)
{
  return Col4(_mm_or_si128(left.m_v, right.m_v));
}

Col4 operator>>(const Col4& left, int right)
{
  return Col4(_mm_srli_epi32(left.m_v, right));
}

Col4 operator<<(const Col4& left, int right)
{
  return Col4(_mm_slli_epi32(left.m_v, right));
}

Col4 operator+(const Col4& left, const Col4& right)
{
  return Col4(_mm_add_epi32(left.m_v, right.m_v));
}

Col4 operator-(const Col4& left, const Col4& right)
{
  return Col4(_mm_sub_epi32(left.m_v, right.m_v));
}

Col4 operator*(const Col4& left, const Col4& right)
{
#if (SQUISH_USE_SSE >= 4)
  return Col4(_mm_mullo_epi32(left.m_v, right.m_v));
#else
  return Col4(_mm_mullo_epi16(left.m_v, right.m_v));
#endif
}

Col4 operator*(const Col4& left, int right)
{
#if (SQUISH_USE_SSE >= 4)
  return Col4(_mm_mullo_epi32(left.m_v, _mm_set1_epi32(right)));
#else
  return Col4(_mm_mullo_epi16(left.m_v, _mm_set1_epi32(right)));
#endif
}

Col4 ShiftRightHalf(const Col4& a, const int n)
{
  return Col4(_mm_srl_epi64(a.m_v, _mm_cvtsi32_si128(n)));
}

Col4 ShiftRightHalf(const Col4& a, const Col4& b)
{
  return Col4(_mm_srl_epi64(a.m_v, b.m_v));
}

Col4 ShiftLeftHalf(const Col4& a, const int n)
{
  return Col4(_mm_sll_epi64(a.m_v, _mm_cvtsi32_si128(n)));
}

Col4 MaskBits(const Col4& a, const int n, const int p)
{
  const int val = 64 - ((p) + (n));

  __m128i shift = _mm_max_epi16(_mm_cvtsi32_si128(val), _mm_set1_epi32(0));
  __m128i mask = _mm_setr_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0);

  mask = _mm_srl_epi64(mask, shift);

  // (0xFFFFFFFFFFFFFFFFULL >> (64 - (p + n) & 63))
  return Col4(_mm_and_si128(a.m_v, mask));
}

Col4 CopyBits(const Col4& left, Col4& right, const int n, const int p)
{
#if (SQUISH_USE_XSSE == 4)
  /* ---- ---bl xxxx xxxx */
  const int val = (p << 8) + (n << 0);

  right.m_v = _mm_unpacklo_epi64(right.m_v, _mm_cvtsi32_si128(val));
  return Col4(_mm_insert_si64(left.m_v, right.m_v));
#else
  return MaskBits(left, p, 0) + MaskBits(ShiftLeftHalf(right, p), n, p);
  //  return         (left      ) + MaskBits(ShiftLeftHalf(right, p), n, p);
#endif
}

Col4 KillBits(const Col4& a, const int n, const int p)
{
  const int val1 = (p + 0);
  const int val2 = 64 - (p + n);

  __m128i shift1 = _mm_max_epi16(_mm_cvtsi32_si128(val1), _mm_set1_epi32(0));
  __m128i shift2 = _mm_max_epi16(_mm_cvtsi32_si128(val2), _mm_set1_epi32(0));
  __m128i mask1 = _mm_setr_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  __m128i mask2 = _mm_setr_epi32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0);

  mask1 = _mm_sll_epi64(mask1, shift1);
  mask2 = _mm_srl_epi64(mask2, shift2);

  return Col4(_mm_and_si128(a.m_v, _mm_xor_si128(mask1, mask2)));
}

Col4 InjtBits(const Col4& left, Col4& right, const int n, const int p)
{
#if (SQUISH_USE_XSSE == 4)
  /* ---- ---bl xxxx xxxx */
  const int val = (p << 8) + (n << 0);

  right.m_v = _mm_unpacklo_epi64(right.m_v, _mm_cvtsi32_si128(val));
  return Col4(_mm_insert_si64(left.m_v, right.m_v));
#else
  return KillBits(left, n, p) + MaskBits(ShiftLeftHalf(right, p), n, p);
  //  return         (left      ) + MaskBits(ShiftLeftHalf(right, p), n, p);
#endif
}

Col4 ExtrBits(const Col4& a, const int n, const int p)
{
#if (SQUISH_USE_XSSE == 4)
  /* ---- ----- ---- ---bl */
  const int val = (p << 8) + (n << 0);

  return Col4(_mm_extract_si64(a.m_v, _mm_cvtsi32_si128(val)));
#else
  return MaskBits(ShiftRightHalf(a, p), n, 0);
#endif
}

Col4 RevsBits(const Col4& v)
{
  // reverse bits:
  // http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith32Bits
  // b = ((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU
  // >> 16;
  __m128i res = v.m_v;

  res = _mm_and_si128(res, _mm_set1_epi32(0x000000FF));
  //  res = _mm_unpacklo_epi64( res, res );
  res = _mm_mul_epu32(res, _mm_setr_epi32(0x00802, 0x0, 0x08020, 0x0));
  res = _mm_and_si128(res, _mm_setr_epi32(0x22110, 0x0, 0x88440, 0x0));
  res = _mm_or_si128(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP64()));
  res = _mm_mul_epu32(res, _mm_setr_epi32(0x10101, 0x0, 0x10101, 0x0));
  res = _mm_srli_epi32(res, 16);

  return Col4(res);
}

Col4 Mul16x16(const Col4& a, const Col4& b)
{
#if (SQUISH_USE_SSE >= 4)
  return Col4(_mm_mullo_epi32(a.m_v, b.m_v));
#else
  __m128i lo = _mm_mullo_epi16(a.m_v, b.m_v);
  __m128i hi = _mm_mulhi_epu16(a.m_v, b.m_v);

  return Col4(_mm_or_si128(lo, _mm_slli_si128(hi, 2)));
#endif
}

Col4 Div32x16(const Col4& a, const Col4& b)
{
  return Col4((int)a.GetR() / (int)b.GetR(), (int)a.GetG() / (int)b.GetG(),
              (int)a.GetB() / (int)b.GetB(), (int)a.GetA() / (int)b.GetA());
}

//! Returns a*b + c
Col4 MultiplyAdd(const Col4& a, const Col4& b, const Col4& c)
{
#if (SQUISH_USE_SSE >= 4)
  return Col4(_mm_add_epi32(_mm_mullo_epi32(a.m_v, b.m_v), c.m_v));
#else
  return Col4(_mm_add_epi32(_mm_mullo_epi16(a.m_v, b.m_v), c.m_v));
#endif
}

//! Returns -( a*b - c )
Col4 NegativeMultiplySubtract(const Col4& a, const Col4& b, const Col4& c)
{
#if (SQUISH_USE_SSE >= 4)
  return Col4(_mm_sub_epi32(c.m_v, _mm_mullo_epi32(a.m_v, b.m_v)));
#else
  return Col4(_mm_sub_epi32(c.m_v, _mm_mullo_epi16(a.m_v, b.m_v)));
#endif
}

Col4 HorizontalAdd(const Col4& a)
{
#if (SQUISH_USE_SSE >= 3)
  __m128i res = _mm_hadd_epi32(a.m_v, a.m_v);
  return Col4(_mm_hadd_epi32(res, res));
#else
  __m128i res = a.m_v;

  res = _mm_add_epi32(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP64()));
  res = _mm_add_epi32(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP32()));

  return Col4(res);
#endif
}

Col4 HorizontalAdd(const Col4& a, const Col4& b)
{
#if (SQUISH_USE_SSE >= 3)
  __m128i resc;

  resc = _mm_hadd_epi32(a.m_v, b.m_v);
  resc = _mm_hadd_epi32(resc, resc);
  resc = _mm_hadd_epi32(resc, resc);

  return Col4(resc);
#else
  __m128i resa = a.m_v;
  __m128i resb = b.m_v;
  __m128i resc;

  resc = _mm_add_epi32(resa, resb);
  resc = _mm_add_epi32(resc, _mm_shuffle_epi32(resc, SQUISH_SSE_SWAP64()));
  resc = _mm_add_epi32(resc, _mm_shuffle_epi32(resc, SQUISH_SSE_SWAP32()));

  return Col4(resc);
#endif
}

Col4 HorizontalAddTiny(const Col4& a)
{
#if (SQUISH_USE_SSE >= 4) && 0
  __m128 res = _mm_castsi128_ps(a.m_v);

  res = _mm_dp_ps(res, _mm_set1_ps(1.0f), 0xFF);

  return Col4(_mm_castps_si128(res));
#elif (SQUISH_USE_SSE >= 3)
  __m128 res = _mm_castsi128_ps(a.m_v);

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  res = _mm_hadd_ps(res, res);
  res = _mm_hadd_ps(res, res);

  return Col4(_mm_castps_si128(res));
#else
  return HorizontalAdd(a);
#endif
}

Col4 HorizontalAddTiny(const Col4& a, const Col4& b)
{
#if (SQUISH_USE_SSE >= 3)
  __m128 resa = _mm_castsi128_ps(a.m_v);
  __m128 resb = _mm_castsi128_ps(b.m_v);
  __m128 resc;

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  resc = _mm_hadd_ps(resa, resb);
  resc = _mm_hadd_ps(resc, resc);
  resc = _mm_hadd_ps(resc, resc);

  return Col4(_mm_castps_si128(resc));
#else
  return HorizontalAdd(a, b);
#endif
}

Col4 HorizontalMaxTiny(const Col4& a)
{
#if (SQUISH_USE_SSE >= 4)
  __m128i inv;
  __m128i res;

  inv = _mm_xor_si128(a.m_v, _mm_set1_epi32(~0));
  inv = _mm_minpos_epu16(inv);
  res = _mm_xor_si128(inv, _mm_set1_epi32(~0));
  res = _mm_and_si128(inv, _mm_set1_epi32(0x0000FFFF));

  return Col4(res).SplatR();
#else
  __m128 res = _mm_castsi128_ps(a.m_v);

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  res = _mm_max_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP64()));
  res = _mm_max_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP32()));

  return Col4(_mm_castps_si128(res));
#endif
}

Col4 HorizontalMinTiny(const Col4& a)
{
#if (SQUISH_USE_SSE >= 4)
  __m128i res;

  res = _mm_minpos_epu16(a.m_v);
  res = _mm_and_si128(res, _mm_set1_epi32(0x0000FFFF));

  return Col4(res).SplatR();
#else
  __m128 res = _mm_castsi128_ps(a.m_v);

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  res = _mm_min_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP64()));
  res = _mm_min_ps(res, _mm_shuffle_ps(res, res, SQUISH_SSE_SWAP32()));

  return Col4(_mm_castps_si128(res));
#endif
}

Col4 Dot(const Col4& left, const Col4& right)
{
#if (SQUISH_USE_SSE >= 4)
  return HorizontalAdd(Col4(_mm_mullo_epi32(left.m_v, right.m_v)));
#else
  return HorizontalAdd(Col4(_mm_mullo_epi16(left.m_v, right.m_v)));
#endif
}

Col4 DotTiny(const Col4& left, const Col4& right)
{
#if (SQUISH_USE_SSE >= 4)
  return HorizontalAddTiny(Col4(_mm_mullo_epi32(left.m_v, right.m_v)));
#else
  return HorizontalAddTiny(Col4(_mm_mullo_epi16(left.m_v, right.m_v)));
#endif
}

Col4 Abs(const Col4& a)
{
  __m128i sign = _mm_srai_epi32(a.m_v, 31);

  return Col4(_mm_sub_epi32(_mm_xor_si128(a.m_v, sign), sign));
}

Col4 Min(const Col4& left, const Col4& right)
{
#if (SQUISH_USE_SSE >= 4)
  return Col4(_mm_min_epi32(left.m_v, right.m_v));
#else
  return Col4(_mm_min_epi16(left.m_v, right.m_v));
#endif
}

Col4 MinTiny(const Col4& left, const Col4& right)
{
  __m128 resa = _mm_castsi128_ps(left.m_v);
  __m128 resb = _mm_castsi128_ps(right.m_v);
  __m128 resc;

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  resc = _mm_min_ps(resa, resb);

  return Col4(_mm_castps_si128(resc));
}

Col4 Max(const Col4& left, const Col4& right)
{
#if (SQUISH_USE_SSE >= 4)
  return Col4(_mm_max_epi32(left.m_v, right.m_v));
#else
  return Col4(_mm_max_epi16(left.m_v, right.m_v));
#endif
}

Col4 MaxTiny(const Col4& left, const Col4& right)
{
  __m128 resa = _mm_castsi128_ps(left.m_v);
  __m128 resb = _mm_castsi128_ps(right.m_v);
  __m128 resc;

  // relies on correct de-normal floating-point treatment
  // doesn't support negative values
  resc = _mm_max_ps(resa, resb);

  return Col4(_mm_castps_si128(resc));
}

#if 0
Col4 AbsoluteDifference( const Col4& left, const Col4& right )
{
  return Max( left, right ) - Min( left, right );
}

Col4 SummedAbsoluteDifference( const Col4& left, const Col4& right )
{
  return HorizontalAdd( AbsoluteDifference( left, right ) );
}

Col4 MaximumAbsoluteDifference( const Col4& left, const Col4& right )
{
  return HorizontalMax( AbsoluteDifference( left, right ) );
}
#endif

bool CompareFirstLessThan(const Col4& left, const Col4& right)
{
  __m128i bits = _mm_cmplt_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return !!(value & 0x000F);
}

bool CompareFirstGreaterThan(const Col4& left, const Col4& right)
{
  __m128i bits = _mm_cmpgt_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return !!(value & 0x000F);
}

bool CompareFirstEqualTo(const Col4& left, const Col4& right)
{
  __m128i bits = _mm_cmpeq_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return !!(value & 0x000F);
}

bool CompareAnyLessThan(const Col4& left, const Col4& right)
{
  __m128i bits = _mm_cmplt_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return value != 0x0000;
}

bool CompareAllEqualTo(const Col4& left, const Col4& right)
{
  __m128i bits = _mm_cmpeq_epi32(left.m_v, right.m_v);
  int value = _mm_movemask_epi8(bits);
  return value == 0xFFFF;
}

Col4 CompareAllGreaterThan_M4(const Col4& left, const Col4& right)
{
  return Col4(_mm_cmpgt_epi32(left.m_v, right.m_v));
}

Col4 CompareAllLessThan_M4(const Col4& left, const Col4& right)
{
  return Col4(_mm_cmplt_epi32(left.m_v, right.m_v));
}

Col4 CompareAllEqualTo_M4(const Col4& left, const Col4& right)
{
  return Col4(_mm_cmpeq_epi32(left.m_v, right.m_v));
}

Col4 CompareAllLessThan_M8(const Col4& left, const Col4& right)
{
  return Col4(_mm_cmplt_epi8(left.m_v, right.m_v));
}

Col4 CompareAllEqualTo_M8(const Col4& left, const Col4& right)
{
  return Col4(_mm_cmpeq_epi8(left.m_v, right.m_v));
}

Col4 IsNotZero(const Col4& v)
{
  return Col4(_mm_cmpgt_epi32(v.m_v, _mm_setzero_si128()));
}

Col4 IsZero(const Col4& v)
{
  return Col4(_mm_cmpeq_epi32(v.m_v, _mm_setzero_si128()));
}

Col4 IsOne(const Col4& v)
{
  return Col4(_mm_cmpeq_epi32(v.m_v, _mm_set1_epi32(0x000000FF)));
}

Col4 TransferA(const Col4& left, const Col4& right)
{
  __m128i l = _mm_and_si128(left.m_v, _mm_setr_epi32(~0, ~0, ~0, 0));
  __m128i r = _mm_and_si128(right.m_v, _mm_setr_epi32(0, 0, 0, ~0));

  return Col4(_mm_or_si128(l, r));
}

Col4 KillA(const Col4& left)
{
  return Col4(_mm_or_si128(left.m_v, _mm_setr_epi32(0x00, 0x00, 0x00, 0xFF)));
}

Col4 CollapseA(const Col4& r, const Col4& g, const Col4& b, const Col4& a)
{
  return Col4(_mm_packus_epi16(
    _mm_packs_epi32(_mm_srli_epi32(r.m_v, 24), _mm_srli_epi32(g.m_v, 24)),
    _mm_packs_epi32(_mm_srli_epi32(b.m_v, 24), _mm_srli_epi32(a.m_v, 24))));
}

void PackBytes(const Col4& a, unsigned int& loc)
{
  __m128i

    r = _mm_packs_epi32(a.m_v, a.m_v);
  r = _mm_packus_epi16(r, r);

  loc = _mm_cvtsi128_si32(r);
}

void PackBytes(const Col4& a, int& loc)
{
  __m128i

    r = _mm_packs_epi32(a.m_v, a.m_v);
  r = _mm_packs_epi16(r, r);

  loc = _mm_cvtsi128_si32(r);
}

void PackWords(const Col4& a, std::uint64_t& loc)
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

void PackWords(const Col4& a, std::int64_t& loc)
{
  __m128i

    r = _mm_packs_epi32(a.m_v, a.m_v);

  //    loc = _mm_cvtsi128_si64( r );
  _mm_storel_epi64((__m128i*)&loc, r);
}

Col4 PackSShorts(const Col4& a)
{
  return Col4(_mm_packs_epi32(a.m_v, a.m_v));
}

Col4 PackUShorts(const Col4& a)
{
#if (SQUISH_USE_SSE >= 4)
  return Col4(_mm_packus_epi32(a.m_v, a.m_v));
#else
  // fix-up/down
  __m128i r = _mm_sub_epi32(a.m_v, _mm_set1_epi32(32768));
  r = _mm_packs_epi32(r, r);
  r = _mm_add_epi16(r, _mm_set1_epi16((short)-32768));

  return Col4(r);
#endif
}

void UnpackBytes(Col4& a, const unsigned int& loc)
{
  __m128i

    r = _mm_cvtsi32_si128(loc);
  r = _mm_unpacklo_epi8(r, _mm_setzero_si128());
  r = _mm_unpacklo_epi16(r, _mm_setzero_si128());

  a = Col4(r);
}

void UnpackBytes(Col4& a, const int& loc)
{
  __m128i

    r = _mm_cvtsi32_si128(loc);
  r = _mm_unpacklo_epi8(r, r);
  r = _mm_unpacklo_epi16(r, r);

  a = ExtendSign<24>(Col4(r));
}

void UnpackWords(Col4& a, const std::uint64_t& loc)
{
  __m128i

    r = _mm_loadl_epi64((__m128i*)&loc);
  r = _mm_unpacklo_epi16(r, _mm_setzero_si128());

  a = Col4(r);
}

void UnpackWords(Col4& a, const std::int64_t& loc)
{
  __m128i

    r = _mm_loadl_epi64((__m128i*)&loc);
  r = _mm_unpacklo_epi16(r, r);

  a = ExtendSign<16>(Col4(r));
}

void Interleave(Col4& a, const Col4& b, const Col4& c)
{
  a = Col4(_mm_shuffle_epi32(_mm_unpacklo_epi32(b.m_v, c.m_v),
                             SQUISH_SSE_SHUF(0, 3, 0, 3)));
}

void LoadAligned(Col4& a, Col4& b, const Col4& c)
{
  a.m_v = c.m_v;
  b.m_v = _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SWAP64());
}

void LoadAligned(Col4& a, void const* source)
{
  a.m_v = _mm_load_si128((__m128i const*)source);
}

void LoadAligned(Col4& a, Col4& b, void const* source)
{
  a.m_v = _mm_load_si128((__m128i const*)source);
  b.m_v = _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SWAP64());
}

void LoadUnaligned(Col4& a, Col4& b, void const* source)
{
  a.m_v = _mm_loadu_si128((__m128i const*)source);
  b.m_v = _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SWAP64());
}

void StoreAligned(const Col4& a, const Col4& b, Col4& c)
{
  c.m_v = _mm_unpacklo_epi64(a.m_v, b.m_v);
}

void StoreAligned(const Col4& a, void* destination)
{
  _mm_store_si128((__m128i*)destination, a.m_v);
}

void StoreAligned(const Col4& a, const Col4& b, void* destination)
{
  _mm_store_si128((__m128i*)destination, _mm_unpacklo_epi64(a.m_v, b.m_v));
}

void StoreUnaligned(const Col4& a, void* destination)
{
  _mm_storeu_si128((__m128i*)destination, a.m_v);
}

void StoreUnaligned(const Col4& a, const Col4& b, void* destination)
{
  _mm_storeu_si128((__m128i*)destination, _mm_unpacklo_epi64(a.m_v, b.m_v));
}

void StoreUnaligned(const Col4& a, std::uint8_t* loc)
{
  PackBytes(a, (unsigned int&)(*((unsigned int*)loc)));
}
void StoreUnaligned(const Col4& a, std::uint16_t* loc)
{
  PackWords(a, (std::uint64_t&)(*((std::uint64_t*)loc)));
}
void StoreUnaligned(const Col4& a, std::int8_t* loc)
{
  PackBytes(a, (int&)(*((int*)loc)));
}
void StoreUnaligned(const Col4& a, std::int16_t* loc)
{
  PackWords(a, (std::int64_t&)(*((std::int64_t*)loc)));
}

void LoadUnaligned(Col4& a, const std::uint8_t* loc)
{
  UnpackBytes(a, (const unsigned int&)(*((const unsigned int*)loc)));
}
void LoadUnaligned(Col4& a, const std::uint16_t* loc)
{
  UnpackWords(a, (const std::uint64_t&)(*((const std::uint64_t*)loc)));
}
void LoadUnaligned(Col4& a, const std::int8_t* loc)
{
  UnpackBytes(a, (const int&)(*((const int*)loc)));
}
void LoadUnaligned(Col4& a, const std::int16_t* loc)
{
  UnpackWords(a, (const std::int64_t&)(*((const std::int64_t*)loc)));
}
}

#endif
