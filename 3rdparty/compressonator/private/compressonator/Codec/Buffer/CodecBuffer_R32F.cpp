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
//  File Name:   CodecBuffer_R32F.cpp
//  Description: implementation of the CCodecBuffer_R32F class
//
//////////////////////////////////////////////////////////////////////////////

#include "compressonator/Common.h"
#include "compressonator/Codec/Buffer/CodecBuffer_R32F.h"

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

const int nChannelCount = 1;
const int nPixelSize = nChannelCount * sizeof(float);

CCodecBuffer_R32F::CCodecBuffer_R32F(std::uint8_t nBlockWidth,
                                     std::uint8_t nBlockHeight,
                                     std::uint8_t nBlockDepth,
                                     std::uint32_t dwWidth,
                                     std::uint32_t dwHeight,
                                     std::uint32_t dwPitch, std::uint8_t* pData)
: CCodecBuffer(nBlockWidth, nBlockHeight, nBlockDepth, dwWidth, dwHeight,
               dwPitch, pData)
{
  assert((m_dwPitch == 0) || (m_dwPitch >= GetWidth() * nPixelSize));
  if (m_dwPitch <= GetWidth() * nPixelSize)
    m_dwPitch = GetWidth() * nPixelSize;

  if (m_pData == nullptr)
  {
    std::uint32_t dwSize = m_dwPitch * GetHeight();
    m_pData = (std::uint8_t*)malloc(dwSize);
  }
}

CCodecBuffer_R32F::~CCodecBuffer_R32F()
= default;

void CCodecBuffer_R32F::Copy(CCodecBuffer& srcBuffer)
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

bool CCodecBuffer_R32F::ReadBlock(std::uint32_t x, std::uint32_t y,
                                  std::uint8_t w, std::uint8_t h, float block[],
                                  std::uint32_t dwChannelIndex)
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  std::uint32_t dwWidth = min(w, (GetWidth() - x));

  std::uint32_t i, j;
  for (j = 0; j < h && (y + j) < GetHeight(); j++)
  {
    auto* pData =
      (float*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (i = 0; i < dwWidth; i++)
    {
      block[(j * w) + i] = pData[dwChannelIndex];
      pData += nChannelCount;
    }

    // Pad line with previous values if necessary
    if (i < w)
      PadLine(i, w, 1, &block[j * w]);
  }

  // Pad block with previous values if necessary
  if (j < h)
    PadBlock(j, w, h, 1, block);

  return true;
}

bool CCodecBuffer_R32F::WriteBlock(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   float block[], std::uint32_t dwChannelIndex)
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  std::uint32_t dwWidth = min(w, (GetWidth() - x));

  for (std::uint32_t j = 0; j < h && (y + j) < GetHeight(); j++)
  {
    auto* pData =
      (float*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (std::uint32_t i = 0; i < dwWidth; i++)
    {
      pData[dwChannelIndex] = block[(j * w) + i];
      pData += nChannelCount;
    }
  }
  return true;
}

bool CCodecBuffer_R32F::ReadBlockA(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   float block[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  memset(block, 0, w * h * sizeof(float));

  return true;
}

bool CCodecBuffer_R32F::ReadBlockR(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   float block[])
{
  return ReadBlock(x, y, w, h, block, CHANNEL_INDEX_R);
}

bool CCodecBuffer_R32F::ReadBlockG(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   float block[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  memset(block, 0, w * h * sizeof(float));

  return true;
}

bool CCodecBuffer_R32F::ReadBlockB(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   float block[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  memset(block, 0, w * h * sizeof(float));

  return true;
}

bool CCodecBuffer_R32F::WriteBlockA(std::uint32_t x, std::uint32_t y,
                                    std::uint8_t /*w*/, std::uint8_t /*h*/,
                                    float /*block*/[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  return !(x >= GetWidth() || y >= GetHeight());
}

bool CCodecBuffer_R32F::WriteBlockR(std::uint32_t x, std::uint32_t y,
                                    std::uint8_t w, std::uint8_t h,
                                    float block[])
{
  return WriteBlock(x, y, w, h, block, CHANNEL_INDEX_R);
}

bool CCodecBuffer_R32F::WriteBlockG(std::uint32_t x, std::uint32_t y,
                                    std::uint8_t /*w*/, std::uint8_t /*h*/,
                                    float /*block*/[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  return !(x >= GetWidth() || y >= GetHeight());
}

bool CCodecBuffer_R32F::WriteBlockB(std::uint32_t x, std::uint32_t y,
                                    std::uint8_t /*w*/, std::uint8_t /*h*/,
                                    float /*block*/[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  return !(x >= GetWidth() || y >= GetHeight());
}

#define GET_PIXEL(i, j) &block[((((j) * w) + (i)) * 4)]
bool CCodecBuffer_R32F::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                      std::uint8_t w, std::uint8_t h,
                                      float block[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());
  assert(x % w == 0);
  assert(y % h == 0);

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  std::uint32_t dwWidth = min(w, (GetWidth() - x));

  std::uint32_t i, j;
  for (j = 0; j < h && (y + j) < GetHeight(); j++)
  {
    auto* pData =
      (float*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (i = 0; i < dwWidth; i++)
    {
      float* pDest = GET_PIXEL(i, j);
      *pDest++ = *pData++;
      *pDest++ = 0.0f;
      *pDest++ = 0.0f;
      *pDest++ = 1.0f;
    }

    // Pad line with previous values if necessary
    if (i < w)
      PadLine(i, w, 4, &block[j * w * 4]);
  }

  // Pad block with previous values if necessary
  if (j < h)
    PadBlock(j, w, h, 4, block);
  return true;
}

bool CCodecBuffer_R32F::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                       std::uint8_t w, std::uint8_t h,
                                       float block[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());
  assert(x % w == 0);
  assert(y % h == 0);

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  std::uint32_t dwWidth = min(w, (GetWidth() - x));

  for (std::uint32_t j = 0; j < h && (y + j) < GetHeight(); j++)
  {
    auto* pData =
      (float*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (std::uint32_t i = 0; i < dwWidth; i++)
    {
      memcpy(pData, GET_PIXEL(i, j), nPixelSize);
      pData += nChannelCount;
    }
  }
  return true;
}
