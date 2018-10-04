/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#include "TootlePCH.h"
#include "JRTCommon.h"
#include "JRTMesh.h"
#include "JRTBoundingBox.h"

///  A mesh must have an array of positions and connectivity.  It may also have
///  per-vertex normals If per-vertex normals are omitted, then per-face normals
///  are used instead.
///
///   The mesh does NOT delete the input arrays
///
/// \param rPositions
///    Array of vertex positions
/// \param pFaceNormals
///    Array of face normals.  Must be same length as nTriangleCount
/// \param nTriangleCount
///     Number of triangles
/// \param pIndices
///     An array of 3*nTriangleCount vertex indices
JRTMesh* JRTMesh::CreateMesh(const Vec3f* pPositions, const Vec3f* pNormals,
                             std::uint32_t nVertices,
                             std::uint32_t nTriangleCount,
                             const std::uint32_t* pIndices)
{
  auto* pMesh = new JRTMesh();

  pMesh->m_nVertexCount = nVertices;
  pMesh->m_nTriangleCount = nTriangleCount;

  // copy vertex positions
  pMesh->m_Positions.assign(pPositions, pPositions + nVertices);

  // copy normals
  pMesh->m_FaceNormals.assign(pNormals, pNormals + nTriangleCount);

  // create triangles
  pMesh->m_Triangles.resize(nTriangleCount);

  for (std::uint32_t i = 0; i < nTriangleCount; i++)
  {
    pMesh->m_Triangles[i].m_pMesh = pMesh;

    JRT_ASSERT(pIndices[0] < nVertices);
    JRT_ASSERT(pIndices[1] < nVertices);
    JRT_ASSERT(pIndices[2] < nVertices);

    pMesh->m_Triangles[i].m_pV1 =
      reinterpret_cast<const float*>(&pMesh->m_Positions[pIndices[0]]);
    pMesh->m_Triangles[i].m_pV2 =
      reinterpret_cast<const float*>(&pMesh->m_Positions[pIndices[1]]);
    pMesh->m_Triangles[i].m_pV3 =
      reinterpret_cast<const float*>(&pMesh->m_Positions[pIndices[2]]);

    pIndices += 3;
  }

  return pMesh;
}

JRTMesh::JRTMesh()
{
}

JRTMesh::~JRTMesh() = default;

void JRTMesh::Transform(const Matrix4f& rXForm, const Matrix4f& rInverse)
{
  // transform positions
  for (std::uint32_t i = 0; i < m_nVertexCount; i++)
  {
    TransformPoint(&m_Positions[i], &rXForm, &m_Positions[i]);
  }

  Matrix4f inverseTranspose = rInverse.Transpose();

  // transform normals
  if (!m_Normals.empty())
  {
    for (std::uint32_t i = 0; i < m_nVertexCount; i++)
    {
      ALIGN16 Vec3f tmp = m_Normals[i];
      TransformVector(&tmp, &inverseTranspose, &m_Normals[i]);
      m_Normals[i] = Normalize(m_Normals[i]);
    }
  }
  else
  {
    // transform face normals
    for (std::uint32_t i = 0; i < m_nTriangleCount; i++)
    {
      ALIGN16 Vec3f tmp = m_FaceNormals[i];
      TransformVector(&tmp, &inverseTranspose, &m_FaceNormals[i]);
      m_FaceNormals[i] = tmp;
    }
  }
}

void JRTMesh::GetInterpolants(std::uint32_t nTriIndex,
                              const float barycentrics[], Vec3f* pNormal,
                              Vec2f* /*pUV*/) const
{
  const std::uint32_t nIndex1 =
    (std::uint32_t)(m_Triangles[nTriIndex].m_pV1 -
                    (const float*)m_Positions.data()) /
    3;
  const std::uint32_t nIndex2 =
    (std::uint32_t)(m_Triangles[nTriIndex].m_pV2 -
                    (const float*)m_Positions.data()) /
    3;
  const std::uint32_t nIndex3 =
    (std::uint32_t)(m_Triangles[nTriIndex].m_pV3 -
                    (const float*)m_Positions.data()) /
    3;

  if (!m_Normals.empty())
  {
    *pNormal =
      Normalize(BarycentricLerp3f(m_Normals[nIndex1], m_Normals[nIndex2],
                                  m_Normals[nIndex3], barycentrics));
  }
  else
  {
    *pNormal = Normalize(m_FaceNormals[nTriIndex]);
  }
}

void JRTMesh::RemoveTriangle(std::uint32_t nTri)
{
  if (nTri <= m_nTriangleCount)
  {
    // swap this triangle with the last one in the mesh
    JRTTriangle tmpTri = m_Triangles[m_nTriangleCount - 1];
    m_Triangles[m_nTriangleCount - 1] = m_Triangles[nTri];
    m_Triangles[nTri] = tmpTri;

    // swap face normals if there are any
    if (m_Normals.empty())
    {
      Vec3f tmpNorm = m_FaceNormals[m_nTriangleCount - 1];
      m_FaceNormals[m_nTriangleCount - 1] = m_FaceNormals[nTri];
      m_FaceNormals[nTri] = tmpNorm;
    }

    m_nTriangleCount--;
  }
}

JRTBoundingBox JRTMesh::ComputeBoundingBox() const
{
  return JRTBoundingBox((const float*)this->m_Positions.data(), m_nVertexCount);
}
