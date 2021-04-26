// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#ifndef JACTORIO_INCLUDE_CORE_CONVERT_H
#define JACTORIO_INCLUDE_CORE_CONVERT_H
#pragma once

#include <type_traits>
#include <typeinfo>

#include "jactorio.h"

namespace jactorio
{
    /// Performs cast, data may be lost
    template <typename TTarget, typename TOriginal>
    J_NODISCARD constexpr TTarget LossyCast(
        TOriginal val,
        std::enable_if_t<std::is_integral_v<TOriginal> || std::is_floating_point_v<TOriginal>, int> = 0,
        std::enable_if_t<std::is_integral_v<TTarget> || std::is_floating_point_v<TTarget>, int>     = 0) noexcept {

        return static_cast<TTarget>(val);
    }

    /// Performs cast ensuring no data is lost
    /// \remark Same behavior as static cast if assertions are disabled
    template <typename TTargetInt, typename TOriginalInt>
    J_NODISCARD constexpr TTargetInt SafeCast(
        TOriginalInt val,
        std::enable_if_t<std::is_integral_v<TOriginalInt>, int>                                       = 0,
        std::enable_if_t<std::is_integral_v<TTargetInt> || std::is_floating_point_v<TTargetInt>, int> = 0) noexcept {

        constexpr bool is_different_signedness =
            (std::is_signed<TTargetInt>::value != std::is_signed<TOriginalInt>::value);

        const auto cast_val = LossyCast<TTargetInt>(val);

        if (LossyCast<TOriginalInt>(cast_val) != val ||
            (is_different_signedness && ((cast_val < TTargetInt{}) != (val < TOriginalInt{})))) {
            assert(false);
        }

        return cast_val;
    }

    template <typename TyLeft, typename TyRight>
    void SafeCastAssign(TyLeft& l_val, TyRight&& r_val) {
        l_val = SafeCast<std::remove_reference_t<decltype(l_val)>>(r_val);
    }

    /// Performs downcast of pointer safely
    /// \remark Same behavior as static cast if non debug
    template <typename TTarget, typename TOriginal>
    J_NODISCARD constexpr auto* SafeCast(
        TOriginal* ptr,
        std::enable_if_t<std::is_pointer_v<TTarget> &&                                       //
                             std::is_base_of_v<TOriginal, std::remove_pointer_t<TTarget>> && //
                             std::is_polymorphic_v<TOriginal>,
                         int> = 0) noexcept {

#ifdef JACTORIO_DEBUG_BUILD
        auto* cast_ptr = dynamic_cast<TTarget>(ptr);

        if (ptr != nullptr) {
            assert(cast_ptr != nullptr);
        }
        return cast_ptr;
#else
        return static_cast<TTarget>(ptr);
#endif
    }

    /// Performs downcast of reference safely
    /// \remark Same behavior as static cast if non debug
    template <typename TTarget, typename TOriginal>
    J_NODISCARD
#ifndef JACTORIO_DEBUG_BUILD
    constexpr
#endif
    auto& SafeCast(
        TOriginal& ref,
        std::enable_if_t<std::is_reference_v<TTarget> &&                                       //
                             std::is_base_of_v<TOriginal, std::remove_reference_t<TTarget>> && //
                             std::is_polymorphic_v<TOriginal>,
                         int> = 0) noexcept {

#ifdef JACTORIO_DEBUG_BUILD
        try {
            auto& cast_ref = dynamic_cast<TTarget>(ref);
            return cast_ref;
        }
        catch (std::bad_cast&) {
            assert(false);
            return static_cast<TTarget>(ref); // Return something to make the compiler happy
        }
#else
        return static_cast<TTarget>(ref);
#endif
    }
} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_CONVERT_H
