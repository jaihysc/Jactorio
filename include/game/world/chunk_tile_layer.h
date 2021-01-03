// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
#pragma once

#include "data/cereal/serialization_type.h"
#include "data/cereal/serialize.h"
#include "data/unique_data_manager.h"
#include "proto/framework/world_object.h"

#include <cereal/cereal.hpp>

namespace jactorio::game
{
    struct MultiTileData
    {
        using ValueT = uint8_t;

        /// Number of tiles the sprite should span
        uint8_t span   = 1;
        uint8_t height = 1;

        friend bool operator==(const MultiTileData& lhs, const MultiTileData& rhs) {
            return std::tie(lhs.span, lhs.height) == std::tie(rhs.span, rhs.height);
        }

        friend bool operator!=(const MultiTileData& lhs, const MultiTileData& rhs) {
            return !(lhs == rhs);
        }
    };

    ///
    /// A Layers within a ChunkTile layers
    class ChunkTileLayer
    {
        using MultiTileValueT = MultiTileData::ValueT;

        using PrototypeContainerT  = data::SerialProtoPtr<const proto::FWorldObject>;
        using UniqueDataContainerT = std::unique_ptr<proto::UniqueDataBase>;

        using PrototypeT = PrototypeContainerT::element_type;

    public:
        ChunkTileLayer() = default;

        explicit ChunkTileLayer(const proto::FWorldObject* proto) : prototypeData_(proto) {}

        ~ChunkTileLayer();

        ChunkTileLayer(const ChunkTileLayer& other);
        ChunkTileLayer(ChunkTileLayer&& other) noexcept;

        ChunkTileLayer& operator=(ChunkTileLayer other) {
            swap(*this, other);
            return *this;
        }


        friend void swap(ChunkTileLayer& lhs, ChunkTileLayer& rhs) noexcept {
            using std::swap;
            swap(lhs.multiTileIndex_, rhs.multiTileIndex_);
            swap(lhs.prototypeData_, rhs.prototypeData_);
            swap(lhs.data_.uniqueData, rhs.data_.uniqueData);
            swap(lhs.orientation_, rhs.orientation_);
        }


        ///
        /// Resets data on this tile and frees any heap allocated data
        void Clear() noexcept;


        // ======================================================================

        ///
        /// Sets orientation at current layer or if multi tile, top left
        void SetOrientation(Orientation orientation);

        ///
        /// Fetches orientation at current layer or if multi tile, top left
        J_NODISCARD Orientation GetOrientation() const;


        // Prototype

        ///
        /// Sets orientation and orientation for current tile layer
        void SetPrototype(Orientation orientation, PrototypeT* prototype);

        ///
        /// Sets prototype at current tile layer to nullptr
        ///
        /// Behaves the same as SetPrototype(Orientation, PrototypeT*) excluding orientation
        void SetPrototype(std::nullptr_t);

        ///
        /// \tparam T Return type which prototypeData is cast to
        template <typename T = PrototypeT>
        J_NODISCARD const T* GetPrototype() const noexcept;

        // Unique data

        ///
        /// Heap allocates unique data
        /// \return Created unique data
        template <typename TData, typename... Args>
        TData& MakeUniqueData(Args&&... args);

        ///
        /// Unique data at current layer or if multi tile, top left
        /// \tparam T Return type which uniqueData is cast to
        template <typename T = proto::FWorldObjectData>
        J_NODISCARD T* GetUniqueData() noexcept;

        ///
        /// Unique data at current layer or if multi tile, top left
        /// \tparam T Return type which uniqueData is cast to
        template <typename T = proto::FWorldObjectData>
        J_NODISCARD const T* GetUniqueData() const noexcept;


        ///
        /// Unique data at current layer
        /// \tparam T Return type which uniqueData is cast to
        template <typename T = proto::FWorldObjectData>
        J_NODISCARD T* GetUniqueDataLocal() noexcept;

        ///
        /// Unique data at current layer
        /// \tparam T Return type which uniqueData is cast to
        template <typename T = proto::FWorldObjectData>
        J_NODISCARD const T* GetUniqueDataLocal() const noexcept;


        // ======================================================================


        J_NODISCARD bool IsTopLeft() const noexcept; // Destructor use safe
        J_NODISCARD bool IsMultiTile() const;
        J_NODISCARD bool IsMultiTileTopLeft() const;
        J_NODISCARD bool IsNonTopLeftMultiTile() const noexcept;


        J_NODISCARD bool HasMultiTileData() const;
        J_NODISCARD MultiTileData GetMultiTileData() const;


        J_NODISCARD MultiTileValueT GetMultiTileIndex() const noexcept;
        void SetMultiTileIndex(MultiTileValueT multi_tile_index);

        ///
        /// \remark Ensure multi tile index is set prior to calling
        J_NODISCARD ChunkTileLayer* GetTopLeftLayer() const noexcept;
        ///
        /// \remark Ensure multi tile index is set prior to calling
        void SetTopLeftLayer(ChunkTileLayer& ctl) noexcept;


        ///
        /// Adjusts provided x, y to coordinates of top left tile
        template <typename Tx, typename Ty>
        void AdjustToTopLeft(Tx& x, Ty& y) const;

        void AdjustToTopLeft(WorldCoord& coord) const;


        ///
        /// \return Number of tiles from top left on X axis
        J_NODISCARD MultiTileValueT GetOffsetX() const;
        ///
        /// \return Number of tiles from top left on Y axis
        J_NODISCARD MultiTileValueT GetOffsetY() const;

    private:
        /// uniqueData when multiTileIndex == 0, topLeft when multiTileIndex != 0
        union UData
        {
            static_assert(sizeof(ChunkTileLayer*) == sizeof(UniqueDataContainerT));

            // Memory management done by ChunkTileLayer
            UData() {
                ConstructUniqueData();
            }

            ~UData() {}

            void ConstructUniqueData() noexcept {
                new (&uniqueData) UniqueDataContainerT{};
            }

            void DestroyUniqueData() noexcept {
                uniqueData.~UniqueDataContainerT();
            }


            UniqueDataContainerT uniqueData;
            ChunkTileLayer* topLeft;
        };

        /// A layer may point to a tile prototype to provide additional data (collisions, world gen)
        PrototypeContainerT prototypeData_;

        UData data_;

        ///
        /// If the layer is multi-tile, eg: 3 x 2
        /// 0 1 2
        /// 3 4 5
        MultiTileValueT multiTileIndex_ = 0;

        Orientation orientation_ = Orientation::up;


        static constexpr size_t GetArchiveSize() {
            return sizeof prototypeData_ + sizeof data_.uniqueData + sizeof multiTileIndex_ + sizeof orientation_;
        }

    public:
        CEREAL_LOAD(archive) {
            constexpr auto archive_size = GetArchiveSize();
            data::CerealArchive<archive_size>(archive, prototypeData_, data_.uniqueData, multiTileIndex_, orientation_);

            const auto& unique_data = data_.uniqueData;

            if (unique_data != nullptr) {
                assert(data::active_unique_data_manager != nullptr);
                data::active_unique_data_manager->StoreRelocationEntry(*unique_data);
            }
        }

        CEREAL_SAVE(archive) {
            constexpr auto archive_size = GetArchiveSize();
            if (IsTopLeft()) {
                const auto& unique_data = data_.uniqueData;

                if (unique_data != nullptr) {
                    assert(data::active_unique_data_manager != nullptr);
                    data::active_unique_data_manager->AssignId(*unique_data);
                }
                data::CerealArchive<archive_size>(archive, prototypeData_, unique_data, multiTileIndex_, orientation_);
            }
            else {
                // Non top left tiles do not have unique data
                UniqueDataContainerT empty_unique = nullptr;
                data::CerealArchive<archive_size>(archive, prototypeData_, empty_unique, multiTileIndex_, orientation_);
            }
        }
    };

    template <typename T>
    const T* ChunkTileLayer::GetPrototype() const noexcept {
        return static_cast<const T*>(prototypeData_.Get());
    }


    template <typename TData, typename... Args>
    TData& ChunkTileLayer::MakeUniqueData(Args&&... args) {
        static_assert(std::is_base_of_v<proto::FWorldObjectData, TData>);

        if (IsMultiTile())
            assert(IsMultiTileTopLeft());
        assert(!data_.uniqueData); // Trying to create already created uniqueData

        data_.uniqueData = std::make_unique<TData>(std::forward<Args>(args)...);

        assert(data_.uniqueData != nullptr);
        return static_cast<TData&>(*data_.uniqueData.get());
    }

    template <typename T>
    T* ChunkTileLayer::GetUniqueData() noexcept {
        if (IsNonTopLeftMultiTile()) {
            assert(data_.topLeft != nullptr);
            return data_.topLeft->GetUniqueDataLocal<T>();
        }

        return GetUniqueDataLocal<T>();
    }

    template <typename T>
    const T* ChunkTileLayer::GetUniqueData() const noexcept {
        return const_cast<ChunkTileLayer*>(this)->GetUniqueData<const T>();
    }

    template <typename T>
    T* ChunkTileLayer::GetUniqueDataLocal() noexcept {
        if (IsMultiTile())
            assert(IsMultiTileTopLeft());

        return static_cast<T*>(data_.uniqueData.get());
    }

    template <typename T>
    const T* ChunkTileLayer::GetUniqueDataLocal() const noexcept {
        return const_cast<ChunkTileLayer*>(this)->GetUniqueDataLocal<const T>();
    }


    template <typename Tx, typename Ty>
    void ChunkTileLayer::AdjustToTopLeft(Tx& x, Ty& y) const {
        if (!IsMultiTile())
            return;

        x -= GetOffsetX();
        y -= GetOffsetY();
    }
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_LAYER_H
