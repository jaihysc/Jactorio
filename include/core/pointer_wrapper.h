// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#ifndef JACTORIO_INCLUDE_CORE_POINTER_WRAPPER_H
#define JACTORIO_INCLUDE_CORE_POINTER_WRAPPER_H
#pragma once

#include <type_traits>

#include "jactorio.h"

namespace jactorio
{
    ///
    /// Manages non owning pointer to prototype
    template <typename Ty>
    class PointerWrapper
    {
    protected:
        static_assert(!std::is_pointer_v<Ty>, "Ty should not be a pointer, remove the indirection");

        using ValueT = std::size_t;

    public:
        using element_type = Ty;

        PointerWrapper() = default;

        PointerWrapper(Ty* proto) noexcept { // Intentionally non explicit to allow assignment from pointer directly
            SetPtr(proto);
        }

        explicit PointerWrapper(Ty& proto) noexcept {
            SetPtr(&proto);
        }


        Ty* operator->() noexcept {
            return GetPtr();
        }
        Ty* operator->() const noexcept {
            return GetPtr();
        }

        Ty& operator*() noexcept {
            return *GetPtr();
        }
        Ty& operator*() const noexcept {
            return *GetPtr();
        }


        friend bool operator==(const PointerWrapper& lhs, const PointerWrapper& rhs) noexcept {
            return lhs.value_ == rhs.value_;
        }
        friend bool operator!=(const PointerWrapper& lhs, const PointerWrapper& rhs) noexcept {
            return !(lhs == rhs);
        }


        friend void swap(PointerWrapper& lhs, PointerWrapper& rhs) noexcept {
            using std::swap;
            swap(lhs.value_, rhs.value_);
        }

        J_NODISCARD Ty* Get() noexcept {
            return GetPtr();
        }
        J_NODISCARD Ty* Get() const noexcept {
            return GetPtr();
        }

    protected:
        /// ptr or internal id
        static_assert(sizeof(ValueT) == sizeof(Ty*));
        ValueT value_ = 0;

        void SetPtr(Ty* proto) noexcept {
            value_ = reinterpret_cast<ValueT>(proto);
        }

    private:
        J_NODISCARD Ty* GetPtr() noexcept {
            return reinterpret_cast<Ty*>(value_);
        }
        J_NODISCARD Ty* GetPtr() const noexcept {
            return reinterpret_cast<Ty*>(value_);
        }
    };

    template <class T, std::enable_if_t<!std::is_same_v<T, std::nullptr_t>, int> = 0>
    PointerWrapper(T) -> PointerWrapper<std::remove_pointer_t<T>>;
} // namespace jactorio

#endif // JACTORIO_INCLUDE_CORE_POINTER_WRAPPER_H
