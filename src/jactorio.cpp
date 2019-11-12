﻿#include <iostream>
#include <thread>

#include "jactorio.h"

#include "core/filesystem.h"
#include "core/logger.h"
#include "core/loop_manager.h"
#include "data/data_manager.h"
#include "data/pybind/pybind_manager.h"
#include "game/logic_loop.h"
#include "renderer/render_main.h"


int main(int ac, char* av[]) {
	// Initial startup message
	std::cout << JACTORIO_BUILD_TARGET_PLATFORM << "  |  " << BUILD_TYPE << " build, version: " <<
		JACTORIO_VERSION << "\n\n\n";

	jactorio::core::filesystem::set_executing_directory(av[0]);

	{
		jactorio::core::loop_manager::initialize_loop_manager();

		
		// Rendering + logic initialization
		log_message(jactorio::core::logger::info, "Jactorio", "1 - Data stage");

		jactorio::data::pybind_manager::py_interpreter_init();

		jactorio::data::data_manager::load_data(
			jactorio::core::filesystem::resolve_path("~/data")
		);

		// Loop manager terminated in renderer_thread as it requires glfw for time
		jactorio::game::logic_loop();
		std::thread renderer_thread = std::thread(jactorio::renderer::render_init);

		renderer_thread.join();
	}


	jactorio::data::pybind_manager::py_interpreter_terminate();

	std::cout << "goodbye!\n";
	return 0;
}
