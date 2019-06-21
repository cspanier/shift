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

#ifndef SQUISH_colorSET_H
#define SQUISH_colorSET_H

#include <squish.h>
#include "maths_all.h"

namespace squish
{
/// Represents a set of block colors.
class color_set
{
public:
  color_set(std::uint8_t const* rgba, std::uint32_t mask, flags_t flags);
  color_set(std::uint16_t const* rgba, std::uint32_t mask, flags_t flags);
  color_set(float const* rgba, std::uint32_t mask, flags_t flags);

  bool IsTransparent() const
  {
    return m_transparent;
  }

  bool IsUnweighted() const
  {
    return m_unweighted;
  }

  int GetCount() const
  {
    return m_count;
  }

  Vec3 const* GetPoints() const
  {
    return m_points.data();
  }

  Scr3 const* GetWeights() const
  {
    return m_weights.data();
  }

  bool RemoveBlack(const Vec3& metric, Scr3& error);
  void RemapIndices(std::uint8_t const* source, std::uint8_t* target) const;

private:
  bool m_transparent, m_unweighted;
  int m_count;
  std::array<Vec3, 16> m_points;
  std::array<Scr3, 16> m_weights;
  std::array<char, 16> m_remap;
};
}  // namespace sqish

#endif  // ndef SQUISH_colorSET_H
