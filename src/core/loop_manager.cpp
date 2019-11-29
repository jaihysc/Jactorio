#include <GLFW/glfw3.h>

#include "core/loop_manager.h"

#include <thread>
#include <atomic>

#include "core/logger.h"

namespace jactorio::core::loop_manager
{
	std::atomic<bool> loop_terminate;
	std::thread loop_manager_thread;

	loop_run_callback render_callback;
	loop_run_callback logic_callback;
	// 2 callbacks indicate it is ready as both callbacks have been set
	std::atomic<unsigned short> callbacks_count = 0;

	// Render
	unsigned short render_refresh_rate = 60;
	std::atomic<float> render_update_interval = 1.f / static_cast<float>(render_refresh_rate);

	// Logic
	unsigned short logic_refresh_rate = 60;
	std::atomic<float> logic_update_interval = 1.f / static_cast<float>(logic_refresh_rate);

	// Used for running the loops
	double render_last_time = 0;
	double logic_last_time = 0;

	std::atomic<bool> render_loop_finished = true;
	std::atomic<bool> logic_loop_finished = true;

	void loop_manager_loop() {
		LOG_MESSAGE(debug, "Loop manager awaiting logic and render callbacks")
		// Wait for both loops to be ready
		while (!loop_terminate && callbacks_count != 2)
			;

		LOG_MESSAGE(debug, "Loop manager initialized")
		while (!loop_terminate) {
			const double time = glfwGetTime();

			if (time - render_last_time > render_update_interval) {
				render_last_time = time;

				if (!render_loop_finished) {
					LOG_MESSAGE(warning, "Render loop missed tick")
					continue;
				}

				render_loop_finished = false;
				render_callback();
			}
			if (time - logic_last_time > logic_update_interval) {
				logic_last_time = time;

				if (!logic_loop_finished) {
					LOG_MESSAGE(warning, "Logic loop missed tick")
					continue;
				}

				logic_loop_finished = false;
				logic_callback();
			}
		}
		LOG_MESSAGE(debug, "Loop manager terminated")
	}
}

// ###################################
// Loop manager settings
// 
void jactorio::core::loop_manager::set_render_refresh_rate(const unsigned short refresh_rate) {
	render_refresh_rate = refresh_rate;
	render_update_interval = 1.f / static_cast<float>(render_refresh_rate);
}

unsigned short jactorio::core::loop_manager::get_render_refresh_rate() {
	return render_refresh_rate;
}


void jactorio::core::loop_manager::set_logic_refresh_rate(const unsigned short refresh_rate) {
	logic_refresh_rate = refresh_rate;
	logic_update_interval = 1.f / static_cast<float>(logic_refresh_rate);
}

unsigned short jactorio::core::loop_manager::get_logic_refresh_rate() {
	return logic_refresh_rate;
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
