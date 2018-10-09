/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#include "TootlePCH.h"
#include "JRTCommon.h"
#include "JRTKDTree.h"
#include "JRTH2KDTreeBuilder.h"
#include "JRTMesh.h"

static const int INTERSECT_COST = 5;

void JRTH2KDTreeBuilder::BuildTreeImpl(
  const JRTBoundingBox& rBounds, const std::vector<const JRTTriangle*>& rTris,
  std::vector<JRTKDNode>& rNodesOut, std::vector<std::uint32_t>& rTriIndicesOut)
{

  std::vector<std::uint32_t> nTris;
  nTris.reserve(rTris.size());

  for (std::uint32_t i = 0; i < rTris.size(); i++)
  {
    nTris.push_back(i);
  }

  for (std::uint32_t i = 0; i < rTris.size(); i++)
  {
    Vec3f verts[3];
    verts[0] = rTris[i]->GetV1();
    verts[1] = rTris[i]->GetV2();
    verts[2] = rTris[i]->GetV3();
    JRTBoundingBox bb(verts, 3);

    m_BBX.emplace_back(bb.GetMin().x, bb.GetMax().x);
    m_BBY.emplace_back(bb.GetMin().y, bb.GetMax().y);
    m_BBZ.emplace_back(bb.GetMin().z, bb.GetMax().z);
  }

  rNodesOut.emplace_back();

  DoBuildTree(JRTKDTree::MAX_TREE_DEPTH, &rNodesOut[0], rBounds, rTris, nTris,
              rNodesOut, rTriIndicesOut);
}

void JRTH2KDTreeBuilder::DoBuildTree(
  std::uint32_t nMaxDepth, JRTKDNode* pNode, const JRTBoundingBox& rBounds,
  const std::vector<const JRTTriangle*>& rTris,
  std::vector<std::uint32_t>& rTrisThisNode, std::vector<JRTKDNode>& rNodesOut,
  std::vector<std::uint32_t>& rTriIndicesOut)
{

  // make a leaf if we've hit the depth limit, or if there is a very small
  // number of triangles
  if (nMaxDepth == 0 || rTris.size() <= 2)
  {
    MakeLeaf(pNode, rTrisThisNode, rNodesOut, rTriIndicesOut);
    return;
  }

  // select best split on each axis
  SplitInfo splits[3];
  FindBestSplit(X_AXIS, rBounds, rTris, rTrisThisNode, &splits[0]);
  FindBestSplit(Y_AXIS, rBounds, rTris, rTrisThisNode, &splits[1]);
  FindBestSplit(Z_AXIS, rBounds, rTris, rTrisThisNode, &splits[2]);

  // locate the best axis to split on
  SplitInfo* pBest = nullptr;

  for (int i = 0; i < 3; i++)
  {
    if (splits[i].fHeuristicValue == 0)
    {
      continue;
    }

    if ((pBest == nullptr) ||
        splits[i].fHeuristicValue < pBest->fHeuristicValue)
    {
      if (pBest != nullptr)
      {
        // get rid of un-needed tri-lists to save memory
        pBest->TrisFront.clear();
        pBest->TrisBack.clear();
      }

      pBest = &splits[i];
    }
  }

  if (pBest == nullptr)
  {
    // no good split, make a leaf
    MakeLeaf(pNode, rTrisThisNode, rNodesOut, rTriIndicesOut);
  }
  else
  {
    // split
    Axis eSplitAxis = pBest->eAxis;
    float fSplitValue = pBest->fPosition;

    pNode->inner.axis = eSplitAxis;
    pNode->inner.is_leaf = 0u;
    pNode->inner.position = fSplitValue;

    JRT_ASSERT(fSplitValue > rBounds.GetMin()[eSplitAxis] &&
               fSplitValue < rBounds.GetMax()[eSplitAxis]);

    pNode->inner.front_offset = (std::uint32_t)rNodesOut.size();
    auto nFront = (std::uint32_t)rNodesOut.size();
    std::uint32_t nBack = nFront + 1;
    rNodesOut.emplace_back();
    rNodesOut.emplace_back();

    // split the node bounding box
    JRTBoundingBox front_bounds, back_bounds;
    rBounds.Split(eSplitAxis, fSplitValue, front_bounds, back_bounds);

    // build front subtree
    DoBuildTree(nMaxDepth - 1, &rNodesOut[nFront], front_bounds, rTris,
                pBest->TrisFront, rNodesOut, rTriIndicesOut);
    DoBuildTree(nMaxDepth - 1, &rNodesOut[nBack], back_bounds, rTris,
                pBest->TrisBack, rNodesOut, rTriIndicesOut);
  }
}

void JRTH2KDTreeBuilder::MakeLeaf(
  JRTKDNode* pNode, const std::vector<std::uint32_t>& rTrisThisNode,
  std::vector<JRTKDNode>& /*rNodesOut*/,
  std::vector<std::uint32_t>& rTriIndicesOut)
{
  pNode->leaf.is_leaf = 1u;
  pNode->leaf.triangle_count = (std::uint32_t)rTrisThisNode.size();
  pNode->leaf.triangle_start = (std::uint32_t)rTriIndicesOut.size();

  rTriIndicesOut.insert(rTriIndicesOut.end(), rTrisThisNode.begin(),
                        rTrisThisNode.end());
}

void JRTH2KDTreeBuilder::FindBestSplit(
  Axis eAxis, const JRTBoundingBox& rBounds,
  const std::vector<const JRTTriangle*>& /*rTris*/,
  const std::vector<std::uint32_t>& rTrisThisNode, SplitInfo* pSplit)
{

  // pSplit->TrisBack.reserve( rTris.size() / 2 );
  // pSplit->TrisFront.reserve( rTris.size() / 2 );

  pSplit->eAxis = eAxis;

  // if bounding box is degenerate on this axis, do not split
  if (rBounds.GetMax()[eAxis] == rBounds.GetMin()[eAxis])
  {
    pSplit->fHeuristicValue = 0;
    return;
  }

  // base cost of not splitting
  float fBase = (float)rTrisThisNode.size() * INTERSECT_COST;

  // do a median split
  float fMedian = (rBounds.GetMax()[eAxis] - rBounds.GetMin()[eAxis]) * 0.5f +
                  rBounds.GetMin()[eAxis];
  pSplit->fPosition = fMedian;
  std::uint32_t nStraddle;
  float fTriMin, fTriMax;
  ClassifyTris(pSplit->eAxis, pSplit->fPosition, pSplit->TrisBack,
               pSplit->TrisFront, nStraddle, fTriMin, fTriMax, rTrisThisNode);

  // optimize the split.  If there are no tris on one side or the other, then
  // shift towards the side with tris
  if (pSplit->TrisBack.empty())
  {
    // shift towards the front side
    pSplit->fPosition = fTriMin - 0.00001f;
  }
  else if (pSplit->TrisFront.empty())
  {
    // shift towards the back side
    pSplit->fPosition = fTriMax + 0.00001f;
  }

  pSplit->fHeuristicValue = CostFunction(rBounds, pSplit);

  if (pSplit->fHeuristicValue > fBase ||
      (nStraddle == pSplit->TrisBack.size() &&
       nStraddle == pSplit->TrisFront.size()))
  {
    // don't split if doing so is worse than just staying put
    pSplit->fHeuristicValue = 0;
  }
}

void JRTH2KDTreeBuilder::ClassifyTris(
  Axis eAxis, float fPosition, std::vector<std::uint32_t>& rTrisBack,
  std::vector<std::uint32_t>& rTrisFront, std::uint32_t& nStraddle,
  float& fTriMin, float& fTriMax,
  const std::vector<std::uint32_t>& rTrisThisNode)
{

  const FloatPair* pBB = nullptr;

  switch (eAxis)
  {
  case X_AXIS:
    pBB = &m_BBX[0];
    break;

  case Y_AXIS:
    pBB = &m_BBY[0];
    break;

  case Z_AXIS:
    pBB = &m_BBZ[0];
    break;
  }

  nStraddle = 0;

  for (auto itr = rTrisThisNode.begin(); itr != rTrisThisNode.end(); itr++)
  {
    float fMin = pBB[*itr].first;
    float fMax = pBB[*itr].second;
    fTriMin = Min(fMin, fTriMin);
    fTriMax = Max(fMax, fTriMax);

    if (fMax <= fPosition)
    {
      // back
      rTrisBack.push_back(*itr);
    }
    else if (fMin > fPosition)
    {
      // front
      rTrisFront.push_back(*itr);
    }
    else
    {
      // both
      rTrisBack.push_back(*itr);
      rTrisFront.push_back(*itr);
      nStraddle++;
    }
  }
}

float JRTH2KDTreeBuilder::CostFunction(const JRTBoundingBox& rBounds,
                                       SplitInfo* pSplitOut)
{
  // optimized surface area heuristic

  Vec3f bbSize = rBounds.GetMax() - rBounds.GetMin();

  Axis eAxis = pSplitOut->eAxis;
  float Su = UCOMP(bbSize, eAxis);
  float Sv = VCOMP(bbSize, eAxis);
  float Sw = WCOMP(bbSize, eAxis);

  float constantFaces = Su * Sv;
  float wholeArea = 1.0f / (constantFaces + (Su * Sw) + (Sv * Sw));

  float backSize = pSplitOut->fPosition - rBounds.GetMin()[eAxis];
  float frontSize = bbSize[eAxis] - backSize;
  float frontArea = (constantFaces + frontSize * (Su + Sv));
  float backArea = (constantFaces + backSize * (Su + Sv));

  frontArea *= wholeArea;
  backArea *= wholeArea;

  return 1.0f + INTERSECT_COST * (frontArea * pSplitOut->TrisFront.size() +
                                  backArea * pSplitOut->TrisBack.size());
}