/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef HEAP_H
#define HEAP_H

/* include file for module heap.c */

using p_heap = struct _t_heap*;
using t_heap_cost = int;
using t_heap_key = size_t;

#ifdef __cplusplus
extern "C"
{
#endif

  int heap_create(p_heap* H, t_heap_key max);
  int heap_destroy(p_heap* H);
  t_heap_key heap_size(p_heap* H);
  int heap_insert(p_heap* H, t_heap_key key, t_heap_cost cost);
  t_heap_key heap_pop(p_heap* H, t_heap_cost* cost);
  int heap_remove(p_heap* H, t_heap_key key);
  t_heap_key heap_gettop(p_heap* H, t_heap_cost* cost);

  int heap_update(p_heap* H, t_heap_key key, t_heap_cost cost);
  t_heap_key heap_position(p_heap* H, t_heap_key key);

#ifdef __cplusplus
}
#endif

#endif
