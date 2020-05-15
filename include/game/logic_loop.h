// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#define JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#pragma once

namespace jactorio::game
{
	void InitLogicLoop();

	inline bool logic_thread_should_exit = false;
}

#endif //JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
