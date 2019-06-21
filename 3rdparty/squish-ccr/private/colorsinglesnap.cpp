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

#include "colorsinglesnap.h"
#include "colorset.h"
#include "colorblock.h"

#include "inlineables.inl"

namespace squish
{

/* *****************************************************************************
 */
struct colorSingleLookup
{
  std::uint8_t start;
  std::uint8_t end;
};

#undef SCL_ITERATIVE
#include "colorsinglelookup.inl"

color_single_snap::color_single_snap(color_set const* colors, flags_t flags)
: color_fit(colors, flags)
{
  // grab the single color
  Vec3 const* values = m_colors->GetPoints();

  // in the 3/4 codebook case it can be observed
  // that the error of the end-point is always
  // higher than the error of any interpolated value
  // in addition the codebook is symmetric, means
  // index 2 can always be made index 1
  // as a result the lookup can be made in such a way
  // that index 1 always contains the best match
  //
  // this holds even if a metric is used, the metric does
  // a linear rescale of the RGB-cube: scaling preserves
  // relative lengths, in either cube the error-vector with
  // smallest length is the same

  // values are directly out of the codebook and
  // natural numbers / 255, no need to round
  PackBytes(FloatToInt<true>((*values) * Vec3(255.0f)),
            (unsigned int&)(m_color));

  /*
  assert(m_color[0] == static_cast<std::uint8_t>(FloatToInt<true,false>(255.0f *
  values->X(), 255)));
  assert(m_color[1] == static_cast<std::uint8_t>(FloatToInt<true,false>(255.0f *
  values->Y(), 255)));
  assert(m_color[2] == static_cast<std::uint8_t>(FloatToInt<true,false>(255.0f *
  values->Z(), 255)));
   */
}

void color_single_snap::Compress3b(void* block)
{
  // grab the single color
  Vec3 const* values = m_colors->GetPoints();

  // if it's black, make it index 3
  if (values[0] == Vec3(0.0f))
  {
    *((std::uint64_t*)block) = 0xFFFFFFFF00000000ULL;
  }
}

void color_single_snap::Compress3(void* block)
{
  // grab the single color
  Vec3 const* values = m_colors->GetPoints();
  Scr3 const* freq = m_colors->GetWeights();

  // just assign the end-points of index 2 (interpolant 1)
  Col3 s =
    Col3(sc_lookup_5_3[m_color[0]].start, sc_lookup_6_3[m_color[1]].start,
         sc_lookup_5_3[m_color[2]].start);
  Col3 e = Col3(sc_lookup_5_3[m_color[0]].end, sc_lookup_6_3[m_color[1]].end,
                sc_lookup_5_3[m_color[2]].end);

  m_start = Vec3(s) * (1.0f / 255.0f);
  m_end = Vec3(e) * (1.0f / 255.0f);

  // created interpolated value and error
  Vec3 code = (m_start + m_end) * 0.5f;
  Scr3 error = LengthSquared(m_metric * (values[0] - code)) * freq[0];

  // build the block if we win
  if (error < m_besterror)
  {
    // save the error
    m_besterror = error;

    // build the block
    std::uint8_t idx = 2, indices[16];
    m_colors->RemapIndices(&idx, indices);

    // save the block
    WritecolorBlock3(m_start, m_end, indices, block);
  }
}

void color_single_snap::Compress4(void* block)
{
  // grab the single color
  Vec3 const* values = m_colors->GetPoints();
  Scr3 const* freq = m_colors->GetWeights();

  // just assign the end-points of index 2 (interpolant 1)
  Col3 s =
    Col3(sc_lookup_5_4[m_color[0]].start, sc_lookup_6_4[m_color[1]].start,
         sc_lookup_5_4[m_color[2]].start);
  Col3 e = Col3(sc_lookup_5_4[m_color[0]].end, sc_lookup_6_4[m_color[1]].end,
                sc_lookup_5_4[m_color[2]].end);

  m_start = Vec3(s) * (1.0f / 255.0f);
  m_end = Vec3(e) * (1.0f / 255.0f);

  // created interpolated value and error
  Vec3 code = (2.0f * m_start + m_end) * (1.0f / 3.0f);
  Scr3 error = LengthSquared(m_metric * (values[0] - code)) * freq[0];

  // build the block if we win
  if (error < m_besterror)
  {
    // save the error
    m_besterror = error;

    // build the block
    std::uint8_t idx = 2, indices[16];
    m_colors->RemapIndices(&idx, indices);

    // save the block
    WritecolorBlock4(m_start, m_end, indices, block);
  }
}
}  // namespace squish

#if defined(SBL_FLAT)
#include "colorsinglesnap_ccr_flat.inl"
#elif defined(SBL_PACKED) && (SBL_PACKED == 1)
#include "colorsinglesnap_ccr_packed.inl"
#elif defined(SBL_PACKED) && (SBL_PACKED == 2)
#include "colorsinglesnap_ccr_packed_copy.inl"
#elif defined(SBL_VECTOR)
#include "colorsinglesnap_ccr_vector.inl"
#endif
