// 
// logic_loop.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/12/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#define JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#pragma once

#include <mutex>

namespace jactorio::game
{
	void init_logic_loop(std::mutex* mutex);

	void terminate_logic_loop();
}

#endif //JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
