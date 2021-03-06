/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#include <cassert>
#include "TootlePCH.h"

#include "soup.h"
#include "clustering.h"
#include "error.h"
#include "overdraw.h"

#include "tootlelib.h"
#include "triorder.h"
#include "viewpoints.h"
#include "Stripifier.h"

#define AMD_TOOTLE_API_FUNCTION_BEGIN \
  try                                 \
  {
#define AMD_TOOTLE_API_FUNCTION_END \
  }                                 \
  catch (const std::bad_alloc&)     \
  {                                 \
    return TOOTLE_OUT_OF_MEMORY;    \
  }                                 \
  catch (const std::exception&)     \
  {                                 \
    return TOOTLE_INTERNAL_ERROR;   \
  }                                 \
  catch (...)                       \
  {                                 \
    return TOOTLE_INTERNAL_ERROR;   \
  }

//=================================================================================================================================
//
//          Static functions
//
//=================================================================================================================================

// compute face mapping from one index to another index
static TootleResult FindFaceMappingFromIndex(const unsigned int* pnIB,
                                             const unsigned int* pnIB2,
                                             unsigned int nIndex,
                                             unsigned int nFaces,
                                             unsigned int* pnFaceMapOut);

// optimize vertex cache using triangle strips (not exactly triangle strips but
// list like triangle strips)
static TootleResult TootleOptimizeVCacheLStrips(
  const unsigned int* pnIB, unsigned int nFaces, unsigned int nVertices,
  unsigned int nCacheSize, unsigned int* pnIBOut, unsigned int* pnFaceRemapOut);

// optimize vertex cache using tipsy algorithm from SIGGRAPH 2007
static TootleResult TootleOptimizeVCacheTipsy(
  const unsigned int* pnIB, unsigned int nFaces, unsigned int nVertices,
  unsigned int nCacheSize, unsigned int* pnIBOut, unsigned int* pnFaceRemapOut);

// optimize overdraw by reordering clusters based on Direct3D rendering
static TootleResult TootleOptimizeOverdrawDirect3DAndRaytrace(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, const float* pfViewpoint,
  unsigned int nViewpoints, TootleFaceWinding eFrontWinding,
  const unsigned int* pnFaceClusters, unsigned int* pnIBOut,
  unsigned int* pnClusterRemapOut);

// measure overdraw using software rendering via raytracing
static TootleResult TootleMeasureOverdrawRaytrace(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, const float* pfViewpoint,
  unsigned int nViewpoints, TootleFaceWinding eFrontWinding, float* pfAvgODOut,
  float* pfMaxODOut);

// converting the cluster array IDs from the full (v1.2 tootle) format to a
// compact format (v2.0 tootle).
static TootleResult ConvertClusterArrayFromFullToCompact(unsigned int* pnID,
                                                         unsigned int nFaces);

// converting the cluster array IDs from the compact (v2.0 tootle) format to a
// full format (v1.2 tootle).
static TootleResult ConvertClusterArrayFromCompactToFull(unsigned int* pnID,
                                                         unsigned int nFaces);

// check whether the cluster array IDs is of type full format (v1.2 tootle).
static bool IsClusterArrayFullFormat(const unsigned int* pnID,
                                     unsigned int nFaces);

// check whether the cluster array IDs is of type compact format (v2.0 tootle).
static bool IsClusterArrayCompactFormat(const unsigned int* pnID,
                                        unsigned int nFaces);

TootleResult TOOTLE_DLL TootleInit()
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // initialize overdraw module
  if (!ODIsInitialized())
  {
    return ODInit();
  }

  return TOOTLE_OK;

  AMD_TOOTLE_API_FUNCTION_END
}

static TootleResult FindFaceMappingFromIndex(const unsigned int* pnIB,
                                             const unsigned int* pnIB2,
                                             const unsigned int nIndex,
                                             const unsigned int nFaces,
                                             unsigned int* pnFaceMapOut)
{
  // sanity checks
  assert(pnIB);
  assert(pnIB2);
  assert(pnFaceMapOut);

  if (nIndex == 0 || nIndex > TOOTLE_MAX_FACES)
  {
    errorf(("FindFaceMappingFromIndex: nIndex is invalid"));
    return TOOTLE_INVALID_ARGS;
  }

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("FindFaceMappingFromIndex: nFaces is invalid"));
    return TOOTLE_INVALID_ARGS;
  }

  const unsigned int nVert1 = pnIB[3 * nIndex];
  const unsigned int nVert2 = pnIB[3 * nIndex + 1];
  const unsigned int nVert3 = pnIB[3 * nIndex + 2];

  for (std::uint32_t i = 0; i < nFaces; i++)
  {
    if (pnIB2[3 * i] == nVert1 && pnIB2[3 * i + 1] == nVert2 &&
        pnIB2[3 * i + 2] == nVert3)
    {
      *pnFaceMapOut = i;

      return TOOTLE_OK;
    }
  }

  return TOOTLE_INTERNAL_ERROR;
}

TootleResult TOOTLE_DLL TootleOptimizeVCache(
  const unsigned int* pnIB, unsigned int nFaces, unsigned int nVertices,
  unsigned int nCacheSize, unsigned int* pnIBOut, unsigned int* pnFaceRemapOut,
  TootleVCacheOptimizer eVCacheOptimizer)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks on the input parameters
  assert(pnIB);

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleOptimizeVCache: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleOptimizeVCache: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nCacheSize == 0)
  {
    errorf(("TootleOptimizeVCache: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  unsigned int* pnIBOutTmp = pnIBOut;

  // if source and destination buffer are the same, we need a local copy
  if (pnIBOut != nullptr)
  {
    if (pnIB == pnIBOut)
    {
      pnIBOutTmp = new unsigned int[3 * nFaces];
    }
  }

  TootleResult result;

  // based on eVCacheOptimizer, we run the appropriate algorithm
  switch (eVCacheOptimizer)
  {
  case TOOTLE_VCACHE_AUTO:

    // This is the default behavior in the old tootle library (<= version 1.2).
    // It calls Tstrips algorithm if the vertex cache size is less than 7,
    // otherwise it used to call the Direct3D version (this is now changed to
    // Tipsy for this version).
    if (nCacheSize <= 6)
    {
      result = TootleOptimizeVCacheLStrips(pnIB, nFaces, nVertices, nCacheSize,
                                           pnIBOutTmp, pnFaceRemapOut);
    }
    else
    {
      result = TootleOptimizeVCacheTipsy(pnIB, nFaces, nVertices, nCacheSize,
                                         pnIBOutTmp, pnFaceRemapOut);
    }

    break;

  case TOOTLE_VCACHE_DIRECT3D:
    fprintf(stderr,
            "TootleOptimizeVCache: No Direct3D support for this version.\n");
    result = TOOTLE_INTERNAL_ERROR;
    break;

  case TOOTLE_VCACHE_LSTRIPS:
    result = TootleOptimizeVCacheLStrips(pnIB, nFaces, nVertices, nCacheSize,
                                         pnIBOutTmp, pnFaceRemapOut);
    break;

  case TOOTLE_VCACHE_TIPSY:
    result = TootleOptimizeVCacheTipsy(pnIB, nFaces, nVertices, nCacheSize,
                                       pnIBOutTmp, pnFaceRemapOut);
    break;

  default:
    errorf(("TootleOptimizeVCache: Invalid eVCacheOptimizer"));
    result = TOOTLE_INVALID_ARGS;
    break;
  };

  // copy the output back
  if ((pnIBOut != nullptr) && pnIBOutTmp != pnIBOut)
  {
    memcpy(pnIBOut, pnIBOutTmp, nFaces * 3 * sizeof(unsigned int));
    delete[] pnIBOutTmp;
  }

  return result;

  AMD_TOOTLE_API_FUNCTION_END
}

static TootleResult TootleOptimizeVCacheLStrips(
  const unsigned int* pnIB, unsigned int nFaces, unsigned int nVertices,
  unsigned int nCacheSize, unsigned int* pnIBOut, unsigned int* pnFaceRemapOut)
{
  // sanity checks
  assert(pnIB);

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleOptimizeVCacheLStrips: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleOptimizeVCacheLStrips: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nCacheSize == 0)
  {
    errorf(("TootleOptimizeVCacheLStrips: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  auto* pnStripResult = new unsigned int[nFaces * 3];

  Stripifier::Process(pnIB, nFaces, pnStripResult, pnFaceRemapOut);

  // re-order faces
  if (pnIBOut != nullptr)
  {
    for (std::uint32_t i = 0; i < nFaces * 3; i++)
    {
      pnIBOut[i] = pnStripResult[i];
    }
  }

  TootleResult result = TOOTLE_OK;

  delete[] pnStripResult;

  return result;
}

static TootleResult TootleOptimizeVCacheTipsy(
  const unsigned int* pnIB, unsigned int nFaces, unsigned int nVertices,
  unsigned int nCacheSize, unsigned int* pnIBOut, unsigned int* pnFaceRemapOut)
{
  // sanity checks
  assert(pnIB);

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleOptimizeVCacheTipsy: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleOptimizeVCacheTipsy: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nCacheSize == 0)
  {
    errorf(("TootleOptimizeVCacheTipsy: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  FanVertOptimizeVCacheOnly((int*)pnIB, (int*)pnIBOut, nVertices, nFaces,
                            nCacheSize, nullptr, nullptr, nullptr);

  // if the face remapping is requested, compute it for the caller.
  // Perhaps, this information should be generated as the indices get built in
  // FanVertOptimizeVCache to be
  //  more efficient.
  if (pnFaceRemapOut != nullptr)
  {
    std::uint32_t nFaceMap;
    TootleResult result;

    for (std::uint32_t i = 0; i < nFaces; i++)
    {
      result = FindFaceMappingFromIndex(pnIB, pnIBOut, i, nFaces, &nFaceMap);

      if (result != TOOTLE_OK)
      {
        return result;
      }

      pnFaceRemapOut[i] = nFaceMap;
    }
  }

  return TOOTLE_OK;
}

TootleResult TOOTLE_DLL TootleClusterMesh(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, unsigned int nTargetClusters,
  unsigned int* pnClusteredIBOut, unsigned int* pnFaceClustersOut,
  unsigned int* pnFaceRemapOut)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pVB);
  assert(pnIB);
  assert(pnClusteredIBOut);
  assert(pnFaceClustersOut);

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleClusterMesh: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleClusterMesh: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVBStride < 3 * sizeof(float))
  {
    errorf(("TootleClusterMesh: nVBStride less than 3*sizeof(float)"));

    return TOOTLE_INVALID_ARGS;
  }

  // because Pedro's clustering implementation is so heavily tied to the
  // Soup/Mesh classes, it's easier just to construct a soup here eventually we
  // should re-work the code to avoid this redundant copying.

  Soup soup;

  if (!MakeSoup(pVB, pnIB, nVertices, nFaces, nVBStride, &soup))
  {
    return TOOTLE_OUT_OF_MEMORY;
  }

  // cluster the mesh
  std::uint32_t nClusters = nTargetClusters;
  std::vector<int> clusterIDs;
  ClusterResult result = Cluster(&soup, nClusters, clusterIDs);

  switch (result)
  {
  case CLUSTER_OK:
    break;

  case CLUSTER_OUT_OF_MEMORY:
    return TOOTLE_OUT_OF_MEMORY;

  default:
    return TOOTLE_INTERNAL_ERROR;
  }

  // create an array to hold the face re-mapping
  // use the output array if the user provided one, otherwise, allocate a new
  // one
  std::uint32_t* pnRemap = pnFaceRemapOut;

  if (pnRemap == nullptr)
  {
    pnRemap = new std::uint32_t[nFaces];
  }

  if (!SortFacesByCluster(soup, clusterIDs, pnRemap))
  {
    return TOOTLE_OUT_OF_MEMORY;
  }

  if (pnRemap != pnFaceRemapOut)
  {
    // don't need this temporary array anymore
    delete[] pnRemap;
  }

  // copy clustered mesh to output array

  // once again, we're memcpying int to unsigned int.
  // This should be ok because, if clustering worked, all cluster IDs will be
  // positive We should still fix it eventually though
  memcpy(pnFaceClustersOut, &clusterIDs[0], sizeof(int) * clusterIDs.size());
  memcpy(pnClusteredIBOut, &soup.t(0), sizeof(int) * soup.t().size() * 3);

  // Append the number of cluster to the last element of the array.
  // This is to ensure we can mix and match the full (old tootle) and compact
  // (new tootle) format.
  pnFaceClustersOut[nFaces] = 1 + pnFaceClustersOut[nFaces - 1];

  return TOOTLE_OK;

  AMD_TOOTLE_API_FUNCTION_END
}

TootleResult TOOTLE_DLL TootleFastOptimizeVCacheAndClusterMesh(
  const unsigned int* pnIB, unsigned int nFaces, unsigned int nVertices,
  unsigned int nCacheSize, unsigned int* pnIBOut, unsigned int* pnClustersOut,
  unsigned int* pnNumClustersOut, float fAlpha)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pnIB);
  assert(pnIBOut);
  assert(pnClustersOut);

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleFastOptimizeVCacheAndClusterMesh: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(
      ("TootleFastOptimizeVCacheAndClusterMesh: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nCacheSize == 0)
  {
    errorf(("TootleOptimizeVCacheAndClusterMesh: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  unsigned int* pnOutput = pnIBOut;

  // if source and destination buffer are the same, we need a local copy
  if (pnIBOut != nullptr)
  {
    if (pnIB == pnIBOut)
    {
      pnOutput = new unsigned int[3 * nFaces];
    }
  }

  float fACMR;
  unsigned int* pnClustersOutTmp;
  unsigned int pnNumClustersOutTmp;

  pnClustersOutTmp = new unsigned int[nFaces + 1];

  // OPTIMIZE VERTEX CACHE
  fACMR = FanVertOptimizeVCacheOnly(
    (int*)pnIB, (int*)pnOutput, nVertices, nFaces, nCacheSize, nullptr,
    (int*)pnClustersOutTmp, (int*)&pnNumClustersOutTmp);

  // copy the output back
  if ((pnIBOut != nullptr) && pnOutput != pnIBOut)
  {
    memcpy(pnIBOut, pnOutput, nFaces * 3 * sizeof(unsigned int));
    delete[] pnOutput;
  }

  float fLambda;
  fLambda = fAlpha + (1.0f - fAlpha) * fACMR;

  // PERFORM LINEAR CLUSTERING based on the output of Vertex Cache Optimization
  // algorithm (hard boundaries) and
  //  fLambda (soft boundaries).
  FanVertOptimizeClusterOnly((int*)pnIBOut, nVertices, nFaces, nCacheSize,
                             fLambda, (int*)pnClustersOutTmp,
                             pnNumClustersOutTmp, (int*)pnClustersOut,
                             (int*)pnNumClustersOut, nullptr);

  delete[] pnClustersOutTmp;

  return TOOTLE_OK;

  AMD_TOOTLE_API_FUNCTION_END
}

TootleResult TOOTLE_DLL TootleOptimizeOverdraw(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, const float* pfViewpoint,
  unsigned int nViewpoints, TootleFaceWinding eFrontWinding,
  const unsigned int* pnFaceClusters, unsigned int* pnIBOut,
  unsigned int* pnClusterRemapOut)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pVB);
  assert(pnIB);
  assert(pnFaceClusters);

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleOptimizeOverdraw: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleOptimizeOverdraw: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVBStride < 3 * sizeof(float))
  {
    errorf(("TootleOptimizeOverdraw: nVBStride less than 3*sizeof(float)"));

    return TOOTLE_INVALID_ARGS;
  }

  // make sure that they're not being stupid and passing us bad enum values
  if (eFrontWinding != TOOTLE_CCW && eFrontWinding != TOOTLE_CW)
  {
    errorf(("TootleOptimizeOverdraw: Invalid face winding."));

    return TOOTLE_INVALID_ARGS;
  }

  return TootleOptimizeOverdrawDirect3DAndRaytrace(
    pVB, pnIB, nVertices, nFaces, nVBStride, pfViewpoint, nViewpoints,
    eFrontWinding, pnFaceClusters, pnIBOut, pnClusterRemapOut);

  AMD_TOOTLE_API_FUNCTION_END
}

static TootleResult TootleOptimizeOverdrawDirect3DAndRaytrace(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, const float* pfViewpoint,
  unsigned int nViewpoints, TootleFaceWinding eFrontWinding,
  const unsigned int* pnFaceClusters, unsigned int* pnIBOut,
  unsigned int* pnClusterRemapOut)
{
  // sanity checks
  assert(pVB);
  assert(pnIB);
  assert(pnFaceClusters);

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleOptimizeOverdrawDirect3D: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleOptimizeOverdrawDirect3D: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVBStride < 3 * sizeof(float))
  {
    errorf(("TootleOptimizeOverdraw: nVBStride less than 3*sizeof(float)"));

    return TOOTLE_INVALID_ARGS;
  }

  // make sure that they're not being stupid and passing us bad enum values
  if (eFrontWinding != TOOTLE_CCW && eFrontWinding != TOOTLE_CW)
  {
    errorf(("TootleOptimizeOverdrawDirect3D: Invalid face winding."));
    return TOOTLE_INVALID_ARGS;
  }

  // perform check whether pnFaceCluster is in the full format.
  if (!IsClusterArrayFullFormat(pnFaceClusters, nFaces))
  {
    ConvertClusterArrayFromCompactToFull((unsigned int*)pnFaceClusters, nFaces);
  }

  // make sure that cluster array is sorted, as required
  bool bOrdered = true;

  if (pnFaceClusters[0] != 0)
  {
    bOrdered = false;
  }

  for (std::uint32_t i = 1; i < nFaces; i++)
  {
    int x = pnFaceClusters[i] - pnFaceClusters[i - 1];

    if (x < 0 || x > 1)
    {
      bOrdered = false;
      break;
    }
  }

  if (!bOrdered)
  {
    errorf(("TootleOptimizeOverdrawDirect3D: Cluster array is not ordered."));

    return TOOTLE_INVALID_ARGS;
  }

  // if there is only one cluster, do nothing, just pass through
  // if we don't do this, various pieces of code will break
  std::uint32_t nClusters = 1 + pnFaceClusters[nFaces - 1];

  if (nClusters == 1)
  {
    if (pnClusterRemapOut != nullptr)
    {
      *pnClusterRemapOut = 0;
    }

    if (pnIBOut != nullptr)
    {
      memcpy(pnIBOut, pnIB, sizeof(unsigned int) * nFaces * 3);
    }

    return TOOTLE_OK;
  }

  // use default viewpoints if they were omitted
  if (pfViewpoint == nullptr)
  {
    pfViewpoint = pDefaultViewpoint;
    nViewpoints = nDefaultViewpoints;
  }

  // make some soup
  Soup soup;

  if (!MakeSoup(pVB, pnIB, nVertices, nFaces, nVBStride, &soup))
  {
    return TOOTLE_OUT_OF_MEMORY;
  }

  // give soup to overdraw module
  TootleResult result;

  result = ODSetSoup(&soup, eFrontWinding);

  if (result != TOOTLE_OK)
  {
    return result;
  }

  // build one of std::vector<> things for the cluster array
  std::vector<int> cluster;

  for (std::uint32_t i = 0; i < nFaces; i++)
  {
    cluster.push_back(pnFaceClusters[i]);
  }

  // build array containing the index of the first face in each cluster
  std::vector<int> ClusterStart;
  std::uint32_t iLast = nClusters;

  for (std::uint32_t i = 0; i < nFaces; i++)
  {
    if (pnFaceClusters[i] != iLast)
    {
      iLast = pnFaceClusters[i];

      ClusterStart.push_back(i);
    }
  }

  // last element needs to contain the number of faces in the mesh. Various
  // pieces of code depend on this
  ClusterStart.push_back(nFaces);

  // compute the overdraw graph
  std::vector<t_edge> graph;
  result =
    ODOverdrawGraph(pfViewpoint, nViewpoints,
                    (eFrontWinding !=
                     TOOTLE_CCW),  // cull CCW faces if they aren't front facing
                    cluster, ClusterStart, graph);

  if (result != TOOTLE_OK)
  {
    return result;
  }

  // reorder clusters
  std::vector<int> order(nClusters);

  if (!graph.empty())
  {
    if (feedback(nClusters, static_cast<int>(graph.size()), &graph[0],
                 &order[0]) == 0)
    {
      return TOOTLE_OUT_OF_MEMORY;
    }
  }
  else
  {
    // this means that there is no overdraw anywhere on the model, so just keep
    // the current cluster order
    for (std::uint32_t i = 0; i < nClusters; i++)
    {
      order[i] = i;
    }
  }

  // reorder triangles in soup based on cluster reordering
  std::vector<Soup::Triangle> tt = soup.t();

  int j = 0;

  for (int i = 0; i < static_cast<int>(order.size()); i++)
  {
    for (int k = ClusterStart[order[i]]; k < ClusterStart[order[i] + 1]; k++)
    {
      soup.t(j) = tt[k];
      cluster[j] = i;  // number clusters sequentially in the file
      j++;
    }
  }

  // copy to output arrays
  if (pnIBOut != nullptr)
  {
    // TODO:  Someday we should fix this memcpy of int to unsigned int
    memcpy(pnIBOut, (unsigned int*)&soup.t(0), sizeof(int) * nFaces * 3);
  }

  if (pnClusterRemapOut != nullptr)
  {
    memcpy(pnClusterRemapOut, &(order[0]), sizeof(std::uint32_t) * nClusters);
  }

  return TOOTLE_OK;
}

//=================================================================================================================================
/// This method should be called when the application is finished Tootling.
/// Failure to call this function when the application is finished Tootling will
/// result in resource leaks
//=================================================================================================================================
void TOOTLE_DLL TootleCleanup()
{
  try
  {
    // clean up overdraw module
    if (ODIsInitialized())
    {
      ODCleanup();
    }
  }
  catch (...)
  {
  }
}

TootleResult TOOTLE_DLL TootleOptimize(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, unsigned int nCacheSize,
  const float* pViewpoints, unsigned int nViewpoints,
  TootleFaceWinding eFrontWinding, unsigned int* pnIBOut,
  unsigned int* pnNumClustersOut, TootleVCacheOptimizer eVCacheOptimizer)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pVB);
  assert(pnIB);
  assert(pnIBOut);

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleOptimize: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleOptimize: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nCacheSize == 0)
  {
    errorf(("TootleOptimize: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVBStride < 3 * sizeof(float))
  {
    errorf(("TootleOptimize: nVBStride is less than 3*sizeof(float)"));

    return TOOTLE_INVALID_ARGS;
  }

  // make sure that they're not being stupid and passing us bad enum values
  if (eFrontWinding != TOOTLE_CCW && eFrontWinding != TOOTLE_CW)
  {
    errorf(("Invalid face winding."));

    return TOOTLE_INVALID_ARGS;
  }

  // allocate an array to hold the cluster ID for each face
  auto* pnFaceClusters = new unsigned int[nFaces + 1];

  TootleResult result;
  // cluster the mesh, and sort faces by cluster
  result = TootleClusterMesh(pVB, pnIB, nVertices, nFaces, nVBStride, 0,
                             pnIBOut, pnFaceClusters, nullptr);

  if (result != TOOTLE_OK)
  {
    return result;
  }

  // perform vertex cache optimization on the clustered mesh
  result =
    TootleVCacheClusters(pnIBOut, nFaces, nVertices, nCacheSize, pnFaceClusters,
                         pnIBOut, nullptr, eVCacheOptimizer);

  if (result != TOOTLE_OK)
  {
    return result;
  }

  // optimize the draw order
  result = TootleOptimizeOverdraw(pVB, pnIBOut, nVertices, nFaces, nVBStride,
                                  pViewpoints, nViewpoints, eFrontWinding,
                                  pnFaceClusters, pnIBOut, nullptr);

  if (result != TOOTLE_OK)
  {
    return result;
  }

  // if the resulting number of clusters is requested, return it to the caller
  if (pnNumClustersOut != nullptr)
  {
    *pnNumClustersOut = pnFaceClusters[nFaces];
  }

  delete[] pnFaceClusters;

  return TOOTLE_OK;

  AMD_TOOTLE_API_FUNCTION_END
}

TootleResult TOOTLE_DLL TootleFastOptimize(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, unsigned int nCacheSize,
  TootleFaceWinding eFrontWinding, unsigned int* pnIBOut,
  unsigned int* pnNumClustersOut, float fAlpha)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pVB);
  assert(pnIB);
  assert(pnIBOut);

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleFastOptimize: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleFastOptimize: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVBStride < 3 * sizeof(float))
  {
    errorf(("TootleFastOptimize: nVBStride is less than 3*sizeof(float)"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nCacheSize == 0)
  {
    errorf(("TootleFastOptimize: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  unsigned int* pnClustersTmp;
  unsigned int pnNumClustersTmp;

  pnClustersTmp = new unsigned int[nFaces + 1];

  TootleResult result;

  // OPTIMIVE VERTEX CACHE AND CLUSTERS
  result = TootleFastOptimizeVCacheAndClusterMesh(
    pnIB, nFaces, nVertices, nCacheSize, pnIBOut, pnClustersTmp,
    &pnNumClustersTmp, fAlpha);

  if (result != TOOTLE_OK)
  {
    // an error detected
    delete[] pnClustersTmp;
    return result;
  }

  // OPTIMIZE OVERDRAW
  result =
    TootleOptimizeOverdraw(pVB, pnIBOut, nVertices, nFaces, nVBStride, nullptr,
                           0, eFrontWinding, pnClustersTmp, pnIBOut, nullptr);

  delete[] pnClustersTmp;

  if (pnNumClustersOut != nullptr)
  {
    *pnNumClustersOut = pnNumClustersTmp;
  }

  return result;

  AMD_TOOTLE_API_FUNCTION_END
}

TootleResult TOOTLE_DLL TootleVCacheClusters(
  const unsigned int* pnIB, unsigned int nFaces, unsigned int nVertices,
  unsigned int nCacheSize, const unsigned int* pnFaceClusters,
  unsigned int* pnIBOut, unsigned int* pnFaceRemapOut,
  TootleVCacheOptimizer eVCacheOptimizer)
{

  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pnIB);

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleVCacheClusters: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleVCacheClusters: Invalid value of nVertices"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nCacheSize == 0)
  {
    errorf(("TootleVCacheClusters: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  if (eVCacheOptimizer != TOOTLE_VCACHE_AUTO &&
      eVCacheOptimizer != TOOTLE_VCACHE_DIRECT3D &&
      eVCacheOptimizer != TOOTLE_VCACHE_LSTRIPS &&
      eVCacheOptimizer != TOOTLE_VCACHE_TIPSY)
  {
    errorf(
      ("TootleVCacheClusters: Invalid selection for vertex cache optimization "
       "algorithm"));

    return TOOTLE_INVALID_ARGS;
  }

  // VCache within clusters
  std::uint32_t nClusterStart = 0;
  TootleResult result;

  for (std::uint32_t i = 0; i < nFaces; i++)
  {
    if (i == nFaces - 1 || (pnFaceClusters[i + 1] != pnFaceClusters[i]))
    {
      std::uint32_t nClusterFaces = 1 + (i - nClusterStart);

      const std::uint32_t* pnClusterIB = &pnIB[3 * nClusterStart];
      std::uint32_t* pnClusterIBOut =
        (pnIBOut) != nullptr ? &pnIBOut[3 * nClusterStart] : nullptr;
      std::uint32_t* pnClusterRemapOut = (pnFaceRemapOut) != nullptr
                                           ? &pnFaceRemapOut[3 * nClusterStart]
                                           : nullptr;

      result = TootleOptimizeVCache(pnClusterIB, nClusterFaces, nVertices,
                                    nCacheSize, pnClusterIBOut,
                                    pnClusterRemapOut, eVCacheOptimizer);

      if (result != TOOTLE_OK)
      {
        return result;
      }

      nClusterStart = i + 1;
    }
  }

  return TOOTLE_OK;

  AMD_TOOTLE_API_FUNCTION_END
}

TootleResult TOOTLE_DLL TootleMeasureCacheEfficiency(const unsigned int* pnIB,
                                                     unsigned int nFaces,
                                                     unsigned int nCacheSize,
                                                     float* pfEfficiencyOut)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pnIB);
  assert(pfEfficiencyOut);

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleMeasureCacheEfficiency: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nCacheSize == 0)
  {
    errorf(("TootleMeasureCacheEfficiency: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  // allocate ourselves a vertex cache
  auto* pnCache = new unsigned int[nCacheSize];

  // initialize cache to EMPTY
  std::uint32_t nEmpty = 0xffffffff;

  for (std::uint32_t i = 0; i < nCacheSize; i++)
  {
    pnCache[i] = nEmpty;
  }

  // simulate vertex processing
  std::uint32_t nFetches = 0;
  std::uint32_t nCacheIndex = 0;
  std::uint32_t nIndices = 3 * nFaces;

  for (std::uint32_t i = 0; i < nIndices; i++)
  {
    std::uint32_t nVert = pnIB[i];

    // search for this vertex in the cache
    bool bFound = false;

    for (std::uint32_t j = 0; j < nCacheSize; j++)
    {
      if (nVert == pnCache[j])
      {
        bFound = true;
        break;
      }
    }

    // it's not there, so put it there
    if (!bFound)
    {
      nFetches++;
      pnCache[nCacheIndex] = pnIB[i];
      nCacheIndex = (nCacheIndex + 1);

      if (nCacheIndex == nCacheSize)
      {
        nCacheIndex = 0;
      }
    }
  }

  delete[] pnCache;

  // I don't know why anyone would pass NULL for this, but just in case...
  if (pfEfficiencyOut != nullptr)
  {
    *pfEfficiencyOut = (float)nFetches / (float)nFaces;
  }

  return TOOTLE_OK;

  AMD_TOOTLE_API_FUNCTION_END
}

TootleResult TOOTLE_DLL TootleMeasureOverdraw(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, const float* pfViewpoint,
  unsigned int nViewpoints, TootleFaceWinding eFrontWinding, float* pfAvgODOut,
  float* pfMaxODOut)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pVB);
  assert(pnIB);

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleMeasureOverdraw: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleMeasureOverdraw: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVBStride < 3 * sizeof(float))
  {
    errorf(("TootleMeasureOverdraw: nVBStride less than 3*sizeof(float)"));

    return TOOTLE_INVALID_ARGS;
  }

  return TootleMeasureOverdrawRaytrace(pVB, pnIB, nVertices, nFaces, nVBStride,
                                       pfViewpoint, nViewpoints, eFrontWinding,
                                       pfAvgODOut, pfMaxODOut);

  AMD_TOOTLE_API_FUNCTION_END
}

TootleResult TootleMeasureOverdrawRaytrace(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, const float* pfViewpoint,
  unsigned int nViewpoints, TootleFaceWinding eFrontWinding, float* pfAvgODOut,
  float* pfMaxODOut)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pVB);
  assert(pnIB);

  if (nVertices == 0 || nVertices > TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleMeasureOverdraw: nCacheSize = 0"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleMeasureOverdraw: Invalid value of nFaces"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVBStride < 3 * sizeof(float))
  {
    errorf(("TootleMeasureOverdraw: nVBStride less than 3*sizeof(float)"));

    return TOOTLE_INVALID_ARGS;
  }

  // make sure that they're not being stupid and passing us bad enum values
  if (eFrontWinding != TOOTLE_CCW && eFrontWinding != TOOTLE_CW)
  {
    errorf(("Invalid face winding."));

    return TOOTLE_INVALID_ARGS;
  }

  // use default viewpoints if they were omitted
  if (pfViewpoint == nullptr)
  {
    pfViewpoint = pDefaultViewpoint;
    nViewpoints = nDefaultViewpoints;
  }

  // create a non-interleaved vertex buffer
  auto* pfVB = new float[3 * nVertices];

  const auto* pVBuffer = static_cast<const char*>(pVB);

  for (unsigned int i = 0; i < nVertices; i++)
  {
    memcpy(&pfVB[3 * i], pVBuffer, sizeof(float) * 3);
    pVBuffer += nVBStride;
  }

  TootleResult result;
  float fAvgOD;
  float fMaxOD;

  result = ODObjectOverdrawRaytrace(
    pfVB, pnIB, nVertices, nFaces, pfViewpoint, nViewpoints,
    (eFrontWinding !=
     TOOTLE_CCW),  // cull CCW faces if they aren't front facing
    fAvgOD, fMaxOD);

  if (pfAvgODOut != nullptr)
  {
    *pfAvgODOut = fAvgOD - 1.0f;
  }

  if (pfMaxODOut != nullptr)
  {
    *pfMaxODOut = fMaxOD - 1.0f;
  }

  // clean up
  delete[] pfVB;

  return result;

  AMD_TOOTLE_API_FUNCTION_END
}

//=================================================================================================================================
/// A helper function to convert the cluster array ID from the compact format to
/// the full format.
///  The compact format is used by old tootle (i3D version) while the full
///  format is used by the new tootle (SIGGRAPH version).
/// \param pnID    The cluster array of size nFaces+1.  Must be of type compact.
/// \param nFaces  The total number of faces of the mesh.
///
/// \return Possible return codes:  TOOTLE_INVALID_ARGS, TOOTLE_INTERNAL_ERROR,
/// TOOTLE_OUT_OF_MEMORY, or TOOTLE_OK.
//=================================================================================================================================
static TootleResult ConvertClusterArrayFromCompactToFull(unsigned int* pnID,
                                                         unsigned int nFaces)
{
  // sanity checks
  assert(pnID);

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("ConvertClusterArrayFromCompactToFull: nFaces is invalid"));

    return TOOTLE_INVALID_ARGS;
  }

  // if less then two triangles, then just return from the function.
  if (nFaces < 2)
  {
    return TOOTLE_OK;
  }

  // check if the cluster array is of compact format.
  if (!IsClusterArrayCompactFormat(pnID, nFaces))
  {
    errorf(
      ("ConvertClusterArrayFromCompactToFull: pnID is not a compact format"));

    return TOOTLE_INTERNAL_ERROR;
  }

  unsigned int nNumCluster;

  nNumCluster = pnID[nFaces];

  // sanity check on the number of cluster
  if (nNumCluster > nFaces)
  {
    errorf(("ConvertClusterArrayFromCompactToFull: nNumCluster > nLength"));

    return TOOTLE_INTERNAL_ERROR;
  }

  // Make a local copy of the compact form array
  auto* pnTmp = new unsigned int[nNumCluster + 1];

  memcpy(pnTmp, pnID, (nNumCluster + 1) * sizeof(unsigned int));

  unsigned int i;
  unsigned int j;
  unsigned int nStart;

  // Compute the full format.
  nStart = 0;

  for (i = 1; i <= nNumCluster; i++)
  {
    if (pnTmp[i] > nFaces)
    {
      errorf(("ConvertClusterArrayFromCompactToFull: pnTmp[i] > nLength"));

      return TOOTLE_INTERNAL_ERROR;
    }

    for (j = nStart; j < pnTmp[i]; j++)
    {
      pnID[j] = i - 1;
    }

    nStart = pnTmp[i];
  }

  // Append the last entry of the array by the number of total clusters
  pnID[nFaces] = nNumCluster;

  delete[] pnTmp;

  return TOOTLE_OK;
}

//=================================================================================================================================
/// A helper function to check whether the cluster array is of a full format
/// type.
///  The full format is used by the old tootle (i3D version).
///  This function does not check whether the array is valid.
///
/// \param pnID    The cluster array of size nFaces+1.  Must be of type compact.
/// \param nFaces  The total number of faces of the mesh.
///
/// \return Possible return codes:  true if the cluster array is of a full
/// format type, false otherwise
//=================================================================================================================================
static bool IsClusterArrayFullFormat(const unsigned int* pnID,
                                     unsigned int nFaces)
{
  assert(pnID);
  assert(nFaces > 0);

  return pnID[nFaces] == (1 + pnID[nFaces - 1]);
}

//=================================================================================================================================
/// A helper function to check whether the cluster array is of a compact format
/// type.
///  The compact format is used by the new tootle (SIGGRAPH version).
///  This function does not check whether the array is valid.
///
/// \param pnID    The cluster array of size nFaces+1.  Must be of type compact.
/// \param nFaces  The total number of faces of the mesh.
///
/// \return Possible return codes:  true if the cluster array is of a full
/// format type, false otherwise
//=================================================================================================================================
static bool IsClusterArrayCompactFormat(const unsigned int* pnID,
                                        unsigned int nFaces)
{
  assert(pnID);
  assert(nFaces > 0);

  unsigned int nNumCluster;

  nNumCluster = pnID[nFaces];

  // a special case of all the faces are in a separate cluster (it is both a
  // compact and full form)
  if (nNumCluster == nFaces)
  {
    return true;
  }
  else
  {
    if (nNumCluster == (1 + pnID[nFaces - 1]))
    {
      return false;  // a full format
    }
    else
    {
      return true;
    }
  }
}

TootleResult TOOTLE_DLL TootleOptimizeVertexMemory(
  const void* pVB, const unsigned int* pnIB, unsigned int nVertices,
  unsigned int nFaces, unsigned int nVBStride, void* pVBOut,
  unsigned int* pnIBOut, unsigned int* pnVertexRemapOut)
{
  AMD_TOOTLE_API_FUNCTION_BEGIN

  // sanity checks
  assert(pVB);
  assert(pnIB);

  // We also check whether nVertices is not equal to TOOTLE_MAX_VERTICES since
  //  we will use TOOTLE_MAX_VERTICES as a flag to denote that the vertex has
  //  not been mapped.
  if (nVertices == 0 || nVertices >= TOOTLE_MAX_VERTICES)
  {
    errorf(("TootleOptimizeVertexMemory: nVertices is invalid"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nFaces == 0 || nFaces > TOOTLE_MAX_FACES)
  {
    errorf(("TootleOptimizeVertexMemory: nFaces is invalid"));

    return TOOTLE_INVALID_ARGS;
  }

  if (nVBStride < 3 * sizeof(float))
  {
    errorf(
      ("TootleOptimizeVertexMemory: nVBStride is less than 3*sizeof(float)"));

    return TOOTLE_INVALID_ARGS;
  }

  // make a local copy for pVBOut and pnIBOut if they are the same as pVB and
  // pnIB.
  auto* pVBOutTmp = (char*)pVBOut;
  unsigned int* pnIBOutTmp = pnIBOut;

  if (pVBOut == nullptr || pVB == pVBOut)
  {
    pVBOutTmp = new char[nVertices * nVBStride];
  }

  if (pnIBOut == nullptr || pnIB == pnIBOut)
  {
    pnIBOutTmp = new unsigned int[3 * nFaces];
  }

  // create an array of vertex id map.
  auto* pnVIDRemap = new unsigned int[nVertices];

  unsigned int i;

  // mark all vertices map as hasn't been touched/remapped.
  for (i = 0; i < nVertices; i++)
  {
    pnVIDRemap[i] = TOOTLE_MAX_VERTICES;
  }

  memcpy(pnIBOutTmp, pnIB, 3 * nFaces * sizeof(unsigned int));

  // REMAP THE VERTICES based on the vertex ids in indices array
  unsigned int nVID;
  unsigned int nVIDCount = 0;
  unsigned int nFaces3 = nFaces * 3;
  bool bWarning = true;

  for (i = 0; i < nFaces3; i++)
  {
    nVID = pnIBOutTmp[i];

    // check whether the vertex has been mapped
    if (nVID < nVertices)
    {
      if (pnVIDRemap[nVID] == TOOTLE_MAX_VERTICES)
      {
        pnVIDRemap[nVID] = nVIDCount++;
      }

      pnIBOutTmp[i] = pnVIDRemap[nVID];
    }
    else
    {
      if (bWarning)
      {
        fprintf(stderr,
                "TootleOptimizeVertexMemory's warning: triangle indices are "
                "referencing out-of-bounds vertex buffer.\n");
        bWarning = false;
      }
    }
  }

  // Make sure we map all the vertices.
  // It is possible for some of the vertices not to be referenced by the
  // triangle indices. In this case, we just assign them to the end of the
  // vertex buffer.
  for (i = 0; i < nVertices; i++)
  {
    if (pnVIDRemap[i] == TOOTLE_MAX_VERTICES)
    {
      pnVIDRemap[i] = nVIDCount++;
    }
  }

  // check the result (make sure we have mapped all the vertices)
  assert(nVIDCount == nVertices);

  for (i = 0; i < nVertices; i++)
  {
    assert(pnVIDRemap[i] != TOOTLE_MAX_VERTICES);
  }

  // if pVBOut is requested by the user, fill it with the right data
  if (pVBOut != nullptr)
  {
    // rearrange the vertex buffer based on the remapping
    const char* pVBuffer = (const char*)pVB;

    for (i = 0; i < nVertices; i++)
    {
      nVID = pnVIDRemap[i];

      memcpy(&pVBOutTmp[nVID * nVBStride], pVBuffer, nVBStride);

      pVBuffer += nVBStride;
    }

    // copy the result if the user is supplying the same pointer for pVB and
    // pVBOut
    if (pVBOut != pVBOutTmp)
    {
      memcpy(pVBOut, pVBOutTmp, nVertices * nVBStride);
    }
  }

  // delete pVBOutTmp if it is created locally
  if (pVBOut != pVBOutTmp)
  {
    delete[] pVBOutTmp;
  }

  if (pnIBOut != pnIBOutTmp && pnIBOut != nullptr)
  {
    memcpy(pnIBOut, pnIBOutTmp, 3 * nFaces * sizeof(unsigned int));
  }

  if (pnIBOut != pnIBOutTmp)
  {
    delete[] pnIBOutTmp;
  }

  // if the vertex id remap is asked by the caller
  if (pnVertexRemapOut != nullptr)
  {
    memcpy(pnVertexRemapOut, pnVIDRemap, nVertices * sizeof(unsigned int));
  }

  delete[] pnVIDRemap;

  return TOOTLE_OK;

  AMD_TOOTLE_API_FUNCTION_END
}
