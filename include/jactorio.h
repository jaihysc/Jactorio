// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_JACTORIO_H
#define JACTORIO_INCLUDE_JACTORIO_H
#pragma once

#include "core/logger.h"

#include <cassert>
#include <cstdint>

#define J_NODISCARD  [[nodiscard]]
#define J_DEPRECATED [[deprecated]]

#ifndef JACTORIO_DEBUG_BUILD

#if defined(_MSC_VER)
#define FORCEINLINE __forceinline
#elif defined(__GNUC__)
#define FORCEINLINE __attribute__((always_inline)) inline
#elif defined(__clang__)
#define FORCEINLINE __attribute__((always_inline))
#else
#define FORCEINLINE
#endif

#else
#define FORCEINLINE
#endif

#endif // JACTORIO_INCLUDE_JACTORIO_H
