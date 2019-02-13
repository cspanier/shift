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
//  File Name:   Codec_DXTC.h
//  Description: interface for the CCodec_DXTC class
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CODEC_DXTC_H_INCLUDED_
#define _CODEC_DXTC_H_INCLUDED_

#include "compressonator/Codec/Block/Codec_Block_4x4.h"

#define DXTC_OFFSET_ALPHA 0
#define DXTC_OFFSET_RGB 2

class CCodec_DXTC : public CCodec_Block_4x4
{
public:
  CCodec_DXTC(CodecType codecType);
  virtual ~CCodec_DXTC();

  virtual bool SetParameter(const char* pszParamName, char* sValue);
  virtual bool SetParameter(const char* pszParamName, std::uint32_t dwValue);
  virtual bool GetParameter(const char* pszParamName, std::uint32_t& dwValue);

  virtual bool SetParameter(const char* pszParamName, float fValue);
  virtual bool GetParameter(const char* pszParamName, float& fValue);

protected:
  virtual CodecError CompressAlphaBlock(std::uint8_t alphaBlock[BLOCK_SIZE_4X4],
                                        std::uint32_t compressedBlock[2]);
  virtual CodecError CompressExplicitAlphaBlock(
    std::uint8_t alphaBlock[BLOCK_SIZE_4X4], std::uint32_t compressedBlock[2]);
  virtual CodecError CompressRGBBlock(std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4],
                                      std::uint32_t compressedBlock[2],
                                      float* pfChannelWeights = nullptr,
                                      bool bDXT1 = false,
                                      bool bDXT1UseAlpha = false,
                                      std::uint8_t nDXT1AlphaThreshold = 0);
  virtual CodecError CompressRGBABlock(std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4],
                                       std::uint32_t compressedBlock[4],
                                       float* pfChannelWeights = nullptr);
  virtual CodecError CompressRGBABlock_ExplicitAlpha(
    std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4],
    float* pfChannelWeights = nullptr);

  virtual CodecError CompressAlphaBlock_Fast(
    std::uint8_t alphaBlock[BLOCK_SIZE_4X4], std::uint32_t compressedBlock[2]);
  virtual CodecError CompressExplicitAlphaBlock_Fast(
    std::uint8_t alphaBlock[BLOCK_SIZE_4X4], std::uint32_t compressedBlock[2]);
  virtual CodecError CompressRGBBlock_Fast(
    std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[2]);
  virtual CodecError CompressRGBBlock_SuperFast(
    std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[2]);
  virtual CodecError CompressRGBABlock_Fast(
    std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4]);
  virtual CodecError CompressRGBABlock_SuperFast(
    std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4]);
  virtual CodecError CompressRGBABlock_ExplicitAlpha_Fast(
    std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4]);
  virtual CodecError CompressRGBABlock_ExplicitAlpha_SuperFast(
    std::uint8_t rgbaBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[4]);

  virtual CodecError CompressAlphaBlock(float alphaBlock[BLOCK_SIZE_4X4],
                                        std::uint32_t compressedBlock[2]);
  virtual CodecError CompressExplicitAlphaBlock(
    float alphaBlock[BLOCK_SIZE_4X4], std::uint32_t compressedBlock[2]);
  virtual CodecError CompressRGBBlock(float rgbBlock[BLOCK_SIZE_4X4X4],
                                      std::uint32_t compressedBlock[2],
                                      float* pfChannelWeights = nullptr,
                                      bool bDXT1 = false,
                                      bool bDXT1UseAlpha = false,
                                      float fDXT1AlphaThreshold = 0.0);
  virtual CodecError CompressRGBABlock(float rgbaBlock[BLOCK_SIZE_4X4X4],
                                       std::uint32_t compressedBlock[4],
                                       float* pfChannelWeights = nullptr);
  virtual CodecError CompressRGBABlock_ExplicitAlpha(
    float rgbaBlock[BLOCK_SIZE_4X4], std::uint32_t compressedBlock[4],
    float* pfChannelWeights = nullptr);

  virtual void DecompressAlphaBlock(std::uint8_t alphaBlock[BLOCK_SIZE_4X4],
                                    std::uint32_t compressedBlock[2]);
  virtual void DecompressExplicitAlphaBlock(
    std::uint8_t alphaBlock[BLOCK_SIZE_4X4], std::uint32_t compressedBlock[2]);
  virtual void DecompressRGBBlock(std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4],
                                  std::uint32_t compressedBlock[2], bool bDXT1);
  virtual void DecompressRGBABlock(std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4],
                                   std::uint32_t compressedBlock[2]);
  virtual void DecompressRGBABlock_ExplicitAlpha(
    std::uint8_t rgbBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[2]);

  virtual void DecompressAlphaBlock(float alphaBlock[BLOCK_SIZE_4X4],
                                    std::uint32_t compressedBlock[2]);
  virtual void DecompressExplicitAlphaBlock(float alphaBlock[BLOCK_SIZE_4X4],
                                            std::uint32_t compressedBlock[2]);
  virtual void DecompressRGBBlock(float rgbBlock[BLOCK_SIZE_4X4X4],
                                  std::uint32_t compressedBlock[2], bool bDXT1);
  virtual void DecompressRGBABlock(float rgbBlock[BLOCK_SIZE_4X4X4],
                                   std::uint32_t compressedBlock[2]);
  virtual void DecompressRGBABlock_ExplicitAlpha(
    float rgbBlock[BLOCK_SIZE_4X4X4], std::uint32_t compressedBlock[2]);

  // Alpha decompression functions
  void GetCompressedAlphaRamp(std::uint8_t alpha[8],
                              std::uint32_t compressedBlock[2]);
  void GetCompressedAlphaRamp(float alpha[8], std::uint32_t compressedBlock[2]);

  // RGB compression functions
  float* CalculateColourWeightings(std::uint8_t block[BLOCK_SIZE_4X4X4]);
  float* CalculateColourWeightings(float block[BLOCK_SIZE_4X4X4]);

  void EncodeAlphaBlock(std::uint32_t compressedBlock[2],
                        std::uint8_t nEndpoints[2],
                        std::uint8_t nIndices[BLOCK_SIZE_4X4]);

  bool m_bUseChannelWeighting;
  bool m_bUseAdaptiveWeighting;
  bool m_bUseFloat;
  bool m_b3DRefinement;
  bool m_bSwizzleChannels;
  std::uint8_t m_nRefinementSteps;
  CMP_Speed m_nCompressionSpeed;

  float m_fBaseChannelWeights[3];
  float m_fChannelWeights[3];
};

#endif  // !defined(_CODEC_DXTC_H_INCLUDED_)
