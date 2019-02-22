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
//  File Name:   ATIXCodec.h
//  Description: performs the DXT-style block compression
//
//////////////////////////////////////////////////////////////////////////////

#ifndef COMPRESSONATOR_ATIXCODEC_H
#define COMPRESSONATOR_ATIXCODEC_H

#include <cstdint>

/*------------------------------------------------------------------------------------
void CompRGBBlock(float* block_32,    [IN]  array of the 3 component color
vectors (888) std::uint32_t block_dxtc[2],                [OUT] compressed data
in DXT1 for mat int _NbrClrs = BLOCK_SIZE_4X4,            [IN]  actuall number
of elements in the array bool _bUseSSE2 = true);                    [IN]  switch
to SSE2 implementation
------------------------------------------------------------------------------------*/
float CompRGBBlock(const float* block_32, std::uint16_t dwBlockSize,
                   std::uint8_t nRedBits, std::uint8_t nGreenBits,
                   std::uint8_t nBlueBits, std::uint8_t nEndpoints[3][2],
                   std::uint8_t* pcIndices, std::uint8_t dwNumPoints,
                   bool _bUseSSE2, bool b3DRefinement,
                   std::uint8_t nRefinementSteps,
                   float* _pfChannelWeights = nullptr, bool _bUseAlpha = false,
                   float _fAlphaThreshold = 0.5);

/*------------------------------------------------------------------------------------
void CompRGBBlock(std::uint32_t* block_32,    [IN]  array of the 3 component
color vectors (888) std::uint32_t block_dxtc[2],                [OUT] compressed
data in DXT1 for mat int _NbrClrs = BLOCK_SIZE_4X4,            [IN]  actuall
number of elements in the array bool _bUseSSE2 = true);                    [IN]
switch to SSE2 implementation
------------------------------------------------------------------------------------*/
float CompRGBBlock(std::uint32_t* block_32, std::uint16_t dwBlockSize,
                   std::uint8_t nRedBits, std::uint8_t nGreenBits,
                   std::uint8_t nBlueBits, std::uint8_t nEndpoints[3][2],
                   std::uint8_t* pcIndices, std::uint8_t dwNumPoints,
                   bool _bUseSSE2, bool b3DRefinement,
                   std::uint8_t nRefinementSteps,
                   float* _pfChannelWeights = nullptr, bool _bUseAlpha = false,
                   std::uint8_t _nAlphaThreshold = 128);

/*--------------------------------------------------------------------------------------------
// input [0,1]
void CompBlock1X(float* _Blk,              [IN] scalar data block (alphas
or normals) in float format std::uint32_t blockCompressed[2], [OUT] compressed
data in DXT5 alpha foramt int _NbrClrs,                    [IN] actual number of
elements in the block int _intPrec,                    [IN} integer precision;
it applies both to the input data and to the ramp points int _fracPrec, [IN]
fractional precision of the ramp points bool _bFixedRamp,                [IN]
non-fixed ramp means we have input and generate output as float. fixed ramp
means that they are fractional numbers. bool _bUseSSE2                    [IN]
forces to switch to the SSE2 implementation
                )

---------------------------------------------------------------------------------------------*/

float CompBlock1X(float* _Blk, std::uint16_t dwBlockSize,
                  std::uint8_t nEndpoints[2], std::uint8_t* pcIndices,
                  std::uint8_t dwNumPoints, bool bFixedRampPoints,
                  bool _bUseSSE2 = true, int _intPrec = 8, int _fracPrec = 0,
                  bool _bFixedRamp = true);

/*--------------------------------------------------------------------------------------------
// input [0,255]
void CompBlock1X(std::uint8_t* _Blk,                [IN] scalar data block
(alphas or normals) in 8 bits format std::uint32_t blockCompressed[2],    [OUT]
compressed data in DXT5 alpha foramt int _NbrClrs,                        [IN]
actual number of elements in the block int _intPrec,                        [IN]
integer precision; it applies both to the input data and to the ramp points int
_fracPrec,                        [IN] fractional precision of the ramp points
                 bool _bFixedRamp,                    [IN] always true at this
point bool _bUseSSE2                        [IN] forces to switch to the SSE2
implementation
                )
---------------------------------------------------------------------------------------------*/

float CompBlock1X(const std::uint8_t* _Blk, std::uint16_t dwBlockSize,
                  std::uint8_t nEndpoints[2], std::uint8_t* pcIndices,
                  std::uint8_t dwNumPoints, bool bFixedRampPoints,
                  bool _bUseSSE2 = true, int _intPrec = 8, int _fracPrec = 0,
                  bool _bFixedRamp = true);

#endif
