/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef JRT_COMMON_H
#define JRT_COMMON_H

/// epsilon value used in intersection computations.
#define JRTEPSILON 0.00001f

#include "TootlePCH.h"

#include <cstdint>
#include <iostream>

using UBYTE = unsigned char;

#include "JML.h"
using namespace JML;

#define JRT_ASSERT(x) assert(x)
#define JRT_SAFE_DELETE(x) \
  {                        \
    if (x)                 \
      delete x;            \
    x = NULL;              \
  }
#define JRT_SAFE_DELETE_ARRAY(x) \
  {                              \
    if (x)                       \
      delete[] x;                \
    x = NULL;                    \
  }

#endif
