// 
// event.cpp
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/20/2020
// Last modified: 03/14/2020
// 

#include "game/event/event.h"

// Initialize private static variables
std::unordered_map<jactorio::game::event_type, std::vector<void(*)()>> jactorio::game::Event::event_handlers_{};
std::unordered_map<jactorio::game::event_type, std::vector<void(*)()>> jactorio::game::Event::event_handlers_once_{};
