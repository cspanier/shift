/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#include "TootlePCH.h"
#include "JRTCommon.h"
#include "JRTKDTreeBuilder.h"
#include "JRTKDTree.h"
#include "JRTMesh.h"
#include "JRTCoreUtils.h"

//
//
//    PartitionTriangles
//        Partitions a set of triangles into triangles that are in front of
//        and behind the splitting plane.  Triangles the straddle the plane are
//        placed in both sets
//
void PartitionTriangles(const std::vector<const JRTTriangle*>& input,
                        const std::vector<std::uint32_t>& rIndicesIn,
                        Axis split_component, float split_value,
                        std::vector<std::uint32_t>& front,
                        std::vector<std::uint32_t>& back)

{
  for (std::uint32_t i = 0; i < rIndicesIn.size(); i++)
  {
    TriPlaneState test_val =
      TriPlaneTest(*input[rIndicesIn[i]], split_component, split_value);

    switch (test_val)
    {
    case IN_FRONT:
      front.push_back(rIndicesIn[i]);
      break;

    case IN_BACK:
      back.push_back(rIndicesIn[i]);
      break;

    case STRADDLE:
      front.push_back(rIndicesIn[i]);
      back.push_back(rIndicesIn[i]);
      break;
    }
  }
}

void BuildTreeSimple(JRTKDNode* current_node,
                     const std::vector<const JRTTriangle*>& triangles_in,
                     const std::vector<std::uint32_t>& rIndicesIn,
                     const JRTBoundingBox& scene_bounds,
                     std::uint32_t depth_max, std::vector<JRTKDNode>& nodes_out,
                     std::vector<std::uint32_t>& rTriIndicesOut)
{
  if (depth_max == 0 || rIndicesIn.size() < 25)
  {
    // make leaf
    current_node->leaf.is_leaf = 1u;
    current_node->leaf.triangle_start = (std::uint32_t)rTriIndicesOut.size();
    current_node->leaf.triangle_count = (std::uint32_t)rIndicesIn.size();

    for (std::uint32_t i = 0; i < rIndicesIn.size(); i++)
    {
      rTriIndicesOut.push_back(rIndicesIn[i]);
    }

    return;
  }

  // choose best split plane and split node bounding box
  float diffs[3];
  Axis maxcomp = X_AXIS;

  // choose split plane based on longest BBox axis
  JRTBoundingBox front_bounds(scene_bounds);
  JRTBoundingBox back_bounds(scene_bounds);

  for (int i = X_AXIS; i < Z_AXIS; i++)
  {
    diffs[i] = scene_bounds.GetMax()[i] - scene_bounds.GetMin()[i];

    if (diffs[i] > diffs[maxcomp])
    {
      maxcomp = (Axis)i;
    }
  }

  float splitval = 0.5f * diffs[maxcomp] + scene_bounds.GetMin()[maxcomp];

  front_bounds.GetMin()[maxcomp] = splitval;
  back_bounds.GetMax()[maxcomp] = splitval;

  // partition polygons
  std::vector<std::uint32_t> front_polys;
  std::vector<std::uint32_t> back_polys;
  PartitionTriangles(triangles_in, rIndicesIn, (Axis)maxcomp, splitval,
                     front_polys, back_polys);

  // make current node an inner node
  current_node->leaf.is_leaf = 0u;
  current_node->inner.axis = (UBYTE)maxcomp;
  current_node->inner.position = splitval;
  current_node->inner.front_offset = (std::uint32_t)nodes_out.size();

  auto front_child = (std::uint32_t)nodes_out.size();
  std::uint32_t back_child = (std::uint32_t)nodes_out.size() + 1;

  // always allocate left child next to right child
  nodes_out.emplace_back();
  nodes_out.emplace_back();

  // recursively continue tree construction
  BuildTreeSimple(&nodes_out[front_child], triangles_in, front_polys,
                  front_bounds, depth_max - 1, nodes_out, rTriIndicesOut);
  BuildTreeSimple(&nodes_out[back_child], triangles_in, back_polys, back_bounds,
                  depth_max - 1, nodes_out, rTriIndicesOut);
}

void JRTKDTreeBuilder::BuildTreeImpl(
  const JRTBoundingBox& rBounds, const std::vector<const JRTTriangle*>& rTris,
  std::vector<JRTKDNode>& rNodesOut, std::vector<std::uint32_t>& rTriIndicesOut)
{

  rNodesOut.emplace_back();  // create root node

  std::vector<std::uint32_t> rIndices;

  for (std::uint32_t i = 0; i < rTris.size(); i++)
  {
    rIndices.push_back(i);
  }

  // construct the tree using our naive tree building function
  BuildTreeSimple(&rNodesOut[0], rTris, rIndices, rBounds,
                  JRTKDTree::MAX_TREE_DEPTH, rNodesOut, rTriIndicesOut);
}

JRTKDTree* JRTKDTreeBuilder::BuildTree(const std::vector<JRTMesh*>& rMeshes)
{
  JRTKDTree* pTree = nullptr;
  std::vector<JRTKDNode> nodes;
  std::vector<std::uint32_t> indices;
  std::vector<const JRTTriangle*> triArray;
  JRTBoundingBox scene_bounds(Vec3f(FLT_MAX), Vec3f(-FLT_MAX));

  // build an array over all of the triangles
  for (std::uint32_t i = 0; i < rMeshes.size(); i++)
  {
    const JRTTriangle* pTri = rMeshes[i]->GetTriangles();

    for (std::uint32_t j = 0; j < rMeshes[i]->GetTriangleCount(); j++)
    {
      triArray.push_back(pTri);
      pTri++;
    }
  }

  JRT_ASSERT(!triArray.empty());

  // compute scene bounding box
  for (std::uint32_t i = 0; i < triArray.size(); i++)
  {
    // this will cause the fourth component of each point
    // passed to Expand() to be junk.  This is ok

    scene_bounds.Expand(triArray[i]->GetV1());
    scene_bounds.Expand(triArray[i]->GetV2());
    scene_bounds.Expand(triArray[i]->GetV3());
  }

  // slight hack:  Expand bounding box a little bit.  This fixes some precision
  // issues with flat polygons on the edge of the model
  scene_bounds.GetMin() += Vec3f(-0.001f, -0.001f, -0.001f);
  scene_bounds.GetMax() += Vec3f(0.001f, 0.001f, 0.001f);

  // construct the tree
  BuildTreeImpl(scene_bounds, triArray, nodes, indices);

  pTree = new JRTKDTree;
  pTree->m_pIndexArray = new std::uint32_t[indices.size()];
  pTree->m_pMailboxes = new std::uint32_t[triArray.size()];
  pTree->m_bBackFacing = new bool[triArray.size()];

  // initialize the tree structure
  pTree->m_pNodeArray =
    (JRTKDNode*)_aligned_malloc(sizeof(JRTKDNode) * nodes.size(), 16);
  pTree->m_pTriArray = (JRTCoreTriangle*)_aligned_malloc(
    sizeof(JRTCoreTriangle) * triArray.size(), 16);

  if ((pTree->m_pNodeArray == nullptr) || (pTree->m_pTriArray == nullptr))
  {
    JRT_SAFE_DELETE(pTree);
    return nullptr;
  }

  pTree->m_nNodeCount = (std::uint32_t)nodes.size();
  pTree->m_nTriangleCount = (std::uint32_t)triArray.size();
  pTree->m_nIndexCount = (std::uint32_t)indices.size();
  pTree->m_treeBounds = scene_bounds;

  // copy node array
  for (std::uint32_t i = 0; i < nodes.size(); i++)
  {
    pTree->m_pNodeArray[i] = nodes[i];
  }

  // create index array
  for (std::uint32_t i = 0; i < indices.size(); i++)
  {
    pTree->m_pIndexArray[i] = indices[i];
  }

  // create mailbox array
  memset(pTree->m_pMailboxes, 0, sizeof(std::uint32_t) * triArray.size());
  pTree->m_nNextRayID = 1;

  // preprocess triangles and copy to array
  for (std::uint32_t i = 0; i < triArray.size(); i++)
  {
    PreprocessTri(triArray[i]->GetV1(), triArray[i]->GetV2(),
                  triArray[i]->GetV3(), &pTree->m_pTriArray[i]);
    pTree->m_pTriArray[i].pMesh = triArray[i]->GetMesh();
    pTree->m_pTriArray[i].nTriIndex = triArray[i]->GetIndexInMesh();
  }

  // initialize array of flags telling whether or not a triangle is backfacing
  // (TOOTLE SPECIFIC)
  for (std::uint32_t i = 0; i < triArray.size(); i++)
  {
    pTree->m_bBackFacing[i] = false;
  }

  return pTree;
}