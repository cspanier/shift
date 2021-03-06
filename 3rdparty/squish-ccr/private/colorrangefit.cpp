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

#include "colorrangefit.h"
#include "colorset.h"
#include "colorblock.h"

#include "colorsinglefit.h"
#include "colorsinglesnap.h"

#include "inlineables.inl"

namespace squish
{
color_range_fit::color_range_fit(color_set const* colors, flags_t flags)
: color_fit(colors, flags)
{
  // initialize endpoints
  ComputeEndPoints();
}

void color_range_fit::ComputeEndPoints()
{
  cQuantizer3<5, 6, 5> q = cQuantizer3<5, 6, 5>();

  // cache some values
  int const count = m_colors->GetCount();
  Vec3 const* values = m_colors->GetPoints();
  Scr3 const* weights = m_colors->GetWeights();

  Sym3x3 covariance;
  Vec3 centroid;
  Vec3 principle;

  // get the covariance matrix
  if (m_colors->IsUnweighted())
    ComputeWeightedCovariance3(covariance, centroid, count, values, m_metric);
  else
    ComputeWeightedCovariance3(covariance, centroid, count, values, m_metric,
                               weights);

  // compute the principle component
  GetPrincipleComponent(covariance, principle);

  // get the min and max range as the codebook endpoints
  Vec3 start(0.0f);
  Vec3 end(0.0f);

  if (count > 0)
  {
#ifdef FEATURE_RANGEFIT_PROJECT
    // compute the projection
    GetPrincipleProjection(start, end, principle, centroid, count, values);
#else
    Scr3 min, max;

    // compute the range
    start = end = values[0];
    min = max = Dot(values[0], principle);

    for (int i = 1; i < count; ++i)
    {
      Scr3 val = Dot(values[i], principle);

      if (val < min)
      {
        start = values[i];
        min = val;
      }
      else if (val > max)
      {
        end = values[i];
        max = val;
      }
    }
#endif
  }

  // snap floating-point-values to the integer-lattice and save
  m_start = q.SnapToLattice(start);
  m_end = q.SnapToLattice(end);
}

void color_range_fit::Compress3b(void* block)
{
  color_set copy = *m_colors;
  m_colors = &copy;

  Scr3 m_destroyed = Scr3(0.0f);
  while (copy.RemoveBlack(m_metric, m_destroyed) &&
         !(m_besterror < m_destroyed))
  {
    m_besterror -= m_destroyed;

    if (copy.GetCount() == 1)
    {
      // always do a single color fit
      color_single_match fit(m_colors, m_flags);

      fit.SetError(m_besterror);
      fit.Compress(block);

      m_besterror = fit.GetError();
    }
    else
    {
      ComputeEndPoints();
      Compress3(block);
    }

    m_besterror += m_destroyed;
  }
}

void color_range_fit::Compress3(void* block)
{
  // cache some values
  int const count = m_colors->GetCount();
  Vec3 const* values = m_colors->GetPoints();
  Scr3 const* freq = m_colors->GetWeights();

  // create a codebook
  // resolve "metric * (value - code)" to "metric * value - metric * code"
  Vec3 codes[3];
  Codebook3(codes, m_metric * m_start, m_metric * m_end);

  // match each point to the closest code
  std::uint8_t closest[16];

  Scr3 error = Scr3(DISTANCE_BASE);
  for (int i = 0; i < count; ++i)
  {
    int idx = 0;

    // find the closest code
    Vec3 value = m_metric * values[i];
    Scr3 dist;
    MinDistance3<true>(dist, idx, value, codes);

    // save the index
    closest[i] = static_cast<std::uint8_t>(idx);

    // accumulate the error
    error += dist * freq[i];
  }

  // save this scheme if it wins
  if (error < m_besterror)
  {
    // save the error
    m_besterror = error;

    // remap the indices
    std::uint8_t indices[16];
    m_colors->RemapIndices(closest, indices);

    // save the block
    WritecolorBlock3(m_start, m_end, indices, block);
  }
}

void color_range_fit::Compress4(void* block)
{
  // cache some values
  int const count = m_colors->GetCount();
  Vec3 const* values = m_colors->GetPoints();
  Scr3 const* freq = m_colors->GetWeights();

  // create a codebook
  // resolve "metric * (value - code)" to "metric * value - metric * code"
  Vec3 codes[4];
  Codebook4(codes, m_metric * m_start, m_metric * m_end);

  // match each point to the closest code
  std::uint8_t closest[16];

  Scr3 error = Scr3(DISTANCE_BASE);
  for (int i = 0; i < count; ++i)
  {
    int idx = 0;

    // find the closest code
    Vec3 value = m_metric * values[i];
    Scr3 dist;
    MinDistance4<true>(dist, idx, value, codes);

    // accumulate the error
    AddDistance(dist, error, freq[i]);

    // save the index
    closest[i] = static_cast<std::uint8_t>(idx);
  }

  // save this scheme if it wins
  if (error < m_besterror)
  {
    // save the error
    m_besterror = error;

    // remap the indices
    std::uint8_t indices[16];
    m_colors->RemapIndices(closest, indices);

    // save the block
    WritecolorBlock4(m_start, m_end, indices, block);
  }
}
}  // namespace squish
