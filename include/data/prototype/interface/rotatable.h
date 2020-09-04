// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
#define JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
#pragma once

#include "data/prototype/sprite.h"

namespace jactorio::game
{
    class WorldData;
}

namespace jactorio::data
{
    ///
    /// \brief Entities which can be rotated by the player
    class IRotatable
    {
    protected:
        IRotatable()          = default;
        virtual ~IRotatable() = default;

        IRotatable(const IRotatable& other)     = default;
        IRotatable(IRotatable&& other) noexcept = default;
        IRotatable& operator=(const IRotatable& other) = default;
        IRotatable& operator=(IRotatable&& other) noexcept = default;


    public:
        // The single sprite from entity serves as north
        PYTHON_PROP_I(Sprite*, spriteE, nullptr);
        PYTHON_PROP_I(Sprite*, spriteS, nullptr);
        PYTHON_PROP_I(Sprite*, spriteW, nullptr);
    };
} // namespace jactorio::data

#endif // JACTORIO_DATA_PROTOTYPE_INTERFACE_ROTATABLE_H
