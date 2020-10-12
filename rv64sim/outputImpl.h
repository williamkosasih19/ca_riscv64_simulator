#pragma once
/* ****************************************************************
   RISC-V Instruction Set Simulator
   Computer Architecture, Semester 1, 2020

   Implementation header file for output function.

**************************************************************** */
#include <cstdarg>
#include <cstdio>

#include "types.h"

extern bool_t verbose;

// Verbose print function.
inline void verbose_print(const char* const format...)
{
  if (verbose)
  {
    va_list varg;
    va_start(varg, format);
    vprintf(format, varg);
  }
}