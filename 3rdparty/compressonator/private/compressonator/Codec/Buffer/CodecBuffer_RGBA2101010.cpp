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
//  File Name:   CodecBuffer_RGBA2101010.cpp
//  Description: implementation of the CCodecBuffer_RGBA2101010 class
//
//////////////////////////////////////////////////////////////////////////////

#include "compressonator/Common.h"
#include "compressonator/Codec/Buffer/CodecBuffer_RGBA2101010.h"

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

const int nChannelCount = 4;
const int nPixelSize = sizeof(std::uint32_t);

CCodecBuffer_RGBA2101010::CCodecBuffer_RGBA2101010(
  std::uint8_t nBlockWidth, std::uint8_t nBlockHeight, std::uint8_t nBlockDepth,
  std::uint32_t dwWidth, std::uint32_t dwHeight, std::uint32_t dwPitch,
  std::uint8_t* pData)
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

CCodecBuffer_RGBA2101010::~CCodecBuffer_RGBA2101010()
{
}

void CCodecBuffer_RGBA2101010::Copy(CCodecBuffer& srcBuffer)
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
      std::uint32_t block[BLOCK_SIZE_4X4];
      srcBuffer.ReadBlockRGBA(i * 4, j * 4, 4, 4, block);
      WriteBlockRGBA(i * 4, j * 4, 4, 4, block);
    }
  }
}

bool CCodecBuffer_RGBA2101010::ReadBlock(std::uint32_t x, std::uint32_t y,
                                         std::uint8_t w, std::uint8_t h,
                                         std::uint16_t block[],
                                         std::uint32_t dwChannelMask,
                                         std::uint32_t dwChannelOffset,
                                         bool b10Bit)
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  std::uint32_t dwWidth = min(w, (GetWidth() - x));

  std::uint32_t i, j;
  for (j = 0; j < h && (y + j) < GetHeight(); j++)
  {
    std::uint32_t* pData =
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (i = 0; i < dwWidth; i++)
    {
      std::uint32_t dwData = *pData++;
      if (b10Bit)
        block[(j * w) + i] = CONVERT_10BIT_TO_WORD(static_cast<std::uint16_t>(
          ((dwData) >> dwChannelOffset) & dwChannelMask));
      else
        block[(j * w) + i] = CONVERT_2BIT_TO_WORD(static_cast<std::uint16_t>(
          ((dwData) >> dwChannelOffset) & dwChannelMask));
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

bool CCodecBuffer_RGBA2101010::WriteBlock(std::uint32_t x, std::uint32_t y,
                                          std::uint8_t w, std::uint8_t h,
                                          std::uint16_t block[],
                                          std::uint32_t dwChannelMask,
                                          std::uint32_t dwChannelOffset,
                                          bool b10Bit)
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  std::uint32_t dwMask = ~((std::uint32_t)dwChannelMask << dwChannelOffset);
  std::uint32_t dwWidth = min(w, (GetWidth() - x));

  for (std::uint32_t j = 0; j < h && (y + j) < GetHeight(); j++)
  {
    std::uint32_t* pData =
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (std::uint32_t i = 0; i < dwWidth; i++)
    {
      if (b10Bit)
        *pData = (*pData & dwMask) |
                 (((std::uint32_t)(CONVERT_WORD_TO_10BIT(block[(j * w) + i])))
                  << dwChannelOffset);
      else
        *pData = (*pData & dwMask) |
                 (((std::uint32_t)(CONVERT_WORD_TO_2BIT(block[(j * w) + i])))
                  << dwChannelOffset);
      pData++;
    }
  }
  return true;
}

bool CCodecBuffer_RGBA2101010::ReadBlockA(std::uint32_t x, std::uint32_t y,
                                          std::uint8_t w, std::uint8_t h,
                                          std::uint16_t block[])
{
  return ReadBlock(x, y, w, h, block, TWO_BIT_MASK, RGBA2101010_OFFSET_A,
                   false);
}

bool CCodecBuffer_RGBA2101010::ReadBlockR(std::uint32_t x, std::uint32_t y,
                                          std::uint8_t w, std::uint8_t h,
                                          std::uint16_t block[])
{
  return ReadBlock(x, y, w, h, block, TEN_BIT_MASK, RGBA2101010_OFFSET_R, true);
}

bool CCodecBuffer_RGBA2101010::ReadBlockG(std::uint32_t x, std::uint32_t y,
                                          std::uint8_t w, std::uint8_t h,
                                          std::uint16_t block[])
{
  return ReadBlock(x, y, w, h, block, TEN_BIT_MASK, RGBA2101010_OFFSET_G, true);
}

bool CCodecBuffer_RGBA2101010::ReadBlockB(std::uint32_t x, std::uint32_t y,
                                          std::uint8_t w, std::uint8_t h,
                                          std::uint16_t block[])
{
  return ReadBlock(x, y, w, h, block, TEN_BIT_MASK, RGBA2101010_OFFSET_B, true);
}

bool CCodecBuffer_RGBA2101010::WriteBlockA(std::uint32_t x, std::uint32_t y,
                                           std::uint8_t w, std::uint8_t h,
                                           std::uint16_t block[])
{
  return WriteBlock(x, y, w, h, block, TWO_BIT_MASK, RGBA2101010_OFFSET_A,
                    false);
}

bool CCodecBuffer_RGBA2101010::WriteBlockR(std::uint32_t x, std::uint32_t y,
                                           std::uint8_t w, std::uint8_t h,
                                           std::uint16_t block[])
{
  return WriteBlock(x, y, w, h, block, TEN_BIT_MASK, RGBA2101010_OFFSET_R,
                    true);
}

bool CCodecBuffer_RGBA2101010::WriteBlockG(std::uint32_t x, std::uint32_t y,
                                           std::uint8_t w, std::uint8_t h,
                                           std::uint16_t block[])
{
  return WriteBlock(x, y, w, h, block, TEN_BIT_MASK, RGBA2101010_OFFSET_G,
                    true);
}

bool CCodecBuffer_RGBA2101010::WriteBlockB(std::uint32_t x, std::uint32_t y,
                                           std::uint8_t w, std::uint8_t h,
                                           std::uint16_t block[])
{
  return WriteBlock(x, y, w, h, block, TEN_BIT_MASK, RGBA2101010_OFFSET_B,
                    true);
}

bool CCodecBuffer_RGBA2101010::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                             std::uint8_t w, std::uint8_t h,
                                             std::uint32_t block[])
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
    std::uint32_t* pData =
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (i = 0; i < dwWidth; i++)
      block[(j * w) + i] = *pData++;

    // Pad line with previous values if necessary
    if (i < w)
      PadLine(i, w, 4, &block[j * w * 4]);
  }

  // Pad block with previous values if necessary
  if (j < h)
    PadBlock(j, w, h, 4, block);

  return true;
}

bool CCodecBuffer_RGBA2101010::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                              std::uint8_t w, std::uint8_t h,
                                              std::uint32_t block[])
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
    std::uint32_t* pData =
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (std::uint32_t i = 0; i < dwWidth; i++)
      *pData++ = block[(j * w) + i];
  }
  return true;
}

#define GET_PIXEL(i, j) &block[(((j * w) + i) * 4)]
bool CCodecBuffer_RGBA2101010::ReadBlockRGBA(std::uint32_t x, std::uint32_t y,
                                             std::uint8_t w, std::uint8_t h,
                                             std::uint16_t block[])
{
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (x >= GetWidth() || y >= GetHeight())
    return false;

  std::uint32_t dwWidth = min(w, (GetWidth() - x));

  std::uint32_t i, j;
  for (j = 0; j < h && (y + j) < GetHeight(); j++)
  {
    std::uint32_t* pData =
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (i = 0; i < dwWidth; i++)
    {
      block[(((j * w) + i) * 4) + 0] = CONVERT_10BIT_TO_WORD(
        ((*pData) >> RGBA2101010_OFFSET_R) & TEN_BIT_MASK);
      block[(((j * w) + i) * 4) + 1] = CONVERT_10BIT_TO_WORD(
        ((*pData) >> RGBA2101010_OFFSET_G) & TEN_BIT_MASK);
      block[(((j * w) + i) * 4) + 2] = CONVERT_10BIT_TO_WORD(
        ((*pData) >> RGBA2101010_OFFSET_B) & TEN_BIT_MASK);
      block[(((j * w) + i) * 4) + 3] =
        CONVERT_2BIT_TO_WORD(((*pData) >> RGBA2101010_OFFSET_A) & TWO_BIT_MASK);
      pData++;
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

bool CCodecBuffer_RGBA2101010::WriteBlockRGBA(std::uint32_t x, std::uint32_t y,
                                              std::uint8_t w, std::uint8_t h,
                                              std::uint16_t block[])
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
    std::uint32_t* pData =
      (std::uint32_t*)(GetData() + ((y + j) * m_dwPitch) + (x * nPixelSize));
    for (std::uint32_t i = 0; i < dwWidth; i++)
    {
      std::uint16_t wRed = block[(((j * w) + i) * 4) + 0];
      std::uint16_t wGreen = block[(((j * w) + i) * 4) + 1];
      std::uint16_t wBlue = block[(((j * w) + i) * 4) + 2];
      std::uint16_t wAlpha = block[(((j * w) + i) * 4) + 3];
      *pData++ = (CONVERT_WORD_TO_10BIT(wRed) << RGBA2101010_OFFSET_R) |
                 (CONVERT_WORD_TO_10BIT(wGreen) << RGBA2101010_OFFSET_G) |
                 (CONVERT_WORD_TO_10BIT(wBlue) << RGBA2101010_OFFSET_B) |
                 (CONVERT_WORD_TO_10BIT(wAlpha) << RGBA2101010_OFFSET_A);
    }
  }
  return true;
}
