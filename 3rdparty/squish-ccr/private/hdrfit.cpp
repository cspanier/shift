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

#include "hdrfit.h"
#include "hdrset.h"

#if 1  // ndef NDEBUG
#include "inlineables.inl"
#endif

namespace squish
{
/*  Mode NS PB SB  DB  EB TB IB NS: Number of subsets in each partition
 *  ---- -- -- --- --- -- -- -- PB: Partition bits
 *   1   2  5  5   5     6  3  SB: Shared MSBs
 *   2   2  5  1   6     9  3  DB: Delta-coded LSBs
 *   3   2  5  6/7 5/4   5  3  EB: Explicit MSBs (delta-coded against 0)
 *   4   2  5  6/7 5/4   5  3  TB: Truncated LSBs
 *   5   2  5  6/7 5/4   5  3  IB: Index bits
 *   6   2  5  4   5     7  3
 *   7   2  5  2/3 6/5   8  3
 *   8   2  5  2/3 6/5   8  3
 *   9   2  5  2/3 6/5   8  3
 *  10   2  5        6  10 3
 *  11   1  0        10 6  4
 *  12   1  0  2   9     5  4
 *  13   1  0  4   8     4  4
 *  14   1  0  12  4     0  4
 */
static const struct
{
  char NS, PB, EB, SB[3], DB[3], TB, IB;
} HBcfg[14] = {{2, 5, 0, {5, 5, 5}, {5, 5, 5}, 6, 3},
               {2, 5, 0, {1, 1, 1}, {6, 6, 6}, 9, 3},
               {2, 5, 0, {6, 7, 7}, {5, 4, 4}, 5, 3},
               {2, 5, 0, {7, 6, 7}, {4, 5, 4}, 5, 3},
               {2, 5, 0, {7, 7, 6}, {4, 4, 5}, 5, 3},
               {2, 5, 0, {4, 4, 4}, {5, 5, 5}, 7, 3},
               {2, 5, 0, {2, 3, 3}, {6, 5, 5}, 8, 3},
               {2, 5, 0, {3, 2, 3}, {5, 6, 5}, 8, 3},
               {2, 5, 0, {3, 3, 2}, {5, 5, 5}, 8, 3},
               {2, 5, 6, {0, 0, 0}, {0, 0, 0}, 10, 3},
               {1, 0, 10, {0, 0, 0}, {0, 0, 0}, 6, 4},
               {1, 0, 0, {2, 2, 2}, {9, 9, 9}, 5, 4},
               {1, 0, 0, {4, 4, 4}, {8, 8, 8}, 4, 4},
               {1, 0, 0, {12, 12, 12}, {4, 4, 4}, 0, 4}};

int hdr_fit::GetNumSets(int mode)
{
  return HBcfg[mode].NS;
}

int hdr_fit::GetPartitionBits(int mode)
{
  return HBcfg[mode].PB;
}

int hdr_fit::GetIndexBits(int mode)
{
  return HBcfg[mode].IB;
}

int hdr_fit::GetExplicitBits(int mode)
{
  return HBcfg[mode].EB;
}

int hdr_fit::GetSharedBits(int mode)
{
  return (HBcfg[mode].SB[2] << 16) + (HBcfg[mode].SB[1] << 8) +
         (HBcfg[mode].SB[0] << 0);
}

int hdr_fit::GetDeltaBits(int mode)
{
  return (HBcfg[mode].DB[2] << 16) + (HBcfg[mode].DB[1] << 8) +
         (HBcfg[mode].DB[0] << 0);
}

int hdr_fit::GetSharedBits(int mode, int channel)
{
  return HBcfg[mode].SB[channel];
}

int hdr_fit::GetDeltaBits(int mode, int channel)
{
  return HBcfg[mode].DB[channel];
}

int hdr_fit::GetTruncationBits(int mode)
{
  return HBcfg[mode].TB;
}

int hdr_fit::GetPrecisionBits(int mode)
{
  return 16 - HBcfg[mode].TB;
}

hdr_fit::hdr_fit(hdr_set const* palette, flags_t flags)
: m_palette(palette), m_flags(flags)
{
  // initialize the metric
  const bool perceptual = (m_flags & squish_flag::color_metric_mask) ==
                          squish_flag::color_metric_perceptual;
  const bool unit = (m_flags & squish_flag::color_metric_mask) ==
                    squish_flag::color_metric_unit;

  // sum is 1.0f
  if (unit)
    m_metric = Vec3(0.5000f, 0.5000f, 0.0000f);
  else if (perceptual)  // linear RGB luminance
    m_metric = Vec3(0.2126f, 0.7152f, 0.0722f);
  else
    m_metric = Vec3(0.3333f, 0.3334f, 0.3333f);

  // initialize the best error
  m_besterror = Scr3(FLT_MAX);
  m_best = false;

  m_mode = (static_cast<std::uint32_t>(
              m_flags & squish_flag::variable_coding_mode_mask) >>
            24) -
           1;
}

void hdr_fit::Compress(void* block, fQuantizer& q)
{
  if (m_mode < 0)
  {
    /*
    for (int m = 0; m < 14; m++) {
      if (m_palette->GetSets() == GetNumSets(m))
        ...
    }
    */

    switch (m_palette->GetSets())
    {
    case 1:
      if (m_palette->GetPartition() < 1)
      {
        // 1 partition
        m_mode = 11, Compress(block, q);
        m_mode = 12, Compress(block, q);
        m_mode = 13, Compress(block, q);
        m_mode = 14, Compress(block, q);
      }
      break;
    case 2:
      // 32 partitions
      m_mode = 0, Compress(block, q);
      m_mode = 1, Compress(block, q);
      m_mode = 2, Compress(block, q);
      m_mode = 3, Compress(block, q);
      m_mode = 4, Compress(block, q);
      m_mode = 5, Compress(block, q);
      m_mode = 6, Compress(block, q);
      m_mode = 7, Compress(block, q);
      m_mode = 8, Compress(block, q);
      m_mode = 9, Compress(block, q);
      m_mode = 10, Compress(block, q);
      break;
    }
  }
  else
    Compress(block, q, m_mode);
}

#if 0  // ndef NDEBUG
void hdr_fit::SumError(std::uint8_t (&closest)[4][16], fQuantizer &q, int mode, Scr4 &error) {
  int ib = GetIndexBits(mode);
  int tb = GetTruncationBits(mode);
  int db = GetDeltaBits(mode);

  q.ChangeField(tb, db);

  // the alpha-set (in theory we can do separate alpha + separate partitioning, but's not codeable)
  int const isets = m_palette->GetSets();

  assume((isets > 0) && (isets <= 3));

  // create a codebook
  Vec4 codes[1 << 4];

  // loop over all sets
  for (int s = 0; s < isets; s++) {
    // in case of separate alpha the colors of the alpha-set have all been set to alpha
    Vec4 metric = m_metric[0];

    // cache some values
    int const count = m_palette->GetCount(s);
    Vec4 const* values = m_palette->GetPoints(s);
    std::uint8_t const* freq = m_palette->GetFrequencies(s);

    // snap floating-point-values to the integer-lattice
    Vec4 start = q.SnapToLattice(m_start[s]);
    Vec4 end   = q.SnapToLattice(m_end  [s]);

    // resolve "metric * (value - code)" to "metric * value - metric * code"
    CodebookP(codes, ib, metric * start, metric * end);

    // error-sum
    for (int i = 0; i < count; ++i)
      error += LengthSquared(metric * values[i] - codes[closest[s][i]]) * freq[i];
  }
}

void hdr_fit::Decompress(std::uint16_t *rgb, fQuantizer &q, int mode)
{
  int ib = GetIndexBits(mode);
  int tb = GetTruncationBits(mode);
  int db = GetDeltaBits(mode);

  q.ChangeField(tb, db);

  // the alpha-set (in theory we can do separate alpha + separate partitioning, but's not codeable)
  int const isets = m_palette->GetSets();

  assume((isets > 0) && (isets <= 2));

  // create a codebook
  std::int64_t codes[1 << 4];

  // loop over all sets
  for (int s = 0; s < isets; s++) {
    // snap floating-point-values to the integer-lattice
    Vec4 fstart = m_start[s];
    Vec4 fend   = m_end  [s];

    // snap floating-point-values to the integer-lattice
    Col4 istart = q.QuantizeToInt(fstart);
    Col4 iend   = q.QuantizeToInt(fend  );

    istart = Col4(
      (istart.R() << (8 - cb)) + (istart.R() >> (cb - (8 - cb))),
      (istart.G() << (8 - cb)) + (istart.G() >> (cb - (8 - cb))),
      (istart.B() << (8 - cb)) + (istart.B() >> (cb - (8 - cb))),
      0
    );

    iend = Col4(
      (iend.R() << (8 - cb)) + (iend.R() >> (cb - (8 - cb))),
      (iend.G() << (8 - cb)) + (iend.G() >> (cb - (8 - cb))),
      (iend.B() << (8 - cb)) + (iend.B() >> (cb - (8 - cb))),
      0
    );

    int ccs;
    switch (ib) {
      case 2: ccs = CodebookP<2>(codes, istart, iend); break;
      case 3: ccs = CodebookP<3>(codes, istart, iend); break;
      case 4: ccs = CodebookP<4>(codes, istart, iend); break;
    }

    m_palette->UnmapIndices(m_indices, rgb, s, codes, ~0LL);
  }
}
#endif
}  // namespace squish
