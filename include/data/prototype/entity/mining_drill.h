// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_MINING_DRILL_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_MINING_DRILL_H
#pragma once

#include <optional>

#include "data/prototype/prototype_type.h"
#include "data/prototype/entity/health_entity.h"
#include "data/prototype/interface/deferred.h"
#include "game/logic/item_logistics.h"

namespace jactorio::data
{
	struct MiningDrillData final : HealthEntityData
	{
		explicit MiningDrillData(game::ItemDropOff output_tile)
			: outputTile(std::move(output_tile)) {
		}

		game::ItemDropOff outputTile;
		WorldCoord outputTileCoords{};

		Item* outputItem = nullptr;

		/// Base number of ticks to mine resource with no modifiers applied (mining speed, boosts, ...)
		uint16_t miningTicks = 1;

		game::LogicData::DeferralTimer::DeferralEntry deferralEntry{};
	};


	///
	/// \brief Drill, Mines resource entities
	class MiningDrill final : public HealthEntity, public IDeferred
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
		PYTHON_PROP_REF_I(MiningDrill, double, miningSpeed, 1.f);

		/// Number of tiles to extend the mining radius around the entity outside of entity tile width and height	
		PYTHON_PROP_REF_I(MiningDrill, uint16_t, miningRadius, 1);

		PYTHON_PROP(MiningDrill, Tile4Way, resourceOutput);


		// ======================================================================
		// Rendering

		bool OnRShowGui(game::PlayerData& player_data, const PrototypeManager& data_manager,
		                game::ChunkTileLayer* tile_layer) const override;


		J_NODISCARD Sprite* OnRGetSprite(Sprite::SetT set) const override;

		J_NODISCARD Sprite::SetT OnRGetSpriteSet(Orientation orientation, game::WorldData& world_data,
		                                         const WorldCoord& world_coords) const override;

		J_NODISCARD Sprite::FrameT OnRGetSpriteFrame(const UniqueDataBase& unique_data, GameTickT game_tick) const override;

		// ======================================================================
		// Logic
		///
		/// \briefs Finds the FIRST output item of the mining drill, beginning from top left
		J_NODISCARD Item* FindOutputItem(const game::WorldData& world_data, WorldCoord world_pair) const;

		void OnDeferTimeElapsed(game::WorldData& world_data, game::LogicData& logic_data,
		                        UniqueDataBase* unique_data) const override;

		///
		/// \brief Ensures that the mining radius covers a resource entity
		J_NODISCARD bool OnCanBuild(const game::WorldData& world_data,
		                            const WorldCoord& world_coords) const
		override;

		void OnBuild(game::WorldData& world_data,
		             game::LogicData& logic_data,
		             const WorldCoord& world_coords,
		             game::ChunkTileLayer& tile_layer, Orientation orientation) const override;

		void OnNeighborUpdate(game::WorldData& world_data,
		                      game::LogicData& logic_data,
		                      const WorldCoord& emit_world_coords,
		                      const WorldCoord& receive_world_coords,
		                      Orientation emit_orientation) const override;

		void OnRemove(game::WorldData& world_data,
		              game::LogicData& logic_data,
		              const WorldCoord& world_coords, game::ChunkTileLayer& tile_layer) const override;


		void PostLoadValidate(const PrototypeManager&) const override {
			J_DATA_ASSERT(sprite != nullptr, "North sprite not provided");
			J_DATA_ASSERT(spriteE != nullptr, "East sprite not provided");
			J_DATA_ASSERT(spriteS != nullptr, "South sprite not provided");
			J_DATA_ASSERT(spriteW != nullptr, "West sprite not provided");
		}

		void ValidatedPostLoad() override {
			sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
			spriteE->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
			spriteS->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
			spriteW->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
		}

	private:
		///
		/// \brief Sets up deferred callback for when it has mined a resource 
		void RegisterMineCallback(game::LogicData::DeferralTimer& timer, MiningDrillData* unique_data) const;

		///
		/// \brief Sets up deferred callback for when it has mined a resource, but failed to output
		void RegisterOutputCallback(game::LogicData::DeferralTimer& timer, MiningDrillData* unique_data) const;
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_MINING_DRILL_H
