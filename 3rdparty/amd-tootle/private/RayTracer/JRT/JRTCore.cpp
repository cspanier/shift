/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#include "TootlePCH.h"
#include "JRTCommon.h"
#include "JRTCore.h"
#include "JRTMesh.h"

#include "JRTKDTree.h"
#include "JRTKDTreeBuilder.h"
#include "JRTHeuristicKDTreeBuilder.h"
#include "JRTH2KDTreeBuilder.h"

JRTCore::JRTCore() : m_pHitArray(new TootleRayHit[5])
{
}

JRTCore::~JRTCore()
{
  JRT_SAFE_DELETE(m_pTree);
  JRT_SAFE_DELETE_ARRAY(m_pHitArray);
}

JRTCore* JRTCore::Build(const std::vector<JRTMesh*>& rMeshes)
{
  auto* pCaster = new JRTCore();

  if (rMeshes.empty())
  {
    pCaster->m_pTree = nullptr;
    return pCaster;
  }

  // build KD tree
  JRTHeuristicKDTreeBuilder builder;
  JRTKDTree* pTree = builder.BuildTree(rMeshes);

  if (pTree == nullptr)
  {
    delete pCaster;
    return nullptr;
  }

  pCaster->m_pTree = pTree;

  return pCaster;
}

int SortTootleHit(const void* h1, const void* h2)
{
  const auto* ph1 = (const TootleRayHit*)h1;
  const auto* ph2 = (const TootleRayHit*)h2;
  return static_cast<int>(ph1->t >= ph2->t);
}

/// \param rOrigin  The ray origin
/// \param rDirection The ray direction
/// \param ppHitArray  A pointer that will be set to point to an array of hits.
/// The caller should NOT delete the returned array \param pHitCount   A pointer
/// that will receive the number of hits in the returned array. \return Returns
/// false if out of memory, true otherwise.
bool JRTCore::FindAllHits(const Vec3f& rOrigin, const Vec3f& rDirection,
                          TootleRayHit** ppHitArray, std::uint32_t* pHitCount)
{
  if (m_pTree == nullptr)
  {
    *ppHitArray = nullptr;
    *pHitCount = 0;
  }

  std::uint32_t nHits =
    m_pTree->FindAllHits(rOrigin, rDirection, &m_pHitArray, &m_nArraySize);

  if (nHits == 0 || nHits == JRTKDTree::OUT_OF_MEMORY)
  {
    *ppHitArray = nullptr;
    *pHitCount = 0;
  }

  if (nHits == JRTKDTree::OUT_OF_MEMORY)
  {
    return false;
  }

#ifdef DEBUG

  // verify that there are no duplicates, just a test
  for (std::uint32_t i = 0; i < nHits; i++)
  {
    for (std::uint32_t j = i + 1; j < nHits; j++)
      JRT_ASSERT(m_pHitArray[i].nFaceID != m_pHitArray[j].nFaceID);
  }

#endif

  // sort hits by distance
  qsort(m_pHitArray, nHits, sizeof(TootleRayHit), &SortTootleHit);
  *ppHitArray = m_pHitArray;
  *pHitCount = nHits;

  return true;
}

void JRTCore::CullBackfaces(const Vec3f& rViewDir, bool bCullCCW)
{
  m_pTree->CullBackfaces(rViewDir, bCullCCW);
}

bool JRTCore::GetSceneBBHit(const Vec3f& rOrigin, const Vec3f& rDirection,
                            Vec3f* pHitPt)
{
  const JRTBoundingBox& rBB = m_pTree->GetSceneBounds();
  float fTMin, fTMax;

  if (!rBB.RayHit(rOrigin, rDirection, &fTMin, &fTMax))
  {
    return false;
  }

  if (fTMax < 0.0f)
  {
    return false;
  }

  *pHitPt = rOrigin + rDirection * fTMax;
  return true;
}

const JRTBoundingBox& JRTCore::GetSceneBB() const
{
  return m_pTree->GetSceneBounds();
}
