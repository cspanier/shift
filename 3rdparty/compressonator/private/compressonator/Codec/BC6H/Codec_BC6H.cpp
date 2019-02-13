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
//
//  File Name:   Codec_BC6H.cpp
//  Description: implementation of the CCodec_BC6H class
//
//////////////////////////////////////////////////////////////////////////////

#pragma warning( \
  disable : 4100)  // Ignore warnings of unreferenced formal parameters
#include "compressonator/Common.h"
#include "compressonator/Codec/BC6H/Codec_BC6H.h"
#include "compressonator/Codec/BC7/BC7_Definitions.h"
#include "compressonator/Codec/BC6H/BC6H_Library.h"
#include "compressonator/Codec/BC6H/BC6H_Definitions.h"
#include "compressonator/Common/HDR_Encode.h"

#include <chrono>

using namespace HDR_Encode;

#ifdef BC6H_COMPDEBUGGER
#include "CompClient.h"
extern CompViewerClient g_CompClient;
#endif

//======================================================================================
#define USE_MULTITHREADING 1

//
// Thread procedure for encoding a block
//
// The thread stays alive, and expects blocks to be pushed to it by a producer
// process that signals it when new work is available. When the producer is
// finished it should set the exit flag in the parameters to allow the tread to
// quit
//

unsigned int BC6HThreadProcEncode(void* param)
{
  BC6HEncodeThreadParam* tp = (BC6HEncodeThreadParam*)param;

  while (tp->exit == FALSE)
  {
    if (tp->run == TRUE)
    {
      tp->encoder->CompressBlock(tp->in, tp->out);
      tp->run = FALSE;
    }

    using namespace chrono;

    std::this_thread::sleep_for(0ms);
  }

  return 0;
}

int g_block = 0;  // Keep track of current encoder block!

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

CCodec_BC6H::CCodec_BC6H(CodecType codecType) : CCodec_DXTC(codecType)
{
  // user definable setting
  m_Exposure = 1.0;
  m_ModeMask = 0xFFFF;
  m_Quality = (float)AMD_CODEC_QUALITY_DEFAULT;
  m_Use_MultiThreading = true;
  m_NumThreads = 8;
  if (codecType == CT_BC6H)
    m_bIsSigned = false;
  else
    m_bIsSigned = true;
  m_UsePatternRec = false;

  // Internal setting
  m_LibraryInitialized = false;
  m_NumEncodingThreads = 1;
  m_EncodingThreadHandle = nullptr;
  m_LiveThreads = 0;
  m_LastThread = 0;
  m_CodecType = codecType;
}

bool CCodec_BC6H::SetParameter(const char* pszParamName, char* sValue)
{
  if (sValue == nullptr)
    return false;

  if (strcmp(pszParamName, "ModeMask") == 0)
  {
    m_ModeMask = std::stoi(sValue) & 0xFFFF;
  }
  else if (strcmp(pszParamName, "PatternRec") == 0)
  {
    m_UsePatternRec = (bool)(std::stoi(sValue) > 0);
  }
  else if (strcmp(pszParamName, "NumThreads") == 0)
  {
    m_NumThreads = (std::uint8_t)std::stoi(sValue);
    m_Use_MultiThreading = m_NumThreads > 1;
  }
  else if (strcmp(pszParamName, "Quality") == 0)
  {
    m_Quality = std::stof(sValue);
    if ((m_Quality < 0) || (m_Quality > 1.0))
    {
      return false;
    }
  }
  // else
  //    if (strcmp(pszParamName, "OutputExposure") == 0)
  //    {
  //        m_Exposure = std::stof(sValue);
  //    }
  else
    return CCodec_DXTC::SetParameter(pszParamName, sValue);

  return true;
}

bool CCodec_BC6H::SetParameter(const char* pszParamName, std::uint32_t dwValue)
{
  if (strcmp(pszParamName, "ModeMask") == 0)
    m_ModeMask = (std::uint8_t)dwValue & 0xFF;
  else if (strcmp(pszParamName, "PatternRec") == 0)
    m_UsePatternRec = (bool)(dwValue > 0);
  else if (strcmp(pszParamName, "NumThreads") == 0)
  {
    m_NumThreads = (std::uint8_t)dwValue;
    m_Use_MultiThreading = m_NumThreads > 1;
  }
  else
    return CCodec_DXTC::SetParameter(pszParamName, dwValue);
  return true;
}

bool CCodec_BC6H::SetParameter(const char* pszParamName, float fValue)
{
  if (strcmp(pszParamName, "Quality") == 0)
    m_Quality = fValue;
  // else
  // if (strcmp(pszParamName, "OutputExposure") == 0)
  //     m_Exposure = fValue;
  else
    return CCodec_DXTC::SetParameter(pszParamName, fValue);
  return true;
}

CCodec_BC6H::~CCodec_BC6H()
{
  if (m_LibraryInitialized)
  {
    if (m_Use_MultiThreading)
    {
      // Tell all the live threads that they can exit when they have finished
      // any current work
      for (int i = 0; i < m_LiveThreads; i++)
      {
        // If a thread is in the running state then we need to wait for it to
        // finish any queued work from the producer before we can tell it to
        // exit.
        //
        // If we don't wait then there is a race condition here where we have
        // told the thread to run but it hasn't yet been scheduled - if we set
        // the exit flag before it runs then its block will not be processed.
#pragma warning(push)
#pragma warning( \
  disable : 4127)  // warning C4127: conditional expression is constant
        while (1)
        {
          if (m_EncodeParameterStorage == nullptr)
            break;
          if (m_EncodeParameterStorage[i].run != TRUE)
          {
            break;
          }
        }
#pragma warning(pop)
        // Signal to the thread that it can exit
        m_EncodeParameterStorage[i].exit = TRUE;
      }

      // Now wait for all threads to have exited
      if (m_LiveThreads > 0)
      {
        for (std::uint32_t dwThread = 0; dwThread < m_LiveThreads; dwThread++)
        {
          std::thread& curThread = m_EncodingThreadHandle[dwThread];

          curThread.join();
        }
      }

      for (unsigned int i = 0; i < m_LiveThreads; i++)
      {
        std::thread& curThread = m_EncodingThreadHandle[i];

        curThread = std::thread();
      }

      delete[] m_EncodingThreadHandle;

    }  // MultiThreading
    else
    {
      // detach thread and delete
      std::thread& curThread = m_EncodingThreadHandle[0];
      curThread.detach();
      delete[] m_EncodingThreadHandle;
    }

    m_EncodingThreadHandle = nullptr;

    if (m_EncodeParameterStorage)
      delete[] m_EncodeParameterStorage;
    m_EncodeParameterStorage = nullptr;

    for (int i = 0; i < m_NumEncodingThreads; i++)
    {
      if (m_encoder[i])
      {
        delete m_encoder[i];
        m_encoder[i] = nullptr;
      }
    }

    if (m_decoder)
    {
      delete m_decoder;
      m_decoder = nullptr;
    }

    m_LibraryInitialized = false;
  }
}

CodecError CCodec_BC6H::CInitializeBC6HLibrary()
{
  if (!m_LibraryInitialized)
  {
    for (std::uint32_t i = 0; i < BC6H_MAX_THREADS; i++)
    {
      m_encoder[i] = nullptr;
    }

    // Create threaded encoder instances
    m_LiveThreads = 0;
    m_LastThread = 0;
    m_NumEncodingThreads = (std::uint16_t)min(m_NumThreads, BC6H_MAX_THREADS);
    if (m_NumEncodingThreads == 0)
      m_NumEncodingThreads = 1;
    m_Use_MultiThreading = m_NumEncodingThreads > 1;

    m_EncodeParameterStorage = new BC6HEncodeThreadParam[m_NumEncodingThreads];
    if (!m_EncodeParameterStorage)
    {
      return CE_Unknown;
    }

    for (int i = 0; i < m_NumEncodingThreads; i++)
      m_EncodeParameterStorage[i].run = false;

    m_EncodingThreadHandle = new std::thread[m_NumEncodingThreads];
    if (!m_EncodingThreadHandle)
    {
      delete[] m_EncodeParameterStorage;
      m_EncodeParameterStorage = nullptr;

      return CE_Unknown;
    }

    for (int i = 0; i < m_NumEncodingThreads; i++)
    {
      // Create single encoder instance
      CMP_BC6H_BLOCK_PARAMETERS user_options;

      user_options.bIsSigned = m_bIsSigned;
      user_options.fQuality = m_Quality;
      user_options.dwMask = m_ModeMask;
      user_options.fExposure = m_Exposure;
      user_options.bUsePatternRec = m_UsePatternRec;

      m_encoder[i] = new BC6HBlockEncoder(user_options);

      // Cleanup if problem!
      if (!m_encoder[i])
      {
        if (m_EncodeParameterStorage)
          delete[] m_EncodeParameterStorage;
        m_EncodeParameterStorage = nullptr;

        delete[] m_EncodingThreadHandle;
        m_EncodingThreadHandle = nullptr;

        for (int j = 0; j < i; j++)
        {
          delete m_encoder[j];
          m_encoder[j] = nullptr;
        }

        return CE_Unknown;
      }

#ifdef USE_DBGTRACE
      DbgTrace(
        ("Encoder[%d]:ModeMask %X, Quality %f\n", i, m_ModeMask, m_Quality));
#endif
    }

    // Create the encoding threads
    for (unsigned int i = 0; i < m_NumEncodingThreads; i++)
    {
      // Initialize thread parameters.
      m_EncodeParameterStorage[i].encoder = m_encoder[i];
      // Inform the thread that at the moment it doesn't have any work to do
      // but that it should wait for some and not exit
      m_EncodeParameterStorage[i].run = FALSE;
      m_EncodeParameterStorage[i].exit = FALSE;

      m_EncodingThreadHandle[i] =
        std::thread(BC6HThreadProcEncode, (void*)&m_EncodeParameterStorage[i]);
      m_LiveThreads++;
    }

    // Create single decoder instance
    m_decoder = new BC6HBlockDecoder();
    if (!m_decoder)
    {
      for (std::uint32_t j = 0; j < m_NumEncodingThreads; j++)
      {
        delete m_encoder[j];
        m_encoder[j] = nullptr;
      }
      return CE_Unknown;
    }

    m_LibraryInitialized = true;
  }
  return CE_OK;
}

CodecError CCodec_BC6H::CEncodeBC6HBlock(
  float in[MAX_SUBSET_SIZE][MAX_DIMENSION_BIG], std::uint8_t* out)
{
  if (m_Use_MultiThreading)
  {
    std::uint16_t threadIndex;

    if ((!m_LibraryInitialized) || (!in) || (!out))
    {
      return CE_Unknown;
    }

    // Loop and look for an available thread
    BOOL found = FALSE;
    threadIndex = m_LastThread;
    while (found == FALSE)
    {
      if (m_EncodeParameterStorage[threadIndex].run == FALSE)
      {
        found = TRUE;
        break;
      }

      // Increment and wrap the thread index
      threadIndex++;
      if (threadIndex == m_LiveThreads)
      {
        threadIndex = 0;
      }
    }

    m_LastThread = threadIndex;

    // Copy the input data into the thread storage
    memcpy(m_EncodeParameterStorage[threadIndex].in, in,
           MAX_SUBSET_SIZE * MAX_DIMENSION_BIG * sizeof(float));

    // Set the output pointer for the thread to the provided location
    m_EncodeParameterStorage[threadIndex].out = out;

    // Tell the thread to start working
    m_EncodeParameterStorage[threadIndex].run = TRUE;
  }
  else
  {
    // Copy the input data into the thread storage
    memcpy(m_EncodeParameterStorage[0].in, in,
           BC6H_MAX_SUBSET_SIZE * MAX_DIMENSION_BIG * sizeof(float));
    // Set the output pointer for the thread to write
    m_EncodeParameterStorage[0].out = out;
    m_encoder[0]->CompressBlock(m_EncodeParameterStorage[0].in,
                                m_EncodeParameterStorage[0].out);
  }
  return CE_OK;
}

CodecError CCodec_BC6H::CFinishBC6HEncoding(void)
{
  if (!m_LibraryInitialized)
  {
    return CE_Unknown;
  }

  if (m_Use_MultiThreading)
  {
    // Wait for all the live threads to finish any current work
    for (std::uint32_t i = 0; i < m_LiveThreads; i++)
    {
      using namespace chrono;

      // If a thread is in the running state then we need to wait for it to
      // finish its work from the producer
      while (m_EncodeParameterStorage[i].run == TRUE)
      {
        std::this_thread::sleep_for(1ms);
      }
    }
  }
  return CE_OK;
}

#ifdef BC6H_DEBUG_TO_RESULTS_TXT
FILE* g_fp = nullptr;
int g_mode = 0;
#endif

CodecError CCodec_BC6H::Compress(CCodecBuffer& bufferIn,
                                 CCodecBuffer& bufferOut,
                                 Codec_Feedback_Proc pFeedbackProc,
                                 std::size_t pUser1, std::size_t pUser2)
{
  assert(bufferIn.GetWidth() == bufferOut.GetWidth());
  assert(bufferIn.GetHeight() == bufferOut.GetHeight());

  if (bufferIn.GetWidth() != bufferOut.GetWidth() ||
      bufferIn.GetHeight() != bufferOut.GetHeight())
    return CE_Unknown;

  CodecError err = CInitializeBC6HLibrary();
  if (err != CE_OK)
    return err;

#ifdef BC6H_COMPDEBUGGER
  CompViewerClient g_CompClient;
  if (g_CompClient.connect())
  {
#ifdef USE_DBGTRACE
    DbgTrace(("-------> Remote Server Connected"));
#endif
  }
#endif

#ifdef BC6H_DEBUG_TO_RESULTS_TXT
  g_fp = fopen("AMD_Results.txt", "w");
  g_block = 0;
#endif

  const std::uint32_t dwBlocksX = ((bufferIn.GetWidth() + 3) >> 2);
  const std::uint32_t dwBlocksY = ((bufferIn.GetHeight() + 3) >> 2);

#ifdef _REMOTE_DEBUG
  DbgTrace(("IN : BufferType %d ChannelCount %d ChannelDepth %d",
            bufferIn.GetBufferType(), bufferIn.GetChannelCount(),
            bufferIn.GetChannelDepth()));
  DbgTrace(("   : Height %d Width %d Pitch %d isFloat %d", bufferIn.GetHeight(),
            bufferIn.GetWidth(), bufferIn.GetWidth(), bufferIn.IsFloat()));

  DbgTrace(("OUT: BufferType %d ChannelCount %d ChannelDepth %d",
            bufferOut.GetBufferType(), bufferOut.GetChannelCount(),
            bufferOut.GetChannelDepth()));
  DbgTrace(("   : Height %d Width %d Pitch %d isFloat %d",
            bufferOut.GetHeight(), bufferOut.GetWidth(), bufferOut.GetWidth(),
            bufferOut.IsFloat()));
#endif;

  char row, col, srcIndex;

  std::uint8_t* pOutBuffer;
  pOutBuffer = bufferOut.GetData();

  std::uint8_t* pInBuffer;
  pInBuffer = bufferIn.GetData();

  std::uint32_t block = 0;

#ifdef _SAVE_AS_BC6
  FILE* bc6file = fopen("Test.bc6", "wb");
#endif

  int lineAtPercent = (dwBlocksY * 0.01);
  if (lineAtPercent <= 0)
    lineAtPercent = 1;
  float fBlockXY = dwBlocksX * dwBlocksY;
  float fProgress;
  float old_fProgress = FLT_MAX;

  for (std::uint32_t j = 0; j < dwBlocksY; j++)
  {
    for (std::uint32_t i = 0; i < dwBlocksX; i++)
    {
      float blockToEncode[BLOCK_SIZE_4X4][CHANNEL_SIZE_ARGB];
      float srcBlock[BLOCK_SIZE_4X4X4];

      memset(srcBlock, 0, sizeof(srcBlock));
      bufferIn.ReadBlockRGBA(i * 4, j * 4, 4, 4, srcBlock);

#ifdef _BC6H_COMPDEBUGGER
      g_CompClient.SendData(1, sizeof(srcBlock), srcBlock);
#endif

      // Create the block for encoding
      srcIndex = 0;
      for (row = 0; row < BLOCK_SIZE_4; row++)
      {
        for (col = 0; col < BLOCK_SIZE_4; col++)
        {
          blockToEncode[row * BLOCK_SIZE_4 + col][BC6H_COMP_RED] =
            (float)srcBlock[srcIndex];
          blockToEncode[row * BLOCK_SIZE_4 + col][BC6H_COMP_GREEN] =
            (float)srcBlock[srcIndex + 1];
          blockToEncode[row * BLOCK_SIZE_4 + col][BC6H_COMP_BLUE] =
            (float)srcBlock[srcIndex + 2];
          blockToEncode[row * BLOCK_SIZE_4 + col][BC6H_COMP_ALPHA] =
            (float)srcBlock[srcIndex + 3];
          srcIndex += 4;
        }
      }

      union BBLOCKS {
        std::uint32_t compressedBlock[4];
        std::uint8_t out[16];
        std::uint8_t in[16];
      } data;

      memset(data.in, 0, sizeof(data));
      CEncodeBC6HBlock(blockToEncode, pOutBuffer + block);

#ifdef _SAVE_AS_BC6
      if (fwrite(pOutBuffer + block, sizeof(char), 16, bc6file) != 16)
        throw "File error on write";
#endif

      block += 16;

#ifdef _BC6H_COMPDEBUGGER  // Checks decompression it should match or be close
                           // to source
      union DBLOCKS {
        float blockToSave[16][4];
        float block[64];
      } savedata;

      std::uint8_t destBlock[BLOCK_SIZE_4X4X4];
      memset(savedata.block, 0, sizeof(savedata));
      m_decoder->DecompressBlock(savedata.blockToSave, data.in);

      for (row = 0; row < 64; row++)
      {
        destBlock[row] = (std::uint8_t)savedata.block[row];
      }
      g_CompClient.SendData(3, sizeof(destBlock), destBlock);
#endif

      if (pFeedbackProc)
      {
        if ((j % lineAtPercent) == 0)
        {
          fProgress = (j * dwBlocksX) / fBlockXY;
          if (fProgress != old_fProgress)
          {
            old_fProgress = fProgress;
            if (pFeedbackProc(fProgress * 100.0f, pUser1, pUser2))
            {
#ifdef _BC6H_COMPDEBUGGER
              g_CompClient.disconnect();
#endif
              CFinishBC6HEncoding();
              return CE_Aborted;
            }
          }
        }
      }
    }
  }

#ifdef _SAVE_AS_BC6
  if (fclose(bc6file))
    throw "Close failed on .bc6 file";
#endif

#ifdef BC6H_COMPDEBUGGER
  g_CompClient.disconnect();
#endif

#ifdef BC6H_DEBUG_TO_RESULTS_TXT
  if (g_fp)
    fclose(g_fp);
#endif

  if (pFeedbackProc)
  {
    float fProgress = 100.f;
    pFeedbackProc(fProgress, pUser1, pUser2);
  }

  return CFinishBC6HEncoding();
}

CodecError CCodec_BC6H::Decompress(CCodecBuffer& bufferIn,
                                   CCodecBuffer& bufferOut,
                                   Codec_Feedback_Proc pFeedbackProc,
                                   std::size_t pUser1, std::size_t pUser2)
{
  assert(bufferIn.GetWidth() == bufferOut.GetWidth());
  assert(bufferIn.GetHeight() == bufferOut.GetHeight());

  CodecError err = CInitializeBC6HLibrary();
  if (err != CE_OK)
    return err;

  if (bufferIn.GetWidth() != bufferOut.GetWidth() ||
      bufferIn.GetHeight() != bufferOut.GetHeight())
    return CE_Unknown;

  const std::uint32_t dwBlocksX = ((bufferIn.GetWidth() + 3) >> 2);
  const std::uint32_t dwBlocksY = ((bufferIn.GetHeight() + 3) >> 2);
  const std::uint32_t dwBlocksXY = dwBlocksX * dwBlocksY;

  for (std::uint32_t j = 0; j < dwBlocksY; j++)
  {
    for (std::uint32_t i = 0; i < dwBlocksX; i++)
    {
      union FBLOCKS {
        float decodedBlock[16][4];
        float destBlock[BLOCK_SIZE_4X4X4];
      } DecData;

      union BBLOCKS {
        std::uint32_t compressedBlock[4];
        std::uint8_t out[16];
        std::uint8_t in[16];
      } CompData;

      float destBlock[BLOCK_SIZE_4X4X4];

      bufferIn.ReadBlock(i * 4, j * 4, CompData.compressedBlock, 4);

      // Encode to the appropriate location in the compressed image
      if (m_CodecType == CT_BC6H_SF)
        m_decoder->bc6signed = true;
      else
        m_decoder->bc6signed = false;

      m_decoder->DecompressBlock(DecData.decodedBlock, CompData.in);

      // Create the block for decoding
      float R, G, B, A;
      int srcIndex = 0;
      for (int row = 0; row < BLOCK_SIZE_4; row++)
      {
        for (int col = 0; col < BLOCK_SIZE_4; col++)
        {
          R = (float)
                DecData.decodedBlock[row * BLOCK_SIZE_4 + col][BC6H_COMP_RED];
          G = (float)
                DecData.decodedBlock[row * BLOCK_SIZE_4 + col][BC6H_COMP_GREEN];
          B = (float)
                DecData.decodedBlock[row * BLOCK_SIZE_4 + col][BC6H_COMP_BLUE];
          A = (float)
                DecData.decodedBlock[row * BLOCK_SIZE_4 + col][BC6H_COMP_ALPHA];
          destBlock[srcIndex] = R;
          destBlock[srcIndex + 1] = G;
          destBlock[srcIndex + 2] = B;
          destBlock[srcIndex + 3] = A;
          srcIndex += 4;
        }
      }

      bufferOut.WriteBlockRGBA(i * 4, j * 4, 4, 4, (float*)destBlock);
    }

    if (pFeedbackProc)
    {
      float fProgress = 100.f * (j * dwBlocksX) / dwBlocksXY;
      if (pFeedbackProc(fProgress, pUser1, pUser2))
      {
        return CE_Aborted;
      }
    }
  }

  return CE_OK;
}

// Not implemented
CodecError CCodec_BC6H::Compress_Fast(CCodecBuffer& bufferIn,
                                      CCodecBuffer& bufferOut,
                                      Codec_Feedback_Proc pFeedbackProc,
                                      std::size_t pUser1, std::size_t pUser2)
{
  return CE_OK;
}

// Not implemented
CodecError CCodec_BC6H::Compress_SuperFast(CCodecBuffer& bufferIn,
                                           CCodecBuffer& bufferOut,
                                           Codec_Feedback_Proc pFeedbackProc,
                                           std::size_t pUser1,
                                           std::size_t pUser2)
{
  return CE_OK;
}
