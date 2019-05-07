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

#include "colorsinglefit.h"
#include "colorset.h"
#include "colorblock.h"

#include "inlineables.inl"

namespace squish {

/* *****************************************************************************
 */
struct SC_SourceBlock
{
  std::uint8_t start;
  std::uint8_t end;
  std::uint8_t error;
};

struct colorSingleLookup
{
  SC_SourceBlock sources[2];
};

#define  SCL_ITERATIVE
#include "colorsinglelookup.inl"

color_single_fit::color_single_fit(color_set const* colors, int flags)
  : color_fit(colors, flags)
{
  // grab the single color
  Vec3 const* values = m_colors->GetPoints();
  Col3 integers = Min(FloatToInt<true>(*values * Vec3(255.0f)), Col3(255));

  m_color[0] = (std::uint8_t)integers.R();
  m_color[1] = (std::uint8_t)integers.G();
  m_color[2] = (std::uint8_t)integers.B();

  // initialize the best error
  m_besterror = Scr3(FLT_MAX);
}

void color_single_fit::Compress3(void* block)
{
  // build the table of lookups
  colorSingleLookup const* const lookups[] =
  {
    sc_lookup_5_3,
    sc_lookup_6_3,
    sc_lookup_5_3
  };

  // find the best end-points and index
  Scr3 error = Scr3(1.0f / (255.0f * 255.0f)) * ComputeEndPoints(lookups);

  // build the block if we win
  if (error < m_besterror) {
    // save the error
    m_besterror = error;

    // remap the indices
    std::uint8_t indices[16]; m_colors->RemapIndices(&m_index, indices);

    // save the block
    WritecolorBlock3(m_start, m_end, indices, block);
  }
}

void color_single_fit::Compress4(void* block)
{
  // build the table of lookups
  colorSingleLookup const* const lookups[] =
  {
    sc_lookup_5_4,
    sc_lookup_6_4,
    sc_lookup_5_4
  };

  // find the best end-points and index
  Scr3 error = Scr3(1.0f / (255.0f * 255.0f)) * ComputeEndPoints(lookups);

  // build the block if we win
  if (error < m_besterror) {
    // save the error
    m_besterror = error;

    // remap the indices
    std::uint8_t indices[16]; m_colors->RemapIndices(&m_index, indices);

    // save the block
    WritecolorBlock4(m_start, m_end, indices, block);
  }
}

int color_single_fit::ComputeEndPoints(colorSingleLookup const* const* lookups)
{
  // check each index combination (endpoint or intermediate)
  int besterror = INT_MAX;

  for (int index = 0; index < 2; ++index) {
    // check the error for this codebook index
    SC_SourceBlock const* sources[3];
    int error = 0;

    for (int channel = 0; channel < 3; ++channel) {
      // grab the lookup table and index for this channel
      colorSingleLookup const* lookup = lookups[channel];
      int target = m_color[channel];

      // store a pointer to the source for this channel
      sources[channel] = lookup[target].sources + index;

      // accumulate the error
      int diff = sources[channel]->error;
      error += diff * diff;
    }

    // keep it if the error is lower
    if (error < besterror) {
      // save the error
      besterror = error;

      m_start = Vec3(
  (float)sources[0]->start,
  (float)sources[1]->start,
  (float)sources[2]->start
      );

      m_end = Vec3(
  (float)sources[0]->end,
  (float)sources[1]->end,
  (float)sources[2]->end
      );

      m_start /= Vec3(31.0f, 63.0f, 31.0f);
      m_end   /= Vec3(31.0f, 63.0f, 31.0f);

      m_index = (std::uint8_t)(2 * index);

      // early out
      if (!besterror)
  return besterror;
    }
  }

  return besterror;
}

} // namespace squish

#if  defined(SBL_FLAT)
#include "colorsinglefit_ccr_flat.inl"
#elif  defined(SBL_PACKED) && (SBL_PACKED == 1)
#include "colorsinglefit_ccr_packed.inl"
#elif  defined(SBL_PACKED) && (SBL_PACKED == 2)
#include "colorsinglefit_ccr_packed_copy.inl"
#elif  defined(SBL_VECTOR)
#include "colorsinglefit_ccr_vector.inl"
#endif
