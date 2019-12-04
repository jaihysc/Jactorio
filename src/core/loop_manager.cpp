#include <GLFW/glfw3.h>

#include "core/loop_manager.h"

#include <thread>
#include <atomic>

#include "core/logger.h"

std::atomic<bool> loop_terminate;
std::thread loop_manager_thread;

jactorio::core::loop_manager::loop_run_callback render_callback;
jactorio::core::loop_manager::loop_run_callback logic_callback;
// 2 callbacks indicate it is ready as both callbacks have been set
std::atomic<unsigned short> callbacks_count = 0;

std::atomic<bool> render_loop_finished = true;
std::atomic<bool> logic_loop_finished = true;

void loop_manager_loop() {
	LOG_MESSAGE(debug, "Loop manager awaiting logic and render callbacks")
	// Wait for both loops to be ready
	while (!loop_terminate && callbacks_count != 2)
		;

	LOG_MESSAGE(debug, "Loop manager initialized")
	auto next_frame = std::chrono::steady_clock::now();
	while (!loop_terminate) {
		{
			if (!render_loop_finished) {
				// Missing a rendering tick is nowhere as severe as a logic tick, since it does not
				// affect the outcome of the game
				LOG_MESSAGE(debug, "Render loop missed tick")
				continue;
			}

			render_loop_finished = false;
			render_callback();
		}
		{
			if (!logic_loop_finished) {
				LOG_MESSAGE(warning, "Logic loop missed tick")
				continue;
			}

			logic_loop_finished = false;
			logic_callback();
		}

		// 1 / 60 seconds
		next_frame += std::chrono::nanoseconds(16666666);
		std::this_thread::sleep_until(next_frame);
	}
	LOG_MESSAGE(debug, "Loop manager terminated")
}

// ###################################
// Loop manager

void jactorio::core::loop_manager::initialize_loop_manager() {
	loop_terminate = false;
	loop_manager_thread = std::thread(loop_manager_loop);
}

void jactorio::core::loop_manager::terminate_loop_manager() {
	loop_terminate = true;
	loop_manager_thread.join();
}

bool jactorio::core::loop_manager::loop_manager_terminated() {
	return loop_terminate;
}


// ###################################
// Render
void jactorio::core::loop_manager::render_loop_ready(const loop_run_callback callback) {
	render_callback = callback;
	++callbacks_count;
}

void jactorio::core::loop_manager::render_loop_complete() {
	render_loop_finished = true;
}


// ###################################
// Logic
void jactorio::core::loop_manager::logic_loop_ready(const loop_run_callback callback) {
	logic_callback = callback;
	++callbacks_count;
}

void jactorio::core::loop_manager::logic_loop_complete() {
	logic_loop_finished = true;
}
