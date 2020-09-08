// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_CEREAL_SERIALIZATION_TYPE_H
#define JACTORIO_INCLUDE_DATA_CEREAL_SERIALIZATION_TYPE_H
#pragma once

#include "jactorio.h"

#include "core/convert.h"
#include "core/pointer_wrapper.h"
#include "data/cereal/serialize.h"
#include "data/prototype_manager.h"
#include "data/unique_data_manager.h"

namespace jactorio::data
{
    ///
    /// Manages non owning pointer to prototype
    /// \remark Uses the global active prototype manager for deserializing
    ///
    /// Converts pointer to internal id when serializing, from internal id to pointer when deserializing
    template <typename TProto>
    class SerialProtoPtr : public core::PointerWrapper<TProto>
    {
        // Removing static_assert allows use with forward declarations
        //        static_assert(std::is_base_of_v<FrameworkBase, TProto>, "TProto must inherit FrameworkBase for
        //        internal id");

        using ValueT                        = typename core::PointerWrapper<TProto>::ValueT;
        static constexpr auto kArchiveSize_ = sizeof(ValueT);

    public:
        using core::PointerWrapper<TProto>::PointerWrapper;

        CEREAL_LOAD(archive) {
            CerealArchive<kArchiveSize_>(archive, this->value_); // Deserialized as internal id

            if (this->value_ == 0) // nullptr
                return;

            assert(active_prototype_manager != nullptr);
            auto* proto_ptr = &active_prototype_manager->RelocationTableGet<TProto>( // Converted to prototype*
                core::SafeCast<PrototypeIdT>(this->value_));
            this->SetPtr(proto_ptr);
        }

        CEREAL_SAVE(archive) {
            ValueT save_val = 0;
            if (this->value_ != 0)
                save_val = static_cast<ValueT>(this->Get()->internalId);

            CerealArchive<kArchiveSize_>(archive, save_val);
        }
    };

    template <class T, std::enable_if_t<!std::is_same_v<T, std::nullptr_t>, int> = 0>
    SerialProtoPtr(T) -> SerialProtoPtr<std::remove_pointer_t<T>>;


    ///
    /// Manages non owning pointer to unique data
    /// \remark Uses the global active unique data manager for deserializing
    template <typename TUnique>
    class SerialUniqueDataPtr : public core::PointerWrapper<TUnique>
    {
        //        static_assert(std::is_base_of_v<UniqueDataBase, TUnique>,
        //                      "TUnique must inherit UniqueDataBase for internal id");

        using ValueT                        = typename core::PointerWrapper<TUnique>::ValueT;
        static constexpr auto kArchiveSize_ = sizeof(data::UniqueDataIdT);

    public:
        using core::PointerWrapper<TUnique>::PointerWrapper;

        CEREAL_LOAD(archive) {
            data::UniqueDataIdT id;
            data::CerealArchive<kArchiveSize_>(archive, id);

            if (id == 0) // nullptr
                return;

            assert(data::active_unique_data_manager != nullptr);
            this->SetPtr(static_cast<TUnique*>(&data::active_unique_data_manager->RelocationTableGet(id)));
        }

        CEREAL_SAVE(archive) {
            const auto* unique_data = this->Get();

            data::UniqueDataIdT id = 0;
            if (unique_data != nullptr)
                id = unique_data->internalId;

            data::CerealArchive<kArchiveSize_>(archive, id);
        }
    };

    template <class T, std::enable_if_t<!std::is_same_v<T, std::nullptr_t>, int> = 0>
    SerialUniqueDataPtr(T) -> SerialUniqueDataPtr<std::remove_pointer_t<T>>;
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_CEREAL_SERIALIZATION_TYPE_H
