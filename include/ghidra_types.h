/* ghidra_types.h -- Ghidra placeholder type aliases for host compilation.
 *
 * Ghidra's decompiler emits `undefined4`, `undefined2`, `undefined1` etc. for
 * storage of unknown type.  These are equivalent to same-size unsigned integers.
 * This header must be force-included before every decompiled translation unit
 * so MSVC/GCC can parse function signatures that use these names.
 *
 * Forced via CMakeLists.txt: /FI (MSVC) or -include (GCC/Clang).
 * Do NOT include this header manually -- it is automatically injected.
 */
#pragma once
#ifndef GHIDRA_TYPES_H
#define GHIDRA_TYPES_H

#include <stdint.h>

typedef uint8_t  undefined;   /* Ghidra: 1-byte undefined storage */
typedef uint8_t  undefined1;  /* Ghidra: 1-byte undefined storage */
typedef uint16_t undefined2;  /* Ghidra: 2-byte undefined storage */
typedef uint32_t undefined4;  /* Ghidra: 4-byte undefined storage */
typedef uint64_t undefined8;  /* Ghidra: 8-byte undefined storage */

/* Ghidra scalar aliases used in some pseudoC outputs */
typedef uint8_t  byte;
typedef uint16_t ushort;
typedef unsigned int uint;
typedef uint32_t dword;
typedef uint64_t qword;

#endif /* GHIDRA_TYPES_H */
