/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#include "TootlePCH.h"
#include "souptomesh.h"

int SoupToMesh(Soup* soup, Mesh* mesh)
{

  if (!soup->v().empty())
  {
    mesh->v(soup->v());
  }

  if (!soup->n().empty())
  {
    mesh->n(soup->n());
  }

  if (!soup->c().empty())
  {
    mesh->c(soup->c());
  }

  if (!soup->vc().empty())
  {
    mesh->vc(soup->vc());
  }

  if (!soup->t().empty())
  {
    mesh->t(soup->t());
  }

  mesh->ae().resize(0);
  mesh->vv().resize(0);
  return 1;
}