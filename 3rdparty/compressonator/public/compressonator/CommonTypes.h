//=====================================================================
// Copyright 2016-2018(c), Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=====================================================================

#ifndef COMMONTYPES_H
#define COMMONTYPES_H

#include <cstdint>

union CMP_COLOR {
  /// The color as an array of components.
  std::uint8_t rgba[4];
  /// The color as a std::uint32_t.
  std::uint32_t asDword;
};

/// Texture format.
/// These enum are all used internally for each version of the SDK the order may
/// change Typically reordered in alpha betical order per catagory for easy
/// referance
enum class cmp_format
{
  Unknown,  /// Undefined texture format.
  // Channel Component formats
  // --------------------------------------------------------------------------------
  /// ARGB format with 8-bit fixed channels.
  ARGB_8888,
  /// ABGR format with 8-bit fixed channels.
  ABGR_8888,
  /// RGBA format with 8-bit fixed channels.
  RGBA_8888,
  /// BGRA format with 8-bit fixed channels.
  BGRA_8888,
  /// RGB format with 8-bit fixed channels.
  RGB_888,
  /// BGR format with 8-bit fixed channels.
  BGR_888,
  /// Two component format with 8-bit fixed channels.
  RG_8,
  /// Single component format with 8-bit fixed channels.
  R_8,
  /// ARGB format with 10-bit fixed channels for color & a 2-bit fixed channel
  /// for alpha.
  ARGB_2101010,
  /// ARGB format with 16-bit fixed channels.
  ARGB_16,
  /// ABGR format with 16-bit fixed channels.
  ABGR_16,
  /// RGBA format with 16-bit fixed channels.
  RGBA_16,
  /// BGRA format with 16-bit fixed channels.
  BGRA_16,
  /// Two component format with 16-bit fixed channels.
  RG_16,
  /// Single component format with 16-bit fixed channels.
  R_16,
  /// RGB format with 9-bit floating point each channel and shared 5 bit
  /// exponent
  RGBE_32F,
  /// ARGB format with 16-bit floating-point channels.
  ARGB_16F,
  /// ABGR format with 16-bit floating-point channels.
  ABGR_16F,
  /// RGBA format with 16-bit floating-point channels.
  RGBA_16F,
  /// BGRA format with 16-bit floating-point channels.
  BGRA_16F,
  /// Two component format with 16-bit floating-point channels.
  RG_16F,
  /// Single component with 16-bit floating-point channels.
  R_16F,
  /// ARGB format with 32-bit floating-point channels.
  ARGB_32F,
  /// ABGR format with 32-bit floating-point channels.
  ABGR_32F,
  /// RGBA format with 32-bit floating-point channels.
  RGBA_32F,
  /// BGRA format with 32-bit floating-point channels.
  BGRA_32F,
  /// RGB format with 32-bit floating-point channels.
  RGB_32F,
  /// BGR format with 32-bit floating-point channels.
  BGR_32F,
  /// Two component format with 32-bit floating-point channels.
  RG_32F,
  /// Single component with 32-bit floating-point channels.
  R_32F,

  // Compression formats:
  /// Single component compression format using the same technique as DXT5
  /// alpha. Four bits per pixel.
  ATI1N,
  /// Two component compression format using the same technique as DXT5 alpha.
  /// Designed for compression of tangent space normal maps. Eight bits per
  /// pixel.
  ATI2N,
  /// Two component compression format using the same technique as DXT5 alpha.
  /// The same as ATI2N but with the channels swizzled. Eight bits per pixel.
  ATI2N_XY,
  /// ATI2N like format using DXT5. Intended for use on GPUs that do not
  /// natively support ATI2N. Eight bits per pixel.
  ATI2N_DXT5,
  /// A compressed RGB format.
  ATC_RGB,
  /// A compressed ARGB format with explicit alpha.
  ATC_RGBA_Explicit,
  /// A compressed ARGB format with interpolated alpha.
  ATC_RGBA_Interpolated,
  /// A four component opaque (or 1-bit alpha) compressed texture format for
  /// Microsoft DirectX10. Identical to DXT1. Four bits per pixel.
  BC1,
  /// A four component compressed texture format with explicit alpha for
  /// Microsoft DirectX10. Identical to DXT3. Eight bits per pixel.
  BC2,
  /// A four component compressed texture format with interpolated alpha for
  /// Microsoft DirectX10. Identical to DXT5. Eight bits per pixel.
  BC3,
  /// A single component compressed texture format for Microsoft DirectX10.
  /// Identical to ATI1N. Four bits per pixel.
  BC4,
  /// A two component compressed texture format for Microsoft DirectX10.
  /// Identical to ATI2N_XY. Eight bits per pixel.
  BC5,
  /// BC6H compressed texture format (UF)
  BC6H,
  /// BC6H compressed texture format (SF)
  BC6H_SF,
  /// BC7  compressed texture format
  BC7,
  /// An DXTC compressed texture matopaque (or 1-bit alpha). Four bits per
  /// pixel.
  DXT1,
  /// DXTC compressed texture format with explicit alpha. Eight bits per pixel.
  DXT3,
  /// DXTC compressed texture format with interpolated alpha. Eight bits per
  /// pixel.
  DXT5,
  /// DXT5 with the red component swizzled into the alpha channel. Eight bits
  /// per pixel.
  DXT5_xGBR,
  /// swizzled DXT5 format with the green component swizzled into the alpha
  /// channel. Eight bits per pixel.
  DXT5_RxBG,
  /// swizzled DXT5 format with the green component swizzled into the alpha
  /// channel & the blue component swizzled into the green channel. Eight bits
  /// per pixel.
  DXT5_RBxG,
  /// swizzled DXT5 format with the green component swizzled into the alpha
  /// channel & the red component swizzled into the green channel. Eight bits
  /// per pixel.
  DXT5_xRBG,
  /// swizzled DXT5 format with the blue component swizzled into the alpha
  /// channel. Eight bits per pixel.
  DXT5_RGxB,
  /// two-component swizzled DXT5 format with the red component swizzled into
  /// the alpha channel & the green component in the green channel. Eight bits
  /// per pixel.
  DXT5_xGxR
};

/// Compress error codes
enum CMP_ERROR
{
  CMP_OK = 0,
  /// The conversion was aborted.
  CMP_ABORTED,
  /// The source texture is invalid.
  CMP_ERR_INVALID_SOURCE_TEXTURE,
  /// The destination texture is invalid.
  CMP_ERR_INVALID_DEST_TEXTURE,
  /// The source format is not a supported format.
  CMP_ERR_UNSUPPORTED_SOURCE_FORMAT,
  /// The destination format is not a supported format.
  CMP_ERR_UNSUPPORTED_DEST_FORMAT,
  /// The gpu hardware is not supported.
  CMP_ERR_UNSUPPORTED_GPU_ASTC_DECODE,
  /// The source and destination texture sizes do not match.
  CMP_ERR_SIZE_MISMATCH,
  /// Compressonator was unable to initialize the codec needed for conversion.
  CMP_ERR_UNABLE_TO_INIT_CODEC,
  /// GPU_Decode Lib was unable to initialize the codec needed for
  /// decompression.
  CMP_ERR_UNABLE_TO_INIT_DECOMPRESSLIB,
  /// Compute Lib was unable to initialize the codec needed for compression.
  CMP_ERR_UNABLE_TO_INIT_COMPUTELIB,
  /// An unknown error occurred.
  CMP_ERR_GENERIC
};

/// The structure describing a texture.
struct CMP_Texture
{
  /// Width of the texture.
  std::uint32_t width;
  /// Height of the texture.
  std::uint32_t height;
  /// Distance to start of next line - necessary only for uncompressed textures.
  std::uint32_t pitch;
  /// Format of the texture.
  cmp_format format;
  /// Size Block to use (Default is 4x4x1).
  std::uint8_t block_height;
  std::uint8_t block_width;
  std::uint8_t block_depth;
  /// Size of the allocated texture data.
  std::uint32_t data_size;
  /// Pointer to the texture data
  std::uint8_t* data;
};

extern "C"
{
  /// CMP_Feedback_Proc
  /// Feedback function for conversion.
  /// \param[in] fProgress The percentage progress of the texture compression.
  /// \param[in] user1 User data as passed to CMP_ConvertTexture.
  /// \param[in] user2 User data as passed to CMP_ConvertTexture.
  /// \return non-nullptr(true) value to abort conversion
  using CMP_Feedback_Proc = bool (*)(float progress, std::size_t user1,
                                     std::size_t user2);
};

#endif
