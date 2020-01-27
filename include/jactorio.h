#ifndef JACTORIO_H
#define JACTORIO_H

#include "core/resource_guard.h"
#include "core/debug/execution_timer.h"
#include "core/logger.h"

#include <cassert>
#include <cstdint>

#define J_NODISCARD [[nodiscard]]
#define J_DEPRECATED [[deprecated]]
#define J_TEST_USE_ONLY [[deprecated("Function is for test use only (May be ignored if compiling for a test)")]]

// Build type
#ifdef JACTORIO_DEBUG_BUILD
#define BUILD_TYPE "Debug"
#else
#define BUILD_TYPE "Release"
#endif


#endif // JACTORIO_H
