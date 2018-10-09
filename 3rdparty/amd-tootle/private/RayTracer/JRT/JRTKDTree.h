/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef JRT_KDTREE_H
#define JRT_KDTREE_H

#include <cstdint>
#include "JRTCommon.h"
#include "JRTBoundingBox.h"
#include "JRTTriangleIntersection.h"
#include "JRTCore.h"

#ifdef _LINUX
#include "../aligned_malloc.h"
#define _aligned_malloc aligned_malloc
#define _aligned_free aligned_free
#endif

// ***********************************************************
//  Node Data Structure
// ***********************************************************

// the #pragma pack(1) is so that VC++ will make this structure 8 bytes wide
// if you omit the pack(), VC++ will pad the internal structs and throw the size
// off
#pragma pack(1)
class JRTKDNode
{
public:
  inline bool IsLeaf() const
  {
    return (inner.is_leaf == 1);
  }

  union {
    struct
    {
      // if an inner node, use this one
      unsigned is_leaf : 1;
      unsigned axis : 3;
      unsigned front_offset : 28;

      float position;  // position of splitting plane

    } inner;

    struct
    {
      // if a leaf, use this one
      unsigned is_leaf : 1;
      unsigned
        triangle_start : 31;  // offset of first triangle in triangle list
      std::uint32_t triangle_count;  // number of triangles in this node

    } leaf;
  };
};
#pragma pack()

class JRTKDTree
{
public:
  static const std::uint32_t MAX_TREE_DEPTH;

  /// Overloaded new operator allocates 16-byte aligned objects using
  /// _aligned_malloc
  void* operator new(size_t nSize)
  {
    return _aligned_malloc(nSize, 16);
  }

  /// Overloaded delete operator uses _aligned_free()
  void operator delete(void* pObj)
  {
    _aligned_free(pObj);
  }

  ~JRTKDTree();

  bool FindFirstHit(const Vec3f& rOrigin, const Vec3f& rDirection,
                    JRTHitInfo* pHit, const JRTMesh* pExcludeMesh,
                    std::uint32_t nExcludeTri, bool* pCSGStates);

  void CullBackfaces(const Vec3f& rViewDir, bool bCullCCW);

  std::uint32_t FindAllHits(const Vec3f& rOrigin, const Vec3f& rDirection,
                            TootleRayHit** ppHitArray,
                            std::uint32_t* pnArraySize);

  std::uint32_t GetNodeCount() const
  {
    return m_nNodeCount;
  }

  /// Returns total number of tris in tree (not taking into account duplication)
  std::uint32_t GetTriCount() const
  {
    return m_nTriangleCount;
  }

  /// Returns total number of triangle references for all leaves
  std::uint32_t GetIndexCount() const
  {
    return m_nIndexCount;
  }

  std::uint32_t GetMaxDepth() const;

  std::uint32_t GetLeafCount() const;

  /// Returns an estimate of the amount of memory used by the tree
  std::uint32_t GetMemoryUsage() const;

  /// Returns the scene bounding box
  const JRTBoundingBox& GetSceneBounds() const
  {
    return m_treeBounds;
  }

  static const std::uint32_t OUT_OF_MEMORY = 0xffffffff;

private:
  std::uint32_t RecurseMaxDepth(std::uint32_t nNode) const;

  friend class JRTKDTreeBuilder;
  JRTKDTree();

  // ***********************************************************
  //   Tree internals
  // ***********************************************************

  // tree bounding box
  JRTBoundingBox m_treeBounds;

  // number of nodes
  std::uint32_t m_nNodeCount;

  // number of triangles
  std::uint32_t m_nTriangleCount = 0;

  // number of triangle indices
  std::uint32_t m_nIndexCount = 0;

  // array of KDTree nodes
  JRTKDNode* m_pNodeArray = nullptr;

  // array of triangle indices.  All indices for all nodes are packed into
  // contiguous memory. each node stores an offset for its triangle list in this
  // array
  std::uint32_t* m_pIndexArray;

  // array of pre-processed triangles
  JRTCoreTriangle* m_pTriArray = nullptr;

  // next ray ID to assign to a traced ray
  std::uint32_t m_nNextRayID;

  //****************** Ray traversal state ***********************

  // this is mutable state that changes during ray traversal.  If we every want
  // to make this thing multi-threaded, we will need to allocate this stuff
  // seperately for each worker thread

  // mailboxes, one per triangle.  These store the ray-id for the last ray to be
  // tested with this triangle
  std::uint32_t* m_pMailboxes = nullptr;

  // flags to indicate whether or not a triangle is back-facing (TOOTLE
  // SPECIFIC)
  bool* m_bBackFacing = nullptr;
};

#endif
