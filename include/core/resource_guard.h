// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_RESOURCE_GUARD_H
#define JACTORIO_INCLUDE_CORE_RESOURCE_GUARD_H
#pragma once

#include <functional>
#include <type_traits>

namespace jactorio
{
    ///
    /// RAII wrapper which calls the provided function in its destructor
    template <typename T>
    class ResourceGuard
    {
        // Function returning T
        using Function = T (*)();

        Function f_;

    public:
        ///
        /// \param f Function which will be called upon exiting current scope
        explicit ResourceGuard(const Function f) : f_(f) {}

        ~ResourceGuard() {
            f_();
        }


        ResourceGuard(const ResourceGuard& other)     = delete;
        ResourceGuard(ResourceGuard&& other) noexcept = delete;
        ResourceGuard& operator=(const ResourceGuard& other) = delete;
        ResourceGuard& operator=(ResourceGuard&& other) noexcept = delete;
    };

    template <typename T>
    ResourceGuard(T) -> ResourceGuard<std::invoke_result_t<std::decay<T>>>;

    ///
    /// Capturing RAII wrapper which calls the provided function in its destructor
    template <typename T = void()>
    class CapturingGuard
    {
        using Function = std::function<T>;

        Function f_;

    public:
        ///
        /// \param f Function which will be called upon exiting current scope
        explicit CapturingGuard(const Function f) : f_(f) {}

        ~CapturingGuard() {
            f_();
        }


        CapturingGuard(const CapturingGuard& other)     = delete;
        CapturingGuard(CapturingGuard&& other) noexcept = delete;
        CapturingGuard& operator=(const CapturingGuard& other) = delete;
        CapturingGuard& operator=(CapturingGuard&& other) noexcept = delete;
    };
} // namespace jactorio
#endif // JACTORIO_INCLUDE_CORE_RESOURCE_GUARD_H
