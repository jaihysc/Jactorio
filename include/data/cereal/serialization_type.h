// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_CEREAL_SERIALIZATION_TYPE_H
#define JACTORIO_INCLUDE_DATA_CEREAL_SERIALIZATION_TYPE_H
#pragma once

#include "core/convert.h"
#include "core/pointer_wrapper.h"
#include "data/cereal/serialize.h"
#include "data/globals.h"
#include "data/prototype_manager.h"
#include "data/unique_data_manager.h"

namespace jactorio::data
{
    /// Manages non owning pointer to prototype
    /// \remark Uses the global active prototype manager for deserializing
    /// Converts pointer to internal id when serializing, from internal id to pointer when deserializing
    template <typename TProto>
    class SerialProtoPtr : public PointerWrapper<TProto>
    {
        static_assert(std::is_base_of_v<proto::FrameworkBase, TProto>,
                      "TProto must inherit FrameworkBase for internal id");

        using ValueT                       = typename PointerWrapper<TProto>::ValueT;
        static constexpr auto kArchiveSize = sizeof(ValueT);

    public:
        using PointerWrapper<TProto>::PointerWrapper;

        CEREAL_LOAD(archive) {
            CerealArchive<kArchiveSize>(archive, this->value_); // Deserialized as internal id

            if (this->value_ == 0) // nullptr
                return;

            assert(active_prototype_manager != nullptr);
            auto* proto_ptr = &active_prototype_manager->RelocationTableGet<TProto>( // Converted to prototype*
                SafeCast<PrototypeIdT>(this->value_));
            this->SetPtr(proto_ptr);
        }

        CEREAL_SAVE(archive) {
            ValueT save_val = 0;
            if (this->value_ != 0)
                save_val = static_cast<ValueT>(this->Get()->internalId);

            CerealArchive<kArchiveSize>(archive, save_val);
        }
    };

    template <class T, std::enable_if_t<!std::is_same_v<T, std::nullptr_t>, int> = 0>
    SerialProtoPtr(T) -> SerialProtoPtr<std::remove_pointer_t<T>>;


    /// Manages non owning pointer to unique data
    /// \remark Uses the global active unique data manager for deserializing
    template <typename TUnique>
    class SerialUniqueDataPtr : public PointerWrapper<TUnique>
    {
        static_assert(std::is_base_of_v<proto::UniqueDataBase, TUnique>,
                      "TUnique must inherit UniqueDataBase for internal id");

        using ValueT                       = typename PointerWrapper<TUnique>::ValueT;
        static constexpr auto kArchiveSize = sizeof(UniqueDataIdT);

    public:
        using PointerWrapper<TUnique>::PointerWrapper;

        CEREAL_LOAD(archive) {
            UniqueDataIdT id;
            data::CerealArchive<kArchiveSize>(archive, id);

            if (id == 0) // nullptr
                return;

            assert(data::active_unique_data_manager != nullptr);
            this->SetPtr(static_cast<TUnique*>(&active_unique_data_manager->RelocationTableGet(id)));
        }

        CEREAL_SAVE(archive) {
            const auto* unique_data = this->Get();

            UniqueDataIdT id = 0;
            if (unique_data != nullptr)
                id = unique_data->internalId;

            data::CerealArchive<kArchiveSize>(archive, id);
        }
    };

    template <class T, std::enable_if_t<!std::is_same_v<T, std::nullptr_t>, int> = 0>
    SerialUniqueDataPtr(T) -> SerialUniqueDataPtr<std::remove_pointer_t<T>>;
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_CEREAL_SERIALIZATION_TYPE_H
