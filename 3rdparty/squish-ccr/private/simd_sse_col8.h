/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels
  FrÃÂÃÂÃÂÃÂ¯ÃÂÃÂÃÂÃÂ¿ÃÂÃÂÃÂÃÂ½hling
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

#ifndef SQUISH_SIMD_SSE_COL8_H
#define SQUISH_SIMD_SSE_COL8_H

#include "simd_sse.h"

namespace squish
{
class Col4;
class Col8;

Col4 LoCol4(const Col8& v, const unsigned dummy);
Col4 HiCol4(const Col8& v, const unsigned dummy);
Col4 LoCol4(const Col8& v, const signed dummy);
Col4 HiCol4(const Col8& v, const signed dummy);
Col8 operator>>(const Col8& left, unsigned int right);
Col8 operator>>(const Col8& left, int right);
Col8 operator<<(const Col8& left, unsigned int right);
Col8 operator<<(const Col8& left, int right);
Col8 operator+(const Col8& left, const Col8& right);
Col8 operator-(const Col8& left, const Col8& right);
Col8 operator*(const Col8& left, const Col8& right);
Col8 operator*(const Col8& left, unsigned int right);
Col8 operator*(const Col8& left, int right);

template <const int n>
Col8 ExtendSign(const Col8& a);

Col8 HorizontalMin(const Col8& a);
Col8 HorizontalMax(const Col8& a);

template <const int n>
Col8 ShiftUp(const Col8& a);

Col4 ExpandUpper(const Col8& a, const unsigned dummy);
Col4 RepeatUpper(const Col8& a, const unsigned dummy);
Col4 InterleaveUpper(const Col8& a, const Col8& b, const unsigned dummy);
Col4 ReplicateUpper(const Col8& a, const Col8& b, const unsigned dummy);
Col4 ExpandUpper(const Col8& a, const signed dummy);
Col4 RepeatUpper(const Col8& a, const signed dummy);
Col4 InterleaveUpper(const Col8& a, const Col8& b, const signed dummy);
Col4 ReplicateUpper(const Col8& a, const Col8& b, const signed dummy);
int CompareEqualTo(const Col8& left, const Col8& right);
Col8 CompareAllEqualTo(const Col8& left, const Col8& right);
Col8 CompareAllLessThan(const Col8& left, const Col8& right);

class Col8
{
public:
  Col8()
  {
  }

  explicit Col8(__m128i v) : m_v(v)
  {
  }

  Col8(const Col8& arg) : m_v(arg.m_v)
  {
  }

  Col8& operator=(const Col8& arg)
  {
    m_v = arg.m_v;
    return *this;
  }

  explicit Col8(const Col4& s) : m_v(s.m_v)
  {
    m_v = _mm_packs_epi32(m_v, m_v);
  }

  explicit Col8(int s) : m_v(_mm_set1_epi16((short)s))
  {
  }
  explicit Col8(std::uint16_t s) : m_v(_mm_set1_epi16(s))
  {
  }
  explicit Col8(std::int16_t s) : m_v(_mm_set1_epi16(s))
  {
  }
  explicit Col8(std::uint8_t s) : m_v(_mm_set1_epi16(s))
  {
  }
  explicit Col8(std::int8_t s) : m_v(_mm_set1_epi16(s))
  {
  }

  Col8(int a, int b, int c, int d, int e, int f, int g, int h)
  : m_v(_mm_setr_epi16((short)a, (short)b, (short)c, (short)d, (short)e,
                       (short)f, (short)g, (short)h))
  {
  }
  Col8(std::uint16_t a, std::uint16_t b, std::uint16_t c, std::uint16_t d,
       std::uint16_t e, std::uint16_t f, std::uint16_t g, std::uint16_t h)
  : m_v(_mm_setr_epi16(a, b, c, d, e, f, g, h))
  {
  }
  Col8(std::int16_t a, std::int16_t b, std::int16_t c, std::int16_t d,
       std::int16_t e, std::int16_t f, std::int16_t g, std::int16_t h)
  : m_v(_mm_setr_epi16(a, b, c, d, e, f, g, h))
  {
  }
  Col8(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d,
       std::uint8_t e, std::uint8_t f, std::uint8_t g, std::uint8_t h)
  : m_v(_mm_setr_epi16(a, b, c, d, e, f, g, h))
  {
  }
  Col8(std::int8_t a, std::int8_t b, std::int8_t c, std::int8_t d,
       std::int8_t e, std::int8_t f, std::int8_t g, std::int8_t h)
  : m_v(_mm_setr_epi16(a, b, c, d, e, f, g, h))
  {
  }

  int Get0() const
  {
    return _mm_extract_epi16(m_v, 0);
  }

  const std::uint16_t& operator[](int pos) const
  {
    return ((std::uint16_t*)&m_v)[pos];
    //  return m_v.m128i_std::uint16_t[pos];
  }

  Col8& operator*=(const Col8& v)
  {
    m_v = _mm_mullo_epi16(m_v, v.m_v);
    return *this;
  }

  __m128i m_v;
};

template <const int n>
Col8 ExtendSign(const Col8& a)
{
  return Col8(_mm_srai_epi16(a.m_v, n));
}

template <const int n>
Col8 ShiftUp(const Col8& a)
{
  return Col8(_mm_slli_si128(a.m_v, n << 1));
}
}

#endif
