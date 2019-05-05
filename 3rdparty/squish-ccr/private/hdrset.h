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
class HDRSet
{
public:
  static void GetMasks(int flags, int partition, int (&masks)[2]);

  int SetMode(int flags);
  int SetMode(int flags, int partition);

  // maximum number of different sets, aligned, the real limit is 3
#define PS_MAX 4

public:
  // constructor for regular operation (with and without initial
  // partition/rotation)
  HDRSet(std::uint16_t const* rgb, int mask, int flags);
  HDRSet(std::uint16_t const* rgb, int mask, int flags, int partition);

  HDRSet(float const* rgb, int mask, int flags);
  HDRSet(float const* rgb, int mask, int flags, int partition);

  // constructors for managing backups and permutations of palette-sets
  HDRSet()
  {
  }
  HDRSet(HDRSet const& palette)
  {
    memcpy(this, &palette, sizeof(*this));
  }
  HDRSet(HDRSet const& palette, int mask, int flags, int partition);

private:
  void BuildSet(std::uint16_t const* rgb, int mask, int flags);
  void BuildSet(float const* rgb, int mask, int flags);
  void PermuteSet(HDRSet const& palette, int mask, int flags);

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
  Vec3 const* GetPoints(int idx) const
  {
    return m_points[idx];
  }
  Scr3 const* GetWeights(int idx) const
  {
    return m_weights[idx];
  }
  int GetCount(int idx) const
  {
    return m_count[idx];
  }
  int GetCount() const
  {
    return m_count[0] +
           /*(m_numsets > 0 ? m_count[0] : 0)*/ +(m_numsets > 1 ? m_count[1]
                                                                : 0);
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

  bool m_unweighted[2];
  int m_mask[2];
  int m_count[2];
  Vec3 m_points[2][16];
  Scr3 m_weights[2][16];
  int m_remap[2][16];

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
  int m_cnst[2];
  int m_grey[2];
#endif
};
}

#endif
