// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_JACTORIO_H
#define JACTORIO_INCLUDE_JACTORIO_H
#pragma once

#include "core/logger.h"

#include <cassert>
#include <cstdint>

#define J_NODISCARD  [[nodiscard]]
#define J_DEPRECATED [[deprecated]]

// Build type
#ifdef JACTORIO_DEBUG_BUILD
#define JACTORIO_BUILD_TYPE "Debug"
#else
#define JACTORIO_BUILD_TYPE "Release"
#endif

#endif // JACTORIO_INCLUDE_JACTORIO_H
