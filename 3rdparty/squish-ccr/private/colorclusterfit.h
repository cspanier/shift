/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2007 Ignacio Castano                   icastano@nvidia.com
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

#ifndef SQUISH_colorCLUSTERFIT_H
#define SQUISH_colorCLUSTERFIT_H

#include <squish.h>
#include "maths_all.h"
#include "simd.h"
#include "colorfit.h"

namespace squish
{
class color_cluster_fit : public color_fit
{
public:
  color_cluster_fit(color_set const* colors, flags_t flags);

public:
  enum
  {
    kMinIterations = 1,
    kMaxIterations = 15
  };

  static flags_t sanitize_flags(flags_t flags)
  {
    if (flags > squish_flag::compressor_color_iterative_cluster_mask)
      return squish_flag::compressor_color_iterative_cluster_mask;
    if (flags > squish_flag::compressor_color_iterative_cluster_fit1)
      return squish_flag::compressor_color_iterative_cluster_fit1;

    return flags;
  }

private:
  void SumError3(std::uint8_t (&closest)[16], Vec4& beststart, Vec4& bestend,
                 Scr4& besterror);
  void SumError4(std::uint8_t (&closest)[16], Vec4& beststart, Vec4& bestend,
                 Scr4& besterror);

  void ComputeEndPoints();
  bool ConstructOrdering(Vec3 const& axis, int iteration);

  void ClusterFit3Constant(void* block);
  void ClusterFit4Constant(void* block);

  void ClusterFit3(void* block);
  void ClusterFit4(void* block);

  virtual void Compress3b(void* block);
  virtual void Compress3(void* block);
  virtual void Compress4(void* block);

  int m_iterationCount;
  Vec3 m_principle;
  Vec4 m_xsum_wsum;
  Vec4 m_points_weights[16];
  SQUISH_ALIGNED std::uint8_t m_order[16 * kMaxIterations];

  bool m_optimizable;
};
}  // namespace squish

#endif  // ndef SQUISH_colorCLUSTERFIT_H
