/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef MESH_H
#define MESH_H

#include <cstdint>
#include "soup.h"
#include "Timer.h"
#include "error.h"

using VTArray = std::vector<std::vector<std::uint32_t>>;

class Mesh : public Soup
{
public:
  Mesh()
  {
    ;
  }
  ~Mesh() override
  {
    ;
  }
  int ComputeVT(VTArray& vtOut);
  int ComputeAE(const VTArray& vt);
  int ComputeVV();

  // across-edge information
  std::vector<std::uint32_t>& ae(int i)
  {
    return ae_[i];
  }
  const std::vector<std::uint32_t>& ae(int i) const
  {
    return ae_[i];
  }
  std::vector<std::vector<std::uint32_t>>& ae()
  {
    return ae_;
  }
  const std::vector<std::vector<std::uint32_t>>& ae() const
  {
    return ae_;
  }

  // neighbor vertex information
  std::vector<std::uint32_t>& vv(int i)
  {
    return vv_[i];
  }
  const std::vector<std::uint32_t>& vv(int i) const
  {
    return vv_[i];
  }
  std::vector<std::vector<std::uint32_t>>& vv()
  {
    return vv_;
  }
  const std::vector<std::vector<std::uint32_t>>& vv() const
  {
    return vv_;
  }

protected:
  // across edge info (same as structure as a triangle)
  std::vector<std::vector<std::uint32_t>> ae_;
  // vertex neighboring vertices
  std::vector<std::vector<std::uint32_t>> vv_;

private:
  // prevent catastrophic copies
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
};

inline int Mesh::ComputeVV()
{
  Timer time;
  debugf(("Finding vertex neighbors"));

  vv().resize(v().size());

  for (int i = 0; i < static_cast<int>(t().size()); i++)
  {
    vv(t(i)[0]).push_back(t(i)[1]);
    vv(t(i)[1]).push_back(t(i)[0]);
    vv(t(i)[1]).push_back(t(i)[2]);
    vv(t(i)[2]).push_back(t(i)[1]);
    vv(t(i)[2]).push_back(t(i)[0]);
    vv(t(i)[0]).push_back(t(i)[2]);
  }

  debugf(("Done in %gs", time.GetElapsed()));
  return 1;
}

inline int Mesh::ComputeVT(VTArray& vtOut)
{
  Timer time;
  debugf(("Finding vertex faces"));

  // get all faces that use each vertex
  vtOut.resize(v().size());

  for (int f = 0; f < static_cast<int>(t().size()); f++)
  {
    Soup::Triangle& face = t(f);

    for (int i = 0; i < 3; i++)
    {
      vtOut[face[i]].push_back(f);
    }
  }

  debugf(("Done in %gs", time.GetElapsed()));
  return 1;
}

inline int Mesh::ComputeAE(const VTArray& vt)
{
  Timer time;
  debugf(("Finding across edge-info"));
  // clean across-edge info
  ae().resize(t().size());

  // find across-edge info
  for (int f = 0; f < static_cast<int>(t().size()); f++)
  {
    for (int i = 0; i < 3; i++)
    {
      int in = (i + 1) % 3;
      // get the vertices in edge
      int v = t(f)[i];
      int vn = t(f)[in];

      // for each face that use v
      for (int j = 0; j < static_cast<int>(vt[v].size()); j++)
      {
        // check if face has vn too and is not f
        int af = vt[v][j];

        if (af != f)
        {
          for (int k = 0; k < 3; k++)
          {
            if (t(af)[k] == vn)
            {
              ae(f).push_back(af);
            }
          }
        }
      }
    }
  }

  debugf(("Done in %gs", time.GetElapsed()));
  return 1;
}

#endif
