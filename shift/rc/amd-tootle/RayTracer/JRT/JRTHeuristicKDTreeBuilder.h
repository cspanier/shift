/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef JRT_HEURISTIC_KD_TREE_BUILDER_H
#define JRT_HEURISTIC_KD_TREE_BUILDER_H

#include <cstdint>
#include "JRTKDTreeBuilder.h"
#include "JRTBoundingBox.h"
#include "JRTCoreUtils.h"

/// \brief A smart KD tree builder which uses the surface area heuristic
/// Most of the inspiration for this code comes from Havran's PhD thesis
class JRTHeuristicKDTreeBuilder : public JRTKDTreeBuilder
{
public:
  struct TriangleBB
  {
    const JRTTriangle* pTri;
    std::uint32_t nIndex;
    JRTBoundingBox box;

    // state flag used to classify this BB with respect to current partitioning
    // plane
    mutable TriPlaneState planeState;
  };

  struct Split
  {
    float value;
    unsigned bMaxSplit : 1;
    unsigned nTriBB : 31;

    // bool bMaxSplit; /// Is this a max split (end of BB) or a min split
    // (beginning of BB) const TriangleBB* pTriBB;
  };

  using SplitVec = std::vector<Split*>;

protected:
  void BuildTreeImpl(const JRTBoundingBox& rBounds,
                     const std::vector<const JRTTriangle*>& rTris,
                     std::vector<JRTKDNode>& rNodesOut,
                     std::vector<std::uint32_t>& rTrisOut) override;

private:
  /* void BuildTreeRecursive( std::uint32_t nDepth,
                            const JRTBoundingBox& rNodeBounds,
                            const std::vector<TriangleBB*> rBBs[3],
                            JRTKDNode* pNode,
                            std::vector<JRTKDNode>& rNodesOut,
                            std::vector<std::uint32_t>& rTriIndicesOut );*/

  void ExtractSplits(std::uint32_t eAxis, const std::vector<TriangleBB>& rBBs,
                     std::vector<Split>& rSplits,
                     const JRTBoundingBox& rSceneBounds);

  void ClassifyBBs(SplitVec& rSplits, std::vector<TriangleBB>& rBBs,
                   std::uint32_t eAxis, float fValue);

  void LocateBestSplit(float fNodeBBInvArea, const JRTBoundingBox& rNodeBounds,
                       const SplitVec splits[3], std::uint32_t axis,
                       std::uint32_t nTriCount, float& fBestCost, bool& bSplit,
                       std::uint32_t& eSplitAxis, float& fSplitValue);

  void BuildTreeRecursive(std::uint32_t nDepthLimit,
                          const JRTBoundingBox& rNodeBounds, SplitVec splits[3],
                          std::vector<TriangleBB*>& rBBsThisNode,
                          JRTKDNode* pNode, std::vector<JRTKDNode>& rNodesOut,
                          std::vector<std::uint32_t>& rTrisOut);

  JRTBoundingBox m_scene_bounds;

  std::vector<TriangleBB> m_bbs;
};

#endif
