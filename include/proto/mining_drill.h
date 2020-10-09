// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_MINING_DRILL_H
#define JACTORIO_INCLUDE_PROTO_MINING_DRILL_H
#pragma once

#include "game/logic/deferral_timer.h"
#include "game/logic/item_logistics.h"
#include "proto/abstract_proto/health_entity.h"
#include "proto/detail/prototype_type.h"

namespace jactorio::proto
{
    struct MiningDrillData final : HealthEntityData
    {
        explicit MiningDrillData(const Orientation orientation) : output(orientation) {}

        game::ItemDropOff output;
        WorldCoord outputTile;

        data::SerialProtoPtr<const Item> outputItem = nullptr;


        /// Top left mining area coordinate
        WorldCoord resourceCoord;
        /// Resource to mine offset to the right wrapping down (wrap on miningRadius)
        uint16_t resourceOffset = 0;


        /// Base number of ticks to mine resource with no modifiers applied (mining speed, boosts, ...)
        uint16_t miningTicks = 1;

        game::DeferralTimer::DeferralEntry deferralEntry;


        CEREAL_SERIALIZE(archive) {
            archive(output.GetOrientation(),
                    outputTile,
                    outputItem,
                    resourceCoord,
                    resourceOffset,
                    miningTicks,
                    deferralEntry,
                    cereal::base_class<HealthEntityData>(this));
        }

        CEREAL_LOAD_CONSTRUCT(archive, construct, MiningDrillData) {
            Orientation orientation;
            archive(orientation);
            construct(orientation);

            archive(construct->outputTile,
                    construct->outputItem,
                    construct->resourceCoord,
                    construct->resourceOffset,
                    construct->miningTicks,
                    construct->deferralEntry,
                    cereal::base_class<HealthEntityData>(construct.ptr()));
        }
    };


    ///
    /// Drill, Mines resource entities
    class MiningDrill final : public HealthEntity
    {
        /*
         * 0  - 7 : North
         * 8  - 15: East
         * 16 - 23: South
         * 24 - 31: West
         */
    public:
        PROTOTYPE_CATEGORY(mining_drill);

        /// Mining ticks divided by this
        PYTHON_PROP_REF_I(ProtoFloatT, miningSpeed, 1.);

        /// Number of tiles to extend the mining radius around the entity outside of entity tile width and height
        PYTHON_PROP_REF_I(uint16_t, miningRadius, 1);

        PYTHON_PROP(Tile4Way, resourceOutput);


        // ======================================================================
        // Rendering

        bool OnRShowGui(const render::GuiRenderer& g_rendr, game::ChunkTileLayer* tile_layer) const override;


        J_NODISCARD Sprite* OnRGetSprite(SpriteSetT set) const override;

        J_NODISCARD SpriteSetT OnRGetSpriteSet(Orientation orientation,
                                               game::WorldData& world_data,
                                               const WorldCoord& world_coords) const override;

        J_NODISCARD SpriteFrameT OnRGetSpriteFrame(const UniqueDataBase& unique_data,
                                                   GameTickT game_tick) const override;

        // ======================================================================
        // Logic
        ///
        /// Finds the FIRST output item of the mining drill, beginning from top left
        J_NODISCARD Item* FindOutputItem(const game::WorldData& world_data, WorldCoord world_pair) const;

        void OnDeferTimeElapsed(game::WorldData& world_data,
                                game::LogicData& logic_data,
                                UniqueDataBase* unique_data) const override;

        ///
        /// Ensures that the mining radius covers a resource entity
        J_NODISCARD bool OnCanBuild(const game::WorldData& world_data, const WorldCoord& world_coords) const override;

        void OnBuild(game::WorldData& world_data,
                     game::LogicData& logic_data,
                     const WorldCoord& world_coords,
                     game::ChunkTileLayer& tile_layer,
                     Orientation orientation) const override;

        void OnNeighborUpdate(game::WorldData& world_data,
                              game::LogicData& logic_data,
                              const WorldCoord& emit_world_coords,
                              const WorldCoord& receive_world_coords,
                              Orientation emit_orientation) const override;

        void OnRemove(game::WorldData& world_data,
                      game::LogicData& logic_data,
                      const WorldCoord& world_coords,
                      game::ChunkTileLayer& tile_layer) const override;

        void OnDeserialize(game::WorldData& world_data,
                           const WorldCoord& world_coord,
                           game::ChunkTileLayer& tile_layer) const override;


        void PostLoadValidate(const data::PrototypeManager& proto_manager) const override;
        void ValidatedPostLoad() override;

    private:
        static bool InitializeOutput(game::WorldData& world_data,
                                     const WorldCoord& output_coord,
                                     MiningDrillData* drill_data);

        J_NODISCARD WorldCoord GetOutputCoord(const WorldCoord& world_coord, Orientation orientation) const;

        J_NODISCARD int GetMiningAreaX() const;
        J_NODISCARD int GetMiningAreaY() const;

        ///
        /// Sets up drill data such that resources can be deducted from the ground
        /// \return true if a resource was found, otherwise false
        bool SetupResourceDeduction(const game::WorldData& world_data, MiningDrillData& drill_data) const;

        ///
        /// Removes resource using resourceCoord + resourceOffset in drill_data, searches for another resource if
        /// depleted \return true if successful
        bool DeductResource(game::WorldData& world_data,
                            MiningDrillData& drill_data,
                            ResourceEntityResourceCount amount = 1) const;

        ///
        /// Sets up deferred callback for when it has mined a resource
        void RegisterMineCallback(game::DeferralTimer& timer, MiningDrillData* unique_data) const;

        ///
        /// Sets up deferred callback for when it has mined a resource, but failed to output
        void RegisterOutputCallback(game::DeferralTimer& timer, MiningDrillData* unique_data) const;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_MINING_DRILL_H
