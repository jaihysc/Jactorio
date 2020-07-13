// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/30/2020

#include "data/prototype/entity/assembly_machine.h"

#include "renderer/gui/gui_menus.h"

using namespace jactorio;

void data::AssemblyMachineData::ChangeRecipe(game::LogicData& logic_data, const PrototypeManager& data_manager,
                                             const Recipe* new_recipe) {
	if (new_recipe) {
		ingredientInv.resize(new_recipe->ingredients.size());
		productInv.resize(1);

		// Set filters
		for (size_t i = 0; i < ingredientInv.size(); ++i) {
			ingredientInv[i].filter = data_manager.DataRawGet<Item>(new_recipe->ingredients[i].first);
		}
		productInv[0].filter = data_manager.DataRawGet<Item>(new_recipe->product.first);
	}
	else {
		// Remove recipe
		logic_data.deferralTimer.RemoveDeferralEntry(deferralEntry);

		ingredientInv.resize(0);
		productInv.resize(0);
	}

	recipe_ = new_recipe;
}

bool data::AssemblyMachineData::CanBeginCrafting() const {
	if (!recipe_)
		return false;

	for (size_t i = 0; i < recipe_->ingredients.size(); ++i) {
		const auto& i_possessed = ingredientInv[i];
		const auto& i_required = recipe_->ingredients[i];

		// No item, Wrong item or not enough of item
		if (!i_possessed.item ||
			i_possessed.item->name != i_required.first ||
			i_possessed.count < i_required.second) {
			return false;
		}
	}
	
	return true;
}

// ======================================================================

data::Sprite::SetT data::AssemblyMachine::OnRGetSet(
	Orientation,
	game::WorldData&,
	const game::WorldData::WorldPair&) const {
	return 0;
}

data::IRenderable::SpritemapFrame data::AssemblyMachine::OnRGetSprite(
	const UniqueDataBase* unique_data, GameTickT game_tick) const {

	const auto* machine_data = static_cast<const AssemblyMachineData*>(unique_data);

	if (machine_data->deferralEntry.second == 0)
		game_tick = 0;

	return AllOfSprite(*sprite, game_tick, 1.f / 6);
}


bool data::AssemblyMachine::OnRShowGui(game::PlayerData& player_data, const PrototypeManager& data_manager,
                                       game::ChunkTileLayer* tile_layer) const {
	renderer::AssemblyMachine(player_data, data_manager, this, tile_layer->GetUniqueData());
	return true;
}

// ======================================================================

bool data::AssemblyMachine::TryBeginCrafting(game::LogicData& logic_data,
											 AssemblyMachineData& data) const {
	if (!data.CanBeginCrafting())
		return false;

	data.deferralEntry = logic_data.deferralTimer.RegisterFromTick(
		*this,
		&data,
		data.GetRecipe()->GetCraftingTime(1.f / this->assemblySpeed));

	return true;
}


void data::AssemblyMachine::OnDeferTimeElapsed(game::WorldData& world_data, game::LogicData& logic_data,
											   UniqueDataBase* unique_data) const {
	LOG_MESSAGE(debug, "Ding!");
}

void data::AssemblyMachine::OnBuild(game::WorldData& world_data,
                                    game::LogicData& logic_data,
                                    const game::WorldData::WorldPair& world_coords,
                                    game::ChunkTileLayer& tile_layer, const Orientation orientation) const {
	auto* data = tile_layer.MakeUniqueData<AssemblyMachineData>();

	data->set = OnRGetSet(orientation, world_data, world_coords);
}

void data::AssemblyMachine::OnRemove(game::WorldData& world_data,
                                     game::LogicData& logic_data,
                                     const game::WorldData::WorldPair& world_coords,
									 game::ChunkTileLayer& tile_layer) const {
	auto& machine_data = *tile_layer.GetMultiTileTopLeft()->GetUniqueData<AssemblyMachineData>();

	logic_data.deferralTimer.RemoveDeferralEntry(machine_data.deferralEntry);
}
