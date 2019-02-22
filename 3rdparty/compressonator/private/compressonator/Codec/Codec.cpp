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
//  File Name:   Codec.cpp
//  Description: Implementation of the CCodec class
//
//////////////////////////////////////////////////////////////////////////////

#include "compressonator/Common.h"
#include "compressonator/Codec/Codec.h"
#include "compressonator/Codec/DXT/Codec_DXT1.h"
#include "compressonator/Codec/DXT/Codec_DXT3.h"
#include "compressonator/Codec/DXT/Codec_DXT5.h"
#include "compressonator/Codec/DXT/Codec_DXT5_xGBR.h"
#include "compressonator/Codec/DXT/Codec_DXT5_RxBG.h"
#include "compressonator/Codec/DXT/Codec_DXT5_RBxG.h"
#include "compressonator/Codec/DXT/Codec_DXT5_xRBG.h"
#include "compressonator/Codec/DXT/Codec_DXT5_RGxB.h"
#include "compressonator/Codec/DXT/Codec_DXT5_xGxR.h"
#include "compressonator/Codec/BC6H/Codec_BC6H.h"
#include "compressonator/Codec/BC7/Codec_BC7.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCodec::CCodec(CodecType codecType)
{
  m_CodecType = codecType;
}

CCodec::~CCodec() = default;

bool CCodec::SetParameter(const char* /*pszParamName*/, char* /*dwValue*/)
{
  return false;
}

bool CCodec::SetParameter(const char* /*pszParamName*/,
                          std::uint32_t /*dwValue*/)
{
  return false;
}

bool CCodec::GetParameter(const char* /*pszParamName*/,
                          std::uint32_t& /*dwValue*/)
{
  return false;
}

bool CCodec::SetParameter(const char* /*pszParamName*/, float /*fValue*/)
{
  return false;
}

bool CCodec::GetParameter(const char* /*pszParamName*/, float& /*fValue*/)
{
  return false;
}

#ifdef _WIN32

#include <intrin.h>

//  Windows
#define cpuid(info, x) __cpuidex(info, x, 0)

#else

//  GCC Intrinsics
#include <cpuid.h>
void cpuid(int info[4], int InfoType)
{
  __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
}

#endif

bool SupportsSSE()
{
#if defined(USE_SSE)
  int info[4];
  cpuid(info, 0);

  int nIds = info[0];

  if (nIds >= 1)
  {
    return ((info[3] & ((int)1 << 25)) != 0);
  }
#endif
  return false;
}

bool SupportsSSE2()
{
#if defined(USE_SSE2) && defined(_WIN32)
  int info[4];
  cpuid(info, 0);

  int nIds = info[0];

  if (nIds >= 1)
  {
    return ((info[3] & ((int)1 << 26)) != 0);
  }
#endif
  return false;
}

CCodec* CreateCodec(CodecType nCodecType)
{
  switch (nCodecType)
  {
  case CT_DXT1:
    return new CCodec_DXT1;
  case CT_DXT3:
    return new CCodec_DXT3;
  case CT_DXT5:
    return new CCodec_DXT5;
  case CT_DXT5_xGBR:
    return new CCodec_DXT5_xGBR;
  case CT_DXT5_RxBG:
    return new CCodec_DXT5_RxBG;
  case CT_DXT5_RBxG:
    return new CCodec_DXT5_RBxG;
  case CT_DXT5_xRBG:
    return new CCodec_DXT5_xRBG;
  case CT_DXT5_RGxB:
    return new CCodec_DXT5_RGxB;
  case CT_DXT5_xGxR:
    return new CCodec_DXT5_xGxR;
  case CT_BC6H:
  case CT_BC6H_SF:
    return new CCodec_BC6H(nCodecType);
  case CT_BC7:
    return new CCodec_BC7;
  case CT_Unknown:
  default:
    assert(0);
    return nullptr;
  }
}

std::uint32_t CalcBufferSize(CodecType nCodecType, std::uint32_t dwWidth,
                             std::uint32_t dwHeight,
                             std::uint8_t /*nBlockWidth*/,
                             std::uint8_t /*nBlockHeight*/)
{
  std::uint32_t dwChannels;
  std::uint32_t dwBitsPerChannel;
  std::uint32_t buffsize = 0;

  switch (nCodecType)
  {
  // Block size is 4x4 and 64 bits per block
  case CT_DXT1:
  case CT_ATI1N:
    dwChannels = 1;
    dwBitsPerChannel = 4;
    dwWidth = ((dwWidth + 3) / 4) * 4;
    dwHeight = ((dwHeight + 3) / 4) * 4;
    buffsize = (dwWidth * dwHeight * dwChannels * dwBitsPerChannel) / 8;
    break;

  // Block size is 4x4 and 128 bits per block
  case CT_DXT3:
  case CT_DXT5:
  case CT_DXT5_xGBR:
  case CT_DXT5_RxBG:
  case CT_DXT5_RBxG:
  case CT_DXT5_xRBG:
  case CT_DXT5_RGxB:
  case CT_DXT5_xGxR:
  case CT_ATI2N:
  case CT_ATI2N_XY:
  case CT_ATI2N_DXT5:
    dwChannels = 2;
    dwBitsPerChannel = 4;
    dwWidth = ((dwWidth + 3) / 4) * 4;
    dwHeight = ((dwHeight + 3) / 4) * 4;
    buffsize = (dwWidth * dwHeight * dwChannels * dwBitsPerChannel) / 8;
    break;

  // Block size is 4x4 and 128 bits per block
  case CT_BC6H:
  case CT_BC6H_SF:
    dwWidth = ((dwWidth + 3) / 4) * 4;
    dwHeight = ((dwHeight + 3) / 4) * 4;
    buffsize = dwWidth * dwHeight;
    if (buffsize < BC6H_BLOCK_PIXELS)
      buffsize = BC6H_BLOCK_PIXELS;
    break;

  // Block size is 4x4 and 128 bits per block
  case CT_BC7:
    dwWidth = ((dwWidth + 3) / 4) * 4;
    dwHeight = ((dwHeight + 3) / 4) * 4;
    buffsize = dwWidth * dwHeight;
    if (buffsize < BC7_BLOCK_PIXELS)
      buffsize = BC7_BLOCK_PIXELS;
    break;

  default:
    return 0;
  }

  return buffsize;
}
