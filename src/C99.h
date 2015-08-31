/*
*  Copyright 2010,2011 Reality Jockey, Ltd.
*                 info@rjdj.me
*                 http://rjdj.me/
*
*  This file is part of ZenGarden.
*
*  ZenGarden is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Lesser General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  ZenGarden is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public License
*  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef _CUSTOM_C99_H_
#define _CUSTOM_C99_H_

// va_start, etc.
#include <stdarg.h>
// _vscprintf
#include <stdio.h>

#include "configuration.h"

// Workarounds for C99 features missing in MSVC

#if _COMPILER_MSVC

// snprintf

inline int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
  int count = -1;

  if (size != 0)
    count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
  if (count == -1)
    count = _vscprintf(format, ap);

  return count;
}

inline int c99_snprintf(char* str, size_t size, const char* format, ...)
{
  int count;
  va_list ap;

  va_start(ap, format);
  count = c99_vsnprintf(str, size, format, ap);
  va_end(ap);

  return count;
}

#define snprintf c99_snprintf

#define M_PI 3.1415926535897932384626433832795

#define M_LN2 0.69314718055994530942

#define M_LOG2E 1.4426950408889634074

#endif  // _COMPILER_MSVC

#endif  // _CUSTOM_C99_H_
