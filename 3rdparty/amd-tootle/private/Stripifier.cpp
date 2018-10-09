/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/

//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable
//          initialization block
//
//=================================================================================================================================
#include "Stripifier.h"
#include <cstdint>

static const int START = 0;
static const int LEFT = 1;
static const int RIGHT = 2;
static const int FINISH = 3;

#ifdef _TIMING  // if we define _TIMING, then we include Windows.h which defines
                // ERROR
#undef ERROR
#endif
static const int ERROR = 9;

//=================================================================================================================================
//
// Face class
//
//=================================================================================================================================
//=================================================================================================================================
//
//          Constructor(s) (private) / Destructor(s) Block
//
//=================================================================================================================================
Face::Face(std::uint32_t uVertexIndex1, std::uint32_t uVertexIndex2,
           std::uint32_t uVertexIndex3)
{
  m_vertexIndices[0] = uVertexIndex1;
  m_vertexIndices[1] = uVertexIndex2;
  m_vertexIndices[2] = uVertexIndex3;
  m_uDegree = 0;

  m_neighbors[0] = nullptr;  // neighbor between vertex 0 and 1
  m_neighbors[1] = nullptr;  // neighbor between vertex 1 and 2
  m_neighbors[2] = nullptr;  // neighbor between vertex 2 and 3

  m_bProcessed = false;
}

//=================================================================================================================================
//
//          Public methods block
//
//=================================================================================================================================

//=================================================================================================================================
/// adds the specified face as a neighbor to the specified side
///
/// \param pFace pointer to a Face to set as a neighbor on side uSide
/// \param uSide the side that the neighbor should be created on
///
/// \return the new number of neighbors
//=================================================================================================================================
std::uint32_t Face::AddNeighbor(Face* pFace, std::uint32_t uSide)
{
  if (m_uDegree < 3 && uSide < 3 &&
      pFace != nullptr)  // can't have more than 3 neighbors
  {
    m_neighbors[uSide] = pFace;
    return ++m_uDegree;
  }
  else
  {
    // TODO: report too many neighbors
    //   OR: invalid side
    assert(!"INVALID NEIGHBOR OR SIDE");
  }

  return m_uDegree;
}

//=================================================================================================================================
/// Removes the specified neighbor
///
/// \param pFace pointer to a neighbor which should be removed from the face
///
/// \return The number of neighbors remaining
//=================================================================================================================================
std::uint32_t Face::RemoveNeighbor(Face* pNeighbor)
{
  if (m_uDegree > 0 && pNeighbor != nullptr)
  {
    const std::uint32_t uTmpDegree = m_uDegree;

    for (std::uint32_t i = 0; i < 3; i++)
    {
      if (pNeighbor == m_neighbors[i])
      {
        m_uDegree--;
        m_neighbors[i] = nullptr;
      }
    }

    if (uTmpDegree == m_uDegree)
    {
      // TODO: report invalid neighbor
      assert(!"INVALID NEIGHBOR");
    }
  }

  return m_uDegree;
}

//=================================================================================================================================
/// Removes the neighbor on the specified side
///
/// \param uSide Index of the side to remove the neighbor from
///
/// \return The number of neighbors remaining
//=================================================================================================================================
std::uint32_t Face::RemoveNeighbor(std::uint32_t uSide)
{
  if (uSide < 3)
  {
    if (m_neighbors[uSide] != nullptr)
    {
      m_uDegree--;
      m_neighbors[uSide] = nullptr;
    }
  }
  else
  {
    // TODO: report invalid side
    assert(!"INVALID SIDE");
  }

  return m_uDegree;
}

//=================================================================================================================================
/// Returns the edge to the right of specified neighbor
///
/// \param pNeighbor pointer to a neighbor
///
/// \return Index to the edge that is to the right of the neighbor; ERROR if
/// pNeighbor not a current neighbor
//=================================================================================================================================
std::uint32_t Face::GetRightEdge(Face* pNeighbor)
{
  for (std::uint32_t i = 0; i < 3; i++)
  {
    if (m_neighbors[i] == pNeighbor)
    {
      return (i + 1) % 3;
    }
  }

  return ERROR;
}

//=================================================================================================================================
/// returns the edge to the left of specified face (must be a neighbor)
///
/// \param pNeighbor pointer to a neighbor
///
/// \return Index to the edge that is to the left of the neighbor; ERROR if
/// pNeighbor not a current neighbor
//=================================================================================================================================
std::uint32_t Face::GetLeftEdge(Face* pNeighbor)
{
  for (std::uint32_t i = 0; i < 3; i++)
  {
    if (m_neighbors[i] == pNeighbor)
    {
      return (i + 2) % 3;
    }
  }

  return ERROR;
}

//=================================================================================================================================
/// returns the edge associated with the specified face (must be a neighbor)
///
/// \param pNeighbor pointer to a neighbor
///
/// \return Index to the edge that is shared with the neighbor; ERROR if
/// pNeighbor not a current neighbor
//=================================================================================================================================
std::uint32_t Face::GetEdge(Face* pNeighbor)
{
  for (std::uint32_t i = 0; i < 3; i++)
  {
    if (m_neighbors[i] == pNeighbor)
    {
      return i;
    }
  }

  return ERROR;
}

//=================================================================================================================================
//
//  FaceManager Class
//
//=================================================================================================================================

//=================================================================================================================================
//
//          Constructor(s) / Destructor(s) Block
//
//=================================================================================================================================
FaceManager::FaceManager()
{
  m_nFaceRemapCount = 0;
}

FaceManager::~FaceManager()
{
  for (auto f = m_faces.begin(); f != m_faces.end(); f++)
  {
    delete *f;
  }
}

//=================================================================================================================================
//
//          Public methods block
//
//=================================================================================================================================

//=================================================================================================================================
/// Makes a Face from the specified vertex indices and associates it with
/// neighbors that have already been created
///
/// \param uVertexIndex1 the first vertex index of the face
/// \param uVertexIndex2 the second vertex index of the face
/// \param uVertexIndex3 the third vertex index of the face
/// \param nID           the ID of the face
///
/// \return false if memory could not be allocated for the face; true otherwise
//=================================================================================================================================
bool FaceManager::MakeFace(std::uint32_t uVertexIndex1,
                           std::uint32_t uVertexIndex2,
                           std::uint32_t uVertexIndex3, std::uint32_t nID)
{
#ifdef _TIMING
  _TIME tMNStart;
  _TIME tAdjStart;

  tMNStart = GetTime();
#endif
  auto* pFace = new Face(uVertexIndex1, uVertexIndex2, uVertexIndex3);

#ifdef _TIMING
  m_tMakeNeighbors += GetTime() - tMNStart;

  tMNStart = GetTime();
#endif

  int nAdjacency = -1;
  Face* pIterFace = nullptr;

  for (auto f = m_faces.begin(); f != m_faces.end() && pFace->Degree() < 3; f++)
  {
    pIterFace = *f;

    // finding adjacency is expensize, only do it if the face doesn't already
    // have three neighbors
    if (pIterFace->Degree() < 3)
    {
#ifdef _TIMING
      tAdjStart = GetTime();
#endif
      nAdjacency = GetFaceAdjacency(*pFace, *pIterFace);

#ifdef _TIMING
      m_tAdjacency += (GetTime() - tAdjStart);
#endif

      if (nAdjacency >= 0)
      {
        pFace->AddNeighbor(pIterFace, (nAdjacency / 10));
        pIterFace->AddNeighbor(pFace, (nAdjacency % 10));
      }
    }
  }

#ifdef _TIMING
  m_tAdjLoop += (GetTime() - tMNStart);

  tMNStart = GetTime();
#endif

  // set ID for the face.  This is used for computing faceRemap.
  pFace->SetID(nID);

  // add to the total list of faces
  // push front since this face is likely to be used in upcoming faces
  // and the loop above will work best if the neighbors are found early
  m_faces.push_front(pFace);
#ifdef _TIMING
  m_tPush += GetTime() - tMNStart;
#endif
  return true;
}

//=================================================================================================================================
/// Generates an efficient list from the previously created faces
///
/// \return true
//=================================================================================================================================
bool FaceManager::Stripify()
{
#ifdef _TIMING
  _TIME tStartStripify = GetTime();
#endif

  for (auto iFace = m_faces.begin(); iFace != m_faces.end(); iFace++)
  {
    m_degreeBins[(*iFace)->Degree()].push_front(*iFace);
  }

#ifdef _TIMING
  m_tSortBins += (GetTime() - tStartStripify);
#endif

  std::uint32_t uWalkMode = START;
  std::uint32_t uFirstTurn = START;

  auto nodesLeft = (std::uint32_t)m_faces.size();
  bool bRepeatedTurn = false;
  int stripIndex = -1;
  FaceStrips strips;
  Face* pCurFace = nullptr;

  // variables that need to get declared before the switch statement
  Face* pNextFace = nullptr;
  Face* pGateFace = nullptr;
  std::uint32_t uGateEdge;
  std::uint32_t uBackEdge = 0;
  std::uint32_t uFirstEdge = 1;

  // make sure that the starting edge is in the 0-2 range
  uFirstEdge %= 3;

  while (nodesLeft > 0)
  {
    if (uWalkMode == START)
    {
      stripIndex++;
      strips.push_back(FaceStrip());

      pCurFace = nullptr;

      // stop when a face with lowest degree is found
      // this means we'll start along edges and the strip will work its way
      // inward Provided good improvement (~10%)
      for (std::uint32_t uDegree = 0; uDegree < 4 && pCurFace == nullptr;
           uDegree++)
      {
        if (!m_degreeBins[uDegree].empty())
        {
          // pick a face that has not already been processed
          for (auto iter = m_degreeBins[uDegree].begin();
               iter != m_degreeBins[uDegree].end() && pCurFace == nullptr;
               iter++)
          {
            if (*iter != NULL && !(*iter)->WasProcessed())
            {
              pCurFace = *(iter);
            }
          }
        }
      }

      // first face has been chosen
      // now choose the direction to travel
      uGateEdge = uFirstEdge;
      pNextFace = nullptr;
      std::uint32_t uLowestDegree = 4;

      for (std::uint32_t uEdge = 0; uEdge < 3; uEdge++)
      {
        // use GateFace as a temporary place holder
        pGateFace = pCurFace->GetNeighbors()[(uFirstEdge + uEdge) % 3];

        if (pGateFace != nullptr && !pGateFace->WasProcessed())
        {
          if (pGateFace->Degree() < uLowestDegree)
          {
            // make the next face the neighbor with the highest degree
            uLowestDegree = pGateFace->Degree();
            pNextFace = pGateFace;
            uGateEdge = (uFirstEdge + uEdge) % 3;
            uBackEdge = pNextFace->GetEdge(pCurFace);
          }
        }
      }

      // Add first face in this strip
      AddFaceToStrip(pCurFace, strips[stripIndex], START, uGateEdge);
      nodesLeft--;

      if (pNextFace != nullptr)
      {
        uWalkMode = LEFT;
        uFirstTurn = START;
      }
      else
      {
        // no next face found
        uWalkMode = FINISH;
      }
    }  // end if uWalkMode == START

    if (uWalkMode == RIGHT)
    {
      std::uint32_t uEnterEdge = uBackEdge;
      pCurFace = pNextFace;
      pNextFace = nullptr;
      uGateEdge = (uBackEdge + 1) % 3;
      pGateFace = pCurFace->GetNeighbors()[uGateEdge];

      if (pGateFace != nullptr && !pGateFace->WasProcessed())
      {
        bRepeatedTurn = false;
        pNextFace = pGateFace;
        uBackEdge = pNextFace->GetEdge(pCurFace);
        uWalkMode = LEFT;

        if (uFirstTurn == START)
        {
          uFirstTurn = RIGHT;
        }
      }
      else
      {
        // continuing to the right, need to indicate that it is a
        // repeated turn, so the output of the vertices should be different!
        bRepeatedTurn = true;
        uGateEdge = (uBackEdge + 2) % 3;
        pGateFace = pCurFace->GetNeighbors()[uGateEdge];

        if (pGateFace != nullptr && !pGateFace->WasProcessed())
        {
          pNextFace = pGateFace;
          uBackEdge = pNextFace->GetEdge(pCurFace);

          if (uFirstTurn == START)
          {
            uFirstTurn = LEFT;
          }
        }
      }

      if (bRepeatedTurn)
      {
        AddFaceToStrip(pCurFace, strips[stripIndex], LEFT, uEnterEdge);
      }
      else
      {
        AddFaceToStrip(pCurFace, strips[stripIndex], RIGHT, uEnterEdge);
      }

      nodesLeft--;

      if (pNextFace == nullptr)
      {
        uWalkMode = FINISH;
      }
    }
    else if (uWalkMode == LEFT)
    {
      std::uint32_t uEnterEdge = uBackEdge;
      pCurFace = pNextFace;
      pNextFace = nullptr;
      uGateEdge = (uBackEdge + 2) % 3;
      pGateFace = pCurFace->GetNeighbors()[uGateEdge];

      if (pGateFace != nullptr && !pGateFace->WasProcessed())
      {
        bRepeatedTurn = false;
        pNextFace = pGateFace;
        uBackEdge = pNextFace->GetEdge(pCurFace);
        uWalkMode = RIGHT;

        if (uFirstTurn == START)
        {
          uFirstTurn = LEFT;
        }
      }
      else
      {
        // continuing to the left, need to indicate that it is a
        // repeated turn, so the output of the vertices should be different!
        bRepeatedTurn = true;
        uGateEdge = (uBackEdge + 1) % 3;
        pGateFace = pCurFace->GetNeighbors()[uGateEdge];

        if (pGateFace != nullptr && !pGateFace->WasProcessed())
        {
          pNextFace = pGateFace;
          uBackEdge = pNextFace->GetEdge(pCurFace);

          if (uFirstTurn == START)
          {
            uFirstTurn = RIGHT;
          }
        }
      }

      if (bRepeatedTurn)
      {
        AddFaceToStrip(pCurFace, strips[stripIndex], RIGHT, uEnterEdge);
      }
      else
      {
        AddFaceToStrip(pCurFace, strips[stripIndex], LEFT, uEnterEdge);
      }

      nodesLeft--;

      if (pNextFace == nullptr)
      {
        uWalkMode = FINISH;
      }
    }

    if (uWalkMode == FINISH)
    {
      bRepeatedTurn = false;
      uWalkMode = START;

      // int nStrips = strips.size() -1;
      // int nTris = strips[ nStrips ].size();
      // if ( nTris > 2 )
      //{
      //   printf( " %d,", nTris );
      //   printf( "\nRotated list:" );
      //   int nRecentVertsStart = m_vertList.size() - (nTris*3);
      //   for ( int i = nRecentVertsStart; i < m_vertList.size(); i+=3 )
      //   {
      //      printf( " %2d %2d %2d,", m_vertList[i], m_vertList[i+1],
      //      m_vertList[i+2] );
      //   }
      //   printf("\n");
      //}
    }
  }

  // At this point, the strips should have all the data in them,
  // the triangles are in a good strip order, but the vertices may
  // have to be rotated to make it have good 2 vertex cache reuse
  // for ( std::uint32_t i = 0; i < strips.size(); i++ )
  //{
  //   printf( "\nStrip %3d has %4d triangles:", i, strips[i].size() );
  //   for ( FaceStrip::iterator fsi = strips[i].begin(); fsi !=
  //   strips[i].end(); fsi++ )
  //   {
  //      printf( " %2d %2d %2d,", (*fsi)->First(), (*fsi)->Second(),
  //      (*fsi)->Third() );
  //   }
  //}
  // printf( "\nRotated list:" );
  // for ( std::uint32_t i = 0; i < m_vertList.size(); i+=3 )
  //{
  //   printf( " %2d %2d %2d,", m_vertList[i], m_vertList[i+1], m_vertList[i+2]
  //   );
  //}
  // printf("\n");

#ifdef _TIMING
  m_tStripify = GetTime() - tStartStripify;
#endif

  return true;
}

//=================================================================================================================================
//
//          Private methods block
//
//=================================================================================================================================

//=================================================================================================================================
/// Removes all the neighbors from a specified Face, with the exception of a
/// specified neighbor
///
/// \param rFace The face to remove from its neighbors
/// \param pExceptNeighbor A neighbor from which rFace should not be removed
///
/// \return true
//=================================================================================================================================
bool FaceManager::DropNeighbors(Face& rFace, Face* pExceptNeighbor)
{
#ifdef _TIMING
  _TIME tStart = GetTime();
#endif
  // get all the neighbors
  Face** neighbors = rFace.GetNeighbors();

  // remove this face from the degree bins
  m_degreeBins[rFace.Degree()].remove(&rFace);

  Face* pNeighbor = nullptr;

  // re-bin the neighbors
  for (unsigned int i = 0; i < 3; i++)
  {
    pNeighbor = neighbors[i];

    if (pNeighbor != nullptr)
    {
      if (pNeighbor != pExceptNeighbor)
      {
        // Note that now the neighbors are only singly linked
        // the striped Face knows it's neighbors, but the neighbors
        // don't know the striped face.
        m_degreeBins[pNeighbor->Degree()].remove(pNeighbor);

        // push on the back so that if the algorithm needs to start a new strip
        // it is more likely to get a face whose vertex is already in the cache
        m_degreeBins[pNeighbor->RemoveNeighbor(&rFace)].push_back(pNeighbor);

        rFace.RemoveNeighbor(pNeighbor);
      }
    }
  }

#ifdef _TIMING
  m_tDropNeighbors += (GetTime() - tStart);
#endif

  return true;
}

//=================================================================================================================================
/// Returns the edge index that is shared by the two faces
///
/// \param cFace1 a constant face reference
/// \param cFace2 a constant face reference
///
/// \return -1 if the faces do share an edge; otherwise the ten's digit is the
/// edge from cFace1 -> cFace2; the one's digit is the edge from cFace1 <-
/// cFace2
//=================================================================================================================================
int FaceManager::GetFaceAdjacency(const Face& cFace1, const Face& cFace2)
{
  // the output is encoded in the following manner:
  // the ten's digit is the edge from cFace1 -> cFace2
  // the one's digit is the edge from cFace1 <- cFace2

  bool uSharedVertices[3] = {false, false, false};
  bool uSharedVertsBack[3] = {false, false, false};
  std::uint32_t uSharedVertCount = 0;

  // first vert
  if (cFace1.First() == cFace2.First())
  {
    uSharedVertsBack[0] = true;
    uSharedVertices[0] = true;
    uSharedVertCount++;
  }
  else if (cFace1.First() == cFace2.Second())
  {
    uSharedVertsBack[1] = true;
    uSharedVertices[0] = true;
    uSharedVertCount++;
  }
  else if (cFace1.First() == cFace2.Third())
  {
    uSharedVertsBack[2] = true;
    uSharedVertices[0] = true;
    uSharedVertCount++;
  }

  // Second vert
  if (cFace1.Second() == cFace2.First())
  {
    uSharedVertsBack[0] = true;
    uSharedVertices[1] = true;
    uSharedVertCount++;
  }
  else if (cFace1.Second() == cFace2.Second())
  {
    uSharedVertsBack[1] = true;
    uSharedVertices[1] = true;
    uSharedVertCount++;
  }
  else if (cFace1.Second() == cFace2.Third())
  {
    uSharedVertsBack[2] = true;
    uSharedVertices[1] = true;
    uSharedVertCount++;
  }

  // try to return early
  if (uSharedVertCount == 0)
  {
    // must have at least one shared vertex by now in order to share an edge
    return -1;
  }
  else if (uSharedVertCount == 1)
  {
    // check third vert for being shared
    if (cFace1.Third() == cFace2.First())
    {
      uSharedVertsBack[0] = true;
      uSharedVertices[2] = true;
      uSharedVertCount++;
    }
    else if (cFace1.Third() == cFace2.Second())
    {
      uSharedVertsBack[1] = true;
      uSharedVertices[2] = true;
      uSharedVertCount++;
    }
    else if (cFace1.Third() == cFace2.Third())
    {
      uSharedVertsBack[2] = true;
      uSharedVertices[2] = true;
      uSharedVertCount++;
    }
  }

  if (uSharedVertCount == 2)
  {
    // default, assume uSharedVertices[2] == false (vertices 0 and 1 are shared)
    // from cFace1 -> cFace2
    int nEncoding = 0;

    // we know it is adjacent, and must use the 2nd vertex from cFace1
    if (!uSharedVertices[0])
    {
      // if Vert0 is not set, then it must be edge 1 (verts 2 and 1) from cFace1
      // -> cFace2
      nEncoding = 10;
    }
    else if (!uSharedVertices[1])
    {
      // else it must be edge 2 from cFace1 -> cFace2
      nEncoding = 20;
    }

    if (!uSharedVertsBack[0])
    {
      // if VertsBack 0 is not set, then it must be BackEdge 1 (verts 1 and 2)
      return nEncoding + 1;
    }
    else if (!uSharedVertsBack[1])
    {
      // if VertsBack 1 is not set, then it must be BackEdge 2 (verts 2 and 0)
      return nEncoding + 2;
    }
    else
    {
      // only BackEdge 0 is left (verts 0 and 1)
      return nEncoding + 0;
    }
  }

  return -1;
}

//=================================================================================================================================
/// Adds the face to the specified list and to the efficient list
///
/// \param pFace Pointer to the face to add to the strip
/// \param rStrip The strip to add the face to
/// \param uDirection The direction that the strip will traveling in (determines
/// output order of vertices) \param uEdge Index of the edge that the face was
/// entered from
///
/// \return false if pFace is null, or uDirection or uEdge are invalid indices;
/// true otherwise
//=================================================================================================================================
bool FaceManager::AddFaceToStrip(Face* pFace, FaceStrip& rStrip,
                                 std::uint32_t uDirection, std::uint32_t uEdge)
{
  if (pFace == nullptr || uDirection > 3 || uEdge > 3)
  {
    return false;
  }

  if (!rStrip.empty())
  {
    DropNeighbors(*pFace, rStrip.back());
  }
  else
  {
    // first Face in strip needs special attention
    DropNeighbors(*pFace, nullptr);
  }

#ifdef _TIMING
  _TIME tStart = GetTime();
#endif

  if (uDirection == START)
  {
    //      printf( "Adding face: %d %d %d\n", pFace->VertexByIndex( (uEdge+2)%3
    //      ),
    //                                         pFace->VertexByIndex( (uEdge+0)%3
    //                                         ), pFace->VertexByIndex(
    //                                         (uEdge+1)%3 ) );
    m_vertList.push_back(pFace->VertexByIndex((uEdge + 2) % 3));
    m_vertList.push_back(pFace->VertexByIndex((uEdge + 0) % 3));
    m_vertList.push_back(pFace->VertexByIndex((uEdge + 1) % 3));
  }
  else if (uDirection == RIGHT)
  {
    //      printf( "Adding face: %d %d %d\n", pFace->VertexByIndex( (uEdge+0)%3
    //      ),
    //                                         pFace->VertexByIndex( (uEdge+1)%3
    //                                         ), pFace->VertexByIndex(
    //                                         (uEdge+2)%3 ) );
    m_vertList.push_back(pFace->VertexByIndex((uEdge + 0) % 3));
    m_vertList.push_back(pFace->VertexByIndex((uEdge + 1) % 3));
    m_vertList.push_back(pFace->VertexByIndex((uEdge + 2) % 3));
  }
  else  // uDirection == LEFT
  {
    //      printf( "Adding face: %d %d %d\n", pFace->VertexByIndex( (uEdge+1)%3
    //      ),
    //                                         pFace->VertexByIndex( (uEdge+2)%3
    //                                         ), pFace->VertexByIndex(
    //                                         (uEdge+0)%3 ) );
    m_vertList.push_back(pFace->VertexByIndex((uEdge + 1) % 3));
    m_vertList.push_back(pFace->VertexByIndex((uEdge + 2) % 3));
    m_vertList.push_back(pFace->VertexByIndex((uEdge + 0) % 3));
  }

  m_faceRemap[pFace->GetID()] = m_nFaceRemapCount++;

  rStrip.push_back(pFace);
  pFace->Processed();

#ifdef _TIMING
  m_tAddFaces += (GetTime() - tStart);
#endif

  return true;
}

//=================================================================================================================================
/// Resize faceMap vector to nFaces (number of triangles).
///
/// \param nFaces  Number of triangles/faces in the mesh.
///
/// \return none
//=================================================================================================================================
void FaceManager::ResizeFaceRemap(std::uint32_t nFaces)
{
  assert(nFaces > 0);

  m_faceRemap.resize(nFaces);
}

//=================================================================================================================================
/// Generates an efficient vertex index buffer from the input vertex indices
///
/// \param pVertexIndicesIN Input vertex index buffer
/// \param uiTriangleCount The number of triangles specified within the input
/// buffer \param pVertexIndicesOUT Output efficient vertex index buffer
///
//=================================================================================================================================
void Stripifier::Process(const unsigned int* pVertexIndicesIN,
                         const unsigned int uiTriangleCount,
                         unsigned int* pVertexIndicesOUT,
                         unsigned int* pnFaceRemapOut)
{
  if (pVertexIndicesIN == nullptr)
  {
    pVertexIndicesOUT = nullptr;
    return;
  }

  FaceManager faceManager;

  std::uint32_t uVertIndexSize = uiTriangleCount * 3;

  faceManager.ResizeFaceRemap(uiTriangleCount);

#ifdef _TIMING
  _TIME tStart = GetTime();
  faceManager.m_tMakeNeighbors = 0;
  faceManager.m_tAdjacency = 0;
  faceManager.m_tDropNeighbors = 0;
  faceManager.m_tAddFaces = 0;
  faceManager.m_tAdjLoop = 0;
  faceManager.m_tPush = 0;
  faceManager.m_tSortBins = 0;
#endif

  // create faces
  for (std::uint32_t u = 0; u < uVertIndexSize; u += 3)
  {
    if (!faceManager.MakeFace(pVertexIndicesIN[u], pVertexIndicesIN[u + 1],
                              pVertexIndicesIN[u + 2], u / 3))
    {
      pVertexIndicesOUT = nullptr;
      return;
    }
  }

#ifdef _TIMING
  time_t tFinish = GetTime() - tStart;
  printf("###################\n");
  printf("#Cluster Size: %u\n", uiTriangleCount);
  printf("#\tMake time: %d\n", tFinish);
  printf("#\t\tNeighbors time: %d\n", faceManager.m_tMakeNeighbors);
  printf("#\t\tAdjacLoop time: %d\n", faceManager.m_tAdjLoop);
  printf("#\t\t\tAdjacency time: %d\n", faceManager.m_tAdjacency);
  printf("#\t\tPush Tris time: %d\n", faceManager.m_tPush);
#endif

  // now all faces should be in their respective bins
  faceManager.Stripify();

#ifdef _TIMING
  tFinish = GetTime() - tStart;
  printf("#\tStripify time: %d\n", faceManager.m_tStripify);
  printf("#\t\tSort bins time: %d\n", faceManager.m_tSortBins);
  printf("#\t\tDropNeigh time: %d\n", faceManager.m_tDropNeighbors);
  printf("#\t\tAdd Faces time: %d\n", faceManager.m_tAddFaces);
  printf("#Overall Time: %d\n\n", tFinish);
#endif

  VertList vl = faceManager.GetStrippedList();

  for (std::uint32_t u = 0; u < uVertIndexSize; u++)
  {
    pVertexIndicesOUT[u] = vl[u];
  }

  if (pnFaceRemapOut != nullptr)
  {
    std::vector<std::uint32_t> pnFaceRemap = faceManager.GetFaceRemap();

    for (std::uint32_t i = 0; i < uiTriangleCount; i++)
    {
      pnFaceRemapOut[i] = pnFaceRemap[i];
    }
  }
}
