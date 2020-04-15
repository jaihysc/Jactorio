// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/15/2019

#ifndef JACTORIO_INCLUDE_JACTORIO_H
#define JACTORIO_INCLUDE_JACTORIO_H
#pragma once

#include "core/debug/execution_timer.h"
#include "core/logger.h"
#include "core/resource_guard.h"

#include <cassert>
#include <cstdint>

#define J_NODISCARD [[nodiscard]]
#define J_DEPRECATED [[deprecated]]

// J_TEST_USE_ONLY | Warning disabled if compiling for test
#ifdef JACTORIO_BUILD_TEST
#define J_TEST_USE_ONLY [[]]
#else
#define J_TEST_USE_ONLY [[deprecated("Function is for test use only")]]
#endif

// Build type
#ifdef JACTORIO_DEBUG_BUILD
#define BUILD_TYPE "Debug"
#else
#define BUILD_TYPE "Release"
#endif

// ======================================================================
// Constants
#define JC_GAME_HERTZ 60  // Game updates per second

#endif //JACTORIO_INCLUDE_JACTORIO_H
