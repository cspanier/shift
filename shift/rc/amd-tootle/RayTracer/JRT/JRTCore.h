/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef JRT_CORE_H
#define JRT_CORE_H

#include <cstdint>
#include "JRTCommon.h"
#include "JRTTriangleIntersection.h"

struct JRTHitInfo
{
  // the triangle that was hit
  const JRTMesh* pMesh;
  std::uint32_t nIndex;

  Vec3f mPosition;
  Vec3f mNormal;
};

struct TootleRayHit
{
  float t;
  std::uint32_t nFaceID;
};

class JRTCSGNode;
class JRTMesh;
class JRTKDTree;
class JRTBoundingBox;

class JRTCore
{
public:
  static JRTCore* Build(const std::vector<JRTMesh*>& rMeshes);

  ~JRTCore();

  bool FindAllHits(const Vec3f& rOrigin, const Vec3f& rDirection,
                   TootleRayHit** ppHitArray, std::uint32_t* pHitCount);

  void CullBackfaces(const Vec3f& rViewDir, bool bCullCCW);

  /// Locates the position at which the given ray hits the scene bounding box.
  /// Returns false if the ray misses the bounding box
  bool GetSceneBBHit(const Vec3f& rOrigin, const Vec3f& rDirection,
                     Vec3f* pHitPt);

  const JRTBoundingBox& GetSceneBB() const;

private:
  JRTCore();

  TootleRayHit* m_pHitArray;
  std::uint32_t m_nArraySize{5};

  JRTKDTree* m_pTree = nullptr;
};

#endif
