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

#ifndef SQUISH_PALETTESINGLEFIT_H
#define SQUISH_PALETTESINGLEFIT_H

#include <squish.h>
#include <limits.h>

#include "palettefit.h"

namespace squish {

// -----------------------------------------------------------------------------
struct PaletteSingleLookup2;
struct PaletteSingleLookup4;
struct PaletteSingleLookup8;

class PaletteSet;
class PaletteSingleFit : public virtual PaletteFit
{
public:
  PaletteSingleFit(PaletteSet const* colours, int flags, int swap = -1, int shared = 0);

private:
  Scr4 ComputeEndPoints(int set, Vec4 const &metric, PaletteSingleLookup2 const* const* lookups, std::uint8_t cmask);
  Scr4 ComputeEndPoints(int set, Vec4 const &metric, PaletteSingleLookup4 const* const* lookups, std::uint8_t cmask);
  Scr4 ComputeEndPoints(int set, Vec4 const &metric, PaletteSingleLookup8 const* const* lookups, std::uint8_t cmask);

  std::uint8_t m_entry[4][4];
  std::uint8_t m_index;

protected:
  Scr4 ComputeEndPoints(int set, Vec4 const &metric, int cb, int ab, int sb, int ib, std::uint8_t cmask);
  std::uint8_t GetIndex() { return m_index; }
};
} // namespace squish

#endif // ndef SQUISH_PALETTESINGLEFIT_H
