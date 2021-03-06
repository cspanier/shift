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

#include "paletteset.h"
#include "helpers.h"

namespace squish
{

// Associated to partition -1, 16 * 0 bit
extern const std::uint16_t partitionmasks_1[1] = {0xFFFF};

// Associated to partition 0-63, 16 * 1 bit
extern const std::uint16_t partitionmasks_2[64] = {
  0xCCCC, 0x8888, 0xEEEE, 0xECC8, 0xC880, 0xFEEC, 0xFEC8, 0xEC80,
  0xC800, 0xFFEC, 0xFE80, 0xE800, 0xFFE8, 0xFF00, 0xFFF0, 0xF000,
  0xF710, 0x008E, 0x7100, 0x08CE, 0x008C, 0x7310, 0x3100, 0x8CCE,
  0x088C, 0x3110, 0x6666, 0x366C, 0x17E8, 0x0FF0, 0x718E, 0x399C,

  0xaaaa, 0xf0f0, 0x5a5a, 0x33cc, 0x3c3c, 0x55aa, 0x9696, 0xa55a,
  0x73ce, 0x13c8, 0x324c, 0x3bdc, 0x6996, 0xc33c, 0x9966, 0x0660,
  0x0272, 0x04e4, 0x4e40, 0x2720, 0xc936, 0x936c, 0x39c6, 0x639c,
  0x9336, 0x9cc6, 0x817e, 0xe718, 0xccf0, 0x0fcc, 0x7744, 0xee22,
};

// Associated to partition 64-127, 16 * 2 bit
extern const unsigned int partitionmasks_3[64] = {
  0xf60008cc, 0x73008cc8, 0x3310cc80, 0x00ceec00, 0xcc003300, 0xcc0000cc,
  0x00ccff00, 0x3300cccc, 0xf0000f00, 0xf0000ff0, 0xff0000f0, 0x88884444,
  0x88886666, 0xcccc2222, 0xec80136c, 0x7310008c, 0xc80036c8, 0x310008ce,
  0xccc03330, 0x0cccf000, 0xee0000ee, 0x77008888, 0xcc0022c0, 0x33004430,
  0x00cc0c22, 0xfc880344, 0x06606996, 0x66009960, 0xc88c0330, 0xf9000066,
  0x0cc0c22c, 0x73108c00,

  0xec801300, 0x08cec400, 0xec80004c, 0x44442222, 0x0f0000f0, 0x49242492,
  0x42942942, 0x0c30c30c, 0x03c0c03c, 0xff0000aa, 0x5500aa00, 0xcccc3030,
  0x0c0cc0c0, 0x66669090, 0x0ff0a00a, 0x5550aaa0, 0xf0000aaa, 0x0e0ee0e0,
  0x88887070, 0x99906660, 0xe00e0ee0, 0x88880770, 0xf0000666, 0x99006600,
  0xff000066, 0xc00c0cc0, 0xcccc0330, 0x90006000, 0x08088080, 0xeeee1010,
  0xfff0000a, 0x731008ce,
};

/* *****************************************************************************
 */
void palette_set::GetMasks(flags_t flags, int partition, int (&masks)[4])
{
  unsigned int partmask = 0;
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode2) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode4) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode8))
    partmask = partitionmasks_2[partition];
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode1) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode3))
    partmask = partitionmasks_3[partition];

  // determine the number of partitions
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode5) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode6) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode7))
    masks[0] = (partmask & 0xFFFF) & (0xFFFFFFFF >> 16),    // color-set
      masks[1] = (partmask & 0xFFFF) & (0xFFFFFFFF >> 16),  // alpha-set
      masks[2] = 0;
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode1) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode3))
    masks[0] = (~partmask & 0xFFFF) & (0xFFFFFFFF >> 16),
    masks[1] = (partmask & 0xFFFF) & (0xFFFFFFFF >> 16), masks[2] = 0;
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode2) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode4) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode8))
    masks[0] = (~partmask & 0xFFFF) & (~partmask >> 16),
    masks[1] = (partmask & 0xFFFF) & (~partmask >> 16),
    masks[2] = (0xFFFFFFFF & 0xFFFF) & (partmask >> 16);
}

int palette_set::SetMode(flags_t flags)
{
  /* build a single set only, we permute that later for specific partitions,
   * separate alpha is an exception as that is fixed for each mode
   */
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode2) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode4) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode8))
  {
    m_numsets = 2;
    m_partmask = 0xFFFFFFFF;
    m_partid = 0;
  }
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode1) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode3))
  {
    m_numsets = 3;
    m_partmask = 0xFFFFFFFF;
    m_partid = 0;
  }
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode5) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode6))
  {
    m_seperatealpha = true;
    m_rotid = 0;
  }
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode7) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode8))
  {
    m_mergedalpha = true;
  }

  // partition_1 mask is: bit cleared -> set 1, bit set -> set 2
  // partition_2 mask is: bit cleared -> set 1, bit set -> set 2, hi bit set ->
  // set 3
  if (1)
    m_mask[0] = 0xFFFF,    // color-set
      m_mask[1] = 0xFFFF,  // alpha-set
      m_mask[2] = 0;
  if (m_numsets > 1)
    m_numsets = 1;

  return flags;
}

int palette_set::SetMode(flags_t flags, int part_or_rot)
{
  /* determine the number of sets and select the partition
  if ((0))
    m_numsets = 1, m_partmask = partitionmasks_1[m_partid = 0]; */
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode2) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode4) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode8))
  {
    m_numsets = 2;
    m_partmask = partitionmasks_2[m_partid = part_or_rot];
  }
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode1) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode3))
  {
    m_numsets = 3;
    m_partmask = partitionmasks_3[m_partid = part_or_rot];
  }
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode5) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode6))
  {
    m_seperatealpha = true;
    m_rotid = part_or_rot;
  }
  if (((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode7) ||
      ((flags & squish_flag::variable_coding_mode_mask) ==
       squish_flag::variable_coding_mode8))
  {
    m_mergedalpha = true;
  }

  // partition_1 mask is: bit cleared -> set 1, bit set -> set 2
  // partition_2 mask is: bit cleared -> set 1, bit set -> set 2, hi bit set ->
  // set 3
  if (m_numsets == 1)
    m_mask[0] = (m_partmask & 0xFFFF) & (0xFFFFFFFF >> 16),    // color-set
      m_mask[1] = (m_partmask & 0xFFFF) & (0xFFFFFFFF >> 16),  // alpha-set
      m_mask[2] = 0;
  if (m_numsets == 2)
    m_mask[0] = (~m_partmask & 0xFFFF) & (0xFFFFFFFF >> 16),
    m_mask[1] = (m_partmask & 0xFFFF) & (0xFFFFFFFF >> 16), m_mask[2] = 0;
  if (m_numsets == 3)
    m_mask[0] = (~m_partmask & 0xFFFF) & (~m_partmask >> 16),
    m_mask[1] = (m_partmask & 0xFFFF) & (~m_partmask >> 16),
    m_mask[2] = (0xFFFFFFFF & 0xFFFF) & (m_partmask >> 16);

  return flags;
}

palette_set::palette_set(std::uint8_t const* rgba, std::uint32_t mask,
                         flags_t flags)
: m_numsets(1),
  m_rotid(0),
  m_partid(0),
  m_partmask(0xFFFF),
  m_seperatealpha(false),
  m_mergedalpha(false),
  m_transparent(false)
{
  // make the set (if succesive partition permutation, preserve alpha)
  BuildSet(rgba, mask, SetMode(flags));
}

palette_set::palette_set(std::uint16_t const* rgba, std::uint32_t mask,
                         flags_t flags)
: m_numsets(1),
  m_rotid(0),
  m_partid(0),
  m_partmask(0xFFFF),
  m_seperatealpha(false),
  m_mergedalpha(false),
  m_transparent(false)
{
  // make the set (if succesive partition permutation, preserve alpha)
  BuildSet(rgba, mask, SetMode(flags));
}

palette_set::palette_set(float const* rgba, std::uint32_t mask, flags_t flags)
: m_numsets(1),
  m_rotid(0),
  m_partid(0),
  m_partmask(0xFFFF),
  m_seperatealpha(false),
  m_mergedalpha(false),
  m_transparent(false)
{
  // make the set (if succesive partition permutation, preserve alpha)
  BuildSet(rgba, mask, SetMode(flags));
}

palette_set::palette_set(std::uint8_t const* rgba, std::uint32_t mask,
                         flags_t flags, int part_or_rot)
: m_numsets(1),
  m_rotid(0),
  m_partid(0),
  m_partmask(0xFFFF),
  m_seperatealpha(false),
  m_mergedalpha(false),
  m_transparent(false)
{
  // make the set
  BuildSet(rgba, mask, SetMode(flags, part_or_rot));
}

palette_set::palette_set(std::uint16_t const* rgba, std::uint32_t mask,
                         flags_t flags, int part_or_rot)
: m_numsets(1),
  m_rotid(0),
  m_partid(0),
  m_partmask(0xFFFF),
  m_seperatealpha(false),
  m_mergedalpha(false),
  m_transparent(false)
{
  // make the set
  BuildSet(rgba, mask, SetMode(flags, part_or_rot));
}

palette_set::palette_set(float const* rgba, std::uint32_t mask, flags_t flags,
                         int part_or_rot)
: m_numsets(1),
  m_rotid(0),
  m_partid(0),
  m_partmask(0xFFFF),
  m_seperatealpha(false),
  m_mergedalpha(false),
  m_transparent(false)
{
  // make the set
  BuildSet(rgba, mask, SetMode(flags, part_or_rot));
}

palette_set::palette_set(palette_set const& palette, std::uint32_t mask,
                         flags_t flags, int part_or_rot)
: m_numsets(1),
  m_rotid(0),
  m_partid(0),
  m_partmask(0xFFFF),
  m_seperatealpha(false),
  m_mergedalpha(false),
  m_transparent(false)
{
  flags = SetMode(flags, part_or_rot);

  // make, permute or copy the new set
  if (m_seperatealpha)
    BuildSet(palette, mask, flags);  // unpermutable
  else if (m_numsets > 1)
    PermuteSet(palette, mask, flags);  // permutable
  else
    memcpy(this, &palette, sizeof(*this));  // identical
}

void palette_set::BuildSet(std::uint8_t const* rgba, std::uint32_t mask,
                           flags_t flags)
{
  const float* rgbLUT =
    ComputeGammaLUT(flags & squish_flag::option_srgb_external);
  const float* aLUT = ComputeGammaLUT(false);

  // check the compression mode for btc
  bool const clearAlpha =
    flags & squish_flag::option_exclude_alpha_from_palette;
  bool const seperateAlpha =
    !(flags & squish_flag::option_exclude_alpha_from_palette) &&
    m_seperatealpha;
  bool const weightByAlpha =
    (flags & squish_flag::option_weight_color_by_alpha) && !m_mergedalpha;
  bool const killByAlpha = flags & squish_flag::option_weight_color_by_alpha;

  // build mapped data
  std::uint8_t const mska = !seperateAlpha ? 0xFF : 0x00;
  std::uint8_t const clra = !clearAlpha ? 0x00 : 0xFF;
  std::uint8_t const wgta = weightByAlpha ? 0x00 : 0xFF;
  std::uint8_t const klla = killByAlpha ? 0x00 : 0xFF;

  std::uint8_t rgbx[4 * 16], wgtx = wgta;
  std::uint8_t ___a[1 * 16], ___w = 0xFF;
  int amask = 0;

  /* Apply the component rotation, while preserving semantics:
   * - swap: aa, ra, ga, ba
   * - LUTs: always pull from the right transform
   * - weights: never make alpha weighted by itself
   *   TODO: as we have no separate component weighting currently we
   *         have to turn alpha-weighting off to allow the cluster-fit
   *         to work (same weight for a AGB-point fe., R-point is ok)
   */
  int rot[4] = {0, 1, 2, 3};
  const float* caLUTs[] = {rgbLUT, rgbLUT, rgbLUT, aLUT};

  switch (m_rotid)
  {
  case 1:
    rot[0] = 3;
    rot[3] = 0;
    caLUTs[0] = aLUT;
    caLUTs[3] = rgbLUT;
    wgtx = 0xFF;
    ___w = wgta;
    break;
  case 2:
    rot[1] = 3;
    rot[3] = 1;
    caLUTs[1] = aLUT;
    caLUTs[3] = rgbLUT;
    wgtx = 0xFF;
    ___w = wgta;
    break;
  case 3:
    rot[2] = 3;
    rot[3] = 2;
    caLUTs[2] = aLUT;
    caLUTs[3] = rgbLUT;
    wgtx = 0xFF;
    ___w = wgta;
    break;
    //  default: rot[3] = 3; rot[3] = 3; caLUTs[3] = aLUT; caLUTs[3] =   aLUT;
    //  wgtx = wgta; ___w = 0xFF; break;
  }

  for (int i = 0; i < 16; ++i)
  {
    std::uint8_t temp[4];

#ifdef FEATURE_IGNORE_ALPHA0
    // kill color
    amask += (!(rgba[4 * i + 3] | klla)) << i;
#endif

    // clear alpha
    temp[0] = rgba[4 * i + 0];
    temp[1] = rgba[4 * i + 1];
    temp[2] = rgba[4 * i + 2];
    temp[3] = rgba[4 * i + 3] | clra;

    // clear channel 3
    rgbx[4 * i + 0] = temp[rot[0]];
    rgbx[4 * i + 1] = temp[rot[1]];
    rgbx[4 * i + 2] = temp[rot[2]];
    rgbx[4 * i + 3] = temp[rot[3]] & mska;

    // separate channel 3
    ___a[1 * i + 0] = temp[rot[3]];

    // check for transparency (after blanking out)
    m_transparent = m_transparent | (temp[3] < 255);

    // temporary, TODO: remove it
    m_weights[3][i] = Weight<std::uint8_t>(rgba, i, 0).GetWeights();
  }

  // combined mask
  amask = mask & (~amask);

  // clean initial state
  m_count[0] = m_count[1] = m_count[2] = m_count[3] = 0;
  m_unweighted[0] = m_unweighted[1] = m_unweighted[2] = m_unweighted[3] = true;

  // TODO: should not be necessary (VC bug?)
  m_remap.fill({0x00, 0x00, 0x00, 0x00});

  // required for being able to reorder the contents of "rgbx"
  assert(m_numsets == 1);
  for (int s = 0; s < m_numsets; s++)
  {
    // combined alpha, exclusion and selection mask
    int pmask = amask & m_mask[s];

#ifdef FEATURE_TEST_LINES
    Col4 m_cnst_s_(~0);
    Col4 m_grey_s_(~0);
#endif

    // create the minimal set, O(16*count/2)
    for (int i = 0, index; i < 16; ++i)
    {
      // check this pixel is enabled
      int bit = 1 << i;
      if ((pmask & bit) == 0)
      {
        if ((amask & bit) == 0)
          m_remap[s][i] = -1;

        continue;
      }

      // calculate point's weights
      Weight<std::uint8_t> wa(rgba, i, wgtx);

      // loop over previous matches for a match
      std::uint8_t* rgbvalue = &rgbx[4 * i + 0];
      for (index = 0; index < m_count[s]; ++index)
      {
        std::uint8_t* crgbvalue = &rgbx[4 * index + 0];

        // check for a match
        if (*((int*)rgbvalue) == *((int*)crgbvalue))
        {
          // get the index of the match
          assume(index >= 0 && index < 16);

          // map to this point and increase the weight
          m_remap[s][i] = (char)index;
          m_weights[s][index] += wa.GetWeights();
          m_unweighted[s] = false;
          break;
        }
      }

      // re-use the memory of already processed pixels
      assert(index <= i);
      {
        std::uint8_t* crgbvalue = &rgbx[4 * index + 0];

        // allocate a new point
        if (index == m_count[s])
        {
          // get the index of the match and advance
          m_count[s] = index + 1;

          // normalize coordinates to [0,1]
          const float* r = &caLUTs[0][rgbvalue[0]];
          const float* g = &caLUTs[1][rgbvalue[1]];
          const float* b = &caLUTs[2][rgbvalue[2]];
          const float* a = &caLUTs[3][rgbvalue[3]];

          // add the point
          m_remap[s][i] = (char)index;
          m_points[s][index] = Vec4(r, g, b, a);
          m_weights[s][index] = wa.GetWeights();
          m_unweighted[s] = m_unweighted[s] & wa.IsOne();

          // remember match for successive checks
          *((int*)crgbvalue) = *((int*)rgbvalue);

#ifdef FEATURE_TEST_LINES
          // if -1, all bytes are identical, checksum to check which bytes flip
          m_cnst_s_ &= CompareAllEqualTo_M4(m_points[s][index], m_points[s][0]);
          m_grey_s_ &= CompareAllEqualTo_M4(m_points[s][index],
                                            RotateLeft<1>(m_points[s][index]));

          //    m_cnst[s] |= (*((int *)rgbx    ) ^ (*((int *)rgbvalue)) >> 0);
          //    m_grey[s] |= (*((int *)rgbvalue) ^ (*((int *)rgbvalue)) >> 8);
#endif
        }
      }
    }

#ifdef FEATURE_IGNORE_ALPHA0
    if ((amask != 0xFFFF))
    {
      // non-separate alpha doesn't have rotations
      if (!m_rotid)
      {
        // a) there are no colors in the set, just add one
        // a) there are no color-alphas in the set, just add one
        // b) there don't exist a alpha == 0 in the set, just add one
        if ((m_count[s] == 0) | (!m_seperatealpha & m_transparent))
        {
          Vec4 sum = Vec4(0.0f);
          int num = 0;
          int p = m_count[s];

          for (int i = 0, imask = amask; i < 16; ++i, imask >>= 1)
          {
            /* assign blanked out pixels when weighting
             */
            if ((imask & 1) == 0)
            {
              m_remap[s][i] = static_cast<std::uint8_t>(p);

              std::uint8_t* rgbvalue = &rgbx[4 * i + 0];

              // normalize coordinates to [0,1]
              const float* r = &caLUTs[0][rgbvalue[0]];
              const float* g = &caLUTs[1][rgbvalue[1]];
              const float* b = &caLUTs[2][rgbvalue[2]];
              const float* a = &caLUTs[3][rgbvalue[3]];

              sum += Vec4(r, g, b, a);
              num += 1;
            }
          }

          sum /= num;
          if (!m_seperatealpha & m_transparent)
            sum = KillW(sum);

          // calculate point's weights
          Weight<std::uint8_t> wa(0, wgtx);

          // add the point
          m_count[s]++;
          m_points[s][p] = sum;
          m_weights[s][p] = wa.GetWeights() * num;
          m_unweighted[s] = m_unweighted[s] & wa.IsOne() & (num == 1);

#ifdef FEATURE_TEST_LINES
          // if -1, all bytes are identical, checksum to check which bytes flip
          m_cnst_s_ &= CompareAllEqualTo_M4(sum, m_points[s][0]);
          m_grey_s_ &= CompareAllEqualTo_M4(sum, RotateLeft<1>(sum));
#endif
        }
      }
      else
      {
        int hmpf = 0;
        hmpf = 0;
      }
    }
#endif

#ifdef FEATURE_TEST_LINES
    m_cnst[s] = ~m_cnst_s_.GetM8();
    m_grey[s] = ~m_grey_s_.GetM8();
#endif

#ifdef FEATURE_WEIGHTS_ROOTED
    // square root the weights
    for (int i = 0; i < m_count[s]; ++i)
      m_weights[s][i] = Sqrt(m_weights[s][i]);
#endif

    // TODO: if not m_transparent this all becomes a constant!
    if (m_seperatealpha)
    {
      // the alpha-set (in theory we can do separate alpha + separate
      // partitioning, but's not codeable)
      int a = s + m_numsets;

      // don't ignore 0-alphas for alphas
      int amask2 = mask;
      // combined alpha, exclusion and selection mask
      int pmask2 = amask2 & m_mask[a];

      // create the minimal set
      for (int i = 0, index; i < 16; ++i)
      {
        // check this pixel is enabled
        int bit = 1 << i;
        if ((pmask2 & bit) == 0)
        {
          if ((amask & bit) == 0)
            m_remap[a][i] = -1;

          continue;
        }

        // calculate point's weights
        Weight<std::uint8_t> wa(rgba, i, ___w);

        // loop over previous matches for a match
        std::uint8_t* avalue = &___a[1 * i + 0];
        for (index = 0; index < m_count[a]; ++index)
        {
          std::uint8_t* cavalue = &___a[1 * index + 0];

          // check for a match
          if (*avalue == *cavalue)
          {
            // get the index of the match
            assume(index >= 0 && index < 16);

            // map to this point and increase the weight
            m_remap[a][i] = (char)index;
            m_weights[a][index] += wa.GetWeights();
            m_unweighted[a] = false;
            break;
          }
        }

        // re-use the memory of already processed pixels
        assert(index <= i);
        {
          std::uint8_t* cavalue = &___a[1 * index + 0];

          // allocate a new point
          if (index == m_count[a])
          {
            // get the index of the match and advance
            m_count[a] = index + 1;

            // normalize coordinates to [0,1]
            const float* c = &caLUTs[3][avalue[0]];

            // add the point
            m_remap[a][i] = (char)index;
            m_points[a][index] = Vec4(c);
            m_weights[a][index] = wa.GetWeights();
            m_unweighted[s] = m_unweighted[s] & wa.IsOne();

            // remember match for successive checks
            *cavalue = *avalue;
          }
        }
      }

#ifdef FEATURE_IGNORE_ALPHA0
      if ((amask2 != 0xFFFF))
      {
        // separate alpha does have rotations
        {
          // a) there are no colors in the set, just add one
          if ((m_count[a] == 0) | (!m_rotid & m_transparent))
          {
            Vec4 sum = Vec4(0.0f);
            int num = 0;
            int p = m_count[a];

            for (int i = 0, imask = amask2; i < 16; ++i, imask >>= 1)
            {
              /* assign blanked out pixels when weighting
               */
              if ((imask & 1) == 0)
              {
                m_remap[a][i] = static_cast<std::uint8_t>(p);

                std::uint8_t* avalue = &___a[1 * i + 0];

                // normalize coordinates to [0,1]
                const float* c = &caLUTs[3][avalue[0]];

                sum += Vec4(c);
                num += 1;
              }
            }

            sum /= num;
            if (!m_rotid && m_transparent)
              sum = Vec4(0.0f);

            // calculate point's weights
            Weight<std::uint8_t> wa(0, ___w);

            // add the point
            m_count[a]++;
            m_points[a][p] = sum;
            m_weights[a][p] = wa.GetWeights() * num;
            m_unweighted[a] = m_unweighted[a] & wa.IsOne() & (num == 1);
          }
          else
          {
            int hmpf = 0;
            hmpf = 0;
          }
        }
      }
#endif

#ifdef FEATURE_TEST_LINES
      m_cnst[a] = 0;
      m_grey[a] = 0;
#endif

#ifdef FEATURE_WEIGHTS_ROOTED
      // square root the weights
      for (int i = 0; i < m_count[a]; ++i)
        m_weights[a][i] = Sqrt(m_weights[a][i]);
#endif
    }
  }

  // clear if we're suppose to throw alway alpha
  m_transparent = m_transparent & !clearAlpha;
}

void palette_set::BuildSet(std::uint16_t const* rgba, std::uint32_t mask,
                           flags_t flags)
{
  /* TODO */
  /*abort()*/;
}

void palette_set::BuildSet(float const* rgba, std::uint32_t mask, flags_t flags)
{
  // const float *rgbLUT = ComputeGammaLUT(flags &
  //   squish_flag::option_srgb_external);
  // const float *aLUT = ComputeGammaLUT(false);

  // check the compression mode for btc
  bool const clearAlpha =
    flags & squish_flag::option_exclude_alpha_from_palette;
  bool const seperateAlpha =
    !(flags & squish_flag::option_exclude_alpha_from_palette) &&
    m_seperatealpha;
  bool const weightByAlpha =
    (flags & squish_flag::option_weight_color_by_alpha) && !m_mergedalpha;
  bool const killByAlpha = flags & squish_flag::option_weight_color_by_alpha;

  // build mapped data
  Vec4 const mska = !seperateAlpha ? Vec4(1.0f) : Vec4(1.0f, 1.0f, 1.0f, 0.0f);
  Vec4 const clra = !clearAlpha ? Vec4(0.0f) : Vec4(0.0f, 0.0f, 0.0f, 1.0f);
  Scr4 const wgta = weightByAlpha ? Scr4(0.0f) : Scr4(1.0f);
  Scr4 const klla = killByAlpha ? Scr4(0.0f) : Scr4(1.0f);

  Vec4 rgbx[16];
  Scr4 wgtx = wgta;
  Scr4 ___a[16], ___w = Scr4(1.0f);
  int amask = 0;

  /* Apply the component rotation, while preserving semantics:
   * - swap: aa, ra, ga, ba
   * - LUTs: always pull from the right transform
   * - weights: never make alpha weighted by itself
   *   TODO: as we have no separate component weighting currently we
   *         have to turn alpha-weighting off to allow the cluster-fit
   *         to work (same weight for a AGB-point fe., R-point is ok)
   */
  // int rot[4] = {0,1,2,3};
  // const float *caLUTs[] = {
  //  rgbLUT, rgbLUT, rgbLUT, aLUT};

  switch (m_rotid)
  {
  case 1:
    wgtx = Scr4(1.0f);
    ___w = wgta;
    break;
  case 2:
    wgtx = Scr4(1.0f);
    ___w = wgta;
    break;
  case 3:
    wgtx = Scr4(1.0f);
    ___w = wgta;
    break;
    //  default: wgtx = wgta; ___w = Scr4(1.0f); break;
  }

  for (int i = 0; i < 16; ++i)
  {
    Vec4 temp;
    LoadUnaligned(temp, &rgba[4 * i]);

#ifdef FEATURE_IGNORE_ALPHA0
    // kill color
    amask += CompareFirstLessEqualTo(Max(temp.SplatW(), klla), Vec4(0.0f)) << i;
#endif

    // clear alpha
    temp = Max(temp, clra);

    // check for transparency (after blanking out)
    m_transparent =
      m_transparent | !!CompareFirstLessThan(temp.SplatW(), Vec4(1.0f));

    // swap channel-weights
    switch (m_rotid)
    {
    case 1:
      temp = Exchange<0, 3>(temp);
      break;
    case 2:
      temp = Exchange<1, 3>(temp);
      break;
    case 3:
      temp = Exchange<2, 3>(temp);
      break;
      //    default: temp = Exchange<3,3>(temp); break;
    }

    // clear channel 3
    rgbx[i] = Min(temp, mska);

    // separate channel 3
    ___a[i] = Scr4(temp.SplatW());

    // temporary, TODO: remove it
    m_weights[3][i] = Weight<float>(rgba, i, Scr4(0.0f)).GetWeights();
  }

  // combined mask
  amask = mask & (~amask);

  // clean initial state
  m_count[0] = m_count[1] = m_count[2] = m_count[3] = 0;
  m_unweighted[0] = m_unweighted[1] = m_unweighted[2] = m_unweighted[3] = true;

  // TODO: should not be necessary (VC bug?)
  m_remap.fill({0x00, 0x00, 0x00, 0x00});

  // required for being able to reorder the contents of "rgbx"
  assert(m_numsets == 1);
  for (int s = 0; s < m_numsets; s++)
  {
    // combined alpha, exclusion and selection mask
    int pmask = amask & m_mask[s];

#ifdef FEATURE_TEST_LINES
    Col4 m_cnst_s_(~0);
    Col4 m_grey_s_(~0);
#endif

    // create the minimal set, O(16*count/2)
    for (int i = 0, index; i < 16; ++i)
    {
      // check this pixel is enabled
      int bit = 1 << i;
      if ((pmask & bit) == 0)
      {
        if ((amask & bit) == 0)
          m_remap[s][i] = -1;

        continue;
      }

      // calculate point's weights
      Weight<float> wa(rgba, i, wgtx);

      // loop over previous matches for a match
      Vec4* rgbvalue = &rgbx[i];
      for (index = 0; index < m_count[s]; ++index)
      {
        Vec4* crgbvalue = &rgbx[index];

        // check for a match
        if (CompareAllEqualTo(*(rgbvalue), *(crgbvalue)))
        {
          // get the index of the match
          assume(index >= 0 && index < 16);

          // map to this point and increase the weight
          m_remap[s][i] = (char)index;
          m_weights[s][index] += wa.GetWeights();
          m_unweighted[s] = false;
          break;
        }
      }

      // re-use the memory of already processed pixels
      assert(index <= i);
      {
        Vec4* crgbvalue = &rgbx[index];

        // allocate a new point
        if (index == m_count[s])
        {
          // get the index of the match and advance
          m_count[s] = index + 1;

          // normalize coordinates to [0,1]
          //    const float *r = &caLUTs[0][rgbvalue[0]];
          //    const float *g = &caLUTs[1][rgbvalue[1]];
          //    const float *b = &caLUTs[2][rgbvalue[2]];
          //    const float *a = &caLUTs[3][rgbvalue[3]];

          // add the point
          m_remap[s][i] = (char)index;
          m_points[s][index] = *(rgbvalue);
          m_weights[s][index] = wa.GetWeights();
          m_unweighted[s] = m_unweighted[s] & wa.IsOne();

          // remember match for successive checks
          *(crgbvalue) = *(rgbvalue);

#ifdef FEATURE_TEST_LINES
          // if -1, all bytes are identical, checksum to check which bytes flip
          m_cnst_s_ &= CompareAllEqualTo_M4(m_points[s][index], m_points[s][0]);
          m_grey_s_ &= CompareAllEqualTo_M4(m_points[s][index],
                                            RotateLeft<1>(m_points[s][index]));

          //    m_cnst[s] |= (*((int *)rgbx    ) ^ (*((int *)rgbvalue)) >> 0);
          //    m_grey[s] |= (*((int *)rgbvalue) ^ (*((int *)rgbvalue)) >> 8);
#endif
        }
      }
    }

#ifdef FEATURE_IGNORE_ALPHA0
    if ((amask != 0xFFFF))
    {
      // non-separate alpha doesn't have rotations
      if (!m_rotid)
      {
        // a) there are no colors in the set, just add one
        // a) there are no color-alphas in the set, just add one
        // b) there don't exist a alpha == 0 in the set, just add one
        if ((m_count[s] == 0) | (!m_seperatealpha & m_transparent))
        {
          Vec4 sum = Vec4(0.0f);
          int num = 0;
          int p = m_count[s];

          for (int i = 0, imask = amask; i < 16; ++i, imask >>= 1)
          {
            /* assign blanked out pixels when weighting
             */
            if ((imask & 1) == 0)
            {
              m_remap[s][i] = static_cast<std::uint8_t>(p);

              Vec4* rgbvalue = &rgbx[i];

              // normalize coordinates to [0,1]
              //        const float *r = &caLUTs[0][rgbvalue[0]];
              //        const float *g = &caLUTs[1][rgbvalue[1]];
              //        const float *b = &caLUTs[2][rgbvalue[2]];
              //        const float *a = &caLUTs[3][rgbvalue[3]];

              sum += *(rgbvalue);
              num += 1;
            }
          }

          sum /= num;
          if (!m_seperatealpha & m_transparent)
            sum = KillW(sum);

          // calculate point's weights
          Weight<float> wa(0.0f, wgtx);

          // add the point
          m_count[s]++;
          m_points[s][p] = sum;
          m_weights[s][p] = wa.GetWeights() * num;
          m_unweighted[s] = m_unweighted[s] & wa.IsOne() & (num == 1);

#ifdef FEATURE_TEST_LINES
          // if -1, all bytes are identical, checksum to check which bytes flip
          m_cnst_s_ &= CompareAllEqualTo_M4(sum, m_points[s][0]);
          m_grey_s_ &= CompareAllEqualTo_M4(sum, RotateLeft<1>(sum));
#endif
        }
      }
      else
      {
        int hmpf = 0;
        hmpf = 0;
      }
    }
#endif

#ifdef FEATURE_TEST_LINES
    m_cnst[s] = ~m_cnst_s_.GetM8();
    m_grey[s] = ~m_grey_s_.GetM8();
#endif

#ifdef FEATURE_WEIGHTS_ROOTED
    // square root the weights
    for (int i = 0; i < m_count[s]; ++i)
      m_weights[s][i] = Sqrt(m_weights[s][i]);
#endif

    // TODO: if not m_transparent this all becomes a constant!
    if (m_seperatealpha)
    {
      // the alpha-set (in theory we can do separate alpha + separate
      // partitioning, but's not codeable)
      int a = s + m_numsets;

      // don't ignore 0-alphas for alphas
      int amask2 = mask;
      // combined alpha, exclusion and selection mask
      int pmask2 = amask2 & m_mask[a];

      // create the minimal set
      for (int i = 0, index; i < 16; ++i)
      {
        // check this pixel is enabled
        int bit = 1 << i;
        if ((pmask2 & bit) == 0)
        {
          if ((amask2 & bit) == 0)
            m_remap[a][i] = -1;

          continue;
        }

        // calculate point's weights
        Weight<float> wa(rgba, i, ___w);

        // loop over previous matches for a match
        Scr4* avalue = &___a[i];
        for (index = 0; index < m_count[a]; ++index)
        {
          Scr4* cavalue = &___a[index];

          // check for a match
          if (*avalue == *cavalue)
          {
            // get the index of the match
            assume(index >= 0 && index < 16);

            // map to this point and increase the weight
            m_remap[a][i] = (char)index;
            m_weights[a][index] += wa.GetWeights();
            m_unweighted[a] = false;
            break;
          }
        }

        // re-use the memory of already processed pixels
        assert(index <= i);
        {
          Scr4* cavalue = &___a[index];

          // allocate a new point
          if (index == m_count[a])
          {
            // get the index of the match and advance
            m_count[a] = index + 1;

            // normalize coordinates to [0,1]
            //      const float *c = &caLUTs[3][avalue[0]];

            // add the point
            m_remap[a][i] = (char)index;
            m_points[a][index] = *(avalue);
            m_weights[a][index] = wa.GetWeights();
            m_unweighted[s] = m_unweighted[s] & wa.IsOne();

            // remember match for successive checks
            *cavalue = *avalue;
          }
        }
      }

#ifdef FEATURE_IGNORE_ALPHA0
      if ((amask2 != 0xFFFF))
      {
        // separate alpha does have rotations
        {
          // a) there are no colors in the set, just add one
          if ((m_count[a] == 0) | (!m_rotid & m_transparent))
          {
            Vec4 sum = Vec4(0.0f);
            int num = 0;
            int p = m_count[a];

            for (int i = 0, imask = amask2; i < 16; ++i, imask >>= 1)
            {
              /* assign blanked out pixels when weighting
               */
              if ((mask & 1) == 0)
              {
                m_remap[a][i] = static_cast<std::uint8_t>(p);

                Scr4* avalue = &___a[i];

                // normalize coordinates to [0,1]
                //    const float *c = &caLUTs[3][avalue[0]];

                sum += *(avalue);
                num += 1;
              }
            }

            sum /= num;
            if (!m_rotid && m_transparent)
              sum = Vec4(0.0f);

            // calculate point's weights
            Weight<float> wa(0.0f, ___w);

            // add the point
            m_count[a]++;
            m_points[a][p] = sum;
            m_weights[a][p] = wa.GetWeights() * num;
            m_unweighted[a] = m_unweighted[a] & wa.IsOne() & (num == 1);
          }
          else
          {
            int hmpf = 0;
            hmpf = 0;
          }
        }
      }
#endif

#ifdef FEATURE_TEST_LINES
      m_cnst[a] = 0;
      m_grey[a] = 0;
#endif

#ifdef FEATURE_WEIGHTS_ROOTED
      // square root the weights
      for (int i = 0; i < m_count[a]; ++i)
        m_weights[a][i] = Sqrt(m_weights[a][i]);
#endif
    }
  }

  // clear if we're suppose to throw alway alpha
  m_transparent = m_transparent & !clearAlpha;
}

void palette_set::BuildSet(palette_set const& palette, std::uint32_t mask,
                           flags_t flags)
{
  // can't be permuted
  assert(m_seperatealpha == true);

  // check the compression mode for btc
  bool const weightByAlpha = flags & squish_flag::option_weight_color_by_alpha;

  // build mapped data
  Vec4 const wgta = weightByAlpha ? Vec4(0.0f) : Vec4(1.0f);

  Vec4 wgtx = wgta;
  Vec4 ___w = Vec4(1.0f);

  // clean initial state
  m_count[0] = m_count[1] = m_count[2] = m_count[3] = 0;
  m_unweighted[0] = m_unweighted[1] = m_unweighted[2] = m_unweighted[3] = true;

  // the alpha-set (in theory we can do separate alpha + separate partitioning,
  // but's not codeable)
  int s = 0;
  int a = 1;
  {
#ifdef FEATURE_TEST_LINES
    Col4 m_cnst_s_(~0);
    Col4 m_grey_s_(~0);
#endif

    // create the minimal set, O(16*count/2)
    for (int i = 0, index; i < 16; ++i)
    {
      // copy "unset"
      int cindex = palette.m_remap[0][i];
      int aindex = palette.m_remap[1][i];

      /*
      assert(
  ((cindex == -1) && (aindex == -1)) ||
  ((cindex != -1) && (aindex != -1))
      );
       */

      if ((cindex == -1) && (aindex == -1))
      {
        m_remap[s][i] = -1;
        m_remap[a][i] = -1;
        continue;
      }

      // TODO: kill off alpha will kill the weighting
      // TODO: select the most probable instead
      Vec4 rgbx = palette.m_points[0][cindex >= 0 ? cindex : 0];
      Vec4 ___a = palette.m_points[1][aindex >= 0 ? aindex : 0];
      Vec4 rgba = TransferW(rgbx, ___a);

      switch (m_rotid)
      {
      case 1:
        rgba = Exchange<0, 3>(rgba);
        wgtx = Vec4(1.0f);
        ___w = wgta;
        break;
      case 2:
        rgba = Exchange<1, 3>(rgba);
        wgtx = Vec4(1.0f);
        ___w = wgta;
        break;
      case 3:
        rgba = Exchange<2, 3>(rgba);
        wgtx = Vec4(1.0f);
        ___w = wgta;
        break;
        //  default: rgba = Exchange<3, 3>(rgba); wgtx = wgta; ___w =
        //  Vec4(1.0f); break;
      }

      rgbx = KillW(rgba);

      // calculate point's weights
      Weight<Scr4> wa(palette.m_weights, i, wgtx);

      // loop over previous matches for a match
      for (index = 0; index < m_count[s]; ++index)
      {
        if (CompareAllEqualTo(rgbx, m_points[s][index]))
        {
          // get the index of the match
          assume(index >= 0 && index < 16);

          // map to this point and increase the weight
          m_remap[s][i] = (char)index;
          m_weights[s][index] += wa.GetWeights();
          m_unweighted[s] = false;
          break;
        }
      }

      assert(index <= i);
      {
        // allocate a new point
        if (index == m_count[s])
        {
          // get the index of the match and advance
          m_count[s] = index + 1;

          // add the point
          m_remap[s][i] = (char)index;
          m_points[s][index] = rgbx;
          m_weights[s][index] = wa.GetWeights();
          m_unweighted[s] = m_unweighted[s] & wa.IsOne();

#ifdef FEATURE_TEST_LINES
          // if -1, all bytes are identical, checksum to check which bytes flip
          m_cnst_s_ &= CompareAllEqualTo_M4(rgbx, m_points[s][0]);
          m_grey_s_ &= CompareAllEqualTo_M4(rgbx, RotateLeft<1>(rgbx));
#endif
        }
      }

      // TODO: if alpha==0 don't add this color
      {
        ___a = rgba.SplatW();

        // calculate point's weights
        Weight<Scr4> aw(palette.m_weights, i, ___w);

        // loop over previous matches for a match
        for (index = 0; index < m_count[a]; ++index)
        {
          if (CompareAllEqualTo(___a, m_points[a][index]))
          {
            // get the index of the match
            assume(index >= 0 && index < 16);

            // map to this point and increase the weight
            m_remap[a][i] = (char)index;
            m_weights[a][index] += aw.GetWeights();
            m_unweighted[a] = false;
            break;
          }
        }

        assert(index <= i);
        {
          // allocate a new point
          if (index == m_count[a])
          {
            // get the index of the match and advance
            m_count[a] = index + 1;

            // add the point
            m_remap[a][i] = (char)index;
            m_points[a][index] = ___a;
            m_weights[a][index] = aw.GetWeights();
            m_unweighted[a] = m_unweighted[a] & aw.IsOne();
          }
        }
      }
    }

#ifdef FEATURE_TEST_LINES
    m_cnst[s] = ~m_cnst_s_.GetM8();
    m_cnst[a] = 0;
    m_grey[s] = ~m_grey_s_.GetM8();
    m_grey[a] = 0;
#endif

#ifdef FEATURE_WEIGHTS_ROOTED
    // square root the weights
    for (int i = 0; i < m_count[s]; ++i)
      m_weights[s][i] = Sqrt(m_weights[s][i]);
    for (int i = 0; i < m_count[a]; ++i)
      m_weights[a][i] = Sqrt(m_weights[a][i]);
#endif
  }

  // clear if we're suppose to throw alway alpha
  m_transparent = palette.m_transparent;
}

void palette_set::PermuteSet(palette_set const& palette, std::uint32_t mask,
                             flags_t flags)
{
  // can't be permuted
  assert(m_seperatealpha == false);

  // check the compression mode for btc
  bool const weightByAlpha =
    (flags & squish_flag::option_weight_color_by_alpha) && !m_mergedalpha;

  // build mapped data
  Vec4 const wgta = weightByAlpha ? Vec4(0.0f) : Vec4(1.0f);

  // clean initial state
  m_count[0] = m_count[1] = m_count[2] = m_count[3] = 0;
  m_unweighted[0] = m_unweighted[1] = m_unweighted[2] = m_unweighted[3] = true;

  for (int s = 0; s < m_numsets; s++)
  {
    // selection mask
    int pmask = mask & m_mask[s];

    // record mappings (multi-assignment possible)
    SQUISH_ALIGNED char gotcha[16];
    memset(gotcha, -1, sizeof(gotcha));

#ifdef FEATURE_TEST_LINES
    Col4 m_cnst_s_(~0);
    Col4 m_grey_s_(~0);
#endif

    // create the minimal set
    for (int i = 0, imask = pmask; i < 16; ++i, imask >>= 1)
    {
      // check this pixel is enabled
      if ((imask & 1) == 0)
        continue;

      // copy "unset"
      int uindex = palette.m_remap[0][i];
      if (uindex == -1)
      {
        m_remap[s][i] = -1;
        continue;
      }

      // TODO: kill off alpha will kill the weighting
      Vec4 rgba = palette.m_points[0][uindex];

      // calculate point's weights
      Weight<Scr4> wa(palette.m_weights, i, wgta);

      int index;
      if ((index = gotcha[uindex]) >= 0)
      {
        // get the index of the match
        assume(index >= 0 && index < 16);

        // map to this point and increase the weight
        m_remap[s][i] = (char)index;
        m_weights[s][index] += wa.GetWeights();
        m_unweighted[s] = false;
        continue;
      }

      {
        // get the index of the match and advance
        gotcha[uindex] = (char)(index = m_count[s]++);

        // add the point
        m_remap[s][i] = (char)index;
        m_points[s][index] = rgba;
        m_weights[s][index] = wa.GetWeights();
        m_unweighted[s] = m_unweighted[s] & wa.IsOne();

#ifdef FEATURE_TEST_LINES
        // if -1, all bytes are identical, checksum to check which bytes flip
        m_cnst_s_ &= CompareAllEqualTo_M4(rgba, m_points[s][0]);
        m_grey_s_ &= CompareAllEqualTo_M4(rgba, RotateLeft<1>(rgba));
#endif
      }
    }

#ifdef FEATURE_TEST_LINES
    m_cnst[s] = ~m_cnst_s_.GetM8();
    m_grey[s] = ~m_grey_s_.GetM8();
#endif

#ifdef FEATURE_WEIGHTS_ROOTED
    // square root the weights
    for (int i = 0; i < m_count[s]; ++i)
      m_weights[s][i] = Sqrt(m_weights[s][i]);
    for (int i = 0; i < m_count[a]; ++i)
      m_weights[a][i] = Sqrt(m_weights[a][i]);
#endif
  }

  // clear if we're suppose to throw alway alpha
  m_transparent = palette.m_transparent;

#ifdef FEATURE_IGNORE_ALPHA0
  // if there are no entries in a set, just
  // assign the first of another set
  for (int s = 0; s < m_numsets; s++)
  {
    if (!m_count[s])
    {
      for (int t = 0; t < m_numsets; t++)
      {
        if (m_count[t] && (t != s))
        {
          Vec4 pnt = m_points[t][0];
          if (m_transparent)
            pnt = KillW(pnt);

          m_count[s] = 1;
          m_points[s][0] = pnt;
          m_weights[s][0] = Scr4(1.0f);
          m_unweighted[s] = false;

#ifdef FEATURE_TEST_LINES
          // if -1, all bytes are identical, checksum to check which bytes flip
          m_cnst[s] = 0;
          m_grey[s] = 0;
#endif
        }
      }
    }
  }
#endif
}

void palette_set::RemapIndices(std::uint8_t const* source, std::uint8_t* target,
                               int set) const
{
  const int s = set;
  {
    // selection mask
    int pmask = m_mask[s];

    for (int i = 0, imask = pmask; i < 16; ++i, imask >>= 1)
    {
      // check this pixel is enabled
      if ((imask & 1) == 0)
        continue;

      std::uint8_t t = 0;
      t = ((m_remap[s][i] == -1) ? t : source[m_remap[s][i]]);
      target[i] = t;
    }
  }
}

void palette_set::UnmapIndices(std::uint8_t const* source,
                               std::uint8_t* destination, int set,
                               unsigned int* codes, int cmask) const
{
  const int s = set;
  {
    // selection mask
    int pmask = m_mask[s];

    for (int i = 0, imask = pmask; i < 16; ++i, imask >>= 1)
    {
      // check this pixel is enabled
      if ((imask & 1) == 0)
        continue;

      if ((cmask >> 0) & 0xFF)
      {
        destination[4 * i + 0] =
          static_cast<std::uint8_t>(codes[source[i]] >> 0);
      }
      if ((cmask >> 8) & 0xFF)
      {
        destination[4 * i + 1] =
          static_cast<std::uint8_t>(codes[source[i]] >> 8);
      }
      if ((cmask >> 16) & 0xFF)
      {
        destination[4 * i + 2] =
          static_cast<std::uint8_t>(codes[source[i]] >> 16);
      }
      if ((cmask >> 24) & 0xFF)
      {
        destination[4 * i + 3] =
          static_cast<std::uint8_t>(codes[source[i]] >> 24);
      }
    }
  }
}
}  // namespace squish
