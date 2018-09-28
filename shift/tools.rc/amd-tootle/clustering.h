/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef CLUSTERING_H
#define CLUSTERING_H

#include <cstdint>

enum ClusterResult
{
  CLUSTER_OK,
  CLUSTER_OUT_OF_MEMORY  ///< Out of memory
};

/// Performs face clustering and returns an array with the cluster ID for each
/// face
ClusterResult Cluster(Soup* soup, std::uint32_t& nClusters,
                      std::vector<int>& cluster);

/// Sorts faces by cluster
bool SortFacesByCluster(Soup& soup, std::vector<int>& clusterIDs,
                        std::uint32_t* pRemapArray);

#endif
