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
//  File Name:   CodecBuffer_Block.cpp
//  Description: implementation of the CCodecBuffer_Block class
//
//////////////////////////////////////////////////////////////////////////////

#include "compressonator/Common.h"
#include "compressonator/Codec/Buffer/CodecBuffer_Block.h"

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

CCodecBuffer_Block::CCodecBuffer_Block(
  CodecBufferType nCodecBufferType, std::uint8_t nBlockWidth,
  std::uint8_t nBlockHeight, std::uint8_t nBlockDepth, std::uint32_t dwWidth,
  std::uint32_t dwHeight, std::uint32_t dwPitch, std::uint8_t* pData)
: CCodecBuffer(nBlockWidth, nBlockHeight, nBlockDepth, dwWidth, dwHeight,
               dwPitch, pData),
  m_nCodecBufferType(nCodecBufferType)
{
  // ToDo: All of these cases all need to be updated from default values
  // to those passed down! for now we has asjusted the case
  // of CBT_4x4Block_8BPP which is common for DXTn and BCn (defaults should be
  // 4x4 however since the introduction of ASTC and GT Codec the sizes are now
  // variable from 4x4 to 12x12 (for ASTC) and 4x4 to BlockW x BlockH for GT
  // Should also investigate changing m_dwBlockBPP

  switch (nCodecBufferType)
  {
  case CBT_4x4Block_2BPP:
    m_dwBlockWidth = 4;
    m_dwBlockHeight = 4;
    m_dwBlockBPP = 2;
    break;
  case CBT_4x4Block_4BPP:
    m_dwBlockWidth = 4;
    m_dwBlockHeight = 4;
    m_dwBlockBPP = 4;
    break;
  case CBT_4x4Block_8BPP:
    m_dwBlockWidth = 4;
    m_dwBlockHeight = 4;
    m_dwBlockBPP = 8;
    break;
  case CBT_4x4Block_16BPP:
    m_dwBlockWidth = 4;
    m_dwBlockHeight = 4;
    m_dwBlockBPP = 16;
    break;
  case CBT_4x4Block_32BPP:
    m_dwBlockWidth = 4;
    m_dwBlockHeight = 4;
    m_dwBlockBPP = 32;
    break;
  case CBT_8x8Block_2BPP:
    m_dwBlockWidth = 8;
    m_dwBlockHeight = 8;
    m_dwBlockBPP = 2;
    break;
  case CBT_8x8Block_4BPP:
    m_dwBlockWidth = 8;
    m_dwBlockHeight = 8;
    m_dwBlockBPP = 4;
    break;
  case CBT_8x8Block_8BPP:
    m_dwBlockWidth = 8;
    m_dwBlockHeight = 8;
    m_dwBlockBPP = 8;
    break;
  case CBT_8x8Block_16BPP:
    m_dwBlockWidth = 8;
    m_dwBlockHeight = 8;
    m_dwBlockBPP = 16;
    break;
  case CBT_8x8Block_32BPP:
    m_dwBlockWidth = 8;
    m_dwBlockHeight = 8;
    m_dwBlockBPP = 32;
    break;
  default:
    m_dwBlockWidth = nBlockWidth;
    m_dwBlockHeight = nBlockHeight;
    m_dwBlockBPP = 8;
    break;
  }

  std::uint32_t dwBlocksX =
    ((GetWidth() + m_dwBlockWidth - 1) / m_dwBlockWidth);
  std::uint32_t dwBlocksY =
    ((GetHeight() + m_dwBlockHeight - 1) / m_dwBlockHeight);
  std::uint32_t dwBlocks = dwBlocksX * dwBlocksY;
  m_dwBlockSize = m_dwBlockWidth * m_dwBlockHeight * m_dwBlockBPP / 8;
  m_dwPitch = dwBlocksX * m_dwBlockSize;

  if (m_pData == nullptr)
  {
    std::uint32_t dwSize = dwBlocks * m_dwBlockSize;
    m_pData = (std::uint8_t*)malloc(dwSize);
  }
}

CCodecBuffer_Block::~CCodecBuffer_Block()
= default;

bool CCodecBuffer_Block::ReadBlock(std::uint32_t x, std::uint32_t y,
                                   std::uint32_t* pBlock,
                                   std::uint32_t dwBlockSize)
{
  assert(pBlock);
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (!pBlock || x >= GetWidth() || y >= GetHeight())
    return false;

  int offset = ((y / m_dwBlockHeight) * m_dwPitch) +
               ((x / m_dwBlockWidth) * dwBlockSize * sizeof(std::uint32_t));

  memcpy(pBlock, GetData() + offset, dwBlockSize * sizeof(std::uint32_t));

  return true;
}

bool CCodecBuffer_Block::WriteBlock(std::uint32_t x, std::uint32_t y,
                                    std::uint32_t* pBlock,
                                    std::uint32_t dwBlockSize)
{
  assert(pBlock);
  assert(x < GetWidth());
  assert(y < GetHeight());

  if (!pBlock || x >= GetWidth() || y >= GetHeight())
    return false;
  int offset = ((y / m_dwBlockHeight) * m_dwPitch) +
               ((x / m_dwBlockWidth) * dwBlockSize * sizeof(std::uint32_t));
  memcpy(GetData() + offset, pBlock, dwBlockSize * sizeof(std::uint32_t));

  return true;
}
