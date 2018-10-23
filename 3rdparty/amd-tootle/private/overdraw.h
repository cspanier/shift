/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef TOOTLE_OVERDRAW_MODULE_H
#define TOOTLE_OVERDRAW_MODULE_H

#include "tootlelib.h"
#include "vector.h"
#include "feedback.h"
#include <vector>

#define TOOTLE_RAYTRACE_IMAGE_SIZE \
  512  // the image size used to optimize and measure overdraw using ray tracing
       // implementation

class Soup;

TootleResult ODInit();

/// Determines whether or not ODInit has been called
bool ODIsInitialized();

TootleResult ODSetSoup(Soup* pSoup, TootleFaceWinding eFrontWinding);

TootleResult ODObjectOverdraw(const float* pViewpoints,
                              unsigned int nViewpoints, float& fODAvg,
                              float& fODMax);
TootleResult ODObjectOverdrawRaytrace(
  const float* pfVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, const float* pViewpoints, unsigned int nViewpoints,
  bool bCullCCW, float& fAvgOD, float& fMaxOD);

TootleResult ODOverdrawGraph(const float* pViewpoints, unsigned int nViewpoints,
                             bool bCullCCW, const std::vector<int>& rClusters,
                             const std::vector<int>& rClusterStart,
                             std::vector<t_edge>& rGraphOut);

void ODCleanup();

#endif
