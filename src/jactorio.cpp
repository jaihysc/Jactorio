﻿#include <iostream>
#include <thread>

#include "jactorio.h"

#include "core/resource_guard.h"
#include "core/filesystem.h"
#include "core/logger.h"
#include "data/data_manager.h"
#include "data/pybind/pybind_manager.h"
#include "game/logic_loop.h"
#include "renderer/render_main.h"


int main(int ac, char* av[]) {
	using namespace jactorio;

	core::filesystem::set_executing_directory(av[0]);

	// Log file
	core::logger::open_log_file("~/log.txt");
	auto log_guard = core::Resource_guard(&core::logger::close_log_file);
	
	// Initial startup message
	LOG_MESSAGE_f(none, "%s | %s build, version: %s\n\n", 
	              JACTORIO_BUILD_TARGET_PLATFORM, BUILD_TYPE, JACTORIO_VERSION)

	{
		// Rendering + logic initialization
		LOG_MESSAGE(info, "1 - Data stage");

		auto py_interpreter_guard = core::Resource_guard(&data::pybind_manager::py_interpreter_terminate);
		data::pybind_manager::py_interpreter_init();
		

		auto data_manager_guard = core::Resource_guard(&data::data_manager::clear_data);
		data::data_manager::load_data(core::filesystem::resolve_path("~/data"));

		
		std::thread logic_thread = std::thread(game::logic_loop);
		std::thread renderer_thread = std::thread(renderer::render_init);
		logic_thread.join();
		renderer_thread.join();
	}

	LOG_MESSAGE(none, "goodbye!");
	return 0;
}
