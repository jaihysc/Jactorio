#include "game/event/event.h"

// Initialize private static variables
std::unordered_map<jactorio::game::event_type, std::vector<void*>> jactorio::game::Event::event_handlers_{};