// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk_tile_layer.h"

#include "core/coordinate_tuple.h"

using namespace jactorio;

game::ChunkTileLayer::~ChunkTileLayer() {
    if (IsTopLeft())
        data_.DestroyUniqueData();
}

game::ChunkTileLayer::ChunkTileLayer(const ChunkTileLayer& other)
    : prototypeData(other.prototypeData), multiTileIndex_{other.multiTileIndex_} {

    // Use prototype defined method for copying uniqueData_ if other has data to copy
    if (IsTopLeft() && other.data_.uniqueData != nullptr) {
        assert(other.prototypeData.Get() != nullptr); // No prototype_data_ available for copying unique_data_
        data_.uniqueData = other.prototypeData->CopyUniqueData(other.data_.uniqueData.get());
    }
}

game::ChunkTileLayer::ChunkTileLayer(ChunkTileLayer&& other) noexcept
    : prototypeData(other.prototypeData), multiTileIndex_{other.multiTileIndex_} {
    data_.uniqueData = std::move(other.data_.uniqueData);
}


void game::ChunkTileLayer::Clear() noexcept {
    if (IsTopLeft()) {
        data_.DestroyUniqueData();
    }
    data_.ConstructUniqueData();

    prototypeData   = nullptr;
    multiTileIndex_ = 0;
}

// ======================================================================

bool game::ChunkTileLayer::IsTopLeft() const noexcept {
    return multiTileIndex_ == 0;
}

bool game::ChunkTileLayer::IsMultiTile() const {
    if (!HasMultiTileData())
        return false;

    return GetMultiTileData().span != 1 || GetMultiTileData().height != 1;
}

bool game::ChunkTileLayer::IsMultiTileTopLeft() const {
    return IsMultiTile() && IsTopLeft();
}

bool game::ChunkTileLayer::IsNonTopLeftMultiTile() const noexcept {
    return multiTileIndex_ != 0;
}


bool game::ChunkTileLayer::HasMultiTileData() const {
    return prototypeData != nullptr;
}

game::MultiTileData game::ChunkTileLayer::GetMultiTileData() const {
    assert(prototypeData != nullptr);
    return {prototypeData->GetWidth(), prototypeData->GetHeight()};
}


game::ChunkTileLayer::MultiTileValueT game::ChunkTileLayer::GetMultiTileIndex() const noexcept {
    return multiTileIndex_;
}

void game::ChunkTileLayer::SetMultiTileIndex(const MultiTileValueT multi_tile_index) {
    multiTileIndex_ = multi_tile_index;

    if (multi_tile_index != 0) {
        data_.DestroyUniqueData();
        data_.topLeft = nullptr;
    }
}


game::ChunkTileLayer* game::ChunkTileLayer::GetTopLeftLayer() const noexcept {
    assert(IsNonTopLeftMultiTile());
    return data_.topLeft;
}

void game::ChunkTileLayer::SetTopLeftLayer(ChunkTileLayer& ctl) noexcept {
    assert(IsNonTopLeftMultiTile());
    assert(&ctl != this);
    data_.topLeft = &ctl;
}


// ======================================================================


void game::ChunkTileLayer::AdjustToTopLeft(WorldCoord& coord) const {
    AdjustToTopLeft(coord.x, coord.y);
}

game::ChunkTileLayer::MultiTileValueT game::ChunkTileLayer::GetOffsetX() const {
    const auto& data = GetMultiTileData();
    return multiTileIndex_ % data.span;
}

game::ChunkTileLayer::MultiTileValueT game::ChunkTileLayer::GetOffsetY() const {
    const auto& data = GetMultiTileData();
    return multiTileIndex_ / data.span;
}
