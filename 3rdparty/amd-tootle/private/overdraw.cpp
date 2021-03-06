/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#include "TootlePCH.h"
#include "overdraw.h"
#include "soup.h"

#include "TootleRaytracer.h"
#include <cstdint>

//=================================================================================================================================
//
//          Internal state
//
//=================================================================================================================================

/// The current soup being optimized
static Soup* s_pSoup = nullptr;

//=================================================================================================================================
//
//          Internal helper functions
//
//=================================================================================================================================
// compute face normals for the mesh.
static std::vector<float> ComputeFaceNormals(const float* pfVB,
                                             const unsigned int* pnIB,
                                             unsigned int nFaces);

//=================================================================================================================================
/// Computes the overdraw graph using the ray tracing implementation
///
/// \param pViewpoints         Array of viewpoints to use for overdraw
/// computation. \param nViewpoints         Size of the viewpoint array \param
/// bCullCCW            Specify true to cull CCW faces, otherwise cull CW faces.
/// \param rClusters           Array identifying the cluster for each face.
/// Faces are assumed sorted by cluster \param nClusters           The number of
/// clusters in rClusters. \param rGraphOut           An array of edges that
/// will contain the overdraw graph \return TOOTLE_OK, or TOOTLE_OUT_OF_MEMORY
//=================================================================================================================================
TootleResult ODComputeGraphRaytrace(const float* pViewpoints,
                                    unsigned int nViewpoints, bool bCullCCW,
                                    const std::vector<int>& rClusters,
                                    std::uint32_t nClusters,
                                    std::vector<t_edge>& rGraphOut)
{
  std::vector<Vector3> tn;

  if (s_pSoup->ComputeTriNormals(tn) == 0)
  {
    return TOOTLE_OUT_OF_MEMORY;
  }

  // initialize per-cluster overdraw table
  TootleOverdrawTable fullgraph(nClusters);

  for (int i = 0; i < (int)nClusters; i++)
  {
    fullgraph[i].resize(nClusters);

    for (int j = 0; j < (int)nClusters; j++)
    {
      fullgraph[i][j] = 0;
    }
  }

  // initialize the ray tracer
  TootleRaytracer tr;
  const float* pVB = &s_pSoup->v(0)[0];
  const auto* pIB = (const std::uint32_t*)&s_pSoup->t(0)[0];
  const float* pFaceNormals = &tn[0][0];
  const auto nVertices = (std::uint32_t)s_pSoup->v().size();
  const auto nFaces = (std::uint32_t)s_pSoup->t().size();

  if (!tr.Init(pVB, pIB, pFaceNormals, nVertices, nFaces,
               (const std::uint32_t*)&rClusters[0]))
  {
    return TOOTLE_OUT_OF_MEMORY;
  }

  // generate the per-cluster overdraw table
  if (!tr.CalculateOverdraw(pViewpoints, nViewpoints,
                            TOOTLE_RAYTRACE_IMAGE_SIZE, bCullCCW, &fullgraph))
  {
    return TOOTLE_OUT_OF_MEMORY;
  }

  // clean up the mess
  tr.Cleanup();

  // extract a directed graph from the overdraw table
  for (int i = 0; i < (int)nClusters; i++)
  {
    for (int j = 0; j < (int)nClusters; j++)
    {
      if (fullgraph[i][j] > fullgraph[j][i])
      {
        t_edge t;
        t.from = i;
        t.to = j;
        t.cost = fullgraph[i][j] - fullgraph[j][i];

        rGraphOut.push_back(t);
      }
    }
  }

  return TOOTLE_OK;
}

//=================================================================================================================================
//
//          Exported functions
//
//=================================================================================================================================

//=================================================================================================================================
/// Initializes the overdraw computation module with a set of viewpoints
/// \return TOOTLE_OK if successful.  May return other results if errors occur
//=================================================================================================================================

TootleResult ODInit()
{
  return TOOTLE_OK;
}

//=================================================================================================================================
/// \return True if ODInit was called successfully, false otherwise
//=================================================================================================================================
bool ODIsInitialized()
{
  return true;
}

//=================================================================================================================================
/// Sets the triangle soup that will be used for the overdraw computations
/// It is not necessary to call this method again when the contents of the soup
/// changes.  This will be done automatically before computing overdraw
///
/// \param pSoup         The soup to use for overdraw computation
/// \param eFrontWinding The front face winding for the soup
/// \return TOOTLE_OK
///         TOOTLE_INTERNAL_ERROR if ODInit() hasn't been called,
///         TOOTLE_3D_API_ERROR if VB/IB allocation fails
//=================================================================================================================================
TootleResult ODSetSoup(Soup* pSoup, TootleFaceWinding eFrontWinding)
{
  s_pSoup = pSoup;

  // Unused parameter in this case
  (void)eFrontWinding;

  return TOOTLE_OK;
}

//=================================================================================================================================
/// Computes the object overdraw for the triangle soup the ray tracing
/// implementation
///
/// \param pfVB           A pointer to the vertex buffer.  The pointer pVB must
/// point to the vertex position.  The vertex
///                        position must be a 3-component floating point value
///                        (X,Y,Z).
/// \param pnIB           The index buffer.  Must be a triangle list.
/// \param nVertices      The number of vertices. This must be non-zero and less
/// than TOOTLE_MAX_VERTICES. \param nFaces         The number of indices.  This
/// must be non-zero and less than TOOTLE_MAX_FACES. \param pViewpoints    The
/// viewpoints to use to measure overdraw \param nViewpoints    The number of
/// viewpoints in the array \param bCullCCW       Set to true to cull CCW faces,
/// otherwise cull CW faces. \param fODAvg         (Output) Average overdraw
/// \param fODMax         (Output) Maximum overdraw
/// \return TOOTLE_OK, TOOTLE_OUT_OF_MEMORY
//=================================================================================================================================
TootleResult ODObjectOverdrawRaytrace(
  const float* pfVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, const float* pViewpoints, unsigned int nViewpoints,
  bool bCullCCW, float& fAvgOD, float& fMaxOD)
{
  assert(pfVB);
  assert(pnIB);

  const std::vector<float> faceNormals = ComputeFaceNormals(pfVB, pnIB, nFaces);

  TootleRaytracer tr;

  if (!tr.Init(pfVB, pnIB, faceNormals.data(), nVertices, nFaces, nullptr))
  {
    return TOOTLE_OUT_OF_MEMORY;
  }

  // generate the per-cluster overdraw table
  if (!tr.MeasureOverdraw(pViewpoints, nViewpoints, TOOTLE_RAYTRACE_IMAGE_SIZE,
                          bCullCCW, fAvgOD, fMaxOD))
  {
    return TOOTLE_OUT_OF_MEMORY;
  }

  // clean up the mess
  tr.Cleanup();

  return TOOTLE_OK;
}

//=================================================================================================================================
/// Calculate face normals for the mesh.
///
/// \param pfVB            A pointer to the vertex buffer.  The pointer pVB must
/// point to the vertex position.  The vertex
///                         position must be a 3-component floating point value
///                         (X,Y,Z).
/// \param pnIB            The index buffer.  Must be a triangle list.
/// \param nFaces          The number of indices.  This must be non-zero and
/// less than TOOTLE_MAX_FACES. \param pFaceNormals    The output face normals.
/// May not be NULL.  Need to be pre-allocated of size 3*nFaces. \param
/// bFrontCWWinding Specify true if the mesh has CW front face winding, false
/// otherwise.
///
/// \return void
//=================================================================================================================================
std::vector<float> ComputeFaceNormals(const float* pfVB,
                                      const unsigned int* pnIB,
                                      unsigned int nFaces)
{
  assert(pnIB);

  // triangle index
  unsigned int nFirst;
  unsigned int nSecond;
  unsigned int nThird;

  std::vector<float> result(nFaces * 3);

  for (unsigned int i = 0; i < nFaces; i++)
  {
    nFirst = pnIB[3 * i];
    nSecond = pnIB[3 * i + 1];
    nThird = pnIB[3 * i + 2];

    const Vector3 p0(pfVB[3 * nFirst], pfVB[3 * nFirst + 1],
                     pfVB[3 * nFirst + 2]);
    const Vector3 p1(pfVB[3 * nSecond], pfVB[3 * nSecond + 1],
                     pfVB[3 * nSecond + 2]);
    const Vector3 p2(pfVB[3 * nThird], pfVB[3 * nThird + 1],
                     pfVB[3 * nThird + 2]);

    const Vector3 a = p0 - p1, b = p1 - p2;
    const Vector3 vNormal = Normalize(Cross(a, b));

    result[3 * i] = vNormal[0];
    result[3 * i + 1] = vNormal[1];
    result[3 * i + 2] = vNormal[2];
  }

  return result;
}

//=================================================================================================================================
/// \param pViewpoints         Array of viewpoints to use for overdraw
/// computation. \param nViewpoints         Size of the viewpoint array \param
/// bCullCCW            Specify true to cull CCW faces, otherwise cull CW faces.
/// \param rClusters           Array identifying the cluster for each face.
/// Faces are assumed sorted by cluster \param rClusterStart       Array giving
/// the index of the first triangle in each cluster.  The size should be one
/// plus the number
///                             of clusters.  The value of the last element of
///                             this array is the number of triangles in the
///                             mesh
/// \param rGraphOut           An array of edges that will contain the overdraw
/// graph \return TOOTLE_OK, TOOTLE_INTERNAL_ERROR, TOOTLE_3D_API_ERROR,
/// TOOTLE_OUT_OF_MEMORY
//=================================================================================================================================
TootleResult ODOverdrawGraph(const float* pViewpoints, unsigned int nViewpoints,
                             bool bCullCCW, const std::vector<int>& rClusters,
                             const std::vector<int>& rClusterStart,
                             std::vector<t_edge>& rGraphOut)
{

  if (s_pSoup == nullptr)
  {
    // ODInit has not been called, or soup isn't set
    return TOOTLE_INTERNAL_ERROR;
  }

  // sanity check
  if (rClusters.size() != s_pSoup->t().size())
  {
    return TOOTLE_INTERNAL_ERROR;
  }

  return ODComputeGraphRaytrace(pViewpoints, nViewpoints, bCullCCW, rClusters,
                                (std::uint32_t)rClusterStart.size() - 1,
                                rGraphOut);
}

//=================================================================================================================================
/// Cleans up any memory allocated by the overdraw module
//=================================================================================================================================

void ODCleanup()
{
}
