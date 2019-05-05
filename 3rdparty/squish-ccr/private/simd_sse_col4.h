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

#ifndef SQUISH_SIMD_SSE_COL4_H
#define SQUISH_SIMD_SSE_COL4_H

#include "simd_sse.h"
#include "simd_sse_col3.h"

namespace squish
{
class Col4;

int operator<(const Col4& left, const Col4& right);
int operator>(const Col4& left, const Col4& right);
int operator==(const Col4& left, const Col4& right);
Col4 operator~(const Col4& left);
Col4 operator&(const Col4& left, const Col4& right);
Col4 operator%(const Col4& left, const Col4& right);
Col4 operator^(const Col4& left, const Col4& right);
Col4 operator|(const Col4& left, const Col4& right);
Col4 operator>>(const Col4& left, int right);
Col4 operator<<(const Col4& left, int right);
Col4 operator+(const Col4& left, const Col4& right);
Col4 operator-(const Col4& left, const Col4& right);
Col4 operator*(const Col4& left, const Col4& right);
Col4 operator*(const Col4& left, int right);

template <const int n>
Col4 FillSign(const Col4& a);

template <const int n>
Col4 ExtendSign(const Col4& a);

template <const int n>
Col4 ShiftLeft(const Col4& a);

template <const int n>
Col4 ShiftRight(const Col4& a);

template <const int n>
Col4 ShiftRightHalf(const Col4& a);

Col4 ShiftRightHalf(const Col4& a, const int n);
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

Col4 CopyBits(const Col4& left, Col4& right, const int n, const int p);

template <const int n, const int p>
Col4 KillBits(const Col4& a);

Col4 KillBits(const Col4& a, const int n, const int p);

template <const int n, const int p>
Col4 InjtBits(const Col4& left, const Col4& right);

Col4 InjtBits(const Col4& left, Col4& right, const int n, const int p);
template <const int n, const int p>
Col4 ExtrBits(const Col4& a);

Col4 ExtrBits(const Col4& a, const int n, const int p);

template <const int n, const int p>
void ExtrBits(const Col4& left, Col4& right);

template <const int n, const int p>
void ConcBits(const Col4& left, Col4& right);

template <const int n, const int p>
void ReplBits(const Col4& left, Col4& right);

Col4 RevsBits(const Col4& v);
Col4 Mul16x16(const Col4& a, const Col4& b);
Col4 Div32x16(const Col4& a, const Col4& b);
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
Col4 HorizontalMaxTiny(const Col4& a);
Col4 HorizontalMinTiny(const Col4& a);
Col4 Dot(const Col4& left, const Col4& right);
Col4 DotTiny(const Col4& left, const Col4& right);
Col4 Abs(const Col4& a);
Col4 Min(const Col4& left, const Col4& right);
Col4 MinTiny(const Col4& left, const Col4& right);
Col4 Max(const Col4& left, const Col4& right);
Col4 MaxTiny(const Col4& left, const Col4& right);

#if 0
Col4 AbsoluteDifference( const Col4& left, const Col4& right );
Col4 SummedAbsoluteDifference( const Col4& left, const Col4& right );
Col4 MaximumAbsoluteDifference( const Col4& left, const Col4& right );
#endif

bool CompareFirstLessThan(const Col4& left, const Col4& right);
bool CompareFirstGreaterThan(const Col4& left, const Col4& right);
bool CompareFirstEqualTo(const Col4& left, const Col4& right);
bool CompareAnyLessThan(const Col4& left, const Col4& right);
bool CompareAllEqualTo(const Col4& left, const Col4& right);
Col4 CompareAllGreaterThan_M4(const Col4& left, const Col4& right);
Col4 CompareAllLessThan_M4(const Col4& left, const Col4& right);
Col4 CompareAllEqualTo_M4(const Col4& left, const Col4& right);
Col4 CompareAllLessThan_M8(const Col4& left, const Col4& right);
Col4 CompareAllEqualTo_M8(const Col4& left, const Col4& right);
Col4 IsNotZero(const Col4& v);
Col4 IsZero(const Col4& v);
Col4 IsOne(const Col4& v);

template <const int value>
Col4 IsValue(const Col4& v);

template <const int value>
Col4 IsValue(const Col4& v);

Col4 TransferA(const Col4& left, const Col4& right);
Col4 KillA(const Col4& left);
Col4 CollapseA(const Col4& r, const Col4& g, const Col4& b, const Col4& a);
void PackBytes(const Col4& a, unsigned int& loc);
void PackBytes(const Col4& a, int& loc);
void PackWords(const Col4& a, std::uint64_t& loc);
void PackWords(const Col4& a, std::int64_t& loc);
Col4 PackSShorts(const Col4& a);
Col4 PackUShorts(const Col4& a);
void UnpackBytes(Col4& a, const unsigned int& loc);
void UnpackBytes(Col4& a, const int& loc);
void UnpackWords(Col4& a, const std::uint64_t& loc);
void UnpackWords(Col4& a, const std::int64_t& loc);
void Interleave(Col4& a, const Col4& b, const Col4& c);
void LoadAligned(Col4& a, Col4& b, const Col4& c);
void LoadAligned(Col4& a, void const* source);
void LoadAligned(Col4& a, Col4& b, void const* source);
void LoadUnaligned(Col4& a, Col4& b, void const* source);
void StoreAligned(const Col4& a, const Col4& b, Col4& c);
void StoreAligned(const Col4& a, void* destination);
void StoreAligned(const Col4& a, const Col4& b, void* destination);
void StoreUnaligned(const Col4& a, void* destination);
void StoreUnaligned(const Col4& a, const Col4& b, void* destination);
void StoreUnaligned(const Col4& a, std::uint8_t* loc);
void StoreUnaligned(const Col4& a, std::uint16_t* loc);
void StoreUnaligned(const Col4& a, std::int8_t* loc);
void StoreUnaligned(const Col4& a, std::int16_t* loc);
void LoadUnaligned(Col4& a, const std::uint8_t* loc);
void LoadUnaligned(Col4& a, const std::uint16_t* loc);
void LoadUnaligned(Col4& a, const std::int8_t* loc);
void LoadUnaligned(Col4& a, const std::int16_t* loc);

class Col4
{
public:
  Col4()
  {
  }

  explicit Col4(__m128i v) : m_v(v)
  {
  }
  explicit Col4(const Col3& v) : m_v(v.m_v)
  {
  }

  Col4(const Col4& arg) : m_v(arg.m_v)
  {
  }

  Col4& operator=(const Col4& arg)
  {
    m_v = arg.m_v;
    return *this;
  }

  explicit Col4(int s) : m_v(_mm_set1_epi32(s))
  {
  }
  explicit Col4(float s) : m_v(_mm_cvttps_epi32(_mm_set1_ps(s)))
  {
  }

  Col4(int r, int g, int b, int a) : m_v(_mm_setr_epi32(r, g, b, a))
  {
  }
  Col4(int r, int g, int b) : m_v(_mm_setr_epi32(r, g, b, 0))
  {
  }
  Col4(int r, int g) : m_v(_mm_setr_epi32(r, g, 0, 0))
  {
  }
  Col4(const Col3& v, int w) : m_v(v.m_v)
  {
    ((int*)&m_v)[3] = w;
  }

  explicit Col4(unsigned int s) : m_v(_mm_set1_epi32(s))
  {
  }
  explicit Col4(const unsigned int (&_rgba)[4])
  : m_v(_mm_load_si128((const __m128i*)&_rgba))
  {
  }
  explicit Col4(std::uint8_t const* source)
  : m_v(_mm_loadu_si128((const __m128i*)source))
  {
  }

  Col3 GetCol3() const
  {
    return Col3(m_v);
  }

  int GetM8() const
  {
    return _mm_movemask_epi8(m_v);
  }

  int GetLong() const
  {
    return _mm_cvtsi128_si32(m_v);
  }

  Col4 SetLong(int v) const
  {
    return Col4(_mm_cvtsi32_si128(v));
  }

  int R() const
  {
    return _mm_extract_epi16(m_v, 0);
  }
  int G() const
  {
    return _mm_extract_epi16(m_v, 2);
  }
  int B() const
  {
    return _mm_extract_epi16(m_v, 4);
  }
  int A() const
  {
    return _mm_extract_epi16(m_v, 6);
  }

  int& GetR()
  {
    return ((int*)&m_v)[0];
  }
  int& GetG()
  {
    return ((int*)&m_v)[1];
  }
  int& GetB()
  {
    return ((int*)&m_v)[2];
  }
  int& GetA()
  {
    return ((int*)&m_v)[3];
  }
  // let the compiler figure this one out, probably spills to memory
  int& GetO(int o)
  {
    return ((int*)&m_v)[o];
  }

  const int& GetR() const
  {
    return ((const int*)&m_v)[0];
  }
  const int& GetG() const
  {
    return ((const int*)&m_v)[1];
  }
  const int& GetB() const
  {
    return ((const int*)&m_v)[2];
  }
  const int& GetA() const
  {
    return ((const int*)&m_v)[3];
  }
  // let the compiler figure this one out, probably spills to memory
  const int& GetO(int o) const
  {
    return ((const int*)&m_v)[o];
  }

  Col4 SplatR() const
  {
    return Col4(_mm_shuffle_epi32(m_v, SQUISH_SSE_SPLAT(0)));
  }
  Col4 SplatG() const
  {
    return Col4(_mm_shuffle_epi32(m_v, SQUISH_SSE_SPLAT(1)));
  }
  Col4 SplatB() const
  {
    return Col4(_mm_shuffle_epi32(m_v, SQUISH_SSE_SPLAT(2)));
  }
  Col4 SplatA() const
  {
    return Col4(_mm_shuffle_epi32(m_v, SQUISH_SSE_SPLAT(3)));
  }

  template <const int inv>
  void SetRGBA(int r, int g, int b, int a)
  {
    __m128i v = _mm_setzero_si128();

    v = _mm_insert_epi16(v, r, 0);
    v = _mm_insert_epi16(v, g, 2);
    v = _mm_insert_epi16(v, b, 4);
    v = _mm_insert_epi16(v, a, 6);

    if (inv)
    {
      v = _mm_sub_epi32(_mm_set1_epi32(inv), v);
    }

    m_v = v;
  }

  template <const int inv>
  void SetRGBApow2(int r, int g, int b, int a)
  {
    __m128i v = _mm_setzero_si128();

    v = _mm_insert_epi16(v, r, 0);
    v = _mm_insert_epi16(v, g, 2);
    v = _mm_insert_epi16(v, b, 4);
    v = _mm_insert_epi16(v, a, 6);

    if (inv)
    {
      v = _mm_sub_epi32(_mm_set1_epi32(inv), v);
    }

    v = _mm_slli_epi32(v, 23);
    v = _mm_add_epi32(v, _mm_castps_si128(_mm_set1_ps(1.0f)));

    m_v = _mm_cvttps_epi32(_mm_castsi128_ps(v));
  }

  template <const int inv>
  void SetRGBApow2(int c)
  {
    __m128i v = _mm_shuffle_epi32(_mm_cvtsi32_si128(c), SQUISH_SSE_SPLAT(0));

    if (inv)
    {
      v = _mm_sub_epi32(_mm_set1_epi32(inv), v);
    }

    v = _mm_slli_epi32(v, 23);
    v = _mm_add_epi32(v, _mm_castps_si128(_mm_set1_ps(1.0f)));

    m_v = _mm_cvttps_epi32(_mm_castsi128_ps(v));
  }

  Col4& operator&=(const Col4& v)
  {
    m_v = _mm_and_si128(m_v, v.m_v);
    return *this;
  }

  Col4& operator^=(const Col4& v)
  {
    m_v = _mm_xor_si128(m_v, v.m_v);
    return *this;
  }

  Col4& operator|=(const Col4& v)
  {
    m_v = _mm_or_si128(m_v, v.m_v);
    return *this;
  }

  Col4& operator>>=(const int n)
  {
    m_v = _mm_srli_epi32(m_v, n);
    return *this;
  }

  Col4& operator<<=(const int n)
  {
    m_v = _mm_slli_epi32(m_v, n);
    return *this;
  }

  Col4& operator+=(const Col4& v)
  {
    m_v = _mm_add_epi32(m_v, v.m_v);
    return *this;
  }

  Col4& operator-=(const Col4& v)
  {
    m_v = _mm_sub_epi32(m_v, v.m_v);
    return *this;
  }

  Col4& operator*=(const Col4& v)
  {
#if (SQUISH_USE_SSE >= 4)
    m_v = _mm_mullo_epi32(m_v, v.m_v);
#else
    m_v = _mm_mullo_epi16(m_v, v.m_v);
#endif
    return *this;
  }

  void SwapRGBA(Col4& with)
  {
    /* inplace swap based on xors */
    m_v = _mm_xor_si128(m_v, with.m_v);
    with.m_v = _mm_xor_si128(with.m_v, m_v);
    m_v = _mm_xor_si128(m_v, with.m_v);
  }

  // clamp the output to [0, 1]
  Col4 Clamp() const
  {
    Col4 const one(0xFF);
    Col4 const zero(0x00);

    return Min(one, Max(zero, *this));
  }

  __m128i m_v;
};

template <const int n>
Col4 FillSign(const Col4& a)
{
  return Col4(_mm_srai_epi32(_mm_slli_epi32(a.m_v, n), n));
}

template <const int n>
Col4 ExtendSign(const Col4& a)
{
  return Col4(_mm_srai_epi32(a.m_v, n));
}

template <const int n>
Col4 ShiftLeft(const Col4& a)
{
  if ((n) <= 0)
    return Col4(a.m_v);
  if ((n) <= 7)
    return Col4(_mm_slli_epi32(a.m_v, (n)&7));
  if ((n)&7)
    return Col4(_mm_slli_epi32(_mm_slli_si128(a.m_v, (n) >> 3), (n)&7));

  return Col4(_mm_slli_si128(a.m_v, (n) >> 3));
}

template <const int n>
Col4 ShiftRight(const Col4& a)
{
  if ((n) <= 0)
    return Col4(a.m_v);
  if ((n) <= 7)
    return Col4(_mm_srli_epi32(a.m_v, (n)&7));
  if ((n)&7)
    return Col4(_mm_srli_epi32(_mm_srli_si128(a.m_v, (n) >> 3), (n)&7));

  return Col4(_mm_srli_si128(a.m_v, (n) >> 3));
}

template <const int n>
Col4 ShiftRightHalf(const Col4& a)
{
  return Col4((n) > 0 ? _mm_srli_epi64(a.m_v, (n)) : a.m_v);
}

template <const int n>
Col4 ShiftLeftHalf(const Col4& a)
{
  return Col4((n) > 0 ? _mm_slli_epi64(a.m_v, (n)) : a.m_v);
}

template <const int r, const int g, const int b, const int a>
Col4 ShiftLeftLo(const Col4& v)
{
  // (1 << r, 1 << g, 1 << b, 1 << a);
  Col4 p2;
  p2.SetRGBApow2<0>(r, g, b, a);

#if (SQUISH_USE_SSE >= 4)
  return Col4(_mm_mullo_epi32(v.m_v, p2.m_v));
#else
  return Col4(_mm_mullo_epi16(v.m_v, p2.m_v));
#endif
}

template <const int n, const int p>
Col4 MaskBits(const Col4& a)
{
  if (((p) + (n)) <= 0)
    return Col4(0);
  if (((p) + (n)) >= 64)
    return a;

  // compile time
  std::int64_t base = ~(0xFFFFFFFFFFFFFFFFULL << (((p) + (n)) & 63));
  //  std::int64_t base =  (0xFFFFFFFFFFFFFFFFULL >> (64 - ((p) + (n)) & 63));
  __m128i mask = _mm_setr_epi32((int)(base >> 0), (int)(base >> 32), 0, 0);

  return Col4(_mm_and_si128(a.m_v, mask));
}

template <const int n, const int p>
Col4 CopyBits(const Col4& left, const Col4& right)
{
  if (!(n))
    return left;
  if (!(p))
    return MaskBits<n, 0>(right);
  if (((p) + (n)) >= 64)
    return (left) + ShiftLeftHalf<p>(right);

#if (SQUISH_USE_XSSE == 4)
  return Col4(_mm_inserti_si64(left.m_v, right.m_v, n, p));
#else
  return MaskBits<p, 0>(left) + MaskBits<n, p>(ShiftLeftHalf<p>(right));
  //  return               (left) + MaskBits<n, p>(ShiftLeftHalf<p>(right));
#endif
}

template <const int n, const int p>
Col4 KillBits(const Col4& a)
{
  if (!n || (p >= 64))
    return a;
  if (!p && (n >= 64))
    return Col4(0);

  // compile time
  std::int64_t base1 = (0xFFFFFFFFFFFFFFFFULL << ((p + 0) & 63));
  std::int64_t base2 = (0xFFFFFFFFFFFFFFFFULL >> (64 - (p + n) & 63));
  //  std::int64_t base1 = ~(0xFFFFFFFFFFFFFFFFULL >> (64 - (p + 0) & 63));
  //  std::int64_t base2 = ~(0xFFFFFFFFFFFFFFFFULL << (64 - (p + n) & 63));

  __m128i mask;

  if ((p + n) >= 64)
    base2 = 0xFFFFFFFFFFFFFFFFULL;

  mask = _mm_setr_epi32((int)((base1 ^ base2) >> 0),
                        (int)((base1 ^ base2) >> 32), 0, 0);

  return Col4(_mm_and_si128(a.m_v, mask));
}

template <const int n, const int p>
Col4 InjtBits(const Col4& left, const Col4& right)
{
  if (!n || (p >= 64))
    return right;
  if ((p + n) >= 64)
    return KillBits<n, p>(left) + ShiftLeftHalf<p>(right);
    //    return               (left) + ShiftLeftHalf<p>(right);

#if (SQUISH_USE_XSSE == 4)
  return Col4(_mm_inserti_si64(left.m_v, right.m_v, n, p));
#else
  return KillBits<n, p>(left) + MaskBits<n, p>(ShiftLeftHalf<p>(right));
  //  return               (left) + MaskBits<n, p>(ShiftLeftHalf<p>(right));
#endif
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

#if (SQUISH_USE_XSSE == 4)
  return Col4(_mm_extracti_si64(a.m_v, n, p));
#else
  return MaskBits<n, 0>(ShiftRightHalf<p>(a));
#endif
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
    right.m_v = _mm_shuffle_epi32(right.m_v, SQUISH_SSE_SHUF(0, 0, 0, 3));
  }
  else
  {
    right = ExtrBits<n, p>(left);
    right.m_v = _mm_shuffle_epi32(right.m_v, SQUISH_SSE_SHUF(0, 0, 0, 0));
  }
}

template <const int f, const int t>
Col4 Shuffle(const Col4& a)
{
  if (f == t)
    return a;

  return Col4(_mm_shuffle_epi32(
    a.m_v, SQUISH_SSE_SHUF((t == 0 ? f : 0), (t == 1 ? f : 1), (t == 2 ? f : 2),
                           (t == 3 ? f : 3))));
}

template <const int f, const int t>
Col4 Exchange(const Col4& a)
{
  if (f == t)
    return a;

  return Col4(
    _mm_shuffle_epi32(a.m_v, SQUISH_SSE_SHUF((t == 0 ? f : (f == 0 ? t : 0)),
                                             (t == 1 ? f : (f == 1 ? t : 1)),
                                             (t == 2 ? f : (f == 2 ? t : 2)),
                                             (t == 3 ? f : (f == 3 ? t : 3)))));
}

template <const int value>
Col4 IsValue(const Col4& v)
{
  return Col4(_mm_cmpeq_epi32(v.m_v, _mm_set1_epi32(value)));
}

inline Col4 LengthSquared(const Col4& v)
{
  return Dot(v, v);
}

inline Col4 LengthSquaredTiny(const Col4& v)
{
  return DotTiny(v, v);
}
}

#endif
