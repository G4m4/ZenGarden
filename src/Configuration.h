/*
 *  Copyright 2009,2011,2012 Reality Jockey, Ltd.
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

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

/// @brief Compiler detection
#undef _COMPILER_GCC
#undef _COMPILER_MSVC
#if(defined(__GNUC__))
  #define _COMPILER_GCC 1
#elif(defined(_MSC_VER))
  #define _COMPILER_MSVC 1
#else
  #error "Compiler could not be detected"
#endif

/// @brief "OS" - framework detection
#undef _OS_MACOSX
#undef _OS_WINDOWS
#undef _OS_LINUX
#if(defined(__APPLE__))
#define _OS_MACOSX 1
#elif(defined(_WIN32))  // Both 32 and 64 bits
#define _OS_WINDOWS 1
#elif(defined(__linux) || defined(__unix))  // Both 32 and 64 bits
#define _OS_LINUX 1
#else
#error "Compiler could not be detected"
#endif

/// @brief Build configuration detection
/// Since there is no easy cross-platform way to do this,
/// we assume that "no asserts" means release
#if(defined(_NDEBUG) || defined(NDEBUG))
  #define _BUILD_CONFIGURATION_DEBUG 0
#else  // defined(NDEBUG) ?
  #define _BUILD_CONFIGURATION_DEBUG 1
#endif  // defined(NDEBUG) ?

/// @brief Architecture detection - compiler specific preprocessor macros
#undef _ARCH_X86
#undef _ARCH_ARM
#if _COMPILER_MSVC
#if defined(_M_IX86)
#define _ARCH_X86 1
#elif defined(__ARM_NEON__)
#define _ARCH_ARM 1
#endif
#elif _COMPILER_GCC
#if (defined(__i386__))
#define _ARCH_X86 1
#elif defined(__ARM_NEON__)
#define _ARCH_ARM 1
#endif
#endif

/// @brief SIMD enabling, based on platform
#if defined(_DISABLE_SIMD)
  #define _USE_SSE 0
  #define _USE_NEON 0
#else
  #if (_ARCH_X86)
    #define _USE_SSE 1
  #elif (_ARCH_ARM)
    #define _USE_NEON 1
  #endif  // _ARCH_
#endif

#endif // _CONFIGURATION_H_
