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

#ifndef SQUISH_SIMD_SSE_COL3_H
#define SQUISH_SIMD_SSE_COL3_H

#include "simd_sse.h"

namespace squish
{
class Col3;

int operator<(const Col3& left, const Col3& right);
int operator>(const Col3& left, const Col3& right);
int operator==(const Col3& left, const Col3& right);
Col3 operator~(const Col3& left);
Col3 operator&(const Col3& left, const Col3& right);
Col3 operator%(const Col3& left, const Col3& right);
Col3 operator^(const Col3& left, const Col3& right);
Col3 operator|(const Col3& left, const Col3& right);
Col3 operator>>(const Col3& left, int right);
Col3 operator<<(const Col3& left, int right);
Col3 operator+(const Col3& left, const Col3& right);
Col3 operator-(const Col3& left, const Col3& right);
Col3 operator*(const Col3& left, const Col3& right);
Col3 operator*(const Col3& left, int right);

template <const int n>
Col3 ShiftLeft(const Col3& a);

template <const int n>
Col3 ShiftRight(const Col3& a);

template <const int n>
Col3 ShiftRightHalf(const Col3& a);

Col3 ShiftRightHalf(const Col3& a, const int n);
Col3 ShiftRightHalf(const Col3& a, const Col3& b);

template <const int n>
Col3 ShiftLeftHalf(const Col3& a);

Col3 ShiftLeftHalf(const Col3& a, const int n);

template <const int r, const int g, const int b>
Col3 ShiftLeftLo(const Col3& v);

template <const int n, const int p>
Col3 MaskBits(const Col3& a);

Col3 MaskBits(const Col3& a, const int n, const int p);

template <const int n, const int p>
Col3 CopyBits(const Col3& left, const Col3& right);

Col3 CopyBits(const Col3& left, Col3& right, const int n, const int p);

template <const int n, const int p>
Col3 ExtrBits(const Col3& a);

Col3 ExtrBits(const Col3& a, const int n, const int p);

template <const int n, const int p>
void ExtrBits(const Col3& left, Col3& right);

template <const int n, const int p>
void ConcBits(const Col3& left, Col3& right);

template <const int n, const int p>
void ReplBits(const Col3& left, Col3& right);

Col3 Mul16x16u(const Col3& a, const Col3& b);
Col3 Mul16x16s(const Col3& a, const Col3& b);
Col3 Div32x16u(const Col3& a, const Col3& b);
Col3 Div32x16s(const Col3& a, const Col3& b);
Col3 Div32x16sr(const Col3& a, const Col3& b);
Col3 MultiplyAdd(const Col3& a, const Col3& b, const Col3& c);
Col3 NegativeMultiplySubtract(const Col3& a, const Col3& b, const Col3& c);

template <const int f, const int t>
Col3 Shuffle(const Col3& a);

template <const int f, const int t>
Col3 Exchange(const Col3& a);

Col3 HorizontalAdd(const Col3& a);
Col3 HorizontalAdd(const Col3& a, const Col3& b);
Col3 HorizontalAddTiny(const Col3& a);
Col3 HorizontalAddTiny(const Col3& a, const Col3& b);
Col3 HorizontalMaxTiny(const Col3& a);
Col3 HorizontalMinTiny(const Col3& a);
Col3 Dot(const Col3& left, const Col3& right);
Col3 DotTiny(const Col3& left, const Col3& right);
Col3 Abs(const Col3& a);
Col3 Min(const Col3& left, const Col3& right);
Col3 MinTiny(const Col3& left, const Col3& right);
Col3 Max(const Col3& left, const Col3& right);
Col3 MaxTiny(const Col3& left, const Col3& right);
bool CompareFirstLessThan(const Col3& left, const Col3& right);
bool CompareFirstGreaterThan(const Col3& left, const Col3& right);
bool CompareFirstEqualTo(const Col3& left, const Col3& right);
bool CompareAnyLessThan(const Col3& left, const Col3& right);
bool CompareAllEqualTo(const Col3& left, const Col3& right);
Col3 CompareAllEqualTo_M4(const Col3& left, const Col3& right);
int CompareGreaterThan(const Col3& left, const Col3& right);
Col3 IsOne(const Col3& v);
Col3 IsZero(const Col3& v);
Col3 IsNotZero(const Col3& v);
void PackBytes(const Col3& a, unsigned int& loc);
void PackBytes(const Col3& a, int& loc);

void PackWords(const Col3& a, std::uint64_t& loc);
void PackWords(const Col3& a, std::int64_t& loc);
void LoadAligned(Col3& a, Col3& b, const Col3& c);
void LoadAligned(Col3& a, void const* source);
void LoadAligned(Col3& a, Col3& b, void const* source);
void LoadUnaligned(Col3& a, Col3& b, void const* source);
void StoreAligned(const Col3& a, const Col3& b, Col3& c);
void StoreAligned(const Col3& a, void* destination);
void StoreAligned(const Col3& a, const Col3& b, void* destination);
void StoreUnaligned(const Col3& a, void* destination);
void StoreUnaligned(const Col3& a, const Col3& b, void* destination);
void StoreUnaligned(const Col3& a, std::uint8_t* loc);
void StoreUnaligned(const Col3& a, std::uint16_t* loc);
void StoreUnaligned(const Col3& a, std::int8_t* loc);
void StoreUnaligned(const Col3& a, std::int16_t* loc);

class Col3
{
public:
  Col3()
  {
  }

  explicit Col3(__m128i v) : m_v(v)
  {
  }

  Col3(const Col3& arg) : m_v(arg.m_v)
  {
  }

  Col3& operator=(const Col3& arg)
  {
    m_v = arg.m_v;
    return *this;
  }

  explicit Col3(int s) : m_v(_mm_set1_epi32(s))
  {
  }
  explicit Col3(float s) : m_v(_mm_cvttps_epi32(_mm_set1_ps(s)))
  {
  }

  Col3(int r, int g, int b) : m_v(_mm_setr_epi32(r, g, b, 0))
  {
  }
  Col3(int r, int g) : m_v(_mm_setr_epi32(r, g, 0, 0))
  {
  }
  Col3(std::uint16_t r, std::uint16_t g, std::uint16_t b)
  : m_v(_mm_setr_epi32(r, g, b, 0))
  {
  }
  Col3(std::int16_t r, std::int16_t g, std::int16_t b)
  : m_v(_mm_setr_epi32(r, g, b, 0))
  {
  }
  Col3(std::uint16_t r, std::uint16_t g) : m_v(_mm_setr_epi32(r, g, 0, 0))
  {
  }
  Col3(std::int16_t r, std::int16_t g) : m_v(_mm_setr_epi32(r, g, 0, 0))
  {
  }
  Col3(std::uint8_t r, std::uint8_t g, std::uint8_t b)
  : m_v(_mm_setr_epi32(r, g, b, 0))
  {
  }
  Col3(std::int8_t r, std::int8_t g, std::int8_t b)
  : m_v(_mm_setr_epi32(r, g, b, 0))
  {
  }
  Col3(std::uint8_t r, std::uint8_t g) : m_v(_mm_setr_epi32(r, g, 0, 0))
  {
  }
  Col3(std::int8_t r, std::int8_t g) : m_v(_mm_setr_epi32(r, g, 0, 0))
  {
  }

  explicit Col3(unsigned int s) : m_v(_mm_set1_epi32(s))
  {
  }
  explicit Col3(const unsigned int (&_rgb)[3])
  : m_v(_mm_load_si128((const __m128i*)&_rgb))
  {
  }
  explicit Col3(std::uint8_t const* source)
  : m_v(_mm_loadu_si128((const __m128i*)source))
  {
  }

  int GetM8() const
  {
    return _mm_movemask_epi8(m_v);
  }

  int GetLong() const
  {
    return _mm_cvtsi128_si32(m_v);
  }

  Col3 SetLong(int v) const
  {
    return Col3(_mm_cvtsi32_si128(v));
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
  // let the compiler figure this one out, probably spills to memory
  const int& GetO(int o) const
  {
    return ((const int*)&m_v)[o];
  }

  Col3 SplatR() const
  {
    return Col3(_mm_shuffle_epi32(m_v, SQUISH_SSE_SPLAT(0)));
  }
  Col3 SplatG() const
  {
    return Col3(_mm_shuffle_epi32(m_v, SQUISH_SSE_SPLAT(1)));
  }
  Col3 SplatB() const
  {
    return Col3(_mm_shuffle_epi32(m_v, SQUISH_SSE_SPLAT(2)));
  }

  template <const int inv>
  void SetRGB(int r, int g, int b)
  {
    __m128i v = _mm_setzero_si128();

    v = _mm_insert_epi16(v, r, 0);
    v = _mm_insert_epi16(v, g, 2);
    v = _mm_insert_epi16(v, b, 4);

    if (inv)
    {
      v = _mm_sub_epi32(_mm_set1_epi32(inv), v);
    }

    m_v = v;
  }

  template <const int inv>
  void SetRGBpow2(int r, int g, int b)
  {
    __m128i v = _mm_setzero_si128();

    v = _mm_insert_epi16(v, r, 0);
    v = _mm_insert_epi16(v, g, 2);
    v = _mm_insert_epi16(v, b, 4);

    if (inv)
    {
      v = _mm_sub_epi32(_mm_set1_epi32(inv), v);
    }

    v = _mm_slli_epi32(v, 23);
    v = _mm_add_epi32(v, _mm_castps_si128(_mm_set1_ps(1.0f)));

    m_v = _mm_cvttps_epi32(_mm_castsi128_ps(v));
  }

  template <const int inv>
  void SetRGBpow2(int c)
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

  Col3& operator&=(const Col3& v)
  {
    m_v = _mm_and_si128(m_v, v.m_v);
    return *this;
  }

  Col3& operator%=(const Col3& v)
  {
    m_v = _mm_andnot_si128(m_v, v.m_v);
    return *this;
  }

  Col3& operator^=(const Col3& v)
  {
    m_v = _mm_xor_si128(m_v, v.m_v);
    return *this;
  }

  Col3& operator|=(const Col3& v)
  {
    m_v = _mm_or_si128(m_v, v.m_v);
    return *this;
  }

  Col3& operator>>=(const int n)
  {
    m_v = _mm_srli_epi32(m_v, n);
    return *this;
  }

  Col3& operator<<=(const int n)
  {
    m_v = _mm_slli_epi32(m_v, n);
    return *this;
  }

  Col3& operator+=(const Col3& v)
  {
    m_v = _mm_add_epi32(m_v, v.m_v);
    return *this;
  }

  Col3& operator-=(const Col3& v)
  {
    m_v = _mm_sub_epi32(m_v, v.m_v);
    return *this;
  }

  Col3& operator*=(const Col3& v)
  {
#if (SQUISH_USE_SSE >= 4)
    m_v = _mm_mullo_epi32(m_v, v.m_v);
#else
    m_v = _mm_mullo_epi16(m_v, v.m_v);
#endif
    return *this;
  }

  Col3& operator*=(short v)
  {
#if (SQUISH_USE_SSE >= 4)
    m_v = _mm_mullo_epi32(m_v, _mm_set1_epi32(v));
#else
    m_v = _mm_mulhi_epu16(m_v, _mm_set1_epi32(v));
#endif
    return *this;
  }

  Col3& operator/=(short v)
  {
    __m128

      fp = _mm_cvtepi32_ps(m_v);
    fp = _mm_div_ps(fp, _mm_set1_ps(v));
    m_v = _mm_cvttps_epi32(fp);

    return *this;
  }

  // clamp the output to [0, 1]
  Col3 Clamp() const
  {
    Col3 const one(0xFF);
    Col3 const zero(0x00);

    return Min(one, Max(zero, *this));
  }

  __m128i m_v;

  friend class Col4;
  friend class Vec3;
};

inline Col3 LengthSquared(const Col3& v)
{
  return Dot(v, v);
}

inline Col3 LengthSquaredTiny(const Col3& v)
{
  return DotTiny(v, v);
}
}

#endif
