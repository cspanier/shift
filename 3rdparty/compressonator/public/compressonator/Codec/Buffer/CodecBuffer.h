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
//  File Name:   CodecBuffer.h
//  Description: interface for the CCodecBuffer class
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CODECBUFFER_H_INCLUDED_
#define _CODECBUFFER_H_INCLUDED_

#include "compressonator/Internal/debug.h"
#include "compressonator/MathMacros.h"

#pragma warning(push)
#pragma warning(disable : 4244)
#include "OpenEXR/half.h"
#pragma warning(pop)

typedef enum _CodecBufferType
{
  CBT_Unknown = 0,
  CBT_RGBA8888,
  CBT_BGRA8888,  // Reserved for future work
  CBT_ARGB8888,  // Reserved for future work
  CBT_ABGR8888,  // Reserved for future work
  CBT_RGB888,
  CBT_BGR888,  // Reserved for future work
  CBT_RG8,
  CBT_R8,
  CBT_RGBA2101010,
  CBT_RGBA16,
  CBT_RG16,
  CBT_R16,
  CBT_RGBA32,
  CBT_RG32,
  CBT_R32,
  CBT_RGBA16F,
  CBT_RGBE32F,
  CBT_RG16F,
  CBT_R16F,
  CBT_RGBA32F,
  CBT_RG32F,
  CBT_R32F,
  CBT_4x4Block_2BPP,
  CBT_4x4Block_4BPP,
  CBT_4x4Block_8BPP,
  CBT_4x4Block_16BPP,
  CBT_4x4Block_32BPP,
  CBT_8x8Block_2BPP,
  CBT_8x8Block_4BPP,
  CBT_8x8Block_8BPP,
  CBT_8x8Block_16BPP,
  CBT_8x8Block_32BPP,
} CodecBufferType;

#define CHANNEL_SIZE_ARGB 4
#define BLOCK_SIZE_4 4

#define BLOCK_SIZE_4X4 16
#define BLOCK_SIZE_4X4X4 64

#define BLOCK_SIZE_8X8 64
#define BLOCK_SIZE_8X8X4 256

#define RGBA8888_CHANNEL_A 3
#define RGBA8888_CHANNEL_R 2
#define RGBA8888_CHANNEL_G 1
#define RGBA8888_CHANNEL_B 0

#define RGBA8888_OFFSET_A (RGBA8888_CHANNEL_A * 8)
#define RGBA8888_OFFSET_R (RGBA8888_CHANNEL_R * 8)
#define RGBA8888_OFFSET_G (RGBA8888_CHANNEL_G * 8)
#define RGBA8888_OFFSET_B (RGBA8888_CHANNEL_B * 8)

#define RGBA2101010_OFFSET_A 30
#define RGBA2101010_OFFSET_R 20
#define RGBA2101010_OFFSET_G 10
#define RGBA2101010_OFFSET_B 0

#define RGBA16_OFFSET_A 3
#define RGBA16_OFFSET_R 0
#define RGBA16_OFFSET_G 1
#define RGBA16_OFFSET_B 2

#define RGBA16F_OFFSET_A 3
#define RGBA16F_OFFSET_R 0
#define RGBA16F_OFFSET_G 1
#define RGBA16F_OFFSET_B 2

#define RGBA32_OFFSET_A 3
#define RGBA32_OFFSET_R 0
#define RGBA32_OFFSET_G 1
#define RGBA32_OFFSET_B 2

#define RGBA32F_OFFSET_A 3
#define RGBA32F_OFFSET_R 0
#define RGBA32F_OFFSET_G 1
#define RGBA32F_OFFSET_B 2

#define TWO_BIT_MASK 0x0003
#define BYTE_MASK 0x00ff
#define TEN_BIT_MASK 0x03ff
#define WORD_MASK 0xffff

#define MAKE_RGBA8888(r, g, b, a)                        \
  ((r << RGBA8888_OFFSET_R) | (g << RGBA8888_OFFSET_G) | \
   (b << RGBA8888_OFFSET_B) | (a << RGBA8888_OFFSET_A))
#define GET_R(i) ((i >> RGBA8888_OFFSET_R) & BYTE_MASK)
#define GET_G(i) ((i >> RGBA8888_OFFSET_G) & BYTE_MASK)
#define GET_B(i) ((i >> RGBA8888_OFFSET_B) & BYTE_MASK)
#define GET_A(i) ((i >> RGBA8888_OFFSET_A) & BYTE_MASK)

#define SWIZZLE_RGBA_RBxG(i)                                       \
  ((((i >> RGBA8888_OFFSET_R) & BYTE_MASK) << RGBA8888_OFFSET_R) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_A) | \
   (((i >> RGBA8888_OFFSET_B) & BYTE_MASK) << RGBA8888_OFFSET_G))
#define SWIZZLE_RBxG_RGBA(i)                                       \
  ((((i >> RGBA8888_OFFSET_R) & BYTE_MASK) << RGBA8888_OFFSET_R) | \
   (((i >> RGBA8888_OFFSET_A) & BYTE_MASK) << RGBA8888_OFFSET_G) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_B) | \
   (BYTE_MASK << RGBA8888_OFFSET_A))

#define SWIZZLE_RGBA_RGxB(i)                                       \
  ((((i >> RGBA8888_OFFSET_R) & BYTE_MASK) << RGBA8888_OFFSET_R) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_G) | \
   (((i >> RGBA8888_OFFSET_B) & BYTE_MASK) << RGBA8888_OFFSET_A))
#define SWIZZLE_RGxB_RGBA(i)                                       \
  ((((i >> RGBA8888_OFFSET_R) & BYTE_MASK) << RGBA8888_OFFSET_R) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_G) | \
   (((i >> RGBA8888_OFFSET_A) & BYTE_MASK) << RGBA8888_OFFSET_B) | \
   (BYTE_MASK << RGBA8888_OFFSET_A))

#define SWIZZLE_RGBA_RxBG(i)                                       \
  ((((i >> RGBA8888_OFFSET_R) & BYTE_MASK) << RGBA8888_OFFSET_R) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_A) | \
   (((i >> RGBA8888_OFFSET_B) & BYTE_MASK) << RGBA8888_OFFSET_B))
#define SWIZZLE_RxBG_RGBA(i)                                       \
  ((((i >> RGBA8888_OFFSET_R) & BYTE_MASK) << RGBA8888_OFFSET_R) | \
   (((i >> RGBA8888_OFFSET_A) & BYTE_MASK) << RGBA8888_OFFSET_G) | \
   (((i >> RGBA8888_OFFSET_B) & BYTE_MASK) << RGBA8888_OFFSET_B) | \
   (BYTE_MASK << RGBA8888_OFFSET_A))

#define SWIZZLE_RGBA_xGBR(i)                                       \
  ((((i >> RGBA8888_OFFSET_R) & BYTE_MASK) << RGBA8888_OFFSET_A) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_G) | \
   (((i >> RGBA8888_OFFSET_B) & BYTE_MASK) << RGBA8888_OFFSET_B))
#define SWIZZLE_xGBR_RGBA(i)                                       \
  ((((i >> RGBA8888_OFFSET_A) & BYTE_MASK) << RGBA8888_OFFSET_R) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_G) | \
   (((i >> RGBA8888_OFFSET_B) & BYTE_MASK) << RGBA8888_OFFSET_B) | \
   (BYTE_MASK << RGBA8888_OFFSET_A))

#define SWIZZLE_RGBA_xRBG(i)                                       \
  ((((i >> RGBA8888_OFFSET_R) & BYTE_MASK) << RGBA8888_OFFSET_G) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_A) | \
   (((i >> RGBA8888_OFFSET_B) & BYTE_MASK) << RGBA8888_OFFSET_B))
#define SWIZZLE_xRBG_RGBA(i)                                       \
  ((((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_R) | \
   (((i >> RGBA8888_OFFSET_A) & BYTE_MASK) << RGBA8888_OFFSET_G) | \
   (((i >> RGBA8888_OFFSET_B) & BYTE_MASK) << RGBA8888_OFFSET_B) | \
   (BYTE_MASK << RGBA8888_OFFSET_A))

#define SWIZZLE_RGBA_xGxR(i)                                       \
  ((((i >> RGBA8888_OFFSET_R) & BYTE_MASK) << RGBA8888_OFFSET_A) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_G))
#define SWIZZLE_xGxR_RGBA(i)                                       \
  ((((i >> RGBA8888_OFFSET_A) & BYTE_MASK) << RGBA8888_OFFSET_R) | \
   (((i >> RGBA8888_OFFSET_G) & BYTE_MASK) << RGBA8888_OFFSET_G) | \
   (BYTE_MASK << RGBA8888_OFFSET_A))

#define BYTE_MAXVAL 255
#define BYTE_MAX_FLOAT 255.0f
#define CONVERT_FLOAT_TO_BYTE(f) \
  static_cast<std::uint8_t>(((f)*BYTE_MAX_FLOAT) + 0.5)
#define CONVERT_BYTE_TO_FLOAT(b) (b) / BYTE_MAX_FLOAT

#define DWORD_MAXVAL 4294967295.0f
#define WORD_MAXVAL 65535.0f
#define CONVERT_FLOAT_TO_DWORD(f) \
  static_cast<std::uint32_t>(((f)*DWORD_MAXVAL) + 0.5)
#define CONVERT_FLOAT_TO_WORD(f) \
  static_cast<std::uint16_t>(((f)*WORD_MAXVAL) + 0.5)
#define CONVERT_DWORD_TO_FLOAT(w) (w) / DWORD_MAXVAL
#define CONVERT_WORD_TO_FLOAT(w) (w) / WORD_MAXVAL
#define CONVERT_WORD_TO_DWORD(w) \
  (((static_cast<std::uint32_t>(w)) << 16) | static_cast<std::uint32_t>(w))
#define CONVERT_DWORD_TO_WORD(dw) \
  static_cast<std::uint8_t>(min(  \
    ((dw >> 16) + ((dw & 0x0000ffff) >= 0x00008000 ? 1 : 0)), WORD_MAXVAL))
#define CONVERT_BYTE_TO_DWORD(b)             \
  (((static_cast<std::uint32_t>(b)) << 24) | \
   ((static_cast<std::uint32_t>(b)) << 16) | \
   ((static_cast<std::uint32_t>(b)) << 8) | static_cast<std::uint32_t>(b))
#define CONVERT_DWORD_TO_BYTE(dw) \
  static_cast<std::uint8_t>(min(  \
    ((dw >> 24) + ((dw & 0x00ffffff) >= 0x00800000 ? 1 : 0)), BYTE_MAXVAL))
#define CONVERT_BYTE_TO_WORD(b) \
  (((static_cast<std::uint16_t>(b)) << 8) | static_cast<std::uint16_t>(b))
#define CONVERT_WORD_TO_BYTE(w) \
  static_cast<std::uint8_t>(    \
    min(((w >> 8) + ((w & BYTE_MASK) >= 128 ? 1 : 0)), BYTE_MAXVAL))
#define CONVERT_10BIT_TO_WORD(b) \
  (((static_cast<std::uint16_t>(b)) << 6) | static_cast<std::uint16_t>(b) >> 2)
#define CONVERT_2BIT_TO_WORD(b)                                               \
  ((static_cast<std::uint16_t>(b)) | ((static_cast<std::uint16_t>(b)) << 2) | \
   ((static_cast<std::uint16_t>(b)) << 4) |                                   \
   ((static_cast<std::uint16_t>(b)) << 6) |                                   \
   ((static_cast<std::uint16_t>(b)) << 8) |                                   \
   ((static_cast<std::uint16_t>(b)) << 10) |                                  \
   ((static_cast<std::uint16_t>(b)) << 12) |                                  \
   ((static_cast<std::uint16_t>(b)) << 14))
#define CONVERT_WORD_TO_10BIT(b) ((b >> 6) & TEN_BIT_MASK)
#define CONVERT_WORD_TO_2BIT(b) ((b >> 14) & TWO_BIT_MASK)

#define SWAP_DWORDS(a, b)     \
  {                           \
    std::uint32_t dwTemp = a; \
    a = b;                    \
    b = dwTemp;               \
  }
#define SWAP_WORDS(a, b)     \
  {                          \
    std::uint16_t wTemp = a; \
    a = b;                   \
    b = wTemp;               \
  }
#define SWAP_HALFS(a, b) \
  {                      \
    half fTemp = a;      \
    a = b;               \
    b = fTemp;           \
  }
#define SWAP_FLOATS(a, b) \
  {                       \
    float fTemp = a;      \
    a = b;                \
    b = fTemp;            \
  }
#define SWAP_DOUBLES(a, b) \
  {                        \
    double dTemp = a;      \
    a = b;                 \
    b = dTemp;             \
  }

template <typename T>
void PadLine(std::uint32_t i, std::uint8_t w, std::uint8_t c, T block[])
{
  // So who do we perform generic padding ?
  // In powers of two

  std::uint32_t dwPadWidth = w - i;
  if (dwPadWidth > i)
  {
    PadLine(i, w >> 1, c, block);
    i = w >> 1;
    dwPadWidth = w - i;
  }

  memcpy(&block[i * c], &block[0], dwPadWidth * c * sizeof(T));
}

template <typename T>
void PadBlock(std::uint32_t j, std::uint8_t w, std::uint8_t h, std::uint8_t c,
              T block[])
{
  // So who do we perform generic padding ?
  // In powers of two

  std::uint32_t dwPadHeight = h - j;
  if (dwPadHeight > j)
  {
    PadBlock(j, w, h >> 1, c, block);
    j = h >> 1;
    dwPadHeight = h - j;
  }
  memcpy(&block[j * w * c], &block[0], dwPadHeight * w * c * sizeof(T));
}

class CCodecBuffer
{
public:
  CCodecBuffer(std::uint8_t nBlockWidth, std::uint8_t nBlockHeight,
               std::uint8_t nBlockDepth, std::uint32_t dwWidth,
               std::uint32_t dwHeight, std::uint32_t dwPitch = 0,
               std::uint8_t* pData = nullptr);
  virtual ~CCodecBuffer();

  virtual void Copy(CCodecBuffer& srcBuffer);

  virtual CodecBufferType GetBufferType() const
  {
    return CBT_Unknown;
  };
  virtual std::uint32_t GetChannelDepth() const = 0;
  virtual std::uint32_t GetChannelCount() const = 0;
  virtual bool IsFloat() const = 0;

  inline const std::uint32_t GetWidth() const
  {
    return m_dwWidth;
  };
  inline const std::uint32_t GetHeight() const
  {
    return m_dwHeight;
  };
  inline const std::uint32_t GetPitch() const
  {
    return m_dwPitch;
  };

  inline const void SetPitch(std::uint32_t dwPitch)
  {
    m_dwPitch = dwPitch;
  };

  inline const void SetFormat(cmp_format dwFormat)
  {
    m_dwFormat = dwFormat;
  };

  inline const cmp_format GetFormat() const
  {
    return m_dwFormat;
  };

  inline const std::uint8_t GetBlockWidth() const
  {
    return m_nBlockWidth;
  };
  inline const std::uint8_t GetBlockHeight() const
  {
    return m_nBlockHeight;
  };
  inline const std::uint8_t GetBlockDepth() const
  {
    return m_nBlockDepth;
  };

  inline const void SetBlockWidth(std::uint8_t BlockWidth)
  {
    m_nBlockWidth = BlockWidth;
  };
  inline const void SetBlockHeight(std::uint8_t BlockHeight)
  {
    m_nBlockHeight = BlockHeight;
  };
  inline const void SetBlockDepth(std::uint8_t BlockDepth)
  {
    m_nBlockDepth = BlockDepth;
  };

  virtual bool ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint8_t cBlock[]);
  virtual bool ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint8_t cBlock[]);
  virtual bool ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint8_t cBlock[]);
  virtual bool ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint8_t cBlock[]);

  virtual bool ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint16_t wblock[]);
  virtual bool ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint16_t wblock[]);
  virtual bool ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint16_t wblock[]);
  virtual bool ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint16_t wblock[]);

  virtual bool ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint32_t dwblock[]);
  virtual bool ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint32_t dwblock[]);
  virtual bool ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint32_t dwblock[]);
  virtual bool ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, std::uint32_t dwblock[]);

  virtual bool ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, float fBlock[]);
  virtual bool ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, float fBlock[]);
  virtual bool ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, float fBlock[]);
  virtual bool ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, float fBlock[]);

  virtual bool ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, half hBlock[]);
  virtual bool ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, half hBlock[]);
  virtual bool ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, half hBlock[]);
  virtual bool ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, half hBlock[]);

  virtual bool ReadBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, double dBlock[]);
  virtual bool ReadBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, double dBlock[]);
  virtual bool ReadBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, double dBlock[]);
  virtual bool ReadBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                          std::uint8_t h, double dBlock[]);

  virtual bool WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint8_t cBlock[]);
  virtual bool WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint8_t cBlock[]);
  virtual bool WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint8_t cBlock[]);
  virtual bool WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint8_t cBlock[]);

  virtual bool WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint16_t wblock[]);
  virtual bool WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint16_t wblock[]);
  virtual bool WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint16_t wblock[]);
  virtual bool WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint16_t wblock[]);

  virtual bool WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint32_t dwblock[]);
  virtual bool WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint32_t dwblock[]);
  virtual bool WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint32_t dwblock[]);
  virtual bool WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, std::uint32_t dwblock[]);

  virtual bool WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, half hBlock[]);
  virtual bool WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, half hBlock[]);
  virtual bool WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, half hBlock[]);
  virtual bool WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, half hBlock[]);

  virtual bool WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, float fBlock[]);
  virtual bool WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, float fBlock[]);
  virtual bool WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, float fBlock[]);
  virtual bool WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, float fBlock[]);

  virtual bool WriteBlockR(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, double dBlock[]);
  virtual bool WriteBlockG(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, double dBlock[]);
  virtual bool WriteBlockB(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, double dBlock[]);
  virtual bool WriteBlockA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                           std::uint8_t h, double dBlock[]);

  virtual bool ReadBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                             std::uint8_t h, std::uint8_t cBlock[]);
  virtual bool WriteBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint8_t cBlock[]);

  virtual bool ReadBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                             std::uint8_t h, std::uint32_t dwBlock[]);
  virtual bool WriteBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint32_t dwBlock[]);

  virtual bool ReadBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                             std::uint8_t h, std::uint16_t wBlock[]);
  virtual bool WriteBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, std::uint16_t wBlock[]);

  virtual bool ReadBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                             std::uint8_t h, half hBlock[]);
  virtual bool WriteBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, half hBlock[]);

  virtual bool ReadBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                             std::uint8_t h, float fBlock[]);
  virtual bool WriteBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, float fBlock[]);

  virtual bool ReadBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                             std::uint8_t h, double dBlock[]);
  virtual bool WriteBlockRGBA(std::uint32_t x, std::uint32_t y, std::uint8_t w,
                              std::uint8_t h, double dBlock[]);

  virtual bool ReadBlock(std::uint32_t x, std::uint32_t y,
                         std::uint32_t* pBlock, std::uint32_t dwBlockSize);
  virtual bool WriteBlock(std::uint32_t x, std::uint32_t y,
                          std::uint32_t* pBlock, std::uint32_t dwBlockSize);

  inline std::uint8_t* GetData() const
  {
    return m_pData;
  };

protected:
  void ConvertBlock(double dBlock[], const float fBlock[], std::uint32_t dwBlockSize);
  void ConvertBlock(double dBlock[], half hBlock[], std::uint32_t dwBlockSize);
  void ConvertBlock(double dBlock[], const std::uint32_t dwBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(double dBlock[], const std::uint16_t wBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(double dBlock[], const std::uint8_t cBlock[],
                    std::uint32_t dwBlockSize);

  void ConvertBlock(float fBlock[], const double dBlock[], std::uint32_t dwBlockSize);
  void ConvertBlock(float fBlock[], half hBlock[], std::uint32_t dwBlockSize);
  void ConvertBlock(float fBlock[], const std::uint32_t dwBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(float fBlock[], const std::uint16_t wBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(float fBlock[], const std::uint8_t cBlock[],
                    std::uint32_t dwBlockSize);

  void ConvertBlock(half hBlock[], double dBlock[], std::uint32_t dwBlockSize);
  void ConvertBlock(half hBlock[], float fBlock[], std::uint32_t dwBlockSize);
  void ConvertBlock(half hBlock[], const std::uint32_t dwBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(half hBlock[], const std::uint16_t wBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(half hBlock[], const std::uint8_t cBlock[],
                    std::uint32_t dwBlockSize);

  void ConvertBlock(std::uint32_t dwBlock[], const double dBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint32_t dwBlock[], const float fBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint32_t dwBlock[], half hBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint32_t dwBlock[], const std::uint16_t wBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint32_t dwBlock[], const std::uint8_t cBlock[],
                    std::uint32_t dwBlockSize);

  void ConvertBlock(std::uint16_t wBlock[], const double dBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint16_t wBlock[], const float fBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint16_t wBlock[], half hBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint16_t wBlock[], const std::uint32_t dwBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint16_t wBlock[], const std::uint8_t cBlock[],
                    std::uint32_t dwBlockSize);

  void ConvertBlock(std::uint8_t cBlock[], const double dBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint8_t cBlock[], const float fBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint8_t cBlock[], half hBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint8_t cBlock[], const std::uint32_t dwBlock[],
                    std::uint32_t dwBlockSize);
  void ConvertBlock(std::uint8_t cBlock[], const std::uint16_t wBlock[],
                    std::uint32_t dwBlockSize);

  void SwizzleBlock(double dBlock[], std::uint32_t dwBlockSize);
  void SwizzleBlock(float fBlock[], std::uint32_t dwBlockSize);
  void SwizzleBlock(half hBlock[], std::uint32_t dwBlockSize);
  void SwizzleBlock(std::uint32_t dwBlock[], std::uint32_t dwBlockSize);
  void SwizzleBlock(std::uint16_t wBlock[], std::uint32_t dwBlockSize);

  std::uint32_t m_dwWidth;   // Final Image Width
  std::uint32_t m_dwHeight;  // Final Image Height
  std::uint32_t m_dwDepth{};   // Final Image Depth
  std::uint32_t m_dwPitch;
  cmp_format m_dwFormat;

  std::uint8_t m_nBlockWidth;   // DeCompression Block Sizes (Default is 4x4x1)
  std::uint8_t m_nBlockHeight;  //
  std::uint8_t m_nBlockDepth;   //

  bool m_bUserAllocedData;
  std::uint8_t* m_pData;

  bool m_bPerformingConversion;
};

CCodecBuffer* CreateCodecBuffer(CodecBufferType nCodecBufferType,
                                std::uint8_t nBlockWidth,
                                std::uint8_t nBlockHeight,
                                std::uint8_t nBlockDepth, std::uint32_t dwWidth,
                                std::uint32_t dwHeight,
                                std::uint32_t dwPitch = 0,
                                std::uint8_t* pData = nullptr);
CodecBufferType GetCodecBufferType(cmp_format format);

#endif  // !defined(_CODECBUFFER_H_INCLUDED_)
