// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
#pragma once

#include <array>

#include "jactorio.h"

#include "game/world/chunk_tile_layer.h"
#include "game/world/tile_layer.h"

#include <cereal/types/array.hpp>

// Cannot include the data headers
namespace jactorio::proto
{
    class Tile;
    class Entity;
    class Sprite;
} // namespace jactorio::proto

namespace jactorio::game
{
    ///
    /// A tile in the world
    /// ! Be careful when adding members to this class, its size should be minimized ! This is created for every chunk
    class ChunkTile
    {
    public:
        ChunkTile() = default;


        J_NODISCARD ChunkTileLayer& GetLayer(const TileLayer layer) {
            return layers[GetLayerIndex(layer)];
        }

        J_NODISCARD const ChunkTileLayer& GetLayer(const TileLayer layer) const {
            return layers[GetLayerIndex(layer)];
        }


        J_NODISCARD ChunkTileLayer& GetLayer(const uint8_t layer_index) {
            return layers[layer_index];
        }

        J_NODISCARD const ChunkTileLayer& GetLayer(const uint8_t layer_index) const {
            return layers[layer_index];
        }


        static unsigned int GetLayerIndex(TileLayer category) {
            return static_cast<unsigned int>(category);
        }


        // ============================================================================================
        // Methods below are type checked to avoid getting / setting the wrong data

        // TODO Remove these method

        // chunk_layer::base only
        J_NODISCARD const proto::Tile* GetTilePrototype(TileLayer category = TileLayer::base) const;
        void SetTilePrototype(Orientation orientation,
                              const proto::Tile* tile_prototype,
                              TileLayer category = TileLayer::base);


        // chunk_layer::resource, chunk_layer::entity only
        J_NODISCARD const proto::Entity* GetEntityPrototype(TileLayer category = TileLayer::entity) const;
        void SetEntityPrototype(Orientation orientation,
                                const proto::Entity* tile_prototype,
                                TileLayer category = TileLayer::entity);


        CEREAL_SERIALIZE(archive) {
            archive(layers);
        }

        /// \remark To access prototype at each location, cast desired prototype_category to int and index
        /// tile_prototypes
        std::array<ChunkTileLayer, kTileLayerCount> layers;
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_CHUNK_TILE_H
