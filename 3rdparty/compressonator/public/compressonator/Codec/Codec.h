//===============================================================================
// Copyright (c) 2007-2016  Advanced Micro Devices, Inc. All rights reserved.
// Copyright (c) 2004-2006 ATI Technologies Inc.
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
//  File Name:   Codec.h
//  Description: interface for the CCodec class
//
//////////////////////////////////////////////////////////////////////////////

#ifndef COMPRESSONATOR_CODEC_H
#define COMPRESSONATOR_CODEC_H

#include "compressonator/Codec/Buffer/CodecBuffer.h"

#define SAFE_DELETE(p) \
  if (p)               \
  {                    \
    delete p;          \
    p = nullptr;       \
  }

typedef enum _CodecType
{
  CT_Unknown = 0,
  CT_None,
  CT_DXT1,
  CT_DXT3,
  CT_DXT5,
  CT_DXT5_xGBR,
  CT_DXT5_RxBG,
  CT_DXT5_RBxG,
  CT_DXT5_xRBG,
  CT_DXT5_RGxB,
  CT_DXT5_xGxR,
  CT_ATI1N,
  CT_ATI2N,
  CT_ATI2N_XY,
  CT_ATI2N_DXT5,
  CT_BC6H,
  CT_BC6H_SF,
  CT_BC7,
  CODECS_AMD_INTERNAL
} CodecType;

typedef enum _CODECError
{
  CE_OK = 0,
  CE_Unknown,
  CE_Aborted,
} CodecError;

typedef bool (*Codec_Feedback_Proc)(float fProgress, std::size_t pUser1,
                                    std::size_t pUser2);

class CCodec
{
public:
  CCodec(CodecType codecType);
  virtual ~CCodec();

  virtual bool SetParameter(const char* pszParamName, std::uint32_t dwValue);
  virtual bool GetParameter(const char* pszParamName, std::uint32_t& dwValue);

  virtual bool SetParameter(const char* pszParamName, float fValue);
  virtual bool GetParameter(const char* pszParamName, float& fValue);

  virtual bool SetParameter(const char* pszParamName, char* dwValue);

  virtual CodecType GetType() const
  {
    return m_CodecType;
  }

  virtual std::uint32_t GetBlockHeight()
  {
    return 1;
  }

  virtual CCodecBuffer* CreateBuffer(
    std::uint8_t nBlockWidth, std::uint8_t nBlockHeight,
    std::uint8_t nBlockDepth, std::uint32_t dwWidth, std::uint32_t dwHeight,
    std::uint32_t dwPitch = 0, std::uint8_t* pData = nullptr) const = 0;

  virtual CodecError Compress(CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
                              Codec_Feedback_Proc pFeedbackProc = nullptr,
                              std::size_t pUser1 = 0,
                              std::size_t pUser2 = 0) = 0;
  virtual CodecError Decompress(CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
                                Codec_Feedback_Proc pFeedbackProc = nullptr,
                                std::size_t pUser1 = 0,
                                std::size_t pUser2 = 0) = 0;

protected:
  CodecType m_CodecType;
};

bool SupportsSSE();
bool SupportsSSE2();

CCodec* CreateCodec(CodecType nCodecType);
std::uint32_t CalcBufferSize(CodecType nCodecType, std::uint32_t dwWidth,
                             std::uint32_t dwHeight, std::uint8_t nBlockWidth,
                             std::uint8_t nBlockHeight);
std::uint32_t CalcBufferSize(cmp_format format, std::uint32_t dwWidth,
                             std::uint32_t dwHeight, std::uint32_t dwPitch,
                             std::uint8_t nBlockWidth,
                             std::uint8_t nBlockHeight);

std::uint8_t DeriveB(std::uint8_t R, std::uint8_t G);
float DeriveB(float R, float G);

#endif
