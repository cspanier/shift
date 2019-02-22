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
//  File Name:   CodecBuffer_RG32.cpp
//  Description: implementation of the CCodecBuffer_RG32 class
//
//////////////////////////////////////////////////////////////////////////////

#include "compressonator/Common.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RG32.h"

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

const int nChannelCount = 2;
const int nPixelSize = nChannelCount * sizeof(std::uint32_t);

CCodecBuffer_RG32::CCodecBuffer_RG32(std::uint8_t nBlockWidth,
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

CCodecBuffer_RG32::~CCodecBuffer_RG32()
= default;

void CCodecBuffer_RG32::Copy(CCodecBuffer& srcBuffer)
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
      std::uint32_t block[BLOCK_SIZE_4X4X4];
      srcBuffer.ReadBlockRGBA(i * 4, j * 4, 4, 4, block);
      WriteBlockRGBA(i * 4, j * 4, 4, 4, block);
    }
  }
}
bool CCodecBuffer_RG32::ReadBlock(std::uint32_t x, std::uint32_t y,
                                  std::uint8_t w, std::uint8_t h,
                                  std::uint32_t dwBlock[],
                                  std::uint32_t dwChannelOffset)
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
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (i = 0; i < dwWidth; i++)
    {
      dwBlock[(j * w) + i] = *(pData + dwChannelOffset);
      pData += nChannelCount;
    }

    // Pad line with previous values if necessary
    if (i < w)
      PadLine(i, w, 1, &dwBlock[j * w]);
  }

  // Pad block with previous values if necessary
  if (j < h)
    PadBlock(j, w, h, 1, dwBlock);

  return true;
}

bool CCodecBuffer_RG32::WriteBlock(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   std::uint32_t dwBlock[],
                                   std::uint32_t dwChannelOffset)
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  std::uint32_t dwWidth = min(w, (GetWidth() - x));

  for (std::uint32_t j = 0; j < h && (y + j) < GetHeight(); j++)
  {
    auto* pData =
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) +
                       (x * nChannelCount * sizeof(std::uint32_t)));
    for (std::uint32_t i = 0; i < dwWidth; i++)
    {
      *(pData + dwChannelOffset) = dwBlock[(j * w) + i];
      pData += nChannelCount;
    }
  }
  return true;
}

bool CCodecBuffer_RG32::ReadBlockA(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   std::uint32_t dwBlock[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  memset(dwBlock, 0, w * h * sizeof(std::uint32_t));

  return true;
}

#define RG32_OFFSET_R 1
#define RG32_OFFSET_G 0

bool CCodecBuffer_RG32::ReadBlockR(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   std::uint32_t dwBlock[])
{
  return ReadBlock(x, y, w, h, dwBlock, RG32_OFFSET_R);
}

bool CCodecBuffer_RG32::ReadBlockG(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   std::uint32_t dwBlock[])
{
  return ReadBlock(x, y, w, h, dwBlock, RG32_OFFSET_G);
}

bool CCodecBuffer_RG32::ReadBlockB(std::uint32_t x, std::uint32_t y,
                                   std::uint8_t w, std::uint8_t h,
                                   std::uint32_t dwBlock[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  memset(dwBlock, 0, w * h * sizeof(std::uint32_t));

  return true;
}

bool CCodecBuffer_RG32::WriteBlockA(std::uint32_t x, std::uint32_t y,
                                    std::uint8_t /*w*/, std::uint8_t /*h*/,
                                    std::uint32_t /*dwBlock*/[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  return !(x >= GetWidth() || y >= GetHeight());
}

bool CCodecBuffer_RG32::WriteBlockR(std::uint32_t x, std::uint32_t y,
                                    std::uint8_t w, std::uint8_t h,
                                    std::uint32_t dwBlock[])
{
  return WriteBlock(x, y, w, h, dwBlock, RG32_OFFSET_R);
}

bool CCodecBuffer_RG32::WriteBlockG(std::uint32_t x, std::uint32_t y,
                                    std::uint8_t w, std::uint8_t h,
                                    std::uint32_t dwBlock[])
{
  return WriteBlock(x, y, w, h, dwBlock, RG32_OFFSET_G);
}

bool CCodecBuffer_RG32::WriteBlockB(std::uint32_t x, std::uint32_t y,
                                    std::uint8_t /*w*/, std::uint8_t /*h*/,
                                    std::uint32_t /*dwBlock*/[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  return !(x >= GetWidth() || y >= GetHeight());
}

#define GET_PIXEL(i, j) &dwBlock[((((j) * w) + (i)) * 4)]
bool CCodecBuffer_RG32::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                      std::uint8_t w, std::uint8_t h,
                                      std::uint32_t dwBlock[])
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
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (i = 0; i < dwWidth; i++)
    {
      std::uint32_t* pDest = GET_PIXEL(i, j);
      memcpy(GET_PIXEL(i, j), pData, nPixelSize);
      pData += nChannelCount;
      pDest += 2;
      *pDest++ = 0;
      *pDest++ = 0xffffffff;
    }

    // Pad line with previous values if necessary
    if (i < w)
      PadLine(i, w, 4, &dwBlock[j * w * 4]);
  }

  // Pad block with previous values if necessary
  if (j < h)
    PadBlock(j, w, h, 4, dwBlock);
  return true;
}

bool CCodecBuffer_RG32::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                       std::uint8_t w, std::uint8_t h,
                                       std::uint32_t dwBlock[])
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
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (std::uint32_t i = 0; i < dwWidth; i++)
    {
      memcpy(pData, GET_PIXEL(i, j), nPixelSize);
      pData += nChannelCount;
    }
  }

  return true;
}
