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

#include <cassert>
#include <cstring>
#include "colorset.h"
#include "helpers.h"

namespace squish
{

/* *****************************************************************************
 */
color_set::color_set(std::uint8_t const* rgba, int mask, int flags)
: m_count(0), m_unweighted(true), m_transparent(false)
{
  const float* rgbLUT = ComputeGammaLUT((flags & kSrgbExternal) != 0);

  // check the compression mode for dxt1
  bool const isBtc1 = ((flags & kBtcp) == kBtc1);
  bool const clearAlpha = ((flags & kExcludeAlphaFromPalette) != 0);
  bool const weightByAlpha = ((flags & kWeightcolorByAlpha) != 0);

  // build mapped data
  int clra = clearAlpha || !isBtc1 ? 0xFFFF : 0x0000;
  int wgta = weightByAlpha ? 0x0000 : 0xFFFF;

  SQUISH_ALIGNED std::uint8_t rgbx[4 * 16];
  int amask;

  Col4 m0 = Col4(&rgba[0 * 16]);
  Col4 m1 = Col4(&rgba[1 * 16]);
  Col4 m2 = Col4(&rgba[2 * 16]);
  Col4 m3 = Col4(&rgba[3 * 16]);
  Col4 al = CollapseA(m0, m1, m2, m3);

  // clear alpha
  m0 &= Col4(0x00FFFFFF);
  m1 &= Col4(0x00FFFFFF);
  m2 &= Col4(0x00FFFFFF);
  m3 &= Col4(0x00FFFFFF);

  StoreAligned(m0, &rgbx[0 * 16]);
  StoreAligned(m1, &rgbx[1 * 16]);
  StoreAligned(m2, &rgbx[2 * 16]);
  StoreAligned(m3, &rgbx[3 * 16]);

  // threshold alpha (signed char)
  amask = CompareAllLessThan_M8(al, Col4(0)).GetM8() | (clra);
#ifdef FEATURE_IGNORE_ALPHA0
  // threshold color
  amask &= (~CompareAllEqualTo_M8(al, Col4(0)).GetM8()) | (wgta);
#endif
  // combined mask
  amask &= mask;

  // create the minimal set, O(16*count/2)
  for (int i = 0, imask = amask, index; i < 16; ++i, imask >>= 1)
  {
    // check this pixel is enabled
    if ((imask & 1) == 0)
    {
      m_remap[i] = -1;

      /* check for transparent pixels when using dxt1
       * check for blanked out pixels when weighting
       */
      m_transparent = m_transparent | ((mask & (1 << i)) != 0);

      continue;
    }

    // calculate point's weights
    Weight<std::uint8_t> wa(rgba, i, (std::uint8_t)wgta);

    // loop over previous matches for a match
    std::uint8_t* rgbvalue = &rgbx[4 * i + 0];
    for (index = 0; index < m_count; ++index)
    {
      std::uint8_t* crgbvalue = &rgbx[4 * index + 0];

      // check for a match
      if (*((int*)rgbvalue) == *((int*)crgbvalue))
      {
        // get the index of the match
        assume(index >= 0 && index < 16);

        // map to this point and increase the weight
        m_remap[i] = (char)index;
        m_weights[index] += wa.GetWeights();
        m_unweighted = false;
        break;
      }
    }

    // re-use the memory of already processed pixels
    assert(index <= i);
    {
      std::uint8_t* crgbvalue = &rgbx[4 * index + 0];

      // allocate a new point
      if (index == m_count)
      {
        // get the index of the match and advance
        m_count = index + 1;

        // normalize coordinates to [0,1]
        const float* r = &rgbLUT[rgbvalue[0]];
        const float* g = &rgbLUT[rgbvalue[1]];
        const float* b = &rgbLUT[rgbvalue[2]];

        // add the point
        m_remap[i] = (char)index;
        m_points[index] = Vec3(r, g, b);
        m_weights[index] = wa.GetWeights();
        m_unweighted = m_unweighted & wa.IsOne();

        // remember match for successive checks
        *((int*)crgbvalue) = *((int*)rgbvalue);
      }
    }
  }

#ifdef FEATURE_IGNORE_ALPHA0
  if ((clra == 0xFFFF) && (amask != 0xFFFF))
  {
    if (!m_count)
    {
      Vec3 sum = Vec3(0.0f);

      for (int i = 0, imask = amask; i < 16; ++i, imask >>= 1)
      {
        /* assign blanked out pixels when weighting
         */
        if ((imask & 1) == 0)
        {
          m_remap[i] = 0;

          std::uint8_t* rgbvalue = &rgbx[4 * i + 0];

          // normalize coordinates to [0,1]
          const float* r = &rgbLUT[rgbvalue[0]];
          const float* g = &rgbLUT[rgbvalue[1]];
          const float* b = &rgbLUT[rgbvalue[2]];

          sum += Vec3(r, g, b);
        }
      }

      // add the point
      m_count = 1;
      m_points[0] = sum * (1.0f / 16.0f);
      m_weights[0] = Scr3(1.0f);
      m_unweighted = true;
    }
    else if (m_transparent)
    {
      for (int i = 0, imask = amask, index; i < 16; ++i, imask >>= 1)
      {
        /* assign blanked out pixels when weighting
         */
        if ((imask & 1) == 0)
        {
          std::uint8_t* rgbvalue = &rgbx[4 * i + 0];

          // normalize coordinates to [0,1]
          const float* r = &rgbLUT[rgbvalue[0]];
          const float* g = &rgbLUT[rgbvalue[1]];
          const float* b = &rgbLUT[rgbvalue[2]];

          // loop over previous matches for a match
          Scr3 d = Scr3(FLT_MAX);
          for (index = 0; index < m_count; ++index)
          {
            Vec3 diff = m_points[index] - Vec3(r, g, b);
            Scr3 dist = Dot(diff, diff);

            if (d > dist)
            {
              d = dist;

              m_remap[i] = (char)index;
            }
          }
        }
      }
    }
  }
#endif

#ifdef FEATURE_WEIGHTS_ROOTED
  // square root the weights
  for (int i = 0; i < m_count; ++i)
    m_weights[i] = math::sqrt(m_weights[i]);
#endif

  // clear if we're suppose to throw alway alpha
  m_transparent = m_transparent & !clearAlpha;
}

color_set::color_set(std::uint16_t const* rgba, int mask, int flags)
: m_count(0), m_unweighted(true), m_transparent(false)
{
}

color_set::color_set(float const* rgba, int mask, int flags)
: m_count(0), m_unweighted(true), m_transparent(false)
{
  // const float *rgbLUT = ComputeGammaLUT((flags & kSrgbIn) != 0);

  // check the compression mode for dxt1
  bool const isBtc1 = ((flags & kBtcp) == kBtc1);
  bool const clearAlpha = ((flags & kExcludeAlphaFromPalette) != 0);
  bool const weightByAlpha = ((flags & kWeightcolorByAlpha) != 0);

  // build mapped data
  Scr4 clra = clearAlpha || !isBtc1 ? Scr4(1.0f) : Scr4(0.0f);
  Scr4 wgta = weightByAlpha ? Scr4(0.0f) : Scr4(1.0f);

  Vec3 rgbx[16];
  int amask = 0;

  for (int i = 0; i < 16; i += 4)
  {
    Vec4 m0;
    LoadUnaligned(m0, &rgba[4 * i + 4 * 0]);
    Vec4 m1;
    LoadUnaligned(m1, &rgba[4 * i + 4 * 1]);
    Vec4 m2;
    LoadUnaligned(m2, &rgba[4 * i + 4 * 2]);
    Vec4 m3;
    LoadUnaligned(m3, &rgba[4 * i + 4 * 3]);
    Vec4 al = CollapseW(m0, m1, m2, m3);
    Vec4 ibit;

    // clear alpha
    rgbx[i + 0] = KillW(m0).GetVec3();
    rgbx[i + 1] = KillW(m1).GetVec3();
    rgbx[i + 2] = KillW(m2).GetVec3();
    rgbx[i + 3] = KillW(m3).GetVec3();

    // threshold alpha
    ibit = IsGreaterEqual(Max(al, clra), Vec4(0.5f));
#ifdef FEATURE_IGNORE_ALPHA0
    // threshold color
    ibit &= IsNotEqualTo(Max(al, wgta), Vec4(0.0f));
#endif

    amask += ibit.GetM4() << i;
  }

  // combined mask
  amask &= mask;

  // create the minimal set, O(16*count/2)
  for (int i = 0, imask = amask, index; i < 16; ++i, imask >>= 1)
  {
    // check this pixel is enabled
    if ((imask & 1) == 0)
    {
      m_remap[i] = -1;

      /* check for transparent pixels when using dxt1
       * check for blanked out pixels when weighting
       */
      m_transparent = m_transparent | ((mask & (1 << i)) != 0);

      continue;
    }

    // calculate point's weights
    Weight<float> wa(rgba, i, wgta);

    // loop over previous matches for a match
    Vec3* rgbvalue = &rgbx[i];
    for (index = 0; index < m_count; ++index)
    {
      Vec3* crgbvalue = &rgbx[index];

      // check for a match
      if (CompareAllEqualTo((*rgbvalue), (*crgbvalue)))
      {
        // get the index of the match
        assume(index >= 0 && index < 16);

        // map to this point and increase the weight
        m_remap[i] = (char)index;
        m_weights[index] += wa.GetWeights();
        m_unweighted = false;
        break;
      }
    }

    // re-use the memory of already processed pixels
    assert(index <= i);
    {
      Vec3* crgbvalue = &rgbx[index];

      // allocate a new point
      if (index == m_count)
      {
        // get the index of the match and advance
        m_count = index + 1;

#if 0
  // normalize coordinates to [0,1]
  const float *r = &rgbLUT[rgbvalue[0]];
  const float *g = &rgbLUT[rgbvalue[1]];
  const float *b = &rgbLUT[rgbvalue[2]];
#endif

        // add the point
        m_remap[i] = (char)index;
        m_points[index] = *(rgbvalue);
        m_weights[index] = wa.GetWeights();
        m_unweighted = m_unweighted & wa.IsOne();

        // remember match for successive checks
        *(crgbvalue) = *(rgbvalue);
      }
    }
  }

#ifdef FEATURE_IGNORE_ALPHA0
  if ((clra == Scr4(1.0f)) && (amask != 0xFFFF))
  {
    if (!m_count)
    {
      Vec3 sum = Vec3(0.0f);

      for (int i = 0, imask = amask; i < 16; ++i, imask >>= 1)
      {
        /* assign blanked out pixels when weighting
         */
        if ((imask & 1) == 0)
        {
          m_remap[i] = 0;

          Vec3* rgbvalue = &rgbx[i];

#if 0
    // normalize coordinates to [0,1]
    const float *r = &rgbLUT[rgbvalue[0]];
    const float *g = &rgbLUT[rgbvalue[1]];
    const float *b = &rgbLUT[rgbvalue[2]];
#endif

          sum += *(rgbvalue);
        }
      }

      // add the point
      m_count = 1;
      m_points[0] = sum * (1.0f / 16.0f);
      m_weights[0] = Scr3(1.0f);
      m_unweighted = true;
    }
    else if (m_transparent)
    {
      for (int i = 0, imask = amask, index; i < 16; ++i, imask >>= 1)
      {
        /* assign blanked out pixels when weighting
         */
        if ((imask & 1) == 0)
        {
          Vec3* rgbvalue = &rgbx[i];

#if 0
    // normalize coordinates to [0,1]
    const float *r = &rgbLUT[rgbvalue[0]];
    const float *g = &rgbLUT[rgbvalue[1]];
    const float *b = &rgbLUT[rgbvalue[2]];
#endif

          // loop over previous matches for a match
          Scr3 d = Scr3(FLT_MAX);
          for (index = 0; index < m_count; ++index)
          {
            Vec3 diff = m_points[index] - *(rgbvalue);
            Scr3 dist = Dot(diff, diff);

            if (d > dist)
            {
              d = dist;

              m_remap[i] = (char)index;
            }
          }
        }
      }
    }
  }
#endif

#ifdef FEATURE_WEIGHTS_ROOTED
  // square root the weights
  for (int i = 0; i < m_count; ++i)
    m_weights[i] = math::sqrt(m_weights[i]);
#endif

  // clear if we're suppose to throw alway alpha
  m_transparent = m_transparent & !clearAlpha;
}

bool color_set::RemoveBlack(const Vec3& metric, Scr3& error)
{
  cQuantizer4<5, 6, 5, 0> q = cQuantizer4<5, 6, 5, 0>();
  bool reduced = false;

  while (m_count > 1)
  {
    Scr3 lowest = LengthSquared(metric * Vec3(32.0f / 255.0f));
    int index = -1;

    for (int i = 0; i < 16; ++i)
    {
      if (m_remap[i] == -1)
        continue;

      // maps to black
      Vec3 color = m_points[m_remap[i]];
      /*Vec3 result = q.SnapToLattice(color);*/
      if (true /*CompareAllEqualTo(result, Vec3(0.0f))*/)
      {
        Scr3 len = LengthSquared(metric * color);
        if (len < lowest)
        {
          lowest = len;
          index = m_remap[i];
        }
      }
    }

    if (index >= 0)
    {
      m_count--;
      m_unweighted = false;

      for (int i = 0; i < 16; ++i)
      {
        if (m_remap[i] == index)
          m_remap[i] = -1, m_transparent = true;
        else if (m_remap[i] > index)
          m_remap[i] = -1 + m_remap[i];
      }

      error += LengthSquared(metric * m_points[index]) * m_weights[index];

      if (m_count > index)
      {
        memcpy(&m_points[index + 0], &m_points[index + 1],
               sizeof(m_points[0]) * (m_count - index));
        memcpy(&m_weights[index + 0], &m_weights[index + 1],
               sizeof(m_weights[0]) * (m_count - index));
      }

      reduced = true;
      return reduced;
    }
    else
      break;
  }

  return reduced;
}

void color_set::RemapIndices(std::uint8_t const* source,
                             std::uint8_t* target) const
{
  for (int i = 0; i < 16; ++i)
  {
    std::uint8_t t = 3;
    t = ((m_remap[i] == -1) ? t : source[m_remap[i]]);
    target[i] = t;
  }
}
}  // namespace squish
