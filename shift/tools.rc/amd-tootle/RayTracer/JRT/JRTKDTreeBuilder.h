/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef JRT_KDTREE_BUILDER_H
#define JRT_KDTREE_BUILDER_H

#include <cstdint>
#include "JRTCommon.h"

class JRTMesh;
class JRTKDTree;
class JRTTriangle;
class JRTKDNode;
class JRTBoundingBox;

/// \brief The tree builder class is responsible for constructing a KD tree from
/// triangle soup. The base implementation uses a stupid naive splitting
/// heuristic
class JRTKDTreeBuilder
{
public:
  JRTKDTree* BuildTree(const std::vector<JRTMesh*>& rMeshes);

protected:
  virtual void BuildTreeImpl(const JRTBoundingBox& rBounds,
                             const std::vector<const JRTTriangle*>& rTris,
                             std::vector<JRTKDNode>& rNodesOut,
                             std::vector<std::uint32_t>& rTriIndicesOut);
};

#endif
