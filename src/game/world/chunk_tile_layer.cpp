// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk_tile_layer.h"

#include "core/coordinate_tuple.h"

using namespace jactorio;

game::ChunkTileLayer::~ChunkTileLayer() {
    if (IsTopLeft())
        data_.DestroyUniqueData();
}

game::ChunkTileLayer::ChunkTileLayer(const ChunkTileLayer& other)
    : prototypeData_{other.prototypeData_}, multiTileIndex_{other.multiTileIndex_}, orientation_{other.orientation_} {

    // Use prototype defined method for copying uniqueData_ if other has data to copy
    if (IsTopLeft() && other.data_.uniqueData != nullptr) {
        assert(other.prototypeData_.Get() != nullptr); // No prototype_data_ available for copying unique_data_
        data_.uniqueData = other.prototypeData_->CopyUniqueData(other.data_.uniqueData.get());
    }
}

game::ChunkTileLayer::ChunkTileLayer(ChunkTileLayer&& other) noexcept
    : prototypeData_{other.prototypeData_}, multiTileIndex_{other.multiTileIndex_}, orientation_{other.orientation_} {
    data_.uniqueData = std::move(other.data_.uniqueData);
}


void game::ChunkTileLayer::Clear() noexcept {
    if (IsTopLeft()) {
        data_.DestroyUniqueData();
    }
    data_.ConstructUniqueData();

    prototypeData_  = nullptr;
    multiTileIndex_ = 0;
}

void game::ChunkTileLayer::SetOrientation(const Orientation orientation) noexcept {
    if (IsNonTopLeftMultiTile()) {
        auto* top_left = GetTopLeftLayer();
        assert(top_left != nullptr);

        top_left->SetOrientation(orientation);
    }
    else {
        orientation_ = orientation;
    }
}

Orientation game::ChunkTileLayer::GetOrientation() const noexcept {
    if (IsNonTopLeftMultiTile()) {
        const auto* top_left = GetTopLeftLayer();
        assert(top_left != nullptr);

        return top_left->GetOrientation();
    }

    return orientation_;
}

void game::ChunkTileLayer::SetPrototype(const Orientation orientation, PrototypeT* prototype) noexcept {
    SetOrientation(orientation);
    prototypeData_ = prototype;
}

void game::ChunkTileLayer::SetPrototype(std::nullptr_t) noexcept {
    prototypeData_ = nullptr;
}

// ======================================================================

bool game::ChunkTileLayer::IsTopLeft() const noexcept {
    return multiTileIndex_ == 0;
}

bool game::ChunkTileLayer::IsMultiTile() const noexcept {
    if (!HasMultiTileData())
        return false;

    return GetMultiTileData().span != 1 || GetMultiTileData().height != 1;
}

bool game::ChunkTileLayer::IsMultiTileTopLeft() const noexcept {
    return IsMultiTile() && IsTopLeft();
}

bool game::ChunkTileLayer::IsNonTopLeftMultiTile() const noexcept {
    return multiTileIndex_ != 0;
}


bool game::ChunkTileLayer::HasMultiTileData() const noexcept {
    return prototypeData_ != nullptr;
}

game::MultiTileData game::ChunkTileLayer::GetMultiTileData() const noexcept {
    assert(prototypeData_ != nullptr);
    return {prototypeData_->GetWidth(), prototypeData_->GetHeight()};
}


game::ChunkTileLayer::MultiTileValueT game::ChunkTileLayer::GetMultiTileIndex() const noexcept {
    return multiTileIndex_;
}

void game::ChunkTileLayer::SetMultiTileIndex(const MultiTileValueT multi_tile_index) noexcept {
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


void game::ChunkTileLayer::AdjustToTopLeft(WorldCoord& coord) const noexcept {
    AdjustToTopLeft(coord.x, coord.y);
}

game::ChunkTileLayer::MultiTileValueT game::ChunkTileLayer::GetOffsetX() const noexcept {
    const auto& data = GetMultiTileData();
    return multiTileIndex_ % data.span;
}

game::ChunkTileLayer::MultiTileValueT game::ChunkTileLayer::GetOffsetY() const noexcept {
    const auto& data = GetMultiTileData();
    return multiTileIndex_ / data.span;
}
