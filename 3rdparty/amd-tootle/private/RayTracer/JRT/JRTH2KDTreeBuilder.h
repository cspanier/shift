/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef JRT_H2_KDTREE_BUILDER_H
#define JRT_H2_KDTREE_BUILDER_H

#include <cstdint>
#include "JRTCoreUtils.h"
#include "JRTKDTreeBuilder.h"

/// The H2 KDTree builder is a simplified heuristic tree builder
///  The algorithm works thusly:
///     - for each axis
///         - Start at spatial median, classify polys
///         - If all polys on one side, shift split plane
///         - Choose axis with lowest spatial median cost (using SAH)

class JRTH2KDTreeBuilder : public JRTKDTreeBuilder
{
public:
  void BuildTreeImpl(const JRTBoundingBox& rBounds,
                     const std::vector<const JRTTriangle*>& rTris,
                     std::vector<JRTKDNode>& rNodesOut,
                     std::vector<std::uint32_t>& rTriIndicesOut) override;

private:
  struct SplitInfo
  {
    Axis eAxis;
    float fHeuristicValue;  ///< Estimated cost of this split.  If 0, it means
                            ///< do not split
    float fPosition;
    std::vector<std::uint32_t> TrisFront;
    std::vector<std::uint32_t> TrisBack;
  };

  void MakeLeaf(JRTKDNode* pNode,
                const std::vector<std::uint32_t>& rTrisThisNode,
                std::vector<JRTKDNode>& rNodesOut,
                std::vector<std::uint32_t>& rTriIndicesOut);

  void DoBuildTree(std::uint32_t nMaxDepth, JRTKDNode* pNode,
                   const JRTBoundingBox& rBounds,
                   const std::vector<const JRTTriangle*>& rTris,
                   std::vector<std::uint32_t>& rTrisThisNode,
                   std::vector<JRTKDNode>& rNodesOut,
                   std::vector<std::uint32_t>& rTriIndicesOut);

  void FindBestSplit(Axis eAxis, const JRTBoundingBox& rBounds,
                     const std::vector<const JRTTriangle*>& rTris,
                     const std::vector<std::uint32_t>& rTrisThisNode,
                     SplitInfo* pSplit);

  void ClassifyTris(Axis eAxis, float fPosition,
                    std::vector<std::uint32_t>& rTrisBack,
                    std::vector<std::uint32_t>& rTrisFront,
                    std::uint32_t& nStraddle, float& fTriMin, float& fTriMax,
                    const std::vector<std::uint32_t>& rTrisThisNode);

  float CostFunction(const JRTBoundingBox& rBounds, SplitInfo* pSplitOut);

  // triangle bounding boxes,used to speed classification
  // these are arranged by axis so that they'll cache better
  typedef std::pair<float, float> FloatPair;
  std::vector<FloatPair> m_BBX;
  std::vector<FloatPair> m_BBY;
  std::vector<FloatPair> m_BBZ;
};

#endif
