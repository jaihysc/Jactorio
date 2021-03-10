// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
#pragma once

#include "data/cereal/serialization_type.h"
#include "data/cereal/serialize.h"
#include "data/unique_data_manager.h"
#include "proto/framework/world_object.h"

#include <cereal/cereal.hpp>

namespace jactorio::game
{
    /// A tile within a chunk
    class ChunkTile
    {
        using TileDistanceT = uint8_t;

        using PrototypeContainerT  = data::SerialProtoPtr<const proto::FWorldObject>;
        using UniqueDataContainerT = std::unique_ptr<proto::FWorldObjectData>;

        using PrototypeT  = PrototypeContainerT::element_type;
        using UniqueDataT = UniqueDataContainerT::element_type;

    public:
        ChunkTile() = default;

        ~ChunkTile();

        ChunkTile(const ChunkTile& other);
        ChunkTile(ChunkTile&& other) noexcept;

        ChunkTile& operator=(ChunkTile other) noexcept {
            swap(*this, other);
            return *this;
        }


        friend void swap(ChunkTile& lhs, ChunkTile& rhs) noexcept {
            using std::swap;
            if (lhs.IsTopLeft()) {
                swap(lhs.data_.topLeft, rhs.data_.topLeft);
            }
            else {
                swap(lhs.data_.nonTopLeft, rhs.data_.nonTopLeft);
            }
            swap(lhs.common_, rhs.common_);
        }


        /// Resets data on this tile, becomes TopLeft again if previously NonTopLeft
        void Clear() noexcept;


        // ======================================================================

        /// Fetches orientation at current tile
        J_NODISCARD Orientation GetOrientation() const noexcept;


        // Prototype

        /// Sets prototype and orientation
        void SetPrototype(Orientation orientation, PrototypeT& prototype) noexcept;

        /// Sets prototype and orientation
        void SetPrototype(Orientation orientation, PrototypeT* prototype) noexcept;

        /// Sets prototype at current tile tile to nullptr
        void SetPrototype(std::nullptr_t) noexcept;

        /// \tparam T Return type which prototypeData is cast to
        template <typename T = PrototypeT>
        J_NODISCARD const T* GetPrototype() const noexcept;

        // Unique data

        /// Heap allocates unique data
        /// \return Created unique data
        template <typename TData, typename... Args>
        TData& MakeUniqueData(Args&&... args);

        /// Unique data at current tile or if multi tile, top left
        /// \tparam T Return type which uniqueData is cast to
        template <typename T = UniqueDataT>
        J_NODISCARD T* GetUniqueData() noexcept;

        /// Unique data at current tile or if multi tile, top left
        /// \tparam T Return type which uniqueData is cast to
        template <typename T = UniqueDataT>
        J_NODISCARD const T* GetUniqueData() const noexcept;


        // ======================================================================


        J_NODISCARD bool IsTopLeft() const noexcept; // Destructor use safe
        J_NODISCARD bool IsMultiTile() const noexcept;
        J_NODISCARD bool IsMultiTileTopLeft() const noexcept;

        /// Looks at prototype to determine dimensions, 1x1 if no prototype
        /// \return Dimensions of multi-tile, or 1x1 if single tile
        J_NODISCARD proto::FWorldObject::Dimension GetDimension() const noexcept;


        /// Turn or unturn ChunkTile to/from a multi tile
        /// Call prior to any operations involving multi tile index or top_left tile
        void SetupMultiTile(TileDistanceT multi_tile_index, ChunkTile& top_left) noexcept;

        J_NODISCARD TileDistanceT GetMultiTileIndex() const noexcept;

        J_NODISCARD ChunkTile* GetTopLeft() noexcept;
        J_NODISCARD const ChunkTile* GetTopLeft() const noexcept;


        //
        /// \return Number of tiles from top left on X axis
        J_NODISCARD TileDistanceT GetOffsetX() const noexcept;
        /// \return Number of tiles from top left on Y axis
        J_NODISCARD TileDistanceT GetOffsetY() const noexcept;


        CEREAL_LOAD(archive) {
            decltype(Common::multiTileIndex) multi_tile_index;
            archive(multi_tile_index);

            if (IsTopLeft(multi_tile_index)) {
                data::CerealArchive<sizeof(Common) + sizeof(TopLeft)>(archive, common_, data_.topLeft);

                const auto& unique_data = data_.topLeft.uniqueData;

                if (unique_data != nullptr) {
                    assert(data::active_unique_data_manager != nullptr);
                    data::active_unique_data_manager->StoreRelocationEntry(*unique_data);
                }
            }
            else {
                data::CerealArchive<sizeof(Common) + sizeof(NonTopLeft)>(archive, common_, data_.nonTopLeft);
            }
        }

        CEREAL_SAVE(archive) {
            archive(GetMultiTileIndex());

            if (IsTopLeft()) {
                const auto& unique_data = data_.topLeft.uniqueData;

                if (unique_data != nullptr) {
                    assert(data::active_unique_data_manager != nullptr);
                    data::active_unique_data_manager->AssignId(*unique_data);
                }
                data::CerealArchive<sizeof(Common) + sizeof(TopLeft)>(archive, common_, data_.topLeft);
            }
            else {
                data::CerealArchive<sizeof(Common) + sizeof(NonTopLeft)>(archive, common_, data_.nonTopLeft);
            }
        }

    private:
        /// Shared between top left and non top left
        struct Common
        {
            friend void swap(Common& lhs, Common& rhs) noexcept {
                using std::swap;
                swap(lhs.multiTileIndex, rhs.multiTileIndex);
                swap(lhs.prototype, rhs.prototype);
                swap(lhs.orientation, rhs.orientation);
            }

            /// If the tile is multi-tile, eg: 3 x 2
            /// 0 1 2
            /// 3 4 5
            TileDistanceT multiTileIndex = 0;

            /// Provide additional data (collisions, world gen)
            PrototypeContainerT prototype;

            Orientation orientation = Orientation::up;

            CEREAL_SERIALIZE(archive) {
                archive(multiTileIndex, prototype, orientation);
            }
        };

        struct TopLeft
        {
            TopLeft() = default;

            /// Does not copy uniqueData
            TopLeft(const TopLeft& other) noexcept;
            TopLeft(TopLeft&& other) noexcept = default;

            TopLeft& operator=(TopLeft other) {
                using std::swap;
                swap(*this, other);
                return *this;
            }

            friend void swap(TopLeft& lhs, TopLeft& rhs) noexcept {
                using std::swap;
                swap(lhs.uniqueData, rhs.uniqueData);
            }

            UniqueDataContainerT uniqueData;

            /// Copies this object, prototype must not be null if uniqueData is not null
            /// \param to UniqueData copied to here
            void CopyUniqueData(TopLeft& to, PrototypeT* prototype) const;

            CEREAL_SERIALIZE(archive) {
                archive(uniqueData);
            }
        };

        struct NonTopLeft
        {
            NonTopLeft() = default;

            NonTopLeft(const NonTopLeft& other);
            NonTopLeft(NonTopLeft&& other) noexcept = default;

            NonTopLeft& operator=(NonTopLeft other) {
                using std::swap;
                swap(*this, other);
                return *this;
            }

            friend void swap(NonTopLeft& lhs, NonTopLeft& rhs) noexcept {
                using std::swap;
                swap(lhs.topLeft, rhs.topLeft);
            }

            ChunkTile* topLeft = nullptr;

            CEREAL_SERIALIZE(archive) {
                // topLeft must be manually redetermined
            }
        };


        /// topLeft when multiTileIndex is 0, otherwise nonTopLeft
        union TileData
        {
            TileData() {
                ConstructTopLeft();
            }

            // Destruction managed by ChunkTile destructor
            ~TileData() {}

            void ConstructTopLeft() noexcept {
                new (&topLeft) TopLeft();
            }

            void DestructTopLeft() noexcept {
                topLeft.~TopLeft();
            }


            void ConstructNonTopLeft() noexcept {
                new (&nonTopLeft) NonTopLeft();
            }

            void DestructNonTopLeft() noexcept {
                nonTopLeft.~NonTopLeft();
            }

            TopLeft topLeft;
            NonTopLeft nonTopLeft;
        };

        Common common_;
        TileData data_;


        /// Sets orientation at current tile
        void SetOrientation(Orientation orientation) noexcept;

        J_NODISCARD static bool IsTopLeft(TileDistanceT multi_tile_index) noexcept;

        /// Returns data within the union topLeft
        J_NODISCARD TopLeft& AsTopLeft() noexcept;
        J_NODISCARD const TopLeft& AsTopLeft() const noexcept;

        /// Returns data within the union nonTopLeft
        J_NODISCARD NonTopLeft& AsNonTopLeft() noexcept;
        J_NODISCARD const NonTopLeft& AsNonTopLeft() const noexcept;
    };

    template <typename T>
    const T* ChunkTile::GetPrototype() const noexcept {
        return static_cast<const T*>(common_.prototype.Get());
    }


    template <typename TData, typename... Args>
    TData& ChunkTile::MakeUniqueData(Args&&... args) {
        static_assert(std::is_base_of_v<UniqueDataT, TData>);

        if (IsMultiTile())
            assert(IsMultiTileTopLeft());

        auto& self = AsTopLeft();
        assert(self.uniqueData == nullptr); // Trying to create already created uniqueData

        self.uniqueData = std::make_unique<TData>(std::forward<Args>(args)...);

        assert(self.uniqueData != nullptr);
        return static_cast<TData&>(*self.uniqueData.get());
    }

    template <typename T>
    T* ChunkTile::GetUniqueData() noexcept {
        if (!IsTopLeft()) {
            auto* tl_layer = AsNonTopLeft().topLeft;
            assert(tl_layer != nullptr);

            return static_cast<T*>(tl_layer->AsTopLeft().uniqueData.get());
        }

        return static_cast<T*>(AsTopLeft().uniqueData.get());
    }

    template <typename T>
    const T* ChunkTile::GetUniqueData() const noexcept {
        return const_cast<ChunkTile*>(this)->GetUniqueData<const T>();
    }


    template <typename TCoord, typename TIncrement>
    void Position2Increment(const ChunkTile& tile, TCoord& coord, const TIncrement increment) {
        if (!tile.IsMultiTile())
            return;

        coord.x -= tile.GetOffsetX() * increment;
        coord.y -= tile.GetOffsetY() * increment;
    }
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
