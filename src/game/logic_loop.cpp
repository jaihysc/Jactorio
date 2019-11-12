#include "game/logic_loop.h"

#include "core/loop_manager.h"
#include "game/input/input_manager.h"
#include "core/logger.h"

namespace jactorio::game
{
	void logic_tick() {
		// Fire input event handlers
		input_manager::dispatch_input_callbacks();

		core::loop_manager::logic_loop_complete();
	}
}

void jactorio::game::logic_loop() {
	// Logic initialization here...
	
	core::loop_manager::logic_loop_ready(logic_tick);
}
