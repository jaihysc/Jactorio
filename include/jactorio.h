#ifndef JACTORIO_H
#define JACTORIO_H

#include "core/debug/execution_timer.h"
#include "core/logger.h"

#define J_NO_DISCARD [[nodiscard]]

// Build type
#ifdef JACTORIO_DEBUG_BUILD
#define BUILD_TYPE "Debug"
#else
#define BUILD_TYPE "Release"
#endif


#endif // JACTORIO_H
