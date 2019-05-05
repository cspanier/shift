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
Col4 LoCol4(const Col8& v, const unsigned dummy)
{
  return Col4(_mm_unpacklo_epi16(v.m_v, _mm_setzero_si128()));
}

Col4 HiCol4(const Col8& v, const unsigned dummy)
{
  return Col4(_mm_unpackhi_epi16(v.m_v, _mm_setzero_si128()));
}

Col4 LoCol4(const Col8& v, const signed dummy)
{
  return Col4(
    _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), v.m_v), 16));
}

Col4 HiCol4(const Col8& v, const signed dummy)
{
  return Col4(
    _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), v.m_v), 16));
}

Col8 operator>>(const Col8& left, unsigned int right)
{
  return Col8(_mm_srli_epi16(left.m_v, right));
}

Col8 operator>>(const Col8& left, int right)
{
  return Col8(_mm_srai_epi16(left.m_v, right));
}

Col8 operator<<(const Col8& left, unsigned int right)
{
  return Col8(_mm_slli_epi16(left.m_v, right));
}

Col8 operator<<(const Col8& left, int right)
{
  return Col8(_mm_slli_epi16(left.m_v, right));
}

Col8 operator+(const Col8& left, const Col8& right)
{
  return Col8(_mm_add_epi16(left.m_v, right.m_v));
}

Col8 operator-(const Col8& left, const Col8& right)
{
  return Col8(_mm_sub_epi16(left.m_v, right.m_v));
}

Col8 operator*(const Col8& left, const Col8& right)
{
  return Col8(_mm_mullo_epi16(left.m_v, right.m_v));
}

Col8 operator*(const Col8& left, unsigned int right)
{
  return Col8(_mm_mulhi_epu16(left.m_v, _mm_set1_epi16((unsigned short)right)));
}

Col8 operator*(const Col8& left, int right)
{
  return Col8(_mm_mulhi_epi16(left.m_v, _mm_set1_epi16((short)right)));
}

Col8 HorizontalMin(const Col8& a)
{
  __m128i res = a.m_v;

#if (SQUISH_USE_SSE >= 4)
  res = _mm_min_epu16(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP64()));
  res = _mm_min_epu16(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP32()));
  res = _mm_min_epu16(res, _mm_shuffle_epi16(res, SQUISH_SSE_SWAP16()));
#else
  res = _mm_sub_epi16(res, _mm_set1_epi16((short)-32768));
  res = _mm_min_epi16(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP64()));
  res = _mm_min_epi16(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP32()));
  res = _mm_min_epi16(res, _mm_shuffle_epi16(res, SQUISH_SSE_SWAP16()));
  res = _mm_add_epi16(res, _mm_set1_epi16((short)-32768));
#endif

  return Col8(res);
}

Col8 HorizontalMax(const Col8& a)
{
  __m128i res = a.m_v;

#if (SQUISH_USE_SSE >= 4)
  res = _mm_max_epu16(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP64()));
  res = _mm_max_epu16(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP32()));
  res = _mm_max_epu16(res, _mm_shuffle_epi16(res, SQUISH_SSE_SWAP16()));
#else
  res = _mm_sub_epi16(res, _mm_set1_epi16((short)-32768));
  res = _mm_max_epi16(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP64()));
  res = _mm_max_epi16(res, _mm_shuffle_epi32(res, SQUISH_SSE_SWAP32()));
  res = _mm_max_epi16(res, _mm_shuffle_epi16(res, SQUISH_SSE_SWAP16()));
  res = _mm_add_epi16(res, _mm_set1_epi16((short)-32768));
#endif

  return Col8(res);
}

Col4 ExpandUpper(const Col8& a, const unsigned dummy)
{
  __m128i res = a.m_v;

  res = _mm_unpackhi_epi16(res, _mm_setzero_si128());

#ifdef _MSV_VER
  assert(res.m128i_u32[0] == a.m_v.m128i_std::uint16_t[4]);
  assert(res.m128i_u32[1] == a.m_v.m128i_std::uint16_t[5]);
  assert(res.m128i_u32[2] == a.m_v.m128i_std::uint16_t[6]);
  assert(res.m128i_u32[3] == a.m_v.m128i_std::uint16_t[7]);
#endif

  return Col4(res);
}

Col4 RepeatUpper(const Col8& a, const unsigned dummy)
{
  __m128i res = a.m_v;

  res = _mm_unpackhi_epi16(res, _mm_setzero_si128());
  res = _mm_shuffle_epi32(res, SQUISH_SSE_SPLAT(3));

#ifdef _MSV_VER
  assert(res.m128i_u32[0] == a.m_v.m128i_std::uint16_t[7]);
  assert(res.m128i_u32[1] == a.m_v.m128i_std::uint16_t[7]);
  assert(res.m128i_u32[2] == a.m_v.m128i_std::uint16_t[7]);
  assert(res.m128i_u32[3] == a.m_v.m128i_std::uint16_t[7]);
#endif

  return Col4(res);
}

Col4 InterleaveUpper(const Col8& a, const Col8& b, const unsigned dummy)
{
  __m128i res;

  res = _mm_unpackhi_epi16(a.m_v, b.m_v);
  res = _mm_unpackhi_epi16(res, _mm_setzero_si128());
  res = _mm_unpackhi_epi64(res, res);

#ifdef _MSV_VER
  assert(res.m128i_u32[0] == a.m_v.m128i_std::uint16_t[7]);
  assert(res.m128i_u32[1] == b.m_v.m128i_std::uint16_t[7]);
  assert(res.m128i_u32[2] == a.m_v.m128i_std::uint16_t[7]);
  assert(res.m128i_u32[3] == b.m_v.m128i_std::uint16_t[7]);
#endif

  return Col4(res);
}

Col4 ReplicateUpper(const Col8& a, const Col8& b, const unsigned dummy)
{
  __m128i res;

  res = _mm_unpackhi_epi16(a.m_v, b.m_v);
  res = _mm_unpackhi_epi16(res, _mm_setzero_si128());
  res = _mm_unpackhi_epi32(res, res);

#ifdef _MSV_VER
  assert(res.m128i_u32[0] == a.m_v.m128i_std::uint16_t[7]);
  assert(res.m128i_u32[1] == a.m_v.m128i_std::uint16_t[7]);
  assert(res.m128i_u32[2] == b.m_v.m128i_std::uint16_t[7]);
  assert(res.m128i_u32[3] == b.m_v.m128i_std::uint16_t[7]);
#endif

  return Col4(res);
}

Col4 ExpandUpper(const Col8& a, const signed dummy)
{
  __m128i res = a.m_v;

  res = _mm_unpackhi_epi16(res, res);
  res = _mm_srai_epi32(res, 16);

#ifdef _MSV_VER
  assert(res.m128i_i32[0] == a.m_v.m128i_i16[4]);
  assert(res.m128i_i32[1] == a.m_v.m128i_i16[5]);
  assert(res.m128i_i32[2] == a.m_v.m128i_i16[6]);
  assert(res.m128i_i32[3] == a.m_v.m128i_i16[7]);
#endif

  return Col4(res);
}

Col4 RepeatUpper(const Col8& a, const signed dummy)
{
  __m128i res = a.m_v;

  res = _mm_srai_epi32(res, 16);
  res = _mm_shuffle_epi32(res, SQUISH_SSE_SPLAT(3));

#ifdef _MSV_VER
  assert(res.m128i_i32[0] == a.m_v.m128i_i16[7]);
  assert(res.m128i_i32[1] == a.m_v.m128i_i16[7]);
  assert(res.m128i_i32[2] == a.m_v.m128i_i16[7]);
  assert(res.m128i_i32[3] == a.m_v.m128i_i16[7]);
#endif

  return Col4(res);
}

Col4 InterleaveUpper(const Col8& a, const Col8& b, const signed dummy)
{
  __m128i res;

  res = _mm_unpackhi_epi32(a.m_v, b.m_v);
  res = _mm_srai_epi32(res, 16);
  res = _mm_unpackhi_epi64(res, res);

#ifdef _MSV_VER
  assert(res.m128i_i32[0] == a.m_v.m128i_i16[7]);
  assert(res.m128i_i32[1] == b.m_v.m128i_i16[7]);
  assert(res.m128i_i32[2] == a.m_v.m128i_i16[7]);
  assert(res.m128i_i32[3] == b.m_v.m128i_i16[7]);
#endif

  return Col4(res);
}

Col4 ReplicateUpper(const Col8& a, const Col8& b, const signed dummy)
{
  __m128i res;

  res = _mm_unpackhi_epi32(a.m_v, b.m_v);
  res = _mm_srai_epi32(res, 16);
  res = _mm_unpackhi_epi32(res, res);

#ifdef _MSV_VER
  assert(res.m128i_i32[0] == a.m_v.m128i_i16[7]);
  assert(res.m128i_i32[1] == a.m_v.m128i_i16[7]);
  assert(res.m128i_i32[2] == b.m_v.m128i_i16[7]);
  assert(res.m128i_i32[3] == b.m_v.m128i_i16[7]);
#endif

  return Col4(res);
}

/*
 Col4 Expand(const Col8& a, int ia) {
  __m128i res = _mm_setzero_si128();

  res = _mm_insert_epi16( res, a.m_v.m128i_std::uint16_t[ia - 0], 0 );
  res = _mm_insert_epi16( res, a.m_v.m128i_std::uint16_t[ia - 1], 2 );
  res = _mm_insert_epi16( res, a.m_v.m128i_std::uint16_t[ia - 2], 4 );
  res = _mm_insert_epi16( res, a.m_v.m128i_std::uint16_t[ia - 3], 6 );

  return Col4( res );
}

 Col4 Repeat(const Col8& a, int ia) {
  __m128i res = _mm_setzero_si128();

  res = _mm_insert_epi16( res, a.m_v.m128i_std::uint16_t[ia], 0 );
  res = _mm_shuffle_epi32( res, SQUISH_SSE_SPLAT(0) );

  return Col4( res );
}

 Col4 Interleave(const Col8& a, const Col8& b, int ia, int ib) {
  __m128i res = _mm_setzero_si128();

  res = _mm_insert_epi16( res, a.m_v.m128i_std::uint16_t[ia], 0 );
  res = _mm_insert_epi16( res, b.m_v.m128i_std::uint16_t[ib], 2 );
  res = _mm_unpacklo_epi64( res, res );

  return Col4( res );
}

 Col4 Replicate(const Col8& a, const Col8& b, int ia, int ib) {
  __m128i res = _mm_setzero_si128();

  res = _mm_insert_epi16( res, a.m_v.m128i_std::uint16_t[ia], 0 );
  res = _mm_insert_epi16( res, b.m_v.m128i_std::uint16_t[ib], 2 );
  res = _mm_unpacklo_epi32( res, res );

  return Col4( res );
}
*/

int CompareEqualTo(const Col8& left, const Col8& right)
{
  return _mm_movemask_epi8(_mm_cmpeq_epi16(left.m_v, right.m_v));
}

Col8 CompareAllEqualTo(const Col8& left, const Col8& right)
{
  return Col8(_mm_cmpeq_epi16(left.m_v, right.m_v));
}

Col8 CompareAllLessThan(const Col8& left, const Col8& right)
{
  return Col8(_mm_cmplt_epi16(left.m_v, right.m_v));
}
}

#endif
