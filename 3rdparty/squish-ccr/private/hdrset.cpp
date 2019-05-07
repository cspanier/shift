/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels Fröhling              niels@paradice-insight.us
  Copyright (c) 2019 Christian Spanier                     github@boxie.eu

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to	deal in the Software without restriction, including
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

#include <cassert>
#include "hdrset.h"

namespace squish
{

// Associated to partition -1, 16 * 0 bit
extern const std::uint16_t partitionmasks_1[1];

// Associated to partition 0-63, 16 * 1 bit
extern const std::uint16_t partitionmasks_2[64];

// Associated to partition 64-127, 16 * 2 bit
extern const unsigned int partitionmasks_3[64];

void hdr_set::GetMasks(int flags, int partition, int (&masks)[2])
{
  unsigned int partmask = 0;
  if (((flags & kVariableCodingModes) >= kVariableCodingMode1) &&
      ((flags & kVariableCodingModes) <= kVariableCodingMode10))
    partmask = partitionmasks_2[partition];

  // determine the number of partitions
  if (((flags & kVariableCodingModes) >= kVariableCodingMode1) &&
      ((flags & kVariableCodingModes) <= kVariableCodingMode10))
    masks[0] = (~partmask & 0xFFFF) & (0xFFFFFFFF >> 16),
    masks[1] = (partmask & 0xFFFF) & (0xFFFFFFFF >> 16);
}

int hdr_set::SetMode(int flags)
{
  /* build a single set only, we permute that later for specific partitions,
   * separate alpha is an exception as that is fixed for each mode
   */
  if (((flags & kVariableCodingModes) >= kVariableCodingMode1) &&
      ((flags & kVariableCodingModes) <= kVariableCodingMode10))
    m_numsets = 2, m_partmask = 0xFFFFFFFF, m_partid = 0;

  // partition_1 mask is: bit cleared -> set 1, bit set -> set 2
  // partition_2 mask is: bit cleared -> set 1, bit set -> set 2, hi bit set ->
  // set 3
  if (1)
    m_mask[0] = 0xFFFF,  // color-set
      m_mask[1] = 0xFFFF;
  if (m_numsets > 1)
    m_numsets = 1;

  return flags;
}

int hdr_set::SetMode(int flags, int partition)
{
  /* determine the number of sets and select the partition
  if ((0))
    m_numsets = 1, m_partmask = partitionmasks_1[m_partid = 0]; */
  if (((flags & kVariableCodingModes) >= kVariableCodingMode1) &&
      ((flags & kVariableCodingModes) <= kVariableCodingMode10))
    m_numsets = 2, m_partmask = partitionmasks_2[m_partid = partition];

  // partition_1 mask is: bit cleared -> set 1, bit set -> set 2
  // partition_2 mask is: bit cleared -> set 1, bit set -> set 2, hi bit set ->
  // set 3
  if (m_numsets == 1)
    m_mask[0] = (m_partmask & 0xFFFF) & (0xFFFFFFFF >> 16),
    m_mask[1] = (m_partmask & 0xFFFF) & (0xFFFFFFFF >> 16);
  if (m_numsets == 2)
    m_mask[0] = (~m_partmask & 0xFFFF) & (0xFFFFFFFF >> 16),
    m_mask[1] = (m_partmask & 0xFFFF) & (0xFFFFFFFF >> 16);

  return flags;
}

hdr_set::hdr_set(std::uint16_t const* rgb, int mask, int flags)
: m_numsets(1), m_partid(0), m_partmask(0xFFFF)
{
  // make the set (if successive partition permutation)
  BuildSet(rgb, mask, SetMode(flags));
}

hdr_set::hdr_set(std::uint16_t const* rgb, int mask, int flags, int partition)
: m_numsets(1), m_partid(0), m_partmask(0xFFFF)
{
  // make the set
  BuildSet(rgb, mask, SetMode(flags, partition));
}

hdr_set::hdr_set(float const* rgb, int mask, int flags)
: m_numsets(1), m_partid(0), m_partmask(0xFFFF)
{
  // make the set (if successive partition permutation)
  BuildSet(rgb, mask, SetMode(flags));
}

hdr_set::hdr_set(float const* rgb, int mask, int flags, int partition)
: m_numsets(1), m_partid(0), m_partmask(0xFFFF)
{
  // make the set
  BuildSet(rgb, mask, SetMode(flags, partition));
}

hdr_set::hdr_set(hdr_set const& palette, int mask, int flags, int partition)
: m_numsets(1), m_partid(0), m_partmask(0xFFFF)
{
  flags = SetMode(flags, partition);

  // make the new set
  if (m_numsets > 1)
    PermuteSet(palette, mask, flags);  // permutable
  else
    memcpy(this, &palette, sizeof(*this));  // identical
}

void hdr_set::BuildSet(std::uint16_t const* rgb, int mask, int flags)
{
  // const float *rgbLUT = ComputeGammaLUT((flags & kSrgbIn) != 0);
  // const float *aLUT   = ComputeGammaLUT(false);

  // check the compression mode for btc
  bool const weightByAlpha = ((flags & kWeightcolorByAlpha) != 0);

  // build mapped data
  std::uint16_t const wgta = weightByAlpha ? 0x0000 : 0xFFFF;

  std::uint16_t rgbx[4 * 16];
  std::uint16_t wgtx[1 * 16];

  for (int i = 0; i < 16; ++i)
  {
    rgbx[4 * i + 0] = rgb[4 * i + 0];
    rgbx[4 * i + 1] = rgb[4 * i + 1];
    rgbx[4 * i + 2] = rgb[4 * i + 2];
    rgbx[4 * i + 3] = 0;

    // separate channel 3
    wgtx[1 * i + 0] = rgb[4 * i + 3] | wgta;
  }

  // clean initial state
  m_count[0] = m_count[1] = 0;
  m_unweighted[0] = m_unweighted[1] = true;

  // required for being able to reorder the contents of "rgbx"
  assert(m_numsets == 1);
  for (int s = 0; s < m_numsets; s++)
  {
    // combined exclusion and selection mask
    int pmask = mask & m_mask[s];

#ifdef FEATURE_TEST_LINES
    Col3 m_cnst_s_(~0);
    Col3 m_grey_s_(~0);
#endif

    // create the minimal set, O(16*count/2)
    for (int i = 0, index; i < 16; ++i)
    {
      // check this pixel is enabled
      int bit = 1 << i;
      if ((pmask & bit) == 0)
      {
        if ((mask & bit) == 0)
          m_remap[s][i] = -1;

        continue;
      }

      // ensure there is always non-zero weight even for zero alpha
      std::uint16_t w = wgtx[1 * i + 0];
      float W = math::sqrt((float)(w + 1) / 65536.0f);

#ifdef FEATURE_IGNORE_ALPHA0
      /* check for blanked out pixels when weighting
       */
      if (!w)
      {
        m_remap[s][i] = -1;
        continue;
      }
#endif

      // loop over previous matches for a match
      std::uint16_t* rgbvalue = &rgbx[4 * i + 0];
      for (index = 0; index < m_count[s]; ++index)
      {
        std::uint16_t* crgbvalue = &rgbx[4 * index + 0];

        // check for a match
        if (*((std::int64_t*)rgbvalue) == *((std::int64_t*)crgbvalue))
        {
          // get the index of the match
          assume(index >= 0 && index < 16);

          // map to this point and increase the weight
          m_remap[s][i] = index;
          m_weights[s][index] += Scr3(W);
          m_unweighted[s] = false;
          break;
        }
      }

      // re-use the memory of already processed pixels
      assert(index <= i);
      {
        std::uint16_t* crgbvalue = &rgbx[4 * index + 0];

        // allocate a new point
        if (index == m_count[s])
        {
          // get the index of the match and advance
          m_count[s] = index + 1;

          // normalize coordinates to [0,1]
          const float r = rgbvalue[0];
          const float g = rgbvalue[1];
          const float b = rgbvalue[2];

          // add the point
          m_remap[s][i] = index;
          m_points[s][index] = Vec3(&r, &g, &b) / 65535.0f;
          m_weights[s][index] = Scr3(W);
          m_unweighted[s] = m_unweighted[s] && !(std::uint16_t)(~w);

          // remember match for successive checks
          *((std::int64_t*)crgbvalue) = *((std::int64_t*)rgbvalue);

#ifdef FEATURE_TEST_LINES
          // if -1, all bytes are identical, checksum to check which bytes flip
          m_cnst_s_ &= CompareAllEqualTo_M8(m_points[s][index], m_points[s][0]);
          m_grey_s_ &= CompareAllEqualTo_M8(m_points[s][index],
                                            RotateLeft<1>(m_points[s][index]));

//	  m_cnst[s] |= (*((int *)rgbx    ) ^ (*((int *)rgbvalue)) >> 0);
//	  m_grey[s] |= (*((int *)rgbvalue) ^ (*((int *)rgbvalue)) >> 8);
#endif
        }
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
#endif
  }
}

void hdr_set::BuildSet(float const* rgb, int mask, int flags)
{
  // const float *rgbLUT = ComputeGammaLUT((flags & kSrgbIn) != 0);
  // const float *aLUT   = ComputeGammaLUT(false);

  // check the compression mode for btc
  bool const weightByAlpha = ((flags & kWeightcolorByAlpha) != 0);

  // build mapped data
  Scr4 wgta = weightByAlpha ? Scr4(0.0f) : Scr4(1.0f);

  Vec3 rgbx[16];
  Scr3 wgtx[16];

  for (int i = 0; i < 16; ++i)
  {
    Vec4 rgbl;
    LoadUnaligned(rgbl, &rgb[4 * i]);

    // clear alpha
    rgbx[i] = Max(KillW(rgbl), Vec4(0.0f)).GetVec3();

    // separate channel 3
    wgtx[i] = Max(rgbl.SplatW(), wgta).GetVec3();
  }

  // clean initial state
  m_count[0] = m_count[1] = 0;
  m_unweighted[0] = m_unweighted[1] = true;

  // required for being able to reorder the contents of "rgbx"
  assert(m_numsets == 1);
  for (int s = 0; s < m_numsets; s++)
  {
    // combined exclusion and selection mask
    int pmask = mask & m_mask[s];

#ifdef FEATURE_TEST_LINES
    Col3 m_cnst_s_(~0);
    Col3 m_grey_s_(~0);
#endif

    // create the minimal set, O(16*count/2)
    for (int i = 0, index; i < 16; ++i)
    {
      // check this pixel is enabled
      int bit = 1 << i;
      if ((pmask & bit) == 0)
      {
        if ((mask & bit) == 0)
          m_remap[s][i] = -1;

        continue;
      }

      // ensure there is always non-zero weight even for zero alpha
      Scr3 w = wgtx[i];
      float W = w.X();

#ifdef FEATURE_IGNORE_ALPHA0
      /* check for blanked out pixels when weighting
       */
      if (!w)
      {
        m_remap[s][i] = -1;
        continue;
      }
#endif

      // loop over previous matches for a match
      Vec3* rgbvalue = &rgbx[i];
      for (index = 0; index < m_count[s]; ++index)
      {
        Vec3* crgbvalue = &rgbx[index];

        // check for a match
        if (CompareAllEqualTo((*rgbvalue), (*crgbvalue)))
        {
          // get the index of the match
          assume(index >= 0 && index < 16);

          // map to this point and increase the weight
          m_remap[s][i] = index;
          m_weights[s][index] += Scr3(W);
          m_unweighted[s] = false;
          break;
        }
      }

      // re-use the memory of already processed pixels
      assert(index <= i);
      {
        Vec3* crgbvalue = &rgbx[index];

        // allocate a new point
        if (index == m_count[s])
        {
          // get the index of the match and advance
          m_count[s] = index + 1;

#if 0
	  // normalize coordinates to [0,1]
	  const float r = rgbvalue[0];
	  const float g = rgbvalue[1];
	  const float b = rgbvalue[2];
#endif

          // add the point
          m_remap[s][i] = index;
          m_points[s][index] = *(rgbvalue);
          m_weights[s][index] = Scr3(W);
          m_unweighted[s] =
            m_unweighted[s] && !CompareFirstLessThan(w, Vec3(1.0f));

          // remember match for successive checks
          *(crgbvalue) = *(rgbvalue);

#ifdef FEATURE_TEST_LINES
          // if -1, all bytes are identical, checksum to check which bytes flip
          m_cnst_s_ &= CompareAllEqualTo_M8(m_points[s][index], m_points[s][0]);
          m_grey_s_ &= CompareAllEqualTo_M8(m_points[s][index],
                                            RotateLeft<1>(m_points[s][index]));

//	  m_cnst[s] |= (*((int *)rgbx    ) ^ (*((int *)rgbvalue)) >> 0);
//	  m_grey[s] |= (*((int *)rgbvalue) ^ (*((int *)rgbvalue)) >> 8);
#endif
        }
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
#endif
  }
}

void hdr_set::PermuteSet(hdr_set const& palette, int mask, int flags)
{
  // check the compression mode for btc
  bool const weightByAlpha = ((flags & kWeightcolorByAlpha) != 0);

  // build mapped data
  Vec4 const wgta = weightByAlpha ? Vec4(0.0f) : Vec4(1.0f);

  // clean initial state
  m_count[0] = m_count[1] = 0;
  m_unweighted[0] = m_unweighted[1] = true;

  for (int s = 0; s < m_numsets; s++)
  {
    // selection mask
    int pmask = mask & m_mask[s];

    // record mappings (multi-assignment possible)
    char gotcha[16];
    memset(gotcha, -1, sizeof(gotcha));

#ifdef FEATURE_TEST_LINES
    Col3 m_cnst_s_(~0);
    Col3 m_grey_s_(~0);
#endif

    // create the minimal set
    for (int i = 0; i < 16; ++i)
    {
      // check this pixel is enabled
      int bit = 1 << i;
      if ((pmask & bit) == 0)
        continue;

      // copy "unset"
      int uindex = palette.m_remap[0][i];
      if (uindex == -1)
      {
        m_remap[s][i] = -1;
        continue;
      }

      // TODO: kill off alpha will kill the weighting
      Vec3 rgb = palette.m_points[0][uindex];

      // ensure there is always non-zero weight even for zero alpha
      Scr3 w = Scr3(1.0f);  // Max(rgb, wgta).SplatW();
      float W = 1.0f;

      int index;
      if ((index = gotcha[uindex]) >= 0)
      {
        // get the index of the match
        assume(index >= 0 && index < 16);

        // map to this point and increase the weight
        m_remap[s][i] = index;
        m_weights[s][index] += Scr3(W);
        m_unweighted[s] = false;
        continue;
      }

      {
        // get the index of the match and advance
        index = gotcha[uindex] = (char)(m_count[s]++);

        // add the point
        m_remap[s][i] = index;
        m_points[s][index] = rgb;
        m_weights[s][index] = Scr3(W);
        m_unweighted[s] =
          m_unweighted[s] && !CompareFirstLessThan(w, Vec3(1.0f));

#ifdef FEATURE_TEST_LINES
        // if -1, all bytes are identical, checksum to check which bytes flip
        m_cnst_s_ &= CompareAllEqualTo_M8(rgb, m_points[s][0]);
        m_grey_s_ &= CompareAllEqualTo_M8(rgb, RotateLeft<1>(rgb));
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
#endif
  }
}

void hdr_set::RemapIndices(std::uint8_t const* source, std::uint8_t* target,
                          int set) const
{
  const int s = set;
  {
    // selection mask
    int pmask = m_mask[s];

    for (int i = 0; i < 16; ++i)
    {
      // check this pixel is enabled
      int bit = 1 << i;
      if ((pmask & bit) == 0)
        continue;

      std::uint8_t t = 0;
      t = ((m_remap[s][i] == -1) ? t : source[m_remap[s][i]]);
      target[i] = t;
    }
  }
}

void hdr_set::UnmapIndices(std::uint8_t const* source,
                          std::uint16_t* destination, int set, std::int64_t* codes,
                          std::int64_t cmask) const
{
  const int s = set;
  {
    // selection mask
    int pmask = m_mask[s];

    for (int i = 0; i < 16; ++i)
    {
      // check this pixel is enabled
      int bit = 1 << i;
      if ((pmask & bit) == 0)
        continue;

      if ((cmask >> 0) & 0xFFFF)
        destination[4 * i + 0] = (std::uint16_t)(codes[source[i]] >> 0);
      if ((cmask >> 16) & 0xFFFF)
        destination[4 * i + 1] = (std::uint16_t)(codes[source[i]] >> 16);
      if ((cmask >> 32) & 0xFFFF)
        destination[4 * i + 2] = (std::uint16_t)(codes[source[i]] >> 32);
      if ((cmask >> 48) & 0xFFFF)
        destination[4 * i + 3] = (std::uint16_t)(codes[source[i]] >> 48);
    }
  }
}
}
