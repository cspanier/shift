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
//  File Name:   Codec_ATI2N_DXT5.cpp
//  Description: implementation of the CCodec_ATI2N_DXT5 class
//
//////////////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4100)

#include "compressonator/Common.h"
#include "compressonator/Codec/ATI/Codec_ATI2N_DXT5.h"

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

CCodec_ATI2N_DXT5::CCodec_ATI2N_DXT5() : CCodec_ATI2N(CT_ATI2N_DXT5)
{
}

CCodec_ATI2N_DXT5::~CCodec_ATI2N_DXT5()
{
}

CodecError CCodec_ATI2N_DXT5::Compress(CCodecBuffer& bufferIn,
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

  bool bUseFixed =
    (!bufferIn.IsFloat() && bufferIn.GetChannelDepth() == 8 && !m_bUseFloat);

  for (std::uint32_t j = 0; j < dwBlocksY; j++)
  {
    for (std::uint32_t i = 0; i < dwBlocksX; i++)
    {
      std::uint32_t compressedBlock[4];
      if (bUseFixed)
      {
        std::uint8_t srcBlock[BLOCK_SIZE_4X4X4];
        std::uint8_t tempBlock[BLOCK_SIZE_4X4X4];
        bufferIn.ReadBlockRGBA(i * 4, j * 4, 4, 4, tempBlock);
        for (std::uint32_t k = 0; k < BLOCK_SIZE_4X4; k++)
          ((std::uint32_t*)srcBlock)[k] =
            SWIZZLE_RGBA_xGxR(((std::uint32_t*)tempBlock)[k]);
        CompressRGBABlock(srcBlock, compressedBlock);
      }
      else
      {
        float srcBlock[BLOCK_SIZE_4X4X4];
        float tempBlock[BLOCK_SIZE_4X4X4];
        bufferIn.ReadBlockRGBA(i * 4, j * 4, 4, 4, tempBlock);
        for (std::uint32_t k = 0; k < BLOCK_SIZE_4X4; k++)
        {
          srcBlock[(k * 4) + RGBA32F_OFFSET_R] = 0;
          srcBlock[(k * 4) + RGBA32F_OFFSET_A] =
            tempBlock[(k * 4) + RGBA32F_OFFSET_R];
          srcBlock[(k * 4) + RGBA32F_OFFSET_B] = 0;
          srcBlock[(k * 4) + RGBA32F_OFFSET_G] =
            tempBlock[(k * 4) + RGBA32F_OFFSET_G];
        }
        CompressRGBABlock(srcBlock, compressedBlock);
      }
      bufferOut.WriteBlock(i * 4, j * 4, compressedBlock, 4);
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

CodecError CCodec_ATI2N_DXT5::Decompress(CCodecBuffer& bufferIn,
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
  const std::uint32_t dwBlocksXY = dwBlocksX * dwBlocksY;

  bool bUseFixed =
    (!bufferOut.IsFloat() && bufferOut.GetChannelDepth() == 8 && !m_bUseFloat);

  for (std::uint32_t j = 0; j < dwBlocksY; j++)
  {
    for (std::uint32_t i = 0; i < dwBlocksX; i++)
    {
      std::uint32_t compressedBlock[4];
      bufferIn.ReadBlock(i * 4, j * 4, compressedBlock, 4);
      if (bUseFixed)
      {
        std::uint8_t tempBlock[BLOCK_SIZE_4X4X4];
        DecompressRGBABlock(tempBlock, compressedBlock);

        std::uint8_t destBlock[BLOCK_SIZE_4X4X4];
        for (std::uint32_t k = 0; k < BLOCK_SIZE_4X4; k++)
        {
          destBlock[(k * 4) + RGBA8888_CHANNEL_R] =
            tempBlock[(k * 4) + RGBA8888_CHANNEL_A];
          destBlock[(k * 4) + RGBA8888_CHANNEL_G] =
            tempBlock[(k * 4) + RGBA8888_CHANNEL_G];
          destBlock[(k * 4) + RGBA8888_CHANNEL_B] = 0;
          destBlock[(k * 4) + RGBA8888_CHANNEL_A] = 0xff;
        }

        bufferOut.WriteBlockRGBA(i * 4, j * 4, 4, 4, destBlock);
      }
      else
      {
        float tempBlock[BLOCK_SIZE_4X4X4];
        DecompressRGBABlock(tempBlock, compressedBlock);

        float destBlock[BLOCK_SIZE_4X4X4];
        for (std::uint32_t k = 0; k < BLOCK_SIZE_4X4; k++)
        {
          destBlock[(k * 4) + RGBA32F_OFFSET_R] =
            tempBlock[(k * 4) + RGBA32F_OFFSET_A];
          destBlock[(k * 4) + RGBA32F_OFFSET_G] =
            tempBlock[(k * 4) + RGBA32F_OFFSET_G];
          destBlock[(k * 4) + RGBA32F_OFFSET_B] = 0.f;
          destBlock[(k * 4) + RGBA32F_OFFSET_A] = 1.0;
        }

        bufferOut.WriteBlockRGBA(i * 4, j * 4, 4, 4, destBlock);
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
