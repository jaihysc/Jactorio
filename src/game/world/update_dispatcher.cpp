// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/update_dispatcher.h"

using namespace jactorio;

game::UpdateDispatcher::ListenerEntry game::UpdateDispatcher::Register(const WorldCoord& current_coord,
                                                                       const WorldCoord& target_coord,
                                                                       const proto::FEntity& proto_listener) {

    auto& collection = container_[std::make_tuple(target_coord.x, target_coord.y)];
    collection.emplace_back(CollectionElement{current_coord, CallbackT(proto_listener)});

    return {current_coord, target_coord};
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

void game::UpdateDispatcher::Dispatch(World& world, const WorldCoord& coord, const proto::UpdateType type) {
    // Must be tuple to index into container_ since it uses a hash function only usable with tuples
    const auto world_tuple = std::make_tuple(coord.x, coord.y);

    if (container_.find(world_tuple) == container_.end())
        return;

    auto& collection = container_[world_tuple];

    for (auto& entry : collection) {
        entry.callback->OnTileUpdate(world, coord, entry.receiver, type);
    }
}

game::UpdateDispatcher::DebugInfo game::UpdateDispatcher::GetDebugInfo() const noexcept {
    return {container_};
}
