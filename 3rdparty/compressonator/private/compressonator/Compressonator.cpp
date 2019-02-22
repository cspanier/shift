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
//  File Name:   Compressonator.cpp
//  Description: A library to compress/decompress textures
//
//  Revisions
//  Feb 2016    -   Fix Parameter processing & Swizzle issue for DXTC Codecs
//  Jan 2016    -   Added ASTC support -
//  Jan 2014    -   Completed support for BC6H and Command line options for new
//  compressonator Apr 2014    -   Refactored Library
//                  Code clean to support MSV 2010 and up
//////////////////////////////////////////////////////////////////////////////

#include "compressonator/Compressonator.h"  // User shared: Keep priviate code out of this header
#include "compressonator/Compress.h"
#include <cassert>
#include "compressonator/Internal/debug.h"

extern CodecType GetCodecType(cmp_format format);
extern CMP_ERROR GetError(CodecError err);

extern bool IsFloatFormat(cmp_format InFormat);
extern CMP_ERROR Byte2Float(short* hfBlock, const std::uint8_t* cBlock,
                            std::uint32_t dwBlockSize);
extern CMP_ERROR Float2Byte(std::uint8_t cBlock[], float* fBlock,
                            CMP_Texture* srcTexture, cmp_format destFormat,
                            const CMP_CompressOptions* pOptions);

extern CMP_ERROR CheckTexture(const CMP_Texture* pTexture, bool bSource);
extern CMP_ERROR CompressTexture(const CMP_Texture* pSourceTexture,
                                 CMP_Texture* pDestTexture,
                                 const CMP_CompressOptions* pOptions,
                                 CMP_Feedback_Proc pFeedbackProc,
                                 std::size_t pUser1, std::size_t pUser2,
                                 CodecType destType);
extern CMP_ERROR ThreadedCompressTexture(const CMP_Texture* pSourceTexture,
                                         CMP_Texture* pDestTexture,
                                         const CMP_CompressOptions* pOptions,
                                         CMP_Feedback_Proc pFeedbackProc,
                                         std::size_t pUser1, std::size_t pUser2,
                                         CodecType destType);

#ifdef _LOCAL_DEBUG
char DbgTracer::buff[MAX_DBGBUFF_SIZE];
char DbgTracer::PrintBuff[MAX_DBGPPRINTBUFF_SIZE];
#endif

std::uint32_t CMP_CalculateBufferSize(const CMP_Texture* pTexture)
{
  assert(pTexture);
  if (pTexture == nullptr)
    return 0;

  assert(pTexture->width > 0);
  if (pTexture->width <= 0)
    return 0;

  assert(pTexture->height > 0);
  if (pTexture->height <= 0)
    return 0;

  return CalcBufferSize(pTexture->format, pTexture->width, pTexture->height,
                        pTexture->pitch, pTexture->block_width,
                        pTexture->block_height);
}

std::uint32_t CalcBufferSize(cmp_format format, std::uint32_t width,
                             std::uint32_t height, std::uint32_t pitch,
                             std::uint8_t block_width,
                             std::uint8_t block_height)
{
  switch (format)
  {
  case cmp_format::RGBA_8888:
  case cmp_format::BGRA_8888:
  case cmp_format::ARGB_8888:
  case cmp_format::ARGB_2101010:
    return ((pitch) ? (pitch * height) : (width * 4 * height));

  case cmp_format::BGR_888:
  case cmp_format::RGB_888:
    return ((pitch) ? (pitch * height)
                    : ((((width * 3) + 3) >> 2) * 4 * height));

  case cmp_format::RG_8:
    return ((pitch) ? (pitch * height) : (width * 2 * height));

  case cmp_format::R_8:
    return ((pitch) ? (pitch * height) : (width * height));

  case cmp_format::ARGB_16:
  case cmp_format::ARGB_16F:
    return ((pitch) ? (pitch * height)
                    : (width * 4 * sizeof(std::uint16_t) * height));

  case cmp_format::RG_16:
  case cmp_format::RG_16F:
    return ((pitch) ? (pitch * height)
                    : (width * 4 * sizeof(std::uint16_t) * height));

  case cmp_format::R_16:
  case cmp_format::R_16F:
    return ((pitch) ? (pitch * height)
                    : (width * 4 * sizeof(std::uint16_t) * height));

#ifdef ARGB_32_SUPPORT
  case cmp_format::ARGB_32:
#endif  // ARGB_32_SUPPORT
  case cmp_format::ARGB_32F:
    return ((pitch) ? (pitch * height) : (width * 4 * sizeof(float) * height));

#ifdef ARGB_32_SUPPORT
  case cmp_format::RG_32:
#endif  // ARGB_32_SUPPORT
  case cmp_format::RG_32F:
    return ((pitch) ? (pitch * height) : (width * 2 * sizeof(float) * height));

#ifdef ARGB_32_SUPPORT
  case cmp_format::R_32:
#endif  // ARGB_32_SUPPORT
  case cmp_format::R_32F:
    return ((pitch) ? (pitch * height) : (width * 1 * sizeof(float) * height));

  default:
    return CalcBufferSize(GetCodecType(format), width, height, block_width,
                          block_height);
  }
}

CMP_ERROR CMP_ConvertTexture(CMP_Texture* pSourceTexture,
                             CMP_Texture* pDestTexture,
                             const CMP_CompressOptions* pOptions,
                             CMP_Feedback_Proc pFeedbackProc,
                             std::size_t pUser1, std::size_t pUser2)
{
  CMP_ERROR tc_err = CheckTexture(pSourceTexture, true);
  if (tc_err != CMP_OK)
    return tc_err;

  bool srcFloat = IsFloatFormat(pSourceTexture->format);
  bool destFloat = IsFloatFormat(pDestTexture->format);

  bool newBuffer = false;
  if (srcFloat && !destFloat)
  {
    std::uint32_t size = pSourceTexture->width * pSourceTexture->height;
    auto* pfData = new float[pSourceTexture->data_size];

    memcpy(pfData, pSourceTexture->data, pSourceTexture->data_size);

    auto* byteData = new std::uint8_t[size * 4];

    Float2Byte(byteData, pfData, pSourceTexture, pDestTexture->format,
               pOptions);

    delete[] pfData;
    pSourceTexture->data = byteData;

    pSourceTexture->format = cmp_format::ARGB_8888;
    pSourceTexture->data_size = size * 4;
    newBuffer = true;
  }

  else if (!srcFloat && destFloat)
  {
    std::uint32_t size = pSourceTexture->width * pSourceTexture->height;
    std::uint8_t* pbData = pSourceTexture->data;
    auto* hfloatData = new short[size * 4];
    Byte2Float(hfloatData, pbData, size * 4);
    pSourceTexture->data = (std::uint8_t*)hfloatData;
    pSourceTexture->format = cmp_format::ARGB_16F;
    pSourceTexture->data_size = size * 4 * 2;
    newBuffer = true;
  }

  tc_err = CheckTexture(pDestTexture, false);
  if (tc_err != CMP_OK)
    return tc_err;

  if (pSourceTexture->width != pDestTexture->width ||
      pSourceTexture->height != pDestTexture->height)
    return CMP_ERR_SIZE_MISMATCH;

  CodecType srcType = GetCodecType(pSourceTexture->format);
  assert(srcType != CT_Unknown);
  if (srcType == CT_Unknown)
    return CMP_ERR_UNSUPPORTED_SOURCE_FORMAT;

  CodecType destType = GetCodecType(pDestTexture->format);
  assert(destType != CT_Unknown);
  if (destType == CT_Unknown)
    return CMP_ERR_UNSUPPORTED_SOURCE_FORMAT;

  if (srcType == destType)
  {
    // Easy case ?
    if (pSourceTexture->format == pDestTexture->format &&
        pSourceTexture->pitch == pDestTexture->pitch)
      memcpy(pDestTexture->data, pSourceTexture->data,
             CMP_CalculateBufferSize(pSourceTexture));
    else
    {
      CodecBufferType srcBufferType =
        GetCodecBufferType(pSourceTexture->format);
      CodecBufferType destBufferType = GetCodecBufferType(pDestTexture->format);

      CCodecBuffer* pSrcBuffer = CreateCodecBuffer(
        srcBufferType, pSourceTexture->block_width,
        pSourceTexture->block_height, pSourceTexture->block_depth,
        pSourceTexture->width, pSourceTexture->height, pSourceTexture->pitch,
        pSourceTexture->data);
      assert(pSrcBuffer);
      if (!pSrcBuffer)
        return CMP_ERR_GENERIC;

      CCodecBuffer* pDestBuffer = CreateCodecBuffer(
        destBufferType, pDestTexture->block_width, pDestTexture->block_height,
        pDestTexture->block_depth, pDestTexture->width, pDestTexture->height,
        pDestTexture->pitch, pDestTexture->data);
      assert(pDestBuffer);
      if (!pDestBuffer)
      {
        delete pSrcBuffer;
        return CMP_ERR_GENERIC;
      }

      DISABLE_FP_EXCEPTIONS;
      pDestBuffer->Copy(*pSrcBuffer);
      RESTORE_FP_EXCEPTIONS;

      delete pSrcBuffer;
      delete pDestBuffer;
    }

    return CMP_OK;
  }
  if (srcType == CT_None && destType != CT_None)
  {

#ifdef THREADED_COMPRESS
    // Note:
    // BC7/BC6H has issues with this setting - we already set multithreading via
    // numThreads so this call is disabled for BC7/BC6H ASTC Codecs. if the use
    // has set DiableMultiThreading then numThreads will be set to 1 (regradless
    // of its original value)
    if (((!pOptions || !pOptions->bDisableMultiThreading) &&
         std::thread::hardware_concurrency() > 1) &&
        (destType != CT_BC7) && (destType != CT_BC6H) &&
        (destType != CT_BC6H_SF)
#ifdef USE_GTC_HDR
        && (destType != CT_GTCH)
#endif
    )
    {
      tc_err = ThreadedCompressTexture(pSourceTexture, pDestTexture, pOptions,
                                       pFeedbackProc, pUser1, pUser2, destType);
      if (pSourceTexture->data && newBuffer)
      {
        free(pSourceTexture->data);
        pSourceTexture->data = nullptr;
      }
      return tc_err;
    }

#endif  // THREADED_COMPRESS

    tc_err = CompressTexture(pSourceTexture, pDestTexture, pOptions,
                             pFeedbackProc, pUser1, pUser2, destType);
    if (pSourceTexture->data && newBuffer)
    {
      free(pSourceTexture->data);
      pSourceTexture->data = nullptr;
    }
    return tc_err;
  }
  else if (srcType != CT_None && destType == CT_None)
  {
    // Decompressing

    auto* pCodec = CreateCodec(srcType);
    assert(pCodec);
    if (pCodec == nullptr)
    {
      if (pSourceTexture->data && newBuffer)
      {
        free(pSourceTexture->data);
        pSourceTexture->data = nullptr;
      }
      return CMP_ERR_UNABLE_TO_INIT_CODEC;
    }

    CodecBufferType destBufferType = GetCodecBufferType(pDestTexture->format);

    CCodecBuffer* pSrcBuffer = pCodec->CreateBuffer(
      pSourceTexture->block_width, pSourceTexture->block_height,
      pSourceTexture->block_depth, pSourceTexture->width,
      pSourceTexture->height, pSourceTexture->pitch, pSourceTexture->data);

    pDestTexture->block_width = pSourceTexture->block_width;
    pDestTexture->block_height = pSourceTexture->block_height;
    pDestTexture->block_depth = pSourceTexture->block_depth;

    CCodecBuffer* pDestBuffer = CreateCodecBuffer(
      destBufferType, pDestTexture->block_width, pDestTexture->block_height,
      pDestTexture->block_depth, pDestTexture->width, pDestTexture->height,
      pDestTexture->pitch, pDestTexture->data);

    // assert(pDestBuffer);
    // assert(pSrcBuffer);

    if (pSrcBuffer == nullptr || pDestBuffer == nullptr)
    {
      SAFE_DELETE(pCodec);
      SAFE_DELETE(pSrcBuffer);
      SAFE_DELETE(pDestBuffer);
      if (pSourceTexture->data && newBuffer)
      {
        free(pSourceTexture->data);
        pSourceTexture->data = nullptr;
      }
      return CMP_ERR_GENERIC;
    }

    DISABLE_FP_EXCEPTIONS;

    pSrcBuffer->SetBlockHeight(pSourceTexture->block_height);
    pSrcBuffer->SetBlockWidth(pSourceTexture->block_width);
    pSrcBuffer->SetBlockDepth(pSourceTexture->block_depth);
    pSrcBuffer->SetFormat(pSourceTexture->format);

    CodecError err1 = pCodec->Decompress(*pSrcBuffer, *pDestBuffer,
                                         pFeedbackProc, pUser1, pUser2);
    RESTORE_FP_EXCEPTIONS;

    SAFE_DELETE(pCodec);
    SAFE_DELETE(pSrcBuffer);
    SAFE_DELETE(pDestBuffer);

    if (pSourceTexture->data && newBuffer)
    {
      free(pSourceTexture->data);
      pSourceTexture->data = nullptr;
    }
    return GetError(err1);
  }
  else  // Decompressing & then compressing
  {
    // Decompressing
    auto* pCodecIn = CreateCodec(srcType);
    auto* pCodecOut = CreateCodec(destType);
    assert(pCodecIn);
    assert(pCodecOut);

    if (pCodecIn == nullptr || pCodecOut == nullptr)
    {
      SAFE_DELETE(pCodecIn);
      SAFE_DELETE(pCodecOut);
      if (pSourceTexture->data && newBuffer)
      {
        free(pSourceTexture->data);
        pSourceTexture->data = nullptr;
      }
      return CMP_ERR_UNABLE_TO_INIT_CODEC;
    }

    CCodecBuffer* pSrcBuffer = pCodecIn->CreateBuffer(
      pSourceTexture->block_width, pSourceTexture->block_height,
      pSourceTexture->block_depth, pSourceTexture->width,
      pSourceTexture->height, pSourceTexture->pitch, pSourceTexture->data);
    CCodecBuffer* pTempBuffer = CreateCodecBuffer(
      CBT_RGBA32F, pDestTexture->block_width, pDestTexture->block_height,
      pDestTexture->block_depth, pDestTexture->width, pDestTexture->height);
    CCodecBuffer* pDestBuffer = pCodecOut->CreateBuffer(
      pDestTexture->block_width, pDestTexture->block_height,
      pDestTexture->block_depth, pDestTexture->width, pDestTexture->height,
      pDestTexture->pitch, pDestTexture->data);

    assert(pSrcBuffer);
    assert(pTempBuffer);
    assert(pDestBuffer);
    if (pSrcBuffer == nullptr || pTempBuffer == nullptr ||
        pDestBuffer == nullptr)
    {
      SAFE_DELETE(pCodecIn);
      SAFE_DELETE(pCodecOut);
      SAFE_DELETE(pSrcBuffer);
      SAFE_DELETE(pTempBuffer);
      SAFE_DELETE(pDestBuffer);
      if (pSourceTexture->data && newBuffer)
      {
        free(pSourceTexture->data);
        pSourceTexture->data = nullptr;
      }
      return CMP_ERR_GENERIC;
    }

    DISABLE_FP_EXCEPTIONS;
    CodecError err2 = pCodecIn->Decompress(*pSrcBuffer, *pTempBuffer,
                                           pFeedbackProc, pUser1, pUser2);
    if (err2 == CE_OK)
    {
      err2 = pCodecOut->Compress(*pTempBuffer, *pDestBuffer, pFeedbackProc,
                                 pUser1, pUser2);
    }
    RESTORE_FP_EXCEPTIONS;

    if (pSourceTexture->data && newBuffer)
    {
      free(pSourceTexture->data);
      pSourceTexture->data = nullptr;
    }
    return GetError(err2);
  }
}
