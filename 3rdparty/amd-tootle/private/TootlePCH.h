/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef _TOOTLEPCH_H_
#define _TOOTLEPCH_H_

// disable VC++ 2K5 warnings about deprecated standard C functions
#if defined(_MSC_VER)
#if _MSC_VER >= 1400
#define _CRT_SECURE_NO_DEPRECATE
#endif
#endif

#ifdef _LINUX
#define __cdecl
// #define _isnan(x) isnan(x)
// #define _finite(x) finite(x)
#endif

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

// suppress Debug and error output stream for both release and debug builds
#define ERROR_SILENT

#include <cassert>
#include <cfloat>

#endif  // _SU_TOOTLEPCH_H_
