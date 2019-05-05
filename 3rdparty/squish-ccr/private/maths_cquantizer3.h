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

#ifndef SQUISH_MATHS_CQUANTIZER3_H
#define SQUISH_MATHS_CQUANTIZER3_H

#include "maths.h"
#include "simd.h"

namespace squish
{
template <const int rb, const int gb, const int bb>
class cQuantizer3
{
public:
  static const int rm = (1 << rb) - 1;
  static const int gm = (1 << gb) - 1;
  static const int bm = (1 << bb) - 1;

  // remainders
  static const int rr = (1 << (8 - rb));
  static const int gr = (1 << (8 - gb));
  static const int br = (1 << (8 - bb));

  /* half remainders
  static const float rh = 0.5f * (1 << (8 - rb)) * (rm / 255.0f);
  static const float gh = 0.5f * (1 << (8 - gb)) * (gm / 255.0f);
  static const float bh = 0.5f * (1 << (8 - bb)) * (bm / 255.0f); */

  Vec3 grid;
  Vec3 gridgap;
  Col3 gridinv;

  cQuantizer3()
  : grid(1.0f * rm, 1.0f * gm, 1.0f * bm),
    gridinv(1 << rb, 1 << gb, 1 << bb),
    gridgap((0.5f * rr * rm) / 255.0f, (0.5f * gr * gm) / 255.0f,
            (0.5f * br * bm) / 255.0f)
  {
  }

  Vec3 LookUpLattice(int r, int g, int b) const
  {
    // exact nearest least-error quantization values
    return Vec3(&qLUT_a[rb][r], &qLUT_a[gb][g], &qLUT_a[bb][b]);
  }

  Vec3 SnapToLattice(const Vec3& val) const
  {
    Col3 p = FloatToInt<false>((grid * val.Clamp()) + gridgap);
#ifndef NDEBUG
    unsigned int lu;
    PackBytes(p, lu);
#endif
    int r = p.R(), g = p.G(), b = p.B();
    assert((r >= 0) && (r <= rm) && (g >= 0) && (g <= gm) && (b >= 0) &&
           (b <= bm));

    // exact nearest least-error quantization values
    Vec3 rgb = LookUpLattice(r, g, b);

    assert(((unsigned int)(rgb.X() * 255.0f) >> (8 - rb)) == ((lu >> 0) & rm));
    assert(((unsigned int)(rgb.Y() * 255.0f) >> (8 - gb)) == ((lu >> 8) & gm));
    assert(((unsigned int)(rgb.Z() * 255.0f) >> (8 - bb)) == ((lu >> 16) & bm));

    return rgb;
  }

  Vec3 SnapToLatticeClamped(const Vec3& val) const
  {
    Col3 p = FloatToInt<false>((grid * val) + gridgap);
#ifndef NDEBUG
    unsigned int lu;
    PackBytes(p, lu);
#endif
    int r = p.R(), g = p.G(), b = p.B();
    assert((r >= 0) && (r <= rm) && (g >= 0) && (g <= gm) && (b >= 0) &&
           (b <= bm));

    // exact nearest least-error quantization values
    Vec3 rgb = LookUpLattice(r, g, b);

    assert(((unsigned int)(rgb.X() * 255.0f) >> (8 - rb)) == ((lu >> 0) & rm));
    assert(((unsigned int)(rgb.Y() * 255.0f) >> (8 - gb)) == ((lu >> 8) & gm));
    assert(((unsigned int)(rgb.Z() * 255.0f) >> (8 - bb)) == ((lu >> 16) & bm));

    return rgb;
  }

  Col3 QuantizeToInt(const Vec3& val) const
  {
    // [0,255]
    Vec3 Qf = SnapToLattice(val);

    // [0,1<<b-1]
    return (FloatToInt<false>(Qf * Vec3(255.0f)) * gridinv) >> 8;
  }

  Col3 QuantizeToIntClamped(const Vec3& val) const
  {
    // [0,255]
    Vec3 Qf = SnapToLatticeClamped(val);

    // [0,1<<b-1]
    return (FloatToInt<false>(Qf * Vec3(255.0f)) * gridinv) >> 8;
  }

  Col3 LatticeToInt(const Vec3& val) const
  {
    // [0,255]
    Vec3 Qf = val.Clamp();

    // [0,1<<b-1]
    return (FloatToInt<false>(Qf * Vec3(255.0f)) * gridinv) >> 8;
  }

  Col3 LatticeToIntClamped(const Vec3& val) const
  {
    // [0,255]
    Vec3 Qf = val;

    // [0,1<<b-1]
    return (FloatToInt<false>(Qf * Vec3(255.0f)) * gridinv) >> 8;
  }
};
}  // namespace squish

#endif
