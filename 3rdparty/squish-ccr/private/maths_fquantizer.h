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

#ifndef SQUISH_MATHS_FQUANTIZER_H
#define SQUISH_MATHS_FQUANTIZER_H

#include "maths.h"
#include "simd.h"

namespace squish
{
class fQuantizer
{
public:
  void ChangeField(const int tb, const int db) const
  {
  }

  int trunc;
  Col3 gridprc;
  Col3 gridrnd;
  Col3 griddlt;
  Col3 griddltp;
  Col3 griddltm;

  fQuantizer(const int tb, const int db)
  {
    /* assign the precision bits */
    //  assert(tb > 0);
    trunc = tb;

    /* truncate: tb = 4, 1 << 4 = 0x10 - 1 = 0xF ~= 0xFFF0 */
    gridprc.SetRGBpow2<0>(tb);
    gridrnd = gridprc;
    gridprc -= Col3(1);
    gridrnd >>= 1;
    gridprc = ~gridprc;

    if (db)
      /* TODO: joint load and unpack */
      griddlt.SetRGBpow2<0>((db >> 0) & 0xFF, (db >> 8) & 0xFF,
                            (db >> 16) & 0xFF /*, (db >> 24) & 0xFF*/);
    else
      griddlt.SetRGBpow2<16>(tb);

    /* plus/minus delta range */
    griddltp = (griddlt >> 1) - Col3(1);
    griddltm = (~griddltp) + Col3(1);
  }

  Col3 MaskLattice(Col3& p) const
  {
    // exact nearest least-error quantization values
    // NOTE: rounding doesn't help ... probably because of the log-scale
    return (p & gridprc) + gridrnd;
  }

  Col3 DeltaLattice(Col3& p, Col3& b) const
  {
    // bring values into signed word range first
    Col3 d = (p - b) >> trunc;

    // exact nearest least-error quantization values
    d = Max(Min(d, griddltp), griddltm);
    d = d << trunc;

    // mask to half-range
    return (d + b) & Col3(0x0000FFFF);
  }

  Col3 QuantizeToLattice(Vec3 const& val) const
  {
    Col3 p = FloatToSHalf<false>(val);
    int prec = (16 - trunc);

    // re-range X bits of precision to 0x7C00
    p <<= prec;
    p /= (0x7BFF + 1);

    // zero bottom bits
    p <<= trunc;
    // add half a unit (if not zero or INF)
    p += gridrnd & CompareAllEqualTo_M4(p, Col3(0x00000000));
    p |= CompareAllEqualTo_M4(p, Col3(0x0000FFFF) & gridprc);

    return p;
  }

  Vec3 UnquantizeFromLattice(Col3 const& val) const
  {
    Col3 p = val;

    // re-range 0xFFFF to X bits of precision
    // 0xFFFF * 32 >> 6 = 0x7BFF
    p *= 31 << (16 - 6);

    return SHalfToFloat(p);
  }

  Col3 QuantizeToLattice(Vec3 const& val, Col3 const& bse) const
  {
    Col3 b = bse;
    Col3 p = QuantizeToLattice(val);

    // exact nearest least-error quantization values
    p = DeltaLattice(p, b);

    return p;
  }

  void QuantizeToLattice(Vec3 const (&val)[2], Col3 (&res)[2]) const
  {
    Col3 b = QuantizeToLattice(val[0]);
    Col3 p = QuantizeToLattice(val[1]);

    // exact nearest least-error quantization values
    //	b = MaskLattice(b);
    p = DeltaLattice(p, b);

    res[0] = b;
    res[1] = p;
  }
};
}  // namespace squish

#endif
