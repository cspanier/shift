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

#ifndef SQUISH_PALETTESET_H
#define SQUISH_PALETTESET_H

#include <squish.h>
#include <memory.h>
#include "maths_all.h"

namespace squish
{
/*! @brief Represents a set of block palettes
 */
class palette_set
{
public:
  static void GetMasks(flags_t flags, int partition, int (&masks)[4]);

  int SetMode(flags_t flags);
  int SetMode(flags_t flags, int part_or_rot);

  // maximum number of different sets, aligned, the real limit is 3
#define PS_MAX 4

public:
  // constructor for regular operation (with and without initial
  // partition/rotation)
  palette_set(std::uint8_t const* rgba, std::uint32_t mask, flags_t flags);
  palette_set(std::uint8_t const* rgba, std::uint32_t mask, flags_t flags,
              int part_or_rot);

  palette_set(std::uint16_t const* rgba, std::uint32_t mask, flags_t flags);
  palette_set(std::uint16_t const* rgba, std::uint32_t mask, flags_t flags,
              int part_or_rot);

  palette_set(float const* rgba, std::uint32_t mask, flags_t flags);
  palette_set(float const* rgba, std::uint32_t mask, flags_t flags,
              int part_or_rot);

  // constructors for managing backups and permutations of palette-sets
  palette_set() = default;
  palette_set(palette_set const& palette)
  {
    memcpy(this, &palette, sizeof(*this));
  }
  palette_set(palette_set const& palette, std::uint32_t mask, flags_t flags,
              int part_or_rot);

private:
  void BuildSet(std::uint8_t const* rgba, std::uint32_t mask, flags_t flags);
  void BuildSet(std::uint16_t const* rgba, std::uint32_t mask, flags_t flags);
  void BuildSet(float const* rgba, std::uint32_t mask, flags_t flags);
  void BuildSet(palette_set const& palette, std::uint32_t mask, flags_t flags);
  void PermuteSet(palette_set const& palette, std::uint32_t mask,
                  flags_t flags);

public:
  // active attributes based on the parameters passed on initialization
  int GetSets() const
  {
    return m_numsets;
  }
  int GetRotation() const
  {
    return m_rotid;
  }
  int GetPartition() const
  {
    return m_partid;
  }

  // information determined when the palette-set has been formed
  bool IsSeperateAlpha() const
  {
    return /*m_transparent &&*/ m_seperatealpha;
  }
  bool IsMergedAlpha() const
  {
    return /*m_transparent &&*/ m_mergedalpha;
  }
  bool IsTransparent() const
  {
    return m_transparent;
  }

  bool IsUnweighted(int idx) const
  {
    return m_unweighted[idx];
  }
  const Vec4* GetPoints(int idx) const
  {
    return m_points[idx].data();
  }
  const Scr4* GetWeights(int idx) const
  {
    return m_weights[idx].data();
  }
  int GetCount(int idx) const
  {
    return m_count[idx];
  }
  int GetCount() const
  {
    return m_count[0] +
           /*(m_numsets > 0 ? m_count[0] : 0)*/
           (m_seperatealpha ? m_count[m_numsets + 0] : 0) +
           (m_numsets > 1
              ? m_count[1]
              : 0) /*(m_seperatealpha ? m_count[m_numsets + 1] : 0)*/
           + (m_numsets > 2
                ? m_count[2]
                : 0) /*(m_seperatealpha ? m_count[m_numsets + 2] : 0)*/;
  }
  int GetOptimal() const
  {
    return (m_count[0] <= 1) &
           /*(m_numsets > 0 ? m_count[0] <= 1 : 1)*/
           (m_seperatealpha ? m_count[m_numsets + 0] <= 1 : 1) &
           (m_numsets > 1
              ? m_count[1] <= 1
              : 1) /*(m_seperatealpha ? m_count[m_numsets + 1] <= 1 : 1)*/
           & (m_numsets > 2
                ? m_count[2] <= 1
                : 1) /*(m_seperatealpha ? m_count[m_numsets + 2] <= 1 : 1)*/;
  }

  // map from the set to indices and back to colors
  void RemapIndices(std::uint8_t const* source, std::uint8_t* target,
                    int set) const;
  void UnmapIndices(std::uint8_t const* source, std::uint8_t* rgba, int set,
                    unsigned int* codes, int cmask) const;

private:
  int m_numsets;
  int m_rotid;
  int m_partid;
  int m_partmask;
  bool m_seperatealpha;
  bool m_mergedalpha;

  bool m_transparent;
  std::array<bool, 4> m_unweighted;
  std::array<int, 4> m_mask;
  std::array<int, 4> m_count;
  std::array<std::array<Vec4, 4>, 16> m_points;
  std::array<std::array<Scr4, 4>, 16> m_weights;
  std::array<std::array<char, 4>, 16> m_remap;

#ifdef FEATURE_TEST_LINES
  /* ---------------------------------------------------------------------------
   */
public:
  int GetChannel(int idx) const
  {
    if (m_mergedalpha || (idx >= m_numsets))
    {
      if (!(m_cnst[idx] & 0x0FFF))
        return 3;
    }
    {
      if (!(m_cnst[idx] & 0xF0FF))
        return 2;
      if (!(m_cnst[idx] & 0xFF0F))
        return 1;
      if (!(m_cnst[idx] & 0xFFF0))
        return 0;
    }
    {
      if (!(m_grey[idx] & 0x0FFF))
        return 8;
      if (!(m_cnst[idx] & 0xF000))
        if (!(m_grey[idx] & 0x00FF))
          return 7;
      if (!(m_cnst[idx] & 0x0F00))
        if (!(m_grey[idx] & 0xF00F))
          return 6;
      if (!(m_cnst[idx] & 0x00F0))
        if (!(m_grey[idx] & 0xFF00))
          return 5;
      if (!(m_cnst[idx] & 0x000F))
        if (!(m_grey[idx] & 0x0FF0))
          return 4;
      if (!(m_cnst[idx] & 0xF00F))
        if (!(m_grey[idx] & 0x00F0))
          return 12;
      if (!(m_cnst[idx] & 0xFF00))
        if (!(m_grey[idx] & 0x000F))
          return 11;
      if (!(m_cnst[idx] & 0x0FF0))
        if (!(m_grey[idx] & 0xF000))
          return 10;
      if (!(m_cnst[idx] & 0x00FF))
        if (!(m_grey[idx] & 0x0F00))
          return 9;
    }

    return -1;
  }

private:
  std::array<int, 4> m_cnst;
  std::array<int, 4> m_grey;
#endif
};
}  // namespace squish

#endif  // ndef SQUISH_PALETTESET_H
