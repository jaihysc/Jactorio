// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CONFIG_H
#define JACTORIO_INCLUDE_CONFIG_H
#pragma once

namespace jactorio
{
    /// Compile time program configuration values
    struct CConfig
    {
        static constexpr auto kVersion =
#include "_config/version"
            ;

        static constexpr auto kBuildTargetPlatform =
#include "_config/build_target_platform"
            ;

#ifdef JACTORIO_DEBUG_BUILD
        static constexpr auto kBuildType = "Debug";
#else
        static constexpr auto kBuildType = "Release";
#endif
    };
} // namespace jactorio

#endif // JACTORIO_INCLUDE_CONFIG_H
