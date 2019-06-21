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

#ifndef SQUISH_HDRSET_H
#define SQUISH_HDRSET_H

#include <squish.h>
#include <memory.h>
#include "maths_all.h"

namespace squish
{
/*! @brief Represents a set of block palettes
 */
class hdr_set
{
public:
  static void GetMasks(flags_t flags, int partition, int (&masks)[2]);

  int SetMode(flags_t flags);
  int SetMode(flags_t flags, int partition);

  // maximum number of different sets, aligned, the real limit is 3
#define PS_MAX 4

public:
  // constructor for regular operation (with and without initial
  // partition/rotation)
  hdr_set(std::uint16_t const* rgb, std::uint32_t mask, flags_t flags);
  hdr_set(std::uint16_t const* rgb, std::uint32_t mask, flags_t flags,
          int partition);

  hdr_set(float const* rgb, std::uint32_t mask, flags_t flags);
  hdr_set(float const* rgb, std::uint32_t mask, flags_t flags, int partition);

  // constructors for managing backups and permutations of palette-sets
  hdr_set()
  {
  }
  hdr_set(const hdr_set& palette)
  : m_numsets(palette.m_numsets),
    m_partid(palette.m_partid),
    m_partmask(palette.m_partmask),
    m_unweighted(palette.m_unweighted),
    m_mask(palette.m_mask),
    m_count(palette.m_count),
    m_points(palette.m_points),
    m_weights(palette.m_weights),
    m_remap(palette.m_remap)
#ifdef FEATURE_TEST_LINES
    ,
    m_cnst(palette.m_cnst),
    m_grey(palette.m_grey)
#endif
  {
  }
  hdr_set(hdr_set const& palette, std::uint32_t mask, flags_t flags,
          int partition);

private:
  void BuildSet(std::uint16_t const* rgb, std::uint32_t mask, flags_t flags);
  void BuildSet(float const* rgb, std::uint32_t mask, flags_t flags);
  void PermuteSet(hdr_set const& palette, std::uint32_t mask, flags_t flags);

public:
  // active attributes based the parameters passed on initializaton
  int GetSets() const
  {
    return m_numsets;
  }
  int GetPartition() const
  {
    return m_partid;
  }

  bool IsUnweighted(int idx) const
  {
    return m_unweighted[idx];
  }
  const Vec3* GetPoints(int idx) const
  {
    return m_points[idx].data();
  }
  const Scr3* GetWeights(int idx) const
  {
    return m_weights[idx].data();
  }
  int GetCount(int idx) const
  {
    return m_count[idx];
  }
  int GetCount() const
  {
    return m_count[0] + (m_numsets > 1 ? m_count[1] : 0);
  }

  // map from the set to indices and back to colors
  void RemapIndices(std::uint8_t const* source, std::uint8_t* target,
                    int set) const;
  void UnmapIndices(std::uint8_t const* source, std::uint16_t* rgb, int set,
                    std::int64_t* codes, std::int64_t cmask) const;

private:
  int m_numsets;
  int m_partid;
  int m_partmask;

  std::array<bool, 2> m_unweighted;
  std::array<int, 2> m_mask;
  std::array<int, 2> m_count;
  std::array<std::array<Vec3, 2>, 16> m_points;
  std::array<std::array<Scr3, 2>, 16> m_weights;
  std::array<std::array<int, 2>, 16> m_remap;

#ifdef FEATURE_TEST_LINES
  /* ---------------------------------------------------------------------------
   */
public:
  int GetChannel(int idx) const
  {
    {
      if (!(m_cnst[idx] & 0xF0FF))
        return 2;
      if (!(m_cnst[idx] & 0xFF0F))
        return 1;
      if (!(m_cnst[idx] & 0xFFF0))
        return 0;
    }

    return -1;
  }

private:
  std::array<int, 2> m_cnst;
  std::array<int, 2> m_grey;
#endif
};
}

#endif
