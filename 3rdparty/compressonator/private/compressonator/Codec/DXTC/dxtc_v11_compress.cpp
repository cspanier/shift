/*
 * ===============================================================================
 *  Copyright (c) 2004-2006 ATI Technologies Inc.
 * ===============================================================================
 *
 * dxtc_v11_compress.c : A high-performance, reasonable quality DXTC compressor
 *
 * PREFACE:
 *
 * All DXTC compressors have a big issue in trading off performance versus image
 * quality. A very high quality image is typically obtained by very many
 * stepwise refinements, looking for the output that contains the lowest error
 * metric. These type of compressors suffer from two major performance issues:
 * first, the need to process each block very many times - in some
 * implementations, the entire block computation is repeated on each iteration -
 * is inherently slow; and secondly, that the performance is not necessarily
 * predictable (in worst-case images, both poor quality and slow results will be
 * obtained.
 *
 *
 *
 * TOWARDS A BETTER SOLUTION:
 *
 * This compressor is an attempt to improve performance by generating a block
 * based on a (mostly) mathematical method (and a pretty simple one at that).
 * The worst-case performance bound of this algorithm is not much more than a
 * factor of 2 worse than the best case (given that it does not hit inherent
 * slowdown conditions such as a lot of denormal operands to the FPU, which is
 * not a major issue).
 *
 * Stepwise refinement is a part of this method, used in a controlled manner to
 * solve a particularly difficult analytical problem (that of the correct
 * mapping of the endpoints to best represent the line). By reducing the
 * refinement to a 1D process this makes it more amenable to rapid
 * implementation (at the cost of a more limited ability to match the source).
 *
 *
 *
 * COMPRESSION ALGORITHM
 *
 * The DXTC compression problem - for compressors avoiding an exhaustive search
 * - is largely divided into two main issues
 *        -    Find the best axis for representing the pixels in the block (the
 * compressed block's pixels must obviously all lie along this axis)
 *        -    Map the points in the block onto this axis in the most efficient
 * manner.
 *
 * Different compressors choose to attack the problem in different ways. In this
 * compressor the two problems are tackled largely totally separately.
 *
 *
 * Finding the axis is performed by a simplistic line-fitting method (using the
 * average and the absolute centred average to find two points and calling this
 * the axis). This provides good results in many circumstances, although it can
 * end up producing poor results where the axis ends up not representative of
 * any of the individual pixels in the block.
 *
 * The pixels are then mapped onto the axis by a nearest-point-of-approach
 * method. The critical value stored is the distance along the axis.
 *
 * In order to correct for cases when the axis has been poorly mapped, the
 * points are clustered (as they would be for the final image) and the average
 * axis mapping error for each cluster is compared. If the error looks bad
 * (typically, in these cases it is large and highly asymmetrical between the
 * two clusters), then the axis is recomputed using the average axis between the
 * two end clusters. This process could be further repeated but this appears
 * reasonably unnecessary - frankly, in the cases where refinement helps it
 * seems unlikely that any very rapid compressor can provide sufficient quality
 * for the image to be definitively usable. In fact, the 'guess' axis has proved
 * sufficiently good in most cases that the limited form I implement this
 * refinement in hurts quality as often as not, and the refinement costs 10-20%
 * in performance so is not worthwhile.
 *
 *
 * Several methods were experimented with for determining the axis endpoints.
 * The best general initial guess was found to be to set the extreme points on
 * the axis to be the resultant block colours and represent those precisely.
 * Although this is not the general 'best solution' it is a very important best
 * solution from a human eye point of view.
 *
 * There is then a stepwise refinement performed that significantly reduces
 * noise by moving these inital endpoints (inwards) and finding the value of
 * minimum error. The compressor can experiment with other modes of this
 * movement, but this doesn't in general provide much higher image quality and
 * does significantly increase compression time.
 *
 *
 * Finally, the RGB565 colour values are computed and the final output is
 * generated. Generating the RGB involves a rounding process that must match up
 * to the inverse unrounding process performed in the decompressor. This also
 * forces the block to be opaque (in the current implementation only 4-colour
 * blocks are generated).
 *
 *
 *
 * OUTPUT BLOCK QUALITY DETERMINATION
 *
 * In order to compute if the compression was sucessful, three error metrics are
 * computed:
 *
 * 1. the axis mapping error produced when mapping from the arbitrary points to
 * the selected axis. This is representative of how poor the colour distortion
 * inherent from the compression is. Since the axis generation bug was fixed,
 * this is actually usually small. Seeing a large axis mapping error almost
 * invariably means that the image is extremely hard to compress.
 * 2. the cluster mapping error, which is the error produced when mapping the
 * points on the axis into the final clusters. This frequently overreacts on
 * highly noisy images.
 * 3. antialiased images fail to trip cluster error, but frequently come out
 * 'lumpy'. This is detected by analysing the clusters on a geometric basis;
 * images that have many blocks with clusters appearing in geometrically ordered
 * patterns tend to be antialiased images.
 *
 *
 *
 * CONCLUSIONS
 *
 * This compressor rapidly produces results that vary from very good on most
 * images highly amenable to compression, to adequate/poor on images not suited
 * to compression. It is never as good as an extremely high quality offline
 * decoder, and should not be considered a substitute for such (although it may
 * be superior to the average offline decoder).
 *
 */

// Further work:
// Add transparent block support
// Add special cases for 1, 2 and some 3 colour blocks (mostly for speed
// reasons) Possibly test if 3 colour blocks would generate better results in
// some cases (done, not apparent in most test images)

#include <cmath>
#include <cassert>
#include <cstring>

#include "compressonator/Codec/DXTC/dxtc_v11_compress.h"

static void DXTCDecompressBlock(std::uint32_t block_32[16],
                                const std::uint32_t block_dxtc[2])
{
  std::uint32_t c0, c1, c2, c3;
  std::uint32_t r0, g0, b0, r1, g1, b1;
  int i;

  c0 = block_dxtc[0] & 0xffff;
  c1 = block_dxtc[0] >> 16;

  if (c0 > c1)
  {
    r0 = ((block_dxtc[0] & 0xf800) >> 8);
    g0 = ((block_dxtc[0] & 0x07e0) >> 3);
    b0 = ((block_dxtc[0] & 0x001f) << 3);

    r1 = ((block_dxtc[0] & 0xf8000000) >> 24);
    g1 = ((block_dxtc[0] & 0x07e00000) >> 19);
    b1 = ((block_dxtc[0] & 0x001f0000) >> 13);

    // Apply the lower bit replication to give full dynamic range
    r0 += (r0 >> 5);
    r1 += (r1 >> 5);
    g0 += (g0 >> 6);
    g1 += (g1 >> 6);
    b0 += (b0 >> 5);
    b1 += (b1 >> 5);

    c0 = (r0 << 16) | (g0 << 8) | b0;
    c1 = (r1 << 16) | (g1 << 8) | b1;
    c2 = (((2 * r0 + r1) / 3) << 16) | (((2 * g0 + g1) / 3) << 8) |
         (((2 * b0 + b1) / 3));
    c3 = (((2 * r1 + r0) / 3) << 16) | (((2 * g1 + g0) / 3) << 8) |
         (((2 * b1 + b0) / 3));

    for (i = 0; i < 16; i++)
    {
      switch ((block_dxtc[1] >> (2 * i)) & 3)
      {
      case 0:
        block_32[i] = c0;
        break;
      case 1:
        block_32[i] = c1;
        break;
      case 2:
        block_32[i] = c2;
        break;
      case 3:
        block_32[i] = c3;
        break;
      }
    }
  }
  else
  {
    // Dont support transparent decode, but have to handle the case when they're
    // both the same
    {
      r0 = ((block_dxtc[0] & 0xf800) >> 8);
      g0 = ((block_dxtc[0] & 0x07e0) >> 3);
      b0 = ((block_dxtc[0] & 0x001f) << 3);

      r1 = ((block_dxtc[0] & 0xf8000000) >> 24);
      g1 = ((block_dxtc[0] & 0x07e00000) >> 19);
      b1 = ((block_dxtc[0] & 0x001f0000) >> 13);

      // Apply the lower bit replication to give full dynamic range
      r0 += (r0 >> 5);
      r1 += (r1 >> 5);
      g0 += (g0 >> 6);
      g1 += (g1 >> 6);
      b0 += (b0 >> 5);
      b1 += (b1 >> 5);

      c0 = (r0 << 16) | (g0 << 8) | b0;
      c1 = (r1 << 16) | (g1 << 8) | b1;
      c2 = (((r0 + r1) / 2) << 16) | (((g0 + g1) / 2) << 8) | (((b0 + b1) / 2));

      for (i = 0; i < 16; i++)
      {
        switch ((block_dxtc[1] >> (2 * i)) & 3)
        {
        case 0:
          block_32[i] = c0;
          break;
        case 1:
          block_32[i] = c1;
          break;
        case 2:
          block_32[i] = c2;
          break;
        case 3:
          block_32[i] = 0xff00ff;
          break;
        }
      }
    }
  }
}

static void DXTCDecompressAlphaBlock(std::uint8_t block_8[16],
                                     const std::uint32_t block_dxtc[2])
{
  std::uint8_t v[8];

  std::uint32_t t;
  int i;

  v[0] = (std::uint8_t)(block_dxtc[0] & 0xff);
  v[1] = (std::uint8_t)((block_dxtc[0] >> 8) & 0xff);

  if (v[0] > v[1])
  {
    // 8-colour block
    for (i = 1; i < 7; i++)
    {
      t = ((7 - i) * v[0] + i * v[1] + 3) / 7;
      v[i + 1] = (std::uint8_t)t;
    }

    for (i = 0; i < 16; i++)
    {
      if (i > 5)
        t = (block_dxtc[1] >> ((3 * (i - 6)) + 2) & 7);
      else if (i == 5)
        t = ((block_dxtc[1] & 3) << 1) + ((block_dxtc[0] >> 31) & 1);
      else
        t = (block_dxtc[0] >> ((3 * i) + 16) & 7);

      block_8[i] = v[t];
    }
  }
  else if (v[0] == v[1])
  {
    for (i = 0; i < 16; i++)
      block_8[i] = v[0];
  }
  else
  {
    assert(0);
  }
}

//#define TRY_3_COLOR

#define MARK_BLOCK                                                \
  {                     /*v_r = v_b = v_g = 0.0f;*/               \
    average_r = 255.0f; /*average_g = 128.0f; average_b = 0.0f;*/ \
  }

//#define AVERAGE_UNIQUE_PIXELS_ONLY        // Even with this the performance
// increase is minor

//#define AXIS_RGB        1
//#define AXIS_YCbCr    1        // Pretty poor in most cases, probably
// shouldn't be used #define AXIS_Y_ONLY    1        // Use for testing;
// generates greyscale output
#define AXIS_MUNGE \
  1  // Raises priority of G at expense of B - seems slightly better than no
     // munging (needs exhaustive testing)
//#define AXIS_MUNGE2    1        // Raises priority of G further at expense of
// R and B - maybe slightly better again...

#if AXIS_RGB

#define CS_RED(r, g, b) (r)
#define CS_GREEN(r, g, b) (g)
#define CS_BLUE(r, g, b) (b)
#define DCS_RED(r, g, b) (r)
#define DCS_GREEN(r, g, b) (g)
#define DCS_BLUE(r, g, b) (b)

#elif AXIS_YCbCr
/*
 *    Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
 *    Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + CENTER
 *    Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + CENTER
 *
 *    R = Y                + 1.40200 * Cr
 *    G = Y - 0.34414 * Cb - 0.71414 * Cr
 *    B = Y + 1.77200 * Cb
 */
#define CS_GREEN(r, g, b) (0.299f * (r) + 0.587f * (g) + 0.114f * (b))
#define CS_RED(r, g, b) (0.5f * (r)-0.41869f * (g)-0.08131f * (b) + 128.0f)
#define CS_BLUE(r, g, b) (-0.16874f * (r)-0.33126f * (g) + 0.5f * (b) + 128.0f)
#define DCS_RED(r, g, b) ((g) + 1.402f * ((r)-128.0f))
#define DCS_GREEN(r, g, b) \
  ((g)-0.34414f * ((b)-128.0f) - 0.71414f * ((r)-128.0f))
#define DCS_BLUE(r, g, b) ((g) + 1.772f * ((b)-128.0f))

#elif AXIS_Y_ONLY
#define CS_GREEN(r, g, b) (0.299f * (r) + 0.587f * (g) + 0.114f * (b))
#define CS_RED(r, g, b) (128.0f)
#define CS_BLUE(r, g, b) (128.0f)
#define DCS_RED(r, g, b) ((g) + 1.402f * ((r)-128.0f))
#define DCS_GREEN(r, g, b) \
  ((g)-0.34414f * ((b)-128.0f) - 0.71414f * ((r)-128.0f))
#define DCS_BLUE(r, g, b) ((g) + 1.772f * ((b)-128.0f))

#elif AXIS_MUNGE

#define CS_RED(r, g, b) (r)
#define CS_GREEN(r, g, b) (g)
#define CS_BLUE(r, g, b) (((b) + (g)) * 0.5f)
#define DCS_RED(r, g, b) (r)
#define DCS_GREEN(r, g, b) (g)
#define DCS_BLUE(r, g, b) ((2.0f * (b)) - (g))

#elif AXIS_MUNGE2

#define CS_RED(r, g, b) ((r + g) * 0.5f)
#define CS_GREEN(r, g, b) (g)
#define CS_BLUE(r, g, b) ((b + 3.0f * g) * 0.25f)
#define DCS_RED(r, g, b) ((2.0f * r) - g)
#define DCS_GREEN(r, g, b) (g)
#define DCS_BLUE(r, g, b) ((4.0f * b) - (3.0f * g))

#else
#error No axis type defined
#endif

#define ROUND_AND_CLAMP(v, shift)          \
  {                                        \
    if ((v) < 0)                             \
      (v) = 0;                               \
    else if ((v) > 255)                      \
      (v) = 255;                             \
    else                                   \
      (v) += (0x80 >> (shift)) - ((v) >> (shift)); \
  }

void DXTCV11CompressExplicitAlphaBlock(const std::uint8_t block_8[16],
                                       std::uint32_t block_dxtc[2])
{
  int i;
  block_dxtc[0] = block_dxtc[1] = 0;
  for (i = 0; i < 16; i++)
  {
    int v = block_8[i];
    v = (v + 7 - (v >> 4));
    v >>= 4;
    if (v < 0)
      v = 0;
    if (v > 0xf)
      v = 0xf;
    if (i < 8)
      block_dxtc[0] |= v << (4 * i);
    else
      block_dxtc[1] |= v << (4 * (i - 8));
  }
}

#ifndef _WIN64

void DXTCV11CompressAlphaBlock(const std::uint8_t block_8[16],
                               std::uint32_t block_dxtc[2])
{
  int i;

  float pos[16];  // The list of colours
  int blocktype;
  float b;
  std::uint32_t n, bit;
  float step, rstep, offset;
  int count_0, count_255;
  float average_inc, average_ex;
  float max_ex, min_ex;

  max_ex = 0;
  min_ex = 255;
  average_inc = average_ex = 0;
  count_0 = count_255 = 0;
  for (i = 0; i < 16; i++)
  {
    int ex = 0;
    if (block_8[i] == 0)
    {
      count_0++;
      ex = 1;
    }
    else if (block_8[i] == 255)
    {
      count_255++;
      ex = 1;
    }

    pos[i] = (float)block_8[i];

    average_inc += pos[i];
    if (!ex)
    {
      average_ex += pos[i];
      if (pos[i] > max_ex)
        max_ex = pos[i];
      if (pos[i] < min_ex)
        min_ex = pos[i];
    }
  }

  // Make assumptions
  if (!count_0 && !count_255)
    blocktype = 8;
  else
  {
    // There are 0 or 255 blocks and we need to represent them
    blocktype = 8;  // Actually should probably try both here

    if (count_0)
      min_ex = 0;
    if (count_255)
      max_ex = 255;
  }

  // Start out assuming our endpoints are the min and max values we've
  // determined If the minimum is 0, it must stay 0, otherwise we shall move
  // inwards cf. the colour compressor

  // Progressive refinement makes very little difference averaged across a whole
  // image, but in certain tricky areas can be noticeably better.

//#define ALPHA_PROGRESSIVE_REFINEMENT
#ifdef ALPHA_PROGRESSIVE_REFINEMENT
  {
    // Attempt a (simple) progressive refinement step to reduce noise in the
    // output image by trying to find a better overall match for the endpoints
    // than the first-guess solution found so far (which is just to take the
    // ends.

    float error, maxerror;
    float oldmin, oldmax;
    int mode, bestmode;
    int first;
    float r, v;

    maxerror = 10000000.0f;
    oldmin = min_ex;
    oldmax = max_ex;
    first = 1;
    do
    {

      for (bestmode = -1, mode = 0; mode < 6;
           mode++)  // Other modes seem more important for alpha block
                    // compression, 4-6 seem broken atm
      {
        if (!first)
        {
          switch (mode)
          {
          case 0:
            min_ex = oldmin + 1.0f;
            max_ex = oldmax - 1.0f;
            break;
          case 1:
            max_ex = oldmax - 1.0f;
            break;
          case 2:
            min_ex = oldmin + 1.0f;
            break;
          case 3:
            min_ex = oldmin - 1.0f;
            max_ex = oldmax + 1.0f;
            break;
          case 4:
            max_ex = oldmax + 1.0f;
            break;
          case 5:
            min_ex = oldmin - 1.0f;
            break;
          }
          if ((min_ex + 1.0f) > max_ex)
            break;
          if ((min_ex < 0.0f) || (max_ex > 255.0f))
            continue;
        }

        error = 0;
        step = (max_ex - min_ex) / (float)(blocktype - 1);
        rstep = 1.0f / step;
        offset = min_ex - (step * 0.5f);

        for (i = 0; i < 16; i++)
        {
          b = pos[i];
          if ((blocktype == 6) && ((b == 0) || (b == 255)))
            continue;

          // Work out which value in the block this selects
          n = (int)((b - offset) * rstep);

          if (n < 0)
            n = 0;
          if (n > 7)
            n = 7;

          // Compute the interpolated value
          v = ((float)n * step) + offset;

          // And accumulate the error
          r = (b - v);
          error += r * r;
        }

        if (error < maxerror)
        {
          maxerror = error;
          bestmode = mode;
        }

        if (first)
          break;
      }
      if (!first)
        switch (bestmode)
        {
        default:
          bestmode = -1;
          break;
        case -1:
          break;
        case 0:
          oldmin += 1.0f;
          oldmax -= 1.0f;
          break;
        case 1:
          oldmax -= 1.0f;
          break;
        case 2:
          oldmin += 1.0f;
          break;
        case 3:
          oldmin -= 1.0f;
          oldmax += 1.0f;
          break;
        case 4:
          oldmax += 1.0f;
          break;
        case 5:
          oldmin -= 1.0f;
          break;
        }
      first = 0;

    } while (bestmode != -1);

    min_ex = oldmin;
    max_ex = oldmax;

    metrics->sum_cluster_errors += maxerror;
    if (maxerror > 2000.0f)
      metrics->high_cluster_error_blocks++;
  }

#endif  // ALPHA_PROGRESSIVE_REFINEMENT

  // Generating the rounded values is slightly arcane.

  if (blocktype == 6)
    block_dxtc[0] = ((int)(min_ex + 0.5f)) | (((int)(max_ex + 0.5f)) << 8);
  else
    block_dxtc[0] = ((int)(max_ex + 0.5f)) | (((int)(min_ex + 0.5f)) << 8);

  step = (max_ex - min_ex) / (float)(blocktype - 1);
  rstep = 1.0f / step;
  offset = min_ex - (step * 0.5f);

  block_dxtc[1] = 0;
  for (i = 0; i < 16; i++)
  {
    b = pos[i];
    if (blocktype == 6)
    {
      if ((b == 0.0f) || (b == 255.0f))
      {
        if (b == 0)
          bit = 6;
        else
          bit = 7;
      }
      else
      {
        // Work out which value in the block this selects
        n = (int)((b - offset) * rstep);

        if (n <= 0)
          bit = 0;
        else if (n >= 5)
          bit = 1;
        else
          bit = n + 1;
      }
    }
    else
    {
      // Blocktype 8
      // Work out which value in the block this selects
      n = (int)((b - offset) * rstep);

      if (n <= 0)
        bit = 1;
      else if (n >= 7)
        bit = 0;
      else
        bit = 8 - n;
    }

    if (i == 5)
    {
      block_dxtc[1] |= (bit >> 1);
      block_dxtc[0] |= (bit & 1) << 31;
    }
    else if (i < 5)
      block_dxtc[0] |= bit << (3 * i + 16);
    else
      block_dxtc[1] |= bit << (3 * (i - 6) + 2);
  }

  // done
}

#endif  // !_WIN64
