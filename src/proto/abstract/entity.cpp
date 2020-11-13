// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include "proto/abstract/entity.h"

#include "proto/item.h"

using namespace jactorio;

proto::Entity* proto::Entity::SetItem(Item* item) {
    item->entityPrototype = this;
    this->item_           = item;

    return this;
}

// ======================================================================

void proto::Entity::SetLocalizedName(const std::string& localized_name) {
    this->localizedName_ = localized_name;
    if (item_ != nullptr)
        item_->SetLocalizedName(localized_name);
}

void proto::Entity::SetLocalizedDescription(const std::string& localized_description) {
    this->localizedDescription_ = localized_description;
    if (item_ != nullptr)
        item_->SetLocalizedDescription(localized_description);
}

// ======================================================================

void proto::Entity::PostLoadValidate(const data::PrototypeManager& /*data_manager*/) const {

    J_DATA_ASSERT(sprite != nullptr, "Sprite was not specified");
    J_DATA_ASSERT(pickupTime >= 0, "Pickup time must be 0 or positive");
}
