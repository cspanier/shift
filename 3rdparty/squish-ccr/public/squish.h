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

#ifndef SQUISH_H
#define SQUISH_H

#include <cstdint>
#include "config.h"

/// All squish API functions live in this namespace.
namespace squish
{
enum
{
  /// Use DXT1/BC1 compression.
  kBtc1 = (1u << 0),
  /// Use DXT3/BC2 compression.
  kBtc2 = (2u << 0),
  /// Use DXT5/BC3 compression.
  kBtc3 = (3u << 0),
  /// Use ATI1/BC4 compression.
  kBtc4 = (4u << 0),
  /// Use ATI2/BC5 compression.
  kBtc5 = (5u << 0),
  /// Use BC6H compression.
  kBtc6 = (6u << 0),
  /// Use BC7 compression.
  kBtc7 = (7u << 0),
  /// Use CTX1 compression.
  kCtx1 = (8u << 0),
  /// Use some compression (mask)
  kBtcp = (15u << 0),

  /// Use a perceptual metric for color error (the default).
  kcolorMetricPerceptual = (1u << 4),
  /// Use a uniform metric for color error.
  kcolorMetricUniform = (2u << 4),
  /// Use a unit metric for color error.
  kcolorMetricUnit = (3u << 4),
  /// Use a multi-channel grayscale metric for color error.
  kcolorMetricGray = (4u << 4),
  /// Use a custom metric for color error.
  kcolorMetricCustom = (7u << 4),
  /// Use some metric (mask)
  kcolorMetrics = (7u << 4),

  /// Weight the color by alpha during cluster fit (disabled by default).
  kWeightcolorByAlpha = (1u << 10),
  /// Don't code alpha, set alpha to 255 after weighting (disabled by default).
  kExcludeAlphaFromPalette = (1u << 11),

  /// Transform values/points from signed (disabled by default).
  kSignedExternal = (1u << 12),  // BC4-6
                                 /// Store/restore values/points as signed
                                 /// internally (disabled by default).
  kSignedInternal = (2u << 12),  // BC4-6
                                 /// Use some datatype transform (mask)
  kSignedness = (3u << 12),

  /// Transform values/points from sRGB (disabled by default).
  kSrgbExternal = (1u << 12),  // BC1-3/7
                               /// Store/restore points/values as sRGB
                               /// internally (disabled by default).
  kSrgbInternal = (2u << 12),  // BC1-3/7
                               /// Use some gamma transform (mask)
  kSrgbness = (3u << 12),

  /// Use a fast but low quality color compressor.
  kcolorRangeFit = (1u << 14),
  kAlphaRangeFit = (1u << 14),
  kNormalRangeFit = (1u << 14),
  /// Use a slow but high quality alpha/gray/normal compressor.
  kAlphaIterativeFit = (1u << 15),
  kNormalIterativeFit = (1u << 15),

  /// Use a slow but high quality color compressor (the default).
  kcolorClusterFit = (1u << 16),
  /// Use a very slow but very high quality color compressor.
  kcolorIterativeClusterFit = (8u << 16),
  /// Specify the number of iterations explicitly. You can go until 15.
  kcolorIterativeClusterFit1 = (1u << 16),
  kcolorIterativeClusterFit2 = (2u << 16),
  kcolorIterativeClusterFit4 = (4u << 16),
  kcolorIterativeClusterFit8 = (8u << 16),
  kcolorIterativeClusterFits = (15u << 16),

  /// Use to code a specific BC6/7 mode, coded as "1 + mode-number" (not
  /// specified by default).
  kVariableCodingMode1 = (1u << 24),
  kVariableCodingMode2 = (2u << 24),
  kVariableCodingMode3 = (3u << 24),
  kVariableCodingMode4 = (4u << 24),
  kVariableCodingMode5 = (5u << 24),
  kVariableCodingMode6 = (6u << 24),
  kVariableCodingMode7 = (7u << 24),
  kVariableCodingMode8 = (8u << 24),
  kVariableCodingMode9 = (9u << 24),
  kVariableCodingMode10 = (10u << 24),
  kVariableCodingMode11 = (11u << 24),
  kVariableCodingMode12 = (12u << 24),
  kVariableCodingMode13 = (13u << 24),
  kVariableCodingMode14 = (14u << 24),
  kVariableCodingModes = (15u << 24),

  /// Use to code a specific multi-channel grayscale precision (not specified by
  /// default).
  kVariableCodingBits10 = (1u << 28),  // 4-1+4-1+4     = 10, BC1-3,BC7,CTX1
  kVariableCodingBits13 = (2u << 28),  // 5-1+5-1+5     = 13, BC1-3,BC7,CTX1
  kVariableCodingBits14 = (3u << 28),  // 5-1+6-1+5     = 14, BC1-3,BC7,CTX1
  kVariableCodingBits15 = (4u << 28),  // 8-1+8         = 15, BC7,CTX1
  kVariableCodingBits16 = (5u << 28),  // 6-1+6-1+6     = 16, BC7
  kVariableCodingBits17 = (6u << 28),  // 5-1+5-1+5-1+5 = 17, BC7
  kVariableCodingBits19 = (7u << 28),  // 7-1+7-1+7     = 19, BC7
  kVariableCodingBits22 = (8u << 28),  // 8-1+8-1+8     = 22, BC7
  kVariableCodingBits25 = (9u << 28),  // 7-1+7-1+7-1+7 = 25, BC7
  kVariableCodingBits = (15u << 28),
};

/*! @brief Validates and corrects compressor flags before use.

  @param flags	Compression flags.

  The flags should be verified before use for the compression
  functions as the inner loop does not make any sanity checks.
  Missing or wrongs flags will be set to the defaults.
*/
int sanitize_flags(int flags);

// -----------------------------------------------------------------------------

/*! @brief Compresses a 4x4 block of pixels.

  @param rgba	The rgba values of the 16 source pixels.
  @param block	Storage for the compressed DXT/BTC block.
  @param flags	Compression flags.

  The source pixels should be presented as a contiguous array of 16 rgba
  values, with each component as 1 byte each. In memory this should be:

    { r1, g1, b1, a1, .... , r16, g16, b16, a16 }

  The flags parameter should specify either kBtc1, kBtc2 or kBtc3 compression,
  however, DXT1/BC1 will be used by default if none is specified. When using
  DXT1/BC1 compression, 8 bytes of storage are required for the compressed
  DXT/BTC block. DXT3/BC2 and DXT5/BC3 compression require 16 bytes of storage
  per block.

  The flags parameter can also specify a preferred color compressor and
  color error metric to use when fitting the RGB components of the data.
  Possible color compressors are: kcolorClusterFit (the default),
  kcolorRangeFit or kcolorIterativeClusterFit. Possible color error metrics
  are: kcolorMetricPerceptual (the default) or kcolorMetricUniform. If no
  flags are specified in any particular category then the default will be
  used. Unknown flags are ignored.

  When using kcolorClusterFit, an additional flag can be specified to
  weight the color of each pixel by its alpha value. For images that are
  rendered using alpha blending, this can significantly increase the
  perceived quality.
*/
void compress(std::uint8_t const* rgba, void* block, int flags);
void compress(std::uint16_t const* rgb, void* block, int flags);
void compress(float const* rgba, void* block, int flags);

// -----------------------------------------------------------------------------

/*! @brief Compresses a 4x4 block of pixels.

  @param rgba	The rgba values of the 16 source pixels.
  @param mask	The valid pixel mask.
  @param block	Storage for the compressed DXT/BTC block.
  @param flags	Compression flags.

  The source pixels should be presented as a contiguous array of 16 rgba
  values, with each component as 1 byte each. In memory this should be:

    { r1, g1, b1, a1, .... , r16, g16, b16, a16 }

  The mask parameter enables only certain pixels within the block. The lowest
  bit enables the first pixel and so on up to the 16th bit. Bits beyond the
  16th bit are ignored. Pixels that are not enabled are allowed to take
  arbitrary colors in the output block. An example of how this can be used
  is in the compress_image function to disable pixels outside the bounds of
  the image when the width or height is not divisible by 4.

  The flags parameter should specify either kBtc1, kBtc2 or kBtc3 compression,
  however, DXT1/BC1 will be used by default if none is specified. When using
  DXT1/BC1 compression, 8 bytes of storage are required for the compressed
  DXT/BTC block. DXT3/BC2 and DXT5/BC3 compression require 16 bytes of storage
  per block.

  The flags parameter can also specify a preferred color compressor and
  color error metric to use when fitting the RGB components of the data.
  Possible color compressors are: kcolorClusterFit (the default),
  kcolorRangeFit or kcolorIterativeClusterFit. Possible color error metrics
  are: kcolorMetricPerceptual (the default) or kcolorMetricUniform. If no
  flags are specified in any particular category then the default will be
  used. Unknown flags are ignored.

  When using kcolorClusterFit, an additional flag can be specified to
  weight the color of each pixel by its alpha value. For images that are
  rendered using alpha blending, this can significantly increase the
  perceived quality.
*/
void compress_masked(std::uint8_t const* rgba, int mask, void* block,
                     int flags);
void compress_masked(std::uint16_t const* rgb, int mask, void* block,
                     int flags);
void compress_masked(float const* rgba, int mask, void* block, int flags);

// -----------------------------------------------------------------------------

/*! @brief Decompresses a 4x4 block of pixels.

  @param rgba	Storage for the 16 decompressed pixels.
  @param block	The compressed DXT/BTC block.
  @param flags	Compression flags.

  The decompressed pixels will be written as a contiguous array of 16 rgba
  values, with each component as 1 byte each. In memory this is:

    { r1, g1, b1, a1, .... , r16, g16, b16, a16 }

  The flags parameter should specify either kBtc1, kBtc2 or kBtc3 compression,
  however, DXT1/BC1 will be used by default if none is specified. All other
  flags are ignored.
*/
void decompress(std::uint8_t* rgba, void const* block, int flags);
void decompress(std::uint16_t* rgb, void const* block, int flags);
void decompress(float* rgba, void const* block, int flags);

// -----------------------------------------------------------------------------

struct sqio
{
  enum dtp
  {
    DT_U8,
    DT_U16,
    DT_F23
  };

  int blockcount;
  int blocksize;
  int compressedsize;
  int decompressedsize;

  typedef void (*enc)(void const* rgba, int mask, void* block, int flags);
  typedef void (*dec)(void* rgba, void const* block, int flags);

  dtp datatype;
  int flags;
  enc encoder;
  dec decoder;
};

struct sqio squish_io(int width, int height, sqio::dtp datatype, int flags);
void weights(int flags, const float* rgba);

// -----------------------------------------------------------------------------

/*! @brief Computes the amount of compressed storage required.

  @param width	The width of the image.
  @param height	The height of the image.
  @param flags	Compression flags.

  The flags parameter should specify either kBtc1, kBtc2 or kBtc3 compression,
  however, DXT1/BC1 will be used by default if none is specified. All other
  flags are ignored.

  Most DXT/BTC images will be a multiple of 4 in each dimension, but this
  function supports arbitrary size images by allowing the outer blocks to
  be only partially used.
*/
int storage_requirements(int width, int height, int flags);

// -----------------------------------------------------------------------------

/*! @brief Compresses an image in memory.

  @param rgba	The pixels of the source.
  @param width	The width of the source image.
  @param height	The height of the source image.
  @param blocks	Storage for the compressed output.
  @param flags	Compression flags.

  The source pixels should be presented as a contiguous array of width*height
  rgba values, with each component as 1 byte each. In memory this should be:

    { r1, g1, b1, a1, .... , rn, gn, bn, an } for n = width*height

  The flags parameter should specify either kBtc1, kBtc2 or kBtc3 compression,
  however, DXT1/BC1 will be used by default if none is specified. When using
  DXT1/BC1 compression, 8 bytes of storage are required for each compressed
  DXT/BTC block. DXT3/BC2 and DXT5/BC3 compression require 16 bytes of storage
  per block.

  The flags parameter can also specify a preferred color compressor and
  color error metric to use when fitting the RGB components of the data.
  Possible color compressors are: kcolorClusterFit (the default),
  kcolorRangeFit or kcolorIterativeClusterFit. Possible color error metrics
  are: kcolorMetricPerceptual (the default) or kcolorMetricUniform. If no
  flags are specified in any particular category then the default will be
  used. Unknown flags are ignored.

  When using kcolorClusterFit, an additional flag can be specified to
  weight the color of each pixel by its alpha value. For images that are
  rendered using alpha blending, this can significantly increase the
  perceived quality.

  Internally this function calls squish::Compress for each block. To see how
  much memory is required in the compressed image, use
  squish::storage_requirements.
*/
void compress_image(std::uint8_t const* rgba, int width, int height,
                    void* blocks, int flags);
// void compress_image(std::uint16_t const* rgb, int width, int height,
//                    void* blocks, int flags);
// void compress_image(float const* rgba, int width, int height, void* blocks,
//                    int flags);

// -----------------------------------------------------------------------------

/*! @brief Decompresses an image in memory.

  @param rgba	Storage for the decompressed pixels.
  @param width	The width of the source image.
  @param height	The height of the source image.
  @param blocks	The compressed DXT/BTC blocks.
  @param flags	Compression flags.

  The decompressed pixels will be written as a contiguous array of width*height
  16 rgba values, with each component as 1 byte each. In memory this is:

    { r1, g1, b1, a1, .... , rn, gn, bn, an } for n = width*height

  The flags parameter should specify either kBtc1, kBtc2 or kBtc3 compression,
  however, DXT1/BC1 will be used by default if none is specified. All other
  flags are ignored.

  Internally this function calls squish::Decompress for each block.
*/
void decompress_image(std::uint8_t* rgba, int width, int height,
                      void const* blocks, int flags);
// void decompress_image(std::uint16_t* rgb, int width, int height,
//                     void const* blocks, int flags);
// void decompress_image(float* rgba, int width, int height, void const* blocks,
//                     int flags);
}

#endif
