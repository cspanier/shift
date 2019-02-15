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
//  File Name:   CodecBuffer_RGBA2101010.h
//  Description: interface for the CCodecBuffer_RGBA2101010 class
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CODECBUFFER_RGBA2101010_H_INCLUDED_
#define _CODECBUFFER_RGBA2101010_H_INCLUDED_

#include "compressonator/Codec/Buffer/CodecBuffer.h"

class CCodecBuffer_RGBA2101010 : public CCodecBuffer
{
public:
  CCodecBuffer_RGBA2101010(std::uint8_t nBlockWidth, std::uint8_t nBlockHeight,
                           std::uint8_t nBlockDepth, std::uint32_t dwWidth,
                           std::uint32_t dwHeight, std::uint32_t dwPitch = 0,
                           std::uint8_t* pData = nullptr);
  virtual ~CCodecBuffer_RGBA2101010();

  virtual void Copy(CCodecBuffer& srcBuffer);

  virtual CodecBufferType GetBufferType() const
  {
    return CBT_RGBA2101010;
  };
  virtual std::uint32_t GetChannelDepth() const
  {
    return 10;
  };
  virtual std::uint32_t GetChannelCount() const
  {
    return 4;
  };
  virtual bool IsFloat() const
  {
    return false;
  };

  virtual bool ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint16_t block[]);
  virtual bool ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint16_t block[]);
  virtual bool ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint16_t block[]);
  virtual bool ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint16_t block[]);

  virtual bool WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint16_t block[]);
  virtual bool WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint16_t block[]);
  virtual bool WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint16_t block[]);
  virtual bool WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint16_t block[]);

  virtual bool ReadBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                             std::uint8_t h, std::uint32_t block[]);
  virtual bool WriteBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint32_t block[]);

  virtual bool ReadBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                             std::uint8_t h, std::uint16_t block[]);
  virtual bool WriteBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint16_t block[]);

protected:
  virtual bool ReadBlock(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                         std::uint8_t h, std::uint16_t block[],
                         std::uint32_t dwChannelMask,
                         std::uint32_t dwChannelOffset, bool b10Bit);
  virtual bool WriteBlock(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint16_t block[],
                          std::uint32_t dwChannelMask,
                          std::uint32_t dwChannelOffset, bool b10Bit);
};

#endif  // !defined(_CODECBUFFER_RGBA2101010_H_INCLUDED_)