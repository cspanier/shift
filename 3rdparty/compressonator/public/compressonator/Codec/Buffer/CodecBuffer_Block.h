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
//  File Name:   CodecBuffer_Block.h
//  Description: interface for the CCodecBuffer_Block class
//
//////////////////////////////////////////////////////////////////////////////

#ifndef COMPRESSONATOR_CODECBUFFER_BLOCK_H
#define COMPRESSONATOR_CODECBUFFER_BLOCK_H

#include "compressonator/Codec/Buffer/CodecBuffer.h"

class CCodecBuffer_Block : public CCodecBuffer
{
public:
  CCodecBuffer_Block(CodecBufferType nCodecBufferType, std::uint8_t nBlockWidth,
                     std::uint8_t nBlockHeight, std::uint8_t nBlockDepth,
                     std::uint32_t dwWidth, std::uint32_t dwHeight,
                     std::uint32_t dwPitch = 0, std::uint8_t* pData = nullptr);

  virtual ~CCodecBuffer_Block();

  virtual CodecBufferType GetBufferType() const
  {
    return m_nCodecBufferType;
  };
  virtual std::uint32_t GetChannelDepth() const
  {
    return 0;
  };
  virtual std::uint32_t GetChannelCount() const
  {
    return 0;
  };
  virtual bool IsFloat() const
  {
    return false;
  };

  virtual bool ReadBlock(std::uint32_t x, std::uint32_t y,
                         std::uint32_t* pBlock, std::uint32_t dwBlockSize);
  virtual bool WriteBlock(std::uint32_t x, std::uint32_t y,
                          std::uint32_t* pBlock, std::uint32_t dwBlockSize);

protected:
  CodecBufferType m_nCodecBufferType;
  std::uint32_t m_dwBlockSize;
  std::uint32_t m_dwBlockWidth;
  std::uint32_t m_dwBlockHeight;
  std::uint32_t m_dwBlockBPP;
};

#endif
