//=====================================================================
// Copyright (c) 2007-2016    Advanced Micro Devices, Inc. All rights reserved.
// Copyright (c) 2004-2006    ATI Technologies Inc.
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
/// \file Compressonator.h
//
//=====================================================================

#ifndef COMPRESSONATOR_COMPRESSONATOR_H
#define COMPRESSONATOR_COMPRESSONATOR_H

#include <cstdint>
#include "compressonator/Common.h"
#include "compressonator/CommonTypes.h"

struct CMP_ANALYSIS_DATA
{
  double SSIM;  // Structural Similarity Index: Average of RGB Channels
  double SSIM_Red;
  double SSIM_Green;
  double SSIM_Blue;

  double PSNR;  // Peak Signal to Noise Ratio: Average of RGB Channels
  double PSNR_Red;
  double PSNR_Green;
  double PSNR_Blue;

  double MSE;  // Mean Square Error
};

// CMP_HALF and float
// bit-layout for a half number, h:
//
// CMP_HALF (HALF)
//    15 (msb)
//    |
//    | 14  10
//    | |   |
//    | |   | 9        0 (lsb)
//    | |   | |        |
//    X XXXXX XXXXXXXXXX
//
// When converted to 32 Float the bit are as follows
// IEEE 754 single-precision
// floating point number, whose bits are arranged as follows:
//
// float
//    31 (msb)
//    |
//    | 30     23
//    | |      |
//    | |      | 22                    0 (lsb)
//    | |      | |                     |
//    X XXXXXXXX XXXXXXXXXXXXXXXXXXXXXXX
//

/// The major version number of this release.
#define AMD_COMPRESS_VERSION_MAJOR 2
/// The minor version number of this release.
#define AMD_COMPRESS_VERSION_MINOR 6

/// An enum selecting the speed vs. quality trade-off.
enum CMP_Speed
{
  CMP_Speed_Normal,  /// Highest quality mode
  CMP_Speed_Fast,    /// Slightly lower quality but much faster compression mode
                     /// - DXTn & ATInN only
  CMP_Speed_SuperFast,  /// Slightly lower quality but much, much faster
                        /// compression mode - DXTn & ATInN only
};

/// An enum selecting the different GPU driver types.
enum CMP_GPUDecode
{
  GPUDecode_OPENGL = 0,  /// Use OpenGL   to decode Textures (default)
  GPUDecode_DIRECTX,     /// Use DirectX  to decode Textures
  GPUDecode_VULKAN,      /// Use Vulkan  to decode Textures
  GPUDecode_INVALID
};

/// An enum selecting the different GPU driver types.
enum CMP_Compute_type
{
  Compute_CPU_HPC = 0,  /// Use CPU High Performance Compute to compress
                        /// textures, full support (default)
  Compute_OPENCL,       /// Use OpenCL  to compress textures, full support
#ifdef ENABLE_V3x_CODE
  Compute_VULKAN,   /// Use Vulkan  SPIR-V to compress textures, full support
  Compute_DIRECTX,  /// Use DirectX to compress textures, minimal codec support
#endif
  Compute_INVALID
};

#define AMD_MAX_CMDS 20
#define AMD_MAX_CMD_STR 32
#define AMD_MAX_CMD_PARAM 16

#define AMD_CODEC_QUALITY_DEFAULT \
  0.05f  /// This is the default value set for all Codecs (Gives fast
         /// Processing and lowest Quality)
#define AMD_CODEC_EXPOSURE_DEFAULT \
  0  /// This is the default value set for exposure value of hdr/exr input
     /// image
#define AMD_CODEC_DEFOG_DEFAULT \
  0  /// This is the default value set for defog value of hdr/exr input image
#define AMD_CODEC_KNEELOW_DEFAULT \
  0  /// This is the default value set for kneelow value of hdr/exr input image
#define AMD_CODEC_KNEEHIGH_DEFAULT \
  5  /// This is the default value set for kneehigh value of hdr/exr input
     /// image
#define AMD_CODEC_GAMMA_DEFAULT \
  2.2f  /// This is the default value set for gamma value of hdr/exr input
        /// image

#define CMP_MESH_COMP_LEVEL \
  7  /// This is the default value set for draco compress level for mesh
     /// compression
#define CMP_MESH_POS_BITS \
  14  /// This is the default value set for draco position quantization bits
      /// for mesh compression
#define CMP_MESH_TEXC_BITS \
  12  /// This is the default value set for draco texture coordinate
      /// quantization bits for mesh compression
#define CMP_MESH_NORMAL_BITS \
  10  /// This is the default value set for draco normal quantization bits for
      /// mesh compression
#define CMP_MESH_GENERIC_BITS \
  8  /// This is the default value set for draco generic quantization bits for
     /// mesh compression

struct CMP_MAP_BYTES_SET
{
  std::uint8_t B0;
  std::uint8_t B1;
  std::uint8_t B2;
  std::uint8_t B3;
};

struct AMD_CMD_SET
{
  char strCommand[AMD_MAX_CMD_STR];
  char strParameter[AMD_MAX_CMD_PARAM];
};

/// Options for the compression.
/// Passing this structure is optional
struct CMP_CompressOptions
{
  /// The size of this structure.
  std::uint32_t dwSize;

  /// Use channel weightings. With swizzled formats the weighting applies to the
  /// data within the specified channel not the channel itself. channel
  /// weigthing is not implemented for BC6H and BC7
  bool bUseChannelWeighting;

  /// The weighting of the Red or X Channel.
  float fWeightingRed;
  /// The weighting of the Green or Y Channel.
  float fWeightingGreen;
  /// The weighting of the Blue or Z Channel.
  float fWeightingBlue;

  /// Adapt weighting on a per-block basis.
  bool bUseAdaptiveWeighting;

  /// Encode single-bit alpha data. Only valid when compressing to DXT1 & BC1.
  bool bDXT1UseAlpha;

  /// Use GPU to decompress. Decode API can be changed by specified in
  /// DecodeWith parameter. Default is OpenGL.
  bool bUseGPUDecompress;

  /// Use GPU to compress. Encode API can be changed by specified in EncodeWith
  /// parameter. Default is OpenCL.
  bool bUseGPUCompress;

  /// The alpha threshold to use when compressing to DXT1 & BC1 with
  /// bDXT1UseAlpha. Texels with an alpha value less than the threshold are
  /// treated as transparent. Note: When nCompressionSpeed is not set to Normal
  /// AphaThreshold is ignored for DXT1 & BC1
  std::uint8_t nAlphaThreshold;

  /// Disable multi-threading of the compression. This will slow the compression
  /// but can be useful if you're managing threads in your application. if set
  /// BC7 dwnumThreads will default to 1 during encoding and then return back to
  /// its original value when done.
  bool bDisableMultiThreading;

  /// The trade-off between compression speed & quality.
  /// Notes:
  /// 1. This value is ignored for BC6H and BC7 (for BC7 the compression speed
  ///    depends on fquaility value)
  /// 2. For 64 bit DXT1 to DXT5 and BC1 to BC5 nCompressionSpeed is ignored and
  ///    set to Noramal Speed
  /// 3. To force the use of nCompressionSpeed setting regarless of Note 2 use
  ///    fQuality at 0.05
  CMP_Speed nCompressionSpeed;

  /// This value is set using DecodeWith argument (OpenGL, DirectX).
  CMP_GPUDecode nGPUDecode = GPUDecode_OPENGL;

  /// This value is set using ComputeWith argument (OpenGL, DirectX).
  CMP_Compute_type nComputeWith = Compute_OPENCL;

  /// Number of threads to initialize for BC7 encoding (Max up to 128).
  std::uint32_t dwnumThreads = 8;

  /// Quality of encoding. This value ranges between 0.0 and 1.0.
  /// Default set to 0.05 setting fquality above 0.0 gives the
  /// fastest, lowest quality encoding, 1.0 is the slowest,
  /// highest quality encoding. Default set to a low value of 0.05.
  float fquality;

  bool brestrictColour;  /// This setting is a quality tuning setting for BC7
                         /// which may be necessary for convenience in some
                         /// applications. Default set to false
  /// if  set and the block does not need alpha it instructs the code not to
  /// use modes that have combined colour + alpha - this avoids the possibility
  /// that the encoder might choose an alpha other than 1.0 (due to parity) and
  /// cause something to become accidentally slightly transparent (it's
  /// possible that when encoding 3-component texture applications will assume
  /// that the 4th component can safely be assumed to be 1.0 all the time.)
  bool brestrictAlpha;  /// This setting is a quality tuning setting for BC7
                        /// which may be necessary for some textures. Default
                        /// set to false, if set it will also apply restriction
                        /// to blocks with alpha to avoid issues with
                        /// punch-through or thresholded alpha encoding
  std::uint32_t
    dwmodeMask;  /// Mode to set BC7 to encode blocks using any of 8 different
                 /// block modes in order to obtain the highest quality.
                 /// Default set to 0xCF, (Skips Color components with separate
                 /// alpha component) You can combine the bits to test for
                 /// which modes produce the best image quality.
  /// The mode that produces the best image quality above a set quality level
  /// (fquality) is used and subsequent modes set in the mask are not tested,
  /// this optimizes the performance of the compression versus the required
  /// quality. If you prefer to check all modes regardless of the quality then
  /// set the fquality to a value of 0
  int NumCmds;  /// Count of the number of command value pairs in CmdSet[]. Max
                /// value that can be set is AMD_MAX_CMDS = 20 on this release
  AMD_CMD_SET CmdSet
    [AMD_MAX_CMDS];  /// Extended command options that can be set for the
                     /// specified codec\n Example to set the number of threads
                     /// and quality used for compression\n
                     ///        CMP_CompressOptions Options;\n
                     ///        memset(Options,0,sizeof(CMP_CompressOptions));\n
                     ///        Options.dwSize = sizeof(CMP_CompressOptions)\n
                     ///        Options.CmdSet[0].strCommand   = "NumThreads"\n
                     ///        Options.CmdSet[0].strParameter = "8";\n
                     ///        Options.CmdSet[1].strCommand   = "Quality"\n
                     ///        Options.CmdSet[1].strParameter = "1.0";\n
                     ///        Options.NumCmds = 2;\n
  float fInputDefog;  /// ToneMap properties for float type image send into non
                      /// float compress algorithm.
  float fInputExposure;  /// ToneMap properties for float type image send into
                         /// non float compress algorithm.
  float fInputKneeLow;   /// ToneMap properties for float type image send into
                         /// non float compress algorithm.
  float fInputKneeHigh;  /// ToneMap properties for float type image send into
                         /// non float compress algorithm.
  float fInputGamma;  /// ToneMap properties for float type image send into non
                      /// float compress algorithm.

  int iCmpLevel;  /// draco setting: compression level (range 0-10: higher mean
                  /// more compressed) - default 7
  int iPosBits;   /// draco setting: quantization bits for position - default 14
  int iTexCBits;  /// draco setting: quantization bits for texture coordinates
                  /// - default 12
  int iNormalBits;  /// draco setting: quantization bits for normal - default 10
  int
    iGenericBits;  /// draco setting: quantization bits for generic - default 8
};

#define MINIMUM_WEIGHT_VALUE 0.01f

//=================================================================================
// Number of image components
#define BC_COMPONENT_COUNT 4

// Number of bytes in a BC7 Block
#define BC_BLOCK_BYTES (4 * 4)

// Number of pixels in a BC7 block
#define BC_BLOCK_PIXELS BC_BLOCK_BYTES

// This defines the ordering in which components should be packed into
// the block for encoding
enum BC_COMPONENT
{
  BC_COMP_RED = 0,
  BC_COMP_GREEN = 1,
  BC_COMP_BLUE = 2,
  BC_COMP_ALPHA = 3
};

enum BC_ERROR
{
  BC_ERROR_NONE,
  BC_ERROR_LIBRARY_NOT_INITIALIZED,
  BC_ERROR_LIBRARY_ALREADY_INITIALIZED,
  BC_ERROR_INVALID_PARAMETERS,
  BC_ERROR_OUT_OF_MEMORY,
};

class BC7BlockEncoder;
class BC6HBlockEncoder;

//=================================================================================

extern "C"
{
  //=================================================================================
  //
  // InitializeBCLibrary() - Startup the BC6H or BC7 library
  //
  // Must be called before any other library methods are valid
  //
  BC_ERROR CMP_InitializeBCLibrary();

  //
  // ShutdownBCLibrary - Shutdown the BC6H or BC7 library
  //
  BC_ERROR CMP_ShutdownBCLibrary();

  struct CMP_BC6H_BLOCK_PARAMETERS
  {
    std::uint16_t
      dwMask;  // User can enable or disable specific modes default is 0xFFFF
    float fExposure;  // Sets the image lighter (using larger values) or darker
                      // (using lower values) default is 0.95
    bool bIsSigned;   // Specify if half floats are signed or unsigned BC6H_UF16
                      // or BC6H_SF16
    float fQuality;   // Reserved: not used in BC6H at this time
    bool bUsePatternRec;  // Reserved: for new algorithm to use mono pattern
                          // shape matching based on two pixel planes
  };

  //
  // CMP_CreateBC6HEncoder() - Creates an encoder object with the specified
  // quality and settings for BC6H codec CMP_CreateBC7Encoder()  - Creates an
  // encoder object with the specified quality and settings for BC7  codec
  //
  // Library must be initialized before calling this function.
  //
  // Arguments and Settings:
  //
  //      quality       - Quality of encoding. This value ranges between 0.0
  //      and 1.0. (Valid only for BC7 in this release) default is 0.01
  //                      0.0 gives the fastest, lowest quality encoding, 1.0 is
  //                      the slowest, highest quality encoding In general even
  //                      quality level 0.0 will give very good results on the
  //                      vast majority of images Higher quality settings may be
  //                      needed for some difficult images (e.g. normal maps) to
  //                      give good results Encoding time will increase
  //                      significantly at high quality levels. Quality levels
  //                      around 0.8 will give very close to the highest
  //                      possible quality, increasing the level above this will
  //                      cause large increases in encoding time for very
  //                      marginal gains in quality
  //
  //      performance   - Perfromance of encoding. This value ranges between 0.0
  //      and 1.0. (Valid only for BC7 in this release) Typical default is 1.0
  //                      Encoding time can be reduced by incresing this value
  //                      for a given Quality level. Lower values will improve
  //                      overall quality with
  //                        optimal setting been performed at a value of 0.
  //
  //      restrictColor - (for BC7) This setting is a quality tuning setting
  //      which may be necessary for convenience in some applications.
  //                      BC7 can be used for encoding data with up to
  //                      four-components (e.g. ARGB), but the output of a BC7
  //                      decoder
  //                        is effectively always 4-components, even if the
  //                        original input contained less
  //                      If BC7 is used to encode three-component data (e.g.
  //                      RGB) then the encoder generally assumes that it
  //                      doesn't matter what ends up in the 4th component of
  //                      the data, however some applications might be written
  //                      in such a way that they expect the 4th component to
  //                      always be 1.0 (this might, for example, allow mixing
  //                      of textures with and without alpha channels without
  //                      special handling). In this example case the default
  //                      behaviour of the encoder might cause some unexpected
  //                      results, as the alpha channel is not guaranteed to
  //                      always contain exactly 1.0 (since some error may be
  //                      distributed into the 4th channel) If the restrictColor
  //                      flag is set then for any input blocks where the 4th
  //                      component is always 1.0 (255) the encoder will
  //                      restrict itself to using encodings where the
  //                      reconstructed 4th component is also always guaranteed
  //                      to contain 1.0 (255) This may cause a very slight loss
  //                      in overall quality measured in absolute RMS error, but
  //                      this will generally be negligible
  //
  //      restrictAlpha - (for BC7) This setting is a quality tuning setting
  //      which may be necessary for some textures. Some textures may need alpha
  //      values
  //                      of 1.0 and 0.0 to be exactly represented, but some BC7
  //                      block modes distribute error between the colour and
  //                      alpha channels (because they have a shared least
  //                      significant bit in the encoding). This could result in
  //                      the alpha values being pulled away from zero or one by
  //                      the global minimization of the error. If this flag is
  //                      specified then the encoder will restrict its behaviour
  //                      so that for blocks which contain an alpha of zero or
  //                      one then these values should be precisely represented
  //
  //      modeMask      - This is an advanced option. (Valid only for BC7 in
  //      this release)
  //                      BC7 can encode blocks using any of 8 different block
  //                      modes in order to obtain the highest quality (for
  //                      reference of how each of these block modes work
  //                      consult the BC7 specification) Under some
  //                      circumstances it is possible that it might be desired
  //                      to manipulate the encoder to only produce certain
  //                      modes Using this setting it is possible to instruct
  //                      the encoder to only use certain block modes. This
  //                      input is a bitmask of permitted modes for the encoder
  //                      to use - for normal operation it should be set to 0xFF
  //                      (all modes valid) The bitmask is arranged such that a
  //                      setting of 0x1 only allows the encoder to use block
  //                      mode 0. 0x80 would only permit the use of block mode 7
  //                      Restricting the available modes will generally reduce
  //                      quality, but will also increase encoding speed
  //
  //      encoder       - Address of a pointer to an encoder.
  //                      This function will allocate a BC7BlockEncoder or
  //                      BC6HBlockEncoder object using new
  //
  //      isSigned      - For BC6H this flag sets the bit layout, false = UF16
  //      (unsigned float) and true = SF16 (signed float)
  //
  // Note: For BC6H quality and modeMask are reserved for future release
  //
  BC_ERROR CMP_CreateBC6HEncoder(CMP_BC6H_BLOCK_PARAMETERS user_settings,
                                 BC6HBlockEncoder** encoder);
  BC_ERROR CMP_CreateBC7Encoder(double quality, bool restrictColour,
                                bool restrictAlpha, std::uint32_t modeMask,
                                double performance, BC7BlockEncoder** encoder);

  // CMP_EncodeBC7Block()  - Enqueue a single BC7  block to the library for
  // encoding CMP_EncodeBC6HBlock() - Enqueue a single BC6H block to the library
  // for encoding
  //
  // For BC7:
  // Input is expected to be a single 16 element block containing 4 components
  // in the range 0.->255. Pixel data in the block should be arranged in
  // row-major order For three-component input images the 4th component
  // (BC7_COMP_ALPHA) should be set to 255 for all pixels to ensure optimal
  // encoding
  //
  // For BC6H:
  // Input is expected to be a single 16 element block containing 4 components
  // in Half-Float format (16bit). Pixel data in the block should be arranged in
  // row-major order. the 4th component should be set to 0, since Alpha is not
  // supported in BC6H
  //
  BC_ERROR CMP_EncodeBC7Block(BC7BlockEncoder* encoder,
                              double in[BC_BLOCK_PIXELS][BC_COMPONENT_COUNT],
                              std::uint8_t* out);
  BC_ERROR CMP_EncodeBC6HBlock(BC6HBlockEncoder* encoder,
                               float in[BC_BLOCK_PIXELS][BC_COMPONENT_COUNT],
                               std::uint8_t* out);

  //
  // CMP_DecodeBC6HBlock() - Decode a BC6H block to an uncompressed output
  // CMP_DecodeBC7Block()  - Decode a BC7 block to an uncompressed output
  //
  // This function takes a pointer to an encoded BC block as input, decodes it
  // and writes out the result
  //
  //
  BC_ERROR CMP_DecodeBC6HBlock(std::uint8_t* in,
                               float out[BC_BLOCK_PIXELS][BC_COMPONENT_COUNT]);
  BC_ERROR CMP_DecodeBC7Block(std::uint8_t* in,
                              double out[BC_BLOCK_PIXELS][BC_COMPONENT_COUNT]);

  //
  // CMP_DestroyBC6HEncoder() - Deletes a previously allocated encoder object
  // CMP_DestroyBC7Encoder()  - Deletes a previously allocated encoder object
  //
  //
  BC_ERROR CMP_DestroyBC6HEncoder(BC6HBlockEncoder* encoder);
  BC_ERROR CMP_DestroyBC7Encoder(BC7BlockEncoder* encoder);

  //=================================================================================

  /// Calculates the required buffer size for the specified texture
  /// \param[in] pTexture A pointer to the texture.
  /// \return    The size of the buffer required to hold the texture data.
  std::uint32_t CMP_CalculateBufferSize(const CMP_Texture* pTexture);

  /// Converts the source texture to the destination texture
  /// This can be compression, decompression or converting between two
  /// uncompressed formats.
  /// \param[in] pSourceTexture A pointer to the source texture.
  /// \param[in] pDestTexture A pointer to the destination texture.
  /// \param[in] pOptions A pointer to the compression options - can be nullptr.
  /// \param[in] pFeedbackProc A pointer to the feedback function - can be
  /// nullptr. \param[in] pUser1 User data to pass to the feedback function.
  /// \param[in] pUser2 User data to pass to the feedback function.
  /// \return    CMP_OK if successful, otherwise the error code.
  CMP_ERROR CMP_ConvertTexture(CMP_Texture* pSourceTexture,
                               CMP_Texture* pDestTexture,
                               const CMP_CompressOptions* pOptions,
                               CMP_Feedback_Proc pFeedbackProc,
                               std::size_t pUser1, std::size_t pUser2);
};

#endif
