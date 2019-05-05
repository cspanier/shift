/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels FrÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ¶hling              niels@paradice-insight.us

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

#ifndef SQUISH_PALETTENORMALFIT_H
#define SQUISH_PALETTENORMALFIT_H

#include <squish.h>
#include "palettesinglefit.h"
#include "palettesinglesnap.h"
#include "maths_all.h"

namespace squish {

// -----------------------------------------------------------------------------
class PaletteSet;
class PaletteNormalFit : public PaletteSingleMatch
{
public:
  static bool IsClusterable(int flags) { return false; }

public:
  PaletteNormalFit(PaletteSet const* palette, int flags, int swap = -1, int shared = -1);

  virtual void Compress(void* block, vQuantizer &q, int mode);

private:
};
} // squish

#endif // ndef SQUISH_PALETTENORMALFIT_H
