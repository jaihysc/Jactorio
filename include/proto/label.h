// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_LABEL_H
#define JACTORIO_INCLUDE_PROTO_LABEL_H
#pragma once

#include "proto/framework/framework_base.h"

namespace jactorio::proto
{
    /// Constants for all labels' names
    struct LabelNames
    {
        /// Access each label by appending the number which corresponds to each PlayerAction::Type
        ///
        /// e.g: __core__/label:player-action-0
        ///      __core__/label:player-action-1
        ///      __core__/label:player-action-2
        static constexpr auto kPlayerActionPrefix = "__core__/label:player-action-";

        static constexpr auto kKeyActionNone     = "__core__/label:key-action-none";
        static constexpr auto kKeyActionKDown    = "__core__/label:key-action-key-down";
        static constexpr auto kKeyActionKPressed = "__core__/label:key-action-key-pressed";
        static constexpr auto kKeyActionKRepeat  = "__core__/label:key-action-key-repeat";
        static constexpr auto kKeyActionKHeld    = "__core__/label:key-action-key-held";
        static constexpr auto kKeyActionKUp      = "__core__/label:key-action-key-up";

        static constexpr auto kKeyMouseLeft   = "__core__/label:key-mouse-left";
        static constexpr auto kKeyMouseMiddle = "__core__/label:key-mouse-middle";
        static constexpr auto kKeyMouseRight  = "__core__/label:key-mouse-right";
        static constexpr auto kKeyMouseX1     = "__core__/label:key-mouse-x1";
        static constexpr auto kKeyMouseX2     = "__core__/label:key-mouse-x2";

        static constexpr auto kMenuMain                      = "__core__/label:menu-main";
        static constexpr auto kMenuNewGame                   = "__core__/label:menu-new-game";
        static constexpr auto kMenuNewGameSeed               = "__core__/label:menu-new-game-seed";
        static constexpr auto kMenuNewGamePlay               = "__core__/label:menu-new-game-play";
        static constexpr auto kMenuLoadGame                  = "__core__/label:menu-load-game";
        static constexpr auto kMenuLoadGamePlay              = "__core__/label:menu-load-game-play";
        static constexpr auto kMenuSaveGame                  = "__core__/label:menu-save-game";
        static constexpr auto kMenuSaveGameInvalidName       = "__core__/label:menu-save-game-invalid-name";
        static constexpr auto kMenuSaveGameSaveName          = "__core__/label:menu-save-game-save-name";
        static constexpr auto kMenuSaveGameSave              = "__core__/label:menu-save-game-save";
        static constexpr auto kMenuOptions                   = "__core__/label:menu-options";
        static constexpr auto kMenuOptionChangeKeybind       = "__core__/label:menu-option-change-keybind";
        static constexpr auto kMenuOptionChangeKeybindKey    = "__core__/label:menu-option-change-keybind-key";
        static constexpr auto kMenuOptionChangeKeybindName   = "__core__/label:menu-option-change-keybind-name";
        static constexpr auto kMenuOptionChangeKeybindAction = "__core__/label:menu-option-change-keybind-action";
        static constexpr auto kMenuOptionChangeKeybindReset  = "__core__/label:menu-option-change-keybind-reset";
        static constexpr auto kMenuOptionToggleFullscreen    = "__core__/label:menu-option-toggle-fullscreen";
        static constexpr auto kMenuQuit                      = "__core__/label:menu-quit";
    };

    /// Stores localized text
    class Label final : public FrameworkBase
    {
    public:
        PROTOTYPE_CATEGORY(label);

        void PostLoadValidate(const data::PrototypeManager& proto) const override {}
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_LABEL_H
