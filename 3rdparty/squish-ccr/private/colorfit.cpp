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

#include "colorfit.h"
#include "colorset.h"

namespace squish
{
extern Vec4 g_metric[8];

color_fit::color_fit(color_set const* colors, flags_t flags)
: m_colors(colors), m_flags(flags)
{
  // initialize the metric
  m_metric = KillW(g_metric[static_cast<std::uint32_t>(
                              flags & squish_flag::color_metric_mask) >>
                            4]);

  // initialize the best error
  m_besterror = Scr3(FLT_MAX);
}

void color_fit::Compress(void* block)
{
  const bool isBtc1f =
    ((m_flags & squish_flag::compression_mask) == squish_flag::compression_bc1);
  const bool isBtc1b = m_flags & squish_flag::option_exclude_alpha_from_palette;

  if (isBtc1f)
  {
    Compress3(block);
    if (!m_colors->IsTransparent())
      Compress4(block);
    if (isBtc1b)
      Compress3b(block);
  }
  else
    Compress4(block);
}
}  // namespace squish
