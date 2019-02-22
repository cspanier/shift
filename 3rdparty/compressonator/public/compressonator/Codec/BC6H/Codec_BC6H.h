//===============================================================================
// Copyright (c) 2014-2016  Advanced Micro Devices, Inc. All rights reserved.
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
//  File Name:   Codec_BC7.cpp
//  Description: implementation of the CCodec_BC7 class
//
//////////////////////////////////////////////////////////////////////////////

#ifndef COMPRESSONATOR_CODEC_BC6H_H
#define COMPRESSONATOR_CODEC_BC6H_H

#include "compressonator/Compressonator.h"
#include "compressonator/Codec/DXTC/Codec_DXTC.h"
#include "BC6H_Encode.h"
#include "compressonator/Codec/BC6H/BC6H_Decode.h"
#include "BC6H_Library.h"

#include <thread>

struct BC6HEncodeThreadParam
{
  BC6HBlockEncoder* encoder;
  float in[MAX_SUBSET_SIZE][MAX_DIMENSION_BIG];
  std::uint8_t* out;
  volatile bool run;
  volatile bool exit;
};

class CCodec_BC6H : public CCodec_DXTC
{
public:
  CCodec_BC6H(CodecType codecType);
  ~CCodec_BC6H() final;

  bool SetParameter(const char* pszParamName, char* sValue) override;
  bool SetParameter(const char* /*pszParamName*/,
                    std::uint32_t /*dwValue*/) override;
  bool SetParameter(const char* /*pszParamName*/, float /*fValue*/) override;

  // Required interfaces
  CodecError Compress(CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
                      Codec_Feedback_Proc pFeedbackProc = nullptr,
                      std::size_t pUser1 = 0, std::size_t pUser2 = 0) override;
  virtual CodecError Compress_Fast(CCodecBuffer& bufferIn,
                                   CCodecBuffer& bufferOut,
                                   Codec_Feedback_Proc pFeedbackProc = nullptr,
                                   std::size_t pUser1 = 0,
                                   std::size_t pUser2 = 0);
  virtual CodecError Compress_SuperFast(
    CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
    Codec_Feedback_Proc pFeedbackProc = nullptr, std::size_t pUser1 = 0,
    std::size_t pUser2 = 0);
  CodecError Decompress(CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
                        Codec_Feedback_Proc pFeedbackProc = nullptr,
                        std::size_t pUser1 = 0,
                        std::size_t pUser2 = 0) override;

private:
  BC6HEncodeThreadParam* m_EncodeParameterStorage{};

  // BC6H User configurable variables
  std::uint16_t m_ModeMask;
  float m_Quality;
  std::uint16_t m_NumThreads;
  bool m_bIsSigned;
  bool m_UsePatternRec;
  float m_Exposure;

  // BC6H Internal status
  bool m_LibraryInitialized;
  bool m_Use_MultiThreading;
  std::uint16_t m_NumEncodingThreads;
  std::uint16_t m_LiveThreads;
  std::uint16_t m_LastThread;

  // BC6H Encoders and decoders: for encding use the interfaces below
  std::thread* m_EncodingThreadHandle;
  BC6HBlockEncoder* m_encoder[BC6H_MAX_THREADS]{};
  BC6HBlockDecoder* m_decoder{};

  // Encoder interfaces
  CodecError CInitializeBC6HLibrary();
  CodecError CEncodeBC6HBlock(float in[BC6H_BLOCK_PIXELS][MAX_DIMENSION_BIG],
                              std::uint8_t* out);
  CodecError CFinishBC6HEncoding(void);
};

#endif
