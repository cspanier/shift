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

#ifndef SQUISH_PALETTEFIT_H
#define SQUISH_PALETTEFIT_H

#include <squish.h>
#include "maths_all.h"

namespace squish
{

// -----------------------------------------------------------------------------
class palette_set;
class palette_fit
{
public:
  static int GetNumSets(std::uint32_t mode);
  static int GetPartitionBits(std::uint32_t mode);
  static int GetIndexBits(std::uint32_t mode);
  static int GetRotationBits(std::uint32_t mode);
  static int GetSelectionBits(std::uint32_t mode);
  static int GetSharedBits(std::uint32_t mode);
  static int GetPrecisionBits(std::uint32_t mode);

  static const int* GetSharedMap(int mode);
  static int GetSharedSkip(int mode);

  // rotate shared bit definition: 0=0 (-), 1=3 (s), 2=2 (u), 3=1 (u)
  // unique p-bit permutations: upper bit start bit set, lower bit stop bit set
  // shared p-bit permutations: upper & lower bit start & stop bit set
  // makes it easier to loop
#define SBSTART 0
#define SBEND 3
#define SR(s) (s < 0 ? s : m_sharedmap[s])
#define SBSKIP -1
#define SK(s) (!(~s))

public:
  palette_fit(palette_set const* palette, int flags, int swap = -1,
              int shared = -1);

  // change parameters while iterating
  void ChangeFit(palette_set const* palette, int flags, int swap, int shared)
  {
    m_palette = palette;
    m_flags = flags;
    m_swapindex = swap;
    m_sharedbits = SR(shared);
    m_best = false;
  }
  void ChangePalette(palette_set const* palette)
  {
    m_palette = palette;
  }
  void ChangeFlags(int flags)
  {
    m_flags = flags;
  }
  void ChangeSwap(int swap)
  {
    m_swapindex = swap;
  }
  void ChangeMode(int mode)
  {
    m_sharedmap = GetSharedMap(m_mode = mode);
  }
  void ChangeShared(int shared)
  {
    m_sharedbits = SR(shared);
  }

  // query some values
  palette_set const* GetPalette() const
  {
    return m_palette;
  }
  int GetFlags() const
  {
    return m_flags;
  }
  int GetSwap() const
  {
    return m_swapindex;
  }
  int GetSharedField() const
  {
    return m_sharedbits;
  }

  // error management
  void SetError(Scr4& error)
  {
    m_besterror = error;
    m_best = false;
  }
  Scr4 GetError()
  {
    return m_besterror;
  }

  void Compress(void* block, vQuantizer& q);
  virtual void Compress(void* block, vQuantizer& q, int mode) = 0;

#if 1  // ndef NDEBUG
  void Decompress(std::uint8_t* rgba, vQuantizer& q, int mode);
  void SumError(std::uint8_t (&closest)[4][16], vQuantizer& q, int mode,
                Scr4& error);
#endif

  bool Lossless()
  {
    return !(m_besterror > Scr4(0.0f));
  }
  bool IsBest()
  {
    return m_best;
  }

protected:
  palette_set const* m_palette;
  const int* m_sharedmap;
  int m_flags;
  int m_mode;
  int m_swapindex;
  int m_sharedbits;

  Vec4 m_start[4];
  Vec4 m_end[4];
  SQUISH_ALIGNED std::uint8_t m_indices[2][16];

  Vec4 m_metric[3];
  Scr4 m_besterror;
  bool m_best;
};
}  // namespace squish

#endif  // ndef SQUISH_PALETTEFIT_H
