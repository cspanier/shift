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

#ifndef SQUISH_INLINEABLES_CPP
#define SQUISH_INLINEABLES_CPP

#include <assert.h>
#include "config.h"
#include "math.h"
#include "simd.h"

namespace squish
{

extern const std::uint16_t weights_u16[5][16];
extern const Vec4 weights_V4[5][16];
extern const Col4 weights_C4[5][16];

/* *****************************************************************************
 */
template <const bool round, const bool clamp>
static inline int FloatToInt(float a, int limit)
{
  // use ANSI round-to-zero behaviour to get round-to-nearest
  assert((a >= 0.0f) || !round);
  int i = (int)(a + (round ? 0.5f : 0.0f));

  // clamp to the limit
  if (clamp)
  {
    if (i < 0)
      i = 0;
    else if (i > limit)
      i = limit;
  }

  // done
  return i;
}

static inline int FloatTo565(const Vec3& color)
{
  // get the components in the correct range
  cQuantizer3<5, 6, 5> q = cQuantizer3<5, 6, 5>();
  Col3 rgb = q.LatticeToIntClamped(color);

  int r = rgb.R();
  int g = rgb.G();
  int b = rgb.B();

  /* not necessarily true
  assert(r == FloatToInt<true,false>(31.0f * color.X(), 31));
  assert(g == FloatToInt<true,false>(63.0f * color.Y(), 63));
  assert(b == FloatToInt<true,false>(31.0f * color.Z(), 31));
   */

  // pack into a single value
  return (r << 11) + (g << 5) + b;
}

static inline int Unpack565(std::uint8_t const* packed, std::uint8_t* color)
{
  // build the packed value
  int value = ((int)packed[0] << 0) + ((int)packed[1] << 8);

  // get the components in the stored range
  std::uint8_t red = static_cast<std::uint8_t>((value >> 11) & 0x1F);
  std::uint8_t green = static_cast<std::uint8_t>((value >> 5) & 0x3F);
  std::uint8_t blue = static_cast<std::uint8_t>(value & 0x1F);

  // scale up to 8 bits
  color[0] = static_cast<std::uint8_t>((red << 3) + (red >> 2));
  color[1] = static_cast<std::uint8_t>((green << 2) + (green >> 4));
  color[2] = static_cast<std::uint8_t>((blue << 3) + (blue >> 2));
  color[3] = 255;

  // return the value
  return value;
}

static inline int FloatTo88(const Vec3& color)
{
  // get the components in the correct range
  Col3 rgb = FloatToInt<true>(color * 255.0f);

  int r = rgb.R();
  int g = rgb.G();

  /* not necessarily true
  assert(r == FloatToInt(255.0f * color.X(), 255));
  assert(g == FloatToInt(255.0f * color.Y(), 255));
   */

  // pack into a single value
  return (r << 8) + g;
}

static inline int Unpack88(std::uint8_t const* packed, std::uint8_t* color)
{
  // build the packed value
  int value = ((int)packed[0] << 0) + ((int)packed[1] << 8);

  // get the components in the stored range
  std::uint8_t red = static_cast<std::uint8_t>((value >> 8) & 0xFF);
  std::uint8_t green = static_cast<std::uint8_t>(value & 0xFF);

  // scale up to 8 bits
  color[0] = (red);
  color[1] = (green);
  color[2] = 0;
  color[3] = 255;

  // return the value
  return value;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
#define COLORA 0
#define UNIQUE 1
#define SHARED 1  // 2
#define FIELDN 2  // 3

static const vQuantizer q7778s0(7, 7, 7, 8, 0);
static const vQuantizer q5556s0(5, 5, 5, 6, 0);
static const vQuantizer q8888s0(8, 8, 8, 8, 0);
static const vQuantizer q6666s0(6, 6, 6, 6, 0);
static const vQuantizer q8880s0(8, 8, 8, 0, 0);
static const vQuantizer q7770s0(7, 7, 7, 0, 0);
static const vQuantizer q5550s0(5, 5, 5, 0, 0);

static const vQuantizer q7778s1(7, 7, 7, 8, ~0);
static const vQuantizer q5556s1(5, 5, 5, 6, ~0);
static const vQuantizer q8888s1(8, 8, 8, 8, ~0);
static const vQuantizer q6666s1(6, 6, 6, 6, ~0);
static const vQuantizer q8880s1(8, 8, 8, 0, ~0);
static const vQuantizer q7770s1(7, 7, 7, 0, ~0);
static const vQuantizer q5550s1(5, 5, 5, 0, ~0);

#define vGetQuantizer(r, g, b, a)                              \
  (((r) == 7) && ((a) == 8)                                    \
     ? q7778s1                                                 \
     : (((r) == 5) && ((a) == 6)                               \
          ? q5556s1                                            \
          : (((r) == 5) && ((a) == 0)                          \
               ? q5550s1                                       \
               : (((r) == 8) && ((a) == 8)                     \
                    ? q8888s1                                  \
                    : (((r) == 6) && ((a) == 6)                \
                         ? q6666s1                             \
                         : (((r) == 8) && ((a) == 1)           \
                              ? q8880s1                        \
                              : (((r) == 7) && ((a) == 1)      \
                                   ? q7770s1                   \
                                   : (((r) == 5) && ((a) == 1) \
                                        ? q5550s1              \
                                        : (vQuantizer&)*(      \
                                            vQuantizer*)nullptr))))))))

#define eGetQuantizer(r, g, b, a, e)                                         \
  (((r) == 7) && ((a) == 8) && ((e) == ~0)                                   \
     ? q7778s1                                                               \
     : (((r) == 5) && ((a) == 6) && ((e) == ~0)                              \
          ? q5556s1                                                          \
          : (((r) == 5) && ((a) == 0) && ((e) == ~0)                         \
               ? q5550s1                                                     \
               : (((r) == 8) && ((a) == 8) && ((e) == 0)                     \
                    ? q8888s0                                                \
                    : (((r) == 6) && ((a) == 6) && ((e) == 0)                \
                         ? q6666s0                                           \
                         : (((r) == 8) && ((a) == 1) && ((e) == 0)           \
                              ? q8880s0                                      \
                              : (((r) == 7) && ((a) == 1) && ((e) == 0)      \
                                   ? q7770s0                                 \
                                   : (((r) == 5) && ((a) == 1) && ((e) == 0) \
                                        ? q5550s0                            \
                                        : (vQuantizer&)*(                    \
                                            vQuantizer*)nullptr))))))))

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void FloatTo(Vec4 (&color)[1], Col4 (&field)[1][FIELDN],
                           int bitset)
{
  /* not both yet */
  assert(!eb || !sb);
  assert((!eb && !sb && !(~bitset)) || ((eb || sb) && (~bitset)));

  // we can't just drop the eb/sb bits in fp-representation, we have to use the
  // exact quantizer
  const vQuantizer& q = eGetQuantizer(rb + eb + sb, gb + eb + sb, bb + eb + sb,
                                      ab + eb + sb, eb + sb ? 0 : ~0);

  // pack into a single value
  field[0][COLORA] = q.QuantizeToInt(color[0], bitset, 1 << 0);
  field[0][COLORA] = ShiftRight<eb + sb>(field[0][COLORA]);

  if (eb)
    field[0][UNIQUE] = (Col4(bitset) >> 0) & Col4(1);
  if (sb)
    field[0][SHARED] = (Col4(bitset) >> 0) & Col4(1);
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void FloatTo(Vec4 (&color)[1], Col4 (&field)[1][FIELDN])
{
  /* not both yet */
  assert(!eb || !sb);

  // const int rm = (1 << (rb + eb + sb)) - 1;
  // const int gm = (1 << (gb + eb + sb)) - 1;
  // const int bm = (1 << (bb + eb + sb)) - 1;
  // const int am = (1 << (ab + eb + sb)) - 1;
  const int em = (1 << (eb + sb)) - 1;
  const int sm = (1 << (sb)) - 1;

  const vQuantizer& q =
    vGetQuantizer(rb + eb + sb, gb + eb + sb, bb + eb + sb, ab + eb + sb);

  Col4 const umask(em);
  Col4 const smask(sm);

  Col4 rexplicit[1];
  Col4 rcomplete[1];
  Col4 runique[1];
  Col4 rshared[1];

  Col4 _e[1];
  Col4 _s[1];

  // get the components in the complete range
  rcomplete[0] = q.QuantizeToInt(color[0]);

  // get the components in the explicit range
  rexplicit[0] = ShiftRight<eb + sb>(rcomplete[0]);

  if (eb + sb)
  {
    /* stick g in a for skewing the rounding */
    if (!ab)
    {
      rcomplete[0] = Shuffle<1, 3>(rcomplete[0]);
    }
#if 0
    else {
      // TODO: this doesn't consider rotations!
      Col4 z0 = IsNotZero(rcomplete[0]).SplatA();
      Col4 o0 = IsOne    (rcomplete[0]).SplatA();

      /* if alpha is black the shared bit must be 0 */
      rcomplete[0] &= z0 | Col4(~em);

      /* if alpha is white the shared bit must be 1 */
      rcomplete[0] |= ShiftRight<32 - (eb + sb)>(o0);
    }
#endif

    if (eb)
    {
      // get the components in the unique range
      runique[0] = ShiftRight<sb>(rcomplete[0] & umask);

      _e[0] = HorizontalAddTiny(runique[0], Col4(2, 0, 0, 0)) >> 2;
    }

    if (sb)
    {
      // get the components in the shared range
      rshared[0] = (rcomplete[0] & smask);

      _s[0] = rshared[0];
    }
  }

  // pack into a single value
  field[0][COLORA] = rexplicit[0];
  if (eb)
    field[0][UNIQUE] = _e[0];
  if (sb)
    field[0][SHARED] = _s[0];
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void FloatTo(Col4 (&fielda)[1][FIELDN], Col4 (&fieldb)[1][FIELDN])
{
  /* not both yet */
  assert(!eb || !sb);

  if (eb + sb)
  {
    if (sb)
    {
      Col4 _s[1];

      _s[0] = (HorizontalAddTiny(fielda[0][SHARED], fieldb[0][SHARED]) +
               Col4(2 + 2)) >>
              3;

      // pack into a single value
      fielda[0][SHARED] = fieldb[0][SHARED] = _s[0];
    }
  }
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void FloatTo(Vec4 (&color)[2], Col4 (&field)[2][FIELDN],
                           int bitset)
{
  /* not both yet */
  assert(!eb || !sb);
  assert((!eb && !sb && !(~bitset)) || ((eb || sb) && (~bitset)));

  // we can't just drop the eb/sb bits in fp-representation, we have to use the
  // exact quantizer
  const vQuantizer& q = eGetQuantizer(rb + eb + sb, gb + eb + sb, bb + eb + sb,
                                      ab + eb + sb, eb + sb ? 0 : ~0);

  // pack into a single value
  field[0][COLORA] = q.QuantizeToInt(color[0], bitset, 1 << 0);
  field[1][COLORA] = q.QuantizeToInt(color[1], bitset, 1 << 1);
  field[0][COLORA] = ShiftRight<eb + sb>(field[0][COLORA]);
  field[1][COLORA] = ShiftRight<eb + sb>(field[1][COLORA]);

  if (eb)
    field[0][UNIQUE] = (Col4(bitset) >> 0) & Col4(1);
  if (eb)
    field[1][UNIQUE] = (Col4(bitset) >> 1) & Col4(1);
  if (sb)
    field[0][SHARED] = (Col4(bitset) >> 0) & Col4(1);
  if (sb)
    field[1][SHARED] = (Col4(bitset) >> 1) & Col4(1);
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void FloatTo(Vec4 (&color)[2], Col4 (&field)[2][FIELDN])
{
  /* not both yet */
  assert(!eb || !sb);

  // const int rm = (1 << (rb + eb + sb)) - 1;
  // const int gm = (1 << (gb + eb + sb)) - 1;
  // const int bm = (1 << (bb + eb + sb)) - 1;
  // const int am = (1 << (ab + eb + sb)) - 1;
  const int em = (1 << (eb + sb)) - 1;
  const int sm = (1 << (sb)) - 1;

  const vQuantizer& q =
    vGetQuantizer(rb + eb + sb, gb + eb + sb, bb + eb + sb, ab + eb + sb);

  Col4 const umask(em);
  Col4 const smask(sm);

  Col4 rcomplete[2];
  Col4 rexplicit[2];
  Col4 runique[2];
  Col4 rshared[2];

  Col4 _e[2];
  Col4 _s[2];

  // get the components in the complete range
  rcomplete[0] = q.QuantizeToInt(color[0]);
  rcomplete[1] = q.QuantizeToInt(color[1]);

  // get the components in the explicit range
  rexplicit[0] = ShiftRight<eb + sb>(rcomplete[0]);
  rexplicit[1] = ShiftRight<eb + sb>(rcomplete[1]);

  if (eb + sb)
  {
    /* stick g in a for skewing the rounding */
    if (!ab)
    {
      rcomplete[0] = Shuffle<1, 3>(rcomplete[0]);
      rcomplete[1] = Shuffle<1, 3>(rcomplete[1]);
    }
#if 0
    else {
      // TODO: this doesn't consider rotations!
      Col4 z0 = IsNotZero(rcomplete[0]).SplatA();
      Col4 z1 = IsNotZero(rcomplete[1]).SplatA();
      Col4 o0 = IsOne    (rcomplete[0]).SplatA();
      Col4 o1 = IsOne    (rcomplete[1]).SplatA();

      /* if alpha is black the shared bit must be 0 */
      rcomplete[0] &= z0 | Col4(~em);
      rcomplete[1] &= z1 | Col4(~em);

      /* if alpha is white the shared bit must be 1 */
      rcomplete[0] |= ShiftRight<32 - (eb + sb)>(o0);
      rcomplete[1] |= ShiftRight<32 - (eb + sb)>(o1);
    }
#endif

    if (eb)
    {
      // get the components in the unique range
      runique[0] = ShiftRight<sb>(rcomplete[0] & umask);
      runique[1] = ShiftRight<sb>(rcomplete[1] & umask);

      _e[0] = HorizontalAddTiny(runique[0], Col4(2, 0, 0, 0)) >> 2;
      _e[1] = HorizontalAddTiny(runique[1], Col4(2, 0, 0, 0)) >> 2;
    }

    if (sb)
    {
      // get the components in the shared range
      rshared[0] = (rcomplete[0] & smask);
      rshared[1] = (rcomplete[1] & smask);

      _s[0] = rshared[0];
      _s[1] = rshared[1];
    }
  }

  // pack into a single value
  field[0][COLORA] = rexplicit[0];
  if (eb)
    field[0][UNIQUE] = _e[0];
  if (sb)
    field[0][SHARED] = _s[0];
  field[1][COLORA] = rexplicit[1];
  if (eb)
    field[1][UNIQUE] = _e[1];
  if (sb)
    field[1][SHARED] = _s[1];
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void FloatTo(Col4 (&fielda)[2][FIELDN], Col4 (&fieldb)[2][FIELDN])
{
  /* not both yet */
  assert(!eb || !sb);

  if (eb + sb)
  {
    if (sb)
    {
      Col4 _s[2];

      _s[0] = (HorizontalAddTiny(fielda[0][SHARED], fieldb[0][SHARED]) +
               Col4(2 + 2)) >>
              3;
      _s[1] = (HorizontalAddTiny(fielda[1][SHARED], fieldb[1][SHARED]) +
               Col4(2 + 2)) >>
              3;

      // pack into a single value
      fielda[0][SHARED] = fieldb[0][SHARED] = _s[0];
      fielda[1][SHARED] = fieldb[1][SHARED] = _s[1];
    }
  }
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void FloatTo(Vec4 (&color)[3], Col4 (&field)[3][FIELDN],
                           int bitset)
{
  /* not both yet */
  assert(!eb || !sb);
  assert((!eb && !sb && !(~bitset)) || ((eb || sb) && (~bitset)));

  // we can't just drop the eb/sb bits in fp-representation, we have to use the
  // exact quantizer
  const vQuantizer& q = eGetQuantizer(rb + eb + sb, gb + eb + sb, bb + eb + sb,
                                      ab + eb + sb, eb + sb ? 0 : ~0);

  // pack into a single value
  field[0][COLORA] = q.QuantizeToInt(color[0], bitset, 1 << 0);
  field[1][COLORA] = q.QuantizeToInt(color[1], bitset, 1 << 1);
  field[2][COLORA] = q.QuantizeToInt(color[2], bitset, 1 << 2);
  field[0][COLORA] = ShiftRight<eb + sb>(field[0][COLORA]);
  field[1][COLORA] = ShiftRight<eb + sb>(field[1][COLORA]);
  field[2][COLORA] = ShiftRight<eb + sb>(field[2][COLORA]);

  if (eb)
    field[0][UNIQUE] = (Col4(bitset) >> 0) & Col4(1);
  if (eb)
    field[1][UNIQUE] = (Col4(bitset) >> 1) & Col4(1);
  if (eb)
    field[2][UNIQUE] = (Col4(bitset) >> 2) & Col4(1);
  if (sb)
    field[0][SHARED] = (Col4(bitset) >> 0) & Col4(1);
  if (sb)
    field[1][SHARED] = (Col4(bitset) >> 1) & Col4(1);
  if (sb)
    field[2][SHARED] = (Col4(bitset) >> 2) & Col4(1);
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void FloatTo(Vec4 (&color)[3], Col4 (&field)[3][FIELDN])
{
  /* not both yet */
  assert(!eb || !sb);

  // const int rm = (1 << (rb + eb + sb)) - 1;
  // const int gm = (1 << (gb + eb + sb)) - 1;
  // const int bm = (1 << (bb + eb + sb)) - 1;
  // const int am = (1 << (ab + eb + sb)) - 1;
  const int em = (1 << (eb + sb)) - 1;
  const int sm = (1 << (sb)) - 1;

  const vQuantizer& q =
    vGetQuantizer(rb + eb + sb, gb + eb + sb, bb + eb + sb, ab + eb + sb);

  Col4 const umask(em);
  Col4 const smask(sm);

  Col4 rcomplete[3];
  Col4 rexplicit[3];
  Col4 runique[3];
  Col4 rshared[3];

  Col4 _e[3];
  Col4 _s[3];

  // get the components in the complete range
  rcomplete[0] = q.QuantizeToInt(color[0]);
  rcomplete[1] = q.QuantizeToInt(color[1]);
  rcomplete[2] = q.QuantizeToInt(color[2]);

  // get the components in the explicit range
  rexplicit[0] = ShiftRight<eb + sb>(rcomplete[0]);
  rexplicit[1] = ShiftRight<eb + sb>(rcomplete[1]);
  rexplicit[2] = ShiftRight<eb + sb>(rcomplete[2]);

  if (eb + sb)
  {
    /* stick g in a for skewing the rounding */
    if (!ab)
    {
      rcomplete[0] = Shuffle<1, 3>(rcomplete[0]);
      rcomplete[1] = Shuffle<1, 3>(rcomplete[1]);
      rcomplete[2] = Shuffle<1, 3>(rcomplete[2]);
    }
#if 0
    else {
      // TODO: this doesn't consider rotations!
      Col4 z0 = IsNotZero(rcomplete[0]).SplatA();
      Col4 z1 = IsNotZero(rcomplete[1]).SplatA();
      Col4 z2 = IsNotZero(rcomplete[2]).SplatA();
      Col4 o0 = IsOne    (rcomplete[0]).SplatA();
      Col4 o1 = IsOne    (rcomplete[1]).SplatA();
      Col4 o2 = IsOne    (rcomplete[2]).SplatA();

      /* if alpha is black the shared bit must be 0 */
      rcomplete[0] &= z0 | Col4(~em);
      rcomplete[1] &= z1 | Col4(~em);
      rcomplete[2] &= z2 | Col4(~em);

      /* if alpha is white the shared bit must be 1 */
      rcomplete[0] |= ShiftRight<32 - (eb + sb)>(o0);
      rcomplete[1] |= ShiftRight<32 - (eb + sb)>(o1);
      rcomplete[2] |= ShiftRight<32 - (eb + sb)>(o2);
    }
#endif

    if (eb)
    {
      // get the components in the unique range
      runique[0] = ShiftRight<sb>(rcomplete[0] & umask);
      runique[1] = ShiftRight<sb>(rcomplete[1] & umask);
      runique[2] = ShiftRight<sb>(rcomplete[2] & umask);

      _e[0] = HorizontalAddTiny(runique[0], Col4(2, 0, 0, 0)) >> 2;
      _e[1] = HorizontalAddTiny(runique[1], Col4(2, 0, 0, 0)) >> 2;
      _e[2] = HorizontalAddTiny(runique[2], Col4(2, 0, 0, 0)) >> 2;
    }

    if (sb)
    {
      // get the components in the shared range
      rshared[0] = (rcomplete[0] & smask);
      rshared[1] = (rcomplete[1] & smask);
      rshared[2] = (rcomplete[2] & smask);

      _s[0] = rshared[0];
      _s[1] = rshared[1];
      _s[2] = rshared[2];
    }
  }

  // pack into a single value
  field[0][COLORA] = rexplicit[0];
  if (eb)
    field[0][UNIQUE] = _e[0];
  if (sb)
    field[0][SHARED] = _s[0];
  field[1][COLORA] = rexplicit[1];
  if (eb)
    field[1][UNIQUE] = _e[1];
  if (sb)
    field[1][SHARED] = _s[1];
  field[2][COLORA] = rexplicit[2];
  if (eb)
    field[2][UNIQUE] = _e[2];
  if (sb)
    field[2][SHARED] = _s[2];
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void FloatTo(Col4 (&fielda)[3][FIELDN], Col4 (&fieldb)[3][FIELDN])
{
  /* not both yet */
  assert(!eb || !sb);

  if (eb + sb)
  {
    if (sb)
    {
      Col4 _s[3];

      _s[0] = (HorizontalAddTiny(fielda[0][SHARED], fieldb[0][SHARED]) +
               Col4(2 + 2)) >>
              3;
      _s[1] = (HorizontalAddTiny(fielda[1][SHARED], fieldb[1][SHARED]) +
               Col4(2 + 2)) >>
              3;
      _s[2] = (HorizontalAddTiny(fielda[2][SHARED], fieldb[2][SHARED]) +
               Col4(2 + 2)) >>
              3;

      // pack into a single value
      fielda[0][SHARED] = fieldb[0][SHARED] = _s[0];
      fielda[1][SHARED] = fieldb[1][SHARED] = _s[1];
      fielda[2][SHARED] = fieldb[2][SHARED] = _s[2];
    }
  }
}

/* -----------------------------------------------------------------------------
 */
template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void UnpackFrom(Col4 (&field)[1][FIELDN])
{
  /* not both yet */
  assert(!eb || !sb);

  static const Col4 scale = Col4(
    // rb = gb = bb = 3, ab = 4, eb = 1, sb = 0 -> 4/5
    // 1 * (rb = gb = bb) + eb + sb ->  4 => (1 << 4) + 1 => 17
    // 1 * (     ab     ) + eb + sb ->  5 => (1 << 5) + 1 => 33
    // 2 * (rb = gb = bb) + eb + sb ->  8 => 16 -  8 = 8 => 17 << 8 => 4352
    // 2 * (     ab     ) + eb + sb -> 10 => 16 - 10 = 6 => 33 << 6 => 2112
    // 00000000 00001111, 00000000 00001111, 00000000 00001111, 00000000
    // 00011111 00000000 11111111, 00000000 11111111, 00000000 11111111,
    // 00000011 11111111 11111111 00000000, 11111111 00000000, 11111111
    // 00000000, 11111111 11000000

    ((1 << (rb + eb + sb)) + 1) << (16 - 2 * (rb + eb + sb)),
    ((1 << (gb + eb + sb)) + 1) << (16 - 2 * (gb + eb + sb)),
    ((1 << (bb + eb + sb)) + 1) << (16 - 2 * (bb + eb + sb)),
    ((1 << (ab + eb + sb)) + 1) << (16 - 2 * (ab + eb + sb)));

  // insert the 1 unique bit
  if (eb)
  {
    field[0][COLORA] <<= eb;

    field[0][COLORA] |= field[0][UNIQUE];
  }

  // insert the 1 shared bit
  if (sb)
  {
    field[0][COLORA] <<= sb;

    field[0][COLORA] |= field[0][SHARED];
  }

  if ((((rb + eb + sb) != 8)) || (((gb + eb + sb) != 8)) ||
      (((bb + eb + sb) != 8)) || (((ab + eb + sb) != 8) && ab))
  {
    // extend X+Y+Z bits to 8 bits
    field[0][COLORA] = (field[0][COLORA] * scale) >> 8;
  }

  // set A to opaque
  if (!ab)
  {
    field[0][COLORA] = KillA(field[0][COLORA]);
  }
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void UnpackFrom(Col4 (&field)[2][FIELDN])
{
  /* not both yet */
  assert(!eb || !sb);

  static const Col4 scale = Col4(
    // rb = gb = bb = 3, ab = 4, eb = 1, sb = 0 -> 4/5
    // 1 * (rb = gb = bb) + eb + sb ->  4 => (1 << 4) + 1 => 17
    // 1 * (     ab     ) + eb + sb ->  5 => (1 << 5) + 1 => 33
    // 2 * (rb = gb = bb) + eb + sb ->  8 => 16 -  8 = 8 => 17 << 8 => 4352
    // 2 * (     ab     ) + eb + sb -> 10 => 16 - 10 = 6 => 33 << 6 => 2112
    // 00000000 00001111, 00000000 00001111, 00000000 00001111, 00000000
    // 00011111 00000000 11111111, 00000000 11111111, 00000000 11111111,
    // 00000011 11111111 11111111 00000000, 11111111 00000000, 11111111
    // 00000000, 11111111 11000000

    ((1 << (rb + eb + sb)) + 1) << (16 - 2 * (rb + eb + sb)),
    ((1 << (gb + eb + sb)) + 1) << (16 - 2 * (gb + eb + sb)),
    ((1 << (bb + eb + sb)) + 1) << (16 - 2 * (bb + eb + sb)),
    ((1 << (ab + eb + sb)) + 1) << (16 - 2 * (ab + eb + sb)));

  // insert the 1 unique bit
  if (eb)
  {
    field[1][COLORA] <<= eb;
    field[0][COLORA] <<= eb;

    field[1][COLORA] |= field[1][UNIQUE];
    field[0][COLORA] |= field[0][UNIQUE];
  }

  // insert the 1 shared bit
  if (sb)
  {
    field[1][COLORA] <<= sb;
    field[0][COLORA] <<= sb;

    field[1][COLORA] |= field[1][SHARED];
    field[0][COLORA] |= field[0][SHARED];
  }

  if ((((rb + eb + sb) != 8)) || (((gb + eb + sb) != 8)) ||
      (((bb + eb + sb) != 8)) || (((ab + eb + sb) != 8) && ab))
  {
    // extend X+Y+Z bits to 8 bits
    field[1][COLORA] = (field[1][COLORA] * scale) >> 8;
    field[0][COLORA] = (field[0][COLORA] * scale) >> 8;
  }

  // set A to opaque
  if (!ab)
  {
    field[1][COLORA] = KillA(field[1][COLORA]);
    field[0][COLORA] = KillA(field[0][COLORA]);
  }
}

template <const int rb, const int gb, const int bb, const int ab, const int eb,
          const int sb>
static inline void UnpackFrom(Col4 (&field)[3][FIELDN])
{
  /* not both yet */
  assert(!eb || !sb);

  static const Col4 scale = Col4(
    // rb = gb = bb = 3, ab = 4, eb = 1, sb = 0 -> 4/5
    // 1 * (rb = gb = bb) + eb + sb ->  4 => (1 << 4) + 1 => 17
    // 1 * (     ab     ) + eb + sb ->  5 => (1 << 5) + 1 => 33
    // 2 * (rb = gb = bb) + eb + sb ->  8 => 16 -  8 = 8 => 17 << 8 => 4352
    // 2 * (     ab     ) + eb + sb -> 10 => 16 - 10 = 6 => 33 << 6 => 2112
    // 00000000 00001111, 00000000 00001111, 00000000 00001111, 00000000
    // 00011111 00000000 11111111, 00000000 11111111, 00000000 11111111,
    // 00000011 11111111 11111111 00000000, 11111111 00000000, 11111111
    // 00000000, 11111111 11000000

    ((1 << (rb + eb + sb)) + 1) << (16 - 2 * (rb + eb + sb)),
    ((1 << (gb + eb + sb)) + 1) << (16 - 2 * (gb + eb + sb)),
    ((1 << (bb + eb + sb)) + 1) << (16 - 2 * (bb + eb + sb)),
    ((1 << (ab + eb + sb)) + 1) << (16 - 2 * (ab + eb + sb)));

  // insert the 1 unique bit
  if (eb)
  {
    field[2][COLORA] <<= eb;
    field[1][COLORA] <<= eb;
    field[0][COLORA] <<= eb;

    field[2][COLORA] |= field[2][UNIQUE];
    field[1][COLORA] |= field[1][UNIQUE];
    field[0][COLORA] |= field[0][UNIQUE];
  }

  // insert the 1 shared bit
  if (sb)
  {
    field[2][COLORA] <<= sb;
    field[1][COLORA] <<= sb;
    field[0][COLORA] <<= sb;

    field[2][COLORA] |= field[2][SHARED];
    field[1][COLORA] |= field[1][SHARED];
    field[0][COLORA] |= field[0][SHARED];
  }

  if ((((rb + eb + sb) != 8)) || (((gb + eb + sb) != 8)) ||
      (((bb + eb + sb) != 8)) || (((ab + eb + sb) != 8) && ab))
  {
    // extend X+Y+Z bits to 8 bits
    field[2][COLORA] = (field[2][COLORA] * scale) >> 8;
    field[1][COLORA] = (field[1][COLORA] * scale) >> 8;
    field[0][COLORA] = (field[0][COLORA] * scale) >> 8;
  }

  // set A to opaque
  if (!ab)
  {
    field[2][COLORA] = KillA(field[2][COLORA]);
    field[1][COLORA] = KillA(field[1][COLORA]);
    field[0][COLORA] = KillA(field[0][COLORA]);
  }
}

/* *****************************************************************************
 * http://embeddedgurus.com/stack-overflow/2009/06/division-of-integers-by-constants/
 * Divide by 3:  (((uint32_t)A * (uint32_t)0xAAAB) >> 16) >> 1
 * Divide by 5:  (((uint32_t)A * (uint32_t)0xCCCD) >> 16) >> 2
 * Divide by 7: ((((uint32_t)A * (uint32_t)0x2493) >> 16) + A) >> 1) >> 2
 */
static inline void Codebook3or4(std::uint8_t (&codes)[4 * 4], bool bw)
{
  // generate the midpoints
  for (int i = 0; i < 3; ++i)
  {
    const int c = codes[0 + i];
    const int d = codes[4 + i];

    if (bw)
    {
      codes[8 + i] = static_cast<std::uint8_t>(((1 * c + 1 * d)) >> 1);
      codes[12 + i] = 0;
    }
    else
    {
      codes[8 + i] =
        static_cast<std::uint8_t>(((2 * c + 1 * d) * 0xAAAB) >> 17);
      codes[12 + i] =
        static_cast<std::uint8_t>(((1 * c + 2 * d) * 0xAAAB) >> 17);
    }
  }

  // fill in alpha for the intermediate values
  codes[8 + 3] = 255;
  codes[12 + 3] = bw ? 0 : 255;
}

template <const int prc>
static inline void Codebook6or8(std::uint8_t (&codes)[8 * 1], bool bw)
{
  // generate the midpoints
  for (int i = 0; i < 1; ++i)
  {
    const int c = codes[0 + i];
    const int d = codes[1 + i];
    int cd;

    if (bw)
    {
      cd = (4 * c + 1 * d);
      codes[2 + i] = static_cast<std::uint8_t>((cd * 0xCCCD) >> 18);
      cd = (3 * c + 2 * d);
      codes[3 + i] = static_cast<std::uint8_t>((cd * 0xCCCD) >> 18);
      cd = (2 * c + 3 * d);
      codes[4 + i] = static_cast<std::uint8_t>((cd * 0xCCCD) >> 18);
      cd = (1 * c + 4 * d);
      codes[5 + i] = static_cast<std::uint8_t>((cd * 0xCCCD) >> 18);

      codes[6 + i] = static_cast<std::uint8_t>(0);
      codes[7 + i] = static_cast<std::uint8_t>(255);
    }
    else
    {
      cd = (6 * c + 1 * d);
      codes[2 + i] =
        static_cast<std::uint8_t>((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (5 * c + 2 * d);
      codes[3 + i] =
        static_cast<std::uint8_t>((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (4 * c + 3 * d);
      codes[4 + i] =
        static_cast<std::uint8_t>((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (3 * c + 4 * d);
      codes[5 + i] =
        static_cast<std::uint8_t>((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (2 * c + 5 * d);
      codes[6 + i] =
        static_cast<std::uint8_t>((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (1 * c + 6 * d);
      codes[7 + i] =
        static_cast<std::uint8_t>((((cd * 0x2493) >> 16) + cd) >> 3);
    }
  }
}

template <const int prc>
static inline void Codebook6or8(std::int8_t (&codes)[8 * 1], bool bw)
{
  // generate the midpoints
  for (int i = 0; i < 1; ++i)
  {
    const int c = codes[0 + i];
    const int d = codes[1 + i];
    int cd;

    if (bw)
    {
      cd = (4 * c + 1 * d);
      codes[2 + i] = (std::int8_t)((cd * 0x3334) >> 16) + (cd < 0);
      cd = (3 * c + 2 * d);
      codes[3 + i] = (std::int8_t)((cd * 0x3334) >> 16) + (cd < 0);
      cd = (2 * c + 3 * d);
      codes[4 + i] = (std::int8_t)((cd * 0x3334) >> 16) + (cd < 0);
      cd = (1 * c + 4 * d);
      codes[5 + i] = (std::int8_t)((cd * 0x3334) >> 16) + (cd < 0);

      codes[6 + i] = (std::int8_t)-127;
      codes[7 + i] = (std::int8_t)127;

      assert(std::int8_t(codes[2]) ==
             (((std::int8_t(4) * std::int8_t(codes[0])) +
               (std::int8_t(1) * std::int8_t(codes[1]))) /
              5));
      assert(std::int8_t(codes[3]) ==
             (((std::int8_t(3) * std::int8_t(codes[0])) +
               (std::int8_t(2) * std::int8_t(codes[1]))) /
              5));
      assert(std::int8_t(codes[4]) ==
             (((std::int8_t(2) * std::int8_t(codes[0])) +
               (std::int8_t(3) * std::int8_t(codes[1]))) /
              5));
      assert(std::int8_t(codes[5]) ==
             (((std::int8_t(1) * std::int8_t(codes[0])) +
               (std::int8_t(4) * std::int8_t(codes[1]))) /
              5));
      assert(std::int8_t(codes[6]) == (-127));
      assert(std::int8_t(codes[7]) == (127));
    }
    else
    {
      cd = (6 * c + 1 * d);
      codes[2 + i] = (std::int8_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (5 * c + 2 * d);
      codes[3 + i] = (std::int8_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (4 * c + 3 * d);
      codes[4 + i] = (std::int8_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (3 * c + 4 * d);
      codes[5 + i] = (std::int8_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (2 * c + 5 * d);
      codes[6 + i] = (std::int8_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (1 * c + 6 * d);
      codes[7 + i] = (std::int8_t)((cd * 0x4925) >> 17) + (cd < 0);

      assert(std::int8_t(codes[2]) ==
             (((std::int8_t(6) * std::int8_t(codes[0])) +
               (std::int8_t(1) * std::int8_t(codes[1]))) /
              7));
      assert(std::int8_t(codes[3]) ==
             (((std::int8_t(5) * std::int8_t(codes[0])) +
               (std::int8_t(2) * std::int8_t(codes[1]))) /
              7));
      assert(std::int8_t(codes[4]) ==
             (((std::int8_t(4) * std::int8_t(codes[0])) +
               (std::int8_t(3) * std::int8_t(codes[1]))) /
              7));
      assert(std::int8_t(codes[5]) ==
             (((std::int8_t(3) * std::int8_t(codes[0])) +
               (std::int8_t(4) * std::int8_t(codes[1]))) /
              7));
      assert(std::int8_t(codes[6]) ==
             (((std::int8_t(2) * std::int8_t(codes[0])) +
               (std::int8_t(5) * std::int8_t(codes[1]))) /
              7));
      assert(std::int8_t(codes[7]) ==
             (((std::int8_t(1) * std::int8_t(codes[0])) +
               (std::int8_t(6) * std::int8_t(codes[1]))) /
              7));
    }
  }
}

template <const int prc>
static inline void Codebook6or8(std::uint16_t (&codes)[8 * 1], bool bw)
{
  // generate the midpoints
  for (int i = 0; i < 1; ++i)
  {
    const int c = codes[0 + i];
    const int d = codes[1 + i];
    int cd;

    if (bw)
    {
      cd = (4 * c + 1 * d);
      codes[2 + i] = (std::uint16_t)((cd * 0xCCCD) >> 18);
      cd = (3 * c + 2 * d);
      codes[3 + i] = (std::uint16_t)((cd * 0xCCCD) >> 18);
      cd = (2 * c + 3 * d);
      codes[4 + i] = (std::uint16_t)((cd * 0xCCCD) >> 18);
      cd = (1 * c + 4 * d);
      codes[5 + i] = (std::uint16_t)((cd * 0xCCCD) >> 18);

      codes[6 + i] = (std::uint16_t)0 << prc;
      codes[7 + i] = (std::uint16_t)255 << prc;
    }
    else
    {
      cd = (6 * c + 1 * d);
      codes[2 + i] = (std::uint16_t)((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (5 * c + 2 * d);
      codes[3 + i] = (std::uint16_t)((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (4 * c + 3 * d);
      codes[4 + i] = (std::uint16_t)((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (3 * c + 4 * d);
      codes[5 + i] = (std::uint16_t)((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (2 * c + 5 * d);
      codes[6 + i] = (std::uint16_t)((((cd * 0x2493) >> 16) + cd) >> 3);
      cd = (1 * c + 6 * d);
      codes[7 + i] = (std::uint16_t)((((cd * 0x2493) >> 16) + cd) >> 3);
    }
  }
}

template <const int prc>
static inline void Codebook6or8(std::int16_t (&codes)[8 * 1], bool bw)
{
  // generate the midpoints
  for (int i = 0; i < 1; ++i)
  {
    const int c = codes[0 + i];
    const int d = codes[1 + i];
    int cd;

    if (bw)
    {
      cd = (4 * c + 1 * d);
      codes[2 + i] = (std::int16_t)((cd * 0x3334) >> 16) + (cd < 0);
      cd = (3 * c + 2 * d);
      codes[3 + i] = (std::int16_t)((cd * 0x3334) >> 16) + (cd < 0);
      cd = (2 * c + 3 * d);
      codes[4 + i] = (std::int16_t)((cd * 0x3334) >> 16) + (cd < 0);
      cd = (1 * c + 4 * d);
      codes[5 + i] = (std::int16_t)((cd * 0x3334) >> 16) + (cd < 0);

      codes[6 + i] = (std::int16_t)-127 << prc;
      codes[7 + i] = (std::int16_t)127 << prc;

      assert(std::int16_t(codes[2]) ==
             (((std::int16_t(4) * std::int16_t(codes[0])) +
               (std::int16_t(1) * std::int16_t(codes[1]))) /
              5));
      assert(std::int16_t(codes[3]) ==
             (((std::int16_t(3) * std::int16_t(codes[0])) +
               (std::int16_t(2) * std::int16_t(codes[1]))) /
              5));
      assert(std::int16_t(codes[4]) ==
             (((std::int16_t(2) * std::int16_t(codes[0])) +
               (std::int16_t(3) * std::int16_t(codes[1]))) /
              5));
      assert(std::int16_t(codes[5]) ==
             (((std::int16_t(1) * std::int16_t(codes[0])) +
               (std::int16_t(4) * std::int16_t(codes[1]))) /
              5));
      assert(std::int16_t(codes[6]) == (-127 << prc));
      assert(std::int16_t(codes[7]) == (127 << prc));
    }
    else
    {
      cd = (6 * c + 1 * d);
      codes[2 + i] = (std::int16_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (5 * c + 2 * d);
      codes[3 + i] = (std::int16_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (4 * c + 3 * d);
      codes[4 + i] = (std::int16_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (3 * c + 4 * d);
      codes[5 + i] = (std::int16_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (2 * c + 5 * d);
      codes[6 + i] = (std::int16_t)((cd * 0x4925) >> 17) + (cd < 0);
      cd = (1 * c + 6 * d);
      codes[7 + i] = (std::int16_t)((cd * 0x4925) >> 17) + (cd < 0);

      assert(std::int16_t(codes[2]) ==
             (((std::int16_t(6) * std::int16_t(codes[0])) +
               (std::int16_t(1) * std::int16_t(codes[1]))) /
              7));
      assert(std::int16_t(codes[3]) ==
             (((std::int16_t(5) * std::int16_t(codes[0])) +
               (std::int16_t(2) * std::int16_t(codes[1]))) /
              7));
      assert(std::int16_t(codes[4]) ==
             (((std::int16_t(4) * std::int16_t(codes[0])) +
               (std::int16_t(3) * std::int16_t(codes[1]))) /
              7));
      assert(std::int16_t(codes[5]) ==
             (((std::int16_t(3) * std::int16_t(codes[0])) +
               (std::int16_t(4) * std::int16_t(codes[1]))) /
              7));
      assert(std::int16_t(codes[6]) ==
             (((std::int16_t(2) * std::int16_t(codes[0])) +
               (std::int16_t(5) * std::int16_t(codes[1]))) /
              7));
      assert(std::int16_t(codes[7]) ==
             (((std::int16_t(1) * std::int16_t(codes[0])) +
               (std::int16_t(6) * std::int16_t(codes[1]))) /
              7));
    }
  }
}

static int CodebookP(std::uint8_t* codes, int bits)
{
  // generate the midpoints
  for (int m = 0; m < 4; ++m)
  {
    const int j = (1 << bits) - 1;

    const int c = codes[0 * 4 + m];
    const int d = codes[j * 4 + m];

    // the quantizer is not equi-distant, but it is symmetric
    for (int i = 1; i < j; i++)
    {
      int s = (weights_u16[bits][j - i]) * c;
      int e = (weights_u16[bits][i + 0]) * d;

      codes[i * 4 + m] = static_cast<std::uint8_t>((s + e + 32) >> 6);
    }
  }

  return (1 << bits);
}

/* -----------------------------------------------------------------------------
 */
static inline void Codebook3(Vec3 (&codes)[3], const Vec3& start,
                             const Vec3& end)
{
  codes[0] = start;
  codes[1] = end;
  codes[2] = (0.5f * start) + (0.5f * end);
  // codes[3] = 0;
}

static inline void Codebook4(Vec3 (&codes)[4], const Vec3& start,
                             const Vec3& end)
{
  codes[0] = start;
  codes[1] = end;
  codes[2] = (2.0f / 3.0f) * start + (1.0f / 3.0f) * end;
  codes[3] = (1.0f / 3.0f) * start + (2.0f / 3.0f) * end;
}

static inline void Codebook6(Vec4 (&codes)[8], const Vec4& start,
                             const Vec4& end)
{
  codes[0] = start;
  codes[1] = end;
  codes[2] = (4.0f / 5.0f) * start + (1.0f / 5.0f) * end;
  codes[3] = (3.0f / 5.0f) * start + (2.0f / 5.0f) * end;
  codes[4] = (2.0f / 5.0f) * start + (3.0f / 5.0f) * end;
  codes[5] = (1.0f / 5.0f) * start + (4.0f / 5.0f) * end;
  codes[6] = Vec4(0.0f);
  codes[7] = Vec4(255.0f);
}

static inline void Codebook8(Vec4 (&codes)[8], const Vec4& start,
                             const Vec4& end)
{
  codes[0] = start;
  codes[1] = end;
  codes[2] = (6.0f / 7.0f) * start + (1.0f / 7.0f) * end;
  codes[3] = (5.0f / 7.0f) * start + (2.0f / 7.0f) * end;
  codes[4] = (4.0f / 7.0f) * start + (3.0f / 7.0f) * end;
  codes[5] = (3.0f / 7.0f) * start + (4.0f / 7.0f) * end;
  codes[4] = (2.0f / 7.0f) * start + (5.0f / 7.0f) * end;
  codes[5] = (1.0f / 7.0f) * start + (6.0f / 7.0f) * end;
}

/* -----------------------------------------------------------------------------
 */
#ifdef FEATURE_NORMALFIT_UNITGUARANTEE
#define DISARM true
#else
#define DISARM false
#endif

static inline void Codebook3n(Vec3 (&codes)[3], const Vec3& start,
                              const Vec3& end)
{
  const Vec3 scale = Vec3(1.0f / 0.5f);
  const Vec3 offset = Vec3(-1.0f * 0.5f);

  Codebook3(codes, start, end);

  codes[0] = Normalize(scale * (offset + codes[0]));
  codes[1] = Normalize(scale * (offset + codes[1]));
  codes[2] = Normalize(scale * (offset + codes[2]));
}

static inline void Codebook4n(Vec3 (&codes)[4], const Vec3& start,
                              const Vec3& end)
{
  const Vec3 scale = Vec3(1.0f / 0.5f);
  const Vec3 offset = Vec3(-1.0f * 0.5f);

  Codebook4(codes, start, end);

  codes[0] = Normalize(scale * (offset + codes[0]));
  codes[1] = Normalize(scale * (offset + codes[1]));
  codes[2] = Normalize(scale * (offset + codes[2]));
  codes[3] = Normalize(scale * (offset + codes[3]));
}

static inline void Codebook3nc(Vec3 (&codes)[3], const Vec3& start,
                               const Vec3& end)
{
  const Vec3 scale = Vec3(1.0f / 0.5f);
  const Vec3 offset = Vec3(-1.0f * 0.5f);

  Codebook3(codes, start, end);

  codes[0] = Complement<DISARM>(scale * (offset + codes[0]));
  codes[1] = Complement<DISARM>(scale * (offset + codes[1]));
  codes[2] = Complement<DISARM>(scale * (offset + codes[2]));
}

static inline void Codebook4nc(Vec3 (&codes)[4], const Vec3& start,
                               const Vec3& end)
{
  const Vec3 scale = Vec3(1.0f / 0.5f);
  const Vec3 offset = Vec3(-1.0f * 0.5f);

  Codebook4(codes, start, end);

  codes[0] = Complement<DISARM>(scale * (offset + codes[0]));
  codes[1] = Complement<DISARM>(scale * (offset + codes[1]));
  codes[2] = Complement<DISARM>(scale * (offset + codes[2]));
  codes[3] = Complement<DISARM>(scale * (offset + codes[3]));
}

/* -----------------------------------------------------------------------------
 */
#define CODEBOOKLQ_PRECISIONBITS (0)
#define CODEBOOKHQ_PRECISIONBITS (5)
#define CODEBOOKLQ_MULTIPLIER (1 << CODEBOOKLQ_PRECISIONBITS)
#define CODEBOOKHQ_MULTIPLIER (1 << CODEBOOKHQ_PRECISIONBITS)

template <const int min, const int max, const int pb>
static inline void Codebook6(Col8& codes, const Col8& start, const Col8& end)
{
  // max unsigned: (5 * 255) << 5 = 40800 / 0x9F60 fits unsigned short
  // max   signed: (5 * 127) << 5 = 20320 / 0x4F60 fits   signed short
  const Col8 smul = Col8(0x05 << pb, 0x00 << pb, 0x04 << pb, 0x03 << pb,
                         0x02 << pb, 0x01 << pb, 0x00 << pb, 0x00 << pb);
  const Col8 emul = Col8(0x00 << pb, 0x05 << pb, 0x01 << pb, 0x02 << pb,
                         0x03 << pb, 0x04 << pb, 0x00 << pb, 0x00 << pb);
  const Col8 mask = Col8(0x00 << pb, 0x00 << pb, 0x00 << pb, 0x00 << pb,
                         0x00 << pb, 0x00 << pb, min << pb, max << pb);

  // range [0,2*5*255]
  Col8 ipol = (smul * start) + (emul * end);

  if (min >= 0)
    // max unsigned:  0x9F60 * 0xCCCD = 0x7F801FE0 = 0x7F80    255 << 7
    codes = ((ipol * 0xCCCDU) >> 2U) + mask;
  else
    // max   signed:  0x4F60 * 0x3334 = 0x0FE03F80 = 0x0FE0    127 << 5
    codes = ((ipol * 0x6667) >> 1) + mask -
            CompareAllLessThan(ipol, Col8(0, 0, 0, 0, 0, 0, 0x8000, 0x8000));

  assert(std::int16_t(codes[0]) ==
         (((std::int16_t(smul[0]) * std::int16_t(start[0])) +
           (std::int16_t(emul[0]) * std::int16_t(end[0]))) /
            5 +
          std::int16_t(mask[0])));
  assert(std::int16_t(codes[1]) ==
         (((std::int16_t(smul[1]) * std::int16_t(start[1])) +
           (std::int16_t(emul[1]) * std::int16_t(end[1]))) /
            5 +
          std::int16_t(mask[1])));
  assert(std::int16_t(codes[2]) ==
         (((std::int16_t(smul[2]) * std::int16_t(start[2])) +
           (std::int16_t(emul[2]) * std::int16_t(end[2]))) /
            5 +
          std::int16_t(mask[2])));
  assert(std::int16_t(codes[3]) ==
         (((std::int16_t(smul[3]) * std::int16_t(start[3])) +
           (std::int16_t(emul[3]) * std::int16_t(end[3]))) /
            5 +
          std::int16_t(mask[3])));
  assert(std::int16_t(codes[4]) ==
         (((std::int16_t(smul[4]) * std::int16_t(start[4])) +
           (std::int16_t(emul[4]) * std::int16_t(end[4]))) /
            5 +
          std::int16_t(mask[4])));
  assert(std::int16_t(codes[5]) ==
         (((std::int16_t(smul[5]) * std::int16_t(start[5])) +
           (std::int16_t(emul[5]) * std::int16_t(end[5]))) /
            5 +
          std::int16_t(mask[5])));
  assert(std::int16_t(codes[6]) == std::int16_t(mask[6]));
  assert(std::int16_t(codes[7]) == std::int16_t(mask[7]));
}

template <const int min, const int max, const int pb>
static inline void Codebook8(Col8& codes, const Col8& start, const Col8& end)
{
  // max unsigned: (7 * 255) << 5 = 57120 / 0xDF20 fits unsigned short
  // max   signed: (7 * 127) << 5 = 28448 / 0x6F20 fits   signed short
  const Col8 smul = Col8(0x07 << pb, 0x00 << pb, 0x06 << pb, 0x05 << pb,
                         0x04 << pb, 0x03 << pb, 0x02 << pb, 0x01 << pb);
  const Col8 emul = Col8(0x00 << pb, 0x07 << pb, 0x01 << pb, 0x02 << pb,
                         0x03 << pb, 0x04 << pb, 0x05 << pb, 0x06 << pb);

  // range [0,2*7*255]
  Col8 ipol = (smul * start) + (emul * end);

  if (min >= 0)
    // max unsigned:  0xDF20 * 0x2493 = 0x1FE09F60 + 0xDF20 = FF00  255 << 8
    codes = (((ipol * 0x2493U) + ipol) >> 3U);
  else
    // max unsigned:  0x6F20 * 0x4925 = 0x1FC02FA0          = 1FC0  127 << 6
    codes = (((ipol * 0x4925)) >> 1) -
            CompareAllLessThan(ipol, Col8(0, 0, 0, 0, 0, 0, 0, 0));

  assert(std::int16_t(codes[0]) ==
         (((std::int16_t(smul[0]) * std::int16_t(start[0])) +
           (std::int16_t(emul[0]) * std::int16_t(end[0]))) /
          7));
  assert(std::int16_t(codes[1]) ==
         (((std::int16_t(smul[1]) * std::int16_t(start[1])) +
           (std::int16_t(emul[1]) * std::int16_t(end[1]))) /
          7));
  assert(std::int16_t(codes[2]) ==
         (((std::int16_t(smul[2]) * std::int16_t(start[2])) +
           (std::int16_t(emul[2]) * std::int16_t(end[2]))) /
          7));
  assert(std::int16_t(codes[3]) ==
         (((std::int16_t(smul[3]) * std::int16_t(start[3])) +
           (std::int16_t(emul[3]) * std::int16_t(end[3]))) /
          7));
  assert(std::int16_t(codes[4]) ==
         (((std::int16_t(smul[4]) * std::int16_t(start[4])) +
           (std::int16_t(emul[4]) * std::int16_t(end[4]))) /
          7));
  assert(std::int16_t(codes[5]) ==
         (((std::int16_t(smul[5]) * std::int16_t(start[5])) +
           (std::int16_t(emul[5]) * std::int16_t(end[5]))) /
          7));
  assert(std::int16_t(codes[6]) ==
         (((std::int16_t(smul[6]) * std::int16_t(start[6])) +
           (std::int16_t(emul[6]) * std::int16_t(end[6]))) /
          7));
  assert(std::int16_t(codes[7]) ==
         (((std::int16_t(smul[7]) * std::int16_t(start[7])) +
           (std::int16_t(emul[7]) * std::int16_t(end[7]))) /
          7));
}

/* -----------------------------------------------------------------------------
 */
static int CodebookP(Vec4* codes, int bits, const Vec4& start, const Vec4& end)
{
  const int j = (1 << bits) - 1;

  codes[0] = start;
  codes[j] = end;

  // the quantizer is not equi-distant, but it is symmetric
  for (int i = 1; i < j; i++)
  {
    const Vec4 s = weights_V4[bits][j - i] * start;
    const Vec4 e = weights_V4[bits][i + 0] * end;

    codes[i] = s + e;
  }

  return (1 << bits);
}

static int CodebookP(Col4* codes, int bits, const Col4& start, const Col4& end)
{
  const int j = (1 << bits) - 1;

  codes[0] = start;
  codes[j] = end;

  // the quantizer is not equi-distant, but it is symmetric
  for (int i = 1; i < j; i++)
  {
    const Col4 s = (weights_C4[bits][j - i]) * start;
    const Col4 e = (weights_C4[bits][i + 0]) * end;

    codes[i] = (s + e + Col4(32)) >> 6;
  }

  return (1 << bits);
}

template <const int bits>
static int CodebookP(unsigned int* codes, const Col4& start, const Col4& end)
{
  const int j = (1 << bits) - 1;

  PackBytes(start, codes[0]);
  PackBytes(end, codes[j]);

  // the quantizer is not equi-distant, but it is symmetric
  for (int i = 1; i < j; i++)
  {
    const Col4 s = (weights_C4[bits][j - i]) * start;
    const Col4 e = (weights_C4[bits][i + 0]) * end;

    PackBytes((s + e + Col4(32)) >> 6, codes[i]);
  }

  return (1 << bits);
}

static int CodebookP(Col3* codes, int bits, const Col3& start, const Col3& end)
{
  const int j = (1 << bits) - 1;

  codes[0] = start;
  codes[j] = end;

  // the quantizer is not equi-distant, but it is symmetric
  for (int i = 1; i < j; i++)
  {
    Col3 s = Mul16x16u(weights_C4[bits][j - i].GetCol3(), start);
    Col3 e = Mul16x16u(weights_C4[bits][i + 0].GetCol3(), end);

    codes[i] = (s + e + Col3(32)) >> 6;
  }

  return (1 << bits);
}

template <const int bits>
static int CodebookP(std::uint64_t* codes, const Col3& start, const Col3& end)
{
  const int j = (1 << bits) - 1;

  PackWords(Col4(start), codes[0]);
  PackWords(Col4(end), codes[j]);

  // the quantizer is not equi-distant, but it is symmetric
  for (int i = 1; i < j; i++)
  {
    Col3 s = Mul16x16u(weights_C4[bits][j - i].GetCol3(), start);
    Col3 e = Mul16x16u(weights_C4[bits][i + 0].GetCol3(), end);

    PackWords(Col4((s + e + Col3(32)) >> 6), codes[i]);
  }

  return (1 << bits);
}
/* -----------------------------------------------------------------------------
 */
static int CodebookPn(Vec4* codes, int bits, const Vec4& start, const Vec4& end)
{
  const Vec4 scale = Vec4(1.0f / 0.5f);
  const Vec4 offset = Vec4(-1.0f * 0.5f);

  CodebookP(codes, bits, start, end);

  const int j = (1 << bits) - 1;
  for (int i = 0; i <= j; i++)
    codes[i] =
      TransferW(Normalize(KillW(scale * (offset + codes[i]))), codes[i]);

  return (1 << bits);
}

/* *****************************************************************************
 */
#define DISTANCE_BASE 0.0f

template <typename dtyp>
static inline void AddDistance(dtyp const& dist, dtyp& error)
{
  error += dist;
}

template <typename dtyp>
static inline void AddDistance(dtyp const& dist, dtyp& error, dtyp const& freq)
{
  error += dist * freq;
}

template <const bool which, const int elements>
static inline void MinDistance3(Scr3& dist, int& index, Vec3& value,
                                Vec3 (&codes)[elements])
{
  Scr3 d0 = LengthSquared(value - codes[0]);
  Scr3 d1 = LengthSquared(value - codes[1]);
  Scr3 d2 = LengthSquared(value - codes[2]);

  // encourage OoO
  Scr3 da = Min(d0, d1);
  Scr3 db = (d2);
  dist = Min(da, db);

  if (which)
  {
    // will cause VS to make them all cmovs
    if (d2 == dist)
    {
      index = 2;
    }
    if (d1 == dist)
    {
      index = 1;
    }
    if (d0 == dist)
    {
      index = 0;
    }
  }
}

template <const bool which, const int elements>
static inline void MinDistance4(Scr3& dist, int& index, Vec3& value,
                                Vec3 (&codes)[elements])
{
  Scr3 d0 = LengthSquared(value - codes[0]);
  Scr3 d1 = LengthSquared(value - codes[1]);
  Scr3 d2 = LengthSquared(value - codes[2]);
  Scr3 d3 = LengthSquared(value - codes[3]);

  // encourage OoO
  Scr3 da = Min(d0, d1);
  Scr3 db = Min(d2, d3);
  dist = Min(da, db);

  if (which)
  {
    // will cause VS to make them all cmovs
    if (d3 == dist)
    {
      index = 3;
    }
    if (d2 == dist)
    {
      index = 2;
    }
    if (d1 == dist)
    {
      index = 1;
    }
    if (d0 == dist)
    {
      index = 0;
    }
  }
}

template <const bool which, const int elements>
static inline void MinDistance4(Scr4& dist, int& index, Vec4& value,
                                Vec4 (&codes)[elements], int& offset)
{
  Scr4 d0 = LengthSquared(value - codes[offset + 0]);
  Scr4 d1 = LengthSquared(value - codes[offset + 1]);
  Scr4 d2 = LengthSquared(value - codes[offset + 2]);
  Scr4 d3 = LengthSquared(value - codes[offset + 3]);

  // encourage OoO
  Scr4 da = Min(d0, d1);
  Scr4 db = Min(d2, d3);
  dist = Min(da, dist);
  dist = Min(db, dist);

  if (which)
  {
    // will cause VS to make them all cmovs
    if (d0 == dist)
    {
      index = offset;
    }
    offset++;
    if (d1 == dist)
    {
      index = offset;
    }
    offset++;
    if (d2 == dist)
    {
      index = offset;
    }
    offset++;
    if (d3 == dist)
    {
      index = offset;
    }
    offset++;
  }
}

/* -----------------------------------------------------------------------------
 */
#define DEVIANCE_SQUARE
#ifndef DEVIANCE_SQUARE
#define DEVIANCE_BASE 16.0f
#define DEVIANCE_MAX -1.0f     // largest angle
#define DEVIANCE_MAXSUM 32.0f  // 16 * [-1,+1]
#else
#define DEVIANCE_BASE 0.0f
#define DEVIANCE_MAX -1.0f     // largest angle
#define DEVIANCE_MAXSUM 64.0f  // 16 * [-2, 0]²
#endif

template <typename dtyp>
static inline void AddDeviance(dtyp const& dist, dtyp& error)
{
#ifndef DEVIANCE_SQUARE
  error -= dist;
#else
  // convert range [-1,+1] -> [-2,0]
  dtyp ang = dist + dtyp(DEVIANCE_MAX);
  dtyp sqr = ang * ang;

  error += sqr;
#endif
}

template <typename dtyp>
static inline void AddDeviance(dtyp const& dist, dtyp& error, dtyp const& freq)
{
#ifndef DEVIANCE_SQUARE
  error -= dist * freq;
#else
  // convert range [-1,+1] -> [-2,0]
  dtyp ang = dist + dtyp(DEVIANCE_MAX);
  dtyp sqr = ang * ang;

  error += sqr * freq;
#endif
}

template <const bool which, const int elements>
static inline void MinDeviance3(Scr3& dist, int& index, Vec3 const& value,
                                Vec3 const (&codes)[elements])
{
  Scr3 d0 = Dot(value, codes[0]);
  Scr3 d1 = Dot(value, codes[1]);
  Scr3 d2 = Dot(value, codes[2]);

  // encourage OoO
  Scr3 da = Max(d0, d1);
  Scr3 db = (d2);
  dist = Max(da, db);

  if (which)
  {
    // will cause VS to make them all cmovs
    if (d2 == dist)
    {
      index = 2;
    }
    if (d1 == dist)
    {
      index = 1;
    }
    if (d0 == dist)
    {
      index = 0;
    }
  }
}

template <const bool which, const int elements>
static inline void MinDeviance4(Scr3& dist, int& index, Vec3 const& value,
                                Vec3 const (&codes)[elements])
{
  Scr3 d0 = Dot(value, codes[0]);
  Scr3 d1 = Dot(value, codes[1]);
  Scr3 d2 = Dot(value, codes[2]);
  Scr3 d3 = Dot(value, codes[3]);

  // encourage OoO
  Scr3 da = Max(d0, d1);
  Scr3 db = Max(d2, d3);
  dist = Max(da, db);

  if (which)
  {
    // will cause VS to make them all cmovs
    if (d3 == dist)
    {
      index = 3;
    }
    if (d2 == dist)
    {
      index = 2;
    }
    if (d1 == dist)
    {
      index = 1;
    }
    if (d0 == dist)
    {
      index = 0;
    }
  }
}

template <const bool which, const int elements>
static inline void MinDeviance4(Scr4& dist, int& index, Vec4 const& value,
                                Vec4 const (&codes)[elements], int& offset)
{
  Scr4 d0 = Dot(value, codes[offset + 0]);
  Scr4 d1 = Dot(value, codes[offset + 1]);
  Scr4 d2 = Dot(value, codes[offset + 2]);
  Scr4 d3 = Dot(value, codes[offset + 3]);

  // encourage OoO
  Scr4 da = Max(d0, d1);
  Scr4 db = Max(d2, d3);
  dist = Max(da, dist);
  dist = Max(db, dist);

  if (which)
  {
    // will cause VS to make them all cmovs
    if (d0 == dist)
    {
      index = offset;
    }
    offset++;
    if (d1 == dist)
    {
      index = offset;
    }
    offset++;
    if (d2 == dist)
    {
      index = offset;
    }
    offset++;
    if (d3 == dist)
    {
      index = offset;
    }
    offset++;
  }
}

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
 */
template <const bool which, const int elements>
static inline void MinDeviance3c(Scr3& dist, int& index, Vec3 const& value,
                                 Vec3 const (&codes)[elements])
{
  Scr3 d0 = Dot(value, codes[0]);
  Scr3 d1 = Dot(value, codes[1]);
  Scr3 d2 = Dot(value, codes[2]);

  // select the smallest deviation (NaN as first arg is ignored!)
  dist = Max(d0, Scr3(DEVIANCE_MAX));
  dist = Max(d1, dist);
  dist = Max(d2, dist);

  if (which)
  {
    // will cause VS to make them all cmovs
    if (d2 == dist)
    {
      index = 2;
    }
    if (d1 == dist)
    {
      index = 1;
    }
    if (d0 == dist)
    {
      index = 0;
    }
  }
}

template <const bool which, const int elements>
static inline void MinDeviance4c(Scr3& dist, int& index, Vec3 const& value,
                                 Vec3 const (&codes)[elements])
{
  Scr3 d0 = Dot(value, codes[0]);
  Scr3 d1 = Dot(value, codes[1]);
  Scr3 d2 = Dot(value, codes[2]);
  Scr3 d3 = Dot(value, codes[3]);

  // select the smallest deviation (NaN as first arg is ignored!)
  dist = Max(d0, Scr3(DEVIANCE_MAX));
  dist = Max(d1, dist);
  dist = Max(d2, dist);
  dist = Max(d3, dist);

  if (which)
  {
    // will cause VS to make them all cmovs
    if (d3 == dist)
    {
      index = 3;
    }
    if (d2 == dist)
    {
      index = 2;
    }
    if (d1 == dist)
    {
      index = 1;
    }
    if (d0 == dist)
    {
      index = 0;
    }
  }
}
}

#endif
