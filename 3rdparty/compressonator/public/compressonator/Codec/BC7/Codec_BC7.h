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

#ifndef COMPRESSONATOR_CODEC_BC7_H
#define COMPRESSONATOR_CODEC_BC7_H

#include "compressonator/Compressonator.h"
#include "compressonator/Codec/DXTC/Codec_DXTC.h"
#include "compressonator/Codec/BC7/BC7_Encode.h"
#include "compressonator/Codec/BC7/BC7_Decode.h"
#include "compressonator/Codec/BC7/BC7_Library.h"
#include <thread>

// #define USE_THREADED_CALLBACKS  // This is experimental code to improve
// compression performance!
#ifdef USE_THREADED_CALLBACKS
typedef struct
{
  float progress;
  bool abort;
} CMP_PROGRESS_THREAD;
#endif

struct BC7EncodeThreadParam
{
  BC7BlockEncoder* encoder;
  double in[MAX_SUBSET_SIZE][MAX_DIMENSION_BIG];
  std::uint8_t* out;
  volatile bool run;
  volatile bool exit;
};

class CCodec_BC7 : public CCodec_DXTC
{
public:
  CCodec_BC7();
  ~CCodec_BC7() final;

  virtual bool SetParameter(const char* pszParamName, char* sValue);
  virtual bool SetParameter(const char* /*pszParamName*/,
                            std::uint32_t /*dwValue*/);
  virtual bool SetParameter(const char* /*pszParamName*/, float /*fValue*/);

  // Required interfaces
  virtual CodecError Compress(CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
                              Codec_Feedback_Proc pFeedbackProc = nullptr,
                              std::size_t pUser1 = 0, std::size_t pUser2 = 0);
  virtual CodecError Compress_Fast(CCodecBuffer& bufferIn,
                                   CCodecBuffer& bufferOut,
                                   Codec_Feedback_Proc pFeedbackProc = nullptr,
                                   std::size_t pUser1 = 0,
                                   std::size_t pUser2 = 0);
  virtual CodecError Compress_SuperFast(
    CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
    Codec_Feedback_Proc pFeedbackProc = nullptr, std::size_t pUser1 = 0,
    std::size_t pUser2 = 0);
  virtual CodecError Decompress(CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
                                Codec_Feedback_Proc pFeedbackProc = nullptr,
                                std::size_t pUser1 = 0, std::size_t pUser2 = 0);

private:
  BC7EncodeThreadParam* m_EncodeParameterStorage{};

  // BC7 User configurable variables
  std::uint32_t m_ModeMask;
  double m_Quality;
  double m_Performance;
  bool m_ColourRestrict;
  bool m_AlphaRestrict;
  std::uint16_t m_NumThreads;
  bool m_ImageNeedsAlpha;

  // BC7 Internal status
  bool m_LibraryInitialized;
  bool m_Use_MultiThreading;
  std::uint16_t m_NumEncodingThreads;
  std::uint16_t m_LiveThreads;
  std::uint16_t m_LastThread;

  // BC7 Encoders and decoders: for encding use the interfaces below
  std::thread* m_EncodingThreadHandle;
  BC7BlockEncoder* m_encoder[MAX_BC7_THREADS]{};
  BC7BlockDecoder* m_decoder{};

  // Encoder interfaces
  CodecError InitializeBC7Library();
  CodecError EncodeBC7Block(double in[BC7_BLOCK_PIXELS][MAX_DIMENSION_BIG],
                            std::uint8_t* out);
  CodecError FinishBC7Encoding(void);

  static void Run();

#ifdef USE_THREADED_CALLBACKS
public:
  static CMP_PROGRESS_THREAD m_progress;
  static Codec_Feedback_Proc m_user_pFeedbackProc;
  static std::size_t m_pUser1;
  static std::size_t m_pUser2;
#endif
};

#endif
