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
//  File Name:   Codec_DXT5_xGBR.cpp
//  Description: implementation of the CCodec_DXT5_xGBR class
//
//////////////////////////////////////////////////////////////////////////////

#include "compressonator/Common.h"
#include "compressonator/Codec/DXT/Codec_DXT5_xGBR.h"

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

CCodec_DXT5_xGBR::CCodec_DXT5_xGBR() : CCodec_DXT5_Swizzled(CT_DXT5_xGBR)
{
}

CCodec_DXT5_xGBR::~CCodec_DXT5_xGBR()
{
}

void CCodec_DXT5_xGBR::ReadBlock(CCodecBuffer& buffer, std::uint32_t x,
                                 std::uint32_t y,
                                 std::uint8_t block[BLOCK_SIZE_4X4X4])
{
  std::uint8_t dwTempBlock[BLOCK_SIZE_4X4X4];
  buffer.ReadBlockRGBA(x, y, 4, 4, dwTempBlock);
  for (std::uint32_t i = 0; i < BLOCK_SIZE_4X4; i++)
    ((std::uint32_t*)block)[i] =
      SWIZZLE_RGBA_xGBR(((std::uint32_t*)dwTempBlock)[i]);
}

void CCodec_DXT5_xGBR::WriteBlock(CCodecBuffer& buffer, std::uint32_t x,
                                  std::uint32_t y,
                                  std::uint8_t block[BLOCK_SIZE_4X4X4])
{
  std::uint8_t dwTempBlock[BLOCK_SIZE_4X4X4];
  for (std::uint32_t i = 0; i < BLOCK_SIZE_4X4; i++)
    ((std::uint32_t*)dwTempBlock)[i] =
      SWIZZLE_xGBR_RGBA(((std::uint32_t*)block)[i]);
  buffer.WriteBlockRGBA(x, y, 4, 4, dwTempBlock);
}

void CCodec_DXT5_xGBR::ReadBlock(CCodecBuffer& buffer, std::uint32_t x,
                                 std::uint32_t y, float block[BLOCK_SIZE_4X4X4])
{
  float fTempBlock[BLOCK_SIZE_4X4X4];
  buffer.ReadBlockRGBA(x, y, 4, 4, fTempBlock);
  for (std::uint32_t i = 0; i < BLOCK_SIZE_4X4; i++)
  {
    block[(i * 4) + RGBA32F_OFFSET_R] = 0.0;
    block[(i * 4) + RGBA32F_OFFSET_G] = fTempBlock[(i * 4) + RGBA32F_OFFSET_G];
    block[(i * 4) + RGBA32F_OFFSET_B] = fTempBlock[(i * 4) + RGBA32F_OFFSET_B];
    block[(i * 4) + RGBA32F_OFFSET_A] = fTempBlock[(i * 4) + RGBA32F_OFFSET_R];
  }
}

void CCodec_DXT5_xGBR::WriteBlock(CCodecBuffer& buffer, std::uint32_t x,
                                  std::uint32_t y,
                                  float block[BLOCK_SIZE_4X4X4])
{
  float fTempBlock[BLOCK_SIZE_4X4X4];
  for (std::uint32_t i = 0; i < BLOCK_SIZE_4X4; i++)
  {
    fTempBlock[(i * 4) + RGBA32F_OFFSET_R] = block[(i * 4) + RGBA32F_OFFSET_A];
    fTempBlock[(i * 4) + RGBA32F_OFFSET_G] = block[(i * 4) + RGBA32F_OFFSET_G];
    fTempBlock[(i * 4) + RGBA32F_OFFSET_B] = block[(i * 4) + RGBA32F_OFFSET_B];
    fTempBlock[(i * 4) + RGBA32F_OFFSET_A] = 0.0;
  }
  buffer.WriteBlockRGBA(x, y, 4, 4, fTempBlock);
}