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
//  File Name:   Codec_ATI1N.h
//  Description: interface for the CCodec_ATI1N class
//
//////////////////////////////////////////////////////////////////////////////

#ifndef COMPRESSONATOR_CODEC_ATI1N_H
#define COMPRESSONATOR_CODEC_ATI1N_H

#include "compressonator/Codec/DXTC/Codec_DXTC.h"

class CCodec_ATI1N : public CCodec_DXTC
{
public:
  CCodec_ATI1N(CodecType codecType = CT_ATI1N);
  ~CCodec_ATI1N() override;

  CodecError Compress(CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
                      Codec_Feedback_Proc pFeedbackProc = nullptr,
                      std::size_t pUser1 = 0, std::size_t pUser2 = 0) override;
  virtual CodecError Compress_Fast(CCodecBuffer& bufferIn,
                                   CCodecBuffer& bufferOut,
                                   Codec_Feedback_Proc pFeedbackProc = nullptr,
                                   std::size_t pUser1 = 0,
                                   std::size_t pUser2 = 0);
  CodecError Decompress(CCodecBuffer& bufferIn, CCodecBuffer& bufferOut,
                        Codec_Feedback_Proc pFeedbackProc = nullptr,
                        std::size_t pUser1 = 0,
                        std::size_t pUser2 = 0) override;

  CCodecBuffer* CreateBuffer(std::uint8_t nBlockWidth,
                             std::uint8_t nBlockHeight,
                             std::uint8_t nBlockDepth, std::uint32_t dwWidth,
                             std::uint32_t dwHeight, std::uint32_t dwPitch = 0,
                             std::uint8_t* pData = nullptr) const override;
};

#endif
