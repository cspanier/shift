/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef JRT_MESH_H
#define JRT_MESH_H

#include <cstdint>
#include "JRTCommon.h"

class JRTTriangle;

class JRTCSGNode;
class JRTSurfaceShader;
class JRTPhotonShader;
class JRTBoundingBox;

class JRTMeshAttributes
{
public:
  JRTMeshAttributes()
  {
  }

  JRTSurfaceShader* pShader{nullptr};
  JRTPhotonShader* pPhotonShader;
  JRTCSGNode* pCSGNode{nullptr};
  bool bCastsShadows{true};
  bool bCastsCaustics{false};
};

class JRTMesh
{
public:
  /// Factory method for creating meshes
  static JRTMesh* CreateMesh(const Vec3f* pPositions, const Vec3f* pNormals,
                             std::uint32_t nVertices,
                             std::uint32_t nTriangleCount,
                             const std::uint32_t* pIndices);

  /// Mesh constructor
  JRTMesh();

  /// Mesh destructor
  ~JRTMesh();

  /// Transforms the mesh and re-preprocesses the triangles
  void Transform(const Matrix4f& rXForm, const Matrix4f& rInverse);

  /// Returns the number of triangles in the mesh
  std::uint32_t GetTriangleCount() const
  {
    return m_nTriangleCount;
  }

  /// Accessor for the array of triangles
  inline const JRTTriangle* GetTriangles() const
  {
    return m_Triangles.data();
  }

  void GetInterpolants(std::uint32_t nTriIndex, const float barycentrics[3],
                       Vec3f* pNormal, Vec2f* pUV) const;

  const JRTMeshAttributes& GetAttributes() const
  {
    return m_attributes;
  }

  void SetAttributes(const JRTMeshAttributes& rAttribs)
  {
    m_attributes = rAttribs;
  }

  /// Removes a particular triangle from the mesh
  void RemoveTriangle(std::uint32_t nTri);

  JRTBoundingBox ComputeBoundingBox() const;

  const Vec3f& GetFaceNormal(std::uint32_t nTri) const
  {
    return m_FaceNormals[nTri];
  }

  const Vec3f& GetVertex(std::uint32_t i) const
  {
    return m_Positions[i];
  }
  void SetVertex(std::uint32_t i, const Vec3f& v)
  {
    m_Positions[i] = v;
  }

private:
  std::vector<Vec3f> m_Positions;
  std::vector<Vec3f> m_Normals;
  std::vector<Vec2f> m_UVs;

  /// Pre-computed array of face normals.  This is used only if no vertex
  /// normals are provided
  std::vector<Vec3f> m_FaceNormals;

  std::vector<JRTTriangle> m_Triangles;

  std::uint32_t m_nTriangleCount{0};
  std::uint32_t m_nVertexCount{0};

  JRTMeshAttributes m_attributes;

  // Disallow copy constructor, as m_Triangles contains pointers into the
  // other data structures
  JRTMesh(const JRTMesh&) = delete;
  JRTMesh& operator=(const JRTMesh&) = delete;
};

class JRTTriangle
{
public:
  /// Returns the first vertex
  inline const Vec3f& GetV1() const
  {
    return *(const Vec3f*)m_pV1;
  }

  /// Returns the second vertex
  inline const Vec3f& GetV2() const
  {
    return *(const Vec3f*)m_pV2;
  }

  /// Returns the third vertex
  inline const Vec3f& GetV3() const
  {
    return *(const Vec3f*)m_pV3;
  }

  /// Returns the mesh which owns this triangle
  inline JRTMesh* GetMesh() const
  {
    return m_pMesh;
  }

  /// Returns the index of this triangle in its parent's triangle array
  inline std::uint32_t GetIndexInMesh() const
  {
    return (std::uint32_t)(this - m_pMesh->GetTriangles());
  }

  inline const Vec3f& GetNormal() const
  {
    return m_pMesh->GetFaceNormal(GetIndexInMesh());
  }

private:
  friend class JRTMesh;

  /// Pointer to first vertex position
  const float* m_pV1 = nullptr;

  /// Pointer to second vertex position
  const float* m_pV2 = nullptr;

  /// Pointer to third vertex position
  const float* m_pV3 = nullptr;

  /// Pointer to the mesh
  JRTMesh* m_pMesh = nullptr;
};

#endif
