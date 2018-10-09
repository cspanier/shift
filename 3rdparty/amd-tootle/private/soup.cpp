/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#include "TootlePCH.h"
#include "soup.h"
#include "error.h"
#include <algorithm>
#include <cstdint>

int Soup::ComputeNormals(bool force)
{
  debugf(("Computing normals"));

  if (n().size() == v().size() && !force)
  {
    return 1;
  }

  n().resize(v().size());

  const auto nf = t().size();
  const auto nv = static_cast<int>(v().size());

  for (int i = 0; i < nv; i++)
  {
    n(i) = Vector3(0, 0, 0);
  }

  for (std::size_t i = 0u; i < nf; i++)
  {
    const Vector3& p0 = v(t(i)[0]);
    const Vector3& p1 = v(t(i)[1]);
    const Vector3& p2 = v(t(i)[2]);
    Vector3 a = p0 - p1;
    Vector3 b = p1 - p2;
    Vector3 tn = Cross(a, b);
    n(t(i)[0]) += tn;
    n(t(i)[1]) += tn;
    n(t(i)[2]) += tn;
  }

  for (int i = 0; i < nv; i++)
  {
    Normalize(n(i));
  }

  debugf(("Done with normals"));
  return 1;
}

int Soup::ComputeResolution(float* resolution, bool force)
{
  const auto nf = static_cast<int>(t().size());

  if (nf < 1 || (r > 0.0f && !force))
  {
    *resolution = r;
    return 1;
  }

  int nsamp = nf / 2;

  if (nsamp > 333)
  {
    nsamp = 333;
  }

  std::vector<float> samples;

  samples.reserve(nsamp * 3);

  for (int i = 0; i < nsamp; i++)
  {
    // Quick 'n dirty portable random number generator
    static unsigned randq = 0;
    randq = unsigned(1664525) * randq + unsigned(1013904223);
    int j = randq % nf;
    const Vector3& v0 = v(t(j)[0]);
    const Vector3& v1 = v(t(j)[1]);
    const Vector3& v2 = v(t(j)[2]);
    samples[3 * i + 0] = Norm2(v0 - v1);
    samples[3 * i + 1] = Norm2(v1 - v2);
    samples[3 * i + 2] = Norm2(v2 - v0);
  }

  nth_element(samples.begin(), samples.begin() + samples.size() / 2,
              samples.end());
  *resolution = r = (float)sqrt(samples[samples.size() / 2]);
  return 1;
}

int Soup::ComputeTriNormals(std::vector<Vector3>& tn)
{
  debugf(("Computing tri normals"));

  tn.resize(t().size());

  const auto nf = static_cast<int>(t().size());

  for (int i = 0; i < nf; i++)
  {
    const Vector3& p0 = v(t(i)[0]);
    const Vector3& p1 = v(t(i)[1]);
    const Vector3& p2 = v(t(i)[2]);
    Vector3 a = p0 - p1, b = p1 - p2;
    tn[i] = Normalize(Cross(a, b));
  }

  debugf(("Done with tri normals"));
  return 1;
}

int Soup::ComputeTriCenters(std::vector<Vector3>& tc)
{
  debugf(("Computing tri centers"));

  tc.resize(t().size());

  const auto nf = static_cast<int>(t().size());

  for (int i = 0; i < nf; i++)
  {
    const Vector3& p0 = v(t(i)[0]);
    const Vector3& p1 = v(t(i)[1]);
    const Vector3& p2 = v(t(i)[2]);
    tc[i] = (p0 + p1 + p2) / 3.f;
  }

  debugf(("Done with tri centers"));
  return 1;
}

//=================================================================================================================================
/// Constructs a 'soup' object from a vertex/index buffer.
/// \param pVB         The vertex buffer
/// \param pIB         The index buffer
/// \param nVertices   The number of vertices
/// \param nFaces      The number of faces
/// \param nVBStride   The stride of the vertex buffer
/// \param pSoup       The soup to initialize
//=================================================================================================================================

bool MakeSoup(const void* pVB, const unsigned int* pIB, unsigned int nVertices,
              unsigned int nFaces, unsigned int nVBStride, Soup* pSoup)
{
  pSoup->v().resize(nVertices);
  pSoup->t().resize(nFaces);

  // note that this code memcpy's from unsigned int to int.
  // This is ok since we have restricted the number of vertices and faces to be
  // <= the largest signed integer I would like to do it right, but it's
  // problematic, because the adjacency and clustering code uses -1 as a
  // sentinel all over the place
  memcpy(&(pSoup->t(0)[0]), pIB, sizeof(std::uint32_t) * 3 * nFaces);

  const auto* pVBuffer = (const char*)pVB;

  for (unsigned int i = 0; i < nVertices; i++)
  {
    memcpy(&(pSoup->v(i)[0]), pVBuffer, sizeof(float) * 3);
    pVBuffer += nVBStride;
  }

  return true;
}
