// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_JACTORIO_H
#define JACTORIO_INCLUDE_JACTORIO_H
#pragma once

#include "core/execution_timer.h"
#include "core/logger.h"
#include "core/resource_guard.h"

#include <cassert>
#include <cstdint>

#define J_NODISCARD [[nodiscard]]
#define J_DEPRECATED [[deprecated]]

// Build type
#ifdef JACTORIO_DEBUG_BUILD
#define BUILD_TYPE "Debug"
#else
#define BUILD_TYPE "Release"
#endif

#endif //JACTORIO_INCLUDE_JACTORIO_H
