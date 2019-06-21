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

#ifndef SQUISH_colorSINGLESNAP_H
#define SQUISH_colorSINGLESNAP_H

#include <squish.h>
#include <limits.h>
#include "colorfit.h"

namespace squish
{

// -----------------------------------------------------------------------------
class color_set;
struct colorSingleLookup;
class color_single_snap : public color_fit
{
public:
  color_single_snap(color_set const* colors, flags_t flags);

  // error management
  void SetError(Scr4& error)
  {
    m_besterror = error;
  }
  void SetError(Scr3& error)
  {
    m_besterror = error;
  }
  Scr3 GetError()
  {
    return m_besterror;
  }

private:
  virtual void Compress3b(void* block);
  virtual void Compress3(void* block);
  virtual void Compress4(void* block);

  std::uint8_t m_color[4];
  Vec3 m_start;
  Vec3 m_end;
};
}  // namespace squish

#endif  // ndef SQUISH_colorSINGLESNAP_H
