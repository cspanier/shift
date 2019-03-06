//===============================================================================
// Copyright (c) 2007-2016  Advanced Micro Devices, Inc. All rights reserved.
// Copyright (c) 2004-2006 ATI Technologies Inc.
//===============================================================================
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
//
//  File Name:   Codec_DXTC.cpp
//  Description: implementation of the CCodec_DXTC class
//
//////////////////////////////////////////////////////////////////////////////

#include "compressonator/Common.h"
#include "compressonator/Codec/ATI/CompressonatorXCodec.h"
#include "compressonator/Codec/DXTC/Codec_DXTC.h"
#include "compressonator/Codec/DXTC/dxtc_v11_compress.h"

// #define PRINT_DECODE_INFO

CodecError CCodec_DXTC::CompressRGBABlock(
  std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4],
  float* pfChannelWeights)
{
  std::uint8_t alphaBlock[BLOCK_SIZE_4X4];
  for (std::uint32_t i = 0; i < 16; i++)
    alphaBlock[i] = static_cast<std::uint8_t>(((std::uint32_t*)rgbaBlock)[i] >>
                                              RGBA8888_OFFSET_A);

  CodecError err =
    CompressAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  if (err != CE_OK)
    return err;

  return CompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB],
                          pfChannelWeights, false);
}

void CCodec_DXTC::DecompressRGBABlock(std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4],
                                      std::uint32_t compressedBlock[4])
{
  std::uint8_t alphaBlock[BLOCK_SIZE_4X4];

  DecompressAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  DecompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB], false);

  for (std::uint32_t i = 0; i < 16; i++)
    ((std::uint32_t*)rgbaBlock)[i] =
      (alphaBlock[i] << RGBA8888_OFFSET_A) |
      (((std::uint32_t*)rgbaBlock)[i] & ~(BYTE_MASK << RGBA8888_OFFSET_A));
}

CodecError CCodec_DXTC::CompressRGBABlock_ExplicitAlpha(
  std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4],
  float* pfChannelWeights)
{
  std::uint8_t alphaBlock[BLOCK_SIZE_4X4];
  for (std::uint32_t i = 0; i < 16; i++)
    alphaBlock[i] = static_cast<std::uint8_t>(((std::uint32_t*)rgbaBlock)[i] >>
                                              RGBA8888_OFFSET_A);

  CodecError err =
    CompressExplicitAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  if (err != CE_OK)
    return err;

  return CompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB],
                          pfChannelWeights, false);
}

void CCodec_DXTC::DecompressRGBABlock_ExplicitAlpha(
  std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4])
{
  std::uint8_t alphaBlock[BLOCK_SIZE_4X4];

  DecompressExplicitAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  DecompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB], false);

  for (std::uint32_t i = 0; i < 16; i++)
    ((std::uint32_t*)rgbaBlock)[i] =
      (alphaBlock[i] << RGBA8888_OFFSET_A) |
      (((std::uint32_t*)rgbaBlock)[i] & ~(BYTE_MASK << RGBA8888_OFFSET_A));
}

#define ConstructColour(r, g, b) (((r) << 11) | ((g) << 5) | (b))

/*
Channel Bits
*/
#define RG 5
#define GG 6
#define BG 5

CodecError CCodec_DXTC::CompressRGBBlock(
  std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[2],
  float* pfChannelWeights, bool bDXT1, bool bDXT1UseAlpha,
  std::uint8_t nDXT1AlphaThreshold)
{
  /*
  ARGB Channel indexes
  */

  int RC = 2, GC = 1, BC = 0;

  if (m_bSwizzleChannels)
  {
    RC = 0;
    GC = 1;
    BC = 2;
  }

  if (bDXT1 && m_nCompressionSpeed == CMP_Speed_Normal)
  {
    std::uint8_t nEndpoints[2][3][2];
    std::uint8_t nIndices[2][BLOCK_SIZE_4X4];

    double fError3 = CompRGBBlock(
      (std::uint32_t*)rgbBlock, BLOCK_SIZE_4X4, RG, GG, BG, nEndpoints[0],
      nIndices[0], 3, m_bUseSSE2, m_b3DRefinement, m_nRefinementSteps,
      pfChannelWeights, bDXT1UseAlpha, nDXT1AlphaThreshold);
    double fError4 =
      (fError3 == 0.0)
        ? FLT_MAX
        : CompRGBBlock((std::uint32_t*)rgbBlock, BLOCK_SIZE_4X4, RG, GG, BG,
                       nEndpoints[1], nIndices[1], 4, m_bUseSSE2,
                       m_b3DRefinement, m_nRefinementSteps, pfChannelWeights,
                       bDXT1UseAlpha, nDXT1AlphaThreshold);

    unsigned int nMethod = (fError3 <= fError4) ? 0 : 1;
    unsigned int c0 = ConstructColour((nEndpoints[nMethod][RC][0] >> (8 - RG)),
                                      (nEndpoints[nMethod][GC][0] >> (8 - GG)),
                                      (nEndpoints[nMethod][BC][0] >> (8 - BG)));
    unsigned int c1 = ConstructColour((nEndpoints[nMethod][RC][1] >> (8 - RG)),
                                      (nEndpoints[nMethod][GC][1] >> (8 - GG)),
                                      (nEndpoints[nMethod][BC][1] >> (8 - BG)));
    if (nMethod == 1 && c0 <= c1 || nMethod == 0 && c0 > c1)
      compressedBlock[0] = c1 | (c0 << 16);
    else
      compressedBlock[0] = c0 | (c1 << 16);

    compressedBlock[1] = 0;
    for (int i = 0; i < 16; i++)
      compressedBlock[1] |= (nIndices[nMethod][i] << (2 * i));
  }
  else
  {
    std::uint8_t nEndpoints[3][2];
    std::uint8_t nIndices[BLOCK_SIZE_4X4];

    CompRGBBlock((std::uint32_t*)rgbBlock, BLOCK_SIZE_4X4, RG, GG, BG,
                 nEndpoints, nIndices, 4, m_bUseSSE2, m_b3DRefinement,
                 m_nRefinementSteps, pfChannelWeights, bDXT1UseAlpha,
                 nDXT1AlphaThreshold);

    unsigned int c0 = ConstructColour((nEndpoints[RC][0] >> (8 - RG)),
                                      (nEndpoints[GC][0] >> (8 - GG)),
                                      (nEndpoints[BC][0] >> (8 - BG)));
    unsigned int c1 = ConstructColour((nEndpoints[RC][1] >> (8 - RG)),
                                      (nEndpoints[GC][1] >> (8 - GG)),
                                      (nEndpoints[BC][1] >> (8 - BG)));
    if (c0 <= c1)
      compressedBlock[0] = c1 | (c0 << 16);
    else
      compressedBlock[0] = c0 | (c1 << 16);

    compressedBlock[1] = 0;
    for (int i = 0; i < 16; i++)
      compressedBlock[1] |= (nIndices[i] << (2 * i));
  }

  return CE_OK;
}

CodecError CCodec_DXTC::CompressRGBBlock_Fast(
  std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[2])
{
  CompressRGBBlock(rgbBlock, compressedBlock);
  return CE_OK;
}

CodecError CCodec_DXTC::CompressRGBBlock_SuperFast(
  std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[2])
{
  CompressRGBBlock(rgbBlock, compressedBlock);
  return CE_OK;
}

CodecError CCodec_DXTC::CompressRGBABlock_Fast(
  std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4])
{
  std::uint8_t alphaBlock[BLOCK_SIZE_4X4];
  for (std::uint32_t i = 0; i < 16; i++)
    alphaBlock[i] = static_cast<std::uint8_t>(((std::uint32_t*)rgbaBlock)[i] >>
                                              RGBA8888_OFFSET_A);

  CodecError err =
    CompressAlphaBlock_Fast(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  if (err != CE_OK)
    return err;

  return CompressRGBBlock_Fast(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB]);
}

CodecError CCodec_DXTC::CompressRGBABlock_SuperFast(
  std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4])
{
  std::uint8_t alphaBlock[BLOCK_SIZE_4X4];
  for (std::uint32_t i = 0; i < 16; i++)
    alphaBlock[i] = static_cast<std::uint8_t>(((std::uint32_t*)rgbaBlock)[i] >>
                                              RGBA8888_OFFSET_A);

  CodecError err =
    CompressAlphaBlock_Fast(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  if (err != CE_OK)
    return err;

  return CompressRGBBlock_SuperFast(rgbaBlock,
                                    &compressedBlock[DXTC_OFFSET_RGB]);
}

CodecError CCodec_DXTC::CompressRGBABlock_ExplicitAlpha_Fast(
  std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4])
{
  std::uint8_t alphaBlock[BLOCK_SIZE_4X4];
  for (std::uint32_t i = 0; i < 16; i++)
    alphaBlock[i] = static_cast<std::uint8_t>(((std::uint32_t*)rgbaBlock)[i] >>
                                              RGBA8888_OFFSET_A);

  CodecError err = CompressExplicitAlphaBlock_Fast(
    alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  if (err != CE_OK)
    return err;

  return CompressRGBBlock_Fast(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB]);
}

CodecError CCodec_DXTC::CompressRGBABlock_ExplicitAlpha_SuperFast(
  std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4])
{
  std::uint8_t alphaBlock[BLOCK_SIZE_4X4];
  for (std::uint32_t i = 0; i < 16; i++)
    alphaBlock[i] = static_cast<std::uint8_t>(((std::uint32_t*)rgbaBlock)[i] >>
                                              RGBA8888_OFFSET_A);

  CodecError err = CompressExplicitAlphaBlock_Fast(
    alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  if (err != CE_OK)
    return err;

  return CompressRGBBlock_SuperFast(rgbaBlock,
                                    &compressedBlock[DXTC_OFFSET_RGB]);
}

CodecError CCodec_DXTC::CompressRGBABlock(float rgbaBlock[BLOCK_SIZE_4X4X4],
                                          std::uint32_t compressedBlock[4],
                                          float* pfChannelWeights)
{
  float alphaBlock[BLOCK_SIZE_4X4];
  for (std::uint32_t i = 0; i < 16; i++)
    alphaBlock[i] = rgbaBlock[(i * 4) + RGBA32F_OFFSET_A];

  CodecError err =
    CompressAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  if (err != CE_OK)
    return err;

  return CompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB],
                          pfChannelWeights, false);
}

CodecError CCodec_DXTC::CompressRGBABlock_ExplicitAlpha(
  float rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4],
  float* pfChannelWeights)
{
  float alphaBlock[BLOCK_SIZE_4X4];
  for (std::uint32_t i = 0; i < 16; i++)
    alphaBlock[i] = rgbaBlock[(i * 4) + RGBA32F_OFFSET_A];

  CodecError err =
    CompressExplicitAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  if (err != CE_OK)
    return err;

  return CompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB],
                          pfChannelWeights, false);
}

CodecError CCodec_DXTC::CompressRGBBlock(float rgbBlock[BLOCK_SIZE_4X4X4],
                                         std::uint32_t compressedBlock[2],
                                         float* pfChannelWeights, bool bDXT1,
                                         bool bDXT1UseAlpha,
                                         float fDXT1AlphaThreshold)
{
  /*
  ARGB Channel indexes
  */

  int RC = 2, GC = 1, BC = 0;

  if (m_bSwizzleChannels)
  {
    RC = 0;
    GC = 1;
    BC = 2;
  }

  if (bDXT1)
  {
    std::uint8_t nEndpoints[2][3][2];
    std::uint8_t nIndices[2][BLOCK_SIZE_4X4];

    double fError3 = CompRGBBlock(
      rgbBlock, BLOCK_SIZE_4X4, RG, GG, BG, nEndpoints[0], nIndices[0], 3,
      m_bUseSSE2, m_b3DRefinement, m_nRefinementSteps, pfChannelWeights,
      bDXT1UseAlpha, fDXT1AlphaThreshold);
    double fError4 =
      (fError3 == 0.0)
        ? FLT_MAX
        : CompRGBBlock(rgbBlock, BLOCK_SIZE_4X4, RG, GG, BG, nEndpoints[1],
                       nIndices[1], 4, m_bUseSSE2, m_b3DRefinement,
                       m_nRefinementSteps, pfChannelWeights, bDXT1UseAlpha,
                       fDXT1AlphaThreshold);

    unsigned int nMethod = (fError3 <= fError4) ? 0 : 1;
    unsigned int c0 = ConstructColour((nEndpoints[nMethod][RC][0] >> (8 - RG)),
                                      (nEndpoints[nMethod][GC][0] >> (8 - GG)),
                                      (nEndpoints[nMethod][BC][0] >> (8 - BG)));
    unsigned int c1 = ConstructColour((nEndpoints[nMethod][RC][1] >> (8 - RG)),
                                      (nEndpoints[nMethod][GC][1] >> (8 - GG)),
                                      (nEndpoints[nMethod][BC][1] >> (8 - BG)));
    if (nMethod == 1 && c0 <= c1 || nMethod == 0 && c0 > c1)
      compressedBlock[0] = c1 | (c0 << 16);
    else
      compressedBlock[0] = c0 | (c1 << 16);

    compressedBlock[1] = 0;
    for (int i = 0; i < 16; i++)
      compressedBlock[1] |= (nIndices[nMethod][i] << (2 * i));
  }
  else
  {
    std::uint8_t nEndpoints[3][2];
    std::uint8_t nIndices[BLOCK_SIZE_4X4];

    CompRGBBlock(rgbBlock, BLOCK_SIZE_4X4, RG, GG, BG, nEndpoints, nIndices, 4,
                 m_bUseSSE2, m_b3DRefinement, m_nRefinementSteps,
                 pfChannelWeights, bDXT1UseAlpha, fDXT1AlphaThreshold);

    unsigned int c0 = ConstructColour((nEndpoints[RC][0] >> (8 - RG)),
                                      (nEndpoints[GC][0] >> (8 - GG)),
                                      (nEndpoints[BC][0] >> (8 - BG)));
    unsigned int c1 = ConstructColour((nEndpoints[RC][1] >> (8 - RG)),
                                      (nEndpoints[GC][1] >> (8 - GG)),
                                      (nEndpoints[BC][1] >> (8 - BG)));
    if (c0 <= c1)
      compressedBlock[0] = c1 | (c0 << 16);
    else
      compressedBlock[0] = c0 | (c1 << 16);

    compressedBlock[1] = 0;
    for (int i = 0; i < 16; i++)
      compressedBlock[1] |= (nIndices[i] << (2 * i));
  }
  return CE_OK;
}

void CCodec_DXTC::DecompressRGBABlock(float rgbaBlock[BLOCK_SIZE_4X4X4],
                                      std::uint32_t compressedBlock[4])
{
  float alphaBlock[BLOCK_SIZE_4X4];

  DecompressAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  DecompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB], false);

  for (std::uint32_t i = 0; i < 16; i++)
    rgbaBlock[(i * 4) + RGBA32F_OFFSET_A] = alphaBlock[i];
}

void CCodec_DXTC::DecompressRGBABlock_ExplicitAlpha(
  float rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4])
{
  float alphaBlock[BLOCK_SIZE_4X4];

  DecompressExplicitAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
  DecompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB], false);

  for (std::uint32_t i = 0; i < 16; i++)
    rgbaBlock[(i * 4) + RGBA32F_OFFSET_A] = alphaBlock[i];
}

// This function decompresses a DXT colour block
// The block is decompressed to 8 bits per channel
void CCodec_DXTC::DecompressRGBBlock(std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4],
                                     std::uint32_t compressedBlock[2],
                                     bool bDXT1)
{
  std::uint32_t n0 = compressedBlock[0] & 0xffff;
  std::uint32_t n1 = compressedBlock[0] >> 16;
  std::uint32_t r0;
  std::uint32_t g0;
  std::uint32_t b0;
  std::uint32_t r1;
  std::uint32_t g1;
  std::uint32_t b1;

  if (m_bSwizzleChannels)
  {
    b0 = ((n0 & 0xf800) >> 8);
    g0 = ((n0 & 0x07e0) >> 3);
    r0 = ((n0 & 0x001f) << 3);

    b1 = ((n1 & 0xf800) >> 8);
    g1 = ((n1 & 0x07e0) >> 3);
    r1 = ((n1 & 0x001f) << 3);
  }
  else
  {
    r0 = ((n0 & 0xf800) >> 8);
    g0 = ((n0 & 0x07e0) >> 3);
    b0 = ((n0 & 0x001f) << 3);

    r1 = ((n1 & 0xf800) >> 8);
    g1 = ((n1 & 0x07e0) >> 3);
    b1 = ((n1 & 0x001f) << 3);
  }

  // Apply the lower bit replication to give full dynamic range
  r0 += (r0 >> 5);
  r1 += (r1 >> 5);
  g0 += (g0 >> 6);
  g1 += (g1 >> 6);
  b0 += (b0 >> 5);
  b1 += (b1 >> 5);

  std::uint32_t c0 = 0xff000000 | (r0 << 16) | (g0 << 8) | b0;
  std::uint32_t c1 = 0xff000000 | (r1 << 16) | (g1 << 8) | b1;

#ifdef PRINT_DECODE_INFO
  FILE* gt_File_decode = fopen("decode_patterns.txt", "a");
#endif

  if (!bDXT1 || n0 > n1)
  {

#ifdef PRINT_DECODE_INFO
    fprintf(gt_File_decode,
            "BC1               : C0(%3d,%3d,%3d) C1(%3d,%3d,%3d) "
            "A0[%3d,%3d:%3d] B0[%3d:%3d:%3d] index = ",
            r0, g0, b0, r1, g1, b1, ((2 * r0 + r1 + 1) / 3),
            ((2 * g0 + g1 + 1) / 3), ((2 * b0 + b1 + 1) / 3),
            ((2 * r1 + r0 + 1) / 3), ((2 * g1 + g0 + 1) / 3),
            ((2 * b1 + b0 + 1) / 3));
#endif

    std::uint32_t c2 = 0xff000000 | (((2 * r0 + r1 + 1) / 3) << 16) |
                       (((2 * g0 + g1 + 1) / 3) << 8) |
                       (((2 * b0 + b1 + 1) / 3));
    std::uint32_t c3 = 0xff000000 | (((2 * r1 + r0 + 1) / 3) << 16) |
                       (((2 * g1 + g0 + 1) / 3) << 8) |
                       (((2 * b1 + b0 + 1) / 3));

    for (int i = 0; i < 16; i++)
    {
      int index = (compressedBlock[1] >> (2 * i)) & 3;
#ifdef PRINT_DECODE_INFO
      fprintf(gt_File_decode, "%2d,", index);
#endif
      switch (index)
      {
      case 0:
        ((std::uint32_t*)rgbBlock)[i] = c0;
        break;
      case 1:
        ((std::uint32_t*)rgbBlock)[i] = c1;
        break;
      case 2:
        ((std::uint32_t*)rgbBlock)[i] = c2;
        break;
      case 3:
        ((std::uint32_t*)rgbBlock)[i] = c3;
        break;
      }
    }
  }
  else
  {

#ifdef PRINT_DECODE_INFO
    fprintf(gt_File_decode,
            "BC1T..............: C0(%3d,%3d,%3d) C1(%3d,%3d,%3d) "
            "A0[%3d,%3d,%3d]                 index = ",
            r0, g0, b0, r1, g1, b1, ((r0 + r1) / 2), ((g0 + g1) / 2),
            ((b0 + b1) / 2));
#endif
    // Transparent decode
    std::uint32_t c2 = 0xff000000 | (((r0 + r1) / 2) << 16) |
                       (((g0 + g1) / 2) << 8) | (((b0 + b1) / 2));

    for (int i = 0; i < 16; i++)
    {
      int index = (compressedBlock[1] >> (2 * i)) & 3;
#ifdef PRINT_DECODE_INFO
      fprintf(gt_File_decode, "%2d,", index);
#endif
      switch (index)
      {
      case 0:
        ((std::uint32_t*)rgbBlock)[i] = c0;
        break;
      case 1:
        ((std::uint32_t*)rgbBlock)[i] = c1;
        break;
      case 2:
        ((std::uint32_t*)rgbBlock)[i] = c2;
        break;
      case 3:
        ((std::uint32_t*)rgbBlock)[i] = 0x00000000;
        break;
      }
    }
  }
#ifdef PRINT_DECODE_INFO
  fprintf(gt_File_decode, "\n");
  fclose(gt_File_decode);
#endif
}

//
// This function decompresses a DXT colour block
// The block is decompressed to 8 bits per channel
//
void CCodec_DXTC::DecompressRGBBlock(float rgbBlock[BLOCK_SIZE_4X4X4],
                                     std::uint32_t compressedBlock[2],
                                     bool bDXT1)
{
  std::uint32_t n0 = compressedBlock[0] & 0xffff;
  std::uint32_t n1 = compressedBlock[0] >> 16;
  std::uint32_t r0;
  std::uint32_t g0;
  std::uint32_t b0;
  std::uint32_t r1;
  std::uint32_t g1;
  std::uint32_t b1;

  if (m_bSwizzleChannels)
  {
    b0 = ((n0 & 0xf800) >> 8);
    g0 = ((n0 & 0x07e0) >> 3);
    r0 = ((n0 & 0x001f) << 3);

    b1 = ((n1 & 0xf800) >> 8);
    g1 = ((n1 & 0x07e0) >> 3);
    r1 = ((n1 & 0x001f) << 3);
  }
  else
  {
    r0 = ((n0 & 0xf800) >> 8);
    g0 = ((n0 & 0x07e0) >> 3);
    b0 = ((n0 & 0x001f) << 3);

    r1 = ((n1 & 0xf800) >> 8);
    g1 = ((n1 & 0x07e0) >> 3);
    b1 = ((n1 & 0x001f) << 3);
  }

  // Apply the lower bit replication to give full dynamic range
  r0 += (r0 >> 5);
  r1 += (r1 >> 5);
  g0 += (g0 >> 6);
  g1 += (g1 >> 6);
  b0 += (b0 >> 5);
  b1 += (b1 >> 5);

#ifdef PRINT_DECODE_INFO
  FILE* gt_File_decode = fopen("decode_patterns.txt", "a");
#endif

  float c0[4], c1[4], c2[4], c3[4];
  c0[RGBA32F_OFFSET_A] = 1.0;
  c0[RGBA32F_OFFSET_R] = CONVERT_BYTE_TO_FLOAT(r0);
  c0[RGBA32F_OFFSET_G] = CONVERT_BYTE_TO_FLOAT(g0);
  c0[RGBA32F_OFFSET_B] = CONVERT_BYTE_TO_FLOAT(b0);

  c1[RGBA32F_OFFSET_A] = 1.0;
  c1[RGBA32F_OFFSET_R] = CONVERT_BYTE_TO_FLOAT(r1);
  c1[RGBA32F_OFFSET_G] = CONVERT_BYTE_TO_FLOAT(g1);
  c1[RGBA32F_OFFSET_B] = CONVERT_BYTE_TO_FLOAT(b1);

  if (!bDXT1 || n0 > n1)
  {
#ifdef PRINT_DECODE_INFO
    fprintf(gt_File_decode,
            "BC1A              : C0(%3d,%3d,%3d) C1(%3d,%3d,%3d)               "
            "                  index = ",
            r0, g0, b0, r1, g1, b1);
#endif

    c2[RGBA32F_OFFSET_A] = 1.0;
    c2[RGBA32F_OFFSET_R] =
      ((2 * c0[RGBA32F_OFFSET_R] + c1[RGBA32F_OFFSET_R]) / 3);
    c2[RGBA32F_OFFSET_G] =
      ((2 * c0[RGBA32F_OFFSET_G] + c1[RGBA32F_OFFSET_G]) / 3);
    c2[RGBA32F_OFFSET_B] =
      ((2 * c0[RGBA32F_OFFSET_B] + c1[RGBA32F_OFFSET_B]) / 3);

    c3[RGBA32F_OFFSET_A] = 1.0;
    c3[RGBA32F_OFFSET_R] =
      ((2 * c1[RGBA32F_OFFSET_R] + c0[RGBA32F_OFFSET_R]) / 3);
    c3[RGBA32F_OFFSET_G] =
      ((2 * c1[RGBA32F_OFFSET_G] + c0[RGBA32F_OFFSET_G]) / 3);
    c3[RGBA32F_OFFSET_B] =
      ((2 * c1[RGBA32F_OFFSET_B] + c0[RGBA32F_OFFSET_B]) / 3);

    for (int i = 0; i < 16; i++)
    {

      int index = (compressedBlock[1] >> (2 * i)) & 3;
#ifdef PRINT_DECODE_INFO
      fprintf(gt_File_decode, "%2d,", index);
#endif
      switch (index)
      {
      case 0:
        memcpy(&rgbBlock[i * 4], c0, 4 * sizeof(float));
        break;
      case 1:
        memcpy(&rgbBlock[i * 4], c1, 4 * sizeof(float));
        break;
      case 2:
        memcpy(&rgbBlock[i * 4], c2, 4 * sizeof(float));
        break;
      case 3:
        memcpy(&rgbBlock[i * 4], c3, 4 * sizeof(float));
        break;
      }
    }
  }
  else
  {

#ifdef PRINT_DECODE_INFO
    fprintf(gt_File_decode,
            "BC1AT             : C0(%3d,%3d,%3d) C1(%3d,%3d,%3d)               "
            "                  index = ",
            r0, g0, b0, r1, g1, b1);
#endif

    // Transparent decode
    c2[RGBA32F_OFFSET_A] = 1.0;
    c2[RGBA32F_OFFSET_R] = ((c0[RGBA32F_OFFSET_R] + c1[RGBA32F_OFFSET_R]) / 2);
    c2[RGBA32F_OFFSET_G] = ((c0[RGBA32F_OFFSET_G] + c1[RGBA32F_OFFSET_G]) / 2);
    c2[RGBA32F_OFFSET_B] = ((c0[RGBA32F_OFFSET_B] + c1[RGBA32F_OFFSET_B]) / 2);

    c3[RGBA32F_OFFSET_A] = 0.0;
    c3[RGBA32F_OFFSET_R] = 0.0;
    c3[RGBA32F_OFFSET_G] = 0.0;
    c3[RGBA32F_OFFSET_B] = 0.0;

    for (int i = 0; i < 16; i++)
    {

      int index = (compressedBlock[1] >> (2 * i)) & 3;
#ifdef PRINT_DECODE_INFO
      fprintf(gt_File_decode, "%2d,", index);
#endif

      switch (index)
      {
      case 0:
        memcpy(&rgbBlock[i * 4], c0, 4 * sizeof(float));
        break;
      case 1:
        memcpy(&rgbBlock[i * 4], c1, 4 * sizeof(float));
        break;
      case 2:
        memcpy(&rgbBlock[i * 4], c2, 4 * sizeof(float));
        break;
      case 3:
        memcpy(&rgbBlock[i * 4], c3, 4 * sizeof(float));
        break;
      }
    }
  }
#ifdef PRINT_DECODE_INFO
  fprintf(gt_File_decode, "\n");
  fclose(gt_File_decode);
#endif
}

float* CCodec_DXTC::CalculateColourWeightings(
  const std::uint8_t block[BLOCK_SIZE_4X4X4])
{
  if (!m_bUseChannelWeighting)
    return nullptr;

  if (m_bUseAdaptiveWeighting)
  {
    float medianR = 0.0f, medianG = 0.0f, medianB = 0.0f;

    for (std::uint32_t k = 0; k < BLOCK_SIZE_4X4; k++)
    {
      std::uint32_t R = (block[k] & 0xff0000) >> 16;
      std::uint32_t G = (block[k] & 0xff00) >> 8;
      std::uint32_t B = block[k] & 0xff;

      medianR += R;
      medianG += G;
      medianB += B;
    }

    medianR /= BLOCK_SIZE_4X4;
    medianG /= BLOCK_SIZE_4X4;
    medianB /= BLOCK_SIZE_4X4;

    // Now skew the colour weightings based on the gravity center of the block
    float largest = max(max(medianR, medianG), medianB);

    if (largest > 0)
    {
      medianR /= largest;
      medianG /= largest;
      medianB /= largest;
    }
    else
      medianR = medianG = medianB = 1.0f;

    // Scale weightings back up to 1.0f
    float fWeightScale =
      1.0f / (m_fBaseChannelWeights[0] + m_fBaseChannelWeights[1] +
              m_fBaseChannelWeights[2]);
    m_fChannelWeights[0] = m_fBaseChannelWeights[0] * fWeightScale;
    m_fChannelWeights[1] = m_fBaseChannelWeights[1] * fWeightScale;
    m_fChannelWeights[2] = m_fBaseChannelWeights[2] * fWeightScale;
    m_fChannelWeights[0] =
      ((m_fChannelWeights[0] * 3 * medianR) + m_fChannelWeights[0]) * 0.25f;
    m_fChannelWeights[1] =
      ((m_fChannelWeights[1] * 3 * medianG) + m_fChannelWeights[1]) * 0.25f;
    m_fChannelWeights[2] =
      ((m_fChannelWeights[2] * 3 * medianB) + m_fChannelWeights[2]) * 0.25f;
    fWeightScale = 1.0f / (m_fChannelWeights[0] + m_fChannelWeights[1] +
                           m_fChannelWeights[2]);
    m_fChannelWeights[0] *= fWeightScale;
    m_fChannelWeights[1] *= fWeightScale;
    m_fChannelWeights[2] *= fWeightScale;
  }

  return m_fChannelWeights;
}

float* CCodec_DXTC::CalculateColourWeightings(float block[BLOCK_SIZE_4X4X4])
{
  if (!m_bUseChannelWeighting)
    return nullptr;

  if (m_bUseAdaptiveWeighting)
  {
    float medianR = 0.0f, medianG = 0.0f, medianB = 0.0f;

    for (std::uint32_t k = 0; k < BLOCK_SIZE_4X4; k++)
    {
      *block++;
      medianB += *block++;
      medianG += *block++;
      medianR += *block++;
    }

    medianR /= BLOCK_SIZE_4X4;
    medianG /= BLOCK_SIZE_4X4;
    medianB /= BLOCK_SIZE_4X4;

    // Now skew the colour weightings based on the gravity center of the block
    float largest = max(max(medianR, medianG), medianB);

    if (largest > 0)
    {
      medianR /= largest;
      medianG /= largest;
      medianB /= largest;
    }
    else
      medianR = medianG = medianB = 1.0f;

    // Scale weightings back up to 1.0f
    float fWeightScale =
      1.0f / (m_fBaseChannelWeights[0] + m_fBaseChannelWeights[1] +
              m_fBaseChannelWeights[2]);
    m_fChannelWeights[0] *= m_fBaseChannelWeights[0] * fWeightScale;
    m_fChannelWeights[1] *= m_fBaseChannelWeights[1] * fWeightScale;
    m_fChannelWeights[2] *= m_fBaseChannelWeights[2] * fWeightScale;
    m_fChannelWeights[0] =
      ((m_fChannelWeights[0] * 3 * medianR) + m_fChannelWeights[0]) * 0.25f;
    m_fChannelWeights[1] =
      ((m_fChannelWeights[1] * 3 * medianG) + m_fChannelWeights[1]) * 0.25f;
    m_fChannelWeights[2] =
      ((m_fChannelWeights[2] * 3 * medianB) + m_fChannelWeights[2]) * 0.25f;
    fWeightScale = 1.0f / (m_fChannelWeights[0] + m_fChannelWeights[1] +
                           m_fChannelWeights[2]);
    m_fChannelWeights[0] *= fWeightScale;
    m_fChannelWeights[1] *= fWeightScale;
    m_fChannelWeights[2] *= fWeightScale;
  }

  return m_fChannelWeights;
}
