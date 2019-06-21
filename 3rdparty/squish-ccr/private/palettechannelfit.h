/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2006 Ignacio Castano                   icastano@nvidia.com
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

#ifndef SQUISH_PALETTECHANNELFIT_H
#define SQUISH_PALETTECHANNELFIT_H

#include <squish.h>
#include <limits.h>

#include "palettefit.h"

namespace squish
{

// -----------------------------------------------------------------------------
class palette_set;
class palette_channel_fit : public virtual palette_fit
{
public:
  palette_channel_fit(palette_set const* colors, flags_t flags, int swap = -1,
                      int shared = 0);

private:
  int m_channel[4];
  Vec4 m_start_candidate[4];
  Vec4 m_end_candidate[4];

protected:
  Scr4 ComputeCodebook(int set, Vec4 const& metric, vQuantizer& q, int sb,
                       int ib, std::uint8_t (&closest)[16]);

  bool IsChannel(int set)
  {
    return m_channel[set] >= 0;
  }
};
}  // namespace squish

#endif  // ndef SQUISH_PALETTECHANNELFIT_H
