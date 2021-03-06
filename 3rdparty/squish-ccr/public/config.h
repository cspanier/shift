/* -----------------------------------------------------------------------------

  Copyright (c) 2006 Simon Brown                          si@sjbrown.co.uk
  Copyright (c) 2012 Niels FrÃ¶hling              niels@paradice-insight.us

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

#ifndef SQUISH_CONFIG_H
#define SQUISH_CONFIG_H

#include <cstdint>

/* define the algorithm to determine the best start/end-point
 * for a single color
 * as Ryg already observed, it's possible to always force the highest
 * precision interpolant to be at index "1" for the euclidian error-
 * distance case [the sole exception being the 4u1, but the improvement
 * there is too mior to justify the loss in speed and the growth of the
 * lookup tables]
 * if you want to use non-euclidian error-distances you can swap your
 * own LUTs in and go back to the iterative fit functions
 *
 * - color_single_snap, color_single_fit
 * - palette_single_snap, palette_single_fit
 * - hdr_single_snap, hdr_single_fit
 */
#define color_single_match color_single_snap
#define palette_single_match palette_single_snap
#define hdr_single_match hdr_single_snap

/* use the power-method to estimate the principle component axis
 * should be more precise if not faster
 */
#define FEATURE_POWERESTIMATE

/* totally blank out any colors in the alpha==0 case when colors
 * are weighted by alpha (which is the indicator too assure the third
 * channel actually really is to be used for alpha or at least contains
 * the importance-factor)
 *
 * we assume the blanked out value receives at least a quantized and
 * rounded alpha value that makes the pixel unperceivable in case it
 * won't be assigned 0 by the matcher (very optimistic but probably
 * better than the "damage" done to the fit-algorithms in case the
 * color gets through)
 *
 * Results (ARGB):
 *
 *          omit               keep
 *   #cluster    R       G       B  J    R       G       B  J
 *   RMSE no
 *m.    2.0099  1.7891  1.9644  1.9235    2.0174  1.7958  1.9753  1.9319 RMSE
 *metric  2.0846  1.6368  2.1797  1.9812    2.0949  1.6444  2.1911  1.9911 SSIM
 *no m.      0.0014593            0.0014612 SSIM metric    0.0012824 0.0012850
 *
 *          omit               keep
 *   #range    R       G       B  J    R       G       B  J
 *   RMSE no
 *m.    3.4200  2.8293  3.1534  3.1435    3.2591  2.7265  3.0184  3.0092 RMSE
 *metric  4.0410  2.3455  3.6444  3.4211    4.0410  2.3455  3.6444  3.4211 SSIM
 *no m.      0.0021402            0.0023996 SSIM metric    0.0020360 0.0023973
 */
#define FEATURE_IGNORE_ALPHA0

/* project the color-values onto the principle component (which is
 * anchored at the centroid) instead of using the just the principle
 * direction
 * problems with the principle direction: if black exist it will always
 * end up as one endpoint (dot-product being 0)
 */
#define FEATURE_RANGEFIT_PROJECT
#define FEATURE_NORMALFIT_PROJECT

/* guarantee that the results of all z-complementing normal-fit algorithms
 * stay inside the unit-sphere
 * end-points, if not selected as index, may still go outside the unit-sphere
 */
#define FEATURE_NORMALFIT_UNITGUARANTEE

/* inline the index-fit error-check, makes it use the minimal amount
 * of instructions possible at the cost of more code (~2x)
 * search all heuristically chosen interpolants for the best one, works
 * only if all checks are inlined
 *
 * first hit improvements:    thorough search:
 *   ib=2: x2.64    ib=2: x8.27  mse: 1.135e-4 vs. 4.221e-5
 *   ib=3: x3.39    ib=3: x7.22  mse: 1.360e-3 vs. 4.924e-4
 *   ib=4: x3.45    ib=4: x47.5  mse: 2.768e-5 vs. 7.118e-6
 */
#define FEATURE_INDEXFIT_INLINED
#define FEATURE_INDEXFIT_THOROUGH true

/* - sqrt() the weights in the colorset, affects all fits
 */
#undef FEATURE_WEIGHTS_ROOTED  // SSIM: 0.0012824 off, 0.0013257 on -
                               // RMSE: 1.9235 off, 1.9540 on

/* - use the metric building the covariance-matrix, affects all fits
 * - sqrt() the metric in the colorset, affects all fits
 *   use linear ( rÂ²*rmetric  +  gÂ²*gmetric  +  bÂ²*bmetric  +  aÂ²*ametric)
 *   instead of ((r*rmetric)Â² + (g*gmetric)Â² + (b*bmetric)Â² + (a*ametric)Â²)
 * - sqr() the metric for least-squares, affects only cluster-fit
 */
#undef FEATURE_METRIC_COVARIANCE  // SSIM: 0.0013466 off, 0.0013596 on
#define FEATURE_METRIC_ROOTED     // SSIM: 0.0006231 off, 0.0006097 on
#define FEATURE_METRIC_SQUARED    // SSIM: 0.0006231 off, 0.0006102 on

/* push start/end values away from the midpoint if the codebook contains
 * less unique entries than possible indices
 * to fill four indices at least one axis need to have an interval of 4/255th
 */
#undef FEATURE_ELIMINATE_FLATBOOKS

/* brute force search for the shared bits with the lowest error
 *
 * 0) ensure shared bit of 1 for opaque merged-alpha cases only (*1)
 * 1) trial shared bits for all cases when block is transparent (*4,*4*2^6)
 * 2) trial shared bits for transparent and low-precision cases
 * (*4,*4*2^6,*4^3*2^4) 3) trial all shared bits 4) check all start/stop up/down
 * combinations (*256 tries), incomplete implementation!
 */
#define SHAREDBITS_TRIAL_ALPHAONLYOPAQUE 0
#define SHAREDBITS_TRIAL_ALPHAONLY 1
#define SHAREDBITS_TRIAL_LOWPRC 2
#define SHAREDBITS_TRIAL_ALL 3
#define SHAREDBITS_TRIAL_PERMUTE 4

#define FEATURE_SHAREDBITS_TRIALS SHAREDBITS_TRIAL_LOWPRC

/* enable the set-builder to detect straight lines of values
 */
#define FEATURE_TEST_LINES

/* .............................................................................
 */

#ifndef NDEBUG

// adjustments working in "Debug" or "Release" builds:
// throw the quantized rgba values back into the input.image
#undef VERIFY_QUANTIZER
// throw the decoded rgba values back into the input-image
#undef VERIFY_ENCODER

// adjustments working only in "Debug" builds:
// code only a specific mode-setting
#undef DEBUG_SETTING
// print out lots of information about the search
#undef DEBUG_DETAILS

// print out lots of information about the algorithm behaviour
#undef TRACK_STATISTICS

#endif  // NDEBUG

#if defined(TRACK_STATISTICS)
namespace squish
{
extern struct statistics
{
  int num_counts[8][64][4][16];
  int num_channels[8][64][4][5];
  int btr_index[5][2];
  int btr_cluster[8][2];
  int win_partition[8][64];
  int win_rotation[8][4];
  int win_swap[8][4][2];
  int win_cluster[8][2];
  int win_mode[8];
  //  int num_lines[4];
  int has_countsets[4];
  int has_noweightsets[8][4][2];
  int num_poweritrs[64];
  int alpha[6];
  float err_index[5][2];
} gstat;
}
#endif

/* -----------------------------------------------------------------------------
 */

// Set to 1 or 2 or 3 or 4 when building squish to use SSE or SSE2, SSE3 or SSE4
// instructions.
#ifndef SQUISH_USE_SSE
#define SQUISH_USE_SSE 0
#endif

// Set to 3 or 4 when building squish to use SSSE3 or SSE4A instructions.
#ifndef SQUISH_USE_XSSE
#define SQUISH_USE_XSSE 0
#endif

// Internally set SQUISH_USE_SIMD when either Altivec or SSE is available.
#if SQUISH_USE_SSE
#define SQUISH_USE_SIMD 1
#define SQUISH_ALIGNED alignas(16)
#else
#define SQUISH_USE_SIMD 0
#define SQUISH_ALIGNED alignas(4)
#endif

#ifndef _MSC_VER
#define assume(x)
#else
#define assume(x) __assume(x)
#endif

#endif
