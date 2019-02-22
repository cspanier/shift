//===============================================================================
// Copyright (c) 2014-2016  Advanced Micro Devices, Inc. All rights reserved.
//===============================================================================
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef COMPRESSONATOR_DEBUG_H
#define COMPRESSONATOR_DEBUG_H

#include <stdarg.h>
#include <stdio.h>

#ifdef _LOCAL_DEBUG

#define MAX_DBGPPRINTBUFF_SIZE 512
#define MAX_DBGBUFF_SIZE 256

class DbgTracer
{
private:
  const char* m_function;

public:
  // Our dynamic class used during DbgTrace calls
  DbgTracer(const char* FunctionName) : m_function(FunctionName)
  {
  }

  // This is used only when class is contructed and when operator overrided
  virtual ~DbgTracer()
  {
    // Cleanup print
    m_function = "";
    buff[0] = '\0';
  }

  // this is used for tracing functions :: with no parameters
  void operator()(void)
  {
    PrintMsg();
  }

  // Clean up operator for existing debug messages in code DBG_ERROR(...)
  // DBG_PRINT(...) etc
  void operator()(const char* Format, ...)
  {
    // define a pointer to save argument list
    va_list args;

    // process the arguments into our debug buffer
    va_start(args, Format);
    vsnprintf(buff, MAX_DBGBUFF_SIZE, Format, args);
    va_end(args);

    // print debug message
    PrintMsg();
  }

protected:
  static char buff[MAX_DBGBUFF_SIZE];
  static char PrintBuff[MAX_DBGPPRINTBUFF_SIZE];
  static ULONG stacklevel;
  static ULONG level;

  void PrintMsg()
  {
    _snprintf_s(PrintBuff, MAX_DBGPPRINTBUFF_SIZE, "[   ]%-30s, %s\n",
                m_function, buff);
    OutputDebugStringA(PrintBuff);
  }
};

#define Dbg_Trace (DbgTracer(__FUNCTION__))
#define DbgTrace(x) Dbg_Trace x;
#else
#define Dbg_Trace
#define DbgTrace(x)
#endif

#endif
