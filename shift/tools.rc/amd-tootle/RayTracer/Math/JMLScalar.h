/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef JML_SCALAR_H
#define JML_SCALAR_H

#include <xmmintrin.h>
#include <stdlib.h>

namespace JML
{
inline float RandomFloat()
{
  return (float)rand() / (float)RAND_MAX;
};

inline float FastSQRT(float v)
{
  __m128 val = _mm_load1_ps(&v);
  val = _mm_sqrt_ss(val);
  return val.m128_f32[0];
};

inline float FastRSQ(float v)
{
  __m128 val = _mm_load1_ps(&v);
  val = _mm_rsqrt_ss(val);
  float frsq = val.m128_f32[0];
  return (0.5f * frsq) * (3.0f - (v * frsq) * frsq);
};
};

#endif
