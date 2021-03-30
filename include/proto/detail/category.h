// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_DETAIL_CATEGORY_H
#define JACTORIO_INCLUDE_PROTO_DETAIL_CATEGORY_H
#pragma once

namespace jactorio::proto
{
    enum class Category
    {
        none = 0,

        // Abstract
        entity,
        health_entity,

        // Non entities
        item,
        label,
        localization,
        noise_layer_entity,
        noise_layer_tile,
        recipe,
        recipe_category,
        recipe_group,
        sound,
        sprite,
        tile,

        // Entities
        assembly_machine,
        container_entity,
        enemy_entity,
        inserter,
        mining_drill,
        resource_entity,
        splitter,
        transport_belt,


        // For test use only
        test,

        count_
    };
}

#endif // JACTORIO_INCLUDE_PROTO_DETAIL_CATEGORY_H
