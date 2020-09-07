// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/update_dispatcher.h"

using namespace jactorio;

game::UpdateDispatcher::ListenerEntry game::UpdateDispatcher::Register(const WorldCoordAxis current_world_x,
                                                                       const WorldCoordAxis current_world_y,
                                                                       const WorldCoordAxis target_world_x,
                                                                       const WorldCoordAxis target_world_y,
                                                                       const data::FEntity& proto_listener) {

    return Register({current_world_x, current_world_y}, {target_world_x, target_world_y}, proto_listener);
}

game::UpdateDispatcher::ListenerEntry game::UpdateDispatcher::Register(const WorldCoord& current_coords,
                                                                       const WorldCoord& target_coords,
                                                                       const data::FEntity& proto_listener) {

    auto& collection = container_[std::make_tuple(target_coords.x, target_coords.y)];
    collection.emplace_back(CollectionElement{current_coords, CallbackT(proto_listener)});

    return {current_coords, target_coords};
}

bool game::UpdateDispatcher::Unregister(const ListenerEntry& entry) {
    const auto world_tuple = std::make_tuple(entry.emitter.x, entry.emitter.y);
    auto& collection       = container_[world_tuple];

    // Collection may be erased, thus its size cannot be checked during the for loop
    auto collection_size = collection.size();

    for (decltype(collection.size()) i = 0; i < collection_size; ++i) {
        auto& element = collection[i];

        if (element.receiver == entry.receiver) {
            collection.erase(collection.begin() + i);
            collection_size = collection.size(); // Collection shrunk, thus max size must be updated

            if (collection.empty())
                container_.erase(world_tuple);
        }
    }

    return false;
}

void game::UpdateDispatcher::Dispatch(WorldData& world_data,
                                      const WorldCoordAxis world_x,
                                      const WorldCoordAxis world_y,
                                      const data::UpdateType type) {
    Dispatch(world_data, {world_x, world_y}, type);
}

void game::UpdateDispatcher::Dispatch(WorldData& world_data,
                                      const WorldCoord& world_pair,
                                      const data::UpdateType type) {
    // Must be tuple to index into container_ since it uses a hash function only usable with tuples
    const auto world_tuple = std::make_tuple(world_pair.x, world_pair.y);

    if (container_.find(world_tuple) == container_.end())
        return;

    auto& collection = container_[world_tuple];

    for (auto& entry : collection) {
        entry.callback->OnTileUpdate(world_data, world_pair, entry.receiver, type);
    }
}

game::UpdateDispatcher::DebugInfo game::UpdateDispatcher::GetDebugInfo() const noexcept {
    return {container_};
}
