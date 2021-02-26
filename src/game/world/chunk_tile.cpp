// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk_tile.h"

#include "core/coordinate_tuple.h"

using namespace jactorio;

game::ChunkTile::~ChunkTile() {
    if (IsTopLeft()) {
        data_.DestructTopLeft();
    }
    else {
        data_.DestructNonTopLeft();
    }
}

game::ChunkTile::ChunkTile(const ChunkTile& other) : common_(other.common_) {
    if (other.IsTopLeft()) {
        AsTopLeft() = other.AsTopLeft();
        other.AsTopLeft().CopyUniqueData(AsTopLeft(), GetPrototype());
    }
    else {
        AsNonTopLeft() = other.AsNonTopLeft();
    }
}

game::ChunkTile::ChunkTile(ChunkTile&& other) noexcept : common_(other.common_) {
    if (IsTopLeft()) {
        AsTopLeft() = std::move(other.AsTopLeft());
    }
    else {
        AsNonTopLeft() = std::move(other.AsNonTopLeft());
    }
}


void game::ChunkTile::Clear() noexcept {
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

void game::ChunkTile::SetOrientation(const Orientation orientation) noexcept {
    common_.orientation = orientation;
}

Orientation game::ChunkTile::GetOrientation() const noexcept {
    return common_.orientation;
}

void game::ChunkTile::SetPrototype(const Orientation orientation, PrototypeT& prototype) noexcept {
    SetPrototype(orientation, &prototype);
}

void game::ChunkTile::SetPrototype(const Orientation orientation, PrototypeT* prototype) noexcept {
    SetOrientation(orientation);
    common_.prototype = prototype;
}

void game::ChunkTile::SetPrototype(std::nullptr_t) noexcept {
    common_.prototype = nullptr;
}

// ======================================================================

bool game::ChunkTile::IsTopLeft() const noexcept {
    return IsTopLeft(common_.multiTileIndex);
}

bool game::ChunkTile::IsMultiTile() const noexcept {
    if (GetMultiTileIndex() > 0)
        return true;

    return GetDimension().span != 1 || GetDimension().height != 1;
}

bool game::ChunkTile::IsMultiTileTopLeft() const noexcept {
    return IsMultiTile() && IsTopLeft();
}

bool game::ChunkTile::IsNonTopLeft() const noexcept {
    return GetMultiTileIndex() != 0;
}


game::MultiTileData game::ChunkTile::GetDimension() const noexcept {
    if (GetPrototype() == nullptr) {
        return {1, 1};
    }
    return {GetPrototype()->GetWidth(GetOrientation()), GetPrototype()->GetHeight(GetOrientation())};
}

void game::ChunkTile::SetupMultiTile(const TileDistanceT multi_tile_index, ChunkTile& top_left) noexcept {
    assert(multi_tile_index > 0);

    data_.DestructTopLeft();
    data_.ConstructNonTopLeft();

    common_.multiTileIndex = multi_tile_index;

    assert(IsNonTopLeft());
    assert(&top_left != this);
    AsNonTopLeft().topLeft = &top_left;
}


game::ChunkTile::TileDistanceT game::ChunkTile::GetMultiTileIndex() const noexcept {
    return common_.multiTileIndex;
}

game::ChunkTile* game::ChunkTile::GetTopLeft() noexcept {
    return const_cast<ChunkTile*>(static_cast<const ChunkTile*>(this)->GetTopLeft());
}

const game::ChunkTile* game::ChunkTile::GetTopLeft() const noexcept {
    if (IsTopLeft())
        return this;

    assert(IsNonTopLeft());
    return AsNonTopLeft().topLeft;
}


// ======================================================================


game::ChunkTile::TileDistanceT game::ChunkTile::GetOffsetX() const noexcept {
    const auto& data = GetDimension();
    return GetMultiTileIndex() % data.span;
}

game::ChunkTile::TileDistanceT game::ChunkTile::GetOffsetY() const noexcept {
    const auto& data = GetDimension();
    return GetMultiTileIndex() / data.span;
}

bool game::ChunkTile::IsTopLeft(const TileDistanceT multi_tile_index) noexcept {
    return multi_tile_index == 0;
}

game::ChunkTile::TopLeft& game::ChunkTile::AsTopLeft() noexcept {
    assert(IsTopLeft());
    return data_.topLeft;
}

const game::ChunkTile::TopLeft& game::ChunkTile::AsTopLeft() const noexcept {
    assert(IsTopLeft());
    return data_.topLeft;
}

game::ChunkTile::NonTopLeft& game::ChunkTile::AsNonTopLeft() noexcept {
    assert(IsNonTopLeft());
    return data_.nonTopLeft;
}

const game::ChunkTile::NonTopLeft& game::ChunkTile::AsNonTopLeft() const noexcept {
    assert(IsNonTopLeft());
    return data_.nonTopLeft;
}

// ======================================================================

game::ChunkTile::TopLeft::TopLeft(const TopLeft& /*other*/) noexcept {}

void game::ChunkTile::TopLeft::CopyUniqueData(TopLeft& to, PrototypeT* prototype) const {
    // Use prototype defined method for copying uniqueData_ if other has data to copy
    if (uniqueData != nullptr) {
        assert(prototype != nullptr); // No prototype available for copying unique data

        auto copied_unique = prototype->CopyUniqueData(uniqueData.get());
        to.uniqueData      = std::unique_ptr<UniqueDataT>(static_cast<UniqueDataT*>(copied_unique.release()));
    }
}

game::ChunkTile::NonTopLeft::NonTopLeft(const NonTopLeft& /*other*/) : topLeft(nullptr) {}
