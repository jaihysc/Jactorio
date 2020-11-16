// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/player_action.h"

using namespace jactorio;

game::PlayerAction::Executor& game::PlayerAction::GetExecutor(const Type type) {
    assert(type != Type::count_);

    return executors_[static_cast<int>(type)];
}

void game::PlayerAction::PlayerMoveUp(GameDataGlobal& data_global) {}
void game::PlayerAction::PlayerMoveRight(GameDataGlobal& data_global) {}
void game::PlayerAction::PlayerMoveDown(GameDataGlobal& data_global) {}
void game::PlayerAction::PlayerMoveLeft(GameDataGlobal& data_global) {}

void game::PlayerAction::ActionTest(GameDataGlobal& data_global) {
    data_global.player.world.SetPlayerX(-100.f);
    data_global.player.world.SetPlayerY(120.f);
}
