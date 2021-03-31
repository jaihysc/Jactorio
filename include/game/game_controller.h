// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_GAME_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_GAME_CONTROLLER_H
#pragma once

#include "data/prototype_manager.h"
#include "data/unique_data_manager.h"
#include "game/event/event.h"
#include "game/input/input_manager.h"
#include "game/input/mouse_selection.h"
#include "game/logic/logic.h"
#include "game/player/keybind_manager.h"
#include "game/player/player.h"
#include "game/world/world.h"

namespace jactorio::game
{
    /// Top level class for controlling game simulation
    class GameController
    {
        static constexpr auto kDefaultWorldCount = 1;

        static constexpr auto kSettingsPath = "settings.json";

    public:
        /// Clears worlds, logic, player
        void ResetGame();

        /// Sets up game for logic updates
        /// \return false if error
        J_NODISCARD bool Init();

        /// One simulation tick update
        void LogicUpdate();


        // Non serialized

        struct GameInput
        {
            MouseSelection mouse;
            InputManager key;
        };

        data::PrototypeManager proto;
        data::UniqueDataManager unique;

        GameInput input;
        EventData event;

        // Serialized settings

        KeybindManager keybindManager{input.key, *this};
        std::string localIdentifier = "en";

        // Serialized per game

        GameWorlds worlds{kDefaultWorldCount};
        Logic logic;
        Player player;

        static_assert(std::is_same_v<GameWorlds::size_type, WorldId>);


        /// \exception std::runtime_error Failed to save
        void SaveSetting() const;

        /// \exception std::runtime_error Failed to load
        void LoadSetting();

        /// \param save_name name of save, no extensions. E.g: "first world"
        /// \exception std::runtime_error Failed to save
        void SaveGame(const char* save_name) const;

        /// \param save_name name of save, no extensions. E.g: "first world"
        /// \exception std::runtime_error Failed to load
        void LoadGame(const char* save_name);

    private:
        /// \return false if error
        J_NODISCARD bool InitPrototypes();


        template <typename T>
        void SerializeSetting(T& archive) {
            archive(keybindManager);
            archive(localIdentifier);
        }

        template <typename T>
        void SerializeGame(T& archive) {
            // Order must be: world, logic, player
            archive(worlds);
            archive(logic);
            archive(player);
        }
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_GAME_CONTROLLER_H
