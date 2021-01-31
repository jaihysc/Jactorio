// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_FRAMEWORK_FRAMEWORK_BASE_H
#define JACTORIO_INCLUDE_PROTO_FRAMEWORK_FRAMEWORK_BASE_H
#pragma once

#include <string>
#include <type_traits>

#include "jactorio.h"

#include "core/data_type.h"
#include "data/cereal/serialize.h"
#include "proto/detail/category.h"
#include "proto/detail/exception.h"

#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

// Creates a setters for python API primarily, to chain initialization

// Setter passed by reference
#define PYTHON_PROP_REF(type__, var_name__)                   \
    type__ var_name__;                                        \
    auto Set_##var_name__(const type__&(parameter_value__)) { \
        this->var_name__ = parameter_value__;                 \
        return this;                                          \
    }                                                         \
    static_assert(true)

// Setter passed by reference with initializer
#define PYTHON_PROP_REF_I(type__, var_name__, initializer)    \
    type__ var_name__ = initializer;                          \
    auto Set_##var_name__(const type__&(parameter_value__)) { \
        this->var_name__ = parameter_value__;                 \
        return this;                                          \
    }                                                         \
    static_assert(true)


// Setter passed by value
#define PYTHON_PROP(type__, var_name__)                \
    type__ var_name__;                                 \
    auto Set_##var_name__(type__(parameter_value__)) { \
        this->var_name__ = parameter_value__;          \
        return this;                                   \
    }                                                  \
    static_assert(true)

// Setter passed by value with initializer
#define PYTHON_PROP_I(type__, var_name__, initializer) \
    type__ var_name__ = initializer;                   \
    auto Set_##var_name__(type__(parameter_value__)) { \
        this->var_name__ = parameter_value__;          \
        return this;                                   \
    }                                                  \
    static_assert(true)

// Assertions for PostLoadValidate
#define J_PROTO_ASSERT(condition__, msg__)                                                   \
    jactorio::proto::DataAssert(condition__,                                                 \
                                "\"%s\", " msg__ "\nTraceback (most recent call last):\n%s", \
                                this->name.c_str(),                                          \
                                this->pythonTraceback.c_str())

#define J_PROTO_ASSERT_F(condition__, format__, ...)                                            \
    jactorio::proto::DataAssert(condition__,                                                    \
                                "\"%s\", " format__ "\nTraceback (most recent call last):\n%s", \
                                this->name.c_str(),                                             \
                                __VA_ARGS__,                                                    \
                                this->pythonTraceback.c_str())

namespace jactorio::data
{
    class PrototypeManager;
}

namespace jactorio::proto
{
    ///
    /// Creates a formatted log message if log level permits
    template <typename... Args, typename = std::common_type<Args...>>
    void DataAssert(const bool condition, const char* format, Args&&... args) {
        constexpr int max_msg_length = 3000;

        if (!(condition)) {
            char buffer[max_msg_length + 1];
            snprintf(buffer, max_msg_length, format, args...);
            throw ProtoError(buffer);
        }
    }

    ///
    /// Abstract base class for all unique data
    struct UniqueDataBase
    {
    protected:
        UniqueDataBase() = default;

    public:
        virtual ~UniqueDataBase() = default;

        UniqueDataBase(const UniqueDataBase& other)     = default;
        UniqueDataBase(UniqueDataBase&& other) noexcept = default;
        UniqueDataBase& operator=(const UniqueDataBase& other) = default;
        UniqueDataBase& operator=(UniqueDataBase&& other) noexcept = default;


        UniqueDataIdT internalId = 0;

        CEREAL_SERIALIZE(archive) {
            archive(internalId);
        }

        CEREAL_LOAD_CONSTRUCT(archive, construct, UniqueDataBase) {}
    };


#define PROTOTYPE_CATEGORY(category__)                                                           \
    static constexpr jactorio::proto::Category category = jactorio::proto::Category::category__; \
    J_NODISCARD jactorio::proto::Category GetCategory() const override {                         \
        return jactorio::proto::Category::category__;                                            \
    }                                                                                            \
    static_assert(true)

#define PROTOTYPE_DATA_TRIVIAL_COPY(data_ty__)                                                 \
    std::unique_ptr<UniqueDataBase> CopyUniqueData(const UniqueDataBase* ptr) const override { \
        return std::make_unique<data_ty__>(*static_cast<const data_ty__*>(ptr));               \
    }                                                                                          \
    static_assert(true)


    class FrameworkBase
    {
    public:
        FrameworkBase()          = default;
        virtual ~FrameworkBase() = default;

        FrameworkBase(const FrameworkBase& other)     = default;
        FrameworkBase(FrameworkBase&& other) noexcept = default;
        FrameworkBase& operator=(const FrameworkBase& other) = default;
        FrameworkBase& operator=(FrameworkBase&& other) noexcept = default;

        friend void swap(FrameworkBase& lhs, FrameworkBase& rhs) noexcept {
            using std::swap;
            swap(lhs.internalId, rhs.internalId);
            swap(lhs.name, rhs.name);
            swap(lhs.order, rhs.order);
            swap(lhs.localizedName_, rhs.localizedName_);
            swap(lhs.localizedDescription_, rhs.localizedDescription_);
        }

        // ======================================================================

        ///
        /// NON VIRTUAL category, use GetCategory if object is not downcast to its actual type
        static constexpr Category category = Category::none;

        ///
        /// Category of this Prototype item
        virtual Category GetCategory() const = 0;

        ///
        /// Unique per prototype, unique & auto assigned per new prototype added
        /// 0 indicates invalid id
        PrototypeIdT internalId = 0;

        /// To location prototype was constructed
        std::string pythonTraceback;


        ///
        /// Internal name, MUST BE unique per data_category
        PYTHON_PROP_REF(std::string, name);

        ///
        /// Determines the priority of this prototype used in certain situations
        /// Automatically assigned incrementally alongside internalId if 0
        /// \remark 0 indicates invalid id
        PYTHON_PROP_REF_I(unsigned int, order, 0);


        // ======================================================================
        // Localized names

        J_NODISCARD const std::string& GetLocalizedName() const {
            return localizedName_;
        }
        virtual void SetLocalizedName(const std::string& localized_name) {
            this->localizedName_ = localized_name;
        }

        J_NODISCARD const std::string& GetLocalizedDescription() const {
            return localizedDescription_;
        }

        virtual void SetLocalizedDescription(const std::string& localized_description) {
            this->localizedDescription_ = localized_description;
        }


        // ======================================================================
        // Unique data associated with entity

        ///
        /// Copies the unique_data associated with a prototype
        virtual std::unique_ptr<UniqueDataBase> CopyUniqueData(const UniqueDataBase* /*other*/) const {
            assert(false); // Not implemented
            return nullptr;
        }


        // ======================================================================
        // Data Events
        ///
        /// Called after all prototypes are loaded prior to validation
        virtual void PostLoad() {}

        ///
        /// Validates properties of the prototype are valid
        /// \exception proto::Data_exception If invalid
        virtual void PostLoadValidate(const data::PrototypeManager& proto_manager) const = 0;

        ///
        /// Called after the prototype has been validated
        virtual void ValidatedPostLoad() {}

    protected:
        std::string localizedName_;
        std::string localizedDescription_;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_FRAMEWORK_FRAMEWORK_BASE_H
