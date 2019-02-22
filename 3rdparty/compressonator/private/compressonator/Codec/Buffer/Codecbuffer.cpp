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
//  File Name:   CodecBuffer.cpp
//  Description: implementation of the CCodecBuffer class
//
//////////////////////////////////////////////////////////////////////////////

#include "compressonator/Common.h"
#include "compressonator/Codec/Buffer/CodecBuffer.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RGBA8888.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RGB888.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RG8.h"
#include "compressonator/Codec/Buffer/CodecBuffer_R8.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RGBA2101010.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RGBA16.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RG16.h"
#include "compressonator/Codec/Buffer/CodecBuffer_R16.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RGBA32.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RG32.h"
#include "compressonator/Codec/Buffer/CodecBuffer_R32.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RGBA16F.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RG16F.h"
#include "compressonator/Codec/Buffer/CodecBuffer_R16F.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RGBA32F.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RG32F.h"
#include "compressonator/Codec/Buffer/CodecBuffer_R32F.h"
#include "compressonator/Codec/Buffer/CodecBuffer_Block.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RGB9995EF.h"

CCodecBuffer* CreateCodecBuffer(CodecBufferType nCodecBufferType,
                                std::uint8_t nBlockWidth,
                                std::uint8_t nBlockHeight,
                                std::uint8_t nBlockDepth, std::uint32_t dwWidth,
                                std::uint32_t dwHeight, std::uint32_t dwPitch,
                                std::uint8_t* pData)
{
  switch (nCodecBufferType)
  {
  case CBT_RGBA8888:
  case CBT_BGRA8888:
  case CBT_ARGB8888:
  case CBT_ABGR8888:
    return new CCodecBuffer_RGBA8888(nBlockWidth, nBlockHeight, nBlockDepth,
                                     dwWidth, dwHeight, dwPitch, pData);
  case CBT_RGB888:
    return new CCodecBuffer_RGB888(nBlockWidth, nBlockHeight, nBlockDepth,
                                   dwWidth, dwHeight, dwPitch, pData);
  case CBT_RG8:
    return new CCodecBuffer_RG8(nBlockWidth, nBlockHeight, nBlockDepth, dwWidth,
                                dwHeight, dwPitch, pData);
  case CBT_R8:
    return new CCodecBuffer_R8(nBlockWidth, nBlockHeight, nBlockDepth, dwWidth,
                               dwHeight, dwPitch, pData);
  case CBT_RGBA2101010:
    return new CCodecBuffer_RGBA2101010(nBlockWidth, nBlockHeight, nBlockDepth,
                                        dwWidth, dwHeight, dwPitch, pData);
  case CBT_RGBA16:
    return new CCodecBuffer_RGBA16(nBlockWidth, nBlockHeight, nBlockDepth,
                                   dwWidth, dwHeight, dwPitch, pData);
  case CBT_RG16:
    return new CCodecBuffer_RG16(nBlockWidth, nBlockHeight, nBlockDepth,
                                 dwWidth, dwHeight, dwPitch, pData);
  case CBT_R16:
    return new CCodecBuffer_R16(nBlockWidth, nBlockHeight, nBlockDepth, dwWidth,
                                dwHeight, dwPitch, pData);
  case CBT_RGBA16F:
    return new CCodecBuffer_RGBA16F(nBlockWidth, nBlockHeight, nBlockDepth,
                                    dwWidth, dwHeight, dwPitch, pData);
  case CBT_RG16F:
    return new CCodecBuffer_RG16F(nBlockWidth, nBlockHeight, nBlockDepth,
                                  dwWidth, dwHeight, dwPitch, pData);
  case CBT_R16F:
    return new CCodecBuffer_R16F(nBlockWidth, nBlockHeight, nBlockDepth,
                                 dwWidth, dwHeight, dwPitch, pData);
  case CBT_RGBA32:
    return new CCodecBuffer_RGBA32(nBlockWidth, nBlockHeight, nBlockDepth,
                                   dwWidth, dwHeight, dwPitch, pData);
  case CBT_RG32:
    return new CCodecBuffer_RG32(nBlockWidth, nBlockHeight, nBlockDepth,
                                 dwWidth, dwHeight, dwPitch, pData);
  case CBT_R32:
    return new CCodecBuffer_R32(nBlockWidth, nBlockHeight, nBlockDepth, dwWidth,
                                dwHeight, dwPitch, pData);
  case CBT_RGBA32F:
    return new CCodecBuffer_RGBA32F(nBlockWidth, nBlockHeight, nBlockDepth,
                                    dwWidth, dwHeight, dwPitch, pData);
  case CBT_RG32F:
    return new CCodecBuffer_RG32F(nBlockWidth, nBlockHeight, nBlockDepth,
                                  dwWidth, dwHeight, dwPitch, pData);
  case CBT_R32F:
    return new CCodecBuffer_R32F(nBlockWidth, nBlockHeight, nBlockDepth,
                                 dwWidth, dwHeight, dwPitch, pData);
  case CBT_RGBE32F:
    return new CCodecBuffer_RGB9995EF(nBlockWidth, nBlockHeight, nBlockDepth,
                                      dwWidth, dwHeight, dwPitch, pData);
  case CBT_4x4Block_2BPP:
  case CBT_4x4Block_4BPP:
  case CBT_4x4Block_8BPP:
  case CBT_4x4Block_16BPP:
    return new CCodecBuffer_Block(nCodecBufferType, nBlockWidth, nBlockHeight,
                                  nBlockDepth, dwWidth, dwHeight, dwPitch,
                                  pData);
  case CBT_8x8Block_2BPP:
  case CBT_8x8Block_4BPP:
  case CBT_8x8Block_8BPP:
  case CBT_8x8Block_16BPP:
    return new CCodecBuffer_Block(nCodecBufferType, nBlockWidth, nBlockHeight,
                                  nBlockDepth, dwWidth, dwHeight, dwPitch,
                                  pData);

  case CBT_Unknown:
  default:
    assert(0);
    return nullptr;
  }
}

CodecBufferType GetCodecBufferType(cmp_format format)
{
  CodecBufferType CBT_type;
  // ToDo Expand the CBT data types listed below so that cmp_format maps to a
  // single CBT_ type
  switch (format)
  {
  case cmp_format::ARGB_32F:
  case cmp_format::ABGR_32F:
  case cmp_format::RGBA_32F:
  case cmp_format::BGRA_32F:
    CBT_type = CBT_RGBA32F;
    break;
  case cmp_format::RG_32F:
    CBT_type = CBT_RG32F;
    break;
  case cmp_format::R_32F:
    CBT_type = CBT_R32F;
    break;
  case cmp_format::ARGB_16F:
  case cmp_format::ABGR_16F:
  case cmp_format::RGBA_16F:
  case cmp_format::BGRA_16F:
    CBT_type = CBT_RGBA16F;
    break;
  case cmp_format::RGBE_32F:
    CBT_type = CBT_RGBE32F;
    break;
  case cmp_format::RG_16F:
    CBT_type = CBT_RG16F;
    break;
  case cmp_format::R_16F:
    CBT_type = CBT_R16F;
    break;
  case cmp_format::ARGB_16:
  case cmp_format::ABGR_16:
  case cmp_format::RGBA_16:
  case cmp_format::BGRA_16:
    CBT_type = CBT_RGBA16;
    break;
  case cmp_format::RG_16:
    CBT_type = CBT_RG16;
    break;
  case cmp_format::R_16:
    CBT_type = CBT_R16;
    break;

#ifdef ARGB_32_SUPPORT
  case TI_TC_FORMAT_ARGB_32:
    CBT_type = CBT_RGBA32;
    break;
  case TI_TC_FORMAT_RG_32:
    CBT_type = CBT_RG32;
    break;
  case TI_TC_FORMAT_R_32:
    CBT_type = CBT_R32;
    break;
#endif  // ARGB_32_SUPPORT
  case cmp_format::ARGB_2101010:
    CBT_type = CBT_RGBA2101010;
    break;
  case cmp_format::ARGB_8888:  // Need to expand on this format
  case cmp_format::ABGR_8888:  // Need to expand on this format
  case cmp_format::BGRA_8888:  // Need to expand on this format
  case cmp_format::RGBA_8888:
    CBT_type = CBT_RGBA8888;
    break;
  case cmp_format::BGR_888:  // Need to expand on this format
  case cmp_format::RGB_888:
    CBT_type = CBT_RGB888;
    break;
  case cmp_format::RG_8:
    CBT_type = CBT_RG8;
    break;
  case cmp_format::R_8:
    CBT_type = CBT_R8;
    break;
  default:
    CBT_type = CBT_Unknown;
    break;
  }
  return CBT_type;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCodecBuffer::CCodecBuffer(std::uint8_t nBlockWidth, std::uint8_t nBlockHeight,
                           std::uint8_t nBlockDepth, std::uint32_t dwWidth,
                           std::uint32_t dwHeight, std::uint32_t dwPitch,
                           std::uint8_t* pData)
{
  m_dwWidth = dwWidth;
  m_dwHeight = dwHeight;
  m_dwPitch = dwPitch;

  m_nBlockWidth = nBlockWidth;
  m_nBlockHeight = nBlockHeight;
  m_nBlockDepth = nBlockDepth;

  m_pData = pData;
  m_bUserAllocedData = (pData != nullptr);

  m_bPerformingConversion = false;
}

CCodecBuffer::~CCodecBuffer()
{
  if (m_pData && !m_bUserAllocedData)
  {
    free(m_pData);
    m_pData = nullptr;
  }
}

void CCodecBuffer::Copy(CCodecBuffer& srcBuffer)
{
  if (GetWidth() != srcBuffer.GetWidth() ||
      GetHeight() != srcBuffer.GetHeight())
    return;

  const std::uint32_t dwBlocksX = ((GetWidth() + 3) >> 2);
  const std::uint32_t dwBlocksY = ((GetHeight() + 3) >> 2);

  for (std::uint32_t j = 0; j < dwBlocksY; j++)
  {
    for (std::uint32_t i = 0; i < dwBlocksX; i++)
    {
      float block[BLOCK_SIZE_4X4X4];
      srcBuffer.ReadBlockRGBA(i * 4, j * 4, 4, 4, block);
      WriteBlockRGBA(i * 4, j * 4, 4, 4, block);
    }
  }
}

#define MAX_BLOCK_WIDTH 8
#define MAX_BLOCK_HEIGHT 8
#define MAX_BLOCK MAX_BLOCK_WIDTH* MAX_BLOCK_HEIGHT

#define ATTEMPT_BLOCK_READ(b, c, t)      \
  {                                      \
    t block[MAX_BLOCK];                  \
    if (ReadBlock##c(x, y, w, h, block)) \
    {                                    \
      ConvertBlock(b, block, w* h);      \
      m_bPerformingConversion = false;   \
      return true;                       \
    }                                    \
  }

bool CCodecBuffer::ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(cBlock, R, double);
    ATTEMPT_BLOCK_READ(cBlock, R, float);
    ATTEMPT_BLOCK_READ(cBlock, R, half);
    ATTEMPT_BLOCK_READ(cBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_READ(cBlock, R, std::uint16_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(cBlock, G, double);
    ATTEMPT_BLOCK_READ(cBlock, G, float);
    ATTEMPT_BLOCK_READ(cBlock, G, half);
    ATTEMPT_BLOCK_READ(cBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_READ(cBlock, G, std::uint16_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(cBlock, B, double);
    ATTEMPT_BLOCK_READ(cBlock, B, float);
    ATTEMPT_BLOCK_READ(cBlock, B, half);
    ATTEMPT_BLOCK_READ(cBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_READ(cBlock, B, std::uint16_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(cBlock, A, double);
    ATTEMPT_BLOCK_READ(cBlock, A, float);
    ATTEMPT_BLOCK_READ(cBlock, A, half);
    ATTEMPT_BLOCK_READ(cBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_READ(cBlock, A, std::uint16_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(wBlock, R, double);
    ATTEMPT_BLOCK_READ(wBlock, R, float);
    ATTEMPT_BLOCK_READ(wBlock, R, half);
    ATTEMPT_BLOCK_READ(wBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_READ(wBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(wBlock, G, double);
    ATTEMPT_BLOCK_READ(wBlock, G, float);
    ATTEMPT_BLOCK_READ(wBlock, G, half);
    ATTEMPT_BLOCK_READ(wBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_READ(wBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(wBlock, B, double);
    ATTEMPT_BLOCK_READ(wBlock, B, float);
    ATTEMPT_BLOCK_READ(wBlock, B, half);
    ATTEMPT_BLOCK_READ(wBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_READ(wBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(wBlock, A, double);
    ATTEMPT_BLOCK_READ(wBlock, A, float);
    ATTEMPT_BLOCK_READ(wBlock, A, half);
    ATTEMPT_BLOCK_READ(wBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_READ(wBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(dwBlock, R, double);
    ATTEMPT_BLOCK_READ(dwBlock, R, float);
    ATTEMPT_BLOCK_READ(dwBlock, R, half);
    ATTEMPT_BLOCK_READ(dwBlock, R, std::uint16_t);
    ATTEMPT_BLOCK_READ(dwBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(dwBlock, G, double);
    ATTEMPT_BLOCK_READ(dwBlock, G, float);
    ATTEMPT_BLOCK_READ(dwBlock, G, half);
    ATTEMPT_BLOCK_READ(dwBlock, G, std::uint16_t);
    ATTEMPT_BLOCK_READ(dwBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(dwBlock, B, double);
    ATTEMPT_BLOCK_READ(dwBlock, B, float);
    ATTEMPT_BLOCK_READ(dwBlock, B, half);
    ATTEMPT_BLOCK_READ(dwBlock, B, std::uint16_t);
    ATTEMPT_BLOCK_READ(dwBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(dwBlock, A, double);
    ATTEMPT_BLOCK_READ(dwBlock, A, float);
    ATTEMPT_BLOCK_READ(dwBlock, A, half);
    ATTEMPT_BLOCK_READ(dwBlock, A, std::uint16_t);
    ATTEMPT_BLOCK_READ(dwBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(hBlock, R, double);
    ATTEMPT_BLOCK_READ(hBlock, R, float);
    ATTEMPT_BLOCK_READ(hBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_READ(hBlock, R, std::uint16_t);
    ATTEMPT_BLOCK_READ(hBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(hBlock, G, double);
    ATTEMPT_BLOCK_READ(hBlock, G, float);
    ATTEMPT_BLOCK_READ(hBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_READ(hBlock, G, std::uint16_t);
    ATTEMPT_BLOCK_READ(hBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(hBlock, B, double);
    ATTEMPT_BLOCK_READ(hBlock, B, float);
    ATTEMPT_BLOCK_READ(hBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_READ(hBlock, B, std::uint16_t);
    ATTEMPT_BLOCK_READ(hBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(hBlock, A, double);
    ATTEMPT_BLOCK_READ(hBlock, A, float);
    ATTEMPT_BLOCK_READ(hBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_READ(hBlock, A, std::uint16_t);
    ATTEMPT_BLOCK_READ(hBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(fBlock, R, double);
    ATTEMPT_BLOCK_READ(fBlock, R, half);
    ATTEMPT_BLOCK_READ(fBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_READ(fBlock, R, std::uint16_t);
    ATTEMPT_BLOCK_READ(fBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(fBlock, G, double);
    ATTEMPT_BLOCK_READ(fBlock, G, half);
    ATTEMPT_BLOCK_READ(fBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_READ(fBlock, G, std::uint16_t);
    ATTEMPT_BLOCK_READ(fBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(fBlock, B, double);
    ATTEMPT_BLOCK_READ(fBlock, B, half);
    ATTEMPT_BLOCK_READ(fBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_READ(fBlock, B, std::uint16_t);
    ATTEMPT_BLOCK_READ(fBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(fBlock, A, double);
    ATTEMPT_BLOCK_READ(fBlock, A, half);
    ATTEMPT_BLOCK_READ(fBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_READ(fBlock, A, std::uint16_t);
    ATTEMPT_BLOCK_READ(fBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(dBlock, R, float);
    ATTEMPT_BLOCK_READ(dBlock, R, half);
    ATTEMPT_BLOCK_READ(dBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_READ(dBlock, R, std::uint16_t);
    ATTEMPT_BLOCK_READ(dBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(dBlock, G, float);
    ATTEMPT_BLOCK_READ(dBlock, G, half);
    ATTEMPT_BLOCK_READ(dBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_READ(dBlock, G, std::uint16_t);
    ATTEMPT_BLOCK_READ(dBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(dBlock, B, float);
    ATTEMPT_BLOCK_READ(dBlock, B, half);
    ATTEMPT_BLOCK_READ(dBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_READ(dBlock, B, std::uint16_t);
    ATTEMPT_BLOCK_READ(dBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_READ(dBlock, A, float);
    ATTEMPT_BLOCK_READ(dBlock, A, half);
    ATTEMPT_BLOCK_READ(dBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_READ(dBlock, A, std::uint16_t);
    ATTEMPT_BLOCK_READ(dBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

#define ATTEMPT_BLOCK_WRITE(b, c, t)      \
  {                                       \
    t block[MAX_BLOCK];                   \
    ConvertBlock(block, b, w* h);         \
    if (WriteBlock##c(x, y, w, h, block)) \
    {                                     \
      m_bPerformingConversion = false;    \
      return true;                        \
    }                                     \
  }

bool CCodecBuffer::WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(cBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(cBlock, R, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(cBlock, R, double);
    ATTEMPT_BLOCK_WRITE(cBlock, R, float);
    ATTEMPT_BLOCK_WRITE(cBlock, R, half);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(cBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(cBlock, G, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(cBlock, G, double);
    ATTEMPT_BLOCK_WRITE(cBlock, G, float);
    ATTEMPT_BLOCK_WRITE(cBlock, G, half);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(cBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(cBlock, B, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(cBlock, B, double);
    ATTEMPT_BLOCK_WRITE(cBlock, B, float);
    ATTEMPT_BLOCK_WRITE(cBlock, B, half);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(cBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(cBlock, A, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(cBlock, A, double);
    ATTEMPT_BLOCK_WRITE(cBlock, A, float);
    ATTEMPT_BLOCK_WRITE(cBlock, A, half);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(wBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(wBlock, R, double);
    ATTEMPT_BLOCK_WRITE(wBlock, R, float);
    ATTEMPT_BLOCK_WRITE(wBlock, R, half);
    ATTEMPT_BLOCK_WRITE(wBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(wBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(wBlock, G, double);
    ATTEMPT_BLOCK_WRITE(wBlock, G, float);
    ATTEMPT_BLOCK_WRITE(wBlock, G, half);
    ATTEMPT_BLOCK_WRITE(wBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(wBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(wBlock, B, double);
    ATTEMPT_BLOCK_WRITE(wBlock, B, float);
    ATTEMPT_BLOCK_WRITE(wBlock, B, half);
    ATTEMPT_BLOCK_WRITE(wBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(wBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(wBlock, A, double);
    ATTEMPT_BLOCK_WRITE(wBlock, A, float);
    ATTEMPT_BLOCK_WRITE(wBlock, A, half);
    ATTEMPT_BLOCK_WRITE(wBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(dwBlock, R, double);
    ATTEMPT_BLOCK_WRITE(dwBlock, R, float);
    ATTEMPT_BLOCK_WRITE(dwBlock, R, half);
    ATTEMPT_BLOCK_WRITE(dwBlock, R, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(dwBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(dwBlock, G, double);
    ATTEMPT_BLOCK_WRITE(dwBlock, G, float);
    ATTEMPT_BLOCK_WRITE(dwBlock, G, half);
    ATTEMPT_BLOCK_WRITE(dwBlock, G, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(dwBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(dwBlock, B, double);
    ATTEMPT_BLOCK_WRITE(dwBlock, B, float);
    ATTEMPT_BLOCK_WRITE(dwBlock, B, half);
    ATTEMPT_BLOCK_WRITE(dwBlock, B, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(dwBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(dwBlock, A, double);
    ATTEMPT_BLOCK_WRITE(dwBlock, A, float);
    ATTEMPT_BLOCK_WRITE(dwBlock, A, half);
    ATTEMPT_BLOCK_WRITE(dwBlock, A, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(dwBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(hBlock, R, double);
    ATTEMPT_BLOCK_WRITE(hBlock, R, float);
    ATTEMPT_BLOCK_WRITE(hBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(hBlock, R, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(hBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(hBlock, G, double);
    ATTEMPT_BLOCK_WRITE(hBlock, G, float);
    ATTEMPT_BLOCK_WRITE(hBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(hBlock, G, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(hBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(hBlock, B, double);
    ATTEMPT_BLOCK_WRITE(hBlock, B, float);
    ATTEMPT_BLOCK_WRITE(hBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(hBlock, B, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(hBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(hBlock, A, double);
    ATTEMPT_BLOCK_WRITE(hBlock, A, float);
    ATTEMPT_BLOCK_WRITE(hBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(hBlock, A, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(hBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(fBlock, R, double);
    ATTEMPT_BLOCK_WRITE(fBlock, R, half);
    ATTEMPT_BLOCK_WRITE(fBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(fBlock, R, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(fBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(fBlock, G, double);
    ATTEMPT_BLOCK_WRITE(fBlock, G, half);
    ATTEMPT_BLOCK_WRITE(fBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(fBlock, G, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(fBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(fBlock, B, double);
    ATTEMPT_BLOCK_WRITE(fBlock, B, half);
    ATTEMPT_BLOCK_WRITE(fBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(fBlock, B, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(fBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(fBlock, A, double);
    ATTEMPT_BLOCK_WRITE(fBlock, A, half);
    ATTEMPT_BLOCK_WRITE(fBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(fBlock, A, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(fBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(dBlock, R, float);
    ATTEMPT_BLOCK_WRITE(dBlock, R, half);
    ATTEMPT_BLOCK_WRITE(dBlock, R, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(dBlock, R, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(dBlock, R, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(dBlock, G, float);
    ATTEMPT_BLOCK_WRITE(dBlock, G, half);
    ATTEMPT_BLOCK_WRITE(dBlock, G, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(dBlock, G, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(dBlock, G, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(dBlock, B, float);
    ATTEMPT_BLOCK_WRITE(dBlock, B, half);
    ATTEMPT_BLOCK_WRITE(dBlock, B, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(dBlock, B, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(dBlock, B, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                               std::uint8_t h, double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    ATTEMPT_BLOCK_WRITE(dBlock, A, float);
    ATTEMPT_BLOCK_WRITE(dBlock, A, half);
    ATTEMPT_BLOCK_WRITE(dBlock, A, std::uint32_t);
    ATTEMPT_BLOCK_WRITE(dBlock, A, std::uint16_t);
    ATTEMPT_BLOCK_WRITE(dBlock, A, std::uint8_t);

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                  std::uint8_t w, std::uint8_t h,
                                  std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    std::uint32_t dwBlock[MAX_BLOCK * 4];
    ConvertBlock(dwBlock, cBlock, w * h * 4);
    SwizzleBlock(dwBlock, w * h);
    if (WriteBlockRGBA(x, y, w, h, dwBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK * 4];
    ConvertBlock(wBlock, cBlock, w * h * 4);
    SwizzleBlock(wBlock, w * h);
    if (WriteBlockRGBA(x, y, w, h, wBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    double dBlock[MAX_BLOCK * 4];
    ConvertBlock(dBlock, cBlock, w * h * 4);
    SwizzleBlock(dBlock, w * h);
    if (WriteBlockRGBA(x, y, w, h, dBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    float fBlock[MAX_BLOCK * 4];
    ConvertBlock(fBlock, cBlock, w * h * 4);
    SwizzleBlock(fBlock, w * h);
    if (WriteBlockRGBA(x, y, w, h, fBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    ConvertBlock(hBlock, cBlock, w * h * 4);
    SwizzleBlock(hBlock, w * h);
    if (WriteBlockRGBA(x, y, w, h, hBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                 std::uint8_t w, std::uint8_t h,
                                 std::uint8_t cBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    double dBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, dBlock))
    {
      SwizzleBlock(dBlock, w * h);
      ConvertBlock(cBlock, dBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    float fBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, fBlock))
    {
      SwizzleBlock(fBlock, w * h);
      ConvertBlock(cBlock, fBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, hBlock))
    {
      SwizzleBlock(hBlock, w * h);
      ConvertBlock(cBlock, hBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint32_t dwBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, dwBlock))
    {
      SwizzleBlock(dwBlock, w * h);
      ConvertBlock(cBlock, dwBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, wBlock))
    {
      SwizzleBlock(wBlock, w * h);
      ConvertBlock(cBlock, wBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                  std::uint8_t w, std::uint8_t h,
                                  std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    double dBlock[MAX_BLOCK * 4];
    ConvertBlock(dBlock, dwBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, dBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    float fBlock[MAX_BLOCK * 4];
    ConvertBlock(fBlock, dwBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, fBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    ConvertBlock(hBlock, dwBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, hBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK];
    SwizzleBlock(dwBlock, w * h);
    ConvertBlock(wBlock, dwBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, wBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                 std::uint8_t w, std::uint8_t h,
                                 std::uint32_t dwBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    double dBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, dBlock))
    {
      ConvertBlock(dwBlock, dBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    float fBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, fBlock))
    {
      ConvertBlock(dwBlock, fBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, hBlock))
    {
      ConvertBlock(dwBlock, hBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    unsigned char cBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, cBlock))
    {
      ConvertBlock(dwBlock, cBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK];
    if (ReadBlockRGBA(x, y, w, h, dwBlock))
    {
      ConvertBlock(dwBlock, wBlock, w * h * 4);
      SwizzleBlock(dwBlock, w * h);
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                  std::uint8_t w, std::uint8_t h,
                                  std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    double dBlock[MAX_BLOCK * 4];
    ConvertBlock(dBlock, wBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, dBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    float fBlock[MAX_BLOCK * 4];
    ConvertBlock(fBlock, wBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, fBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    ConvertBlock(hBlock, wBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, hBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    std::uint32_t dwBlock[MAX_BLOCK];
    SwizzleBlock(wBlock, w * h);
    ConvertBlock((std::uint8_t*)dwBlock, wBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, dwBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                 std::uint8_t w, std::uint8_t h,
                                 std::uint16_t wBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    double dBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, dBlock))
    {
      ConvertBlock(wBlock, dBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    float fBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, fBlock))
    {
      ConvertBlock(wBlock, fBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, hBlock))
    {
      ConvertBlock(wBlock, hBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint32_t dwBlock[MAX_BLOCK];
    if (ReadBlockRGBA(x, y, w, h, dwBlock))
    {
      ConvertBlock(wBlock, (std::uint8_t*)dwBlock, w * h * 4);
      SwizzleBlock(wBlock, w * h);
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                  std::uint8_t w, std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    double dBlock[MAX_BLOCK * 4];
    ConvertBlock(dBlock, hBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, dBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    float fBlock[MAX_BLOCK * 4];
    ConvertBlock(fBlock, hBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, fBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK * 4];
    ConvertBlock(wBlock, hBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, wBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    std::uint32_t dwBlock[MAX_BLOCK];
    SwizzleBlock(hBlock, w * h);
    ConvertBlock((std::uint8_t*)dwBlock, hBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, dwBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                 std::uint8_t w, std::uint8_t h, half hBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    double dBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, dBlock))
    {
      ConvertBlock(hBlock, dBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    float fBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, fBlock))
    {
      ConvertBlock(hBlock, fBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, wBlock))
    {
      ConvertBlock(hBlock, wBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint32_t dwBlock[MAX_BLOCK];
    if (ReadBlockRGBA(x, y, w, h, dwBlock))
    {
      ConvertBlock(hBlock, (std::uint8_t*)dwBlock, w * h * 4);
      SwizzleBlock(hBlock, w * h);
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                  std::uint8_t w, std::uint8_t h,
                                  float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    double dBlock[MAX_BLOCK * 4];
    ConvertBlock(dBlock, fBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, dBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    ConvertBlock(hBlock, fBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, hBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK * 4];
    ConvertBlock(wBlock, fBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, wBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    std::uint32_t dwBlock[MAX_BLOCK];
    SwizzleBlock(fBlock, w * h);
    ConvertBlock((std::uint8_t*)dwBlock, fBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, dwBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                 std::uint8_t w, std::uint8_t h, float fBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    double dBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, dBlock))
    {
      ConvertBlock(fBlock, dBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, hBlock))
    {
      ConvertBlock(fBlock, hBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, wBlock))
    {
      ConvertBlock(fBlock, wBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint32_t dwBlock[MAX_BLOCK];
    if (ReadBlockRGBA(x, y, w, h, dwBlock))
    {
      ConvertBlock(fBlock, (std::uint8_t*)dwBlock, w * h * 4);
      SwizzleBlock(fBlock, w * h);
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                  std::uint8_t w, std::uint8_t h,
                                  double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    float fBlock[MAX_BLOCK * 4];
    ConvertBlock(fBlock, dBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, fBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    ConvertBlock(hBlock, dBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, hBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK * 4];
    ConvertBlock(wBlock, dBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, wBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    std::uint32_t dwBlock[MAX_BLOCK];
    SwizzleBlock(dBlock, w * h);
    ConvertBlock((std::uint8_t*)dwBlock, dBlock, w * h * 4);
    if (WriteBlockRGBA(x, y, w, h, dwBlock))
    {
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                 std::uint8_t w, std::uint8_t h,
                                 double dBlock[])
{
  // Ok, so we don't support this format
  // So we try other formats to find one that is supported

  if (m_bPerformingConversion)
  {
    return false;
  }
  
  
    m_bPerformingConversion = true;

    float fBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, fBlock))
    {
      ConvertBlock(dBlock, fBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    half hBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, hBlock))
    {
      ConvertBlock(dBlock, hBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint16_t wBlock[MAX_BLOCK * 4];
    if (ReadBlockRGBA(x, y, w, h, wBlock))
    {
      ConvertBlock(dBlock, wBlock, w * h * 4);
      m_bPerformingConversion = false;
      return true;
    }

    std::uint32_t dwBlock[MAX_BLOCK];
    if (ReadBlockRGBA(x, y, w, h, dwBlock))
    {
      ConvertBlock(dBlock, (std::uint8_t*)dwBlock, w * h * 4);
      SwizzleBlock(dBlock, w * h);
      m_bPerformingConversion = false;
      return true;
    }

    assert(0);
    m_bPerformingConversion = false;
    return false;
  
}

bool CCodecBuffer::WriteBlock(std::uint32_t /*x*/, std::uint32_t /*y*/,
                              std::uint32_t* /*pBlock*/,
                              std::uint32_t /*dwBlockSize*/)
{
  assert(0);
  return false;
}

bool CCodecBuffer::ReadBlock(std::uint32_t /*x*/, std::uint32_t /*y*/,
                             std::uint32_t* /*pBlock*/,
                             std::uint32_t /*dwBlockSize*/)
{
  assert(0);
  return false;
}

void CCodecBuffer::ConvertBlock(double dBlock[], const float fBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dBlock);
  assert(fBlock);
  assert(dwBlockSize);
  if (dBlock && fBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dBlock[i] = fBlock[i];
  }
}

void CCodecBuffer::ConvertBlock(double dBlock[], half hBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dBlock);
  assert(hBlock);
  assert(dwBlockSize);
  if (dBlock && hBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dBlock[i] = hBlock[i];
  }
}

void CCodecBuffer::ConvertBlock(double dBlock[], const std::uint32_t dwBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dBlock);
  assert(dwBlock);
  assert(dwBlockSize);
  if (dBlock && dwBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dBlock[i] = CONVERT_DWORD_TO_FLOAT(dwBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(double dBlock[], const std::uint16_t wBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dBlock);
  assert(wBlock);
  assert(dwBlockSize);
  if (dBlock && wBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dBlock[i] = CONVERT_WORD_TO_FLOAT(wBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(double dBlock[], const std::uint8_t cBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dBlock);
  assert(cBlock);
  assert(dwBlockSize);
  if (dBlock && cBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dBlock[i] = CONVERT_BYTE_TO_FLOAT(cBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(float fBlock[], const double dBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(fBlock);
  assert(dBlock);
  assert(dwBlockSize);
  if (fBlock && dBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      fBlock[i] = (float)dBlock[i];
  }
}

void CCodecBuffer::ConvertBlock(float fBlock[], half hBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(fBlock);
  assert(hBlock);
  assert(dwBlockSize);
  if (fBlock && hBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      fBlock[i] = (float)hBlock[i];
  }
}

void CCodecBuffer::ConvertBlock(float fBlock[], const std::uint32_t dwBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(fBlock);
  assert(dwBlock);
  assert(dwBlockSize);
  if (fBlock && dwBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      fBlock[i] = CONVERT_DWORD_TO_FLOAT(dwBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(float fBlock[], const std::uint16_t wBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(fBlock);
  assert(wBlock);
  assert(dwBlockSize);
  if (fBlock && wBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      fBlock[i] = CONVERT_WORD_TO_FLOAT(wBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(float fBlock[], const std::uint8_t cBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(fBlock);
  assert(cBlock);
  assert(dwBlockSize);
  if (fBlock && cBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      fBlock[i] = CONVERT_BYTE_TO_FLOAT(cBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(half hBlock[], double dBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(hBlock);
  assert(dBlock);
  assert(dwBlockSize);
  if (hBlock && dBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      hBlock[i] = (float)dBlock[i];
  }
}

void CCodecBuffer::ConvertBlock(half hBlock[], float fBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(hBlock);
  assert(fBlock);
  assert(dwBlockSize);
  if (hBlock && fBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      hBlock[i] = (float)fBlock[i];
  }
}

void CCodecBuffer::ConvertBlock(half hBlock[], const std::uint32_t dwBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(hBlock);
  assert(dwBlock);
  assert(dwBlockSize);
  if (hBlock && dwBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      hBlock[i] = CONVERT_DWORD_TO_FLOAT(dwBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(half hBlock[], const std::uint16_t wBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(hBlock);
  assert(wBlock);
  assert(dwBlockSize);
  if (hBlock && wBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      hBlock[i] = CONVERT_WORD_TO_FLOAT(wBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(half hBlock[], const std::uint8_t cBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(hBlock);
  assert(cBlock);
  assert(dwBlockSize);
  if (hBlock && cBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      hBlock[i] = CONVERT_BYTE_TO_FLOAT(cBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint32_t dwBlock[], const double dBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dwBlock);
  assert(dBlock);
  assert(dwBlockSize);
  if (dwBlock && dBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dwBlock[i] = CONVERT_FLOAT_TO_DWORD(dBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint32_t dwBlock[], const float fBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dwBlock);
  assert(fBlock);
  assert(dwBlockSize);
  if (dwBlock && fBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dwBlock[i] = CONVERT_FLOAT_TO_DWORD(fBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint32_t dwBlock[], half hBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dwBlock);
  assert(hBlock);
  assert(dwBlockSize);
  if (dwBlock && hBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dwBlock[i] = CONVERT_FLOAT_TO_DWORD(hBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint32_t dwBlock[], const std::uint16_t wBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dwBlock);
  assert(wBlock);
  assert(dwBlockSize);
  if (dwBlock && wBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dwBlock[i] = CONVERT_WORD_TO_DWORD(wBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint32_t dwBlock[], const std::uint8_t cBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dwBlock);
  assert(cBlock);
  assert(dwBlockSize);
  if (dwBlock && cBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      dwBlock[i] = CONVERT_BYTE_TO_DWORD(cBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint16_t wBlock[], const double dBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(wBlock);
  assert(dBlock);
  assert(dwBlockSize);
  if (wBlock && dBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      wBlock[i] = CONVERT_FLOAT_TO_WORD(dBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint16_t wBlock[], const float fBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(wBlock);
  assert(fBlock);
  assert(dwBlockSize);
  if (wBlock && fBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      wBlock[i] = CONVERT_FLOAT_TO_WORD(fBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint16_t wBlock[], half hBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(wBlock);
  assert(hBlock);
  assert(dwBlockSize);
  if (wBlock && hBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      wBlock[i] = CONVERT_FLOAT_TO_WORD(hBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint16_t wBlock[], const std::uint32_t dwBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(wBlock);
  assert(dwBlock);
  assert(dwBlockSize);
  if (wBlock && dwBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      wBlock[i] = CONVERT_DWORD_TO_WORD(dwBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint16_t wBlock[], const std::uint8_t cBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(wBlock);
  assert(cBlock);
  assert(dwBlockSize);
  if (wBlock && cBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      wBlock[i] = CONVERT_BYTE_TO_WORD(cBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint8_t cBlock[], const double dBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(cBlock);
  assert(dBlock);
  assert(dwBlockSize);
  if (cBlock && dBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      cBlock[i] = CONVERT_FLOAT_TO_BYTE(dBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint8_t cBlock[], const float fBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(cBlock);
  assert(fBlock);
  assert(dwBlockSize);
  if (cBlock && fBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      cBlock[i] = CONVERT_FLOAT_TO_BYTE(fBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint8_t cBlock[], half hBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(cBlock);
  assert(hBlock);
  assert(dwBlockSize);
  if (cBlock && hBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      cBlock[i] = CONVERT_FLOAT_TO_BYTE(hBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint8_t cBlock[], const std::uint32_t dwBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(cBlock);
  assert(dwBlock);
  assert(dwBlockSize);
  if (cBlock && dwBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      cBlock[i] = CONVERT_DWORD_TO_BYTE(dwBlock[i]);
  }
}

void CCodecBuffer::ConvertBlock(std::uint8_t cBlock[], const std::uint16_t wBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(cBlock);
  assert(wBlock);
  assert(dwBlockSize);
  if (cBlock && wBlock && dwBlockSize)
  {
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      cBlock[i] = CONVERT_WORD_TO_BYTE(wBlock[i]);
  }
}

void CCodecBuffer::SwizzleBlock(double dBlock[], std::uint32_t dwBlockSize)
{
  assert(dBlock);
  assert(dwBlockSize);
  if (dBlock && dwBlockSize)
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      SWAP_DOUBLES(dBlock[(i * 4)], dBlock[(i * 4) + 2]);
}

void CCodecBuffer::SwizzleBlock(float fBlock[], std::uint32_t dwBlockSize)
{
  assert(fBlock);
  assert(dwBlockSize);
  if (fBlock && dwBlockSize)
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      SWAP_FLOATS(fBlock[(i * 4)], fBlock[(i * 4) + 2]);
}

void CCodecBuffer::SwizzleBlock(half hBlock[], std::uint32_t dwBlockSize)
{
  assert(hBlock);
  assert(dwBlockSize);
  if (hBlock && dwBlockSize)
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      SWAP_HALFS(hBlock[(i * 4)], hBlock[(i * 4) + 2]);
}

void CCodecBuffer::SwizzleBlock(std::uint32_t dwBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(dwBlock);
  assert(dwBlockSize);
  if (dwBlock && dwBlockSize)
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      SWAP_DWORDS(dwBlock[(i * 4)], dwBlock[(i * 4) + 2]);
}

void CCodecBuffer::SwizzleBlock(std::uint16_t wBlock[],
                                std::uint32_t dwBlockSize)
{
  assert(wBlock);
  assert(dwBlockSize);
  if (wBlock && dwBlockSize)
    for (std::uint32_t i = 0; i < dwBlockSize; i++)
      SWAP_WORDS(wBlock[(i * 4)], wBlock[(i * 4) + 2]);
}
