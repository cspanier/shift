/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels FrÃ¶hling              niels@paradice-insight.us

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

#include "colourrangefit.h"
#include "colourset.h"
#include "colourblock.h"

#include "coloursinglefit.h"
#include "coloursinglesnap.h"

#include "inlineables.inl"

namespace squish {

/* *****************************************************************************
 */
ColourRangeFit::ColourRangeFit(ColourSet const* colours, int flags)
  : ColourFit(colours, flags)
{
  // initialize endpoints
  ComputeEndPoints();
}

void ColourRangeFit::ComputeEndPoints()
{
  cQuantizer3<5,6,5> q = cQuantizer3<5,6,5>();

  // cache some values
  int const count = m_colours->GetCount();
  Vec3 const* values = m_colours->GetPoints();
  Scr3 const* weights = m_colours->GetWeights();

  Sym3x3 covariance;
  Vec3 centroid;
  Vec3 principle;

  // get the covariance matrix
  if (m_colours->IsUnweighted())
    ComputeWeightedCovariance3(covariance, centroid, count, values, m_metric);
  else
    ComputeWeightedCovariance3(covariance, centroid, count, values, m_metric, weights);

  // compute the principle component
  GetPrincipleComponent(covariance, principle);

  // get the min and max range as the codebook endpoints
  Vec3 start(0.0f);
  Vec3 end(0.0f);

  if (count > 0) {
#ifdef  FEATURE_RANGEFIT_PROJECT
    // compute the projection
    GetPrincipleProjection(start, end, principle, centroid, count, values);
#else
    Scr3 min, max;

    // compute the range
    start = end = values[0];
    min = max = Dot(values[0], principle);

    for (int i = 1; i < count; ++i) {
      Scr3 val = Dot(values[i], principle);

      if (val < min) {
  start = values[i];
  min = val;
      }
      else if (val > max) {
  end = values[i];
  max = val;
      }
    }
#endif
  }

  // snap floating-point-values to the integer-lattice and save
  m_start = q.SnapToLattice(start);
  m_end   = q.SnapToLattice(end  );
}

void ColourRangeFit::Compress3b(void* block)
{
  ColourSet copy = *m_colours;
  m_colours = &copy;

  Scr3 m_destroyed = Scr3(0.0f);
  while (copy.RemoveBlack(m_metric, m_destroyed) && !(m_besterror < m_destroyed)) {
    m_besterror -= m_destroyed;

    if (copy.GetCount() == 1) {
      // always do a single colour fit
      ColourSingleMatch fit(m_colours, m_flags);

      fit.SetError(m_besterror);
      fit.Compress(block);

      m_besterror = fit.GetError();
    }
    else {
      ComputeEndPoints();
      Compress3(block);
    }

    m_besterror += m_destroyed;
  }
}

void ColourRangeFit::Compress3(void* block)
{
  // cache some values
  int const count = m_colours->GetCount();
  Vec3 const* values = m_colours->GetPoints();
  Scr3 const* freq = m_colours->GetWeights();

  // create a codebook
  // resolve "metric * (value - code)" to "metric * value - metric * code"
  Vec3 codes[3]; Codebook3(codes, m_metric * m_start, m_metric * m_end);

  // match each point to the closest code
  std::uint8_t closest[16];

  Scr3 error = Scr3(DISTANCE_BASE);
  for (int i = 0; i < count; ++i) {
    int idx = 0;

    // find the closest code
    Vec3 value = m_metric * values[i];
    Scr3 dist; MinDistance3<true>(dist, idx, value, codes);

    // save the index
    closest[i] = (std::uint8_t)idx;

    // accumulate the error
    error += dist * freq[i];
  }

  // save this scheme if it wins
  if (error < m_besterror) {
    // save the error
    m_besterror = error;

    // remap the indices
    std::uint8_t indices[16]; m_colours->RemapIndices(closest, indices);

    // save the block
    WriteColourBlock3(m_start, m_end, indices, block);
  }
}

void ColourRangeFit::Compress4(void* block)
{
  // cache some values
  int const count = m_colours->GetCount();
  Vec3 const* values = m_colours->GetPoints();
  Scr3 const* freq = m_colours->GetWeights();

  // create a codebook
  // resolve "metric * (value - code)" to "metric * value - metric * code"
  Vec3 codes[4]; Codebook4(codes, m_metric * m_start, m_metric * m_end);

  // match each point to the closest code
  std::uint8_t closest[16];

  Scr3 error = Scr3(DISTANCE_BASE);
  for (int i = 0; i < count; ++i) {
    int idx = 0;

    // find the closest code
    Vec3 value = m_metric * values[i];
    Scr3 dist; MinDistance4<true>(dist, idx, value, codes);

    // accumulate the error
    AddDistance(dist, error, freq[i]);

    // save the index
    closest[i] = (std::uint8_t)idx;
  }

  // save this scheme if it wins
  if (error < m_besterror) {
    // save the error
    m_besterror = error;

    // remap the indices
    std::uint8_t indices[16]; m_colours->RemapIndices(closest, indices);

    // save the block
    WriteColourBlock4(m_start, m_end, indices, block);
  }
}
} // namespace squish
