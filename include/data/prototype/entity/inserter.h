// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/17/2020

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_INSERTER_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_INSERTER_H
#pragma once

#include <decimal.h>

#include "data/prototype/entity/health_entity.h"

namespace jactorio::data
{
	class Inserter final : public HealthEntity
	{
		static constexpr auto kInserterRotationDecimals = 3;
	public:
		PROTOTYPE_CATEGORY(inserter);

		using RotationDegree = dec::decimal<kInserterRotationDecimals>;

		J_NODISCARD static RotationDegree ToRotationDegree(const double val) {
			return dec::decimal_cast<kInserterRotationDecimals>(val);
		}


		///
		/// \brief Degrees to rotate per tick 
		/// \remark For Python API use only
		PYTHON_PROP_I(Inserter, double, rotationSpeedFloat, 0.1f);

		///
		/// \brief Degrees to rotate per tick 
		PYTHON_PROP_REF(Inserter, RotationDegree, rotationSpeed);


		void PostLoad() override {
			rotationSpeed = ToRotationDegree(rotationSpeedFloat);
		}


		// ======================================================================
		void OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const override;

		std::pair<uint16_t, uint16_t> MapPlacementOrientation(Orientation orientation,
		                                                      game::WorldData& world_data,
		                                                      const game::WorldData::WorldPair& world_coords) const override;


		void OnBuild(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
		             game::ChunkTileLayer& tile_layer, Orientation orientation) const override;

		void OnRemove(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
		              game::ChunkTileLayer& tile_layer) const override;

		void ValidatedPostLoad() override {
			sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
		}
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_INSERTER_H
