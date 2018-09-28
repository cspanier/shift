/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/

#ifndef TOOTLE_FEEDBACK_H
#define TOOTLE_FEEDBACK_H

typedef struct _t_edge
{
  int from, to, cost;
} t_edge;

int feedback(int nVerts, int nArcs, t_edge* graph, int* order);

#endif
