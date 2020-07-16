// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#define JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#pragma once

#include <atomic>

namespace jactorio::game
{
	void InitLogicLoop();

	inline bool logic_thread_should_exit                = false;
	inline std::atomic<bool> prototype_loading_complete = false;
}

#endif //JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
