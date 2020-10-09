// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_INTERFACE_ROTATABLE_H
#define JACTORIO_INCLUDE_PROTO_INTERFACE_ROTATABLE_H
#pragma once

namespace jactorio
{
    namespace proto
    {
        class Sprite;
    }
    namespace game
    {
        class WorldData;

    }
} // namespace jactorio

namespace jactorio::proto
{
    ///
    /// Entities which can be rotated by the player
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
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_INTERFACE_ROTATABLE_H
