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

#include <squish.h>
#include <assert.h>
#include <memory.h>

#include "alpha.h"

#include "bitoneset.h"
#include "colorset.h"
#include "paletteset.h"
#include "hdrset.h"

#include "maths_all.h"

// Btc2/Btc3/Btc4/Btc5
#include "alphanormalfit.h"

// Ctx1
#include "bitonenormalfit.h"
#include "bitonerangefit.h"
#include "bitoneclusterfit.h"
#include "bitoneblock.h"

// Btc1/Btc2/Btc3
#include "colornormalfit.h"
#include "colorrangefit.h"
#include "colorclusterfit.h"
#include "colorblock.h"

// Btc7
#include "palettenormalfit.h"
#include "paletterangefit.h"
#include "paletteclusterfit.h"
#include "paletteblock.h"

// Btc6
#include "hdrrangefit.h"
// nclude "hdrclusterfit.h"
#include "hdrblock.h"

#include "colorsinglefit.h"
#include "colorsinglesnap.h"
#include "palettesinglefit.h"
#include "palettesinglesnap.h"
#include "hdrsinglefit.h"
#include "hdrsinglesnap.h"

namespace squish
{

flags_t sanitize_flags(flags_t flags)
{
  // grab the flag bits
  auto method = flags & compression::mask;
  auto fit =
    flags & (compressor::color_range_fit | compressor::alpha_iterative_fit |
             compressor::color_iterative_cluster_fit);
  auto metric = flags & color_metric::mask;
  auto extra = flags & option::weight_color_by_alpha;
  auto mode = flags & variable_coding::mode_mask;
  auto map = flags & (option::srgb_external | option::srgb_internal |
                      option::signed_external | option::signed_internal);

  // set defaults
  if (!method || (method > compression::ctx1))
    method = compression::bc1;
  if (!metric ||
      ((metric > color_metric::unit) && (metric != color_metric::custom)))
    metric = kcolorMetricPerceptual;

  if (!fit)
    fit = compressor::color_cluster_fit;
  if (fit & compressor::color_iterative_cluster_fit)
  {
    if (method <= compression::bc3)
      fit = color_cluster_fit::sanitize_flags(fit);
    else if (method == compression::bc7)
      fit = palette_cluster_fit::sanitize_flags(fit);
  }

  if ((method == compression::bc6) && (mode > variable_coding::mode14))
    mode = 0;
  if ((method == compression::bc7) && (mode > variable_coding::mode8))
    mode = 0;

  return method | fit | metric | extra | mode | map;
}

/* *****************************************************************************
 */
Vec4 g_metric[8] = {
#ifdef FEATURE_METRIC_ROOTED
  // sum squared is 2.0f
  Vec4(0.5773f, 0.5773f, 0.5773f, 1.0f),
  Vec4(0.4611f, 0.8456f, 0.2687f, 1.0f),  // kcolorMetricPerceptual
  Vec4(0.5773f, 0.5773f, 0.5773f, 1.0f),  // kcolorMetricUniform
  Vec4(0.7071f, 0.7071f, 0.0000f, 1.0f),  // kcolorMetricUnit
  Vec4(0.5773f, 0.5773f, 0.5773f, 1.0f),  // kcolorMetricGray
  Vec4(0.5773f, 0.5773f, 0.5773f, 1.0f),
  Vec4(0.5773f, 0.5773f, 0.5773f, 1.0f),
  Vec4(0.5773f, 0.5773f, 0.5773f, 1.0f)  // kcolorMetricCustom
#else
  // sum is 2.0f
  Vec4(0.3333f, 0.3333f, 0.3333f, 1.0f),
  Vec4(0.2126f, 0.7152f, 0.0722f, 1.0f),  // kcolorMetricPerceptual
  Vec4(0.3333f, 0.3333f, 0.3333f, 1.0f),  // kcolorMetricUniform
  Vec4(0.5000f, 0.5000f, 0.0000f, 1.0f),  // kcolorMetricUnit
  Vec4(0.3333f, 0.3333f, 0.3333f, 1.0f),  // kcolorMetricGray
  Vec4(0.3333f, 0.3333f, 0.3333f, 1.0f),
  Vec4(0.3333f, 0.3333f, 0.3333f, 1.0f),
  Vec4(0.3333f, 0.3333f, 0.3333f, 1.0f)  // kcolorMetricCustom
#endif
};

void weights(int flags, const float* rgba)
{
  // initialize the metric
  const bool custom = ((flags & kcolorMetrics) == kcolorMetricCustom);

  if (custom)
  {
    g_metric[7] = Vec4(rgba[0], rgba[1], rgba[2], 1.0f);
    g_metric[7] /= Vec4(HorizontalAdd(g_metric[7].GetVec3()), 1.0f);

#ifdef FEATURE_METRIC_ROOTED
    g_metric[7] = Sqrt(g_metric[7]);
#endif
  }
}

/* *****************************************************************************
 */
template <typename dtyp>
void CompressBitoneCtx1u(dtyp const* rgba, int mask, void* block, int flags)
{
  // create the minimal point set
  bitone_set colors(rgba, mask, flags);

  if (((flags & kcolorRangeFit) != 0) || (colors.GetCount() == 0))
  {
    // do a range fit
    bitone_range_fit fit(&colors, flags);
    fit.Compress(block);
  }
  else
  {
    // default to a cluster fit (could be iterative or not)
    bitone_cluster_fit fit(&colors, flags);
    fit.Compress(block);
  }
}

template <typename dtyp>
void CompressNormalCtx1u(dtyp const* xyzd, int mask, void* block, int flags)
{
  // create the minimal point set
  bitone_set bitones(xyzd, mask, flags);

  // check the compression type and compress normals
  {
    // do a normal fit
    bitone_normal_fit fit(&bitones, flags);
    fit.Compress(block);
  }
}

template <typename dtyp>
void CompresscolorBtc1u(dtyp const* rgba, int mask, void* block, int flags)
{
  // create the minimal point set
  color_set colors(rgba, mask, flags);

  // check the compression type and compress color
  if (colors.GetCount() == 1)
  {
    // always do a single color fit
    color_single_match fit(&colors, flags);
    fit.Compress(block);
  }
  else if (((flags & kcolorRangeFit) != 0) || (colors.GetCount() == 0))
  {
    // do a range fit
    color_range_fit fit(&colors, flags);
    fit.Compress(block);
  }
  else
  {
    // default to a cluster fit (could be iterative or not)
    color_cluster_fit fit(&colors, flags);
    fit.Compress(block);
  }
}

template <typename dtyp>
void CompressNormalBtc1u(dtyp const* xyzd, int mask, void* block, int flags)
{
  // create the minimal point set
  color_set normals(xyzd, mask, flags);

  // check the compression type and compress normals
  if (normals.GetCount() == 1)
  {
    // always do a single color fit
    color_single_match fit(&normals, flags);
    fit.Compress(block);
  }
  else
  {
    // do a range fit
    color_normal_fit fit(&normals, flags);
    fit.Compress(block);
  }
}

#if defined(TRACK_STATISTICS)
struct statistics gstat = {0};
#endif

template <typename dtyp, class PaletteTypeFit>
Scr4 CompressPaletteBtc7uV1(dtyp const* rgba, int mask, void* block, int flags)
{
#if !defined(NDEBUG) && defined(DEBUG_SETTING)
#define DEBUG_MODE kVariableCodingMode1
#define DEBUG_FIT kcolorRangeFit  // kcolorClusterFit * 15

  flags = (flags & (~kVariableCodingModes)) | (DEBUG_MODE);
  flags = (flags & (~kcolorIterativeClusterFit)) | (DEBUG_FIT);
#endif

  /* we start with 1 set so we get some statistics about the color-
   * palette, based on that we decide if we need to search into higher
   * number of sets
   *
   * observations:
   * - if there is 1-2 color(s), we only need 1 set:
   *   the index-precision doesn't matter in that case and we choose
   *   the coding with the highest start/end-point precision
   *   nevertheless the 2 colors are not necessarily also start/end
   *   interpolated colors may achieve superior precision
   * - if there is 3-4 color(s), we only need 2 sets:
   *   the available partitions may not correspond exactly to the
   *   distribution of the 3-4 colors, so for maximum quality we need
   *   to do the whole search regardless (including 3 sets)
   *   if we've found a 2 set partition with 1-2 colors in each we can
   *   abort immediately
   *
   * rangefit searches for the best configuration (partition/swap/rotation)
   * optionally clusterfit makes the best of that partition
   */
  static const int modeorder[3][8] = {
    {
#define MODEORDER_EXPL 0
#define MODEORDER_EXPL_MIN 0
#define MODEORDER_EXPL_MAX 7
      // order: mode (lo to hi)
      kVariableCodingMode1,  //{ 3, 4, 0, 0,  4, 0, 1,  0,  3, 0 },
      kVariableCodingMode2,  //{ 2, 6, 0, 0,  6, 0, 0,  1,  3, 0 },
      kVariableCodingMode3,  //{ 3, 6, 0, 0,  5, 0, 0,  0,  2, 0 },
      kVariableCodingMode4,  //{ 2, 6, 0, 0,  7, 0, 1,  0,  2, 0 },
      kVariableCodingMode5,  //{ 1, 0, 2, 1,  5, 6, 0,  0,  2, 3 },
      kVariableCodingMode6,  //{ 1, 0, 2, 0,  7, 8, 0,  0,  2, 2 },
      kVariableCodingMode7,  //{ 1, 0, 0, 0,  7, 7, 1,  0,  4, 0 },
      kVariableCodingMode8,  //{ 2, 6, 0, 0,  5, 5, 1,  0,  2, 0 },
    },
    {
#define MODEORDER_OPAQ 1
#define MODEORDER_OPAQ_MIN 0
#define MODEORDER_OPAQ_MAX 6
      // order: sets (lo to hi), ibs (hi to lo), prc (hi to lo)
      kVariableCodingMode7,  //{ 1, 0, 0, 0,  7, 7, 1,  0,  4, 0 },
      kVariableCodingMode5,  //{ 1, 0, 2, 1,  5, 6, 0,  0,  2, 3 },
      kVariableCodingMode6,  //{ 1, 0, 2, 0,  7, 8, 0,  0,  2, 2 },

      kVariableCodingMode2,  //{ 2, 6, 0, 0,  6, 0, 0,  1,  3, 0 },  //
                             // non-alpha variant of mode 8
      kVariableCodingMode4,  //{ 2, 6, 0, 0,  7, 0, 1,  0,  2, 0 },  //
                             // non-alpha variant of mode 8

      kVariableCodingMode1,  //{ 3, 4, 0, 0,  4, 0, 1,  0,  3, 0 },
      kVariableCodingMode3,  //{ 3, 6, 0, 0,  5, 0, 0,  0,  2, 0 },

      0,
    },
    {
#define MODEORDER_TRNS 2
#define MODEORDER_TRNS_MIN 0
#define MODEORDER_TRNS_MAX 3
      // order: sets (lo to hi), ibs (hi to lo), prc (hi to lo)
      kVariableCodingMode7,  //{ 1, 0, 0, 0,  7, 7, 1,  0,  4, 0 },
      kVariableCodingMode5,  //{ 1, 0, 2, 1,  5, 6, 0,  0,  2, 3 },
      kVariableCodingMode6,  //{ 1, 0, 2, 0,  7, 8, 0,  0,  2, 2 },

      kVariableCodingMode8,  //{ 2, 6, 0, 0,  5, 5, 1,  0,  2, 0 },  // alpha
                             // variant of mode 2/4

      0, 0, 0, 0}};

  int numm = flags & (kVariableCodingModes),
      sm = (numm == 0 ? MODEORDER_OPAQ_MIN : (numm >> 24) - 1),
      em = (numm == 0 ? MODEORDER_OPAQ_MAX : sm),
      om = (numm == 0 ? MODEORDER_OPAQ : MODEORDER_EXPL);
  flags &= (~kVariableCodingModes);

#undef MODEORDER_EXPL
#undef MODEORDER_EXPL_MIN
#undef MODEORDER_EXPL_MAX

  // limits sets to 3 and choose the partition freely
  int lmts = 3;
  int lmtp = -1;

  // use the same data-structure all the time
  palette_set bestpal;
  int bestmde = -1;
  int bestswp = -1;
  int bestbit = -1;
  int besttyp = -1;

  Scr4 error(FLT_MAX);

  for (int m = sm; m <= em; m++)
  {
    int mode = modeorder[om][m];
    int mnum = (mode >> 24) - 1;

    // a mode has a specific number of sets, and variable rotations and
    // partitions
    int nums = palette_fit::GetNumSets(mnum);
    int numr = palette_fit::GetRotationBits(mnum);
    int nump = palette_fit::GetPartitionBits(mnum);
    int numx = palette_fit::GetSelectionBits(mnum);
    int numb = palette_fit::GetSharedBits(mnum);
    int numi = palette_fit::GetIndexBits(mnum);

    // stop if set-limit reached
    if (nums > lmts)
      break;

    // lock on the perfect partition
    int sp = (lmtp == -1 ? 0 : lmtp),
        ep = (lmtp == -1 ? (1 << nump) - 1 : lmtp);
    // search through rotations
    int sr = 0, er = (1 << numr) - 1;
    // search through index-swaps
    int sx = 0, ex = (1 << numx) - 1;
    // search through shared bits
#ifdef FEATURE_SHAREDBITS_TRIALS
    int sb = (numb > 0 ? 0 : SBSKIP), eb = (numb > 0 ? numb : SBSKIP);
#else
    int sb = (numb > 0 ? SBSKIP : SBSKIP), eb = (numb > 0 ? SBSKIP : SBSKIP);
#endif

#if !defined(NDEBUG) && defined(DEBUG_SETTING)
#define DEBUG_PARTITION 0
#define DEBUG_ROTATION 0
#define DEBUG_SELECTION 0
#define DEBUG_SHAREDBIT (numb > 0 ? 0 : -1)

    //  sp = ep = DEBUG_PARTITION;
    sr = er = DEBUG_ROTATION;
    sx = ex = DEBUG_SELECTION;
//  sb = eb = DEBUG_SHAREDBIT;
#endif

    int cb = palette_fit::GetPrecisionBits(mnum);
    int ab = cb >> 16;
    cb = cb & 0xFF;

    // create the initial point set and quantizer
    palette_set initial(rgba, mask, flags + mode);
    vQuantizer qnt(cb, cb, cb, ab);

    // signal if we do we have anything better this iteration of the search
    bool better = false;
    // check if we can do a cascade with the cluster-fit (merged alpha 4 bit is
    // the only exception)
    bool cluster = PaletteTypeFit::IsClusterable(flags) &&
                   (((numi >> 0) & 0xFF) <= CLUSTERINDICES) &&
                   (((numi >> 16) & 0xFF) <= CLUSTERINDICES);

    // if we see we have transparent values, back up from trying to test
    // non-alpha only modes this will affect only successive trials, if an
    // explicit mode is requested it's a NOP
    if (initial.IsTransparent())
      om = MODEORDER_TRNS, em = MODEORDER_TRNS_MAX;

    // if we see we have no transparent values, don't try non-rotated palettes
    // (alpha is constant for all)
    if (!initial.IsTransparent() && initial.IsSeperateAlpha())
      sr = 1;
#if defined(FEATURE_SHAREDBITS_TRIALS)
    // if we see we have no transparent values, force all shared bits to 1, or
    // non-opaque codebook-entries occur the all transparent case isn't so
    // crucial, when we use IGNORE_ALPHA0 it's redundant to force 0 anyway
    if (!initial.IsTransparent() && initial.IsMergedAlpha())
      sb = eb;
    // otherwise just use the most occurring bit (parity) for all other cases
    // otherwise just use the most occurring bit (parity) for all non-alpha
    // cases
    else if (((FEATURE_SHAREDBITS_TRIALS ==
               SHAREDBITS_TRIAL_ALPHAONLYOPAQUE)) ||
             ((FEATURE_SHAREDBITS_TRIALS == SHAREDBITS_TRIAL_ALPHAONLY) &&
              (mode != kVariableCodingMode7) && !initial.IsTransparent()) ||
             ((FEATURE_SHAREDBITS_TRIALS == SHAREDBITS_TRIAL_LOWPRC) &&
              (mode < kVariableCodingMode5) && (mode != kVariableCodingMode1)))
      sb = eb = SBSKIP;
#endif

    // choose rotation or partition, they're mutually exclusive
    int spr = (er ? sr : sp), epr = (er ? er : ep);

    // search for the best partition/rotation
    for (int pr = spr; pr <= epr; pr++)
    {
      // create the minimal point set
      palette_set palette(initial, mask, flags + mode, pr);

#if 0
      // if we see we have less colors than sets, 
      // then back up from trying to test with more sets
      // or even other partitions
      if (palette.GetOptimal())
  lmtp = pr, lmts = nums;
#endif

#if defined(TRACK_STATISTICS)
      for (int xu = 0; xu < nums; xu++)
      {
        int cnt = palette.GetCount(xu);
        gstat.num_counts[mnum][p][xu][cnt]++;
#ifdef FEATURE_TEST_LINES
        if (cnt > 2)
        {
          int chn = palette.GetChannel(xu) + 1;
          gstat.num_channels[mnum][p][xu][chn]++;
        }
#endif
      }

      if (palette.GetCount() <= nums)
        gstat.has_countsets[nums]++;
#endif

      // do a range fit (which uses single palette fit if appropriate)
      PaletteTypeFit fit(&palette, flags + mode);

      // TODO: swap & shared are mutual exclusive

      // search for the best swap
      for (int x = sx; x <= ex; x++)
      {
        fit.ChangeSwap(x);
        // search for the best shared bit
        for (int b = sb; b <= eb; b++)
        {
          fit.ChangeShared(b);

          // update with old best error (reset IsBest)
          fit.SetError(error);

          // we could code it lossless, no point in trying any further at all
          fit.PaletteTypeFit::Compress(block, qnt, mnum);
          if (fit.IsBest())
          {
            if (fit.Lossless())
              return Scr4(0.0f);

#if !defined(TRACK_STATISTICS) && !defined(VERIFY_QUANTIZER)
            if (cluster)
#endif
            {
              bestmde = mode, bestpal = palette, bestswp = x, bestbit = b,
              besttyp = 0, better = true;
            }
          }

          // update with new best error
          error = fit.GetError();
        }
      }
    }

    // check the compression type and compress palette of the chosen partition
    // even better
    if (better && cluster)
    {
      int degree = (flags & kcolorIterativeClusterFits);

      // default to a cluster fit (could be iterative or not)
      palette_cluster_fit fit(&bestpal, flags + mode);

      // we want the whole shebang, this takes looong!
      if (degree < (kcolorClusterFit * 15))
        sb = eb = bestbit;
      if (degree < (kcolorClusterFit * 14))
        sx = ex = bestswp;

      // search for the best swap
      for (int x = sx; x <= ex; x++)
      {
        fit.ChangeSwap(x);
        // search for the best shared bit
        for (int b = sb; b <= eb; b++)
        {
          fit.ChangeShared(b);

          // update with old best error (reset IsBest)
          fit.SetError(error);

          // we could code it lossless, no point in trying any further at all
          fit.palette_cluster_fit::Compress(block, qnt, mnum);
          if (fit.IsBest())
          {
            if (fit.Lossless())
              return Scr4(0.0f);

            if (cluster || 1)
              besttyp = 1;
          }

#if defined(TRACK_STATISTICS)
          gstat.btr_cluster[mnum][fit.IsBest() ? 1 : 0]++;
#endif

          // update with new best error
          error = fit.GetError();
        }
      }
    }

#if defined(TRACK_STATISTICS)
    gstat.win_partition[mnum][bestpal.GetPartition()]++;
    gstat.win_rotation[mnum][bestpal.GetRotation()]++;
    gstat.win_swap[mnum][bestpal.GetRotation()][bestswp]++;
#endif
  }

#if defined(TRACK_STATISTICS)
  gstat.win_mode[(bestmde >> 24) - 1]++;
  gstat.win_cluster[(bestmde >> 24) - 1][besttyp]++;
#endif

#if defined(VERIFY_QUANTIZER)
  int cb = palette_fit::GetPrecisionBits((bestmde >> 24) - 1);
  int ab = cb >> 16;
  cb = cb & 0xFF;

  // create the initial point set and quantizer
  vQuantizer qnt(cb, cb, cb, ab);

  if (!besttyp)
  {
    // do a range fit (which uses single palette fit if appropriate)
    palette_range_fit fit(&bestpal, flags + bestmde, bestswp, bestbit);

    fit.Compress(block, qnt, (bestmde >> 24) - 1);
    fit.Decompress((std::uint8_t*)rgba, qnt, (bestmde >> 24) - 1);
  }
  else
  {
    // default to a cluster fit (could be iterative or not)
    palette_cluster_fit fit(&bestpal, flags + bestmde, bestswp, bestbit);

    fit.Compress(block, qnt, (bestmde >> 24) - 1);
    fit.Decompress((std::uint8_t*)rgba, qnt, (bestmde >> 24) - 1);
  }
#endif

#if defined(VERIFY_ENCODER)
  DecompresscolorsBtc7u((std::uint8_t*)rgba, block);
#endif

  return error;
}

template <typename dtyp, class PaletteTypeFit>
Scr4 CompressPaletteBtc7uV2(dtyp const* rgba, int mask, void* block, int flags)
{
  vQuantizer q7778(7, 7, 7, 8);
  vQuantizer q5556(5, 5, 5, 6);
  vQuantizer q8888(8, 8, 8, 8);
  vQuantizer q6666(6, 6, 6, 6);
  vQuantizer q8880(8, 8, 8, 0);
  vQuantizer q7770(7, 7, 7, 0);
  vQuantizer q5550(5, 5, 5, 0);

  const struct
  {
    vQuantizer* qnt;
  } caseqnt[8] = {
    {&q7778},  //{ 1, 0, 2, 0,  7, 8, 0,  0,  2, 2 }, // 7+0,8+0  7778
    {&q5556},  //{ 1, 0, 2, 1,  5, 6, 0,  0,  2, 3 }, // 5+0,6+0  5556

    {&q8888},  //{ 1, 0, 0, 0,  7, 7, 1,  0,  4, 0 }, // 7+1,7+1  8888
    {&q6666},  //{ 2, 6, 0, 0,  5, 5, 1,  0,  2, 0 }, // 5+1,5+1  6666

    {&q8880},  //{ 2, 6, 0, 0,  7, 0, 1,  0,  2, 0 }, // 7+1,0+0  8880
    {&q7770},  //{ 2, 6, 0, 0,  6, 0, 0,  1,  3, 0 }, // 6+1,0+0  7770

    {&q5550},  //{ 3, 6, 0, 0,  5, 0, 0,  0,  2, 0 }, // 5+0,0+0  5550
    {&q5550},  //{ 3, 4, 0, 0,  4, 0, 1,  0,  3, 0 }, // 4+1,0+0  5550
  };

  /* we start with 1 set so we get some statistics about the color-
   * palette, based on that we decide if we need to search into higher
   * number of sets
   *
   * observations:
   * - if there is 1-2 color(s), we only need 1 set:
   *   the index-precision doesn't matter in that case and we choose
   *   the coding with the highest start/end-point precision
   *   nevertheless the 2 colors are not necessarily also start/end
   *   interpolated colors may achieve superior precision
   * - if there is 3-4 color(s), we only need 2 sets:
   *   the available partitions may not correspond exactly to the
   *   distribution of the 3-4 colors, so for maximum quality we need
   *   to do the whole search regardless (including 3 sets)
   *   if we've found a 2 set partition with 1-2 colors in each we can
   *   abort immediately
   *
   * rangefit searches for the best configuration (partition/swap/rotation)
   * optionally clusterfit makes the best of that partition
   */
  static const struct
  {
    int mode, mnum, casen, groupn;
  } caseorder[8] = {
    {kVariableCodingMode6, 5, 0, 0},  //{ 1, 0, 2, 0,  7, 8, 0,  0,  2, 2 },
    {kVariableCodingMode5, 4, 0, 0},  //{ 1, 0, 2, 1,  5, 6, 0,  0,  2, 3 },

    {kVariableCodingMode7, 6, 1, 1},  //{ 1, 0, 0, 0,  7, 7, 1,  0,  4, 0 },
    {kVariableCodingMode8, 7, 1,
     0},  //{ 2, 6, 0, 0,  5, 5, 1,  0,  2, 0 },  // alpha variant of mode 2/4

    {kVariableCodingMode4, 3, 2,
     0},  //{ 2, 6, 0, 0,  7, 0, 1,  0,  2, 0 },  // non-alpha variant of mode 8
    {kVariableCodingMode2, 1, 2,
     0},  //{ 2, 6, 0, 0,  6, 0, 0,  1,  3, 0 },  // non-alpha variant of mode 8

    {kVariableCodingMode3, 2, 2, 1},  //{ 3, 6, 0, 0,  5, 0, 0,  0,  2, 0 },
    {kVariableCodingMode1, 0, 2, 1},  //{ 3, 4, 0, 0,  4, 0, 1,  0,  3, 0 },
  };

#define MODECASE_MIN 0
#define MODECASE_MAX 2
  static const struct
  {
    int num[2], size;
  } casegroups[3] = {
    {{1, 1}, 2},  // 1x2 for both transparent and opaque
    {{1, 2}, 1},  // 2x1 for transparent, 1x1 for opaque
    {{2, 0}, 2},  // 0x2 for transparent, 2x2 for opaque
  };

  static int caselimit[2] = {2, 1};

  // use the same data-structure all the time
  palette_set bestpal[2];
  Vec4 bestblock[2];
  int bestqnt[2] = {-1, -1};
  int bestmde[2] = {-1, -1};
  int bestswp[2] = {-1, -1};
  int bestbit[2] = {-1, -1};
  int besttyp[2] = {-1, -1};

  Scr4 error[2];

  error[0] = Scr4(FLT_MAX);
  error[1] = Scr4(FLT_MAX);

  // number of cases to walk through
  int numm = flags & (kVariableCodingModes),
      sc = (numm == 0 ? MODECASE_MIN : caseorder[(numm >> 24) - 1].casen),
      ec = (numm == 0 ? MODECASE_MAX : sc);
  flags &= (~kVariableCodingModes);

  // cases: separate (2x), merged alpha (2x), and no alpha (4x)
  for (int mc = sc; mc <= ec; mc++)
  {
    // offset of the current case
    int go = mc * 2;

    // create the initial point set
    palette_set initial(rgba, mask, flags + caseorder[go].mode);

    // if we see we have transparent values, back up from trying to test
    // non-alpha only modes this will affect only successive trials, if an
    // explicit mode is requested it's a NOP
    ec = caselimit[initial.IsTransparent()];

    // number of groups per case, modes per group (1x2, 2x1, 2x2)
    int ng = casegroups[mc].num[initial.IsTransparent()];
    int gm = casegroups[mc].size;

    int sg = 0;
    int eg = ng - 1;

    for (int mg = sg; mg <= eg; mg++)
    {
      // offset of the current group's start and end
      int sm = go + (mg * gm);
      int em = sm + (gm - 1);

      // a mode has a specific number of sets, and variable rotations and
      // partitions
      int numr = palette_fit::GetRotationBits(caseorder[sm].mnum);
      int nump = palette_fit::GetPartitionBits(caseorder[sm].mnum);

      // search through partitions
      int sp = 0, ep = (1 << nump) - 1;
      // search through rotations
      int sr = 0, er = (1 << numr) - 1;

      // if we see we have no transparent values, don't try non-rotated palettes
      // (alpha is constant for all)
      if (!initial.IsTransparent() && initial.IsSeperateAlpha())
        sr = 1;

      // choose rotation or partition, they're mutually exclusive
      int spr = (er ? sr : sp), epr = (er ? er : ep);

      // signal if we do we have anything better this iteration of the search
      bool better[2] = {false, false};

      for (int pr = spr; pr <= epr; pr++)
      {
        // create the minimal point set
        palette_set palette(initial, mask, flags + caseorder[sm].mode, pr);

        // do a range fit (which uses single palette fit if appropriate)
        PaletteTypeFit fit(&palette, flags + caseorder[sm].mode);

        // exclude mode 1 from the upper partitions
        if ((em == 7) && (pr >= (1 << 4)))
          em = em - 1;

        for (int m = sm; m <= em; m++)
        {
          int mode = caseorder[m].mode;
          int mnum = caseorder[m].mnum;
          int mofs = (flags & kcolorRangeFit ? 0 : m - sm);

          // a mode has a specific number of sets, and variable rotations and
          // partitions
          int numx = palette_fit::GetSelectionBits(mnum);
          int numb = palette_fit::GetSharedBits(mnum);

          // search through index-swaps
          int sx = 0, ex = (1 << numx) - 1;
          // search through shared bits
#ifdef FEATURE_SHAREDBITS_TRIALS
          int sb = (numb > 0 ? 0 : SBSKIP), eb = (numb > 0 ? numb : SBSKIP);
#else
          int sb = (numb > 0 ? SBSKIP : SBSKIP),
              eb = (numb > 0 ? SBSKIP : SBSKIP);
#endif

#if defined(FEATURE_SHAREDBITS_TRIALS)
          // if we see we have no transparent values, force all shared bits to
          // 1, or non-opaque codebook-entries occur the all transparent case
          // isn't so crucial, when we use IGNORE_ALPHA0 it's redundant to force
          // 0 anyway
          if (!initial.IsTransparent() && initial.IsMergedAlpha())
            sb = eb;
          // otherwise just use the most occurring bit (parity) for all other
          // cases otherwise just use the most occurring bit (parity) for all
          // non-alpha cases
          else if (((FEATURE_SHAREDBITS_TRIALS ==
                     SHAREDBITS_TRIAL_ALPHAONLYOPAQUE)) ||
                   ((FEATURE_SHAREDBITS_TRIALS == SHAREDBITS_TRIAL_ALPHAONLY) &&
                    (mode != kVariableCodingMode7) &&
                    !initial.IsTransparent()) ||
                   ((FEATURE_SHAREDBITS_TRIALS == SHAREDBITS_TRIAL_LOWPRC) &&
                    (mode < kVariableCodingMode5) &&
                    (mode != kVariableCodingMode1)))
            sb = eb = SBSKIP;
#endif

          // TODO: swap & shared are mutual exclusive
          fit.ChangeMode(mnum);
          // search for the best swap
          for (int x = sx; x <= ex; x++)
          {
            fit.ChangeSwap(x);
            // search for the best shared bit
            for (int b = sb; b <= eb; b++)
            {
              fit.ChangeShared(b);

              // update with old best error (reset IsBest)
              fit.SetError(error[mofs]);

              // we could code it lossless, no point in trying any further at
              // all
              fit.PaletteTypeFit::Compress(block, *caseqnt[m].qnt, mnum);
              if (fit.IsBest())
              {
                if (fit.Lossless())
                  return Scr4(0.0f);

#if !defined(TRACK_STATISTICS) && !defined(VERIFY_QUANTIZER)
                if (PaletteTypeFit::IsClusterable(flags))
#endif
                {
                  bestqnt[mofs] = m, bestmde[mofs] = mode,
                  bestpal[mofs] = palette, bestswp[mofs] = x, bestbit[mofs] = b,
                  besttyp[mofs] = 0, better[mofs] = true;

                  LoadUnaligned(bestblock[mofs], block);
                }

                // update with new best error
                error[mofs] = fit.GetError();
              }
            }
          }
        }

#if 0
  // if we see we have less colors than sets, 
  // then back up from trying to test with more sets
  // or even other partitions
  if (palette.GetOptimal())
    // modes are ordered by decreasing precision
    // and increasing number of sets, this guarantees
    // that no other following mode can possibly have
    // more precise end-point than the current one
    return;
#endif
      }

      if (!PaletteTypeFit::IsClusterable(flags))
        continue;

      Scr4 tobeat = Min(error[0], error[1]);
      if (better[0] | better[1])
      {
        // ambiguous result, choose the better
        if (better[0] & better[1])
          StoreUnaligned(bestblock[error[1] < error[0]], block);

        for (int m = 0; m <= 1; m++)
        {
          if (better[m])
          {
            // a mode has a specific number of sets, and variable rotations and
            // partitions
            int mode = bestmde[m];
            int mnum = (mode >> 24) - 1;
            int numi = palette_fit::GetIndexBits(mnum);

            // check if we can do a cascade with the cluster-fit (merged alpha 4
            // bit is the only exception)
            bool cluster = (((numi >> 0) & 0xFF) <= CLUSTERINDICES) &&
                           (((numi >> 16) & 0xFF) <= CLUSTERINDICES);

            // check the compression type and compress palette of the chosen
            // partition even better
            if (cluster)
            {
              int degree = (flags & kcolorIterativeClusterFits);

              // default to a cluster fit (could be iterative or not)
              palette_cluster_fit fit(&bestpal[m], flags + mode);

              // a mode has a specific number of sets, and variable rotations
              // and partitions
              int numx = palette_fit::GetSelectionBits(mnum);
              int numb = palette_fit::GetSharedBits(mnum);

              // search through index-swaps
              int sx = 0, ex = (1 << numx) - 1;
              // search through shared bits
#ifdef FEATURE_SHAREDBITS_TRIALS
              int sb = (numb > 0 ? 0 : SBSKIP), eb = (numb > 0 ? numb : SBSKIP);
#else
              int sb = (numb > 0 ? SBSKIP : SBSKIP),
                  eb = (numb > 0 ? SBSKIP : SBSKIP);
#endif

#if defined(FEATURE_SHAREDBITS_TRIALS)
              // if we see we have no transparent values, force all shared bits
              // to 1, or non-opaque codebook-entries occur the all transparent
              // case isn't so crucial, when we use IGNORE_ALPHA0 it's redundant
              // to force 0 anyway
              if (!bestpal[m].IsTransparent() && bestpal[m].IsMergedAlpha())
                sb = eb;
              // otherwise just use the most occurring bit (parity) for all
              // other cases otherwise just use the most occurring bit (parity)
              // for all non-alpha cases
              else if (((FEATURE_SHAREDBITS_TRIALS ==
                         SHAREDBITS_TRIAL_ALPHAONLYOPAQUE)) ||
                       ((FEATURE_SHAREDBITS_TRIALS ==
                         SHAREDBITS_TRIAL_ALPHAONLY) &&
                        (mode != kVariableCodingMode7) &&
                        !bestpal[m].IsTransparent()) ||
                       ((FEATURE_SHAREDBITS_TRIALS ==
                         SHAREDBITS_TRIAL_LOWPRC) &&
                        (mode < kVariableCodingMode5) &&
                        (mode != kVariableCodingMode1)))
                sb = eb = SBSKIP;
#endif

              // we want the whole shebang, this takes looong!
              if (degree < (kcolorClusterFit * 15))
                sb = eb = bestbit[m];
              if (degree < (kcolorClusterFit * 14))
                sx = ex = bestswp[m];

              // TODO: swap & shared are mutual exclusive
              fit.ChangeMode(mnum);
              // search for the best swap
              for (int x = sx; x <= ex; x++)
              {
                fit.ChangeSwap(x);
                // search for the best shared bit
                for (int b = sb; b <= eb; b++)
                {
                  fit.ChangeShared(b);

                  // update with old best error (reset IsBest)
                  fit.SetError(tobeat);

                  // we could code it lossless, no point in trying any further
                  // at all
                  fit.palette_cluster_fit::Compress(
                    block, *caseqnt[bestqnt[m]].qnt, mnum);
                  if (fit.IsBest())
                  {
                    if (fit.Lossless())
                      return Scr4(0.0f);

                    if (cluster || 1)
                      besttyp[m] = 1;

                    // update with new best error
                    tobeat = fit.GetError();
                  }

#if defined(TRACK_STATISTICS)
                  gstat.btr_cluster[mnum][fit.IsBest() ? 1 : 0]++;
#endif
                }
              }
            }
          }
        }
      }

      error[0] = error[1] = tobeat;
    }
  }

#if defined(TRACK_STATISTICS)
  gstat.win_partition[(bestmde >> 24) - 1][bestpal.GetPartition()]++;
  gstat.win_rotation[(bestmde >> 24) - 1][bestpal.GetRotation()]++;
  gstat.win_swap[(bestmde >> 24) - 1][bestpal.GetRotation()][bestswp]++;

  gstat.win_mode[(bestmde >> 24) - 1]++;
  gstat.win_cluster[(bestmde >> 24) - 1][besttyp]++;
#endif

#if defined(VERIFY_ENCODER)
  DecompresscolorsBtc7u((std::uint8_t*)rgba, block);
#endif

  return error[0];
}

template <typename dtyp>
void CompresscolorBtc6u(dtyp const* rgb, int mask, void* block, int flags)
{
  static const int modeorder[1][14] = {{
#define MODEORDER_EXPL 0
#define MODEORDER_EXPL_MIN 0
#define MODEORDER_EXPL_MAX 13
    // order: mode (lo to hi)
    kVariableCodingMode1,   //{ 2, 5,   0, { 5, 5, 5}, {5,5,5},  6, 3 },
    kVariableCodingMode2,   //{ 2, 5,   0, { 1, 1, 1}, {6,6,6},  9, 3 },
    kVariableCodingMode3,   //{ 2, 5,   0, { 6, 7, 7}, {5,4,4},  5, 3 },
    kVariableCodingMode4,   //{ 2, 5,   0, { 7, 6, 7}, {4,5,4},  5, 3 },
    kVariableCodingMode5,   //{ 2, 5,   0, { 7, 7, 6}, {4,4,5},  5, 3 },
    kVariableCodingMode6,   //{ 2, 5,   0, { 4, 4, 4}, {5,5,5},  7, 3 },
    kVariableCodingMode7,   //{ 2, 5,   0, { 2, 3, 3}, {6,5,5},  8, 3 },
    kVariableCodingMode8,   //{ 2, 5,   0, { 3, 2, 3}, {5,6,5},  8, 3 },
    kVariableCodingMode9,   //{ 2, 5,   0, { 3, 3, 2}, {5,5,5},  8, 3 },
    kVariableCodingMode10,  //{ 2, 5,   6, { 0, 0, 0}, {0,0,0}, 10, 3 },
    kVariableCodingMode11,  //{ 1, 0,  10, { 0, 0, 0}, {0,0,0},  6, 4 },
    kVariableCodingMode12,  //{ 1, 0,   0, { 2, 2, 2}, {9,9,9},  5, 4 },
    kVariableCodingMode13,  //{ 1, 0,   0, { 4, 4, 4}, {8,8,8},  4, 4 },
    kVariableCodingMode14,  //{ 1, 0,   0, {12,12,12}, {4,4,4},  0, 4 }
  }};

  int numm = flags & (kVariableCodingModes),
      sm = (numm == 0 ? MODEORDER_EXPL_MIN : (numm >> 24) - 1),
      em = (numm == 0 ? MODEORDER_EXPL_MAX : sm),
      om = (numm == 0 ? MODEORDER_EXPL : MODEORDER_EXPL);
  flags &= (~kVariableCodingModes);

#undef MODEORDER_EXPL
#undef MODEORDER_EXPL_MIN
#undef MODEORDER_EXPL_MAX

  // limits sets to 2 and choose the partition freely
  int lmts = 2;
  int lmtp = -1;

  // use the same data-structure all the time
  hdr_set bestpal;
  int bestmde = -1;
  int besttyp = -1;

  Scr3 error(FLT_MAX);

  for (int m = sm; m <= em; m++)
  {
    int mode = modeorder[om][m];
    int mnum = (mode >> 24) - 1;

    // a mode has a specific number of sets, and variable partitions
    int nums = hdr_fit::GetNumSets(mnum);
    int nump = hdr_fit::GetPartitionBits(mnum);
    int numi = hdr_fit::GetIndexBits(mnum);
    numi = numi;

    // stop if set-limit reached
    if (nums > lmts)
      break;

    // lock on the perfect partition
    int sp = (lmtp == -1 ? 0 : lmtp),
        ep = (lmtp == -1 ? (1 << nump) - 1 : lmtp);

    int tb = hdr_fit::GetTruncationBits(mnum);
    int db = hdr_fit::GetDeltaBits(mnum);

    // create the initial point set and quantizer
    hdr_set initial(rgb, mask, flags + mode);
    fQuantizer qnt(tb, db);

    // signal if we do we have anything better this iteration of the search
    bool better = false;

    // search for the best partition
    for (int p = sp; p <= ep; p++)
    {
      // create the minimal point set
      hdr_set palette(initial, mask, flags + mode, p);

#if 0
      // if we see we have less colors than sets, 
      // then back up from trying to test with more sets
      // or even other partitions
      if (palette.GetCount() <= nums)
  lmtp = p, lmts = nums;
#endif

      // do a range fit (which uses single palette fit if appropriate)
      hdr_range_fit fit(&palette, flags + mode);

      // update with old best error (reset IsBest)
      fit.SetError(error);
      fit.Compress(block, qnt, mnum);

      // we could code it lossless, no point in trying any further at all
      if (fit.IsBest())
      {
        if (fit.Lossless())
          return;

        error = fit.GetError();
        if (1)
          bestmde = mode, bestpal = palette, besttyp = 0, better = true;
      }
    }
  }

#if defined(VERIFY_ENCODER)
  DecompressHDRsBtc6u((float*)rgb, block);
#endif
}

/* *****************************************************************************
 */
template <typename dtyp>
void CompressMaskedBitoneCtx1u(dtyp const* rgba, int mask, void* block,
                               int flags)
{
  // get the block locations
  void* colorBlock = block;

  // compress color separately if necessary
  CompressBitoneCtx1u(rgba, mask, colorBlock, flags);
}

template <typename dtyp>
void CompressMaskedcolorBtc1u(dtyp const* rgba, int mask, void* block,
                              int flags)
{
  // get the block locations
  void* colorBlock = block;

  // compress color separately if necessary
  CompresscolorBtc1u(rgba, mask, colorBlock, flags);
}

template <typename dtyp>
void CompressMaskedcolorBtc2u(dtyp const* rgba, int mask, void* block,
                              int flags)
{
  // get the block locations
  void* colorBlock = reinterpret_cast<std::uint8_t*>(block) + 8;
  void* alphaBlock = block;

  // compress color separately if necessary
  CompresscolorBtc1u(rgba, mask, colorBlock, flags);
  // compress alpha separately if necessary
  CompressAlphaBtc2u(rgba, mask, alphaBlock);
}

template <typename dtyp>
void CompressMaskedcolorBtc3u(dtyp const* rgba, int mask, void* block,
                              int flags)
{
  // get the block locations
  void* colorBlock = reinterpret_cast<std::uint8_t*>(block) + 8;
  void* alphaBlock = block;

  // compress color separately if necessary
  CompresscolorBtc1u(rgba, mask, colorBlock, flags);
  // compress alpha separately if necessary
  CompressAlphaBtc3u(rgba, mask, alphaBlock, flags);
}

template <typename dtyp>
void CompressMaskedNormalCtx1u(dtyp const* xyzd, int mask, void* block,
                               int flags)
{
  // get the block locations
  void* normalBlock = block;

  // compress color separately if necessary
  CompressNormalCtx1u(xyzd, mask, normalBlock, flags);
}

template <typename dtyp>
void CompressMaskedNormalBtc1u(dtyp const* xyzd, int mask, void* block,
                               int flags)
{
  // get the block locations
  void* normalBlock = block;

  // compress color separately if necessary
  CompressNormalBtc1u(xyzd, mask, normalBlock, flags);
}

template <typename dtyp>
void CompressMaskedNormalBtc2u(dtyp const* xyzd, int mask, void* block,
                               int flags)
{
  // get the block locations
  void* colorBlock = reinterpret_cast<std::uint8_t*>(block) + 8;
  void* alphaBlock = block;

  // compress color separately if necessary
  CompressNormalBtc1u(xyzd, mask, colorBlock, flags);
  // compress alpha separately if necessary
  CompressAlphaBtc2u(xyzd, mask, alphaBlock);
}

template <typename dtyp>
void CompressMaskedNormalBtc3u(dtyp const* xyzd, int mask, void* block,
                               int flags)
{
  // get the block locations
  void* colorBlock = reinterpret_cast<std::uint8_t*>(block) + 8;
  void* alphaBlock = block;

  // compress color separately if necessary
  CompressNormalBtc1u(xyzd, mask, colorBlock, flags);
  // compress alpha separately if necessary
  CompressAlphaBtc3u(xyzd, mask, alphaBlock, flags);
}

template <typename dtyp>
void CompressMaskedAlphaBtc4u(dtyp const* rgba, int mask, void* block,
                              int flags)
{
  // get the block locations
  void* plane1Block = block;

  // compress a into plane 1
  CompressDepthBtc4u(rgba - 3, mask, plane1Block, flags);
}

template <typename dtyp>
void CompressMaskedAlphaBtc4s(dtyp const* rgba, int mask, void* block,
                              int flags)
{
  // get the block locations
  void* plane1Block = block;

  // compress a into plane 1
  CompressDepthBtc4s(rgba - 3, mask, plane1Block, flags);
}

template <typename dtyp>
void CompressMaskedAlphaBtc5u(dtyp const* rgba, int mask, void* block,
                              int flags)
{
  // get the block locations
  void* plane1Block = reinterpret_cast<std::uint8_t*>(block) + 8;
  void* plane2Block = block;

  // compress a into plane 1
  CompressDepthBtc4u(rgba - 3, mask, plane1Block, flags);
  // compress b into plane 2
  CompressDepthBtc4u(rgba - 2, mask, plane2Block, flags);
}

template <typename dtyp>
void CompressMaskedAlphaBtc5s(dtyp const* rgba, int mask, void* block,
                              int flags)
{
  // get the block locations
  void* plane1Block = reinterpret_cast<std::uint8_t*>(block) + 8;
  void* plane2Block = block;

  // compress a into plane 1
  CompressDepthBtc4s(rgba - 3, mask, plane1Block, flags);
  // compress b into plane 2
  CompressDepthBtc4s(rgba - 2, mask, plane2Block, flags);
}

template <typename dtyp>
void CompressMaskedNormalBtc5u(dtyp const* xyzd, int mask, void* block,
                               int flags)
{
  // get the block locations
  void* plane1Block = reinterpret_cast<std::uint8_t*>(block) + 8;
  void* plane2Block = block;

  // compress xy into plane 1/2
  CompressNormalsBtc5u(xyzd, mask, plane1Block, plane2Block, flags);
}

template <typename dtyp>
void CompressMaskedNormalBtc5s(dtyp const* xyzd, int mask, void* block,
                               int flags)
{
  // get the block locations
  void* plane1Block = reinterpret_cast<std::uint8_t*>(block) + 8;
  void* plane2Block = block;

  // compress xy into plane 1/2
  CompressNormalsBtc5s(xyzd, mask, plane1Block, plane2Block, flags);
}

template <typename dtyp>
void CompressMaskedcolorBtc6u(dtyp const* rgb, int mask, void* block, int flags)
{
  // get the block locations
  void* mixedBlock = block;

  // compress color and alpha merged if necessary
  CompresscolorBtc6u(rgb, mask, mixedBlock, flags);
}

template <typename dtyp>
void CompressMaskedcolorBtc7u(dtyp const* rgba, int mask, void* block,
                              int flags)
{
  // get the block locations
  void* mixedBlock = block;

#ifdef DEBUG_DETAILS
  // compress color and alpha merged if necessary
  fprintf(stderr, "CompressPaletteBtc7uV1\n");
  Scr4 errora = CompressPaletteBtc7uV1<dtyp, palette_range_fit>(
    rgba, mask, mixedBlock, flags);
  fprintf(stderr, "CompressPaletteBtc7uV2\n");
  Scr4 errorb = CompressPaletteBtc7uV2<dtyp, palette_range_fit>(
    rgba, mask, mixedBlock, flags);

  if (errorb > errora)
  {
    bool damn = true;
    damn = false;

    fprintf(stderr, "CompressPaletteBtc7uV2\n");
    errorb = CompressPaletteBtc7uV2<dtyp, palette_range_fit>(rgba, mask,
                                                             mixedBlock, flags);
  }
  else if (errorb < errora)
  {
    bool cool = true;
    cool = false;
  }
#else
  CompressPaletteBtc7uV2<dtyp, palette_range_fit>(rgba, mask, mixedBlock,
                                                  flags);
#endif
}

template <typename dtyp>
void CompressMaskedNormalBtc7u(dtyp const* xyzd, int mask, void* block,
                               int flags)
{
  // get the block locations
  void* mixedBlock = block;

  CompressPaletteBtc7uV2<dtyp, palette_normal_fit>(xyzd, mask, mixedBlock,
                                                   flags);
}

void compress_masked(std::uint8_t const* rgba, int mask, void* block, int flags)
{
  // DXT-type compression
  /**/ if ((flags & (kBtcp | kcolorMetrics)) == (kCtx1 | kcolorMetricUnit))
    CompressMaskedNormalCtx1u(rgba, mask, block, flags);
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc1 | kcolorMetricUnit))
    CompressMaskedNormalBtc1u(rgba, mask, block, flags);
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc2 | kcolorMetricUnit))
    CompressMaskedNormalBtc2u(rgba, mask, block, flags);
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc3 | kcolorMetricUnit))
    CompressMaskedNormalBtc3u(rgba, mask, block, flags);
  // 3Dc-type compression
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc5 | kcolorMetricUnit))
    CompressMaskedNormalBtc5u(rgba, mask, block, flags);
  // BTC-type compression
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc7 | kcolorMetricUnit))
    CompressMaskedNormalBtc7u(rgba, mask, block, flags);

  // DXT-type compression
  else if ((flags & kBtcp) == (kBtc1))
    CompressMaskedcolorBtc1u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc2))
    CompressMaskedcolorBtc2u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc3))
    CompressMaskedcolorBtc3u(rgba, mask, block, flags);
  // ATI-type compression
  else if ((flags & kBtcp) == (kBtc4))
    CompressMaskedAlphaBtc4u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc5))
    CompressMaskedAlphaBtc5u(rgba, mask, block, flags);
  // BTC-type compression
  else if ((flags & kBtcp) == (kBtc7))
    CompressMaskedcolorBtc7u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc6))
  {
  }  // while this is possible (up-cast), should we support it?
}

void compress_masked(std::uint16_t const* rgba, int mask, void* block,
                     int flags)
{
  // DXT-type compression
  /**/ if ((flags & (kBtcp | kcolorMetrics)) == (kCtx1 | kcolorMetricUnit))
    CompressMaskedNormalCtx1u(rgba, mask, block, flags);
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc1 | kcolorMetricUnit))
    CompressMaskedNormalBtc1u(rgba, mask, block, flags);
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc2 | kcolorMetricUnit))
    CompressMaskedNormalBtc2u(rgba, mask, block, flags);
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc3 | kcolorMetricUnit))
    CompressMaskedNormalBtc3u(rgba, mask, block, flags);
  // 3Dc-type compression
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc5 | kcolorMetricUnit))
    CompressMaskedNormalBtc5u(rgba, mask, block, flags);
  // BTC-type compression
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc7 | kcolorMetricUnit))
    CompressMaskedNormalBtc7u(rgba, mask, block, flags);

  // DXT-type compression
  else if ((flags & kBtcp) == (kBtc1))
    CompressMaskedcolorBtc1u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc2))
    CompressMaskedcolorBtc2u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc3))
    CompressMaskedcolorBtc3u(rgba, mask, block, flags);
  // ATI-type compression
  else if ((flags & kBtcp) == (kBtc4))
    CompressMaskedAlphaBtc4u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc5))
    CompressMaskedAlphaBtc5u(rgba, mask, block, flags);
  // BTC-type compression
  else if ((flags & kBtcp) == (kBtc7))
    CompressMaskedcolorBtc7u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc6))
    CompressMaskedcolorBtc6u(rgba, mask, block, flags);
}

void compress_masked(float const* rgba, int mask, void* block, int flags)
{
  // DXT-type compression
  /**/ if ((flags & (kBtcp | kcolorMetrics)) == (kCtx1 | kcolorMetricUnit))
    CompressMaskedNormalCtx1u(rgba, mask, block, flags);
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc1 | kcolorMetricUnit))
    CompressMaskedNormalBtc1u(rgba, mask, block, flags);
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc2 | kcolorMetricUnit))
    CompressMaskedNormalBtc2u(rgba, mask, block, flags);
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc3 | kcolorMetricUnit))
    CompressMaskedNormalBtc1u(rgba, mask, block, flags);
  // 3Dc-type compression
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc5 | kcolorMetricUnit))
    CompressMaskedNormalBtc5u(rgba, mask, block, flags);
  // BTC-type compression
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc7 | kcolorMetricUnit))
    CompressMaskedNormalBtc7u(rgba, mask, block, flags);

  // DXT-type compression
  else if ((flags & kBtcp) == (kBtc1))
    CompressMaskedcolorBtc1u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc2))
    CompressMaskedcolorBtc2u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc3))
    CompressMaskedcolorBtc3u(rgba, mask, block, flags);
  // ATI-type compression
  else if ((flags & kBtcp) == (kBtc4))
    CompressMaskedAlphaBtc4u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc5))
    CompressMaskedAlphaBtc5u(rgba, mask, block, flags);
  // BTC-type compression
  else if ((flags & kBtcp) == (kBtc7))
    CompressMaskedcolorBtc7u(rgba, mask, block, flags);
  else if ((flags & kBtcp) == (kBtc6))
    CompressMaskedcolorBtc6u(rgba, mask, block, flags);
}

void compress(std::uint8_t const* rgba, void* block, int flags)
{
  // compress with full mask
  compress_masked(rgba, -1, block, flags);
}

void compress(std::uint16_t const* rgb, void* block, int flags)
{
  // compress with full mask
  compress_masked(rgb, -1, block, flags);
}

void compress(float const* rgba, void* block, int flags)
{
  // compress with full mask
  compress_masked(rgba, -1, block, flags);
}

/* *****************************************************************************
 */
template <typename dtyp>
void DecompressNormalCtx1u(dtyp* xyzd, void const* block, int flags)
{
  // get the block locations
  void const* normalBlock = block;

  // decompress normals
  DecompressNormalsCtx1u(xyzd, normalBlock);
}

template <typename dtyp>
void DecompressBitoneCtx1u(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* colorBlock = block;

  // decompress color
  DecompressBitonesCtx1u(rgba, colorBlock);
}

template <typename dtyp>
void DecompresscolorBtc1u(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* colorBlock = block;

  // decompress color
  DecompresscolorsBtc1u(rgba, colorBlock, true);
}

template <typename dtyp>
void DecompresscolorBtc2u(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* colorBlock = reinterpret_cast<std::uint8_t const*>(block) + 8;
  void const* alphaBlock = block;

  // decompress color
  DecompresscolorsBtc1u(rgba, colorBlock, false);
  // decompress alpha separately if necessary
  DecompressAlphaBtc2u(rgba, alphaBlock);
}

template <typename dtyp>
void DecompresscolorBtc3u(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* colorBlock = reinterpret_cast<std::uint8_t const*>(block) + 8;
  void const* alphaBlock = block;

  // decompress color
  DecompresscolorsBtc1u(rgba, colorBlock, false);
  // decompress alpha separately if necessary
  DecompressAlphaBtc3u(rgba, alphaBlock, flags);
}

template <typename dtyp>
void DecompressAlphaBtc4u(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* plane1Block = block;

  // decompress plane 1 into a
  DecompressDepthBtc4u(rgba - 3, plane1Block, flags);
}

template <typename dtyp>
void DecompressAlphaBtc4s(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* plane1Block = block;

  // decompress plane 1 into a
  DecompressDepthBtc4s(rgba - 3, plane1Block, flags);
}

template <typename dtyp>
void DecompressAlphaBtc5u(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* plane1Block = reinterpret_cast<std::uint8_t const*>(block) + 8;
  void const* plane2Block = block;

  // decompress plane 1 into a
  DecompressDepthBtc4u(rgba - 3, plane1Block, flags);
  // decompress plane 2 into b
  DecompressDepthBtc4u(rgba - 2, plane2Block, flags);
}

template <typename dtyp>
void DecompressAlphaBtc5s(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* plane1Block = reinterpret_cast<std::uint8_t const*>(block) + 8;
  void const* plane2Block = block;

  // decompress plane 1 into a
  DecompressDepthBtc4s(rgba - 3, plane1Block, flags);
  // decompress plane 2 into b
  DecompressDepthBtc4s(rgba - 2, plane2Block, flags);
}

template <typename dtyp>
void DecompressNormalBtc5u(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* plane1Block = reinterpret_cast<std::uint8_t const*>(block) + 8;
  void const* plane2Block = block;

  // compress xy into plane 1/2
  DecompressNormalsBtc5u(rgba, plane1Block, plane2Block);
}

template <typename dtyp>
void DecompressNormalBtc5s(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* plane1Block = reinterpret_cast<std::uint8_t const*>(block) + 8;
  void const* plane2Block = block;

  // compress xy into plane 1/2
  DecompressNormalsBtc5s(rgba, plane1Block, plane2Block);
}

template <typename dtyp>
void DecompresscolorBtc6u(dtyp* rgb, void const* block, int flags)
{
  // get the block locations
  void const* mixedBlock = block;

  // decompress color and alpha merged if necessary
  DecompressHDRsBtc6u(rgb, mixedBlock);
}

template <typename dtyp>
void DecompresscolorBtc7u(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* mixedBlock = block;

  // decompress color and alpha merged if necessary
  DecompresscolorsBtc7u(rgba, mixedBlock);
}

template <typename dtyp>
void DecompressNormalBtc7u(dtyp* rgba, void const* block, int flags)
{
  // get the block locations
  void const* mixedBlock = block;

  // decompress normals
  DecompressNormalsBtc7u(rgba, mixedBlock);
}

void decompress(std::uint8_t* rgba, void const* block, int flags)
{
  // DXT-type compression
  /**/ if ((flags & (kBtcp | kcolorMetrics)) == (kCtx1 | kcolorMetricUnit))
    DecompressNormalCtx1u(rgba, block, flags);
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc1 | kcolorMetricUnit))
  //  DecompressNormalBtc1u(rgba, block, flags);
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc2 | kcolorMetricUnit))
  //  DecompressNormalBtc2u(rgba, block, flags);
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc3 | kcolorMetricUnit))
  //  DecompressNormalBtc3u(rgba, block, flags);
  // 3Dc-type compression
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc5 | kcolorMetricUnit))
    DecompressNormalBtc5u(rgba, block, flags);
  // BTC-type compression
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc7 | kcolorMetricUnit))
  //  DecompressNormalBtc7u(rgba, block, flags);

  // DXT-type compression
  else if ((flags & kBtcp) == (kBtc1))
    DecompresscolorBtc1u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc2))
    DecompresscolorBtc2u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc3))
    DecompresscolorBtc3u(rgba, block, flags);
  // ATI-type compression
  else if ((flags & kBtcp) == (kBtc4))
    DecompressAlphaBtc4u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc5))
    DecompressAlphaBtc5u(rgba, block, flags);
  // BTC-type compression
  else if ((flags & kBtcp) == (kBtc7))
    DecompresscolorBtc7u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc6))
  {
  }  // while this is possible (down-cast), should we support it?
}

void decompress(std::uint16_t* rgba, void const* block, int flags)
{
  // DXT-type compression
  /**/ if ((flags & (kBtcp | kcolorMetrics)) == (kCtx1 | kcolorMetricUnit))
    DecompressNormalCtx1u(rgba, block, flags);
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc1 | kcolorMetricUnit))
  //  DecompressNormalBtc1u(rgba, block, flags);
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc2 | kcolorMetricUnit))
  //  DecompressNormalBtc2u(rgba, block, flags);
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc3 | kcolorMetricUnit))
  //  DecompressNormalBtc3u(rgba, block, flags);
  // 3Dc-type compression
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc5 | kcolorMetricUnit))
    DecompressNormalBtc5u(rgba, block, flags);
  // BTC-type compression
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc7 | kcolorMetricUnit))
  //  DecompressNormalBtc7u(rgba, block, flags);

  // DXT-type compression
  else if ((flags & kBtcp) == (kBtc1))
    DecompresscolorBtc1u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc2))
    DecompresscolorBtc2u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc3))
    DecompresscolorBtc3u(rgba, block, flags);
  // ATI-type compression
  else if ((flags & kBtcp) == (kBtc4))
    DecompressAlphaBtc4u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc5))
    DecompressAlphaBtc5u(rgba, block, flags);
  // BTC-type compression
  else if ((flags & kBtcp) == (kBtc7))
    DecompresscolorBtc7u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc6))
    DecompresscolorBtc6u(rgba, block, flags);
}

void decompress(float* rgba, void const* block, int flags)
{
  // DXT-type compression
  /**/ if ((flags & (kBtcp | kcolorMetrics)) == (kCtx1 | kcolorMetricUnit))
    DecompressNormalCtx1u(rgba, block, flags);
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc1 | kcolorMetricUnit))
  //  DecompressNormalBtc1u(rgba, block, flags);
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc2 | kcolorMetricUnit))
  //  DecompressNormalBtc2u(rgba, block, flags);
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc3 | kcolorMetricUnit))
  //  DecompressNormalBtc3u(rgba, block, flags);
  // 3Dc-type compression
  else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc5 | kcolorMetricUnit))
    DecompressNormalBtc5u(rgba, block, flags);
  // BTC-type compression
  // else if ((flags & (kBtcp | kcolorMetrics)) == (kBtc7 | kcolorMetricUnit))
  //  DecompressNormalBtc7u(rgba, block, flags);

  // DXT-type compression
  else if ((flags & kBtcp) == (kBtc1))
    DecompresscolorBtc1u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc2))
    DecompresscolorBtc2u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc3))
    DecompresscolorBtc3u(rgba, block, flags);
  // ATI-type compression
  else if ((flags & kBtcp) == (kBtc4))
    DecompressAlphaBtc4u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc5))
    DecompressAlphaBtc5u(rgba, block, flags);
  // BTC-type compression
  else if ((flags & kBtcp) == (kBtc7))
    DecompresscolorBtc7u(rgba, block, flags);
  else if ((flags & kBtcp) == (kBtc6))
    DecompresscolorBtc6u(rgba, block, flags);
}

/* *****************************************************************************
 */
int storage_requirements(int width, int height, int flags)
{
  // compute the storage requirements
  int blockcount = ((width + 3) / 4) * ((height + 3) / 4);
  int blocksize = 16;

  /**/ if ((flags & kBtcp) <= kBtc3)
    blocksize = ((flags & kBtcp) == kBtc1) ? 8 : 16;
  else if ((flags & kBtcp) <= kBtc5)
    blocksize = ((flags & kBtcp) == kBtc4) ? 8 : 16;
  else if ((flags & kBtcp) <= kBtc7)
    blocksize = 16;
  else if ((flags & kBtcp) == kCtx1)
    blocksize = 8;

  return blockcount * blocksize;
}

/* *****************************************************************************
 */
struct sqio squish_io(int width, int height, sqio::dtp datatype, int flags)
{
  struct sqio s;

  s.datatype = datatype;
  s.flags = sanitize_flags(flags);

  // compute the storage requirements
  s.blockcount = ((width + 3) / 4) * ((height + 3) / 4);
  s.blocksize = 16;

  /**/ if ((flags & kBtcp) <= kBtc3)
    s.blocksize = ((flags & kBtcp) == kBtc1) ? 8 : 16;
  else if ((flags & kBtcp) <= kBtc5)
    s.blocksize = ((flags & kBtcp) == kBtc4) ? 8 : 16;
  else if ((flags & kBtcp) <= kBtc7)
    s.blocksize = 16;
  else if ((flags & kBtcp) == kCtx1)
    s.blocksize = 8;

  s.compressedsize = s.blockcount * s.blocksize;
  s.decompressedsize = sizeof(std::uint8_t) * 4 * (width * height);

  if (datatype == sqio::DT_U8)
  {
    // 3Dc-type compression
    /**/ if ((s.flags & (kBtcp | kSignedness | kcolorMetrics)) ==
           (kBtc5 | kSignedExternal | kSignedInternal | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc5s<std::int8_t>,
      s.decoder = (sqio::dec)DecompressNormalBtc5s<std::int8_t>;
    // ATI-type compression
    else if ((s.flags & (kBtcp | kSignedness | kcolorMetrics)) ==
             (kBtc4 | kSignedExternal | kSignedInternal | kcolorMetricUniform))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc4s<std::int8_t>,
      s.decoder = (sqio::dec)DecompressAlphaBtc4s<std::int8_t>;
    else if ((s.flags & (kBtcp | kSignedness | kcolorMetrics)) ==
             (kBtc5 | kSignedExternal | kSignedInternal | kcolorMetricUniform))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc5s<std::int8_t>,
      s.decoder = (sqio::dec)DecompressAlphaBtc5s<std::int8_t>;

    // DXT-type compression
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kCtx1 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalCtx1u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompressNormalCtx1u<std::uint8_t>;
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc1 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc1u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc1u<std::uint8_t>;
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc2 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc2u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc2u<std::uint8_t>;
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc3 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc3u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc3u<std::uint8_t>;
    // 3Dc-type compression
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc5 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc5u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompressNormalBtc5u<std::uint8_t>;
    // BTC-type compression
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc7 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc7u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc7u<std::uint8_t>;

    // DXT-type compression
    else if ((s.flags & kBtcp) == (kCtx1))
      s.encoder = (sqio::enc)CompressMaskedBitoneCtx1u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompressBitoneCtx1u<std::uint8_t>;
    else if ((s.flags & kBtcp) == (kBtc1))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc1u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc1u<std::uint8_t>;
    else if ((s.flags & kBtcp) == (kBtc2))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc2u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc2u<std::uint8_t>;
    else if ((s.flags & kBtcp) == (kBtc3))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc3u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc3u<std::uint8_t>;
    // ATI-type compression
    else if ((s.flags & kBtcp) == (kBtc4))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc4u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompressAlphaBtc4u<std::uint8_t>;
    else if ((s.flags & kBtcp) == (kBtc5))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc5u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompressAlphaBtc5u<std::uint8_t>;
    // BTC-type compression
    else if ((s.flags & kBtcp) == (kBtc7))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc7u<std::uint8_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc7u<std::uint8_t>;
    else if ((s.flags & kBtcp) == (kBtc6))
    {
    }  // while this is possible (down-cast), should we support it?
  }
  else if (datatype == sqio::DT_U16)
  {
    // 3Dc-type compression
    /**/ if ((s.flags & (kBtcp | kSignedness | kcolorMetrics)) ==
           (kBtc5 | kSignedExternal | kSignedInternal | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc5s<std::int16_t>,
      s.decoder = (sqio::dec)DecompressNormalBtc5s<std::int16_t>;
    // ATI-type compression
    else if ((s.flags & (kBtcp | kSignedness | kcolorMetrics)) ==
             (kBtc4 | kSignedExternal | kSignedInternal | kcolorMetricUniform))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc4s<std::int16_t>,
      s.decoder = (sqio::dec)DecompressAlphaBtc4s<std::int16_t>;
    else if ((s.flags & (kBtcp | kSignedness | kcolorMetrics)) ==
             (kBtc5 | kSignedExternal | kSignedInternal | kcolorMetricUniform))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc5s<std::int16_t>,
      s.decoder = (sqio::dec)DecompressAlphaBtc5s<std::int16_t>;

    // DXT-type compression
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kCtx1 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalCtx1u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompressNormalCtx1u<std::uint16_t>;
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc1 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc1u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc1u<std::uint16_t>;
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc2 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc2u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc2u<std::uint16_t>;
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc3 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc3u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc3u<std::uint16_t>;
    // 3Dc-type compression
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc5 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc5u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompressNormalBtc5u<std::uint16_t>;
    // BTC-type compression
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc7 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc7u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc7u<std::uint16_t>;

    // DXT-type compression
    else if ((s.flags & kBtcp) == (kCtx1))
      s.encoder = (sqio::enc)CompressMaskedBitoneCtx1u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompressBitoneCtx1u<std::uint16_t>;
    else if ((s.flags & kBtcp) == (kBtc1))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc1u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc1u<std::uint16_t>;
    else if ((s.flags & kBtcp) == (kBtc2))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc2u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc2u<std::uint16_t>;
    else if ((s.flags & kBtcp) == (kBtc3))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc3u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc3u<std::uint16_t>;
    // ATI-type compression
    else if ((s.flags & kBtcp) == (kBtc4))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc4u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompressAlphaBtc4u<std::uint16_t>;
    else if ((s.flags & kBtcp) == (kBtc5))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc5u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompressAlphaBtc5u<std::uint16_t>;
    // BTC-type compression
    else if ((s.flags & kBtcp) == (kBtc7))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc7u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc7u<std::uint16_t>;
    else if ((s.flags & kBtcp) == (kBtc6))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc6u<std::uint16_t>,
      s.decoder = (sqio::dec)DecompresscolorBtc6u<std::uint16_t>;
  }
  else if (datatype == sqio::DT_F23)
  {
    // 3Dc-type compression
    /**/ if ((s.flags & (kBtcp | kSignedness | kcolorMetrics)) ==
           (kBtc5 | kSignedExternal | kSignedInternal | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc5s<float>,
      s.decoder = (sqio::dec)DecompressNormalBtc5s<float>;
    // ATI-type compression
    else if ((s.flags & (kBtcp | kSignedness | kcolorMetrics)) ==
             (kBtc4 | kSignedExternal | kSignedInternal | kcolorMetricUniform))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc4s<float>,
      s.decoder = (sqio::dec)DecompressAlphaBtc4s<float>;
    else if ((s.flags & (kBtcp | kSignedness | kcolorMetrics)) ==
             (kBtc5 | kSignedExternal | kSignedInternal | kcolorMetricUniform))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc5s<float>,
      s.decoder = (sqio::dec)DecompressAlphaBtc5s<float>;

    // DXT-type compression
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kCtx1 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalCtx1u<float>,
      s.decoder = (sqio::dec)DecompressNormalCtx1u<float>;
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc1 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc1u<float>,
      s.decoder = (sqio::dec)DecompresscolorBtc1u<float>;
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc2 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc2u<float>,
      s.decoder = (sqio::dec)DecompresscolorBtc2u<float>;
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc3 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc3u<float>,
      s.decoder = (sqio::dec)DecompresscolorBtc3u<float>;
    // 3Dc-type compression
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc5 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc5u<float>,
      s.decoder = (sqio::dec)DecompressNormalBtc5u<float>;
    // BTC-type compression
    else if ((s.flags & (kBtcp | kcolorMetrics)) == (kBtc7 | kcolorMetricUnit))
      s.encoder = (sqio::enc)CompressMaskedNormalBtc7u<float>,
      s.decoder = (sqio::dec)DecompresscolorBtc7u<float>;

    // DXT-type compression
    else if ((s.flags & kBtcp) == (kCtx1))
      s.encoder = (sqio::enc)CompressMaskedBitoneCtx1u<float>,
      s.decoder = (sqio::dec)DecompressBitoneCtx1u<float>;
    else if ((s.flags & kBtcp) == (kBtc1))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc1u<float>,
      s.decoder = (sqio::dec)DecompresscolorBtc1u<float>;
    else if ((s.flags & kBtcp) == (kBtc2))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc2u<float>,
      s.decoder = (sqio::dec)DecompresscolorBtc2u<float>;
    else if ((s.flags & kBtcp) == (kBtc3))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc3u<float>,
      s.decoder = (sqio::dec)DecompresscolorBtc3u<float>;
    // ATI-type compression
    else if ((s.flags & kBtcp) == (kBtc4))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc4u<float>,
      s.decoder = (sqio::dec)DecompressAlphaBtc4u<float>;
    else if ((s.flags & kBtcp) == (kBtc5))
      s.encoder = (sqio::enc)CompressMaskedAlphaBtc5u<float>,
      s.decoder = (sqio::dec)DecompressAlphaBtc5u<float>;
    // BTC-type compression
    else if ((s.flags & kBtcp) == (kBtc7))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc7u<float>,
      s.decoder = (sqio::dec)DecompresscolorBtc7u<float>;
    else if ((s.flags & kBtcp) == (kBtc6))
      s.encoder = (sqio::enc)CompressMaskedcolorBtc6u<float>,
      s.decoder = (sqio::dec)DecompresscolorBtc6u<float>;
  }

  return s;
}

void compress_image(std::uint8_t const* rgba, int width, int height,
                    void* blocks, int flags)
{
  // fix any bad flags
  flags = sanitize_flags(flags);

  // initialize the block output
  unsigned char* targetBlock = reinterpret_cast<unsigned char*>(blocks);
  int bytesPerBlock = 16;

  /**/ if ((flags & kBtcp) <= kBtc3)
    bytesPerBlock = ((flags & kBtcp) == kBtc1) ? 8 : 16;
  else if ((flags & kBtcp) <= kBtc5)
    bytesPerBlock = ((flags & kBtcp) == kBtc4) ? 8 : 16;
  else if ((flags & kBtcp) <= kBtc7)
    bytesPerBlock = 16;
  else if ((flags & kBtcp) == kCtx1)
    bytesPerBlock = 8;

  // loop over blocks
  for (int y = 0; y < height; y += 4)
  {
    for (int x = 0; x < width; x += 4)
    {
      // build the 4x4 block of pixels
      std::uint8_t sourceRgba[16 * 4];
      std::uint8_t* targetPixel = sourceRgba;

      int mask = 0;
      for (int py = 0; py < 4; ++py)
      {
        for (int px = 0; px < 4; ++px)
        {
          // get the source pixel in the image
          int sx = x + px;
          int sy = y + py;

          // enable if we're in the image
          if (sx < width && sy < height)
          {
            // copy the rgba value
            std::uint8_t const* sourcePixel = rgba + 4 * (width * sy + sx);
            for (int i = 0; i < 4; ++i)
              *targetPixel++ = *sourcePixel++;

            // enable this pixel
            mask |= (1 << (4 * py + px));
          }
          else
          {
            // skip this pixel as its outside the image
            targetPixel += 4;
          }
        }
      }

      // compress it into the output
      compress_masked(sourceRgba, mask, targetBlock, flags);

      // advance
      targetBlock += bytesPerBlock;
    }
  }
}

void decompress_image(std::uint8_t* rgba, int width, int height,
                      void const* blocks, int flags)
{
  // fix any bad flags
  flags = sanitize_flags(flags);

  // initialize the block input
  unsigned char const* sourceBlock =
    reinterpret_cast<unsigned char const*>(blocks);
  int bytesPerBlock = 16;

  /**/ if ((flags & kBtcp) <= kBtc3)
    bytesPerBlock = ((flags & kBtcp) == kBtc1) ? 8 : 16;
  else if ((flags & kBtcp) <= kBtc5)
    bytesPerBlock = ((flags & kBtcp) == kBtc4) ? 8 : 16;
  else if ((flags & kBtcp) <= kBtc7)
    bytesPerBlock = 16;
  else if ((flags & kBtcp) == kCtx1)
    bytesPerBlock = 8;

  // loop over blocks
  for (int y = 0; y < height; y += 4)
  {
    for (int x = 0; x < width; x += 4)
    {
      // decompress the block
      std::uint8_t targetRgba[4 * 16];

      decompress(targetRgba, sourceBlock, flags);

      // write the decompressed pixels to the correct image locations
      std::uint8_t const* sourcePixel = targetRgba;
      for (int py = 0; py < 4; ++py)
      {
        for (int px = 0; px < 4; ++px)
        {
          // get the target location
          int sx = x + px;
          int sy = y + py;

          if (sx < width && sy < height)
          {
            std::uint8_t* targetPixel = rgba + 4 * (width * sy + sx);

            // copy the rgba value
            for (int i = 0; i < 4; ++i)
              *targetPixel++ = *sourcePixel++;
          }
          else
          {
            // skip this pixel as its outside the image
            sourcePixel += 4;
          }
        }
      }

      // advance
      sourceBlock += bytesPerBlock;
    }
  }
}
}
