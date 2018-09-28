/************************************************************************************/ /**
 // Copyright (c) 2006-2015 Advanced Micro Devices, Inc. All rights reserved.
 /// \author AMD Developer Tools Team
 /// \file
 ****************************************************************************************/
#ifndef _TIMER_H
#define _TIMER_H

class Timer
{
public:
  Timer();
  void Reset();
  double GetElapsed();
  double Get();

private:
  double time;
};

#endif  // _TIME_H
