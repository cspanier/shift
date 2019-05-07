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

#ifndef SQUISH_HDRSINGLEFIT_H
#define SQUISH_HDRSINGLEFIT_H

#include <squish.h>
#include <limits.h>

#include "hdrfit.h"

namespace squish
{
struct HDRSingleLookup2;
struct HDRSingleLookup4;
struct HDRSingleLookup8;

class hdr_set;
class hdr_single_fit : public virtual hdr_fit
{
public:
  hdr_single_fit(hdr_set const* colors, int flags);

private:
  Scr4 ComputeEndPoints(int set, Vec4 const& metric,
                        HDRSingleLookup2 const* const* lookups,
                        std::uint8_t cmask);
  Scr4 ComputeEndPoints(int set, Vec4 const& metric,
                        HDRSingleLookup4 const* const* lookups,
                        std::uint8_t cmask);
  Scr4 ComputeEndPoints(int set, Vec4 const& metric,
                        HDRSingleLookup8 const* const* lookups,
                        std::uint8_t cmask);

  std::uint8_t m_entry[4][4];
  std::uint8_t m_index;

protected:
  Scr4 ComputeEndPoints(int set, Vec4 const& metric, int tb, int db, int ib,
                        std::uint8_t cmask);
  std::uint8_t GetIndex()
  {
    return m_index;
  }
};
}

#endif
