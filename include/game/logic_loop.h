#ifndef JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
#define JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H

#include <mutex>

namespace jactorio::game
{
	/**
	 *
	 * @param mutex Will lock when the logic loop runs and unlock while sleeping until the next logic update
	 */
	void init_logic_loop(std::mutex* mutex);

	void terminate_logic_loop();
}

#endif //JACTORIO_INCLUDE_GAME_LOGIC_LOOP_H
