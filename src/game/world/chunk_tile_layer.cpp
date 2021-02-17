// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk_tile_layer.h"

#include "core/coordinate_tuple.h"

using namespace jactorio;

game::ChunkTileLayer::~ChunkTileLayer() {
    if (IsTopLeft()) {
        data_.DestructTopLeft();
    }
    else {
        data_.DestructNonTopLeft();
    }
}

game::ChunkTileLayer::ChunkTileLayer(const ChunkTileLayer& other) : common_(other.common_) {
    if (other.IsTopLeft()) {
        AsTopLeft() = other.AsTopLeft();
        other.AsTopLeft().CopyUniqueData(AsTopLeft(), GetPrototype());
    }
    else {
        AsNonTopLeft() = other.AsNonTopLeft();
    }
}

game::ChunkTileLayer::ChunkTileLayer(ChunkTileLayer&& other) noexcept : common_(other.common_) {
    if (IsTopLeft()) {
        AsTopLeft() = std::move(other.AsTopLeft());
    }
    else {
        AsNonTopLeft() = std::move(other.AsNonTopLeft());
    }
}


void game::ChunkTileLayer::Clear() noexcept {
    if (IsTopLeft()) {
        data_.DestructTopLeft();
    }
    else {
        data_.DestructNonTopLeft();
    }

    data_.ConstructTopLeft();

    common_.~Common();
    new (&common_) Common();
}

void game::ChunkTileLayer::SetOrientation(const Orientation orientation) noexcept {
    common_.orientation = orientation;
}

Orientation game::ChunkTileLayer::GetOrientation() const noexcept {
    return common_.orientation;
}

void game::ChunkTileLayer::SetPrototype(const Orientation orientation, PrototypeT& prototype) noexcept {
    SetPrototype(orientation, &prototype);
}

void game::ChunkTileLayer::SetPrototype(const Orientation orientation, PrototypeT* prototype) noexcept {
    SetOrientation(orientation);
    common_.prototype = prototype;
}

void game::ChunkTileLayer::SetPrototype(std::nullptr_t) noexcept {
    common_.prototype = nullptr;
}

// ======================================================================

bool game::ChunkTileLayer::IsTopLeft() const noexcept {
    return IsTopLeft(common_.multiTileIndex);
}

bool game::ChunkTileLayer::IsMultiTile() const noexcept {
    if (GetMultiTileIndex() > 0)
        return true;

    return GetDimensions().span != 1 || GetDimensions().height != 1;
}

bool game::ChunkTileLayer::IsMultiTileTopLeft() const noexcept {
    return IsMultiTile() && IsTopLeft();
}

bool game::ChunkTileLayer::IsNonTopLeft() const noexcept {
    return GetMultiTileIndex() != 0;
}


game::MultiTileData game::ChunkTileLayer::GetDimensions() const noexcept {
    if (GetPrototype() == nullptr) {
        return {1, 1};
    }
    return {GetPrototype()->GetWidth(GetOrientation()), GetPrototype()->GetHeight(GetOrientation())};
}

void game::ChunkTileLayer::SetupMultiTile(const TileDistanceT multi_tile_index, ChunkTileLayer& top_left) noexcept {
    assert(multi_tile_index > 0);

    data_.DestructTopLeft();
    data_.ConstructNonTopLeft();

    common_.multiTileIndex = multi_tile_index;

    assert(IsNonTopLeft());
    assert(&top_left != this);
    AsNonTopLeft().topLeft = &top_left;
}


game::ChunkTileLayer::TileDistanceT game::ChunkTileLayer::GetMultiTileIndex() const noexcept {
    return common_.multiTileIndex;
}

game::ChunkTileLayer* game::ChunkTileLayer::GetTopLeftLayer() noexcept {
    return const_cast<ChunkTileLayer*>(static_cast<const ChunkTileLayer*>(this)->GetTopLeftLayer());
}

const game::ChunkTileLayer* game::ChunkTileLayer::GetTopLeftLayer() const noexcept {
    if (IsTopLeft())
        return this;

    assert(IsNonTopLeft());
    return AsNonTopLeft().topLeft;
}


// ======================================================================


game::ChunkTileLayer::TileDistanceT game::ChunkTileLayer::GetOffsetX() const noexcept {
    const auto& data = GetDimensions();
    return GetMultiTileIndex() % data.span;
}

game::ChunkTileLayer::TileDistanceT game::ChunkTileLayer::GetOffsetY() const noexcept {
    const auto& data = GetDimensions();
    return GetMultiTileIndex() / data.span;
}

bool game::ChunkTileLayer::IsTopLeft(const TileDistanceT multi_tile_index) noexcept {
    return multi_tile_index == 0;
}

game::ChunkTileLayer::TopLeft& game::ChunkTileLayer::AsTopLeft() noexcept {
    assert(IsTopLeft());
    return data_.topLeft;
}

const game::ChunkTileLayer::TopLeft& game::ChunkTileLayer::AsTopLeft() const noexcept {
    assert(IsTopLeft());
    return data_.topLeft;
}

game::ChunkTileLayer::NonTopLeft& game::ChunkTileLayer::AsNonTopLeft() noexcept {
    assert(IsNonTopLeft());
    return data_.nonTopLeft;
}

const game::ChunkTileLayer::NonTopLeft& game::ChunkTileLayer::AsNonTopLeft() const noexcept {
    assert(IsNonTopLeft());
    return data_.nonTopLeft;
}

// ======================================================================

game::ChunkTileLayer::TopLeft::TopLeft(const TopLeft& /*other*/) noexcept {}

void game::ChunkTileLayer::TopLeft::CopyUniqueData(TopLeft& to, PrototypeT* prototype) const {
    // Use prototype defined method for copying uniqueData_ if other has data to copy
    if (uniqueData != nullptr) {
        assert(prototype != nullptr); // No prototype available for copying unique data

        auto copied_unique = prototype->CopyUniqueData(uniqueData.get());
        to.uniqueData      = std::unique_ptr<UniqueDataT>(static_cast<UniqueDataT*>(copied_unique.release()));
    }
}

game::ChunkTileLayer::NonTopLeft::NonTopLeft(const NonTopLeft& /*other*/) : topLeft(nullptr) {}
