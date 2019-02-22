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
//  File Name:   Codec_ATI1N.cpp
//  Description: implementation of the CCodec_ATI1N class
//
//////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4100)

#include "compressonator/Common.h"
#include "compressonator/Codec/ATI/Codec_ATI1N.h"

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

CCodec_ATI1N::CCodec_ATI1N(CodecType codecType) : CCodec_DXTC(codecType)
{
}

CCodec_ATI1N::~CCodec_ATI1N()
= default;

CCodecBuffer* CCodec_ATI1N::CreateBuffer(
  std::uint8_t  /*nBlockWidth*/, std::uint8_t  /*nBlockHeight*/, std::uint8_t  /*nBlockDepth*/,
  std::uint32_t dwWidth, std::uint32_t dwHeight, std::uint32_t dwPitch,
  std::uint8_t* pData) const
{
  return CreateCodecBuffer(CBT_4x4Block_4BPP, 4, 4, 1, dwWidth, dwHeight,
                           dwPitch, pData);
}

CodecError CCodec_ATI1N::Compress(CCodecBuffer& bufferIn,
                                  CCodecBuffer& bufferOut,
                                  Codec_Feedback_Proc pFeedbackProc,
                                  std::uint32_t pUser1, std::uint32_t pUser2)
{
  if ((m_nCompressionSpeed == CMP_Speed_Fast ||
       m_nCompressionSpeed == CMP_Speed_SuperFast) &&
      m_bUseSSE)
    return Compress_Fast(bufferIn, bufferOut, pFeedbackProc, pUser1, pUser2);

  assert(bufferIn.GetWidth() == bufferOut.GetWidth());
  assert(bufferIn.GetHeight() == bufferOut.GetHeight());

  if (bufferIn.GetWidth() != bufferOut.GetWidth() ||
      bufferIn.GetHeight() != bufferOut.GetHeight())
    return CE_Unknown;

  const std::uint32_t dwBlocksX = ((bufferIn.GetWidth() + 3) >> 2);
  const std::uint32_t dwBlocksY = ((bufferIn.GetHeight() + 3) >> 2);

  bool bUseFixed =
    (!bufferIn.IsFloat() && bufferIn.GetChannelDepth() == 8 && !m_bUseFloat);

  for (std::uint32_t j = 0; j < dwBlocksY; j++)
  {
    for (std::uint32_t i = 0; i < dwBlocksX; i++)
    {
      std::uint32_t compressedBlock[2];
      if (bUseFixed)
      {
        std::uint8_t cAlphaBlock[BLOCK_SIZE_4X4];
        bufferIn.ReadBlockR(i * 4, j * 4, 4, 4, cAlphaBlock);
        CompressAlphaBlock(cAlphaBlock, compressedBlock);
      }
      else
      {
        float fAlphaBlock[BLOCK_SIZE_4X4];
        bufferIn.ReadBlockR(i * 4, j * 4, 4, 4, fAlphaBlock);
        CompressAlphaBlock(fAlphaBlock, compressedBlock);
      }
      bufferOut.WriteBlock(i * 4, j * 4, compressedBlock, 2);
    }

    if (pFeedbackProc)
    {
      float fProgress = 100.f * (j * dwBlocksX) / (dwBlocksX * dwBlocksY);
      if (pFeedbackProc(fProgress, pUser1, pUser2))
        return CE_Aborted;
    }
  }

  return CE_OK;
}

CodecError CCodec_ATI1N::Compress_Fast(CCodecBuffer& bufferIn,
                                       CCodecBuffer& bufferOut,
                                       Codec_Feedback_Proc pFeedbackProc,
                                       std::uint32_t pUser1,
                                       std::uint32_t pUser2)
{
  assert(bufferIn.GetWidth() == bufferOut.GetWidth());
  assert(bufferIn.GetHeight() == bufferOut.GetHeight());

  if (bufferIn.GetWidth() != bufferOut.GetWidth() ||
      bufferIn.GetHeight() != bufferOut.GetHeight())
    return CE_Unknown;

  const std::uint32_t dwBlocksX = ((bufferIn.GetWidth() + 3) >> 2);
  const std::uint32_t dwBlocksY = ((bufferIn.GetHeight() + 3) >> 2);

  for (std::uint32_t j = 0; j < dwBlocksY; j++)
  {
    for (std::uint32_t i = 0; i < dwBlocksX; i++)
    {
      std::uint32_t compressedBlock[2];
      std::uint8_t cAlphaBlock[BLOCK_SIZE_4X4];
      bufferIn.ReadBlockR(i * 4, j * 4, 4, 4, cAlphaBlock);
      CompressAlphaBlock_Fast(cAlphaBlock, compressedBlock);
      bufferOut.WriteBlock(i * 4, j * 4, compressedBlock, 2);
    }

    if (pFeedbackProc)
    {
      float fProgress = 100.f * (j * dwBlocksX) / (dwBlocksX * dwBlocksY);
      if (pFeedbackProc(fProgress, pUser1, pUser2))
        return CE_Aborted;
    }
  }

  return CE_OK;
}

CodecError CCodec_ATI1N::Decompress(CCodecBuffer& bufferIn,
                                    CCodecBuffer& bufferOut,
                                    Codec_Feedback_Proc pFeedbackProc,
                                    std::uint32_t pUser1, std::uint32_t pUser2)
{
  assert(bufferIn.GetWidth() == bufferOut.GetWidth());
  assert(bufferIn.GetHeight() == bufferOut.GetHeight());

  if (bufferIn.GetWidth() != bufferOut.GetWidth() ||
      bufferIn.GetHeight() != bufferOut.GetHeight())
    return CE_Unknown;

  const std::uint32_t dwBlocksX = ((bufferIn.GetWidth() + 3) >> 2);
  const std::uint32_t dwBlocksY = ((bufferIn.GetHeight() + 3) >> 2);
  const std::uint32_t dwBlocksXY = dwBlocksX * dwBlocksY;

  bool bUseFixed =
    (!bufferOut.IsFloat() && bufferOut.GetChannelDepth() == 8 && !m_bUseFloat);

  for (std::uint32_t j = 0; j < dwBlocksY; j++)
  {
    for (std::uint32_t i = 0; i < dwBlocksX; i++)
    {
      std::uint32_t compressedBlock[2];
      bufferIn.ReadBlock(i * 4, j * 4, compressedBlock, 2);

      if (bUseFixed)
      {
        std::uint8_t alphaBlock[BLOCK_SIZE_4X4];
        DecompressAlphaBlock(alphaBlock, compressedBlock);
        bufferOut.WriteBlockR(i * 4, j * 4, 4, 4, alphaBlock);
        bufferOut.WriteBlockG(i * 4, j * 4, 4, 4, alphaBlock);
        bufferOut.WriteBlockB(i * 4, j * 4, 4, 4, alphaBlock);
        bufferOut.WriteBlockA(i * 4, j * 4, 4, 4, alphaBlock);
      }
      else
      {
        float alphaBlock[BLOCK_SIZE_4X4];
        DecompressAlphaBlock(alphaBlock, compressedBlock);
        bufferOut.WriteBlockR(i * 4, j * 4, 4, 4, alphaBlock);
        bufferOut.WriteBlockG(i * 4, j * 4, 4, 4, alphaBlock);
        bufferOut.WriteBlockB(i * 4, j * 4, 4, 4, alphaBlock);
        bufferOut.WriteBlockA(i * 4, j * 4, 4, 4, alphaBlock);
      }
    }

    if (pFeedbackProc)
    {
      float fProgress = 100.f * (j * dwBlocksX) / dwBlocksXY;
      if (pFeedbackProc(fProgress, pUser1, pUser2))
      {
        return CE_Aborted;
      }
    }
  }

  return CE_OK;
}
