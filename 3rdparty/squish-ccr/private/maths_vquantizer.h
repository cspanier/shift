/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels
  FrÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ¶hling
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

#ifndef SQUISH_MATHS_VQUANTIZER_H
#define SQUISH_MATHS_VQUANTIZER_H

#include "maths.h"
#include "simd.h"

namespace squish
{
#define SMALLEST_MIDPOINT (0.5f / 255.0f)

class vQuantizer
{

public:
  /* remainders
  static const int rr = (1 << (8 - rb));
  static const int gr = (1 << (8 - gb));
  static const int br = (1 << (8 - bb));
  static const int ar = (1 << (8 - ab));

  // half remainders
  static const float rh = 0.5f * (1 << (8 - rb)) * (rm / 255.0f);
  static const float gh = 0.5f * (1 << (8 - gb)) * (gm / 255.0f);
  static const float bh = 0.5f * (1 << (8 - bb)) * (bm / 255.0f);
  static const float ah = 0.5f * (1 << (8 - ab)) * (am / 255.0f); */

  const float* qLUT_t[8];

  void ChangeShared(const int rb, const int gb, const int bb, const int ab,
                    const int sb = -1)
  {
    qLUT_t[0] = qLUT_a[rb];
    qLUT_t[1] = qLUT_a[gb];
    qLUT_t[2] = qLUT_a[bb];
    qLUT_t[3] = qLUT_a[ab];

#ifdef FEATURE_SHAREDBITS_TRIALS
    // allow bailout if whole == -1 (bit-test always succeeds)
    if ((FEATURE_SHAREDBITS_TRIALS >= SHAREDBITS_TRIAL_ALL) || (~sb))
    {
      qLUT_t[0] = qLUT_s[rb];
      qLUT_t[4] = qLUT_c[rb];
      qLUT_t[1] = qLUT_s[gb];
      qLUT_t[5] = qLUT_c[gb];
      qLUT_t[2] = qLUT_s[bb];
      qLUT_t[6] = qLUT_c[bb];
      qLUT_t[3] = qLUT_s[ab];
      qLUT_t[7] = qLUT_c[ab];
    }
#endif
  }

  Vec4 grid;
  Vec4 gridgap;
  Vec4 gridint;
  // Col4 gridinv;

  vQuantizer(const int rb, const int gb, const int bb, const int ab,
             const int sb = -1) /* :
rm(1 << rb), gm(1 << gb), bm(1 << bb), am(1 << ab),
gridrcp( 1.0f / rm, 1.0f / gm, 1.0f / bm, ab ? 1.0f / am : 1.0f ),
grid   ( 1.0f + rm, 1.0f + gm, 1.0f + bm, ab ? 1.0f + am : 1.0f )*/
  {

    /* assign the LUTs */
    ChangeShared(rb, gb, bb, ab, sb);

    //  gridinv.SetRGBApow2<0>(rb, gb, bb, ab);
    gridgap.SetXYZWpow2<8>(rb, gb, bb, ab);
    grid.SetXYZWpow2<0>(rb, gb, bb, ab);

    // set inactive channels to 255.0 (Truncate will preserve the channel)
    Vec4 tff = Vec4(255.0f) & grid.IsOne();
    Vec4 one = Vec4(1.0f);

    // merge "(FloatToInt<false>(Qf * Vec4(255.0f)) * gridinv) >> 8"
    // into "FloatToInt<false>(Qf * gridinv * Vec4(255.0f) >> 8)"
    gridint = grid;
    gridint *= Vec4(255.0f / 256.0f);
    gridint = Max(gridint, tff);

    // if a value has zero bits, to prevent
    // the singularity we set rcp to 1 as well
    grid -= one;

    // results in "x * 0 + 255", if ab is "0"
    gridgap *= grid;
    gridgap *= Vec4(0.5f / 255.0f);
    gridgap = Max(gridgap, tff);

    // silence the compiler
    bool hb = !!sb;
    hb = false;
  }

  Vec4 LookUpLattice(int r, int g, int b, int a) const
  {
    // exact nearest least-error quantization values
    return Vec4(&qLUT_t[0][r], &qLUT_t[1][g], &qLUT_t[2][b], &qLUT_t[3][a]);
  }

  Vec4 LookUpLattice(int r, int g, int b, int a, int o) const
  {
    // exact nearest least-error quantization values
    return Vec4(&qLUT_t[0 + o][r], &qLUT_t[1 + o][g], &qLUT_t[2 + o][b],
                &qLUT_t[3 + o][a]);
  }

  /* -------------------------------------------------------------------------------
   */
  Vec4 SnapToLattice(Vec4 const& val) const
  {
    Col4 p = FloatToInt<false>((grid * val.Clamp()) + gridgap);
    int r = p.R(), g = p.G(), b = p.B(), a = p.A();
    assert((r >= 0) && (r <= 0xFF) && (g >= 0) && (g <= 0xFF) && (b >= 0) &&
           (b <= 0xFF) && (a >= 0) && (a <= 0xFF));

    // exact nearest least-error quantization values
    return LookUpLattice(r, g, b, a);
  }

  Vec4 SnapToLatticeClamped(Vec4 const& val) const
  {
    Col4 p = FloatToInt<false>((grid * val) + gridgap);
    int r = p.R(), g = p.G(), b = p.B(), a = p.A();
    assert((r >= 0) && (r <= 0xFF) && (g >= 0) && (g <= 0xFF) && (b >= 0) &&
           (b <= 0xFF) && (a >= 0) && (a <= 0xFF));

    // exact nearest least-error quantization values
    return LookUpLattice(r, g, b, a);
  }

  /* -------------------------------------------------------------------------------
   */
  Col4 QuantizeToInt(Vec4 const& val) const
  {
    // [0,255]
    Vec4 Qf = SnapToLattice(val);

    // [0,1<<b-1]
    return FloatToInt<false>(Qf * gridint);
  }

  Col4 QuantizeToIntClamped(Vec4 const& val) const
  {
    // [0,255]
    Vec4 Qf = SnapToLatticeClamped(val);

    // [0,1<<b-1]
    return FloatToInt<false>(Qf * gridint);
  }

  /* -------------------------------------------------------------------------------
   * if "bitset & bittest" is a compiler constant it becomes 0 penalty
   * no-op vs. the regular version (no additional instructions)
   */
#if !defined(FEATURE_SHAREDBITS_TRIALS)
#define bitrial (bittest || bitset || 1)  // silence compiler
#else
#define bitrial (bittest & bitset)
#endif

  Vec4 SnapToLattice(Vec4 const& val, int bitset, int bittest) const
  {
    Col4 p = FloatToInt<false>((grid * val.Clamp()) + gridgap);
    int r = p.R(), g = p.G(), b = p.B(), a = p.A();
    assert((r >= 0) && (r <= 0xFF) && (g >= 0) && (g <= 0xFF) && (b >= 0) &&
           (b <= 0xFF) && (a >= 0) && (a <= 0xFF));

    // exact nearest least-error quantization values
    return LookUpLattice(r, g, b, a, (bitrial ? 0 : 4));
  }

  Vec4 SnapToLatticeClamped(Vec4 const& val, int bitset, int bittest) const
  {
    Col4 p = FloatToInt<false>((grid * val) + gridgap);
    int r = p.R(), g = p.G(), b = p.B(), a = p.A();
    assert((r >= 0) && (r <= 0xFF) && (g >= 0) && (g <= 0xFF) && (b >= 0) &&
           (b <= 0xFF) && (a >= 0) && (a <= 0xFF));

    // exact nearest least-error quantization values
    return LookUpLattice(r, g, b, a, (bitrial ? 0 : 4));
  }

  /* -------------------------------------------------------------------------------
   */
  Col4 QuantizeToInt(Vec4 const& val, int bitset, int bittest) const
  {
    // [0,255]
    Vec4 Qf = SnapToLattice(val, bitset, bittest);

    // [0,1<<b-1]
    return FloatToInt<false>(Qf * gridint);
  }

  Col4 QuantizeToIntClamped(Vec4 const& val, int bitset, int bittest) const
  {
    // [0,255]
    Vec4 Qf = SnapToLatticeClamped(val, bitset, bittest);

    // [0,1<<b-1]
    return FloatToInt<false>(Qf * gridint);
  }

  /* -------------------------------------------------------------------------------
   */
#if defined(FEATURE_SHAREDBITS_TRIALS) && \
  (FEATURE_SHAREDBITS_TRIALS >= SHAREDBITS_TRIAL_PERMUTE)
  Vec4 SnapToLattice(Vec4 const& val, int bitset, int bittest, int oppose) const
  {
    // truncate the last valid bit (half multiplier)
    Vec4 Qf = Truncate(val * gridhlf);

    // add it in if wanted (half 1.0f)
    if (bitrial && (bitset & bittest))
    {
      Qf += Vec4(0.5f);
      // but go down by two as well (half 2.0f) per component
      if (oppose)
        Qf -= Vec4(oppose & 0x01 ? 1.0f : 0.0f, oppose & 0x04 ? 1.0f : 0.0f,
                   oppose & 0x10 ? 1.0f : 0.0f, oppose & 0x40 ? 1.0f : 0.0f);
    }
    // don't add it in if wanted (half 0.0f)
    else
    {
      // but go up by two as well (half 2.0f) per component
      if (oppose)
        Qf += Vec4(oppose & 0x01 ? 1.0f : 0.0f, oppose & 0x04 ? 1.0f : 0.0f,
                   oppose & 0x10 ? 1.0f : 0.0f, oppose & 0x40 ? 1.0f : 0.0f);
    }

    // get the bit back in (double multiplier)
    return (Qf * griddbl).Clamp();
  }
#endif

#undef qLUT_t
};

// extern cQuantizer3<5,6,5  > q3_565;
// extern cQuantizer4<5,6,5,0> q4_565;
}  // namespace squish

#endif
